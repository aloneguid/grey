#include "process.h"
#include <chrono>
#include <filesystem>
#include <format>
#include "str.h"

using namespace std;
using namespace std::chrono;

#if PLATFORM_WINDOWS

#include <Pdh.h>
#include <PdhMsg.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <winnt.h>
#include <winternl.h>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "version.lib")

#define MAX_STR 1024
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#define MAX_EXTENDED_PATH 32768

namespace grey::common {
    process::process() : pid{::GetCurrentProcessId()} {
    }

    process::~process() {
        if(pdhCpuInitialised) {
            ::PdhRemoveCounter(pdhCpuCounter);
            ::PdhCloseQuery(pdhCpuQuery);
        }
    }

    vector<process> process::enumerate() {
        vector<process> r;

        DWORD process_ids[1024], needed;
        if(::EnumProcesses(process_ids, sizeof(process_ids), &needed)) {
            int proc_count = needed / sizeof(DWORD);
            for(int i = 0; i < proc_count; i++) {
                auto process_id = process_ids[i];

                r.emplace_back(process_id);
            }
        }

        return r;
    }

    DWORD process::start(const std::string &cmdline, bool wait_for_exit) {
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};
        DWORD pid{0};

        if(::CreateProcess(nullptr,
                           const_cast<wchar_t *>(str::to_wstr(cmdline).c_str()),
                           nullptr,
                           nullptr,
                           false,
                           0,
                           nullptr,
                           nullptr,
                           &si,
                           &pi)) {
            if(wait_for_exit) {
                ::WaitForSingleObject(pi.hProcess, INFINITE);
            }

            pid = pi.dwProcessId;

            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);
        }

        return pid;
    }

    int process::exec(const std::string &cmdline, std::function<void(std::string &)> std_out_new_data) {
        HANDLE hStdOutRead = nullptr, hStdOutWrite = nullptr;
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE; // Allow the child process to inherit the handle
        sa.lpSecurityDescriptor = nullptr;

        // Create a pipe for the child process's standard output
        if(!::CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
            return -1; // Failed to create pipe
        }

        // Ensure the read handle to the pipe is not inherited
        if(!::SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
            ::CloseHandle(hStdOutRead);
            ::CloseHandle(hStdOutWrite);
            return -1; // Failed to set handle information
        }

        STARTUPINFO si{};
        si.cb = sizeof(STARTUPINFO);
        si.hStdOutput = hStdOutWrite; // Redirect standard output to the writing end of the pipe
        si.hStdError = hStdOutWrite; // Redirect standard error to the same pipe
        si.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION pi{};
        DWORD exit_code = -1;

        // Convert the command line to a wide string
        std::wstring wCmdline = str::to_wstr(cmdline);

        // Create the child process
        if(::CreateProcess(
            nullptr,
            const_cast<wchar_t *>(wCmdline.c_str()),
            nullptr,
            nullptr,
            TRUE, // Inherit handles
            0,
            nullptr,
            nullptr,
            &si,
            &pi)) {
            // Close the write end of the pipe in the parent process
            ::CloseHandle(hStdOutWrite);

            // Read the output from the child process
            char buffer[4096];
            DWORD bytesRead;
            while(::ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                buffer[bytesRead] = '\0'; // Null-terminate the buffer
                string token{buffer};
                std_out_new_data(token);
            }

            // Wait for the child process to exit
            ::WaitForSingleObject(pi.hProcess, INFINITE);

            // Get the exit code of the process
            ::GetExitCodeProcess(pi.hProcess, &exit_code);

            // Close handles
            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);
        } else {
            // Failed to create process
            ::CloseHandle(hStdOutWrite);
        }

        ::CloseHandle(hStdOutRead);
        return static_cast<int>(exit_code);
    }

    int process::exec(const std::string &cmdline, string &std_out) {
        return exec(cmdline, [&](string &token) {
            std_out += token;
        });
    }

    std::string process::get_module_filename() const {
        // Try the modern API first: needs only LIMITED_INFORMATION,
        // works on elevated/protected processes, handles long paths.
        {
            HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
            if(hProcess) {
                wchar_t buf[32768];
                DWORD len = static_cast<DWORD>(std::size(buf));
                if(::QueryFullProcessImageNameW(hProcess, 0, buf, &len)) {
                    ::CloseHandle(hProcess);
                    return str::to_str(std::wstring(buf, len));
                }
                ::CloseHandle(hProcess);
            }
        }

        // Fall back to the older API.
        HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if(!hProcess) return "";

        wchar_t buf[MAX_PATH];
        std::string r;
        if(::GetModuleFileNameExW(hProcess, nullptr, buf, MAX_PATH)) {
            r = str::to_str(std::wstring(buf));
        }
        ::CloseHandle(hProcess);
        return r;
    }

    std::string process::get_name() const {
        string mfn = get_module_filename();

        size_t idx = mfn.find_last_of('\\');
        if(idx != string::npos) {
            mfn = mfn.substr(idx + 1);
        }

        return mfn;
    }

    struct find_token {
        HWND best_handle{0};
        DWORD process_id{0};
    };

    BOOL CALLBACK FindMainWindowEnumWindowsProc(HWND hwnd, LPARAM lParam) {
        find_token *t = (find_token *) lParam;

        HWND owner = ::GetWindow(hwnd, GW_OWNER);
        bool is_visible = ::IsWindowVisible(hwnd);
        bool is_main_window = !owner && is_visible;

        DWORD process_id;
        ::GetWindowThreadProcessId(hwnd, &process_id);

        if(process_id != t->process_id || !is_main_window)
            return TRUE;

        t->best_handle = hwnd;
        return FALSE;
    }

    std::string process::get_description() const {
        // --- 1. Get executable path ---
        std::wstring exePath = str::to_wstr(get_module_filename());
        if(exePath.empty()) return "";

        // --- 2. Load version-info block ---
        DWORD dummy = 0;
        DWORD viSize = ::GetFileVersionInfoSizeW(exePath.c_str(), &dummy);
        if(viSize == 0) return "";

        std::vector<BYTE> vi(viSize);
        if(!::GetFileVersionInfoW(exePath.c_str(), 0, viSize, vi.data())) return "";

        // --- 3. Enumerate translations actually present in this binary ---
        struct LANGANDCODEPAGE {
            WORD language;
            WORD codepage;
        };

        LANGANDCODEPAGE *translations = nullptr;
        UINT translationBytes = 0;
        ::VerQueryValueW(vi.data(), L"\\VarFileInfo\\Translation",
                         reinterpret_cast<void **>(&translations), &translationBytes);

        const UINT translationCount = translationBytes / sizeof(LANGANDCODEPAGE);

        auto queryDescription = [&](WORD lang, WORD cp) -> std::wstring {
            wchar_t subBlock[64];
            ::swprintf_s(subBlock, L"\\StringFileInfo\\%04X%04X\\FileDescription",
                         lang, cp);
            void *buf = nullptr;
            UINT sz = 0;
            if(::VerQueryValueW(vi.data(), subBlock, &buf, &sz) && sz > 0) {
                return std::wstring(static_cast<wchar_t *>(buf));
            }
            return {};
        };

        for(UINT i = 0; i < translationCount; ++i) {
            auto desc = queryDescription(translations[i].language,
                                         translations[i].codepage);
            if(!desc.empty()) return str::to_str(desc);
        }

        // --- 4. Try common fallback combos that some tools emit without declaring ---
        for(auto [lang, cp]: std::initializer_list<std::pair<WORD, WORD> >{
                {0x0409, 0x04B0},
                {0x0409, 0x04E4},
                {0x0000, 0x04B0},
            }) {
            auto desc = queryDescription(lang, cp);
            if(!desc.empty()) return str::to_str(desc);
        }

        return "";
    }

    HWND process::find_main_window() {
        find_token t;
        t.process_id = pid;

        ::EnumWindows(FindMainWindowEnumWindowsProc, (LPARAM) &t);

        return t.best_handle;
    }

    void process::set_priority(DWORD priority_class) {
        HANDLE hProcess = ::OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
        if(hProcess) {
            ::SetPriorityClass(hProcess, priority_class);

            ::CloseHandle(hProcess);
        }
    }

    bool process::enable_efficiency_mode() {
        BOOL ok = FALSE;
        HANDLE hProcess = ::OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
        if(hProcess) {
            PROCESS_POWER_THROTTLING_STATE pts = {0};
            pts.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
            pts.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
            pts.StateMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;

            ok = ::SetProcessInformation(hProcess, ProcessPowerThrottling, &pts, sizeof(pts));

            ::CloseHandle(hProcess);
        }
        return ok;
    }

    process process::get_parent() {
        HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if(snapshot == INVALID_HANDLE_VALUE)
            return process{0};

        PROCESSENTRY32 entry{};
        entry.dwSize = sizeof(entry); // must be set before Process32First, or the call fails

        DWORD parentPid = 0;
        if(::Process32First(snapshot, &entry)) {
            do {
                if(entry.th32ProcessID == pid) {
                    parentPid = entry.th32ParentProcessID;
                    break;
                }
            } while(::Process32Next(snapshot, &entry));
        }

        ::CloseHandle(snapshot);
        return process{parentPid};
    }

    bool process::get_memory_info(uint64_t &working_set_bytes) {
        bool ok{false};
        // see https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessmemoryinfo
        HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if(hProcess) {
            PROCESS_MEMORY_COUNTERS pmc{0};
            ::GetProcessMemoryInfo(hProcess, &pmc, sizeof(PROCESS_MEMORY_COUNTERS));
            working_set_bytes = pmc.WorkingSetSize;

            ::CloseHandle(hProcess);

            ok = true;
        }
        return ok;
    }

    double process::get_uptime_sec() {
        double r{0};
        HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if(hProcess) {
            FILETIME creationTime, exitTime, kernelTime, userTime;
            if(::GetProcessTimes(
                hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                // convert to time_point
                file_clock::duration d{
                    (static_cast<int64_t>(creationTime.dwHighDateTime) << 32) | creationTime.dwLowDateTime
                };
                file_clock::time_point creation_time{d};

                // get duration
                auto uptime = file_clock::now() - creation_time;
                auto uptime_sec = duration_cast<seconds>(uptime);
                r = uptime_sec.count();
            }
            ::CloseHandle(hProcess);
        }
        return r;
    }

    bool process::terminate() {
        HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        bool ok = false;
        if(hProcess) {
            ok = ::TerminateProcess(hProcess, 1);
            ::CloseHandle(hProcess);
        }
        return ok;
    }

    double process::get_cpu_usage_percentage() {
        double perc{-1.0};

        // initialise pdh
        if(!pdhCpuInitialised) {
            auto status = ::PdhOpenQuery(NULL, NULL, &pdhCpuQuery);
            if(status == ERROR_SUCCESS) {
                // get process name and strip out extension
                string process_name = get_name();
                size_t idx = process_name.find_last_of('.');
                if(idx != string::npos) process_name = process_name.substr(0, idx);

                auto path = str::to_wstr(std::format("\\Process V2({}:8520)\\% Processor Time", process_name));
                status = ::PdhAddEnglishCounter(pdhCpuQuery, path.c_str(), NULL, &pdhCpuCounter);
                if(status == ERROR_SUCCESS) {
                    pdhCpuInitialised = true;
                } else {
                    ::PdhCloseQuery(pdhCpuQuery);
                    pdhCpuQuery = 0;
                }
            }
        }

        // get sample
        if(pdhCpuInitialised && pdhCpuQuery != 0) {
            auto status = ::PdhCollectQueryData(pdhCpuQuery);
            if(status == ERROR_SUCCESS) {
                // Retrieve the counter value
                PDH_FMT_COUNTERVALUE fv;
                status = ::PdhGetFormattedCounterValue(pdhCpuCounter, PDH_FMT_DOUBLE, NULL, &fv);
                if(ERROR_SUCCESS == status) {
                    perc = fv.doubleValue;
                } else {
                    if(status == PDH_INVALID_ARGUMENT) {
                        perc = -2;
                    } else if(status == PDH_INVALID_DATA) {
                        perc = -3;
                    } else if(status == PDH_INVALID_HANDLE) {
                        perc = -4;
                    }
                }
            }
        }

        return perc;
    }

#else

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <libgen.h>
#include <climits>
#include <cctype>
#include <cstdlib>

    process::process() : pid{getpid()} {
    }

    process::~process() {
    }

    vector<process> process::enumerate() {
        vector<process> r;
        DIR *dir = opendir("/proc");
        if(dir) {
            struct dirent *entry;
            while((entry = readdir(dir)) != nullptr) {
                if(isdigit(entry->d_name[0])) {
                    r.emplace_back(static_cast<PidType>(atoi(entry->d_name)));
                }
            }
            closedir(dir);
        }
        return r;
    }

    process::PidType process::start(const std::string &cmdline, bool wait_for_exit) {
        pid_t pid = fork();
        if(pid == 0) {
            // Child process
            execl("/bin/sh", "sh", "-c", cmdline.c_str(), (char *) nullptr);
            _exit(1);
        } else if(pid > 0) {
            // Parent process
            if(wait_for_exit) {
                int status;
                waitpid(pid, &status, 0);
            }
            return pid;
        }
        return 0;
    }

    int process::exec(const std::string &cmdline, std::function<void(std::string &)> std_out_new_data) {
        int pipefd[2];
        if(pipe(pipefd) == -1) return -1;

        pid_t pid = fork();
        if(pid == 0) {
            // Child process
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);

            execl("/bin/sh", "sh", "-c", cmdline.c_str(), (char *) nullptr);
            _exit(1);
        } else if(pid > 0) {
            // Parent process
            close(pipefd[1]);
            char buffer[4096];
            ssize_t bytesRead;
            while((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytesRead] = '\0';
                string s(buffer);
                std_out_new_data(s);
            }
            close(pipefd[0]);

            int status;
            waitpid(pid, &status, 0);
            if(WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            return -1;
        }
        return -1;
    }

    int process::exec(const std::string &cmdline, string &std_out) {
        return exec(cmdline, [&](string &token) {
            std_out += token;
        });
    }

    std::string process::get_module_filename() const {
        char buf[PATH_MAX];
        string path = "/proc/" + to_string(pid) + "/exe";
        ssize_t len = readlink(path.c_str(), buf, sizeof(buf) - 1);
        if(len != -1) {
            buf[len] = '\0';
            return string(buf);
        }
        return "";
    }

    std::string process::get_name() const {
        string mfn = get_module_filename();
        if(mfn.empty()) return "";
        size_t idx = mfn.find_last_of('/');
        if(idx != string::npos) {
            return mfn.substr(idx + 1);
        }
        return mfn;
    }

    std::string process::get_description() const {
        string path = "/proc/" + to_string(pid) + "/comm";
        ifstream f(path);
        string line;
        if(getline(f, line)) {
            return line;
        }
        return get_name();
    }

    HWND process::find_main_window() {
        return nullptr;
    }

    void process::set_priority(DWORD priority_class) {
        setpriority(PRIO_PROCESS, pid, 0);
    }

    bool process::enable_efficiency_mode() {
        return setpriority(PRIO_PROCESS, pid, 19) == 0;
    }

    process process::get_parent() {
        string path = "/proc/" + to_string(pid) + "/stat";
        ifstream f(path);
        if(!f.is_open()) return process(0);
        string line;
        getline(f, line);
        size_t last_paren = line.find_last_of(')');
        if(last_paren == string::npos) return process(0);
        stringstream ss(line.substr(last_paren + 2));
        char state;
        pid_t ppid;
        ss >> state >> ppid;
        return process(ppid);
    }

    bool process::get_memory_info(uint64_t &working_set_bytes) {
        string path = "/proc/" + to_string(pid) + "/statm";
        ifstream f(path);
        uint64_t size, resident;
        if(f >> size >> resident) {
            working_set_bytes = resident * sysconf(_SC_PAGESIZE);
            return true;
        }
        return false;
    }

    double process::get_uptime_sec() {
        string path = "/proc/" + to_string(pid) + "/stat";
        ifstream f(path);
        if(!f.is_open()) return 0;

        string line;
        getline(f, line);
        size_t last_paren = line.find_last_of(')');
        if(last_paren == string::npos) return 0;
        stringstream ss(line.substr(last_paren + 2));

        char state;
        pid_t ppid, pgrp, session, tty_nr, tpgid;
        uint32_t flags;
        uint64_t minflt, cminflt, majflt, cmajflt, utime, stime, cutime, cstime, priority, nice, num_threads,
                itrealvalue;
        unsigned long long starttime;

        ss >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt >>
                utime >> stime >> cutime >> cstime >> priority >> nice >> num_threads >> itrealvalue >> starttime;

        ifstream uptime_f("/proc/uptime");
        double system_uptime;
        uptime_f >> system_uptime;

        long clock_ticks = sysconf(_SC_CLK_TCK);
        double start_time_sec = (double) starttime / clock_ticks;

        return system_uptime - start_time_sec;
    }

    bool process::terminate() {
        return kill(pid, SIGTERM) == 0;
    }

    double process::get_cpu_usage_percentage() {
        auto get_total_system_time = []() -> uint64_t {
            ifstream f("/proc/stat");
            string line;
            if(!getline(f, line)) return 0;
            stringstream ss(line);
            string cpu;
            ss >> cpu;
            uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
            if(ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal) {
                return user + nice + system + idle + iowait + irq + softirq + steal;
            }
            return 0;
        };

        auto get_process_times = [this](uint64_t &utime, uint64_t &stime) -> bool {
            string path = "/proc/" + to_string(pid) + "/stat";
            ifstream f(path);
            string line;
            if(!getline(f, line)) return false;
            size_t last_paren = line.find_last_of(')');
            if(last_paren == string::npos) return false;
            stringstream ss(line.substr(last_paren + 2));
            char state;
            pid_t ppid, pgrp, session, tty_nr, tpgid;
            uint32_t flags;
            uint64_t minflt, cminflt, majflt, cmajflt;
            if(ss >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >>
               cmajflt >> utime >> stime) {
                return true;
            }
            return false;
        };

        uint64_t utime = 0, stime = 0;
        if(!get_process_times(utime, stime)) return -1.0;
        uint64_t system_time = get_total_system_time();
        if(system_time == 0) return -1.0;

        if(!cpu_initialised) {
            last_utime = utime;
            last_stime = stime;
            last_system_time = system_time;
            cpu_initialised = true;
            return 0.0;
        }

        double perc = 0.0;
        if(system_time > last_system_time) {
            uint64_t process_diff = (utime >= last_utime ? utime - last_utime : 0) +
                                    (stime >= last_stime ? stime - last_stime : 0);
            uint64_t system_diff = system_time - last_system_time;
            perc = 100.0 * (double) process_diff / (double) system_diff;
        }

        last_utime = utime;
        last_stime = stime;
        last_system_time = system_time;

        return perc;
    }

#endif
}

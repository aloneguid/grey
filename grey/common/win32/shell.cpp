#include "shell.h"
#include <ShlObj_core.h>
#include <shellapi.h>
#include <filesystem>
#include "../../common/str.h"
#include "../../common/fss.h"

using namespace std;
namespace fs = std::filesystem;

namespace grey::common::win32 {
    namespace shell {
        static bool co_initialised{false};

        static void ensure_co_initalised() {
            if(co_initialised) return;

            HRESULT ok = ::CoInitializeEx(0, COINIT_MULTITHREADED);

            co_initialised = true;
        }

        string get_shell_folder_path(int clsid) {
            TCHAR szPath[MAX_PATH];

            if(SUCCEEDED(::SHGetFolderPath(nullptr,
                clsid,
                nullptr,
                0,
                szPath))) {
                return str::to_str(szPath);
            }

            return "";
        }

        string get_local_app_data_path() {
            return get_shell_folder_path(CSIDL_LOCAL_APPDATA);
        }

        std::string get_app_data_folder() {
            return get_shell_folder_path(CSIDL_APPDATA);
        }

        void exec(const std::string& path, const std::string& parameters) {
            HINSTANCE hi = ::ShellExecute(
                nullptr,
                L"open",
                str::to_wstr(path).c_str(),
                str::to_wstr(parameters).c_str(),
                nullptr,
                SW_SHOWDEFAULT);

            /*fs::path fs_path(path);
            string work_dir = fs_path.parent_path();
            string process_name = fs_path.filename();

            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ::ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ::ZeroMemory(&pi, sizeof(pi));

            string all = string(L"\"") + path + L"\" " + parameters;

            if (!::CreateProcess(
                path.c_str(),
                (LPWSTR)parameters.c_str(),
                nullptr,
                nullptr,
                false,
                0,
                nullptr,
                work_dir.c_str(),
                &si,
                &pi))
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_SUCCESS)
                {
                    return;
                }

            }

            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);*/
        }

        void set_rounded_corners(HWND hWnd, int radius /*= 20*/) {
            RECT rect;
            if(::GetWindowRect(hWnd, &rect)) {
                HRGN rgn = CreateRoundRectRgn(0, 0, rect.right - rect.left, rect.bottom - rect.top, radius, radius);
                ::SetWindowRgn(hWnd, rgn, TRUE);
            }
        }

        void send_wm_copydata(HWND hWnd, const std::string& data, long data_type) {
            COPYDATASTRUCT cds;
            cds.dwData = data_type;
            cds.cbData = (data.size() + 1) * sizeof(wchar_t);
            cds.lpData = (LPVOID) (data.c_str());

            ::SendMessage(hWnd, WM_COPYDATA, (WPARAM) hWnd, (LPARAM) (LPVOID) &cds);
        }

        std::string get_wm_copydata_data(WPARAM wParam, LPARAM lParam, long& data_type) {
            PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT) lParam;
            data_type = pcds->dwData; // data_type
            return str::to_str(static_cast<wchar_t *>(pcds->lpData));
        }

        void open_mssettings(const std::string name) {
            string url("ms-settings:");
            url += name;

            HINSTANCE hi = ::ShellExecute(
                nullptr,
                L"open",
                str::to_wstr(url).c_str(),
                nullptr,
                nullptr,
                SW_SHOWDEFAULT);
        }

        void open_default_apps(const std::string& app_registered_name, bool user_scoped) {
            string url = "defaultapps";

            if(!app_registered_name.empty()) {
                url += (user_scoped ? "?registeredAppUser=" : "registeredAppMachine=");
                url += app_registered_name;
            }

            open_mssettings(url);
        }

        shell_link read_link(const std::string& path) {
            // see https://renenyffenegger.ch/notes/Windows/development/WinAPI/Shell/read-lnk-file

            ensure_co_initalised();

            shell_link lnk;

            // create shell link interface
            IShellLink* shl;
            HRESULT rc = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
                                            (LPVOID *) &shl);
            if(SUCCEEDED(rc)) {
                // load file into link
                IPersistFile* ipf;
                rc = shl->QueryInterface(IID_IPersistFile, (LPVOID *) &ipf);
                if(SUCCEEDED(rc)) {
                    wstring wpath = str::to_wstr(path);
                    rc = ipf->Load(wpath.c_str(), STGM_READ);
                    if(SUCCEEDED(rc)) {
                        rc = shl->Resolve(0, 0);
                        if(SUCCEEDED(rc)) {
                            int ibuf;
                            const size_t buf_size = 1024;
                            wchar_t buf[buf_size];

                            rc = shl->GetPath(&buf[0], buf_size, 0, SLGP_RAWPATH);
                            if(SUCCEEDED(rc)) {
                                lnk.is_valid = true;
                                lnk.path = str::to_str(buf);
                            }

                            if(SUCCEEDED(shl->GetDescription(buf, buf_size))) {
                                lnk.description = str::to_str(buf);
                            }

                            if(SUCCEEDED(shl->GetArguments(buf, buf_size))) {
                                lnk.args = str::to_str(buf);
                            }

                            if(SUCCEEDED(shl->GetIconLocation(buf, buf_size, &ibuf))) {
                                lnk.icon = str::to_str(buf) + ":" + std::to_string(ibuf);
                            }

                            if(SUCCEEDED(shl->GetWorkingDirectory(&buf[0], buf_size))) {
                                lnk.pwd = str::to_str(buf);
                            }
                        }
                    }
                }
            }


            return lnk;
        }

        unsigned int get_dpi() {
            //vector<HMONITOR> hmons;
            //::EnumDisplayMonitors(NULL, NULL, GetDpiMonitorEnumProc, (LPARAM)&hmons);

            //if (monitor_idx >= hmons.size()) return 0;

            return ::GetDpiForSystem();
        }

        unsigned int get_dpi(HWND hWnd) {
            return ::GetDpiForWindow(hWnd);
        }

        fs::path get_start_menu_path(const string& link_name = "") {
            PWSTR programs_path = nullptr;
            HRESULT hr = ::SHGetKnownFolderPath(FOLDERID_Programs, KF_FLAG_CREATE, nullptr, &programs_path);
            if(FAILED(hr) || programs_path == nullptr) {
                return "";
            }
            std::filesystem::path start_menu_programs(programs_path);
            ::CoTaskMemFree(programs_path);

            if(!link_name.empty())
                return start_menu_programs / (link_name + ".lnk");

            return start_menu_programs;
        }

        void create_start_menu_shortcut(const string& name, const std::string& path) {

            fs::path shortcut_path = get_start_menu_path(name);
            std::filesystem::create_directories(shortcut_path.parent_path());
            fs::path working_directory = fs::path{path}.parent_path();

            IShellLinkW* shell_link = nullptr;
            HRESULT hr = ::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW,
                                    reinterpret_cast<void **>(&shell_link));
            if(SUCCEEDED(hr) && shell_link != nullptr) {
                shell_link->SetPath(str::to_wstr(path).c_str());
                shell_link->SetDescription(str::to_wstr(name).c_str());
                shell_link->SetIconLocation(str::to_wstr(path).c_str(), 0);
                shell_link->SetWorkingDirectory(working_directory.wstring().c_str());

                IPersistFile* persist_file = nullptr;
                hr = shell_link->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&persist_file));
                if(SUCCEEDED(hr) && persist_file != nullptr) {
                    persist_file->Save(shortcut_path.wstring().c_str(), TRUE);
                    persist_file->Release();
                }

                shell_link->Release();
            }
        }

        void remove_start_menu_shortcut(const std::string& name) {
            fs::path shortcut_path = get_start_menu_path(name);
            fs::remove(shortcut_path);
        }

        bool exists_start_menu_shortcut(const std::string& name) {
            fs::path shortcut_path = get_start_menu_path(name);
            return fs::exists(shortcut_path);
        }

        std::wstring get_startup_folder_path() {
            PWSTR path = nullptr;
            std::wstring result;
            if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Startup, 0, nullptr, &path))) {
                result = path;
                CoTaskMemFree(path);
            }
            return result;
        }

        bool create_startup_shortcut(const std::string& name, const std::string& path, const std::string& args) {
            //HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
            //bool comInitializedHere = SUCCEEDED(hr);

            IShellLinkW* shellLink = nullptr;
            HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                          IID_IShellLinkW, (void **) &shellLink);
            if(FAILED(hr)) {
                return false;
            }

            wstring wname = str::to_wstr(name);
            wstring wpath = str::to_wstr(path);
            wstring wargs = str::to_wstr(args);

            shellLink->SetPath(wpath.c_str());
            shellLink->SetArguments(wargs.c_str());

            // Working dir = exe's own directory, not Startup folder
            std::wstring workDir = wpath.substr(0, wpath.find_last_of(L'\\'));
            shellLink->SetWorkingDirectory(workDir.c_str());

            // Optional: icon (defaults to exe's own icon if omitted)
            shellLink->SetIconLocation(wpath.c_str(), 0);

            IPersistFile* persistFile = nullptr;
            hr = shellLink->QueryInterface(IID_IPersistFile, (void **) &persistFile);
            if(SUCCEEDED(hr)) {
                std::wstring shortcutPath = get_startup_folder_path() + L"\\" + wname + L".lnk";
                hr = persistFile->Save(shortcutPath.c_str(), TRUE);
                persistFile->Release();
            }

            shellLink->Release();
            return SUCCEEDED(hr);
        }

        bool remove_startup_shortcut(const std::string& name) {
            auto path = get_startup_folder_path() + L"\\" + str::to_wstr(name) + L".lnk";
            return std::filesystem::remove(path);
        }

        bool exists_startup_shortcut(const std::string& name) {
            auto path = get_startup_folder_path() + L"\\" + str::to_wstr(name) + L".lnk";
            return std::filesystem::exists(path);
        }
    }
}

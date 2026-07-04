#include "os.h"
#include <string>
#include <memory>
#include <array>
#include <algorithm>
#include <iostream>
#include <unistd.h>

#include "str.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#include <ShlObj_core.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include "win32/reg.h"
#pragma comment(lib, "Shcore.lib")
#elif PLATFORM_LINUX
#include <fcntl.h>
#include <sys/wait.h>
#endif

using namespace std;

namespace grey::common::os {
    bool is_app_light_theme() {

#if PLATFORM_WINDOWS
        string s = win32::reg::get_value(win32::reg::hive::current_user,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            "AppsUseLightTheme");

        return s.empty() ? true : s == "1";
#elif PLATFORM_LINUX
        auto run = [](const char* cmd) {
            std::string out;
            if(FILE* p = popen(cmd, "r")) {
                char buf[256];
                while(fgets(buf, sizeof(buf), p)) out += buf;
                pclose(p);
            }
            return out;
        };

        // 1 - prefer dark, 2 - prefer light
        std::string r = run("dbus-send --session --print-reply --dest=org.freedesktop.portal.Desktop "
            "/org/freedesktop/portal/desktop org.freedesktop.portal.Settings.Read "
            "string:'org.freedesktop.appearance' string:'color-scheme' 2>/dev/null");
        if(r.find("uint32 1") != std::string::npos) return false;
        if(r.find("uint32 2") != std::string::npos) return true;

        // "default", "prefer-dark", "prefer-light"
        r = run("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null");
        return r.find("dark") == std::string::npos;
#else
        return false;
#endif
    }

    bool is_system_light_theme() {
#if PLATFORM_WINDOWS
        string s = win32::reg::get_value(win32::reg::hive::current_user,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            "SystemUsesLightTheme");

        return s.empty() ? true : s == "1";
#else
        return is_app_light_theme();
#endif
    }

    unsigned int get_dpi() {
#if PLATFORM_WINDOWS
        return ::GetDpiForSystem();
#else
        return 96;
#endif
    }

    std::string get_system_fonts_path() {
#if PLATFORM_WINDOWS
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, path))) {
            return grey::common::str::to_str(path);
        }
        else {
            return "";
        }
#else
        return "/usr/share/fonts";
#endif
    }


#if PLATFORM_WINDOWS
    std::string get_win32_last_error_text() {
        wchar_t err[256];
        memset(err, 0, 256);
        ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
           nullptr,
           GetLastError(),
           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255,  nullptr);
        return str::to_str(wstring(err));
    }

#else
    struct pclose_deleter {
        void operator()(FILE* f) const { pclose(f); }
    };

    std::string get_system_font_path(const std::string& font_name) {
        std::string command = "fc-match -f \"%{file}\" " + font_name;
        std::array<char, 512> buffer;
        std::string result;
        std::unique_ptr<FILE, pclose_deleter> pipe(popen(command.c_str(), "r"));
        if (!pipe) {
            return "";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        // remove trailing newline if any
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        return result;
    }
#endif

    void set_dpi_awareness() {
#if PLATFORM_WINDOWS
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
    }

    bool get_current_monitor(int &left, int &top, int &right, int &bottom) {

#if PLATFORM_WINDOWS
        POINT pt;
        if(!::GetCursorPos(&pt)) return false;
        HMONITOR hMon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX mi{};
        mi.cbSize = sizeof(mi);
        if (!::GetMonitorInfo(hMon, &mi)) return false;
        left = mi.rcMonitor.left;
        top = mi.rcMonitor.top;
        right = mi.rcMonitor.right;
        bottom = mi.rcMonitor.bottom;
        return true;
#endif

        return false;
    }

   bool shell_open(const std::string& path) {
#if PLATFORM_WINDOWS
    // ShellExecuteW explicitly — avoids ShellExecute macro's ANSI/Wide ambiguity.
    // nullptr for args when empty; some handlers mishandle L"".
    HINSTANCE hi = ::ShellExecuteW(
        nullptr,
        L"open",
        str::to_wstr(path).c_str(),
        nullptr,
        nullptr,
        SW_SHOWDEFAULT);
    // HINSTANCE is a pointer type; cast to INT_PTR before comparing to integer.
    return (INT_PTR)hi > 32;

#elif PLATFORM_LINUX
        pid_t pid = fork();

        if (pid < 0) {
            // Fork failed
            std::cerr << "Failed to fork process." << std::endl;
            return false;
        }

        if (pid == 0) {
            // Inside Child Process

            // Fork a second time to orphan the execution process.
            // This prevents "zombie" processes without making the parent call waitpid().
            pid_t grandchild_pid = fork();

            if (grandchild_pid == 0) {
                // Inside Grandchild Process
                // execlp replaces the current process image with xdg-open
                execlp("xdg-open", "xdg-open", path.c_str(), nullptr);

                // If execlp returns, it means it failed to execute
                std::cerr << "Failed to execute xdg-open" << std::endl;
                _exit(1);
            }

            // Child exits immediately, orphaning the grandchild.
            // init/systemd will automatically reap the grandchild.
            _exit(0);
        }

        // Inside Parent Process
        // Wait for the immediate child to exit (which happens instantly).
        int status;
        waitpid(pid, &status, 0);
        return true;

#else
    (void)path; (void)args;
    return false;
#endif
}
}

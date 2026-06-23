#include "os.h"
#include <string>
#include "str.h"

#if WIN32

#include <Windows.h>
#include <ShlObj_core.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include "win32/reg.h"

#pragma comment(lib, "Shcore.lib")

#endif

using namespace std;

namespace grey::common::os {
    bool is_app_light_theme(bool& value) {

        string s = win32::reg::get_value(win32::reg::hive::current_user,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            "AppsUseLightTheme");

        value = s == "1";

        return !s.empty();
    }

    bool is_system_light_theme(bool& value) {
        string s = win32::reg::get_value(win32::reg::hive::current_user,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        "SystemUsesLightTheme");

        value = s == "1";

        return !s.empty();
    }

    unsigned int get_dpi() {
        return ::GetDpiForSystem();
    }

    std::string get_system_fonts_path() {
#if WIN32
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


#if WIN32
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
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }

    bool get_current_monitor(int &left, int &top, int &right, int &bottom) {

#if WIN32
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
}

#include "os.h"
#include <string>
#include <Windows.h>
#include <ShlObj_core.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include "reg.h"
#include "../str.h"

using namespace std;

#pragma comment(lib, "Shcore.lib")

namespace grey::common::win32::os {
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
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, path))) {
            return grey::common::str::to_str(path);
        }
        else {
            return "";
        }
    }

    void set_dpi_awareness() {
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
}
#include "os.h"
#include <string>
#include "windows.h"
#include "reg.h"

using namespace std;

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
}
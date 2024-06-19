#pragma once

namespace grey::common::win32::os {
    // returns true if app theme is set to Light
    bool is_app_light_theme(bool& value);

    // return true if system theme is set to Light
    bool is_system_light_theme(bool& value);

    unsigned int get_dpi();
}
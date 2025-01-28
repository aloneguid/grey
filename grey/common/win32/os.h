#pragma once
#include <string>

namespace grey::common::win32::os {
    // returns true if app theme is set to Light
    bool is_app_light_theme(bool& value);

    // return true if system theme is set to Light
    bool is_system_light_theme(bool& value);

    unsigned int get_dpi();

    /**
     * @brief Gets location of the system fonts folder, typically c:\\windows\\fonts
     * @return 
     */
    std::string get_system_fonts_path();

    void set_dpi_awareness();
}
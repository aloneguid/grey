#pragma once
#include <string>
#include "platform.h"

namespace grey::common::os {
    // returns true if app theme is set to Light
    bool is_app_light_theme(bool& value);

    // return true if system theme is set to Light
    bool is_system_light_theme(bool& value);

    unsigned int get_dpi();

    /**
     * @brief Gets the location of the system fonts folder, typically c:\\windows\\fonts
     * @return 
     */
    std::string get_system_fonts_path();

#if WIN32
    /**
     * @brief Calls ::GetLastError() and returns the error text.
     * @return
     */
    std::string get_win32_last_error_text();
#else
    /**
     * @brief Gets the path to a specific system font using fontconfig (fc-match).
     * @param font_name Generic name like "sans-serif" or "monospace".
     * @return Path to the font file.
     */
    std::string get_system_font_path(const std::string& font_name);
#endif

    void set_dpi_awareness();

    /**
     * Get current monitor bounds in virtual screen coordinates. The current monitor is the one closes to mouse cursor.
     */
    bool get_current_monitor(int& left, int& top, int& right, int& bottom);
}
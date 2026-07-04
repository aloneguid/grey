#pragma once
#include <string>
#include "platform.h"

namespace grey::common::os {
    /**
     * @brief Determines application theme. Prefer this to make decisions about colours.
     */
    bool is_app_light_theme();

    /**
     * @brief Determines system theme (can different from application theme). Rarely used.
     */
    bool is_system_light_theme();

    unsigned int get_dpi();

    /**
     * @brief Gets the location of the system fonts folder, typically c:\\windows\\fonts
     * @return 
     */
    std::string get_system_fonts_path();

#if PLATFORM_WINDOWS
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
     * Get current monitor bounds in virtual screen coordinates. The current monitor is the one closes to the mouse cursor.
     */
    bool get_current_monitor(int& left, int& top, int& right, int& bottom);

    /**
     * Opens URL or path with the default UI shell.
     * Uses:
     * - Linux: xdg-open path
     * - Windows: ShellExecute::open path
     * @param path Program name, URL or path, whatever the shell can handle.
     * @return
     */
    bool shell_open(const std::string& path);
}
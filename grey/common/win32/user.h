#pragma once
#include <windows.h>
#include <string>

namespace grey::common::win32::user {
    std::wstring load_string(HINSTANCE hInst, UINT id);

    std::wstring load_version(bool full = true);

    // see string_name possible values in Remarks:
    // https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-verqueryvaluea#remarks
    std::wstring get_file_version_info_string(
        const std::string &module_path,
        const std::string &string_name = "FileDescription");

    /// <summary>
    /// Sets window position and size.
    /// To keep current position, pass -1 to x and y.
    /// To keep current size, pass -1 to width and height.
    /// </summary>
    void set_window_pos(HWND hwnd, int x, int y, int width, int height);

    bool is_kbd_ctrl_down();

    bool is_kbd_alt_down();

    bool is_kbd_shift_down();

    bool is_kbd_caps_locks_on();

    /**
     * @brief Determine if the application is running in Light theme.
     * @return True if the application is using Light theme, false otherwise. If theme cannot be determined, returns true to default to Light theme.
     */
    bool is_app_light_theme();

    /**
     * @brief Determines whether the system is currently using a light theme.
     * @return true if the system is using a light theme; otherwise, false. If the theme cannot be determined, returns true to default to light theme.
     */
    bool is_system_light_theme();

    void message_box(const std::string &caption, const std::string &text);
}

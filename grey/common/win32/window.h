#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>

namespace grey::common::win32 {
    /**
     * Windows specific window overlord.
     */
    class window {
    public:
        window(HWND hwnd) : hwnd{ hwnd } { }

        window(const window& w) : hwnd{ w.get_handle() } { }

        bool is_valid();

        [[nodiscard]] HWND get_handle() const { return hwnd; }

        static std::vector<window> enumerate();

        static window get_foreground();

        void set_foreground();

        std::string get_text();

        bool is_visible();

        std::shared_ptr<window> get_owner();

        [[nodiscard]] DWORD get_pid() const;

        std::string get_class_name();

        LONG get_styles();

        /**
         * @brief Sets the window to be topmost (above all windows) or removes the flag.
         * @param topmost 
        */
        void set_topmost(bool topmost);

        // style related functions
        bool is_child();
        bool is_minimized();

        void restore();

        // Minimize this window. Note that some windows like Task Manager (system app) do not respond to this.
        void minimize();

        /// <summary>
        /// Sets window position and size.
        /// To keep current position, pass -1 to x and y.
        /// To keep current size, pass -1 to width and height.
        /// </summary>
        static void set_pos(HWND hwnd, int x, int y, int width, int height);

        void set_rounded_corners(bool smaller_rounding = false);

        /**
         * Sets window opacity, 0 - invisible, 1 - fully opaque.
         */
        bool set_opacity(float opacity) const;

        /**
         * @brief Mark the window as ecluded from video capture. This is useful to display in-app control that should not be recorded by screen recording software.
         * @param exclude set/unset.
         * @return True if operation was successful.
         */
        void excluded_from_capture(bool exclude);

        bool excluded_from_capture();

    private:
        HWND hwnd;
    };
}
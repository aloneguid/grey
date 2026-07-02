#pragma once
#include <Windows.h>
#include <functional>
#include <string>
#include <map>

namespace grey::common::win32 {

    /**
     * @brief Represents a minimal win32 app.
     */
    class app {
    public:
        app(const std::string& class_name = "AppMsgReceiver", const std::string& window_title = "Receiver");
        ~app();

        struct mouse_hook_data {
            UINT_PTR msg;
            POINT pt;
            short wheel_delta{0};
        };

        HWND get_hwnd() { return hwnd; }

        // https://learn.microsoft.com/en-gb/windows/win32/api/winuser/nf-winuser-addclipboardformatlistener?redirectedfrom=MSDN
        
        
        /**
         * @brief WM_CLIPBOARDUPDATE is sent when the content of the clipboard changes.
         */
        void add_clipboard_listener();

        // runs message loop until WM_QUIT
        void run() const;

        // called when an unhandled message arrives to this class' fake window
        std::function<LRESULT(UINT, WPARAM, LPARAM)> on_app_window_message;

        // called from message loop (run()) when any message arrives
        std::function<void(MSG&)> on_message_loop_message;

        std::function<void(const std::string&)> on_global_hotkey_pressed;

        void set_message_timeout(size_t milliseconds = -1);

        void set_max_fps_mode(bool v) { max_fps_mode = v; }

        bool install_low_level_keyboard_hook(std::function<bool(UINT_PTR, KBDLLHOOKSTRUCT&)> fn);
        bool install_low_level_mouse_hook(std::function<bool(mouse_hook_data)> fn);
        void uninstall_low_level_keyboard_hook();
        void uninstall_low_level_mouse_hook();

        /**
         * @brief Same as above, but takes hotkey in human readable format, e.g. "ctrl+shift+A"
         * @param hotkey_id Unique ID for this hotkey. You can use it to identify which hotkey was pressed.
         * @param hotkey Human readable hotkey string. Supported format is combination of modifiers (ctrl, alt, shift) and a key, separated by "+", e.g. "ctrl+shift+A". Supported keys are letters, numbers and F1-F12. Modifiers can be in any order, but key must be at the end. Example valid hotkeys: "ctrl+shift+A", "alt+F4", "shift+1". Example invalid hotkeys: "A+ctrl", "ctrl+alt", "ctrl+shift".
         * @return 
         */
        bool register_global_hotkey(const std::string& hotkey, std::string& error_msg);

        void unregister_global_hotkey(const std::string& hotkey);

        void unregister_all_global_hotkeys();

    private:
        WNDCLASSEX wc;
        HWND hwnd{ nullptr };
        UINT_PTR timeout_timer_id{0};
        bool max_fps_mode{false};
        HHOOK hLLKbdHook{nullptr};
        HHOOK hLLMouseHook{nullptr};
        //std::vector<int> registered_hotkeys;  // to keep track of registered hotkeys, so we can unregister them on exit
        std::vector<std::pair<int, std::string>> hotkey_id_to_name; // to keep track of the registered hotkeys

        static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

        std::function<bool(UINT_PTR, KBDLLHOOKSTRUCT&)> on_low_level_keyboard_hook_func;
        std::function<bool(mouse_hook_data)> on_low_level_mouse_hook_func;
    };
}
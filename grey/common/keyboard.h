#pragma once

#include "platform.h"

#include <type_traits>
#include <vector>

#if PLATFORM_WINDOWS
    #include <windows.h>
#elif PLATFORM_LINUX
    #include <linux/input-event-codes.h>
#elif PLATFORM_MACOS
    #include <Carbon/Carbon.h>
#endif

namespace grey::common {

    enum class key {
#if PLATFORM_WINDOWS
        backspace = VK_BACK,
        tab = VK_TAB,
        enter = VK_RETURN,
        pause = VK_PAUSE,
        caps_lock = VK_CAPITAL,
        escape = VK_ESCAPE,
        space = VK_SPACE,
        page_up = VK_PRIOR,
        page_down = VK_NEXT,
        end = VK_END,
        home = VK_HOME,
        left = VK_LEFT,
        up = VK_UP,
        right = VK_RIGHT,
        down = VK_DOWN,
        print_screen = VK_SNAPSHOT,
        insert = VK_INSERT,
        del = VK_DELETE,

        num_0 = '0',
        num_1 = '1',
        num_2 = '2',
        num_3 = '3',
        num_4 = '4',
        num_5 = '5',
        num_6 = '6',
        num_7 = '7',
        num_8 = '8',
        num_9 = '9',

        a = 'A',
        b = 'B',
        c = 'C',
        d = 'D',
        e = 'E',
        f = 'F',
        g = 'G',
        h = 'H',
        i = 'I',
        j = 'J',
        k = 'K',
        l = 'L',
        m = 'M',
        n = 'N',
        o = 'O',
        p = 'P',
        q = 'Q',
        r = 'R',
        s = 'S',
        t = 'T',
        u = 'U',
        v = 'V',
        w = 'W',
        x = 'X',
        y = 'Y',
        z = 'Z',

        f1 = VK_F1,
        f2 = VK_F2,
        f3 = VK_F3,
        f4 = VK_F4,
        f5 = VK_F5,
        f6 = VK_F6,
        f7 = VK_F7,
        f8 = VK_F8,
        f9 = VK_F9,
        f10 = VK_F10,
        f11 = VK_F11,
        f12 = VK_F12,

        num_lock = VK_NUMLOCK,
        scroll_lock = VK_SCROLL,

        left_shift = VK_LSHIFT,
        right_shift = VK_RSHIFT,
        left_ctrl = VK_LCONTROL,
        right_ctrl = VK_RCONTROL,
        left_alt = VK_LMENU,
        right_alt = VK_RMENU,
#elif PLATFORM_LINUX
        backspace = KEY_BACKSPACE,
        tab = KEY_TAB,
        enter = KEY_ENTER,
        pause = KEY_PAUSE,
        caps_lock = KEY_CAPSLOCK,
        escape = KEY_ESC,
        space = KEY_SPACE,
        page_up = KEY_PAGEUP,
        page_down = KEY_PAGEDOWN,
        end = KEY_END,
        home = KEY_HOME,
        left = KEY_LEFT,
        up = KEY_UP,
        right = KEY_RIGHT,
        down = KEY_DOWN,
        print_screen = KEY_SYSRQ,
        insert = KEY_INSERT,
        del = KEY_DELETE,

        num_0 = KEY_0,
        num_1 = KEY_1,
        num_2 = KEY_2,
        num_3 = KEY_3,
        num_4 = KEY_4,
        num_5 = KEY_5,
        num_6 = KEY_6,
        num_7 = KEY_7,
        num_8 = KEY_8,
        num_9 = KEY_9,

        a = KEY_A,
        b = KEY_B,
        c = KEY_C,
        d = KEY_D,
        e = KEY_E,
        f = KEY_F,
        g = KEY_G,
        h = KEY_H,
        i = KEY_I,
        j = KEY_J,
        k = KEY_K,
        l = KEY_L,
        m = KEY_M,
        n = KEY_N,
        o = KEY_O,
        p = KEY_P,
        q = KEY_Q,
        r = KEY_R,
        s = KEY_S,
        t = KEY_T,
        u = KEY_U,
        v = KEY_V,
        w = KEY_W,
        x = KEY_X,
        y = KEY_Y,
        z = KEY_Z,

        f1 = KEY_F1,
        f2 = KEY_F2,
        f3 = KEY_F3,
        f4 = KEY_F4,
        f5 = KEY_F5,
        f6 = KEY_F6,
        f7 = KEY_F7,
        f8 = KEY_F8,
        f9 = KEY_F9,
        f10 = KEY_F10,
        f11 = KEY_F11,
        f12 = KEY_F12,

        num_lock = KEY_NUMLOCK,
        scroll_lock = KEY_SCROLLLOCK,

        left_shift = KEY_LEFTSHIFT,
        right_shift = KEY_RIGHTSHIFT,
        left_ctrl = KEY_LEFTCTRL,
        right_ctrl = KEY_RIGHTCTRL,
        left_alt = KEY_LEFTALT,
        right_alt = KEY_RIGHTALT,
#elif PLATFORM_MACOS
        backspace = kVK_Delete,
        tab = kVK_Tab,
        enter = kVK_Return,
        pause = kVK_F16,
        caps_lock = kVK_CapsLock,
        escape = kVK_Escape,
        space = kVK_Space,
        page_up = kVK_PageUp,
        page_down = kVK_PageDown,
        end = kVK_End,
        home = kVK_Home,
        left = kVK_LeftArrow,
        up = kVK_UpArrow,
        right = kVK_RightArrow,
        down = kVK_DownArrow,
        print_screen = kVK_F13,
        insert = kVK_Help,
        del = kVK_ForwardDelete,

        num_0 = kVK_ANSI_0,
        num_1 = kVK_ANSI_1,
        num_2 = kVK_ANSI_2,
        num_3 = kVK_ANSI_3,
        num_4 = kVK_ANSI_4,
        num_5 = kVK_ANSI_5,
        num_6 = kVK_ANSI_6,
        num_7 = kVK_ANSI_7,
        num_8 = kVK_ANSI_8,
        num_9 = kVK_ANSI_9,

        a = kVK_ANSI_A,
        b = kVK_ANSI_B,
        c = kVK_ANSI_C,
        d = kVK_ANSI_D,
        e = kVK_ANSI_E,
        f = kVK_ANSI_F,
        g = kVK_ANSI_G,
        h = kVK_ANSI_H,
        i = kVK_ANSI_I,
        j = kVK_ANSI_J,
        k = kVK_ANSI_K,
        l = kVK_ANSI_L,
        m = kVK_ANSI_M,
        n = kVK_ANSI_N,
        o = kVK_ANSI_O,
        p = kVK_ANSI_P,
        q = kVK_ANSI_Q,
        r = kVK_ANSI_R,
        s = kVK_ANSI_S,
        t = kVK_ANSI_T,
        u = kVK_ANSI_U,
        v = kVK_ANSI_V,
        w = kVK_ANSI_W,
        x = kVK_ANSI_X,
        y = kVK_ANSI_Y,
        z = kVK_ANSI_Z,

        f1 = kVK_F1,
        f2 = kVK_F2,
        f3 = kVK_F3,
        f4 = kVK_F4,
        f5 = kVK_F5,
        f6 = kVK_F6,
        f7 = kVK_F7,
        f8 = kVK_F8,
        f9 = kVK_F9,
        f10 = kVK_F10,
        f11 = kVK_F11,
        f12 = kVK_F12,

        num_lock = kVK_ANSI_KeypadClear,
        scroll_lock = kVK_F14,

        left_shift = kVK_Shift,
        right_shift = kVK_RightShift,
        left_ctrl = kVK_Control,
        right_ctrl = kVK_RightControl,
        left_alt = kVK_Option,
        right_alt = kVK_RightOption,
#endif
    };

    class keyboard {
    public:
        /**
         * @brief Refresh the cached keyboard state from the OS
         *
         * Called automatically by is_key_down/are_keys_down when their `rescan` parameter is true.
         * Call this directly if you want to batch multiple is_key_down/are_keys_down calls
         * (with rescan = false) against a single, consistent snapshot.
         *
         * Note that on Linux this does not work by default due to the lack of permissions to read input events globally.
         * The easiest way to enable this is to add yourself to "input" group: `sudo usermod -aG input $USER`.
         *
         */
        static void refresh_state();

        /**
         * Forces internal keyboard buffer to clear. This is useful for instance when installing a Windows low-level hook and keyboard functions are not working as expected.
         */
        static void clear_state();

        /**
         * @brief Check whether a key is currently pressed
         *
         * @param k Key to check
         * @param rescan If true (default), the cached keyboard state is refreshed first; pass false to reuse the last refreshed state
         */
        static bool is_key_down(key k, bool rescan = true);

        /**
         * @brief Check whether a key is currently toggled. Makes sense for keys like Caps Locks, Num Lock, Scroll Lock.
         *
         * @param k Key to check
         * @param rescan If true (default), the cached keyboard state is refreshed first; pass false to reuse the last refreshed state
         */
        static bool is_key_toggled(key k, bool rescan = true);

        /**
         * @brief Check whether all of the given keys are currently pressed
         *
         * @param keys Keys to check
         * @param rescan If true (default), the cached keyboard state is refreshed once before checking all keys; pass false to reuse the last refreshed state
         */
        static bool are_keys_down(const std::vector<key>& keys, bool rescan = true);

        /**
         * @brief Check whether all of the given keys are currently pressed
         *
         * Convenience overload of are_keys_down that accepts a variable number of `key` arguments
         * instead of a vector.
         *
         * @param keys Keys to check
         */
        template<typename... Keys>
        static bool are_keys_down(Keys... keys) {
            static_assert((std::is_same_v<Keys, key> && ...), "are_keys_down expects key arguments");
            return are_keys_down(std::vector<key>{keys...}, false);
        }

        /**
         * @brief Check whether any of the given keys is currently pressed
         *
         * @param keys Keys to check
         * @param rescan If true (default), the cached keyboard state is refreshed once before checking all keys; pass false to reuse the last refreshed state
         */
        static bool is_any_key_down(const std::vector<key>& keys, bool rescan = true);

        /**
         * @brief Check whether any of the given keys is currently pressed
         *
         * Convenience overload of is_any_key_down that accepts a variable number of `key` arguments
         * instead of a vector.
         *
         * @param keys Keys to check
         */
        template<typename... Keys>
        static bool is_any_key_down(Keys... keys) {
            static_assert((std::is_same_v<Keys, key> && ...), "is_any_key_down expects key arguments");
            return is_any_key_down(std::vector<key>{keys...}, false);
        }

    private:
#if PLATFORM_WINDOWS
        static inline BYTE state_[256]{};
#elif PLATFORM_LINUX
        static constexpr size_t bits_per_long = 8 * sizeof(unsigned long);
        static inline unsigned long state_[(KEY_MAX / bits_per_long) + 1]{};
        static inline unsigned long led_state_[(LED_MAX / bits_per_long) + 1]{};
#elif PLATFORM_MACOS
        static constexpr size_t max_key_code = 127;
        static inline bool state_[max_key_code + 1]{};
        static inline bool caps_lock_state_{};
#endif
    };
}

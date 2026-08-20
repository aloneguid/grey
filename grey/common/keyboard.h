#pragma once

#include "platform.h"

#include <type_traits>
#include <vector>

#if PLATFORM_WINDOWS
    #include <windows.h>
#elif PLATFORM_LINUX
    #include <linux/input-event-codes.h>
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
         * @brief Check whether a key is currently pressed
         *
         * @param k Key to check
         * @param rescan If true (default), the cached keyboard state is refreshed first; pass false to reuse the last refreshed state
         */
        static bool is_key_down(key k, bool rescan = true);

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
#endif
    };
}

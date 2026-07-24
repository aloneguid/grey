#pragma once

namespace grey::common {
    class keyboard {
    public:
        static bool global_input_read_supported();

        static bool is_kbd_ctrl_down();

        static bool is_kbd_alt_down();

        static bool is_kbd_shift_down();

        static bool is_kbd_caps_locks_on();

    };
}

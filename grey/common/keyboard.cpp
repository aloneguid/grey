#include "keyboard.h"
#include "platform.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace grey::common {
#if PLATFORM_WINDOWS
    bool keyboard::global_input_read_supported() {
        return true;
    }

    bool keyboard::is_kbd_ctrl_down() {
        // https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

        return ::GetKeyState(VK_CONTROL) & 0x8000;
    }

    bool keyboard::is_kbd_alt_down() {
        return (::GetKeyState(VK_LMENU) & 0x8000) || (::GetKeyState(VK_RMENU) & 0x8000);
    }

    bool keyboard::is_kbd_shift_down() {
        return ::GetKeyState(VK_SHIFT) & 0x8000;
    }

    bool keyboard::is_kbd_caps_locks_on() {
        return (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    }
#endif

#if PLATFORM_LINUX
    bool keyboard::global_input_read_supported() {
        return false;
    }

    bool keyboard::is_kbd_ctrl_down() {
        return false;
    }

    bool keyboard::is_kbd_alt_down() {
        return false;
    }

    bool keyboard::is_kbd_shift_down() {
        return false;
    }

    bool keyboard::is_kbd_caps_locks_on() {
        return false;
    }

    bool keyboard::is_kbd_caps_locks_on() {
        return false;
    }
#endif
}

#include "mouse.h"

#include "platform.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#elif PLATFORM_LINUX
#include <GLFW/glfw3.h>
#endif

namespace grey::common {
#if PLATFORM_WINDOWS
    bool mouse::get_pos(long& x, long& y) {
        POINT position{};
        if (!GetCursorPos(&position)) {
            return false;
        }

        x = position.x;
        y = position.y;
        return true;
    }

    bool mouse::set_pos(long x, long y) {
        return SetCursorPos(static_cast<int>(x), static_cast<int>(y)) != 0;
    }
#elif PLATFORM_LINUX
    bool mouse::get_pos(long& x, long& y) {
        GLFWwindow* window = glfwGetCurrentContext();
        if (window == nullptr) {
            return false;
        }

        double cursor_x{};
        double cursor_y{};
        glfwGetCursorPos(window, &cursor_x, &cursor_y);
        x = static_cast<long>(cursor_x);
        y = static_cast<long>(cursor_y);
        return true;
    }

    bool mouse::set_pos(long x, long y) {
        GLFWwindow* window = glfwGetCurrentContext();
        if (window == nullptr) {
            return false;
        }

        glfwSetCursorPos(window, static_cast<double>(x), static_cast<double>(y));
        return true;
    }
#else
    bool mouse::get_pos(long&, long&) {
        return false;
    }

    bool mouse::set_pos(long, long) {
        return false;
    }
#endif
}

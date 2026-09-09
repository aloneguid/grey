#pragma once
#include "platform.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#elif PLATFORM_LINUX
#include <GLFW/glfw3.h>
#endif

namespace grey::common {
    /**
     * OS-independent UI window class.
     * On non-Windows platforms it uses GLFW.
     */
    class ui_window {
    public:
#if PLATFORM_WINDOWS
        ui_window(HWND hwnd) : h{ hwnd } { }
        ui_window(void* hwnd) : h{(HWND)hwnd} {}
#else
        ui_window(GLFWwindow* glw) : h{glw} {}
        ui_window(void* glw) : h{(GLFWwindow*)glw} {}
#endif

        /**
         * Sets window opacity, 0 - invisible, 1 - fully opaque.
         */
        void opacity(float opacity) const;

        /**
         * Sets window as "always on top"
         */
        void always_on_op() const;

        operator bool() const { return h != nullptr; }


    private:
#if PLATFORM_WINDOWS
        HWND h;
#else
        GLFWwindow* h;
#endif
    };
}

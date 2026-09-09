#include "ui_window.h"

#include <algorithm>

namespace grey::common {
    void ui_window::opacity(float opacity) const {

#if PLATFORM_WINDOWS
        if(opacity < 0.0f || opacity > 1.0f) return false;
        const BYTE alpha = static_cast<BYTE>(opacity * 255);
        LONG_PTR exStyle = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        ::SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
        return ::SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
#else
        float normalized_alpha = std::clamp(opacity, 0.0f, 1.0f);
        glfwSetWindowOpacity(h, normalized_alpha);
#endif
    }

    void ui_window::always_on_op() const {
#if PLATFORM_WINDOWS
        LONG_PTR exStyle = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        ::SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TOPMOST);
#else
        glfwSetWindowAttrib(h, GLFW_FLOATING, GLFW_TRUE);
#endif

    }
}

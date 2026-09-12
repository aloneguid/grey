#include "ui_window.h"
#include <algorithm>

#if PLATFORM_WINDOWS
#include <dwmapi.h>
#endif

namespace grey::common {
    void ui_window::opacity(float opacity) const {

#if PLATFORM_WINDOWS
        if(opacity < 0.0f || opacity > 1.0f) return;
        const BYTE alpha = static_cast<BYTE>(opacity * 255);
        LONG_PTR exStyle = ::GetWindowLongPtr(h, GWL_EXSTYLE);
        ::SetWindowLongPtr(h, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
        ::SetLayeredWindowAttributes(h, 0, alpha, LWA_ALPHA);
#else
        float normalized_alpha = std::clamp(opacity, 0.0f, 1.0f);
        glfwSetWindowOpacity(h, normalized_alpha);
#endif
    }

    void ui_window::always_on_op() const {
#if PLATFORM_WINDOWS
        LONG_PTR exStyle = ::GetWindowLongPtr(h, GWL_EXSTYLE);
        ::SetWindowLongPtr(h, GWL_EXSTYLE, exStyle | WS_EX_TOPMOST);
#else
        glfwSetWindowAttrib(h, GLFW_FLOATING, GLFW_TRUE);
#endif

    }

    void ui_window::apply_native_decorations() const {
#if PLATFORM_WINDOWS
        constexpr bool smaller_rounding = false;
        constexpr int preference = smaller_rounding ? DWMWCP_ROUNDSMALL : DWMWCP_ROUND;
        ::DwmSetWindowAttribute(
            h,
            DWMWA_WINDOW_CORNER_PREFERENCE,
            &preference,
            sizeof(preference));
#endif
    }

    void ui_window::allow_screen_capture(bool allow) const {
#if PLATFORM_WINDOWS
        ::SetWindowDisplayAffinity(h, allow ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
#endif
    }
}

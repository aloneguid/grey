#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "../widgets.h"

namespace grey::widgets::x {

    enum class toast_phase {
        fade_in,
        wait,
        fade_out,
        expired
    };

    /**
     * ImGuiNotify.hpp rewritten in modern C++ and using Grey infrastructure for rendering. Faster and safer than the original. Features:
     * - [ ] Fade in/out animation
     * - [ ] Dismiss button
     */
    class toast {
    public:
        toast(emphasis emp, std::string title, std::string message, int dismiss_time_ms);

        toast& operator=(const toast& other);

        emphasis emp;
        std::string title;
        std::string message;
        int dismiss_time_ms;

        [[nodiscard]] toast_phase get_phase() const;

        static void render_frame();
        static void push(emphasis emp, const std::string& title, const std::string& message, int dismiss_time_ms);



    private:
        static constexpr int64_t FadeInOutMs = 200;
        static constexpr float Opacity = 0.8f;  // final toast opacity
        static constexpr float WindowPadding = 10.0f;

        std::chrono::system_clock::time_point creation_time;
        window w;

        [[nodiscard]] std::chrono::nanoseconds get_elapsed_time() const;
        [[nodiscard]] int64_t get_elapsed_time_ms() const;
        [[nodiscard]] float get_fade_alpha() const;
        [[nodiscard]] static std::string get_icon(emphasis emp);
        [[nodiscard]] static std::string get_title(emphasis emp);

        static std::vector<toast> toasts;
    };
}

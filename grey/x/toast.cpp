#include "toast.h"
#include <utility>
#include <format>
#include "../widgets.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "../fonts/MaterialIcons.h"

using namespace std;

namespace grey::widgets::x {
    std::chrono::nanoseconds toast::get_elapsed_time() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now() - creation_time);
    }

    int64_t toast::get_elapsed_time_ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(get_elapsed_time()).count();
    }

    float toast::get_fade_alpha() const {
        const toast_phase phase = get_phase();
        const int64_t elapsed = get_elapsed_time_ms();

        if(phase == toast_phase::fade_in) {
            return (static_cast<float>(elapsed) / static_cast<float>(FadeInMs)) * Opacity;
        }

        if(phase == toast_phase::fade_out) {
            return (1.f - (static_cast<float>(elapsed) - static_cast<float>(FadeInMs) - static_cast<float>(
                               dismiss_time_ms)) / static_cast<float>(FadeOutMs)) * Opacity;
        }

        return Opacity;
    }

    std::string toast::get_icon(const emphasis emp) {
        switch(emp) {
            case emphasis::success:
                return ICON_MD_LIGHTBULB;
            case emphasis::info:
                return ICON_MD_INFO;
            case emphasis::warning:
                return ICON_MD_WARNING;
            case emphasis::error:
                return ICON_MD_REPORT;
            case emphasis::primary:
                return ICON_MD_FEEDBACK;
            default:
                return "";
        }
    }

    std::string toast::get_title(const emphasis emp) {
        if(emp == emphasis::none) return "";
        return string{magic_enum::enum_name(emp)};
    }

    std::vector<toast> toast::toasts;

    toast::toast(const emphasis emp, std::string title, std::string message, const int dismiss_time_ms) :
        emp(emp),
        title(std::move(title)),
        message(std::move(message)),
        dismiss_time_ms(dismiss_time_ms),
        creation_time{chrono::system_clock::now()},
        w_title{format("##toast{}", generate_int_id())} {

    }

    toast& toast::operator=(const toast& other) {
        if(this != &other) {
            emp = other.emp;
            title = other.title;
            message = other.message;
            dismiss_time_ms = other.dismiss_time_ms;
            creation_time = other.creation_time;
        }

        return *this;
    }

    toast_phase toast::get_phase() const {
        const int64_t elapsed_time_ms = get_elapsed_time_ms();

        if(elapsed_time_ms > FadeInMs + dismiss_time_ms + FadeOutMs)
            return toast_phase::expired;

        if(elapsed_time_ms > FadeInMs + dismiss_time_ms)
            return toast_phase::fade_out;

        if(elapsed_time_ms > FadeInMs) {
            return toast_phase::wait;
        }

        return toast_phase::fade_in;
    }

    void toast::render_frame() {
        // speedy skip, especially relevant because this is called FPS times a second.
        if(toasts.empty()) return;

        // draw toast on the monitor rather than relative to current window
        const monitor mm = mon(0).value();

        float height_shift = 0.f;

        // erase all notifications that are expired, because we need to delete the collection first
        std::erase_if(toasts, [](const toast& t) { return t.get_phase() == toast_phase::expired; });

        // there will be no expired notifications left in the collection
        for(auto& toast: toasts) {

            // Set notification window position to bottom right corner of the main window, considering the main window size and location in relation to the display
            // const point toast_pos = mon_pos + mon_size - sz{window_padding, window_padding + height_shift};
            point toast_pos = mm.work_area.rb();
            toast_pos.y -= height_shift;
            toast_pos.x -= WindowPadding * mm.dpi_scale;

            const float fade_alpha = toast.get_fade_alpha();

            wnd ww{toast.w_title,{
                .opacity = fade_alpha,
                .show_title_bar = false,
                .always_on_top = true,
                .pos = toast_pos,
                .pos_pivot = point_pivot::bottom_right,
                .pos_cond = act_condition::always}};

            bool has_title{false};

            // title

            if(string icon = get_icon(toast.emp); !icon.empty()) {
                lbl(icon, {.emp = toast.emp});
                has_title = true;
            }

            string title_text = toast.title;
            if(title_text.empty()) title_text = get_title(toast.emp);
            if(!title_text.empty()) {
                if(has_title) sl();
                lbl(title_text, {.emp = toast.emp});
                has_title = true;
            }

            if(has_title) sep();

            // content
            lbl(toast.message);

            // save height for next toasts
            // decrease height relative to fade alpha i.e. decrease height by fade alpha animated value
            float anim_height = fade_alpha / Opacity;
            height_shift += ww.height() * anim_height + scaled(WindowPadding);
        }
    }

    void toast::push(emphasis emp, const std::string& title, const std::string& message, int dismiss_time_ms) {
        toasts.emplace_back(emp, title, message, dismiss_time_ms);
    }
}

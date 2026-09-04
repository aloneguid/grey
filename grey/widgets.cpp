#include "widgets.h"
#include "themes.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "3rdparty/imspinner.h"
#include "x/md.h"
#include "x/toast.h"
#include "fonts/font_loader.h"
#include <iostream>
#include <utility>

// for Windows-specific hacks
#if PLATFORM_WINDOWS
#include <Windows.h>
#include "common/win32/window.h"
#endif

using namespace std;
using namespace grey;

namespace grey::widgets {
    // ---- general ----

    float scale = 1.0f;

    static int incrementing_id;

    // Windows Draw List, re-assigned on window initialisation on every frame redraw
    static ImDrawList* wdl{nullptr};

    int generate_int_id() {
        return incrementing_id++;
    }

    bool initialized() {
        return nullptr != ImGui::GetCurrentContext();
    }

    static string generate_id(const string& prefix = "") {
        return prefix + std::to_string(incrementing_id++);
    }

    ImGuiHoveredFlags to_hovered_flags(show_delay delay) {
        ImGuiHoveredFlags flags = ImGuiHoveredFlags_None;
        switch(delay) {
            case show_delay::immediate:
                flags = ImGuiHoveredFlags_DelayNone;
                break;
            case show_delay::quick:
                flags = ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay;
                break;
            case show_delay::normal:
                flags = ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay;
                break;
            case show_delay::slow:
                flags = ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_NoSharedDelay;
                break;
            default:
                break;
        }
        return flags;
    }

    // ---- guard ----

    texter::texter(float size_delta, font_weight weight) {
        ImFont* font{nullptr};
        float font_size{0.0f};

        if(make_font(size_delta, weight, &font, font_size)) {
            ImGui::PushFont(font, font_size);
            font_pushed = true;
        } else {
            font_pushed = false;
        }
    }

    texter::~texter() {
        if(font_pushed) {
            ImGui::PopFont();
        }
    }

    bool texter::make_font(float size_delta, font_weight weight, ImFont** out_font, float& out_font_size) {
        out_font_size = ImGui::GetStyle().FontSizeBase;
        if(size_delta > 0.01 || size_delta < -0.01 || weight != font_weight::regular) {
            float new_size = out_font_size + size_delta;
            ImFont* font = fonts::font_loader::get_font(weight);
            if(new_size > 0) {
                *out_font = font;
                out_font_size = new_size;
                return true;
            }
        }
        *out_font = nullptr;
        out_font_size = 0.0f;
        return false;
    }

    clip_rect::clip_rect(const ImVec2& min, const ImVec2& max) {
        wdl->PushClipRect(min, max, false);
    }

    clip_rect::~clip_rect() {
        wdl->PopClipRect();
    }

    id_frame::id_frame(int scope_id) {
        ImGui::PushID(scope_id);
    }

    id_frame::id_frame(const std::string& scope_id) {
        ImGui::PushID(scope_id.c_str());
    }

    id_frame::~id_frame() {
        ImGui::PopID();
    }


    // ---- window ----

    window::window(std::string title, bool* p_open) : title{std::move(title)} {
        this->p_open = p_open;
        wc.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    }

    window& window::size(int width, int height) {
        init_size = sz(width * scale, height * scale);
        return *this;
    }

    window& window::resize(float width, float height) {
        resize_to = sz(width * scale, height * scale);
        return *this;
    }

    window& window::has_menubar() {
        flags |= ImGuiWindowFlags_MenuBar;
        return *this;
    }

    window& window::no_resize() {
        flags |= ImGuiWindowFlags_NoResize;
        return *this;
    }

    window& window::auto_resize() {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;
        return *this;
    }

    window& window::no_collapse() {
        flags |= ImGuiWindowFlags_NoCollapse;
        return *this;
    }

    window& window::no_titlebar() {
        flags |= ImGuiWindowFlags_NoTitleBar;
        return *this;
    }

    window& window::no_background() {
        flags |= ImGuiWindowFlags_NoBackground;
        return *this;
    }

    window& window::border(const float width) {
        border_size = width;
        return *this;
    }

    window& window::no_scroll() {
        flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        return *this;
    }


    window& window::center(void* monitor_handle) {
        init_center_monitor = monitor_handle;
        init_center = true;

        return *this;
    }

    window& window::fill_viewport() {
        fill_viewport_enabled = true;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        return *this;
    }

    void window::enter() {
        //ImGui::SetNextWindowBgAlpha(1.0f);

        if(border_size >= 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, border_size);
        }

        // set window class to prevent viewports to be merged with main window
        //ImGui::SetNextWindowClass(&wc);

        if(init_size.width > 0)
            ImGui::SetNextWindowSize(init_size, ImGuiCond_Once);

        if(resize_to.width > 0) {
            ImGui::SetNextWindowSize(resize_to);
            resize_to = ImVec2{0, 0};
        }

        if(init_center && !init_center_pos.x) {
            ImVector<ImGuiPlatformMonitor> monitors = ImGui::GetPlatformIO().Monitors;
            size_t midx = 0;
            for(size_t i = 0; i < monitors.Size; i++) {
                if(monitors[i].PlatformHandle == init_center_monitor) {
                    midx = i;
                    break;
                }
            }

            init_center_imgui_monitor = monitors[midx];

            init_center_pos = ImVec2(
                init_center_imgui_monitor.WorkSize.x / 2 - init_size.width / 2 + init_center_imgui_monitor.WorkPos.x,
                init_center_imgui_monitor.WorkSize.y / 2 - init_size.height / 2 + init_center_imgui_monitor.WorkPos.y);
        }

        if(init_center && init_center_pos.x) {
            ImGui::SetNextWindowPos(init_center_pos, ImGuiCond_Appearing);
        }

        if(fill_viewport_enabled) {
#ifdef IMGUI_HAS_VIEWPORT
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
#else
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
            // window rounding will be handled by parent viewport, therefore we need to disable it
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        }

        ImGui::Begin(title.c_str(), p_open, flags);
        wdl = ImGui::GetWindowDrawList();
    }

    void window::leave() {
#if PLATFORM_WINDOWS
        const ImGuiViewport* vp = ImGui::GetWindowViewport();
        if(vp && vp->PlatformWindowCreated && vp->PlatformHandleRaw) {
            auto h_wnd = static_cast<HWND>(vp->PlatformHandleRaw);
            common::win32::window wnd{h_wnd};

            if(win32_x_style_applied_to_handle != vp->PlatformHandleRaw) {
                wnd.set_rounded_corners(false);
                win32_x_style_applied_to_handle = vp->PlatformHandleRaw;
            }

            if(opacity != last_opacity) {
                wnd.set_opacity(opacity);
                last_opacity = opacity;
            }

            if(!win32_brought_forward) {
                wnd.set_foreground();
                win32_brought_forward = true;
            }
            if(win32_exclude_from_capture_current != win32_exclude_from_capture) {
                ::SetWindowDisplayAffinity(h_wnd, win32_exclude_from_capture ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
                win32_exclude_from_capture_current = win32_exclude_from_capture;
            }
            if(win32_always_on_top_current != win32_always_on_top) {
                ::SetWindowPos(h_wnd, win32_always_on_top ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                               SWP_NOMOVE | SWP_NOSIZE);
                win32_always_on_top_current = win32_always_on_top;
            }
        }
#endif

        if(border_size >= 0)
            ImGui::PopStyleVar();

        wdl = nullptr;
        ImGui::End();

        if(fill_viewport_enabled) {
            ImGui::PopStyleVar();
        }
    }

    window& window::fullscreen() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(viewport->Size, ImGuiCond_Once);

        return *this;
    }

    window::~window() {
    }

    // ---- container ----

    container::container(float width, float height) : id{generate_id()}, size{width, height} {
    }

    container::container(const std::string& id, float width, float height) : id{id}, size{width, height} {
    }

    void container::enter() {
        if(size.y < 0) {
            ImVec2 tsz = size;
            // pad from the bottom
            ImVec2 wsz = ImGui::GetWindowSize();
            tsz = ImVec2(tsz.x, wsz.y + size.y);
        }

        if(pad.x > 0 || pad.y > 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, pad);
        }
        ImGui::BeginChild(id.c_str(), size, flags, window_flags);
    }

    void container::leave() {
        ImGui::EndChild();
        if(pad.x > 0 || pad.y > 0) {
            ImGui::PopStyleVar();
        }
    }

    bool mi(const std::string& text, bool reserve_icon_space, const std::string& icon) {
        bool r;
        const string prefix = "       ";

        if(reserve_icon_space) {
            const ImVec2 cp = ImGui::GetCursorPos();
            r = ImGui::MenuItem((prefix + text).c_str());
            if(!icon.empty()) {
                ImGui::SetCursorPos(cp);
                lbl(icon);
            }
        } else {
            r = ImGui::MenuItem(text.c_str());
        }

        return r;
    }

    void mi_themes(const std::function<void(const std::string&)>& on_changed) {
        if(menu m{"Theme", true, ICON_MD_BRUSH}) {
            ImDrawList* dl = ImGui::GetWindowDrawList();

            for(auto& theme: themes::list_themes()) {
                const float sz = ImGui::GetTextLineHeight();
                ImVec2 p = ImGui::GetCursorScreenPos();

                // draw a triangle with accent colour in top left corner
                dl->AddTriangleFilled(p, ImVec2(p.x + sz, p.y), ImVec2(p.x, p.y + sz), theme.accent);

                // draw a triangle with base colour in bottom right corner
                dl->AddTriangleFilled(ImVec2(p.x + sz, p.y + sz), ImVec2(p.x + sz, p.y), ImVec2(p.x, p.y + sz),
                                      theme.is_dark ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255));

                //dl->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), theme.accent);
                ImGui::Dummy(ImVec2(sz, sz));
                sl();

                if(mi(theme.name)) {
                    //themes::set_theme(theme.id, scale);
                    on_changed(theme.id);
                }
            }
        }
    }

    // ---- menu_bar ----

    menu::menu(const std::string& title, bool reserve_icon_space, std::string icon) : icon{std::move(icon)} {
        const string icon_prefix = "       ";
        if(reserve_icon_space) {
            cp = ImGui::GetCursorPos();
            rendered = ImGui::BeginMenu((icon_prefix + title).c_str());
        } else {
            rendered = ImGui::BeginMenu(title.c_str());
        }
    }

    menu::~menu() {
        if(rendered) {
            ImGui::EndMenu();
        }

        if(!icon.empty()) {
            ImGui::SetCursorPos(cp);
            lbl(icon);
        }
    }

    menu_bar::menu_bar() {
        rendered = ImGui::BeginMenuBar();
    }

    menu_bar::~menu_bar() {
        if(rendered) {
            ImGui::EndMenuBar();
        }
    }

    // ---- label ----

    void lbl(const std::string& text, const style& style) {
        const rgb_colour text_color = style.colour ? style.colour : get_color(style.emp, sub_emphasis::normal);
        ImGui::PushStyleColor(ImGuiCol_Text, text_color);

        optional<texter> tx; // used for RAII
        if(style.font_size != .0f || style.font_w != font_weight::regular) {
            tx.emplace(style.font_size, style.font_w);
        }

        if(style.center_x) {
            float avail_width = avail_x();
            float text_width = ImGui::CalcTextSize(text.c_str()).x;
            float offset = (avail_width - text_width) / 2;
            if(offset > 0.0f) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            }
        }

        if(style.center_y) {
            float avail_height = avail_y();
            float text_height = ImGui::CalcTextSize(text.c_str()).y;
            float offset = (avail_height - text_height) / 2;
            if(offset > 0.0f) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
            }
        }

        if(style.text_wrap_pos > .0f)
            ImGui::PushTextWrapPos(style.text_wrap_pos);

        ImGui::TextUnformatted(text.c_str());

        if(style.text_wrap_pos > .0f)
            ImGui::PopTextWrapPos();

        ImGui::PopStyleColor();
    }

    sz text_size_get(const string& text, float font_size_diff, float wrap_width) {
        texter scaler(font_size_diff);
        return ImGui::CalcTextSize(text.c_str(), nullptr, false, wrap_width);
    }

    bool selectable(const std::string& text, bool span_columns) {
        return ImGui::Selectable(text.c_str(),
                                 false,
                                 span_columns ? ImGuiSelectableFlags_SpanAllColumns : ImGuiSelectableFlags_None);
    }

    template<typename T>
    bool input(T& value, int value_length, const std::string& label, bool enabled, float width, bool is_readonly) {
        bool fired;
        if(!enabled) ImGui::BeginDisabled();
        if(width != 0)
            ImGui::PushItemWidth(width);

        ImGuiInputTextFlags flags{};
        if(is_readonly) flags |= ImGuiInputTextFlags_ReadOnly;

        if constexpr(std::is_same_v<T, std::string>) {
            fired = ImGui::InputText(label.c_str(), &value, flags);
        } else if constexpr(std::is_same_v<T, char *>) {
            fired = ImGui::InputText(label.c_str(), value, value_length, flags);
        } else if constexpr(std::is_same_v<T, int>) {
            fired = ImGui::InputInt(label.c_str(), &value, 1, 100, flags);
        }

        if(width != 0)
            ImGui::PopItemWidth();
        if(!enabled) ImGui::EndDisabled();
        return fired;
    }

    bool input(std::string& value, const std::string& label, bool enabled, float width, bool is_readonly) {
        return input<std::string>(value, 0, label, enabled, width, is_readonly);
    }

    bool input(char* value, int value_length, const std::string& label, bool enabled, float width, bool is_readonly) {
        return input<char *>(value, value_length, label, enabled, width, is_readonly);
    }

    bool input(int& value, const std::string& label, bool enabled, float width, bool is_readonly) {
        return input<int>(value, 0, label, enabled, width, is_readonly);
    }

    template<typename T>
    bool slider(T& value, T min, T max, const std::string& label) {
        bool fired;

        if constexpr(std::is_same_v<T, float>) {
            fired = ImGui::SliderFloat(label.c_str(), &value, min, max);
        } else if constexpr(std::is_same_v<T, int>) {
            fired = ImGui::SliderInt(label.c_str(), &value, min, max);
        } else {
            fired = false;
        }

        return fired;
    }

    bool slider_classic(float& value, float min, float max, const std::string& label) {
        return slider<float>(value, min, max, label);
    }

    bool slider_classic(int& value, int min, int max, const std::string& label) {
        return slider<int>(value, min, max, label);
    }

    template<typename T>
    bool slider_impl(T& value, T min, T max, const std::string& label, T step, bool ticks, emphasis emp,
                     bool is_small) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if(window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label.c_str());
        const float w = ImGui::CalcItemWidth();

        // Calculate dimensions
        const float knob_radius = (is_small ? 3.0f : 6.0f) * scale;
        const float track_height = (is_small ? 1.0f : 2.0f) * scale;
        const float height = knob_radius + style.FramePadding.y * 2;

        // Reserve space for the widget
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size(w, height);
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(size, style.FramePadding.y);
        if(!ImGui::ItemAdd(bb, id))
            return false;

        // Handle input
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
        bool value_changed = false;

        if(held) {
            float mouse_x = g.IO.MousePos.x;
            float t = ImClamp((mouse_x - (bb.Min.x + knob_radius)) / (bb.GetWidth() - knob_radius * 2.0f), 0.0f, 1.0f);
            T new_value;

            if constexpr(std::is_integral_v<T>) {
                new_value = static_cast<T>(min + std::round(t * (max - min)));
            } else {
                new_value = min + t * (max - min);
            }

            // Snap to step if specified
            if(step > 0) {
                if constexpr(std::is_integral_v<T>) {
                    new_value = min + static_cast<T>(std::round(static_cast<float>(new_value - min) / step) * step);
                } else {
                    new_value = min + std::round((new_value - min) / step) * step;
                }
                new_value = ImClamp(new_value, min, max);
            }

            if(new_value != value) {
                value = new_value;
                value_changed = true;
            }
        }

        // Calculate knob position based on current value
        const float t = (max > min)
                            ? ImClamp(static_cast<float>(value - min) / static_cast<float>(max - min), 0.0f, 1.0f)
                            : 0.0f;
        const float knob_x = bb.Min.x + knob_radius + t * (bb.GetWidth() - knob_radius * 2.0f);
        const float knob_y = bb.Min.y + height / 2.0f;

        // Determine colors based on emphasis
        const auto track_filled_color = emp == emphasis::none
            ? rgb_colour{ImGuiCol_SliderGrabActive}
            : get_color(emp, sub_emphasis::normal);

        // Draw track (line)
        ImDrawList* draw_list = window->DrawList;
        float track_y = knob_y;
        ImU32 track_color = ImGui::GetColorU32(ImGuiCol_FrameBg);

        // Background track
        draw_list->AddLine(
            ImVec2(bb.Min.x + knob_radius, track_y),
            ImVec2(bb.Max.x - knob_radius, track_y),
            track_color, track_height);

        // Filled portion of track (from start to knob)
        draw_list->AddLine(
            ImVec2(bb.Min.x + knob_radius, track_y),
            ImVec2(knob_x, track_y),
            track_filled_color, track_height);

        // Draw ticks if enabled and step is set
        if(ticks && step > 0) {
            const float tick_height = knob_radius * 0.6f;
            ImU32 tick_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            float track_start = bb.Min.x + knob_radius;
            float track_end = bb.Max.x - knob_radius;
            float track_width = track_end - track_start;

            for(T v = min; v <= max; v += step) {
                float tick_t = (max > min) ? static_cast<float>(v - min) / static_cast<float>(max - min) : 0.0f;
                float tick_x = track_start + tick_t * track_width;
                draw_list->AddLine(
                    ImVec2(tick_x, track_y - tick_height),
                    ImVec2(tick_x, track_y + tick_height),
                    tick_color, 1.0f);
            }
        }

        // Draw knob (bubble)
        const auto knob_colour = emp == emphasis::none
            ? rgb_colour{held ? ImGuiCol_SliderGrabActive : (hovered ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab)}
            : get_color(emp, held ? sub_emphasis::active : hovered ? sub_emphasis::hovered : sub_emphasis::normal);

        draw_list->AddCircleFilled(ImVec2(knob_x, knob_y), knob_radius, knob_colour);

        // Show tooltip only when the knob itself is hovered (not the whole widget)
        ImVec2 mouse_pos = g.IO.MousePos;
        float dx = mouse_pos.x - knob_x;
        float dy = mouse_pos.y - knob_y;
        if(dx * dx + dy * dy <= knob_radius * knob_radius) {
            // If a label is provided show it, otherwise show the current value
            if constexpr(std::is_integral_v<T>) {
                ImGui::SetTooltip("%d", static_cast<int>(value));
            } else {
                char buf[64];
                int decimals = 3;
                if(step > 0) {
                    decimals = std::max(0, (int) std::ceil(-std::log10(step)));
                }
                std::string fmt = "%." + std::to_string(decimals) + "f";
                sprintf(buf, fmt.c_str(), static_cast<double>(value));
                ImGui::SetTooltip("%s", buf);
            }
        }

        // Draw label vertically centered with the track
        if(!label.empty() && label[0] != '#') {
            float text_height = ImGui::GetTextLineHeight();
            float label_y = bb.Min.y + (height - text_height) / 2.0f;
            ImGui::SetCursorScreenPos(ImVec2(bb.Max.x + style.ItemInnerSpacing.x, label_y));
            ImGui::TextUnformatted(label.c_str());
        }

        return value_changed;
    }

    bool slider(float& value, float min, float max, const std::string& label, float step, bool ticks, emphasis emp,
                bool is_small) {
        return slider_impl<float>(value, min, max, label, step, ticks, emp, is_small);
    }

    bool slider(int& value, int min, int max, const std::string& label, int step, bool ticks, emphasis emp,
                bool is_small) {
        return slider_impl<int>(value, min, max, label, step, ticks, emp, is_small);
    }

    void autoscroll_input_ml(const string& id) {
        const char* child_window_name = NULL;
        ImGuiContext* g = ImGui::GetCurrentContext();
        ImFormatStringToTempBuffer(&child_window_name, NULL, "%s/%s_%08X",
                                   g->CurrentWindow->Name, id.c_str(), ImGui::GetID(id.c_str()));
        ImGuiWindow* child_window = ImGui::FindWindowByName(child_window_name);
        if(child_window) {
            ImGui::SetScrollY(child_window, child_window->ScrollMax.y);
        }
    }

    bool input_ml(const string& id, string& value, unsigned int line_height, bool autoscroll, bool enabled) {
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImVec2 size{-FLT_MIN, ImGui::GetTextLineHeight() * line_height};

        if(!enabled) ImGui::BeginDisabled();

        bool ret = ImGui::InputTextMultiline(id.c_str(), &value, size, flags);

        if(!enabled) ImGui::EndDisabled();

        if(autoscroll) {
            autoscroll_input_ml(id);
        }

        return ret;
    }

    template<typename T>
    bool input_ml(const string& id, T value, int value_length, float height, bool autoscroll, bool enabled,
                  bool use_fixed_font) {
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImVec2 size{-FLT_MIN, height == 0 ? -FLT_MIN : height};

        if(!enabled) ImGui::BeginDisabled();

        ImFont* f = use_fixed_font ? fonts::font_loader::get_font(font_weight::bold) : nullptr;

        if(f) {
            ImGui::PushFont(f);
        }

        bool ret;
        if constexpr(std::is_same_v<T, std::string&>) {
            ret = ImGui::InputTextMultiline(id.c_str(), &value, size, flags);
        } else if constexpr(std::is_same_v<T, char *>) {
            ret = ImGui::InputTextMultiline(id.c_str(), value, value_length, size, flags);
        } else {
            ret = false;
        }

        if(f) {
            ImGui::PopFont();
        }

        if(!enabled) ImGui::EndDisabled();

        if(autoscroll) {
            autoscroll_input_ml(id);
        }

        return ret;
    }

    bool input_ml(const string& id, string& value, float height, bool autoscroll, bool enabled, bool use_fixed_font) {
        return input_ml<string&>(id, value, 0, height, autoscroll, enabled, use_fixed_font);
    }

    bool input_ml(const std::string& id, char* value, int value_length, float height, bool autoscroll, bool enabled,
                  bool use_fixed_font) {
        return input_ml<char *>(id, value, value_length, height, autoscroll, enabled, use_fixed_font);
    }

    void markdown(const std::string& text, const markdown_config& config) {
        static x::md md;
        // container c;
        // c.auto_size_y();
        // guard gc{c};
        md.print(text.c_str(), text.c_str() + text.size(), config);
    }

    // ---- tooltip ----

    void tt(const std::string& text, show_delay delay) {
        if(!ImGui::IsItemHovered(to_hovered_flags(delay))) {
            return;
        }

        ImGui::SetTooltip("%s", text.c_str());
    }

    void tt(const char* text, show_delay delay) {
        if(!ImGui::IsItemHovered(to_hovered_flags(delay))) {
            return;
        }

        ImGui::SetTooltip("%s", text);
    }

    // ---- position ----

    void get_pos(float& x, float& y) {
        ImVec2 p = ImGui::GetCursorPos();
        x = p.x;
        y = p.y;
    }

    void cur_get(float& x, float& y) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        x = p.x;
        y = p.y;
    }

    point cur_get() {
        return ImGui::GetCursorScreenPos();
    }

    void cur_set(float x, float y) {
        ImGui::SetCursorScreenPos(ImVec2{x, y});
    }

    void cur_set(const point& pos) {
        ImGui::SetCursorScreenPos(pos);
    }

    void cur_move(float x, float y) {
        auto pos = ImGui::GetCursorScreenPos();
        pos.x += x;
        pos.y += y;
        ImGui::SetCursorScreenPos(pos);
    }

    void cur_move(ImVec2 shift) {
        auto pos = ImGui::GetCursorScreenPos();
        pos.x += shift.x;
        pos.y += shift.y;
        ImGui::SetCursorScreenPos(pos);
    }

    rect window_rect_get() {
        point pos = ImGui::GetWindowPos();
        sz size = ImGui::GetWindowSize();
        return rect{pos, pos + size};
    }

    float avail_x() {
        return ImGui::GetContentRegionAvail().x;
    }

    float avail_y() {
        return ImGui::GetContentRegionAvail().y;
    }

    // ----- basic drawing ----

    rect item_rect_get() {
        return rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }

    void draw_text(const point& pos, emphasis emp, const std::string& text) {
        wdl->AddText(pos, get_color(emp, sub_emphasis::normal), text.c_str());
    }

    void draw_text(const point& pos, const rgb_colour& colour, const std::string& text) {
        wdl->AddText(pos, colour, text.c_str());
    }

    void draw_rect(const rect& rect, rgb_colour colour, float thickness, float rounding) {
        wdl->AddRect(rect.lt(), rect.rb(), colour, rounding, ImDrawFlags_None, thickness);
    }

    void draw_rect_filled(const rect& rect, rgb_colour colour, float rounding) {
        wdl->AddRectFilled(rect.lt(), rect.rb(), colour, rounding);
    }

    void draw_circle(const point& center, float radius, rgb_colour colour, bool filled, float thickness,
                     int num_segments) {
        if(filled) {
            wdl->AddCircleFilled(center, radius, colour, num_segments);
        } else {
            wdl->AddCircle(center, radius, colour, num_segments, thickness);
        }
    }

    void dummy(float width, float height) {
        ImGui::Dummy(ImVec2(width, height));
    }

    void dummy(ImVec2 size) {
        ImGui::Dummy(size);
    }

    // ---- image ----

    void image(app& app, const std::string& key, size_t width, size_t height,
               float uv0_x, float uv0_y, float uv1_x, float uv1_y) {
        auto tex = app.get_texture(key);
        if(tex && tex->data) {
            ImGui::Image((ImTextureID) tex->data, ImVec2(width, height),
                         ImVec2(uv0_x, uv0_y), ImVec2(uv1_x, uv1_y));
        } else {
            ImGui::Dummy(ImVec2(width, height));
        }
    }

    void image_rounded(app& app, const std::string& key, size_t width, size_t height, float rounding,
                       float uv0_x, float uv0_y, float uv1_x, float uv1_y) {
        auto tex = app.get_texture(key);
        if(tex && tex->data) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max = ImVec2(p_min.x + width, p_min.y + height);
            ImGui::Dummy(ImVec2(width, height));
            dl->AddImageRounded((ImTextureID) tex->data, p_min, p_max,
                                ImVec2(uv0_x, uv0_y), ImVec2(uv1_x, uv1_y), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
                                rounding);
        }
    }

    void icon_image(app& app, const std::string& key) {
        float size = 16 * app.scale;
        image(app, key, size, size);
    }

    bool icon_selector(app& app, const std::string& path, size_t square_size) {
        group g;

        if(path.empty()) {
            ImGui::Dummy(ImVec2(square_size, square_size));
        } else {
            app.preload_texture(path, path);
            image_rounded(app, path, square_size, square_size, square_size / 2);
        }
        return is_leftclicked();
    }

    // ---- spacing ----

    void spc(size_t repeat) {
        for(int i = 0; i < repeat; i++)
            ImGui::Spacing();
    }

    // ---- same line ----

    void sl(float offset, bool spacing) {
        ImGui::SameLine(offset, spacing ? -1 : 0);
    }

    // ---- separator ----

    void sep(const string& text) {
        if(text.empty())
            ImGui::Separator();
        else
            ImGui::SeparatorText(text.c_str());
    }

    // ---- button ----

    bool button(const std::string& text, emphasis emp, bool is_enabled, bool is_small, const string& tooltip_text,
                float width, float height) {
        if(!is_enabled) {
            ImGui::BeginDisabled(true);
        }

        bool clicked;

        if(emp != emphasis::none) {
            ImGui::PushStyleColor(ImGuiCol_Button, get_color(emp, sub_emphasis::normal));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, get_color(emp, sub_emphasis::hovered));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, get_color(emp, sub_emphasis::active));
            ImGui::PushStyleColor(ImGuiCol_Text, get_color(emp, sub_emphasis::normal_text));
        }

        if(is_small) {
            clicked = ImGui::SmallButton(text.c_str());
        } else {
            clicked = ImGui::Button(text.c_str(), ImVec2(width, height));
        }

        if(is_enabled && is_hovered())
            mouse_cursor(mouse_cursor_type::hand);

        if(emp != emphasis::none) {
            ImGui::PopStyleColor(4);
        }

        if(!is_enabled) {
            ImGui::EndDisabled();
        }

        if(!tooltip_text.empty()) {
            tt(tooltip_text);
        }

        return clicked;
    }

    bool icon_checkbox(const std::string& icon, bool& is_checked, bool reversed, const string& tooltip) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        bool is_active = (reversed ? !is_checked : is_checked);
        if(!is_active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        }

        bool clicked = ImGui::Button(icon.c_str(), ImVec2(0, 0));

        if(!is_active) {
            ImGui::PopStyleColor();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        if(ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if(!tooltip.empty()) {
                ImGui::SetTooltip("%s", tooltip.c_str());
            }
        }

        if(clicked) {
            is_checked = !is_checked;
        }

        return clicked;
    }

    bool checkbox(const std::string& label, bool& is_checked) {
        return ImGui::Checkbox(label.c_str(), &is_checked);
    }

    bool hyperlink(const std::string& text, const std::string& url_to_open) {
        if(!url_to_open.empty()) {
            ImGui::TextLinkOpenURL(text.c_str(), url_to_open.c_str());
            return false;
        }

        return ImGui::TextLink(text.c_str());
    }

    bool colour(const std::string& label, rgb_colour& colour) {
        ImColor col = colour;
        if(ImGui::ColorEdit4(label.c_str(), &col.Value.x,
                             ImGuiColorEditFlags_NoAlpha |
                             ImGuiColorEditFlags_NoSidePreview |
                             ImGuiColorEditFlags_NoInputs |
                             ImGuiColorEditFlags_NoTooltip)) {
            col.Value.w = 1.0f; // due to no-alpha flag, set to max alpha on change
            colour = rgb_colour{col};
            return true;
        }
        return false;
    }

    bool small_checkbox(const std::string& label, bool& is_checked) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        bool r = checkbox(label, is_checked);
        ImGui::PopStyleVar();
        return r;
    }

    bool icon_list(const std::vector<std::pair<std::string, string> >& options, unsigned int& selected) {
        bool changed{false};
        for(int si = 0; si < options.size(); si++) {
            if(si > 0) ImGui::SameLine();

            if(bool is_selected = selected == si) {
                lbl(options[si].first);
            } else {
                lbl(options[si].first, {.emp = emphasis::disabled});

                // show "hand" cursor for disabled (selectable) options
                if(ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                    // check if mouse is clicked on this item
                    if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        selected = si;
                        changed = true;
                    }
                }
            }

            if(!options[si].second.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("%s", options[si].second.c_str());
            }
        }
        return changed;
    }

    bool accordion(const std::string& header, bool default_open) {
        ImGuiTreeNodeFlags flags = 0;
        if(default_open) flags |= ImGuiTreeNodeFlags_DefaultOpen;
        return ImGui::CollapsingHeader(header.c_str(), flags);
    }

    bool combo(const string& label, const std::vector<std::string>& options, unsigned int& selected, float width) {
        bool ret{false};

        if(width != 0) {
            width *= scale;
            ImGui::PushItemWidth(width);
        }

        if(selected >= options.size()) {
            selected = options.empty() ? 0 : options.size() - 1;
        }

        string preview_value = options.empty() ? "" : options[selected];

        if(ImGui::BeginCombo(label.c_str(), preview_value.c_str())) {
            for(size_t i = 0; i < options.size(); i++) {
                bool is_selected = selected == i;
                if(ImGui::Selectable(options[i].c_str(), is_selected)) {
                    selected = i;
                    ret = true;
                }

                if(is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        if(width != 0)
            ImGui::PopItemWidth();

        return ret;
    }

    bool list(const std::string& label, const std::vector<std::string>& options, unsigned int& selected, float width) {
        bool ret{false};

        if(width != 0) {
            width *= scale;
            ImGui::PushItemWidth(width);
        }

        if(selected >= options.size()) {
            selected = options.empty() ? 0 : options.size() - 1;
        }

        string preview_value = options.empty() ? "" : options[selected];

        if(ImGui::BeginListBox(label.c_str())) {
            for(size_t i = 0; i < options.size(); i++) {
                bool is_selected = selected == i;
                if(ImGui::Selectable(options[i].c_str(), is_selected,
                                     is_selected ? ImGuiSelectableFlags_Highlight : 0)) {
                    selected = i;
                    ret = true;
                }

                if(is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }

        if(width != 0)
            ImGui::PopItemWidth();

        return ret;
    }

    bool radio(const std::string& label, bool is_active) {
        return ImGui::RadioButton(label.c_str(), is_active);
    }

    bool small_radio(const std::string& label, bool is_active) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        bool r = radio(label, is_active);
        ImGui::PopStyleVar();
        return r;
    }

    void spinner_hbo_dots(float radius, float thickness, float speed, size_t dot_count) {
        //ImSpinner::demoSpinners();
        ImSpinner::SpinnerHboDots("SpinnerHboDots", radius, thickness, ImSpinner::white, 0.1f, 0.5f, speed, dot_count,
                                  0);
    }

    void toast(emphasis emp, const std::string& message) {
        x::toast::push(emp, "", message, 5000);
    }

    void notify_render_frame() {
        // Notifications style setup
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f); // Disable round borders
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.1f); // really thin border
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f);

        // Notifications color setup
        //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f)); // Background color


        // Main rendering function
        x::toast::render_frame();


        // WARNING: Argument MUST match the amount of ImGui::PushStyleVar() calls
        //ImGui::PopStyleVar(2);
        // ImGui::PopStyleVar(2);

        // Argument MUST match the amount of ImGui::PushStyleColor() calls
        //ImGui::PopStyleColor(1);
    }

    // ---- group ----

    group::group(bool full_width) : full_width{full_width} {
        ImGui::BeginGroup();
    }

    group::~group() {
        if(full_width) {
            // add some content horizontally to force full width
            float max_width = ImGui::GetWindowWidth();
            ImGui::SetCursorPosX(0.0);
            //ImGui::InvisibleButton("##ib", ImVec2(max_width, 0.1));
            ImGui::Dummy(ImVec2(max_width, 0.1));
            ImGui::SameLine();
        }

        ImGui::EndGroup();
    }

    // ---- status bar ----

    status_bar::status_bar() : style{ImGui::GetStyle()}, cursor_before{ImGui::GetCursorPos()} {
        //auto io = ImGui::GetIO();

        float height = ImGui::GetFontBaked()->Size + style.FramePadding.y * 2.0f;

        ImVec2 ws = ImGui::GetWindowSize();

        ImGui::SetCursorPos(ImVec2(0, ws.y - height));
        ImGui::BeginChild("##StatusBar", ImVec2(ws.x, height));
        ImGui::SetCursorPos(ImVec2(style.FramePadding.x, style.FramePadding.y));
    }

    status_bar::~status_bar() {
        ImGui::EndChild();

        auto min = ImGui::GetItemRectMin();
        auto max = ImGui::GetItemRectMax();
        ImDrawList* fdl = ImGui::GetWindowDrawList();

        fdl->AddRectFilled(min, max,
                           (ImU32) rgb_colour{
                               style.Colors[ImGuiCol_MenuBarBg]
                           }, style.FrameRounding);
    }

    // mouse helpers

    bool is_leftclicked() {
        return ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    }

    bool is_rightclicked() {
        return ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);
    }

    bool is_hovered() {
        return ImGui::IsItemHovered();
    }

    bool is_focused() {
        return ImGui::IsItemFocused();
    }

    float frame_delta() {
        return initialized() ? ImGui::GetIO().DeltaTime : 0.0f;
    }

    void mouse_cursor(mouse_cursor_type mct) {
        ImGui::SetMouseCursor(static_cast<ImGuiMouseCursor_>(mct));
    }

    tree_node::tree_node(const std::string& label,
                         const bool open_by_default,
                         const bool is_leaf,
                         const bool span_all_cols,
                         const emphasis emp) {
        ImGuiTreeNodeFlags flags{0};
        if(open_by_default) {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }
        if(is_leaf) {
            flags |= ImGuiTreeNodeFlags_Leaf; // | ImGuiTreeNodeFlags_Bullet;
        }

        if(span_all_cols) {
            flags |= ImGuiTreeNodeFlags_SpanAllColumns;
        }

        // todo: since 1.92 trees can draw hierarchy lines

        if(emp == emphasis::none) {
            opened = ImGui::TreeNodeEx(label.c_str(), flags);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, get_color(emp, sub_emphasis::normal));
            opened = ImGui::TreeNodeEx(label.c_str(), flags);
            ImGui::PopStyleColor();
        }
    }

    tree_node::~tree_node() {
        if(opened) {
            ImGui::TreePop();
        }
    }

    // colour helpers

    ImU32 imcol32(ImGuiCol idx) {
        ImVec4 color = ImGui::GetStyle().Colors[idx]; // Retrieve the color as ImVec4
        // Convert from ImVec4 (floats) to ImU32
        return IM_COL32((int)(color.x * 255.0f), (int)(color.y * 255.0f), (int)(color.z * 255.0f),
                        (int)(color.w * 255.0f));
    }

    rgb_colour get_color(const emphasis emp, const sub_emphasis as) {
        switch(emp) {
            case emphasis::primary:
                switch(as) {
                    case sub_emphasis::normal:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisPrimary]};
                    case sub_emphasis::normal_text:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisPrimaryText]};
                    case sub_emphasis::hovered:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisPrimaryHovered]};
                    case sub_emphasis::active:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisPrimaryActive]};
                }
                break;
            case emphasis::secondary:
                switch(as) {
                    case sub_emphasis::normal:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSecondary]};
                    case sub_emphasis::normal_text:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSecondaryText]};
                    case sub_emphasis::hovered:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSecondaryHovered]};
                    case sub_emphasis::active:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSecondaryActive]};
                }
                break;
            case emphasis::success:
                switch(as) {
                    case sub_emphasis::normal:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSuccess]};
                    case sub_emphasis::normal_text:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSuccessText]};
                    case sub_emphasis::hovered:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSuccessHovered]};
                    case sub_emphasis::active:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisSuccessActive]};
                }
                break;
            case emphasis::error:
                switch(as) {
                    case sub_emphasis::normal:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisError]};
                    case sub_emphasis::normal_text:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisErrorText]};
                    case sub_emphasis::hovered:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisErrorHovered]};
                    case sub_emphasis::active:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisErrorActive]};
                }
                break;
            case emphasis::warning:
                switch(as) {
                    case sub_emphasis::normal:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisWarning]};
                    case sub_emphasis::normal_text:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisWarningText]};
                    case sub_emphasis::hovered:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisWarningHovered]};
                    case sub_emphasis::active:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisWarningActive]};
                }
                break;
            case emphasis::info:
                switch(as) {
                    case sub_emphasis::normal:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisInfo]};
                    case sub_emphasis::normal_text:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisInfoText]};
                    case sub_emphasis::hovered:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisInfoHovered]};
                    case sub_emphasis::active:
                        return rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisInfoActive]};
                }
                break;
            case emphasis::disabled:
                return as == sub_emphasis::normal_text
                    ? rgb_colour{themes::GreyColors[themes::GreyCol_EmphasisDisabledText]}
                    : rgb_colour{ImGuiCol_TextDisabled};
            default:
                return rgb_colour{ImGuiCol_Text};
        }
        return rgb_colour{ImGuiCol_Text};
    }

    void label_debug_info() {
        // FPS
        const float fps = ImGui::GetIO().Framerate;
        char buf[32];
        sprintf(buf, "%.2f", fps);
        lbl("fps: ");
        sl();
        lbl(buf);

        // scale
        sl();
        lbl(", scale: ");
        sl();
        lbl(std::to_string(scale));
    }

    // ---- tab bar ----

    tab_bar::tab_bar(const std::string& id, bool tab_list_popup, bool scroll) {
        if(tab_list_popup)
            flags |= ImGuiTabBarFlags_TabListPopupButton;
        if(scroll)
            flags |= ImGuiTabBarFlags_FittingPolicyScroll;
        rendered = ImGui::BeginTabBar(id.c_str(), flags);
        tab_index = 0;
    }

    tab_bar::~tab_bar() {
        if(rendered) {
            ImGui::EndTabBar();
        }
    }

    tab_bar_item tab_bar::next_tab(const string& title, bool unsaved, bool selected) {
        return tab_bar_item{title + "##" + std::to_string(tab_index++), unsaved, selected};
    }

    tab_bar_item::tab_bar_item(const std::string& id, bool unsaved, bool selected) : id{id} {
        if(unsaved) {
            flags |= ImGuiTabItemFlags_UnsavedDocument;
        }
        if(selected) {
            flags |= ImGuiTabItemFlags_SetSelected;
        }
        rendered = ImGui::BeginTabItem(id.c_str(), nullptr, flags);
    }

    tab_bar_item::~tab_bar_item() {
        if(rendered) {
            ImGui::EndTabItem();
            rendered = false;
        }
    }

    // ---- popup ----

    popup::popup(std::string id) : id{std::move(id)} {
    }

    void popup::enter() {
        if(do_open) {
            ImGui::OpenPopup(id.c_str());
            do_open = false;
        }

        if(rendered && open_x != 0 && open_y != 0) {
            ImGui::SetNextWindowPos(ImVec2(open_x, open_y));
        }
        rendered = ImGui::BeginPopup(id.c_str());
    }

    void popup::leave() {
        if(rendered) {
            ImGui::EndPopup();
        }
    }

    void popup::open() {
        do_open = true;
    }

    void popup::open(float x, float y) {
        do_open = true;
        open_x = x;
        open_y = y;
    }

    // ImGuiColorTextEdit

    code_editor::code_editor(code_editor::language l, bool border, bool show_line_numbers) : id{
            generate_id("TextEditor")
        },
        border{border},
        show_line_numbers{show_line_numbers},
        lng{l}, current_lng{-1} {
        //editor.SetShowWhitespaces(true);
        editor.SetTabSize(2);
        editor.SetShowLineNumbersEnabled(false);
        //editor.SetShowKeywordTooltips(false);
    }

    void code_editor::set_text(const std::string& text) {
        editor.SetText(text);
    }

    std::string code_editor::get_text() {
        return editor.GetText();
    }

    bool code_editor::render(float width, float height) {
        ImFont* f = fonts::font_loader::get_font(font_weight::fixed_size);
        if(f) {
            ImGui::PushFont(f);
        }

        if(current_lng != lng) {
            //editor.SetLanguageDefinition((TextEditor::LanguageDefinitionId)lng);
            set_language(lng);
            current_lng = lng;
        }

        //editor.SetAutoIndentEnabled(true);
        editor.SetShowLineNumbersEnabled(show_line_numbers);
        editor.Render(id.c_str(), ImVec2(width, height), border);
        //editor.Render(id.c_str(), false, ImVec2(width, height), border);


        if(f) {
            ImGui::PopFont();
        }

        return false;
        //return editor.IsTextChanged();
    }

    void code_editor::set_language(language l) {
        switch(l) {
            case language::none:
                editor.SetLanguage(nullptr);
                break;
            case language::cpp:
                editor.SetLanguage(TextEditor::Language::Cpp());
                break;
            case language::c:
                editor.SetLanguage(TextEditor::Language::C());
                break;
            case language::cs:
                editor.SetLanguage(TextEditor::Language::Cs());
                break;
            case language::lua:
                editor.SetLanguage(TextEditor::Language::Lua());
                break;
            case language::python:
                editor.SetLanguage(TextEditor::Language::Python());
                break;
            case language::json:
                editor.SetLanguage(TextEditor::Language::Json());
                break;
            case language::markdown:
                editor.SetLanguage(TextEditor::Language::Markdown());
                break;
            default:
                editor.SetLanguage(nullptr);
                break;
        }
    }

    big_table::big_table(const std::string& id, const vector<string>& columns, size_t row_count,
                         float outer_width,
                         float outer_height,
                         bool alternate_row_bg) : columns_size{columns.size()}, outer_size{outer_width, outer_height} {
        if(alternate_row_bg) {
            flags |= ImGuiTableFlags_RowBg;
        }
        rendered = ImGui::BeginTable(id.c_str(), static_cast<int>(columns.size()), flags, outer_size);
        if(rendered) {
            ImGui::TableSetupScrollFreeze(0, 1);
            clipper.Begin(static_cast<int>(row_count));

            // setup columns
            for(const string& cn: columns) {
                if(cn.empty() || !cn.ends_with('+')) {
                    ImGui::TableSetupColumn(cn.c_str());
                } else {
                    string n = cn.substr(0, cn.size() - 1);
                    ImGui::TableSetupColumn(n.c_str(), ImGuiTableColumnFlags_WidthStretch);
                }
            }
            ImGui::TableHeadersRow();
        }
    }

    big_table::~big_table() {
        if(rendered) {
            ImGui::EndTable();
        }
    }

    void big_table::render_data(const std::function<void(int, int)>& cell_render) {
        if(!rendered) return;

        while(clipper.Step()) {
            for(int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                ImGui::TableNextRow();
                for(int col = 0; col < columns_size; col++) {
                    if(!ImGui::TableSetColumnIndex(col)) {
                        // don't bother rendering invisible columns
                        continue;
                    }

                    if(cell_render) {
                        cell_render(row, col);
                    }
                }
            }
        }
    }

    // -- plotting

#if GREY_INCLUDE_IMPLOT

    int bar_data[4] = {10, 20, 30, 40};
    const char* labels[] = {"A", "B", "C", "D"};


    void plot_demo() {
        if(ImPlot::BeginPlot("##plot")) {
            ImPlot::SetupAxisTicks(ImAxis_Y1, 0, 3, 4, labels, false);
            ImPlot::PlotBars("##bars", bar_data, 4, 0.67, 0, ImPlotBarsFlags_Horizontal);

            ImPlot::EndPlot();
        }
    }

    void plot_realtime(const string& name, scrolling_buffer& points, float x_min, float x_max, float y_min,
                       float y_max) {
        if(ImPlot::BeginPlot(name.c_str())) {
            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
            ImPlot::SetupAxes(nullptr, nullptr, flags, flags);

            ImPlot::SetupAxisLimits(ImAxis_X1, x_min, x_max, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, y_min, y_max, ImGuiCond_Always);

            //ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
            ImPlot::PlotLine("##realtime", &points.data[0].x, &points.data[0].y, points.data.size(),
                             0,
                             points.offset, 2 * sizeof(float));


            ImPlot::EndPlot();
        }
    }

    void plot_realtime(const string& name,
                       float x_min, float x_max, float y_min, float y_max,
                       const std::string& name1, scrolling_buffer& points1,
                       const std::string& name2, scrolling_buffer& points2,
                       bool fill) {
        if(ImPlot::BeginPlot(name.c_str())) {
            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
            ImPlot::SetupAxes(nullptr, nullptr, flags, flags);

            ImPlot::SetupAxisLimits(ImAxis_X1, x_min, x_max, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, y_min, y_max, ImGuiCond_Always);

            if(fill) {
                ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
                ImPlot::PlotShaded(name1.c_str(), &points1.data[0].x, &points1.data[0].y, points1.data.size(),
                                   -INFINITY,
                                   0, points1.offset, 2 * sizeof(float));
                ImPlot::PlotShaded(name2.c_str(), &points2.data[0].x, &points2.data[0].y, points2.data.size(),
                                   -INFINITY,
                                   0, points2.offset, 2 * sizeof(float));

                ImPlot::PopStyleVar();
            }

            //ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
            ImPlot::PlotLine(name1.c_str(), &points1.data[0].x, &points1.data[0].y, points1.data.size(),
                             0,
                             points1.offset, 2 * sizeof(float));
            ImPlot::PlotLine(name2.c_str(), &points2.data[0].x, &points2.data[0].y, points2.data.size(),
                             0,
                             points2.offset, 2 * sizeof(float));

            ImPlot::EndPlot();
        }
    }

#endif

    table::table(const std::string& id, const std::vector<std::string>& columns,
                 float outer_width, float outer_height,
                 bool alternate_row_bg)
        : columns_size{columns.size()}, outer_size{outer_width, outer_height} {
        if(alternate_row_bg) {
            flags |= ImGuiTableFlags_RowBg;
        }

        rendered = ImGui::BeginTable(id.c_str(), columns.size(), flags, outer_size);
        if(rendered) {
            ImGui::TableSetupScrollFreeze(0, 1);
            // setup columns
            for(const string& cn: columns) {
                if(cn.empty() || !cn.ends_with("+")) {
                    ImGui::TableSetupColumn(cn.c_str());
                } else {
                    string n = cn.substr(0, cn.size() - 1);
                    ImGui::TableSetupColumn(n.c_str(), ImGuiTableColumnFlags_WidthStretch);
                }
            }
            ImGui::TableHeadersRow();
        }
    }

    table::~table() {
        if(rendered) {
            ImGui::EndTable();
        }
    }

    bool table::begin_row() {
        ImGui::TableNextRow();
        return ImGui::TableNextColumn();
    }

    bool table::next_column() {
        return ImGui::TableNextColumn();
    }

    rich_tt::rich_tt(show_delay delay) {
        if(!ImGui::IsItemHovered(to_hovered_flags(delay))) {
            rendered = false;
            return;
        }

        rendered = ImGui::BeginTooltip();
    }

    rich_tt::~rich_tt() {
        if(rendered) {
            ImGui::EndTooltip();
        }
    }
}

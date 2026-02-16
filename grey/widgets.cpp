#include "widgets.h"
#include "themes.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "3rdparty/ImGuiNotify.hpp"
#include "3rdparty/imspinner.h"
#include "3rdparty/imgui_markdown/imgui_markdown.h"
#include "fonts/font_loader.h"
#include <iostream>
// for Windows-specific hacks
#ifdef _WIN32
#include <Windows.h>
#endif

// 3rdparty
#include "3rdparty/ImGuiColorTextEdit/langdefs.h"

using namespace std;

namespace grey::widgets {

    // ---- general ----

    float scale = 1.0f;
    static int incrementing_id;

    int generate_int_id() {
        return incrementing_id++;
    }

    static string generate_id(const string& prefix = "") {
        return prefix + std::to_string(incrementing_id++);
    }

    ImGuiHoveredFlags to_hovered_flags(show_delay delay) {
        ImGuiHoveredFlags flags = ImGuiHoveredFlags_None;
        switch(delay) {
            case grey::widgets::show_delay::immediate:
                flags = ImGuiHoveredFlags_DelayNone;
                break;
            case grey::widgets::show_delay::quick:
                flags = ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay;
                break;
            case grey::widgets::show_delay::normal:
                flags = ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay;
                break;
            case grey::widgets::show_delay::slow:
                flags = ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_NoSharedDelay;
                break;
            default:
                break;
        }
        return flags;
    }


    //inline std::string sys_label(const std::string& label) { return label + "##" + id; }

    bool set_emphasis_colours(emphasis em, ImVec4& normal, ImVec4& hovered, ImVec4& active) {
        if (em == emphasis::none) return false;

        switch (em) {
        case emphasis::primary:
            normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisPrimary];
            hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisPrimaryHovered];
            active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisPrimaryActive];
            return true;
        case emphasis::secondary:
            normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisSecondary];
            hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisSecondaryHovered];
            active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisSecondaryActive];
            return true;
        case emphasis::success:
            normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisSuccess];
            hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisSuccessHovered];
            active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisSuccessActive];
            return true;
        case emphasis::error:
            normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisError];
            hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisErrorHovered];
            active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisErrorActive];
            return true;
        case emphasis::warning:
            normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisWarning];
            hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisWarningHovered];
            active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisWarningActive];
            return true;
        case emphasis::info:
            normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisInfo];
            hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisInfoHovered];
            active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisInfoActive];
            return true;
        }

        return false;
    }

    // ---- guard ----

    id_frame::id_frame(ImGuiID id) {
        ImGui::PushID(id);
    }

    id_frame::id_frame(int scope_id) : id_frame(ImGui::GetID(scope_id)) {}

    id_frame::id_frame(const std::string& scope_id) : id_frame(ImGui::GetID(scope_id.c_str())) {}

    id_frame::~id_frame() {
        ImGui::PopID();
    }


    // ---- window ----

    window::window(const std::string& title, bool* p_open) : title{ title } {
        this->p_open = p_open;
        wc.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    }

    window& window::size(int width, int height) {
        init_size = ImVec2(width * scale, height * scale);
        return *this;
    }

    window& window::resize(float width, float height) {
        resize_to = ImVec2(width * scale, height * scale);
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

    window& window::border(float width) {
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

        if (border_size >= 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, border_size);
        }

        // set window class to prevent viewports to be merged with main window
        //ImGui::SetNextWindowClass(&wc);

        if (init_size.x)
            ImGui::SetNextWindowSize(init_size, ImGuiCond_Once);

        if (resize_to.x) {
            ImGui::SetNextWindowSize(resize_to);
            resize_to = ImVec2{ 0, 0 };
        }

        if (init_center && !init_center_pos.x) {
            ImVector<ImGuiPlatformMonitor> monitors = ImGui::GetPlatformIO().Monitors;
            size_t midx = 0;
            for (size_t i = 0; i < monitors.Size; i++) {
                if (monitors[i].PlatformHandle == init_center_monitor) {
                    midx = i;
                    break;
                }
            }

            init_center_imgui_monitor = monitors[midx];

            init_center_pos = ImVec2(
                init_center_imgui_monitor.WorkSize.x / 2 - init_size.x / 2 + init_center_imgui_monitor.WorkPos.x,
                init_center_imgui_monitor.WorkSize.y / 2 - init_size.y / 2 + init_center_imgui_monitor.WorkPos.y);
        }

        if (init_center && init_center_pos.x) {
            ImGui::SetNextWindowPos(init_center_pos, ImGuiCond_Appearing);
        }

        if (fill_viewport_enabled) {
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
    }

    void window::leave() {
        if (border_size >= 0)
            ImGui::PopStyleVar();
        ImGui::End();

#ifdef _WIN32
        if (!win32_brought_forward) {
            ImGuiViewport* vp = ImGui::GetWindowViewport();
            if (vp) {
                ::SetForegroundWindow((HWND)vp->PlatformHandleRaw);
                win32_brought_forward = true;
            }
        }
#endif

        if (fill_viewport_enabled) {
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

    container::container(float width, float height) : id{ generate_id() }, size{ width, height } {
    }

    container::container(const std::string& id, float width, float height) : id{ id }, size{ width, height } {
    }

    void container::enter() {
        if (size.y < 0) {
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
        const string IconedPrefix = "       ";

        if (reserve_icon_space) {
            ImVec2 cp = ImGui::GetCursorPos();
            r = ImGui::MenuItem((IconedPrefix + text).c_str());
            if (!icon.empty()) {
                ImGui::SetCursorPos(cp);
                ImGui::Text(icon.c_str());
            }
        }
        else {
            r = ImGui::MenuItem(text.c_str());
        }

        return r;
    }

    void mi_themes(std::function<void(const std::string&)> on_changed) {
        menu m{ "Theme", true, ICON_MD_BRUSH };
        if (m) {
            ImDrawList* dl = ImGui::GetWindowDrawList();

            for (auto& theme : grey::themes::list_themes()) {

                float sz = ImGui::GetTextLineHeight();
                ImVec2 p = ImGui::GetCursorScreenPos();

                // draw a triangle with accent colour in top left corner
                dl->AddTriangleFilled(p, ImVec2(p.x + sz, p.y), ImVec2(p.x, p.y + sz), theme.accent);

                // draw a triangle with base colour in bottom right corner
                dl->AddTriangleFilled(ImVec2(p.x + sz, p.y + sz), ImVec2(p.x + sz, p.y), ImVec2(p.x, p.y + sz),
                    theme.is_dark ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255));

                //dl->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), theme.accent);
                ImGui::Dummy(ImVec2(sz, sz));
                sl();

                if (mi(theme.name)) {
                    //grey::themes::set_theme(theme.id, scale);
                    on_changed(theme.id);
                }
            }
        }
    }

    // ---- menu_bar ----

    menu::menu(const std::string& title, bool reserve_icon_space, const std::string& icon) : icon{ icon } {
        const string IconedPrefix = "       ";
        if (reserve_icon_space) {
            cp = ImGui::GetCursorPos();
            rendered = ImGui::BeginMenu((IconedPrefix + title).c_str());
        }
        else {
            rendered = ImGui::BeginMenu(title.c_str());
        }
    }

    menu::~menu() {
        if (rendered) {
            ImGui::EndMenu();
        }

        if (!icon.empty()) {
            ImGui::SetCursorPos(cp);
            ImGui::Text(icon.c_str());
        }
    }

    menu_bar::menu_bar() {
        rendered = ImGui::BeginMenuBar();
    }

    menu_bar::~menu_bar() {
        if (rendered) {
            ImGui::EndMenuBar();
        }
    }

    // ---- label ----

    void label(const std::string& text, size_t text_wrap_pos, bool enabled) {
        if (text_wrap_pos > 0)
            ImGui::PushTextWrapPos(text_wrap_pos);

        if (enabled)
            ImGui::TextUnformatted(text.c_str());
        else
            ImGui::TextDisabled(text.c_str());

        if (text_wrap_pos > 0)
            ImGui::PopTextWrapPos();
    }

    void label(const std::string& text, rgb_colour colour) {
        ImGui::PushStyleColor(ImGuiCol_Text, colour);
        label(text);
        ImGui::PopStyleColor();
    }

    void label(const std::string& text, emphasis emp, size_t text_wrap_pos, bool enabled) {

        if (emp == emphasis::none || !enabled) {
            label(text, text_wrap_pos, enabled);
        } else {
            ImVec4 normal, hovered, active;
            if (set_emphasis_colours(emp, normal, hovered, active)) {
                ImGui::PushStyleColor(ImGuiCol_Text, normal);
                label(text, text_wrap_pos);
                ImGui::PopStyleColor();
            }
            else {
                label(text, text_wrap_pos);
            }
        }
    }

    bool selectable(const std::string& text, bool span_columns) {
        return ImGui::Selectable(text.c_str(),
            false,
            span_columns ? ImGuiSelectableFlags_SpanAllColumns : ImGuiSelectableFlags_None );
    }

    template<typename T>
    bool input(T& value, int value_length, const std::string& label, bool enabled, float width, bool is_readonly) {
        bool fired;
        if (!enabled) ImGui::BeginDisabled();
        if (width != 0)
            ImGui::PushItemWidth(width);

        ImGuiInputTextFlags flags{};
        if (is_readonly) flags |= ImGuiInputTextFlags_ReadOnly;

        if constexpr (std::is_same_v<T, std::string>) {
            fired = ImGui::InputText(label.c_str(), &value, flags);
        }
        else if constexpr (std::is_same_v<T, char*>) {
            fired = ImGui::InputText(label.c_str(), value, value_length, flags);
        }
        else if constexpr (std::is_same_v<T, int>) {
            fired = ImGui::InputInt(label.c_str(), &value, 1, 100, flags);
        }

        if (width != 0)
            ImGui::PopItemWidth();
        if (!enabled) ImGui::EndDisabled();
        return fired;
    }

    bool input(std::string& value, const std::string& label, bool enabled, float width, bool is_readonly) {
        return input<std::string>(value, 0, label, enabled, width, is_readonly);
    }

    bool input(char* value, int value_length, const std::string& label, bool enabled, float width, bool is_readonly) {
        return input<char*>(value, value_length, label, enabled, width, is_readonly);
    }

    bool input(int& value, const std::string& label, bool enabled, float width, bool is_readonly) {
        return input<int>(value, 0, label, enabled, width, is_readonly);
    }

    template<typename T>
    bool slider(T& value, T min, T max, const std::string& label) {

        bool fired;

        if constexpr (std::is_same_v<T, float>) {
            fired = ImGui::SliderFloat(label.c_str(), &value, min, max);
        }
        else if constexpr (std::is_same_v<T, int>) {
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
    bool slider_impl(T& value, T min, T max, const std::string& label, T step, bool ticks, emphasis emp) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if(window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label.c_str());
        const float w = ImGui::CalcItemWidth();

        // Calculate dimensions
        const float knob_radius = style.FramePadding.y + 8.0f;
        const float track_height = 4.0f;
        const float height = knob_radius * 2.0f;

        // Reserve space for the widget
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size(w, height);
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        // Handle input
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
        bool value_changed = false;

        if (held) {
            float mouse_x = g.IO.MousePos.x;
            float t = ImClamp((mouse_x - (bb.Min.x + knob_radius)) / (bb.GetWidth() - knob_radius * 2.0f), 0.0f, 1.0f);
            T new_value;
            
            if constexpr (std::is_integral_v<T>) {
                new_value = static_cast<T>(min + std::round(t * (max - min)));
            } else {
                new_value = min + t * (max - min);
            }

            // Snap to step if specified
            if (step > 0) {
                if constexpr (std::is_integral_v<T>) {
                    new_value = min + static_cast<T>(std::round(static_cast<float>(new_value - min) / step) * step);
                } else {
                    new_value = min + std::round((new_value - min) / step) * step;
                }
                new_value = ImClamp(new_value, min, max);
            }

            if (new_value != value) {
                value = new_value;
                value_changed = true;
            }
        }

        // Calculate knob position based on current value
        float t = (max > min) ? ImClamp(static_cast<float>(value - min) / static_cast<float>(max - min), 0.0f, 1.0f) : 0.0f;
        float knob_x = bb.Min.x + knob_radius + t * (bb.GetWidth() - knob_radius * 2.0f);
        float knob_y = bb.Min.y + height / 2.0f;

        // Determine colors based on emphasis
        ImVec4 emp_normal, emp_hovered, emp_active;
        bool has_emphasis = set_emphasis_colours(emp, emp_normal, emp_hovered, emp_active);

        // Draw track (line)
        ImDrawList* draw_list = window->DrawList;
        float track_y = knob_y;
        ImU32 track_color = ImGui::GetColorU32(ImGuiCol_FrameBg);
        ImU32 track_filled_color = has_emphasis 
            ? ImGui::GetColorU32(emp_normal) 
            : ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
        
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
        if (ticks && step > 0) {
            const float tick_height = knob_radius * 0.6f;
            ImU32 tick_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            float track_start = bb.Min.x + knob_radius;
            float track_end = bb.Max.x - knob_radius;
            float track_width = track_end - track_start;

            for (T v = min; v <= max; v += step) {
                float tick_t = (max > min) ? static_cast<float>(v - min) / static_cast<float>(max - min) : 0.0f;
                float tick_x = track_start + tick_t * track_width;
                draw_list->AddLine(
                    ImVec2(tick_x, track_y - tick_height),
                    ImVec2(tick_x, track_y + tick_height),
                    tick_color, 1.0f);
            }
        }

        // Draw knob (bubble)
        ImU32 knob_color;
        if (has_emphasis) {
            knob_color = ImGui::GetColorU32(held ? emp_active : (hovered ? emp_hovered : emp_normal));
        } else {
            knob_color = ImGui::GetColorU32(held ? ImGuiCol_SliderGrabActive : (hovered ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab));
        }
        draw_list->AddCircleFilled(ImVec2(knob_x, knob_y), knob_radius, knob_color);

        // Show tooltip only when the knob itself is hovered (not the whole widget)
        ImVec2 mouse_pos = g.IO.MousePos;
        float dx = mouse_pos.x - knob_x;
        float dy = mouse_pos.y - knob_y;
        if (dx*dx + dy*dy <= knob_radius * knob_radius) {
            // If a label is provided show it, otherwise show the current value
            if constexpr (std::is_integral_v<T>) {
                ImGui::SetTooltip("%d", static_cast<int>(value));
            } else {
                char buf[64];
                sprintf(buf, "%.3f", static_cast<double>(value));
                ImGui::SetTooltip("%s", buf);
            }
        }

        // Draw label vertically centered with the track
        if (!label.empty() && label[0] != '#') {
            float text_height = ImGui::GetTextLineHeight();
            float label_y = bb.Min.y + (height - text_height) / 2.0f;
            ImGui::SetCursorScreenPos(ImVec2(bb.Max.x + style.ItemInnerSpacing.x, label_y));
            ImGui::TextUnformatted(label.c_str());
        }

        return value_changed;
    }

    bool slider(float& value, float min, float max, const std::string& label, float step, bool ticks, emphasis emp) {
        return slider_impl<float>(value, min, max, label, step, ticks, emp);
    }

    bool slider(int& value, int min, int max, const std::string& label, int step, bool ticks, emphasis emp) {
        return slider_impl<int>(value, min, max, label, step, ticks, emp);
    }

    void autoscroll_input_ml(const string& id) {
        const char* child_window_name = NULL;
        ImGuiContext* g = ImGui::GetCurrentContext();
        ImFormatStringToTempBuffer(&child_window_name, NULL, "%s/%s_%08X",
            g->CurrentWindow->Name, id.c_str(), ImGui::GetID(id.c_str()));
        ImGuiWindow* child_window = ImGui::FindWindowByName(child_window_name);
        if (child_window) {
            ImGui::SetScrollY(child_window, child_window->ScrollMax.y);
        }
    }

    bool input_ml(const string& id, string& value, unsigned int line_height, bool autoscroll, bool enabled) {
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImVec2 size{ -FLT_MIN, ImGui::GetTextLineHeight() * line_height };

        if (!enabled) ImGui::BeginDisabled();

        bool ret = ImGui::InputTextMultiline(id.c_str(), &value, size, flags);

        if (!enabled) ImGui::EndDisabled();

        if (autoscroll) {
            autoscroll_input_ml(id);
        }

        return ret;
    }

    template<typename T>
    bool input_ml(const string& id, T value, int value_length, float height, bool autoscroll, bool enabled, bool use_fixed_font) {
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImVec2 size{ -FLT_MIN, height == 0 ? -FLT_MIN : height };

        if (!enabled) ImGui::BeginDisabled();

        ImFont* f = use_fixed_font ? grey::fonts::font_loader::get_fixed_size_font(scale) : nullptr;

        if (f) {
            ImGui::PushFont(f);
        }

        bool ret;
        if constexpr(std::is_same_v<T, std::string&>) {
            ret = ImGui::InputTextMultiline(id.c_str(), &value, size, flags);
        } else if constexpr(std::is_same_v<T, char*>) {
            ret = ImGui::InputTextMultiline(id.c_str(), value, value_length, size, flags);
        } else {
            ret = false;
        }

        if (f) {
            ImGui::PopFont();
        }

        if (!enabled) ImGui::EndDisabled();

        if (autoscroll) {
            autoscroll_input_ml(id);
        }

        return ret;
    }

    bool input_ml(const string& id, string& value, float height, bool autoscroll, bool enabled, bool use_fixed_font) {
        return input_ml<string&>(id, value, 0, height, autoscroll, enabled, use_fixed_font);
    }

    bool input_ml(const std::string& id, char* value, int value_length, float height, bool autoscroll, bool enabled, bool use_fixed_font) {
        return input_ml<char*>(id, value, value_length, height, autoscroll, enabled, use_fixed_font);
    }

    static ImGui::MarkdownConfig mdConfig{
        nullptr,
        nullptr,
        nullptr,
        ICON_MD_LINK
    };

    void markdown(const std::string& text) {
        // integration example: https://github.com/enkisoftware/imgui_markdown?tab=readme-ov-file#example-use-on-windows-with-links-opening-in-browser
        ImGui::Markdown(text.c_str(), text.size(), mdConfig);
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

    ImVec2 cur_get() {
        return ImGui::GetCursorScreenPos();
    }

    void cur_set(float x, float y) {
        ImGui::SetCursorScreenPos(ImVec2{x, y});
    }

    void cur_set(const ImVec2& pos) {
        ImGui::SetCursorScreenPos(pos);
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

    // ---- image ----

    void image(app& app, const std::string& key, size_t width, size_t height) {
        auto tex = app.get_texture(key);
        if (tex->data) {
            ImGui::Image((ImTextureID)tex->data, ImVec2(width, height));
        }
        else {
            ImGui::Dummy(ImVec2(width, height));
        }
    }

    void icon_image(app& app, const std::string& key) {
        float size = 16 * app.scale;
        image(app, key, size, size);
    }

    void rounded_image(app& app, const std::string& key, size_t width, size_t height, float rounding) {
        auto tex = app.get_texture(key);
        if (tex->data) {
            //ImDrawList* dl = ImGui::GetForegroundDrawList();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max = ImVec2(p_min.x + width, p_min.y + height);
            ImGui::Dummy(ImVec2(width, height));
            dl->AddImageRounded((ImTextureID)tex->data, p_min, p_max,
                ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
                rounding);
            //ImGui::SetCursorPos(p_max);
        }
    }

    bool icon_selector(app& app, const std::string& path, size_t square_size) {
        group g;
        //g.border_hover(ImGuiCol_ButtonHovered).render();

        if (path.empty()) {
            ImGui::Dummy(ImVec2(square_size, square_size));
        }
        else {
            app.preload_texture(path, path);
            rounded_image(app, path, square_size, square_size, square_size / 2);
        }
        return is_leftclicked();
    }

    // ---- spacing ----

    void spc(size_t repeat) {
        for (int i = 0; i < repeat; i++)
            ImGui::Spacing();
    }

    // ---- same line ----

    void sl(float offset) {
        ImGui::SameLine(offset);
    }

    // ---- separator ----

    void sep(const string& text) {
        if (text.empty())
            ImGui::Separator();
        else
            ImGui::SeparatorText(text.c_str());
    }

    // ---- button ----

    bool button(const std::string & text, emphasis emp, bool is_enabled, bool is_small, const string& tooltip_text, float width) {

        if (!is_enabled) {
            ImGui::BeginDisabled(true);
        }

        bool clicked;

        if (emp != emphasis::none) {
            ImVec4 normal, hovered, active;
            set_emphasis_colours(emp, normal, hovered, active);
            ImGui::PushStyleColor(ImGuiCol_Button, normal);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
        }

        if (is_small) {
            clicked = ImGui::SmallButton(text.c_str());
        }
        else {
            clicked = ImGui::Button(text.c_str(), ImVec2(width, 0));
        }

        if (emp != emphasis::none) {
            ImGui::PopStyleColor(3);
        }

        if (!is_enabled) {
            ImGui::EndDisabled();
        }

        if(!tooltip_text.empty()) {
            tt(tooltip_text);
        }

        return clicked;
    }

    bool icon_checkbox(const std::string& icon, bool& is_checked, bool reversed, const string& tooltip) {

        if (reversed ? !is_checked : is_checked) {
            ImGui::Text(icon.c_str());
        }
        else {
            ImGui::TextDisabled(icon.c_str());
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            if(!tooltip.empty()) {
                ImGui::SetTooltip("%s", tooltip.c_str());
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                is_checked = !is_checked;
                return true;
            }
        }

        return false;
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

    bool small_checkbox(const std::string& label, bool& is_checked) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        bool r = checkbox(label, is_checked);
        ImGui::PopStyleVar();
        return r;
    }

    bool icon_list(const std::vector<std::pair<std::string, string>>& options, size_t& selected) {
        bool changed{ false };
        for (int si = 0; si < options.size(); si++) {
            if (si > 0) ImGui::SameLine();
            bool is_selected = selected == si;

            if (is_selected) {
                ImGui::Text(options[si].first.c_str());
            }
            else {
                ImGui::TextDisabled(options[si].first.c_str());

                // show "hand" cursor for disabled (selectable) options
                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                    // check if mouse is clicked on this item
                    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        selected = si;
                        changed = true;
                    }
                }
            }

            if (!options[si].second.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("%s", options[si].second.c_str());
            }
        }
        return changed;
    }

    bool accordion(const std::string& header, bool default_open) {
        ImGuiTreeNodeFlags flags = 0;
        if (default_open) flags |= ImGuiTreeNodeFlags_DefaultOpen;
        return ImGui::CollapsingHeader(header.c_str(), flags);
    }

    bool combo(const string& label, const std::vector<std::string>& options, unsigned int& selected, float width) {
        bool ret{ false };

        if (width != 0) {
            width *= scale;
            ImGui::PushItemWidth(width);
        }

        if (selected >= options.size()) {
            selected = options.empty() ? 0 : options.size() - 1;
        }

        string preview_value = options.empty() ? "" : options[selected];

        if (ImGui::BeginCombo(label.c_str(), preview_value.c_str())) {
            for (size_t i = 0; i < options.size(); i++) {
                bool is_selected = selected == i;
                if (ImGui::Selectable(options[i].c_str(), is_selected)) {
                    selected = i;
                    ret = true;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        if (width != 0)
            ImGui::PopItemWidth();

        return ret;
    }

    bool list(const std::string& label, const std::vector<std::string>& options, unsigned int& selected, float width) {
        bool ret{ false };

        if (width != 0) {
            width *= scale;
            ImGui::PushItemWidth(width);
        }

        if (selected >= options.size()) {
            selected = options.empty() ? 0 : options.size() - 1;
        }

        string preview_value = options.empty() ? "" : options[selected];

        if (ImGui::BeginListBox(label.c_str())) {
            for (size_t i = 0; i < options.size(); i++) {
                bool is_selected = selected == i;
                if (ImGui::Selectable(options[i].c_str(), is_selected, is_selected ? ImGuiSelectableFlags_Highlight : 0)) {
                    selected = i;
                    ret = true;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }

        if (width != 0)
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
        ImSpinner::SpinnerHboDots("SpinnerHboDots", radius, thickness, ImSpinner::white, 0.1f, 0.5f, speed, dot_count, 0);
    }

    void notify_info(const std::string& message) {
        ImGui::InsertNotification({ ImGuiToastType::Info, 5000, message.c_str() });
    }

    void notify_render_frame() {
        // Notifications style setup
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f); // Disable round borders
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.1f); // really thin border

        // Notifications color setup
        //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f)); // Background color


        // Main rendering function
        ImGui::RenderNotifications();


        //——————————————————————————————— WARNING ———————————————————————————————
        // Argument MUST match the amount of ImGui::PushStyleVar() calls 
        //ImGui::PopStyleVar(2);
        ImGui::PopStyleVar(1);

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

    status_bar::status_bar() : style{ ImGui::GetStyle() }, cursor_before{ ImGui::GetCursorPos() } {
        auto io = ImGui::GetIO();

        float height = io.Fonts->Fonts[0]->FontSize + style.FramePadding.y * 2.0f;

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
            (ImU32)rgb_colour {
            style.Colors[ImGuiCol_MenuBarBg]
        }, style.FrameRounding);

        ImGui::SetCursorPos(cursor_before);
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

    void mouse_cursor(mouse_cursor_type mct) {
        ImGui::SetMouseCursor((ImGuiMouseCursor_)mct);
    }

    tree_node::tree_node(const std::string& label, bool open_by_default, bool is_leaf, bool span_all_cols, emphasis emp) {
        ImGuiTreeNodeFlags flags{0};
        if (open_by_default) {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }
        if (is_leaf) {
            flags |= ImGuiTreeNodeFlags_Leaf;// | ImGuiTreeNodeFlags_Bullet;
        }

        if(span_all_cols) {
            flags |= ImGuiTreeNodeFlags_SpanAllColumns;
        }

        // todo: since 1.92 trees can draw hierarchy lines

        if(emp == emphasis::none) {
            opened = ImGui::TreeNodeEx(label.c_str(), flags);
        } else {
            ImVec4 normal, hovered, active;
            if(set_emphasis_colours(emp, normal, hovered, active)) {
                ImGui::PushStyleColor(ImGuiCol_Text, normal);
                opened = ImGui::TreeNodeEx(label.c_str(), flags);
                ImGui::PopStyleColor();
            } else {
                opened = ImGui::TreeNodeEx(label.c_str(), flags);
            }
        }
    }

    tree_node::~tree_node() {
        if (opened) {
            ImGui::TreePop();
        }
    }

    /*bool tree_node(const std::string& label, ImGuiTreeNodeFlags flags, emphasis emp) {
        bool ok;
        if (emp == emphasis::none) {
            ok = ImGui::TreeNodeEx(label.c_str(), flags);
        }
        else {
            ImVec4 normal, hovered, active;
            if (set_emphasis_colours(emp, normal, hovered, active)) {
                ImGui::PushStyleColor(ImGuiCol_Text, normal);
                ok = ImGui::TreeNodeEx(label.c_str(), flags);
                ImGui::PopStyleColor();
            }
            else {
                ok = ImGui::TreeNodeEx(label.c_str(), flags);
            }
        }

        return ok;
    }*/

    // colour helpers

    ImU32 imcol32(ImGuiCol idx) {
        ImVec4 color = ImGui::GetStyle().Colors[idx]; // Retrieve the color as ImVec4
        // Convert from ImVec4 (floats) to ImU32
        return IM_COL32((int)(color.x * 255.0f), (int)(color.y * 255.0f), (int)(color.z * 255.0f), (int)(color.w * 255.0f));
    }

    void label_debug_info() {
        // FPS
        float fps = ImGui::GetIO().Framerate;
        char buf[32];
        sprintf(buf, "%.2f", fps);
        label("fps: ");
        sl();
        label(buf);

        // scale
        sl(); label(", scale: ");
        sl(); label(std::to_string(scale));
    }

    // ---- tab bar ----

    tab_bar::tab_bar(const std::string& id, bool tab_list_popup, bool scroll) {
        if (tab_list_popup)
            flags |= ImGuiTabBarFlags_TabListPopupButton;
        if (scroll)
            flags |= ImGuiTabBarFlags_FittingPolicyScroll;
        rendered = ImGui::BeginTabBar(id.c_str(), flags);
        tab_index = 0;
    }

    tab_bar::~tab_bar() {
        if (rendered) {
            ImGui::EndTabBar();
        }
    }

    tab_bar_item tab_bar::next_tab(const string& title, bool unsaved, bool selected) {
        return tab_bar_item{ title + "##" + std::to_string(tab_index++), unsaved, selected };
    }

    tab_bar_item::tab_bar_item(const std::string& id, bool unsaved, bool selected) : id{ id } {
        //cout << "tab_bar_item::tab_bar_item " << id << endl;
        if (unsaved) {
            flags |= ImGuiTabItemFlags_UnsavedDocument;
        }
        if(selected) {
            flags |= ImGuiTabItemFlags_SetSelected;
        }
        rendered = ImGui::BeginTabItem(id.c_str(), nullptr, flags);
    }

    tab_bar_item::~tab_bar_item() {
        //cout << "tab_bar_item::~tab_bar_item " << id << endl;
        if (rendered) {
            ImGui::EndTabItem();
            rendered = false;
        }
    }

    // ---- popup ----

    popup::popup(const std::string& id) : id{ id } {
    }

    void popup::enter() {

        if (do_open) {
            ImGui::OpenPopup(id.c_str());
            do_open = false;
        }

        if (rendered && open_x != 0 && open_y != 0) {
            ImGui::SetNextWindowPos(ImVec2(open_x, open_y));
        }
        rendered = ImGui::BeginPopup(id.c_str());
    }

    void popup::leave() {
        if (rendered) {
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

    text_editor::text_editor(bool border) :
        id{ generate_id("TextEditor") },
        border{ border },
        lang{ grey::thirdparty::langdefs::lua() } {
        editor.SetLanguageDefinition(lang);
        //editor.SetShowWhitespaces(true);
        editor.SetTabSize(2);
        editor.SetShowKeywordTooltips(false);
    }

    void text_editor::set_text(const std::string& text) {
        editor.SetText(text);
    }

    std::string text_editor::get_text() {
        return editor.GetText();
    }

    bool text_editor::render(float width, float height) {
        ImFont* f = fonts::font_loader::get_fixed_size_font(scale);
        if (f) {
            ImGui::PushFont(f);
        }

        editor.Render(id.c_str(), ImVec2(width, height), border);

        if (f) {
            ImGui::PopFont();
        }

        return editor.IsTextChanged();
    }

    big_table::big_table(const std::string& id, const vector<string>& columns, size_t row_count,
        float outer_width,
        float outer_height,
        bool alternate_row_bg) : columns_size{columns.size()}, outer_size{outer_width, outer_height} {
        if(alternate_row_bg) {
            flags |= ImGuiTableFlags_RowBg;
        }
        rendered = ImGui::BeginTable(id.c_str(), columns.size(), flags, outer_size);
        if (rendered) {
            ImGui::TableSetupScrollFreeze(0, 1);
            clipper.Begin(row_count);

            // setup columns
            for (const string& cn : columns) {
                if (cn.empty() || !cn.ends_with("+")) {
                    ImGui::TableSetupColumn(cn.c_str());
                }
                else {
                    string n = cn.substr(0, cn.size() - 1);
                    ImGui::TableSetupColumn(n.c_str(), ImGuiTableColumnFlags_WidthStretch);
                }
            }
            ImGui::TableHeadersRow();
        }
    }

    big_table::~big_table() {
        if (rendered) {
            ImGui::EndTable();
        }
    }

    void big_table::render_data(std::function<void(int, int)> cell_render) {
        if(!rendered) return;

        while(clipper.Step()) {
            for(int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                ImGui::TableNextRow();
                for (int col = 0; col < columns_size; col++) {
                    if(!ImGui::TableSetColumnIndex(col)) {
                        // don't bother rendering invisible columns
                        continue;
                    }

                    if (cell_render) {
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

    void plot_realtime(const string& name, scrolling_buffer& points, float x_min, float x_max, float y_min, float y_max) {
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

        if (alternate_row_bg) {
            flags |= ImGuiTableFlags_RowBg;
        }

        rendered = ImGui::BeginTable(id.c_str(), columns.size(), flags, outer_size);
        if (rendered) {
            ImGui::TableSetupScrollFreeze(0, 1);
            // setup columns
            for (const string& cn : columns) {
                if (cn.empty() || !cn.ends_with("+")) {
                    ImGui::TableSetupColumn(cn.c_str());
                }
                else {
                    string n = cn.substr(0, cn.size() - 1);
                    ImGui::TableSetupColumn(n.c_str(), ImGuiTableColumnFlags_WidthStretch);
                }
            }
            ImGui::TableHeadersRow();
        }
    }

    table::~table() {
        if (rendered) {
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
        if (rendered) {
            ImGui::EndTooltip();
        }
    }


}
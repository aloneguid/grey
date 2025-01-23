#include "widgets.h"
#include "themes.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "3rdparty/ImGuiNotify.hpp"
#include "fonts/font_loader.h"
// for Windows-specific hacks
#include <Windows.h>

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

    //inline std::string sys_label(const std::string& label) { return label + "##" + id; }

    bool set_emphasis_colours(emphasis em, ImVec4& normal, ImVec4& hovered, ImVec4& active) {
        if(em == emphasis::none) return false;

        switch(em) {
            case emphasis::primary:
                normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisPrimary];
                hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisPrimaryHovered];
                active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisPrimaryActive];
                return true;
            case emphasis::error:
                normal = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisError];
                hovered = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisErrorHovered];
                active = grey::themes::GreyColors[grey::themes::GreyCol_EmphasisErrorActive];
                return true;
        }

        return false;
    }

    // ---- window ----

    window::window(const std::string& title, bool* p_open) : title{title} {
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

        if(border_size >= 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, border_size);
        }

        // set window class to prevent viewports to be merged with main window
        //ImGui::SetNextWindowClass(&wc);

        if(init_size.x)
            ImGui::SetNextWindowSize(init_size, ImGuiCond_Once);

        if(resize_to.x) {
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
                init_center_imgui_monitor.WorkSize.x / 2 - init_size.x / 2 + init_center_imgui_monitor.WorkPos.x,
                init_center_imgui_monitor.WorkSize.y / 2 - init_size.y / 2 + init_center_imgui_monitor.WorkPos.y);
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
    }

    void window::leave() {
        if(border_size >= 0)
            ImGui::PopStyleVar();
        ImGui::End();

        if(!win32_brought_forward) {
            ImGuiViewport* vp = ImGui::GetWindowViewport();
            if(vp) {
                ::SetForegroundWindow((HWND)vp->PlatformHandleRaw);
                win32_brought_forward = true;
            }
        }

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

        ImVec2 tsz = size;

        if(size.y < 0) {
            // pad from the bottom
            ImVec2 wsz = ImGui::GetWindowSize();
            tsz = ImVec2(tsz.x, wsz.y + size.y);
        }

        ImGui::BeginChild(id.c_str(), size, flags);
    }

    void container::leave() {
        ImGui::EndChild();
    }

    bool mi(const std::string& text, bool reserve_icon_space, const std::string& icon) {
        bool r;
        const string IconedPrefix = "       ";

        if(reserve_icon_space) {
            ImVec2 cp = ImGui::GetCursorPos();
            r = ImGui::MenuItem((IconedPrefix + text).c_str());
            if(!icon.empty()) {
                ImGui::SetCursorPos(cp);
                ImGui::Text(icon.c_str());
            }
        } else {
            r = ImGui::MenuItem(text.c_str());
        }

        return r;
    }

    void mi_themes(std::function<void(const std::string&)> on_changed) {
        menu m{"Theme", true, ICON_MD_BRUSH};
        if(m) {
            ImDrawList* dl = ImGui::GetWindowDrawList();

            for(auto& theme : grey::themes::list_themes()) {

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

                if(mi(theme.name)) {
                    //grey::themes::set_theme(theme.id, scale);
                    on_changed(theme.id);
                }
            }
        }
    }

    // ---- menu_bar ----

    menu::menu(const std::string& title, bool reserve_icon_space, const std::string& icon) : icon{icon} {
        const string IconedPrefix = "       ";
        if(reserve_icon_space) {
            cp = ImGui::GetCursorPos();
            rendered = ImGui::BeginMenu((IconedPrefix + title).c_str());
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
            ImGui::Text(icon.c_str());
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

    void label(const std::string& text, size_t text_wrap_pos, bool enabled) {
        if(text_wrap_pos > 0)
            ImGui::PushTextWrapPos(text_wrap_pos);

        if(enabled)
            ImGui::TextUnformatted(text.c_str());
        else
            ImGui::TextDisabled(text.c_str());

        if(text_wrap_pos > 0)
            ImGui::PopTextWrapPos();
    }

    void label(const std::string& text, emphasis emp, size_t text_wrap_pos, bool enabled) {

        if(emp == emphasis::none || !enabled) {
            label(text, text_wrap_pos);
        } else {
            ImVec4 normal, hovered, active;
            if(set_emphasis_colours(emp, normal, hovered, active)) {
                ImGui::PushStyleColor(ImGuiCol_Text, normal);
                label(text, text_wrap_pos);
                ImGui::PopStyleColor();
            } else {
                label(text, text_wrap_pos);
            }
        }
    }

    bool input(std::string& value, const std::string& label, bool enabled, float width, bool is_readonly) {
        bool fired;
        if(!enabled) ImGui::BeginDisabled();
        if(width != 0)
            ImGui::PushItemWidth(width);

        ImGuiInputTextFlags flags{};
        if(is_readonly) flags |= ImGuiInputTextFlags_ReadOnly;
        fired = ImGui::InputText(label.c_str(), &value, flags);

        if(width != 0)
            ImGui::PopItemWidth();
        if(!enabled) ImGui::EndDisabled();
        return fired;
    }

    bool input(int& value, const std::string& label, bool enabled, float width, bool is_readonly) {
        bool fired;
        if(!enabled) ImGui::BeginDisabled();
        if(width != 0)
            ImGui::PushItemWidth(width);

        ImGuiInputTextFlags flags{};
        if(is_readonly) flags |= ImGuiInputTextFlags_ReadOnly;
        fired = ImGui::InputInt(label.c_str(), &value, 1, 100, flags);

        if(width != 0)
            ImGui::PopItemWidth();
        if(!enabled) ImGui::EndDisabled();
        return fired;
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

    bool input_ml(const string& id, string& value, float height, bool autoscroll, bool enabled) {
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImVec2 size{-FLT_MIN, height};

        if(!enabled) ImGui::BeginDisabled();

        bool ret = ImGui::InputTextMultiline(id.c_str(), &value, size, flags);

        if(!enabled) ImGui::EndDisabled();

        if(autoscroll) {
            autoscroll_input_ml(id);
        }

        return ret;
    }

    // ---- tooltip ----

    void tooltip(const std::string& text) {
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", text.c_str());
        }
    }

    // ---- position ----

    void set_pos(float x, float y) {
        if(x < 0 && y >= 0) {
            ImGui::SetCursorPosY(y);
        } else if(x >= 0 && y < 0) {
            ImGui::SetCursorPosX(x);
        } else {
            ImGui::SetCursorPos(ImVec2{x, y});
        }
    }

    void get_pos(float& x, float& y) {
        ImVec2 p = ImGui::GetCursorPos();
        x = p.x;
        y = p.y;
    }

    void move_pos(float x, float y) {
        ImVec2 mv = ImGui::GetCursorPos();
        mv.x += x;
        mv.y += y;
        ImGui::SetCursorPos(mv);
    }

    // ---- image ----

    void image(app& app, const std::string& key, size_t width, size_t height) {
        auto tex = app.get_texture(key);
        if(tex.data) {
            ImGui::Image((ImTextureID)tex.data, ImVec2(width, height));
        } else {
            ImGui::Dummy(ImVec2(width, height));
        }
    }

    void icon_image(app& app, const std::string& key) {
        float size = 16 * app.scale;
        image(app, key, size, size);
    }

    void rounded_image(app& app, const std::string& key, size_t width, size_t height, float rounding) {
        auto tex = app.get_texture(key);
        if(tex.data) {
            //ImDrawList* dl = ImGui::GetForegroundDrawList();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max = ImVec2(p_min.x + width, p_min.y + height);
            ImGui::Dummy(ImVec2(width, height));
            dl->AddImageRounded((ImTextureID)tex.data, p_min, p_max,
                ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
                rounding);
            //ImGui::SetCursorPos(p_max);
        }
    }

    bool icon_selector(app& app, const std::string& path, size_t square_size) {
        group g;
        g.border_hover(ImGuiCol_ButtonHovered).render();

        if(path.empty()) {
            ImGui::Dummy(ImVec2(square_size, square_size));
        } else {
            app.preload_texture(path, path);
            rounded_image(app, path, square_size, square_size, square_size / 2);
        }
        return is_leftclicked();
    }

    // ---- spacing ----

    void spc(size_t repeat) {
        for(int i = 0; i < repeat; i++)
            ImGui::Spacing();
    }

    // ---- same line ----

    void sl(float offset) {
        ImGui::SameLine(offset);
    }

    // ---- separator ----

    void sep(const string& text) {
        if(text.empty())
            ImGui::Separator();
        else
            ImGui::SeparatorText(text.c_str());
    }

    // ---- button ----

    bool button(const std::string& text, emphasis emp, bool is_enabled, bool is_small) {

        if(!is_enabled) {
            ImGui::BeginDisabled(true);
        }

        bool clicked;

        if(emp != emphasis::none) {
            ImVec4 normal, hovered, active;
            set_emphasis_colours(emp, normal, hovered, active);
            ImGui::PushStyleColor(ImGuiCol_Button, normal);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
        }

        if(is_small) {
            clicked = ImGui::SmallButton(text.c_str());
        } else {
            clicked = ImGui::Button(text.c_str());
        }

        if(emp != emphasis::none) {
            ImGui::PopStyleColor(3);
        }

        if(!is_enabled) {
            ImGui::EndDisabled();
        }

        return clicked;
    }

    bool icon_checkbox(const std::string& icon, bool& is_checked, bool reversed) {

        if(reversed ? !is_checked : is_checked) {
            ImGui::Text(icon.c_str());
        } else {
            ImGui::TextDisabled(icon.c_str());
        }

        if(ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                is_checked = !is_checked;
                return true;
            }
        }

        return false;
    }

    bool checkbox(const std::string& label, bool& is_checked) {
        return ImGui::Checkbox(label.c_str(), &is_checked);
    }

    bool small_checkbox(const std::string& label, bool& is_checked) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        bool r = checkbox(label, is_checked);
        ImGui::PopStyleVar();
        return r;
    }

    bool icon_list(const std::vector<std::pair<std::string, string>>& options, size_t& selected) {
        bool changed{false};
        for(int si = 0; si < options.size(); si++) {
            if(si > 0) ImGui::SameLine();
            bool is_selected = selected == si;

            if(is_selected) {
                ImGui::Text(options[si].first.c_str());
            } else {
                ImGui::TextDisabled(options[si].first.c_str());

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

    bool combo(const string& label, const std::vector<std::string>& options, size_t& selected, float width) {
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

    bool list(const std::string& label, const std::vector<std::string>& options, size_t& selected, float width) {
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
                if(ImGui::Selectable(options[i].c_str(), is_selected)) {
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

    void notify_info(const std::string& message) {
        ImGui::InsertNotification({ImGuiToastType::Info, 5000, message.c_str()});
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

    group::group() {
        
    }

    void group::render() {
        ImGui::BeginGroup();
    }

    group::~group() {

        if(full_width) {
            float max_width = ImGui::GetWindowWidth();
            ImGui::SetCursorPosX(0.0);
            ImGui::InvisibleButton("ib", ImVec2(max_width, 0.1));
            ImGui::SameLine();
        }

        ImGui::EndGroup();

        if(bdr_ci || bdr_hover_ci || bg_ci || bg_hover_ci) {
            auto& style = ImGui::GetStyle();

            auto min = ImGui::GetItemRectMin();
            auto max = ImGui::GetItemRectMax();
            ImDrawList* fdl = ImGui::GetWindowDrawList();
            ImDrawList* bdl = ImGui::GetBackgroundDrawList();

            if(bg_ci > 0) {
                bdl->AddRectFilled(min, max,
                    (ImU32)rgb_colour { style.Colors[bg_ci] },
                    style.FrameRounding);
            }

            if(bdr_ci > 0) {
                fdl->AddRect(min, max,
                    (ImU32)rgb_colour { style.Colors[bdr_ci] },
                    style.FrameRounding);
            }

            if(ImGui::IsItemHovered()) {
                if(bg_hover_ci > 0) {
                    bdl->AddRectFilled(min, max,
                        (ImU32)rgb_colour { style.Colors[bg_hover_ci] },
                        style.FrameRounding);
                }

                if(bdr_hover_ci > 0) {
                    fdl->AddRect(min, max,
                        (ImU32)rgb_colour { style.Colors[bdr_hover_ci] },
                        style.FrameRounding);
                }
            }
        }

    }

    // ---- status bar ----

    status_bar::status_bar() : style{ImGui::GetStyle()}, cursor_before{ImGui::GetCursorPos()} {
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

    bool tree_node(const std::string& label, ImGuiTreeNodeFlags flags, emphasis emp) {
        bool ok;
        if(emp == emphasis::none) {
            ok = ImGui::TreeNodeEx(label.c_str(), flags);
        } else {
            ImVec4 normal, hovered, active;
            if(set_emphasis_colours(emp, normal, hovered, active)) {
                ImGui::PushStyleColor(ImGuiCol_Text, normal);
                ok = ImGui::TreeNodeEx(label.c_str(), flags);
                ImGui::PopStyleColor();
            } else {
                ok = ImGui::TreeNodeEx(label.c_str(), flags);
            }
        }

        return ok;
    }

    // colour helpers

    ImU32 imcol32(ImGuiCol idx) {
        ImVec4 color = ImGui::GetStyle().Colors[idx]; // Retrieve the color as ImVec4
        // Convert from ImVec4 (floats) to ImU32
        return IM_COL32((int)(color.x * 255.0f), (int)(color.y * 255.0f), (int)(color.z * 255.0f), (int)(color.w * 255.0f));
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

    tab_bar_item tab_bar::next_tab(const string& title, bool unsaved) {
        return tab_bar_item{title + "##" + std::to_string(tab_index++), unsaved};
    }

    tab_bar_item::tab_bar_item(const std::string& id, bool unsaved) : id{id} {
        if(unsaved) {
            flags |= ImGuiTabItemFlags_UnsavedDocument;
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

    popup::popup(const std::string& id) : id{id} {
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

    text_editor::text_editor(bool border) : 
        id{generate_id("TextEditor")},
        border{border},
        lang{grey::thirdparty::langdefs::lua()} {
        editor.SetLanguageDefinition(lang);
    }

    void text_editor::set_text(const std::string& text) {
        editor.SetText(text);
    }

    std::string text_editor::get_text() {
        return editor.GetText();
    }

    bool text_editor::render(float width, float height) {
        ImFont* f = fonts::font_loader::get_fixed_size_font(scale);
        if(f) {
            ImGui::PushFont(f);
        }

        editor.Render(id.c_str(), ImVec2(width, height), border);

        if(f) {
            ImGui::PopFont();
        }
        
        return editor.IsTextChanged();
    }
}
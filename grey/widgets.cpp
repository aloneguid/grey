#include "widgets.h"
#include "themes.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

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

    window& window::has_menubar() {
        flags |= ImGuiWindowFlags_MenuBar;
        return *this;
    }

    window& window::no_resize() {
        flags |= ImGuiWindowFlags_NoResize;
        return *this;
    }

    window& window::no_focus() {
        flags |= ImGuiWindowFlags_NoFocusOnAppearing;
        return *this;
    }

    window& window::no_scroll() {
        flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        return *this;
    }

    window& window::center(void* monitor_handle) {
        init_center_monitor = monitor_handle;
        capture_size = true;
        init_center = true;

        return *this;
    }

    void window::enter() {
        ImGui::SetNextWindowBgAlpha(1.0f);

        // set window class to prevent viewports to be merged with main window
        ImGui::SetNextWindowClass(&wc);

        if(!rendered_once) {
            if(init_size.x && init_size.y) {
                ImGui::SetNextWindowSize(init_size, ImGuiCond_Once);
            }
            rendered_once = true;
        }

        if(init_center && !init_center_done && size_in.x && size_in.y) {

            ImVector<ImGuiPlatformMonitor> monitors = ImGui::GetPlatformIO().Monitors;
            size_t midx = 0;
            for(size_t i = 0; i < monitors.Size; i++) {
                if(monitors[i].PlatformHandle == init_center_monitor) {
                    midx = i;
                    break;
                }
            }

            ImGuiPlatformMonitor monitor = monitors[midx];

            auto pos = ImVec2(
                monitor.WorkSize.x / 2 - size_in.x / 2 + monitor.WorkPos.x,
                monitor.WorkSize.y / 2 - size_in.y / 2 + monitor.WorkPos.y);
            ImGui::SetNextWindowPos(pos);

            init_center_done = true;
        }

        ImGui::Begin(title.c_str(), p_open, flags);

        if(capture_size)
            size_in = ImGui::GetWindowSize();
    }

    void window::leave() {
        ImGui::End();
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

        ImGui::BeginChild(id.c_str(), size, has_border ? ImGuiChildFlags_Border : 0);
    }

    void container::leave() {
        ImGui::EndChild();
    }

    // ---- menu_bar ----

    vector<menu_item> menu_item::make_ui_theme_items() {

        vector<menu_item> items;

        for(auto& theme : grey::themes::list_themes()) {
            items.push_back({string{"set_theme_"} + theme.id, theme.name});
        }

        return items;
    }

    menu_bar::menu_bar() {
        rendered = ImGui::BeginMenuBar();
    }

    menu_bar::menu_bar(const std::vector<menu_item>& items, std::function<void(const std::string&)> clicked) 
        : menu_bar::menu_bar() {
        if(rendered) {
            render(items, clicked);
        }
    }

    void menu_bar::render(const std::vector<menu_item>& items, std::function<void(const std::string&)> clicked) {
        if(rendered) {

            bool has_icon = false;
            for(auto& item : items) {
                if(!item.icon.empty()) {
                    has_icon = true;
                    break;
                }
            }

            for(auto& item : items) {

                if(item.text == "-") {
                    ImGui::Separator();
                    continue;
                }

                string label = has_icon
                    ? string{"       "} + item.text
                    : item.text;

                ImVec2 cp = ImGui::GetCursorPos();

                if(item.children.empty()) {
                    if(ImGui::MenuItem(label.c_str())) {
                        if(clicked) {
                            clicked(item.id);
                        }
                    }
                } else {
                    if(ImGui::BeginMenu(label.c_str())) {

                        render(item.children, clicked);

                        ImGui::EndMenu();
                    }
                }

                if(!item.icon.empty()) {
                    ImGui::SetCursorPos(cp);
                    ImGui::Text(item.icon.c_str());
                }
            }
        }
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
            ImGui::Text(text.c_str());
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
            ImGui::Image(tex.data, ImVec2(width, height));
        }
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

    bool icon_checkbox(const std::string& icon, bool is_checked) {
        if(is_checked) {
            ImGui::Text(icon.c_str());
        } else {
            ImGui::TextDisabled(icon.c_str());
        }

        if(ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                return !is_checked;
            }
        }

        return is_checked;
    }

    void icon_list(const std::vector<std::pair<std::string, string>>& options, size_t& selected) {
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
                    }
                }
            }

            if(!options[si].second.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("%s", options[si].second.c_str());
            }
        }
    }

    bool accordion(const std::string& header) {
        return ImGui::CollapsingHeader(header.c_str());
    }

    void combo(const string& label, const std::vector<std::string>& options, size_t& selected, float width) {

        if(width != 0)
            ImGui::PushItemWidth(width);

        if(ImGui::BeginCombo(label.c_str(), options[selected].c_str())) {
            for(size_t i = 0; i < options.size(); i++) {
                bool is_selected = selected == i;
                if(ImGui::Selectable(options[i].c_str(), is_selected)) {
                    selected = i;
                }

                if(is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        if(width != 0)
            ImGui::PopItemWidth();
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

    // ---- tab bar ----

    tab_bar::tab_bar(const std::string& id) {
        rendered = ImGui::BeginTabBar(id.c_str(), flags);
        tab_index = 0;
    }

    tab_bar::~tab_bar() {
        if(rendered) {
            ImGui::EndTabBar();
        }
    }

    tab_bar_item tab_bar::next_tab(const string& title) {
        return tab_bar_item{title + "##" + std::to_string(tab_index++)};
    }

    tab_bar_item::tab_bar_item(const std::string& id) : id{id} {
        rendered = ImGui::BeginTabItem(id.c_str());
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

#ifdef GREY_INCLUDE_IMNODES

    // ---- ImNodes ----

    node_editor::node_editor() {
        //config.SettingsFile = nullptr;
        context = ed::CreateEditor(&config);
    }

    node_editor::~node_editor() {
        ed::DestroyEditor(context);
    }

    void node_editor::enter() {
        ed::SetCurrentEditor(context);
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    }

    void node_editor::leave() {
        ed::End();
        ed::SetCurrentEditor(nullptr);
    }

    node_editor_node::node_editor_node(int id) {
        ed::BeginNode(id);
    }

    node_editor_node::~node_editor_node() {
        ed::EndNode();
    }

#endif

}
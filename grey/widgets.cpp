#include "widgets.h"
#include "themes.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

using namespace std;

namespace grey::widgets {

    // ---- general ----

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

    window::window(const std::string& title, bool* p_open) {
        this->title = title;
        this->p_open = p_open;
    }

    window& window::size(int width, int height, float scale) {
        ImGui::SetNextWindowSize(ImVec2(width * scale, height * scale), ImGuiCond_Once);
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

    window& window::fullscreen() {

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        return *this;
    }

    void window::render() {
        rendered = true;
        ImGui::SetNextWindowBgAlpha(1.0f);

        // set window class to prevent viewports to be merged with main window
        ImGuiWindowClass wc;
        wc.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;

        ImGui::Begin(title.c_str(), p_open, flags);
    }

    window::~window() {
        ImGui::End();
    }

    // ---- container ----

    container::container(float width, float height) : id{generate_id()}, size{width, height} {
    }

    container::container(const std::string& id) : id{id}, size{0, 0} {
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

    void input(std::string& value, const std::string& label, bool enabled, float width) {
        if(!enabled) ImGui::BeginDisabled();
        if(width != 0)
            ImGui::PushItemWidth(width);
        ImGui::InputText(label.c_str(), &value);
        if(width != 0)
            ImGui::PopItemWidth();
        if(!enabled) ImGui::EndDisabled();
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

    void sl() {
        ImGui::SameLine();
    }

    // ---- separator ----

    void sep() {
        ImGui::Separator();
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

    status_bar::status_bar() {
        // source: https://github.com/ocornut/imgui/issues/3518#issuecomment-807398290
        ImGuiViewport* win_vp = ImGui::GetWindowViewport();
        float height = ImGui::GetFrameHeight();
        ImGuiWindowFlags flags{ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar};

        if(ImGui::BeginViewportSideBar("##MainStatusBar", win_vp, ImGuiDir_Down, height, flags)) {
            rendered = ImGui::BeginMenuBar();
        }
    }

    status_bar::~status_bar() {
        if(rendered) ImGui::EndMenuBar();
        ImGui::End(); // status bar, should be outside of BeginViewportSideBar just like for a normal window
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

}
#include "widgets.h"
#include "themes.h"

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

        if(border_colour_index > 0) {
            auto& style = ImGui::GetStyle();

            auto min = ImGui::GetItemRectMin();
            auto max = ImGui::GetItemRectMax();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            draw_list->AddRect(min, max,
                (ImU32)rgb_colour { style.Colors[border_colour_index] },
                style.FrameRounding);
        }

        if(border_hover_colour_index && ImGui::IsItemHovered()) {
            auto& style = ImGui::GetStyle();

            auto min = ImGui::GetItemRectMin();
            auto max = ImGui::GetItemRectMax();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            draw_list->AddRect(min, max,
                (ImU32)rgb_colour { style.Colors[border_hover_colour_index] },
                style.FrameRounding);
        }
    }
}
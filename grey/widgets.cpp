#include "widgets.h"
#include "themes.h"

using namespace std;

namespace grey::widgets {

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

    void label(const std::string& text, size_t text_wrap_pos) {
        if(text_wrap_pos > 0)
            ImGui::PushTextWrapPos(text_wrap_pos);

        ImGui::Text(text.c_str());

        if(text_wrap_pos > 0)
            ImGui::PopTextWrapPos();
    }

    void tooltip(const std::string& text) {
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", text.c_str());
        }
    }

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

    void image(app& app, const std::string& key, size_t width, size_t height) {
        auto tex = app.get_texture(key);
        if(tex.data) {
            ImGui::Image(tex.data, ImVec2(width, height));
        }
    }

    void sp(size_t repeat) {
        for(int i = 0; i < repeat; i++)
            ImGui::Spacing();
    }

    void sl() {
        ImGui::SameLine();
    }
}
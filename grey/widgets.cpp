#include "widgets.h"

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

    void label(const std::string& text) {
        ImGui::Text(text.c_str());
    }
}
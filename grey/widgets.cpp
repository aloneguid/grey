#include "widgets.h"

using namespace std;

namespace grey::widgets {
    window::window(const std::string& title, bool has_menubar, bool can_resize, int width, int height) {

        if(width != 0 && height != 0) {
            ImGui::SetNextWindowSize(ImVec2(width, height));
        }

        ImGuiWindowFlags flags = 0;
        if(has_menubar)
            flags |= ImGuiWindowFlags_MenuBar;
        if(!can_resize)
            flags |= ImGuiWindowFlags_NoResize;
        ImGui::Begin(title.c_str(), nullptr, flags);

    }

    window::~window() {
        ImGui::End();
    }

    menu_bar::menu_bar() {
        rendered = ImGui::BeginMenuBar();
    }

    menu_bar::~menu_bar() {
        if(rendered) {
            ImGui::EndMenuBar();
        }
    }

    void label(const std::string& text) {
        ImGui::Text(text.c_str());
    }
}
#include "widgets.h"

using namespace std;

namespace grey {
    window::window(const std::string& title, bool has_menubar, bool can_resize) {
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

    void label(const std::string& text) {
        ImGui::Text(text.c_str());
    }
}
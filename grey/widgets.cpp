#include "widgets.h"

using namespace std;

namespace grey::widgets {
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
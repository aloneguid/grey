#pragma once
#include "imgui.h"
#include "fonts/font_awesome_6.h"
#include "fonts/font_awesome_6_brands.h"
#include <string>

namespace grey::widgets {
    class window {
    public:
        window(const std::string& title, bool* p_open = nullptr);

        window& size(int width, int height, float scale);
        window& has_menubar();
        window& fullscreen();
        window& no_resize();
        window& no_focus();

        void render();

        ~window();

    private:
        std::string title;
        bool* p_open{nullptr};
        ImGuiWindowFlags flags{0};
        bool rendered{false};
    };

    class menu_bar {
    public:
        menu_bar();
        ~menu_bar();

        operator bool() const {
            return rendered;
        }

    private:
        bool rendered{false};
    };

    void label(const std::string& text);

}
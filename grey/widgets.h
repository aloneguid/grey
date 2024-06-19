#pragma once
#include "imgui.h"
#include "fonts/font_awesome_6.h"
#include "fonts/font_awesome_6_brands.h"
#include <string>

namespace grey::widgets {
    class window {
    public:
        window(const std::string& title, bool has_menubar = false, bool can_resize = true,
            int width = 0, int height = 0);
        ~window();
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
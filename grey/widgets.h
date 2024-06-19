#pragma once
#include "imgui.h"
#include "fonts/font_awesome_6.h"
#include "fonts/font_awesome_6_brands.h"
#include <string>

namespace grey {
    class window {
    public:
        window(const std::string& title, bool has_menubar = false, bool can_resize = true);
        ~window();
    };

    void label(const std::string& text);

}
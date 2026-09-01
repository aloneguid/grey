#pragma once
#include "imgui.h"
#include "../model.h"

namespace grey::fonts {
    class font_loader {
    public:
        static void preload_fonts(const font_config& cfg);
        static ImFont* get_font(font_weight weight);

    private:
        static ImFont* font_fixed;
        static ImFont* font_bold;
    };
}
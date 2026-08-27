#pragma once

#include "imgui.h"
#include "model.h"

namespace grey::fonts {
    class font_loader {
    public:
        static void load_font(
            bool load_fa = true,
            bool load_fixed = false,
            bool load_bold = false);
        static ImFont* get_fixed_size_font(float scale);
        static ImFont* get_font(font_weight weight);

    private:
        static ImFont* fixed_size_font;
        static ImFont* bold_font;
        static void load_system_fonts(ImGuiIO& io,
            ImFont** system_font,
            bool load_bold_font, ImFont** bold_font);
        static ImFont* load_fixed_font(ImGuiIO& io);
    };
}
#pragma once

#include "imgui.h"

namespace grey::fonts {
    class font_loader {
    public:
        static void load_font(float scale, bool load_fa = true, bool load_fixed = false);
        static ImFont* get_fixed_size_font(float scale);

    private:
        static ImFont* fixed_size_font;
        static ImFont* load_system_font(ImGuiIO& io, float scale);
        static ImFont* load_fixed_font(ImGuiIO& io, float scale);
    };
}
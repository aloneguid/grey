#include "app.h"
#include "backends/win32dx11app.hpp"
#include "fonts/font_loader.hpp"
#include "themes.hpp"

using namespace std;

namespace grey {
    std::unique_ptr<grey::app> app::make(const string& title) {
        auto app = make_unique<grey::backends::win32dx11app>(title);

        return app;
    }

    app::app() {
        int dpi = grey::common::win32::os::get_dpi();
        scale = dpi / 96.f;
    }

    void app::on_after_initialised() {
        // disable built-in .ini file creation
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        // apply scaling settings
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(scale);
        // don't apply font global scaling, they will look terrible on high DPI.
        // instead, when loading a font, apply scaling factor
        //io.FontGlobalScale = scale;
        io.DisplayFramebufferScale = {scale, scale};

        grey::themes::set_theme("dark");

        grey::load_font(scale);
    }
}
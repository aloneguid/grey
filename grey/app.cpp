#include "app.h"
#include "backends/win32dx11app.hpp"
#include "fonts/font_loader.hpp"
#include "themes.h"
#include "widgets.h"

using namespace std;

namespace grey {
    std::unique_ptr<grey::app> app::make(const string& title, int width, int height) {
        auto app = make_unique<grey::backends::win32dx11app>(title, width, height);

        return app;
    }

    app::app() {
        int dpi = grey::common::win32::os::get_dpi();
        scale = dpi / 96.f;
        grey::widgets::scale = scale;
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

        string theme_id = initial_theme_id.empty() ? grey::themes::FollowOsThemeId : initial_theme_id;
        set_theme(theme_id);

        grey::load_font(scale, load_icon_font);

        if(on_initialised)
            on_initialised();
    }

    void app::set_theme(const std::string& theme_id) {
        auto theme = grey::themes::get_theme(theme_id);
        grey::themes::set_theme(theme_id, scale);
        set_dark_mode(theme.is_dark);
    }

    texture app::get_texture(const std::string& key) {
        auto entry = textures.find(key);
        if(entry == textures.end()) {
            return {nullptr, 0, 0};
        }
        return entry->second;
    }

    bool app::preload_texture(const std::string& key, unsigned char* buffer, unsigned int len) {
        auto entry = textures.find(key);
        if(entry != textures.end()) {
            return true;
        }
        grey::common::raw_img img_data = grey::common::load_image_from_memory(buffer, len);
        if(!img_data) return false;

        void* native_texture = make_native_texture(img_data);
        if(!native_texture) return false;
        textures[key] = texture{native_texture, img_data.x, img_data.y};

        return true;
    }

    bool app::preload_texture(const std::string& key, const std::string& path) {
        auto entry = textures.find(key);
        if(entry != textures.end()) {
            return true;
        }

        grey::common::raw_img img_data = grey::common::load_image_from_file(path);
        if(!img_data) return false;

        void* native_texture = make_native_texture(img_data);
        if(!native_texture) return false;
        textures[key] = texture{native_texture, img_data.x, img_data.y};

        return false;
    }
}
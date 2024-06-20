#include "app.h"
#include "backends/win32dx11app.hpp"
#include "fonts/font_loader.hpp"
#include "themes.h"

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

        grey::themes::set_theme(grey::themes::FollowOsThemeName, scale);

        grey::load_font(scale);
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
#include "app.h"

#include <imgui_internal.h>

#include "fonts/font_loader.h"
#include "themes.h"
#include "widgets.h"
#include "common/os.h"
#include <thread>
#include <utility>

#if PLATFORM_WINDOWS
#include "backends/win32_dx11_app.hpp"
#elif PLATFORM_LINUX
#include "backends/glfw_opengl3_app.hpp"
#elif PLATFORM_MACOS
#include "backends/glfw_metal_app.mm"
#endif

using namespace std;

namespace grey {
    std::unique_ptr<app> app::make(const std::string& title, sz size) {

#if PLATFORM_WINDOWS
        auto app = make_unique<backends::win32_dx11_app>(title, size);
#elif PLATFORM_LINUX
        auto app = make_unique<backends::glfw_gl3_app>(title, size);
#elif PLATFORM_MACOS
        auto app = make_unique<backends::glfw_metal_app>(title, size);
#endif

        return app;
    }

    app::app(std::string title, sz initial_size) : title{std::move(title)}, initial_size{initial_size}  {
        wnd_main_opts.open_ptr = &wnd_main_is_open;
    }

    void app::on_after_initialised() {
        ImGuiIO& io = ImGui::GetIO();

        // disable built-in .ini file creation
        io.IniFilename = nullptr;

        std::string theme_id = initial_theme_id.empty() ? grey::themes::FollowOsThemeId : initial_theme_id;
        set_theme(theme_id);

        fonts::font_loader::preload_fonts(fonts);

        if(on_initialised)
            on_initialised();
    }

    void app::fps_pause() const {
        if(fps < 0.0f) return;

        // can't use ImGui::GetIO().DeltaTime for this, because it would include the pause we added on

        static auto last_frame_time = std::chrono::steady_clock::now();
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> elapsed = now - last_frame_time;

        const float target_frame_delta_sec = 1.0f / fps; // do not cache, fps can change
        const float sleep_time_sec = target_frame_delta_sec - elapsed.count();

        if (sleep_time_sec > 0.0f) {
            std::this_thread::sleep_for(std::chrono::duration<float>(sleep_time_sec));
        }

        last_frame_time = std::chrono::steady_clock::now();
    }

    bool app::render_main_window(const std::function<bool()>& render_frame) {
        widgets::wnd wnd_main{title, wnd_main_opts};

        if(wnd_main) {
            wnd_main_is_open = render_frame();
        }

        return wnd_main_is_open;
    }

    void app::set_theme(const std::string& theme_id) {
        auto theme = themes::get_theme(theme_id);
        themes::set_theme(theme_id, widgets::main_scale);
        set_dark_mode(theme.is_dark);
    }

    std::array<float, 4> app::get_clear_color() const {
        float alpha = use_transparency_colour_key_value ? 0.0f : ClearColor[3];
        return { ClearColor[0], ClearColor[1], ClearColor[2], alpha };
    }

    shared_ptr<texture> app::get_texture(const std::string& key) {
        auto entry = textures.find(key);
        if(entry == textures.end()) {
            return nullptr;
        }
        return entry->second;
    }

    bool app::preload_texture(const std::string& key, const unsigned char* buffer, unsigned int len) {
        auto entry = textures.find(key);
        if(entry != textures.end()) {
            return true;
        }
        grey::common::raw_img img_data = grey::common::load_image_from_memory(buffer, len);
        if(!img_data) return false;

        std::shared_ptr<texture> native_texture = make_native_texture(img_data);
        if(!native_texture) return false;

        native_texture->size.width = img_data.x;
        native_texture->size.height = img_data.y;
        textures[key] = native_texture;

        return true;
    }

    bool app::preload_texture(const std::string& key, const std::string& path) {
        auto entry = textures.find(key);
        if(entry != textures.end()) {
            return true;
        }

        grey::common::raw_img img_data = common::load_image_from_file(path);
        if(!img_data) return false;

        std::shared_ptr<texture> native_texture = make_native_texture(img_data);
        if(!native_texture) return false;

        native_texture->size.width = img_data.x;
        native_texture->size.height = img_data.y;
        textures[key] = native_texture;


        return true;
    }

    bool app::release_texture(const std::string& key) {
        return textures.erase(key) > 0;
    }
}
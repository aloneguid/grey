#include "app.h"
#include "fonts/font_loader.h"
#include "themes.h"
#include "widgets.h"
#include "common/os.h"
#include <thread>

#if PLATFORM_WINDOWS
#include "backends/win32_dx11_app.hpp"
#elif PLATFORM_LINUX
#include "backends/glfw_opengl3_app.hpp"
#elif PLATFORM_MACOS
#include "backends/glfw_metal_app.hpp"
#endif

using namespace std;

namespace grey {
    std::unique_ptr<app> app::make(const string& title, sz size) {

#if PLATFORM_WINDOWS
        auto app = make_unique<backends::win32_dx11_app>(title, size);
#elif PLATFORM_LINUX
        auto app = make_unique<backends::glfw_gl3_app>(title, size);
#elif PLATFORM_MACOS
        auto app = make_unique<backends::glfw_metal_app>(title, size);
#endif

        return app;
    }

    app::app(const string& title) : wnd_main{title, &wnd_main_is_open} {

        // main window fills entire viewport, therefore remove any decorations
        wnd_main
            .no_title_bar()
            .border(0)
            .fill_viewport();
    }

    void app::on_after_initialised() {
        ImGuiIO& io = ImGui::GetIO();

        // disable built-in .ini file creation
        io.IniFilename = nullptr;

        string theme_id = initial_theme_id.empty() ? grey::themes::FollowOsThemeId : initial_theme_id;
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
        widgets::guard g{wnd_main};

        wnd_main_is_open = render_frame();

        return wnd_main_is_open;
    }

    void app::set_theme(const std::string& theme_id) {
        auto theme = themes::get_theme(theme_id);
        themes::set_theme(theme_id, widgets::scale);
        set_dark_mode(theme.is_dark);
    }

    std::array<float, 4> app::get_clear_color() const {
        float alpha = use_transparency_colour_key_value ? 0.0f : ClearColor[3];
        return { ClearColor[0], ClearColor[1], ClearColor[2], alpha };
    }

    int app::find_monitor_for_main_viewport() {
        ImGuiViewport* vp = ImGui::GetMainViewport();
        if (!vp) return -1;
        const ImVec2 v_min = vp->Pos;
        const ImVec2 v_max = ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y);

        const ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
        if (pio.Monitors.empty()) return -1;

        int best = 0;
        float best_area = -1.0f;

        for (int i = 0; i < pio.Monitors.Size; ++i) {
            const ImGuiPlatformMonitor& m = pio.Monitors[i];
            const ImVec2 m_min = m.MainPos;
            const ImVec2 m_max = ImVec2(m.MainPos.x + m.MainSize.x, m.MainPos.y + m.MainSize.y);

            const float ix_min = (v_min.x > m_min.x ? v_min.x : m_min.x);
            const float iy_min = (v_min.y > m_min.y ? v_min.y : m_min.y);
            const float ix_max = (v_max.x < m_max.x ? v_max.x : m_max.x);
            const float iy_max = (v_max.y < m_max.y ? v_max.y : m_max.y);

            const float iw = ix_max - ix_min;
            const float ih = iy_max - iy_min;
            const float area = (iw > 0.0f && ih > 0.0f) ? (iw * ih) : 0.0f;

            if (area > best_area) {
                best_area = area;
                best = i;
            }
        }
        return best;
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

        native_texture->width = img_data.x;
        native_texture->height = img_data.y;
        textures[key] = native_texture;

        return true;
    }

    bool app::preload_texture(const std::string& key, const std::string& path) {
        auto entry = textures.find(key);
        if(entry != textures.end()) {
            return true;
        }

        grey::common::raw_img img_data = grey::common::load_image_from_file(path);
        if(!img_data) return false;

        std::shared_ptr<texture> native_texture = make_native_texture(img_data);
        if(!native_texture) return false;

        native_texture->width = img_data.x;
        native_texture->height = img_data.y;
        textures[key] = native_texture;


        return true;
    }

    bool app::release_texture(const std::string& key) {
        return textures.erase(key) > 0;
    }
}
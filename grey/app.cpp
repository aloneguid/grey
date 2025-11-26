#include "app.h"
#include "fonts/font_loader.h"
#include "themes.h"
#include "widgets.h"

#if _WIN32
#include "backends/win32dx11app.hpp"
#include "common/win32/os.h"
#elif defined(__APPLE__)
// Factory function for Metal backend (defined in glfw_metal.mm)
std::unique_ptr<grey::app> create_glfw_metal_app(const std::string& title, int width, int height, float scale);
#else
#include "backends/glfw_gl3.hpp"
#endif

using namespace std;

namespace grey {
    std::unique_ptr<grey::app> app::make(const string& title, int width, int height, float scale) {

#if _WIN32
        auto app = make_unique<grey::backends::win32dx11app>(title, width, height, scale);
#elif defined(__APPLE__)
        auto app = create_glfw_metal_app(title, width, height, scale);
#else
        auto app = make_unique<grey::backends::glfw_gl3_app>(title, width, height, scale);
#endif

        return app;
    }

    app::app(float scale) {
#if _WIN32
        if (scale == 0.0f) {
            int dpi = grey::common::win32::os::get_dpi();
            scale = dpi / 96.f;
        }
#else
        if (scale == 0.0f)
            scale = 1.0f;
#endif
        this->scale = scale;

        grey::widgets::scale = scale;

        set_target_fps(40);
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

        grey::fonts::font_loader::load_font(scale, load_icon_font, load_fixed_font);

        if(on_initialised)
            on_initialised();
    }

    void app::set_theme(const std::string& theme_id) {
        auto theme = grey::themes::get_theme(theme_id);
        grey::themes::set_theme(theme_id, scale);
        set_dark_mode(theme.is_dark);
    }

    void app::set_target_fps(int fps) {
        max_frame_interval_ms = 1000.0f / fps;
    }

    std::array<float, 4> app::get_clear_color() const {
        return { ClearColor[0], ClearColor[1], ClearColor[2], ClearColor[3] };
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
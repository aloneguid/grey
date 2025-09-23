#pragma once
#include <memory>
#include <string>
#include <functional>
#include <map>
#include "common/img.h"

namespace grey {

    struct texture {
        void* data;
        size_t width;
        size_t height;
    };

    class app {
    public:
        /**
         * @brief Creates app instance, which will be different implementation depending on platform we run on.
         *        Only one instance of the app should be created per process lifetime.
         * @param title 
         * @return 
         */
        static std::unique_ptr<app> make(const std::string& title, int width, int height, float scale = 0.0f);

        /**
         * @brief When set, application will set this theme on startup.
         */
        std::string initial_theme_id;

        /**
         * @brief When true, loads icon font on startup. You can set this to "false" after creating the app instance
         *        (but before running it) to save some memory.
         */
        bool load_icon_font{true};

        bool load_fixed_font{false};

        float scale{1.0f};

        std::function<void()> on_initialised;

        std::function<void(int, const std::string&)> on_user_message;

        app(float scale = 0.0f);

        virtual void run(std::function<bool(const app&)> render_frame) = 0;

        texture get_texture(const std::string& key);

        bool preload_texture(const std::string& key, unsigned char* buffer, unsigned int len);

        bool preload_texture(const std::string& key, const std::string& path);

        /**
         * @brief Use this to set theme for the application. This will also apply scaling factor to the theme and OS specific customisations if needed.
         * @param theme_id 
         */
        void set_theme(const std::string& theme_id);


        /**
         * @brief Resizes the main viewport of the application. This is the area where the application renders its main content. The width and height parameters will be multiplied by the scale factor.
         * @param width 
         * @param height 
         */
        virtual void resize_main_viewport(int width, int height) = 0;

        /**
         * @brief Limits maximum FPS for the application. This is useful when you want to limit the CPU usage of the application.
         * @param fps 
         */
        void set_target_fps(int fps);

        /**
         * @brief Returns the clear color of the application as RGBA array of floats (0-1).
         * @return 
         */
        std::array<float, 4> get_clear_color() const;

        /**
         * @brief Find the monitor with the largest overlap with the given viewport.
         * @param vp 
         * @return Monitor index, or -1 if no monitors are found.
         */
        int find_monitor_for_main_viewport();

        // platform specific flags

#if _WIN32
        /**
         * @brief When set to true, will enable dark mode for the application on Windows 10/11.
         */
        bool win32_title_bar{true};
        bool win32_can_resize{true};
        bool win32_center_on_screen{false};
        bool win32_transparent{false};
        std::string win32_window_class_name{"GreyDX11"};

        /**
         * @brief When window loses focus, it will be closed. This is useful for modal windows. Needs to be set before application starts up to take effect.
         */
        bool win32_close_on_focus_lost{false};

        /**
         * @brief When set to true, window will be always on top.
         */
        bool win32_always_on_top{false};
#endif

    protected:

        // clear color as RGBA array of floats (0-1)
        const float ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.00f};

        void on_after_initialised();

        virtual void* make_native_texture(grey::common::raw_img& img) = 0;

        /**
         * @brief Hints if dark mode should be enabled for this application on the OS level. For instance, on Windows 10/11 dark mode will paint window chrome in dark color.
         * @param enabled
         */
        virtual void set_dark_mode(bool enabled) = 0;

        float max_frame_interval_ms;

    private:
        std::map<std::string, texture> textures;
    };
}
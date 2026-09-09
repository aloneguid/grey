#pragma once
#include <memory>
#include <string>
#include <functional>
#include <map>
#include "common/img.h"
#include "common/platform.h"
#include "model.h"
#include "widgets.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace grey {

    class app : public texture_loader {
    public:
        explicit app(const std::string& title);
        virtual ~app() = default;

        /**
         * @brief Creates app instance, which will be different implementation depending on the platform we run on.
         *        Only one instance of the app should be created per process lifetime.
         * @param title Application title, affects what is displayed on the taskbar and window title in your OS.
         * @param size Main window size in logical pixels.
         * @return 
         */
        static std::unique_ptr<app> make(const std::string& title, sz size = sz{-1, -1});

        widgets::window& main_window() { return wnd_main; }

        /**
         * @brief When set, application will set this theme on startup.
         */
        std::string initial_theme_id;

        /**
         * tells which fonts to load on startup. By default, only the system font is loaded.
         */
        font_config fonts{};

        /**
         * Called after UI framework is initialised. Useful for loading assets, etc.
         */
        std::function<void()> on_initialised;

        /**
         * Used for IPC on Windows. Do not use.
         */
        std::function<void(int, const std::string&)> on_user_message;

        /**
         * Call to start the application. This will block until the application is closed.
         * @param render_frame Callback that will be called to render a frame. Return true to continue rendering, false to exit.
         */
        virtual void run(std::function<bool()> render_frame) = 0;

        std::shared_ptr<texture> get_texture(const std::string& key) override;

        bool preload_texture(const std::string& key, const unsigned char* buffer, unsigned int len) override;

        bool preload_texture(const std::string& key, const std::string& path) override;

        /**
         * @brief Releases texture from memory
         * @param key 
         * @return True, if texture was found and released, false if texture with this key was not found.
         */
        bool release_texture(const std::string& key);

        /**
         * @brief Use this to set theme for the application. This will also apply scaling factor to the theme and OS specific customisations if needed.
         * @param theme_id 
         */
        void set_theme(const std::string& theme_id);


        /**
         * @brief Resizes the main viewport of the application. This is the area where the application renders its main content.
         * @param size Logical size.
         */
        virtual void resize(sz size) = 0;

        /**
         * @brief Moves the main viewport of the application to the specified position on the screen. This is monitor/platform-dependent and may not work on all platforms.
         * @param pos Logical position.
         */
        virtual void move(point pos) = 0;

        /**
         * @brief Brings the main viewport of the application to the foreground.
         */
        virtual void foreground() = 0;

        /**
         * Target rendering FPS. Set to -1 to disable FPS control (default). Fractional FPS values are supported and useful for application idling.
         */
        float fps{-1};

        /**
         * @brief Returns the clear color of the application as the RGBA array of floats (0-1).
         * @return 
         */
        [[nodiscard]] std::array<float, 4> get_clear_color() const;

        /**
         * @brief Find the monitor with the largest overlap with the given viewport.
         * @return Monitor index, or -1 if no monitors are found.
         */
        int find_monitor_for_main_viewport();

        /**
         * @brief When set to true, will allow the window to be resized.
         */
        bool can_resize{true};

        /**
         * @brief When set to true, will center the window on the screen where mouse is currently located.
         */
        bool center_on_screen{false};

        /**
         * When set (default) will show native window manager's title bar, otherwise nothing.
         */
        bool show_title_bar{true};

        /**
         * When set, will keep the window always on top of other windows.
         */
        bool always_on_top{false};

        // transparency

        /**
         * @brief If set to true, clear colour will be fully transparent
         */
        bool use_transparency_colour_key_value{false};

        /**
         * @brief If less than 255, this value will be used as the alpha value for the entire window, making it semi-transparent.
         */
        int transparency_window_alpha{255};

        // platform-specific flags

#if PLATFORM_WINDOWS

        bool win32_hide_from_taskbar{false};

        /**
         * @brief sets WS_EX_NOACTIVATE on the window (if you need to create a tool window that does not take focus, useful for notification windows)
         */
        bool win32_no_activate{false};
        std::string win32_window_class_name{"GreyDX11"};

        /**
         * @brief When the window loses focus, it will be closed. This is useful for modal windows. Needs to be set before application starts up to take effect.
         */
        bool win32_close_on_focus_lost{false};


        /**
         * @brief When set to true, a window can be grabbed and moved by dragging inside any area of the window.
         * Can't make this work reliably yet.
         */
        //bool win32_grab_and_move{false};

        /**
         * @brief Called when a Windows message arrives to the application main window's message loop. Returning zero will indicate the usual message handling should occur. Non-zero result skips the further message handling and returns from windowproc with this code.
         */
        std::function<LRESULT(UINT, WPARAM, LPARAM)> on_app_window_message;

#endif

    protected:

        // clear color as RGBA array of floats (0-1)
        const float ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.00f};

        void on_after_initialised();

        virtual std::shared_ptr<texture> make_native_texture(common::raw_img& img) = 0;

        /**
         * @brief Hints if dark mode should be enabled for this application on the OS level. For instance, on Windows 10/11 dark mode will paint window chrome in dark color.
         * @param enabled
         */
        virtual void set_dark_mode(bool enabled) = 0;

        /**
         * Called by backend on each frame to pause the application if FPS control is enabled.
         */
        void fps_pause() const;

        // pre-initialised main window
        widgets::window wnd_main;
        bool wnd_main_is_open{true};
        bool render_main_window(const std::function<bool()>& render_frame);

    private:
        // key is texture name, value is texture data. The app will take care of disposing of the textures when the app is closed.
        std::map<std::string, std::shared_ptr<texture>> textures;
    };
}
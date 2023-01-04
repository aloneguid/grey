#pragma once
#include <string>
#include <vector>
#include <memory>
#include "grey_context.h"
#include "components.h"

namespace grey
{
    class backend : public grey_context {
    public:
        std::function<void(const std::string& key, const std::string&)> on_save_settings;
        std::function<std::string(const std::string& key)> on_load_settings;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="title"></param>
        /// <param name="is_visible">Set to false to adjust the window before showing, otherwise you might see it flicker.</param>
        backend(const std::string& title, bool is_visible = true) : is_visible{is_visible}, title{title} {}

        // destructor must be virtual to ensure proper cleanup of graphics resources
        virtual ~backend() {};

        /**
         * @brief Creates rendering backend best suited for current platform. One application should have one backend instance only.
         * @param title
         * @param is_visible
         * @return
        */
        static std::unique_ptr<backend> make_platform_default(const std::string& title, bool is_visible = true);

        //virtual void run(std::function<bool(void)> frame) = 0;
        virtual void run_one_frame() = 0;
        virtual void run() = 0;
        void post_run_one_frame();

        virtual std::vector<app_theme> list_themes();

        virtual colour_theme get_theme() { return theme; }
        virtual void set_theme(colour_theme theme);
        virtual void set_theme(const std::string& theme_id);

        void set_default_font(font font);

        bool get_visibility() { return is_visible; }
        virtual void set_visibility(bool visible) = 0;

        virtual void resize(int width, int height) = 0;
        virtual void move(int x, int y) = 0;
        virtual void center_on_screen() = 0;
        virtual void bring_to_top() = 0;
        virtual void set_is_resizeable(bool value) { is_resizeable = value; };
        virtual float get_system_scale() { return 1.0f; }
        virtual void exit() = 0;

        void enable_docking();

        //void set_borderless(bool is_borderless) { this->is_borderless = is_borderless; }

        virtual void attach(std::shared_ptr<grey::window> w);
        virtual void detach(std::shared_ptr<grey::window> w);
        virtual void detach(std::string window_id);

        size_t window_count() { return windows.size(); }

    protected:
        bool is_visible;
        bool is_borderless{false};
        bool is_resizeable{true};
        std::vector<std::shared_ptr<grey::window>> windows;
        bool windows_dirty{false};
        std::vector<std::shared_ptr<grey::window>> windows_new;
        colour_theme theme{colour_theme::dark};
        std::string title;

        bool any_window_visible();
    };
}
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
        static std::unique_ptr<app> make(const std::string& title);

        /**
         * @brief When set, application will set this theme on startup.
         */
        std::string initial_theme_id;

        /**
         * @brief When true, loads icon font on startup. You can set this to "false" after creating the app instance
         *        (but before running it) to save some memory.
         */
        bool load_icon_font{true};

        float scale{1.0f};

        std::function<void()> on_initialised;

        app();

        virtual void run(std::function<bool(const app&)> render_frame) = 0;

        texture get_texture(const std::string& key);

        bool preload_texture(const std::string& key, unsigned char* buffer, unsigned int len);

        bool preload_texture(const std::string& key, const std::string& path);

    protected:
        void on_after_initialised();
        virtual void* make_native_texture(grey::common::raw_img& img) = 0;

    private:
        std::map<std::string, texture> textures;
    };
}
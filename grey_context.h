#pragma once
#include <string>
#include <vector>

namespace grey
{
    enum class colour_theme {
        follow_os,
        dark,
        light,
        light2,
        classic,
        corp_grey,
        cherry
    };

    enum class font {
        proggy_clean,
        //sweet_16_mono,
        roboto,
        //opensans
    };

    struct app_theme {
        const std::string id;
        const std::string name;
        const colour_theme theme;
    };

    class grey_context {
    public:
        virtual void* load_texture_from_file(
           const std::string& path, int& width, int& height) = 0;

        virtual void* load_texture_from_memory(
           const std::string& cache_name,
           unsigned char* buffer, unsigned int len, int& width, int& height) = 0;

        virtual float get_system_scale() = 0;

        virtual std::vector<app_theme> list_themes() = 0;

        virtual colour_theme get_theme() = 0;
        virtual void set_theme(colour_theme theme) = 0;
        virtual void set_theme(const std::string& theme_id) = 0;

        /**
         * @brief OS specific. On Windows returns HWND.
         * @return 
        */
        virtual void* get_native_window_handle() = 0;

    };
}
#pragma once

#include <cstdint>

#ifdef _WIN32
#define EXPORTED  __declspec(dllexport)
#endif

typedef void (*RenderFrameCallback)();

extern "C"
{
    /**
     * @brief The main entry that creates platform window with the specified title, width and height.
     * @param c_title 
     * @param width 
     * @param height 
     * @param c_frame_callback Callback function that is called on every frame.
     * @return 
     */
    EXPORTED void app_run(
        bool* is_running,
        const char* c_title,
        int32_t width,
        int32_t height,
        bool has_menubar,
        RenderFrameCallback c_frame_callback);

    EXPORTED void label(const char* c_text);

    EXPORTED bool button(const char* c_text);

    EXPORTED void sep(const char* c_text);
}
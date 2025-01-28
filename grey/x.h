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

    EXPORTED void sl(float offset);

    EXPORTED void label(const char* c_text);

    EXPORTED bool button(const char* c_text, int32_t emphasis);

    EXPORTED void sep(const char* c_text);

    EXPORTED bool push_tab_bar(const char* c_id);

    EXPORTED void pop_tab_bar();

    EXPORTED bool push_next_tab(const char* c_title);

    EXPORTED void pop_next_tab();

    EXPORTED bool push_table(const char* c_id, int32_t column_count, int32_t row_count, float outer_width, float outer_height);

    EXPORTED void pop_table();

    EXPORTED void table_col(const char* c_label, bool is_stretch);

    EXPORTED void table_headers_row();

    EXPORTED bool table_step(int& display_start, int& display_end);

    EXPORTED void table_next_row();

    EXPORTED void table_to_col(int32_t i);
}
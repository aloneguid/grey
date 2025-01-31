#pragma once

#include <cstdint>

#ifdef _WIN32
#define EXPORTED  __declspec(dllexport)
#else
#define EXPORTED
#endif

typedef bool (*RenderFrameCallback)();

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
        const char* c_title,
        int32_t width,
        int32_t height,
        bool has_menubar,
        bool can_scroll,
        bool center_on_screen,
        RenderFrameCallback c_frame_callback);

    EXPORTED void sl(float offset);

    EXPORTED void label(const char* c_text, int32_t emphasis, int32_t text_wrap_pos, bool enabled);

    EXPORTED bool checkbox(const char* c_label, bool* is_checked);

    EXPORTED bool button(const char* c_text, int32_t emphasis, bool is_enabled, bool is_small);

    EXPORTED void sep(const char* c_text);

    EXPORTED bool accordion(const char* c_header, bool default_open);

    EXPORTED bool hyperlink(const char* c_text, const char* c_url_to_open);

    EXPORTED void notify(const char* c_message);

    EXPORTED bool input_string(char* c_value, int32_t value_max_length,
            const char* c_label, bool enabled, float width, bool is_readonly);

    EXPORTED bool input_int(int32_t* value,
        const char* c_label, bool enabled, float width, bool is_readonly);

    EXPORTED bool input_multiline(const char* c_id,
        char* c_value, int32_t value_max_length,
        float height,
        bool autoscroll, bool enabled, bool use_fixed_font);

    // -- tab bars

    EXPORTED bool push_tab_bar(const char* c_id);

    EXPORTED void pop_tab_bar();

    EXPORTED bool push_next_tab(const char* c_title);

    EXPORTED void pop_next_tab();

    EXPORTED void spinner_hbo_dots(float radius, float thickness, float speed, int32_t dot_count);

    EXPORTED bool slider_float(float* value, float min, float max, const char* c_label);
    EXPORTED bool slider_int(int32_t* value, int32_t min, int32_t max, const char* c_label);

    // -- tables

    EXPORTED bool push_table(const char* c_id, int32_t column_count, float outer_width, float outer_height);

    EXPORTED void pop_table();

    EXPORTED void table_col(const char* c_label);

    EXPORTED void table_begin_data();

    EXPORTED void table_begin_row();

    EXPORTED void table_begin_col();

    // -- system debug

    EXPORTED void label_fps();
}
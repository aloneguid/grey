#pragma once

#include <cstdint>

#ifdef _WIN32
#define EXPORTED  __declspec(dllexport)
#else
#define EXPORTED
#endif

typedef bool (*RenderFrameCallback)();
typedef void (*RenderCallback)();
typedef void (*RenderTreeNodeCallback)(bool is_open);
typedef void (*RenderTableCellCallback)(int32_t row_index, int32_t col_index);
typedef void (*RenderPtrCallback)(void* ptr);


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

    EXPORTED void id_frame(int32_t scope_id, RenderCallback c_ptr_callback);

    EXPORTED void sl(float offset);

    EXPORTED void label(const char* c_text, int32_t emphasis, int32_t text_wrap_pos, bool enabled);

    EXPORTED bool selectable(const char* c_text, bool span_columns = false);

    EXPORTED bool checkbox(const char* c_label, bool* is_checked, bool is_small);

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

    EXPORTED bool slider_int(int32_t* value, int32_t min, int32_t max, const char* c_label, int step, bool ticks, int32_t emphasis);

    EXPORTED bool slider_float(float* value, float min, float max, const char* c_label, float step, bool ticks, int32_t emphasis);

    EXPORTED void tt(const char* text, int32_t delay);

    EXPORTED void rich_tt(RenderCallback c_render_callback, int32_t delay);

    EXPORTED bool combo(const char* c_label, const char** options, int32_t options_size, uint32_t* selected, float width);

    EXPORTED bool list(const char* c_label, const char** options, int32_t options_size, uint32_t* selected, float width);

    EXPORTED void tab_bar(const char* c_id, RenderPtrCallback c_render_callback);

    EXPORTED void tab(void* tab_bar_ptr, const char* c_title, bool unsaved, bool selected, RenderCallback c_render_callback);

    EXPORTED void spinner_hbo_dots(float radius, float thickness, float speed, int32_t dot_count);

    EXPORTED void status_bar(RenderCallback c_render_callback);

    EXPORTED bool is_hovered();

    EXPORTED bool is_leftclicked();

    EXPORTED bool is_rightclicked();

    /**
     * @brief Table call in a single function call.
     * @param c_cell_callback Callback function that is called for each cell in the table, and receives row and column indices.
     */
    EXPORTED void big_table(const char* c_id, const char** c_columns, int32_t c_columns_size, int32_t row_count,
        float outer_width, float outer_height,
        bool alternate_row_bg,
        RenderTableCellCallback c_cell_callback);

    EXPORTED void table(const char* c_id, const char** c_columns, int32_t c_columns_size,
        float outer_width, float outer_height,
        bool alternate_row_bg,
        RenderPtrCallback c_ptr_callback);

    EXPORTED bool table_begin_row(void* table_ptr);

    EXPORTED bool table_next_column(void* table_ptr);

    EXPORTED void tree_node(const char* c_label, bool open_by_default, bool is_leaf, bool span_all_cols, RenderTreeNodeCallback c_render_callback);

    EXPORTED void menu_bar(RenderCallback c_render_callback);

    EXPORTED void menu(const char* c_title, RenderCallback c_render_callback);

    EXPORTED bool menu_item(const char* c_text, bool reserve_icon_space, const char* c_icon = "");

    // -- system debug

    EXPORTED void label_fps();
}
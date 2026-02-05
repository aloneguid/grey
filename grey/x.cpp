#include "x.h"
#include "grey.h"
#include <string>
#include <memory>
#include <vector>
#include <stack>
#include <iostream>

using namespace std;
namespace w = grey::widgets;

#ifdef _WIN32
#include "common/win32/os.h"
#endif

float scale = 1.0f;

void platform_init() {
#if _WIN32
    grey::common::win32::os::set_dpi_awareness();
#endif
}

EXPORTED void app_run(
    const char* c_title,
    int32_t width,
    int32_t height,
    bool has_menubar,
    bool can_scroll,
    bool center_on_screen,
    RenderFrameCallback c_frame_callback) {

    platform_init();

    string title = c_title;

    // main window
    bool is_running = true;
    w::window wnd{title, &is_running};
    wnd
        .no_titlebar()
        .no_resize()
        .fill_viewport()
        .border(0);

    if(has_menubar) {
        wnd.has_menubar();
    }

    if(!can_scroll) {
        wnd.no_scroll();
    }

    if(center_on_screen) {
        wnd.center();
    }

    auto app = grey::app::make(title, width, height);
#ifdef _WIN32
    app->win32_can_resize = true;
#endif
    app->load_fixed_font = true;
    app->run([c_frame_callback, &is_running, &wnd](const grey::app& app) {

        scale = app.scale;

        w::guard g{wnd};

        if(c_frame_callback) {
            if(!c_frame_callback()) {
                return false;
            }
        }

        w::notify_render_frame();

        return true;
    });
}

EXPORTED void sl(float offset) {
    w::sl(offset);
}

EXPORTED void label(const char* c_text, int32_t emphasis, int32_t text_wrap_pos, bool enabled) {
    string text{c_text};
    w::label(text, (w::emphasis)emphasis, text_wrap_pos, enabled);
}

EXPORTED bool selectable(const char* c_text, bool span_columns) {
    string text{ c_text };
    return w::selectable(text, span_columns);
}

EXPORTED bool checkbox(const char* c_label, bool* is_checked, bool is_small) {
    string label{ c_label };
    return is_small ? w::small_checkbox(label, *is_checked) : w::checkbox(label, *is_checked);
}

EXPORTED bool button(const char* c_text, int32_t emphasis, bool is_enabled, bool is_small) {
    string text{c_text};
    return w::button(text, (w::emphasis)emphasis, is_enabled, is_small);
}

EXPORTED void sep(const char* c_text) {
    string text{c_text};
    w::sep(text);
}

EXPORTED bool accordion(const char* c_header, bool default_open) {
    string header{c_header};
    return w::accordion(header, default_open);
}

EXPORTED bool hyperlink(const char* c_text, const char* c_url_to_open) {
    string text{ c_text };
    string url = c_url_to_open ? c_url_to_open : "";
    return w::hyperlink(text, url);
}

EXPORTED void notify(const char* c_message) {
    string message{ c_message };
    w::notify_info(message);
}

EXPORTED bool input_string(char* c_value, int32_t value_max_length, const char* c_label, bool enabled, float width, bool is_readonly) {
    string label{ c_label };
    return w::input(c_value, value_max_length, label, enabled, width * scale, is_readonly);
}

EXPORTED bool input_int(int32_t* value, const char* c_label, bool enabled, float width, bool is_readonly) {
    string label{ c_label };
    return w::input(*value, label, enabled, width * scale, is_readonly);
}

EXPORTED bool input_multiline(const char* c_id, char* c_value, int32_t value_max_length, float height, bool autoscroll, bool enabled, bool use_fixed_font) {
    string id{ c_id };
    //cout << "fixed font: " << use_fixed_font << endl;
    return w::input_ml(id, c_value, value_max_length, height, autoscroll, enabled, use_fixed_font);
}

EXPORTED void spinner_hbo_dots(float radius, float thickness, float speed, int32_t dot_count) {
    w::spinner_hbo_dots(radius, thickness, speed, dot_count);
}

EXPORTED bool slider_float(float* value, float min, float max, const char* c_label, float step, bool ticks, int32_t emphasis) {
    string label{ c_label };
    return w::slider(*value, min, max, label, step, ticks, (w::emphasis)emphasis);
}

EXPORTED bool slider_int(int32_t* value, int32_t min, int32_t max, const char* c_label) {
    string label{ c_label };
    return w::slider_classic(*value, min, max, label);
}

EXPORTED void tt(const char* text, int32_t delay) {
    w::tt(text, static_cast<w::show_delay>(delay));
}

void rich_tt(RenderCallback c_render_callback, int32_t delay) {
    w::rich_tt tt{static_cast<w::show_delay>(delay)};
    if(tt) {
        c_render_callback();
    }
}

EXPORTED bool combo(const char* c_label, const char** options, int32_t options_size, uint32_t* selected, float width) {
    return w::combo(c_label, vector<string>(options, options + options_size), *selected, width * scale);
}

EXPORTED bool list(const char* c_label, const char** options, int32_t options_size, uint32_t* selected, float width) {
    return w::list(c_label, vector<string>(options, options + options_size), *selected, width * scale);
}

EXPORTED void tab_bar(const char* c_id, RenderPtrCallback c_render_callback) {
    w::tab_bar tb{ c_id };
    c_render_callback(&tb); 
}

EXPORTED void tab(void* tab_bar_ptr, const char* c_title, bool unsaved, bool selected, RenderCallback c_render_callback) {
    w::tab_bar* tb = static_cast<w::tab_bar*>(tab_bar_ptr);
    w::tab_bar_item tbi = tb->next_tab(c_title, unsaved, selected);
    if(tbi) {
        c_render_callback();
    }
}

EXPORTED void status_bar(RenderCallback c_render_callback) {
    w::status_bar sb;
    c_render_callback();
}

bool is_hovered() {
    return w::is_hovered();
}

bool is_leftclicked() {
    return w::is_leftclicked();
}

bool is_rightclicked() {
    return w::is_rightclicked();
}

EXPORTED void big_table(const char* c_id,
    const char** c_columns, int32_t c_columns_size, int32_t row_count,
    float outer_width, float outer_height,
    bool alternate_row_bg,
    RenderTableCellCallback c_cell_callback)
{
    vector<string> cols;
    // copy columns into cols vector
    for (int i = 0; i < c_columns_size; i++) {
        cols.push_back(c_columns[i]);
    }

    w::big_table t{ c_id, cols, static_cast<size_t>(row_count), outer_width * scale, outer_height * scale, alternate_row_bg };
    t.render_data([c_cell_callback](int row_idx, int column_idx) {
            c_cell_callback(row_idx, column_idx);
        });
}

EXPORTED void table(const char* c_id, const char** c_columns, int32_t c_columns_size, float outer_width, float outer_height, bool alternate_row_bg, RenderPtrCallback c_ptr_callback) {
    vector<string> cols;
    // copy columns into cols vector
    for(int i = 0; i < c_columns_size; i++) {
        cols.push_back(c_columns[i]);
    }

    w::table t{c_id, cols, outer_width * scale, outer_height * scale, alternate_row_bg};
    if(t) {
        c_ptr_callback(&t);
    }
}

EXPORTED bool table_begin_row(void* table_ptr) {
    w::table* t = static_cast<w::table*>(table_ptr);
    return t->begin_row();
}

bool table_next_column(void* table_ptr) {
    w::table* t = static_cast<w::table*>(table_ptr);
    return t->next_column();
}

EXPORTED void tree_node(const char* c_label, bool open_by_default, bool is_leaf, bool span_all_cols, RenderTreeNodeCallback c_render_callback) {
    w::tree_node tn{c_label, open_by_default, is_leaf, span_all_cols};
    c_render_callback(tn);
}

EXPORTED void menu_bar(RenderCallback c_render_callback) {
    if(w::menu_bar mb; mb) {
        c_render_callback();
    }
}

void menu(const char* c_title, RenderCallback c_render_callback) {
    string title = c_title;
    if(w::menu m{title}; m) {
        c_render_callback();
    }
}

// -- application menus

EXPORTED bool menu_item(const char* c_text, bool reserve_icon_space, const char* c_icon) {
    string text{c_text};
    string icon{c_icon ? c_icon : ""};
    return w::mi(text, reserve_icon_space, icon);
}

EXPORTED void label_fps() {
    float fps = ImGui::GetIO().Framerate;
    // format fps to string with 2 floating points
    char buf[32];
    sprintf(buf, "%.2f", fps);
    w::label(buf);
}

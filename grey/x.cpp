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

EXPORTED bool checkbox(const char* c_label, bool* is_checked) {
    string label{ c_label };
    return w::checkbox(label, *is_checked);
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

stack<w::tab_bar> tab_bars;
stack<w::tab_bar_item> tab_items;

EXPORTED bool push_tab_bar(const char* c_id) {

    //cout << "push_tab_bar: " << c_id << endl;
    tab_bars.emplace(c_id);
    return true;
}

EXPORTED void pop_tab_bar() {
    //cout << "pop_tab_bar " << tab_bars.size() << endl;
    tab_bars.pop();
}


EXPORTED bool push_next_tab(const char* c_title) {

    //cout << "push_next_tab: " << c_title << endl;

    if(tab_bars.empty()) {
        return false;
    }

    w::tab_bar& bar = tab_bars.top();
    string id = string{c_title} + "##" + std::to_string(bar.increment_tab_index());
    tab_items.emplace(id, false, false);

    auto& item = tab_items.top();
    return item;
}

EXPORTED void pop_next_tab() {
    //cout << "pop_next_tab" << endl;

    tab_items.pop();
}

EXPORTED void spinner_hbo_dots(float radius, float thickness, float speed, int32_t dot_count) {
    w::spinner_hbo_dots(radius, thickness, speed, dot_count);
}

EXPORTED bool slider_float(float* value, float min, float max, const char* c_label) {
    string label{ c_label };
    return w::slider(*value, min, max, label);
}

EXPORTED bool slider_int(int32_t* value, int32_t min, int32_t max, const char* c_label) {
    string label{ c_label };
    return w::slider(*value, min, max, label);
}

EXPORTED void tooltip(const char* text) {
    w::tooltip(text);
}

EXPORTED bool combo(const char* c_label, const char** options, int32_t options_size, uint32_t* selected, float width) {
    return w::combo(c_label, vector<string>(options, options + options_size), *selected, width * scale);
}

// -- status bar

vector<w::status_bar> status_bars;

EXPORTED void push_status_bar() {
    status_bars.emplace_back();
}

EXPORTED void pop_status_bar() {
    status_bars.pop_back();
}

EXPORTED void table(const char* c_id,
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

// -- application menus

stack<w::menu_bar> menu_bars;

EXPORTED bool push_menu_bar() {
    menu_bars.emplace();
    auto& t = menu_bars.top();
    return t;
}

EXPORTED void pop_menu_bar() {
    menu_bars.pop();
}

stack<w::menu> menus;

EXPORTED bool push_menu(const char* c_title) {
    string title = c_title;
    menus.emplace(title);
    auto& t = menus.top();
    return t;
}

EXPORTED void pop_menu() {
    menus.pop();
}

EXPORTED bool menu_item(const char* c_text, bool reserve_icon_space, const char* c_icon) {
    string text{c_text};
    string icon{c_icon ? c_icon : ""};
    auto& m = menus.top();
    return w::mi(text, reserve_icon_space, icon);
}

EXPORTED void label_fps() {
    float fps = ImGui::GetIO().Framerate;
    // format fps to string with 2 floating points
    char buf[32];
    sprintf(buf, "%.2f", fps);
    w::label(buf);
}

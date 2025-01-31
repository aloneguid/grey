#include "x.h"
#include "grey.h"
#include <string>
#include <memory>
#include <vector>
#include <map>
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
    tab_items.emplace(id, false);

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

stack<w::table> tables;

EXPORTED bool push_table(const char* c_id, int32_t column_count, float outer_width, float outer_height) {
    string id = c_id;
    tables.emplace(id, column_count, outer_width, outer_height);
    auto& t = tables.top();
    return t;
}

EXPORTED void pop_table() {
    tables.pop();
}

EXPORTED void table_col(const char* c_label) {
    string label = c_label;
    auto& t = tables.top();
    t.columns.push_back(label);
}

EXPORTED void table_begin_data() {
    auto& t = tables.top();
    t.begin_data();
}

EXPORTED void table_begin_row() {
    auto& t = tables.top();
    t.begin_row();
}

EXPORTED void table_begin_col() {
    auto& t = tables.top();
    t.begin_col();
}

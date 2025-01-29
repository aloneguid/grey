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

void platform_init() {
#if _WIN32
    grey::common::win32::os::set_dpi_awareness();
#endif
}

EXPORTED void app_run(
    bool* is_running,
    const char* c_title,
    int32_t width,
    int32_t height,
    bool has_menubar,
    RenderFrameCallback c_frame_callback) {

    platform_init();

    string title = c_title;

    // main window
    w::window wnd{title, is_running};
    wnd
        .no_titlebar()
        .no_scroll()
        .no_resize()
        .fill_viewport()
        .border(0);

    if(has_menubar) {
        wnd.has_menubar();
    }

    auto app = grey::app::make(title, width, height);
#ifdef _WIN32
    app->win32_can_resize = true;
#endif
    app->run([c_frame_callback, &is_running, &wnd](const grey::app& app) {

        w::guard g{wnd};

        if(c_frame_callback) {
            c_frame_callback();
        }

        return *is_running;
    });
}

EXPORTED void sl(float offset) {
    w::sl(offset);
}

EXPORTED void label(const char* c_text) {
    string text{c_text};
    w::label(text);
}

EXPORTED bool button(const char* c_text, int32_t emphasis) {
    string text{c_text};
    return w::button(text, (w::emphasis)emphasis);
}

EXPORTED void sep(const char* c_text) {
    string text{c_text};
    w::sep(text);
}

EXPORTED bool accordion(const char* c_header, bool default_open) {
    string header{c_header};
    return w::accordion(header, default_open);
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

stack<w::table> tables;

EXPORTED bool push_table(const char* c_id, int32_t column_count, int32_t row_count, float outer_width, float outer_height) {
    string id = c_id;
    tables.emplace(id, column_count, row_count, ImVec2(outer_width, outer_height));
    auto& t = tables.top();
    return t;
}

EXPORTED void pop_table() {
    tables.pop();
}

EXPORTED void table_col(const char* c_label, bool is_stretch) {
    string label = c_label;
    auto& t = tables.top();
    t.col(label, is_stretch);
}

EXPORTED void table_headers_row() {
    auto& t = tables.top();
    t.headers_row();
}

EXPORTED bool table_step(int& display_start, int& display_end) {
    auto& t = tables.top();
    return t.step(display_start, display_end);
}

EXPORTED void table_next_row() {
    auto& t = tables.top();
    t.next_row();
}

EXPORTED void table_to_col(int32_t i) {
    auto& t = tables.top();
    t.to_col(i);
}

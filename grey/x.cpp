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
    app->win32_can_resize = true;
    //app->load_fixed_font = true;
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

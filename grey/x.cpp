#include "x.h"
#include "grey.h"
#include <string>
#include <memory>
#include <vector>
#include <map>

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

EXPORTED void label(const char* c_text) {
    string text{c_text};
    w::label(text);
}

EXPORTED bool button(const char* c_text) {
    string text{c_text};
    return w::button(text);
}
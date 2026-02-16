//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"

using namespace std;
namespace w = grey::widgets;

bool app_open{true};
string window_title = "Demo app";
w::window wnd{window_title, &app_open};

#if WIN32
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
#else
int main(int argc, char* argv[]) {
#endif

    wnd
        .no_resize()
        .no_titlebar()
        .fill_viewport()
        .border(0)
        .no_background();


    auto app = grey::app::make("transparent", 800, 600);
    float scale = app->scale;
    app->win32_use_transparency_colour_key_value = true;

    app->run([&app](const grey::app& c_app) {
        w::guard wg{wnd};

        w::button("hopefully transparent");

        return app_open;
    });

    return 0;
}
//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"

using namespace std;
namespace w = grey::widgets;

bool app_open{true};
string window_title = "Demo app";

#if WIN32
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
#else
int main(int argc, char* argv[]) {
#endif

    auto app = grey::app::make("transparent", {800, 600});
    float scale = w::scale;
    app->use_transparency_colour_key_value = true;
    app->main_window().no_background();

    app->run([&app]() {

        w::button("hopefully transparent");

        return app_open;
    });

    return 0;
}
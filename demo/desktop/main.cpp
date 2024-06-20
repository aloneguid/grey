//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"

using namespace std;
namespace w = grey::widgets;

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

    //auto backend = grey::backend::make_platform_default(APP_LONG_NAME);
    //auto wnd = backend->make_window<demo::main_wnd>();
    //backend->run();

    auto app = grey::app::make(APP_LONG_NAME);
    bool app_open{true};

    vector<w::menu_item> menu_items{
        { "File", {
            { "file_new", "New", ICON_FK_ACTIVITYPUB },
            { "file_open", "Open" },
            { "file_save", "Save" },
            { "file_save_as", "Save As" },
            { "", "-"},
            { "file_exit", "Exit" },
            { "Recent", { {"1", "file1.txt" }}}
            }
        }
    };

    app->run([&app_open, &menu_items](const grey::app& app) {
        w::window wnd{"Hello, world!", &app_open};
        wnd
            .size(800, 600, app.scale)
            //.no_focus()
            .has_menubar()
            .render();

        // menu
        {
            w::menu_bar menu{menu_items, [&app_open](const string& id) {
                if(id == "file_exit") {
                    app_open = false;
                }
            }};
        }

        // labels
        w::label("simple text");
        w::label(ICON_FK_BOOK " icon1");

        return app_open;
    });

    return 0;
}
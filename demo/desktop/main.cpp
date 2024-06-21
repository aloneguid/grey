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
        },
        { "Theme", w::menu_item::make_ui_theme_items() }
    };

    string s;

    app->run([&app_open, &menu_items, &s](const grey::app& app) {
        w::window wnd{"Hello, world!", &app_open};
        wnd
            .size(800, 600, app.scale)
            //.no_focus()
            .has_menubar()
            .render();

        // menu
        {
            w::menu_bar menu{menu_items, [&app_open, &app](const string& id) {
                if(id == "file_exit") {
                    app_open = false;
                } else if(id.starts_with("set_theme_")) {
                    grey::themes::set_theme(id, app.scale);
                }
            }};
        }

        // labels
        w::label("simple text");
        w::sl();  w::label(ICON_FK_BOOK " icon1");
        w::sl();  w::label("primary", w::emphasis::primary);
        w::sl();  w::label("error", w::emphasis::error);

        // buttons
        w::button("simple");
        w::sl(); w::button("primary", w::emphasis::primary);
        w::sl(); w::button("error", w::emphasis::error);
        w::sl(); w::label(s);
        if(w::button("add dot")) {
            s += ".";
        }

        return app_open;
    });

    return 0;
}
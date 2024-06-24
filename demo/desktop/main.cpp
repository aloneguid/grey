//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"

using namespace std;
namespace w = grey::widgets;

vector<string> items = { "item1", "item2", "item3" };
size_t current_item = 0;

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

    //auto backend = grey::backend::make_platform_default(APP_LONG_NAME);
    //auto wnd = backend->make_window<demo::main_wnd>();
    //backend->run();

    auto app = grey::app::make(APP_LONG_NAME);
    bool app_open{true};
    bool show_demo{false};
    string text;
    w::container scroller{400, 100};

    vector<w::menu_item> menu_items{
        { "File", {
            { "file_new", "New", ICON_MD_DONUT_LARGE },
            { "file_open", "Open" },
            { "file_save", "Save" },
            { "file_save_as", "Save As" },
            { "", "-"},
            { "file_exit", "Exit" },
            { "Recent", { {"1", "file1.txt" }}}
            }
        },
        { "Theme", w::menu_item::make_ui_theme_items() },
        { "ImGui", {
            { "demo", "Show Demo", ICON_MD_DONUT_LARGE }
        }}
    };

    string s;

    app->run([&app_open, &menu_items, &s, &scroller, &show_demo, &text](const grey::app& app) {
        w::window wnd{"Hello, world!", &app_open};
        wnd
            .size(800, 600, app.scale)
            //.no_focus()
            .has_menubar()
            .render();

        // menu
        {
            w::menu_bar menu{menu_items, [&app_open, &app, &show_demo](const string& id) {
                if(id == "file_exit") {
                    app_open = false;
                } else if(id.starts_with("set_theme_")) {
                    grey::themes::set_theme(id, app.scale);
                } else if(id == "demo") {
                    show_demo = true;
                }
            }};
        }

        // labels
        w::label("simple text");
        w::sl(); w::label(ICON_MD_5G " icon1");
        w::sl(); w::label("primary", w::emphasis::primary);
        w::sl(); w::label("error", w::emphasis::error);

        w::sep();

        // buttons
        w::button("simple");
        w::sl(); w::button("primary", w::emphasis::primary);
        w::sl(); w::button("error", w::emphasis::error);
        w::sl(); w::label(s);
        if(w::button("add dot")) {
            s += ".";
        }

        {
            w::guard g{scroller};

            // add 100 buttons
            for(int i = 0; i < 100; i++) {
                w::button("button " + to_string(i));
            }
        }

        {
            w::group g;
            g.render();

            w::label("group");
        }


        {
            w::group g;
            g.border(ImGuiCol_FrameBgActive).render();

            w::label("bordered group");
        }

        {
            w::group g;
            g.border(ImGuiCol_FrameBgActive).spread_horizontally().render();

            w::label("full width");
        }

        {
            w::group g;
            g
                .background_hover(ImGuiCol_Border)
                .border_hover(ImGuiCol_FrameBgHovered)
                .render();

            w::label("border on hover");
        }

        w::icon_checkbox(ICON_MD_BATHTUB, show_demo);


        {
            w::status_bar sb;

            w::label("status bar");
        }

        // tabs
        {
            w::tab_bar tabs{"tabs1"};

            {
                auto t = tabs.next_tab("tab 1");
                if(t) {
                    w::label("tab 1");
                }
            }

            {
                auto t = tabs.next_tab("tab 2");
                if(t) {
                    w::button("tab 2");
                }
            }
        }

        if(w::accordion("Content inside")) {
            w::label("accordion content");
        }

        w::label(text);
        w::input(text, "text");

        w::label("selected item: "); w::sl(); w::label(items[current_item]);
        w::combo("si", items, current_item);

        if(show_demo)
            ImGui::ShowDemoWindow();


        return app_open;
    });

    return 0;
}
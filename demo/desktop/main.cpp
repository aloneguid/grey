//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"
#include <imgui_internal.h>
#include <vector>
#include <iostream>

using namespace std;
namespace w = grey::widgets;

vector<string> items = { "item1", "item2", "item3" };
unsigned int current_item = 0;
bool app_open{true};
bool show_demo{false};
string window_title = "Demo app";
w::window wnd{window_title, &app_open};
string text;
w::container scroller{400, 100};
w::popup status_pop {"status_pop"};
bool ned_initialised{false};
w::text_editor ted;
bool selected{false};
// multiline string with sample for text editor
string text_editor_text = R"(
-- This is a comment
function foo()
    print("Hello, world!")
end
)";

void plot_demo() {

}

#if WIN32
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
#else
int main(int argc, char* argv[]) {
#endif

    //auto backend = grey::backend::make_platform_default(APP_LONG_NAME);
    //auto wnd = backend->make_window<demo::main_wnd>();
    //backend->run();

    auto app = grey::app::make("demo", 800, 600);
    app->load_fixed_font = true;
    float scale = app->scale;

    wnd
        .no_titlebar()
        .no_scroll()
        .no_resize()
        .fill_viewport()
        .border(0)
        .has_menubar();

    ted.set_text("-- add your Lua code here");


    app->run([&app](const grey::app& c_app) {
        
        w::guard wg{wnd};

        // menu
        {
            w::menu_bar menu;
            if(menu) {
                {
                    w::menu m("File");
                    if(m) {
                        w::mi("New", true, ICON_MD_DONUT_LARGE);
                        if(w::mi("Exit", true)) {
                            app_open = false;
                        }
                    }
                }

                {
                    w::menu m("View");
                    if(m) {
                        w::mi_themes([&app](const std::string& id) {
                            w::notify_info("theme changed to " + id);
                            app->set_theme(id);
                        });
                    }
                }

                {
                    w::menu m("Help");
                    if(m) {
                        w::mi("About");
                    }
                }
            }
        }

        // top tabs
        {
            w::tab_bar tabs{"topTabs"};

            // basics
            {
                auto tab = tabs.next_tab("Basics");
                if(tab) {
                    w::label("simple text");
                    w::tooltip("simple tooltip");
                    w::sl(); w::label(ICON_MD_5G " icon1");
                    w::sl(); w::label("primary", w::emphasis::primary);
                    w::sl(); w::label("error", w::emphasis::error);

                    w::sep();

                    w::sep("buttons");

                    w::button("simple");
                    w::sl(); w::button("primary", w::emphasis::primary);
                    w::sl(); w::button("error", w::emphasis::error);
                    w::sl(); w::label(text);
                    if(w::button("add dot")) {
                        text += ".";
                    }

                    w::sep("radios");
                    w::radio("radio1", selected);
                    w::radio("radio2", !selected);
                    w::small_radio("small radio1", selected);
                    w::small_radio("small radio2", !selected);

                    w::sep("checkboxes");
                    w::checkbox("basic", selected);
                    w::small_checkbox("small", selected);

                    if(w::hyperlink("click me")) {
                        w::notify_info("hyperlink clicked");
                    }

                    w::hyperlink("blog", "https://www.aloneguid.uk/posts/");

                    w::input(text, "##input1");
                }
            }

            // container
            {
                auto tab = tabs.next_tab("Container");
                if(tab) {
                    w::guard g{scroller};

                    // add 100 buttons
                    for(int i = 0; i < 100; i++) {
                        w::button("button " + to_string(i));
                    }
                }
            }

            // group
            {
                auto tab = tabs.next_tab("Group");
                if(tab) {
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

                }
            }

            // accordion
            {
                auto tab = tabs.next_tab("Accordion");
                if(tab) {
                    if(w::accordion("Content inside")) {
                        w::label("accordion content");
                    }
                }
            }

            // lists
            {
                auto tab = tabs.next_tab("Lists");
                if(tab) {
                    w::combo("combo", items, current_item);
                    w::list("list", items, current_item);

                    w::label("selected item: "); w::sl(); w::label(items[current_item]);
                    w::label("selected index: "); w::sl(); w::label(to_string(current_item));



                    if(w::button("center on screen")) {
                        wnd.center();
                    }
                }
            }

            // notifications
            {
                auto tab = tabs.next_tab("Notifications");
                if(tab) {
                    if(w::button("info message")) {
                        w::notify_info("inof");
                    }
                }
            }

            // tables
            {
                auto tab = tabs.next_tab("Table");
                if(tab) {
                    int row_count = 1000;
                    w::table t{"table1", 3, 0, -20 * c_app.scale};
                    if(t) {
                        t.columns.push_back("id");
                        t.columns.push_back("name");
                        t.columns.push_back("description+");
                      
                        t.begin_data();
                        for(int i = 0; i < row_count; i++) {
                            t.begin_row();

                            t.begin_col();
                            w::label(to_string(i));

                            t.begin_col();
                            w::label("name " + to_string(i));

                            t.begin_col();
                            w::label("description " + to_string(i));
                        }
                    }
                }
            }
            {
                auto tab = tabs.next_tab("Big Table");
                if(tab) {
                    static int row_count = 1000;
                    static int col_count = 3;

                    w::slider(row_count, 0, 1000000000, "row count");
                    w::slider(col_count, 1, 100, "col count");

                    static vector<string> columns;
                    if(col_count != columns.size()) {
                        columns.clear();
                        for(int i = 0; i < col_count; i++) {
                            columns.push_back("col " + to_string(i));
                        }
                    }

                    static bool row_bg = false;
                    w::checkbox("alternate row bg", row_bg);

                    w::big_table t{"table2", columns, (size_t)row_count, 0.0f, -20 * w::scale, row_bg};
                    if(t) {
                        t.render_data([](int row, int col) {
                            if(col == 0) {
                                w::label(to_string(row));
                            } else {
                                w::label(to_string(row) + "x" + to_string(col));
                            }
                        });
                    }
                }
            }


            // Spinners
            {
                auto tab = tabs.next_tab("Spinners");
                if(tab) {
                    static float radius = 50;
                    static float thickness = 4;
                    static float speed = 1;
                    static int dot_count = 8;
                    w::slider(radius, 5, 500, "radius");
                    w::slider(thickness, 1, 50, "thickness");
                    w::slider(speed, 0.1, 10, "speed");
                    w::slider(dot_count, 1, 100, "dot count");
                    w::spinner_hbo_dots(radius, thickness, speed, dot_count);
                }
            }

            // Multiline edit
            {
                auto tab = tabs.next_tab("Editor");
                if(tab) {
                    static float height = 0;
                    static bool autoscroll = false;
                    static bool enabled = true;
                    static bool use_fixed_font = false;
                    w::slider(height, -500, 500, "height");
                    w::checkbox("autoscroll", autoscroll);
                    w::checkbox("enabled", enabled);
                    w::checkbox("fixed font", use_fixed_font);
                    w::input_ml("##ml", text_editor_text, height == 0 ? -FLT_MIN : height, autoscroll, enabled, use_fixed_font);
                }
            }

            // ImGuiColorTextEdit
            with_tab(tabs, "Lua Editor",
                if(ted.render()) {
                    w::label("code changed");
                })

            // ImPlot
            with_tab(tabs, "Plots",
                //w::plot_demo();
                plot_demo();)
        }


        with_status_bar(
            w::label(ICON_MD_HEAT_PUMP, w::emphasis::primary);
            w::sl();
            w::label("|", 0, false);
            w::sl();
            if(w::button("pop!", w::emphasis::none, true, true)) {
                status_pop.open();
            }
            w::sl();

            w::guard g{status_pop};
            if(status_pop) {
                w::label("popup content");
            }

            w::sl(); w::label("|", 0, false);
            w::sl(); w::label_debug_info();

            w::sl(); w::label("|", 0, false);
            w::sl(); w::label(ImGui::GetVersion());
        )


        if(show_demo)
            ImGui::ShowDemoWindow();

        w::notify_render_frame();

        return app_open;
    });

    return 0;
}
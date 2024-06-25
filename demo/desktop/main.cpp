//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"
#include <imgui_internal.h>

using namespace std;
namespace w = grey::widgets;

vector<string> items = { "item1", "item2", "item3" };
size_t current_item = 0;
bool app_open{true};
bool show_demo{false};
string window_title = "Demo app";
w::window wnd{window_title, &app_open};
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


int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

    //auto backend = grey::backend::make_platform_default(APP_LONG_NAME);
    //auto wnd = backend->make_window<demo::main_wnd>();
    //backend->run();

    auto app = grey::app::make(APP_LONG_NAME);
    float scale = app->scale;

    wnd
        .size(800, 600, app->scale)
        .center()
        .no_scroll()
        .has_menubar();

    app->run([](const grey::app& app) {
        
        w::guard wg{wnd};

        // menu
        {
            w::menu_bar menu{menu_items, [&app](const string& id) {
                if(id == "file_exit") {
                    app_open = false;
                } else if(id.starts_with("set_theme_")) {
                    grey::themes::set_theme(id, app.scale);
                } else if(id == "demo") {
                    show_demo = true;
                }
            }};
        }

        // top tabs
        {
            w::tab_bar tabs{"topTabs"};

            // labels
            {
                auto tab = tabs.next_tab("Labels");
                if(tab) {
                    w::label("simple text");
                    w::sl(); w::label(ICON_MD_5G " icon1");
                    w::sl(); w::label("primary", w::emphasis::primary);
                    w::sl(); w::label("error", w::emphasis::error);
                }
            }

            // buttons
            {
                auto tab = tabs.next_tab("Buttons");
                if(tab) {
                    w::button("simple");
                    w::sl(); w::button("primary", w::emphasis::primary);
                    w::sl(); w::button("error", w::emphasis::error);
                    w::sl(); w::label(text);
                    if(w::button("add dot")) {
                        text += ".";
                    }
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
                    w::label(text);
                    w::input(text, "text");

                    w::label("selected item: "); w::sl(); w::label(items[current_item]);
                    w::combo("si", items, current_item);

                    if(w::button("center on screen")) {
                        wnd.center();
                    }
                }
            }
        }

        //w::icon_checkbox(ICON_MD_BATHTUB, show_demo);

        //ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetWindowViewport();
        //ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
        //float height = ImGui::GetFrameHeight();

        //if(ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags)) {
        //    if(ImGui::BeginMenuBar()) {
        //        ImGui::Text("Happy status bar");
        //        ImGui::EndMenuBar();
        //    }
        //    ImGui::End();
        //}

        {
            w::status_bar sb;

            w::label(ICON_MD_HEAT_PUMP, w::emphasis::primary);
            w::label("|", 0, false);
        }


        if(show_demo)
            ImGui::ShowDemoWindow();

        return app_open;
    });

    return 0;
}
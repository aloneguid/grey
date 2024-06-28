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
w::popup status_pop {"status_pop"};
w::node_editor ned;

vector<w::menu_item> menu_items {
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
        .size(800, 600)
        .center()
        //.no_scroll()
        .has_menubar();

    app->run([](const grey::app& app) {
        
        w::guard wg{wnd};

        // menu
        {
            w::menu_bar menu{menu_items, [&app](const string& id) {
                if(id == "file_exit") {
                    app_open = false;
                } else if(id.starts_with(w::SetThemeMenuPrefix)) {
                    string theme_id = w::menu_item::remove_theme_prefix(id);
                    grey::themes::set_theme(theme_id, app.scale);
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

                    w::sep();
                    w::sep("with text");
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

            // imnodes
            {
                auto tab = tabs.next_tab("ImNodes");
                if(tab) {
                    w::guard g{ned};

                    {
                        ned.node(0);
                    }

                    {
                        ned.node(1);
                    }

                    int uniqueId = 10;

                    // Start drawing nodes.
                    ed::BeginNode(uniqueId++);
                    ImGui::Text("Node A");
                    ed::BeginPin(uniqueId++, ed::PinKind::Input);
                    ImGui::Text("-> In");
                    ed::EndPin();
                    ImGui::SameLine();
                    int pin0 = uniqueId++;
                    ed::BeginPin(pin0, ed::PinKind::Output);
                    ImGui::Text("Out ->");
                    ed::EndPin();
                    ed::EndNode();

                    ed::BeginNode(uniqueId++);
                    ImGui::Text("Node B");
                    ed::BeginPin(uniqueId++, ed::PinKind::Input);
                    ImGui::Text("-> In");
                    ed::EndPin();
                    ImGui::SameLine();
                    ed::BeginPin(uniqueId++, ed::PinKind::Output);
                    ImGui::Text("Out ->");
                    ed::EndPin();
                    ed::EndNode();

                    auto style = ImGui::GetStyle();

                    ed::PushStyleVar(ed::StyleVar_NodeRounding, style.FrameRounding);
                    ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(5, 5, 5, 5));
                    ed::BeginNode(uniqueId++);

                    w::label("Rule processor", w::emphasis::primary);
                    //w::sep();
                    w::label("pick browser");

                    int pin1 = uniqueId++;
                    ed::BeginPin(pin1, ed::PinKind::Input);
                    w::label(ICON_MD_INPUT, w::emphasis::primary);
                    ed::EndPin();

                    w::sl();

                    ed::BeginPin(uniqueId++, ed::PinKind::Output);
                    w::label(ICON_MD_OUTPUT, w::emphasis::error);
                    ed::EndPin();


                    ed::EndNode();
                    ed::PopStyleVar(2);

                    ed::Link(uniqueId++, pin0, pin1);

                }
            }
        }



        {
            w::status_bar sb;

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
        }


        if(show_demo)
            ImGui::ShowDemoWindow();

        return app_open;
    });

    return 0;
}
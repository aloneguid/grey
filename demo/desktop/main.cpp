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
    app->run([&app_open](const grey::app& app) {
        w::window wnd{"Hello, world!", &app_open};
        wnd
            .size(800, 600, app.scale)
            .no_focus()
            .render();

        // menu
        {
            w::menu_bar menu;
            if(menu) {

                if(ImGui::BeginMenu("File")) {

                    if(ImGui::MenuItem("New")) {
                        // do something
                    }

                    if(ImGui::BeginMenu("Recent")) {
                        if(ImGui::MenuItem("file1.txt")) {
                            // do something
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();

                }
            }
        }

        // labels
        w::label("simple text");
        w::label(ICON_FA_ADDRESS_BOOK " icon1");
        w::label(ICON_FA_CALENDAR " icon2");
        w::label(ICON_FA_CHROME " icon3");

        return app_open;
    });

    return 0;
}
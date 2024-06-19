//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"

using namespace std;

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

    //auto backend = grey::backend::make_platform_default(APP_LONG_NAME);
    //auto wnd = backend->make_window<demo::main_wnd>();
    //backend->run();

    auto app = grey::app::make(APP_LONG_NAME);
    app->run([]() {
        grey::window wnd{"Hello, world!", true};

        // menu

        ImGui::BeginMenuBar();
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

        ImGui::EndMenuBar();

        // labels
        grey::label("simple text");
        grey::label(ICON_FA_ADDRESS_BOOK " icon1");
        grey::label(ICON_FA_CALENDAR " icon2");
        grey::label(ICON_FA_CHROME " icon3");

        return true;
    });

    return 0;
}
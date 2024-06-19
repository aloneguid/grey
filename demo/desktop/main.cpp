#include "main_wnd.h"

using namespace std;

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

    auto backend = grey::backend::make_platform_default(APP_LONG_NAME);
    auto wnd = backend->make_window<demo::main_wnd>();
    backend->run();

    return 0;
}
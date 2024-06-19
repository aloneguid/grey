#include "main_wnd.h"
#include <fstream>
#include "../grey/3rdparty/imnodes/imnodes.h"

using namespace std;

namespace demo {

	main_wnd::main_wnd(grey::grey_context& ctx) : grey::window(ctx, APP_LONG_NAME, 800, 600) {
		detach_on_close = true;
		has_menu_space = true;

        auto menu = make_menu_bar();
        auto l0 = menu->items()->add("", "L0");
        auto l1 = l0->add("", "L1");
        auto l2 = l1->add("", "L2");
	}
}
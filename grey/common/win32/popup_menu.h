#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <vector>

namespace grey::common::win32 {
    class popup_menu {
    public:
        popup_menu(HWND h_wnd_owner);
        ~popup_menu();

        void add(const std::string& id, const std::string& title, bool disabled = false, bool checked = false);
        void separator();
        void enter_submenu(const std::string& title);
        void exit_submenu();

        // Shows menu to the user.
        // When an item is selected, owner window receives WM_COMMAND with LOWORD(wParam) set to WM_APP + item index.
        void show();

        // Clears menu by recreating it in place
        void clear();

        std::string id_from_loword_wparam(int loword_wparam);

    private:
        size_t count{ 0 };
        UINT_PTR next_id{ 0 };
        HMENU h_menu;
        HWND h_wnd_owner;
        std::map<int, std::string> loword_wparam_to_id;
        std::vector<HMENU> h_menu_nesting;
    };
}
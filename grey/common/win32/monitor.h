#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace grey::common::win32 {
    class monitor {
    public:
        size_t index;
        HMONITOR h;
        std::string name;
        RECT area;

        static std::vector<monitor> list();

        static monitor get_nearest(HWND hwnd);
    };
}

#pragma once
#include <string>
#include <windows.h>

namespace grey::common::win32
{
    class gdi {
    public:
        gdi();
        ~gdi();

        void ico_to_png(const std::string& ico_path, const std::string& png_path);

        unsigned char* ico_to_png(const std::string& ico_path, size_t& buf_size);

        void exe_to_png(const std::string& exe_path, const std::string& png_path);

        unsigned char* exe_to_png(const std::string& exe_path, size_t& buf_size);

    private:
        ULONG_PTR gdiplus_token;
        CLSID png_clsid;
    };
}
#pragma once
#include <string>
#include <windows.h>

namespace grey::common::win32 {
    /**
     * @brief GDI operations, lifetime is RAII managed.
     */
    class gdi {
    public:
        gdi();

        ~gdi();

        void ico_to_png(const std::string &ico_path, const std::string &png_path) const;

        unsigned char *ico_to_png(const std::string &ico_path, size_t &buf_size);

        void exe_to_png(const std::string &exe_path, const std::string &png_path);

        unsigned char *exe_to_png(const std::string &exe_path, size_t &buf_size);

        void save_to_png_file(HBITMAP hBitmap, const std::string &png_path) const;

    private:
        ULONG_PTR gdi_token{0};
        CLSID png_clsid{};
    };
}

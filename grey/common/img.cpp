#include "img.h"

#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "../3rdparty/stb_image.h"

#if _WIN32
#include <Windows.h>
#include "win32/gdi.h"
#endif

using namespace std;

namespace grey::common
{
#if _WIN32
    grey::common::win32::gdi g;
#endif

    raw_img load_image_from_file(const std::string& path) {
        int width, height;
        unsigned char* image_data;

        // stb_image does not support ICO, so we use GDI+ to convert it to PNG first
        if(path.ends_with(".ico")) {
#if _WIN32
            size_t size;
            unsigned char* buf = g.ico_to_png(path, size);
            image_data = stbi_load_from_memory(buf, size, &width, &height, nullptr, 4);
            delete buf;
#else
            image_data = nullptr;
            width = 0;
            height = 0;
#endif
        }
        // trick to extract main icon from .exe
        else if(path.ends_with(".exe")) {
#if _WIN32
            size_t size;
            unsigned char* buf = g.exe_to_png(path, size);
            image_data = stbi_load_from_memory(buf, size, &width, &height, nullptr, 4);
            delete buf;
#else
            image_data = nullptr;
            width = 0;
            height = 0;
#endif
        } else {
            image_data = stbi_load(path.c_str(), &width, &height, nullptr, 4);
        }

        return raw_img(image_data, width, height);
    }

    raw_img load_image_from_memory(unsigned char* buffer, unsigned int len) {
        int width, height;
        unsigned char* image_data = stbi_load_from_memory(buffer, len, &width, &height, nullptr, 4);

        return raw_img(image_data, width, height);
    }

    raw_img::~raw_img() {
        if(data) {
            stbi_image_free(data);
            data = nullptr;
        }
    }
}
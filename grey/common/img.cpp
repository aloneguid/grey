#include "img.h"

#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "../3rdparty/stb_image.h"

#if __APPLE__
#include <AppKit/AppKit.h>
#include <CoreGraphics/CoreGraphics.h>
#include <cstdlib>
#include <filesystem>
#endif

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

#if __APPLE__
    raw_img load_apple_icon(const std::string& path) {
        @autoreleasepool {
            std::error_code ec;
            const bool source_exists = path.ends_with(".app")
                                           ? std::filesystem::is_directory(path, ec)
                                           : std::filesystem::is_regular_file(path, ec);
            if(!source_exists) return raw_img(nullptr, 0, 0);

            NSString *file_path = [NSString stringWithUTF8String:path.c_str()];
            if(!file_path) return raw_img(nullptr, 0, 0);

            NSImage *image = nil;
            bool owns_image = false;
            if(path.ends_with(".app")) {
                image = [[NSWorkspace sharedWorkspace] iconForFile:file_path];
            } else {
                image = [[NSImage alloc] initWithContentsOfFile:file_path];
                owns_image = true;
            }

            if(!image) return raw_img(nullptr, 0, 0);

            NSRect proposed_rect = NSMakeRect(0, 0, 128, 128);
            CGImageRef source = [image CGImageForProposedRect:&proposed_rect context:nil hints:nil];
            const size_t width = source ? CGImageGetWidth(source) : 0;
            const size_t height = source ? CGImageGetHeight(source) : 0;
            if(!source || width == 0 || height == 0) {
                if(owns_image) [image release];
                return raw_img(nullptr, 0, 0);
            }

            unsigned char *image_data = static_cast<unsigned char *>(malloc(width * height * 4));
            if(!image_data) {
                if(owns_image) [image release];
                return raw_img(nullptr, 0, 0);
            }

            CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
            const CGBitmapInfo bitmap_info =
                static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedLast) | kCGBitmapByteOrder32Big;
            CGContextRef context = CGBitmapContextCreate(
                image_data,
                width,
                height,
                8,
                width * 4,
                color_space,
                bitmap_info);
            if(!context) {
                CGColorSpaceRelease(color_space);
                free(image_data);
                if(owns_image) [image release];
                return raw_img(nullptr, 0, 0);
            }

            CGContextTranslateCTM(context, 0, static_cast<CGFloat>(height));
            CGContextScaleCTM(context, 1, -1);
            CGContextDrawImage(context, CGRectMake(0, 0, width, height), source);
            CGContextRelease(context);
            CGColorSpaceRelease(color_space);
            if(owns_image) [image release];

            return raw_img(image_data, width, height);
        }
    }
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
        } else if(path.ends_with(".app") || path.ends_with(".icns")) {
#if __APPLE__
            return load_apple_icon(path);
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

    raw_img load_image_from_memory(const unsigned char* buffer, unsigned int len) {
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
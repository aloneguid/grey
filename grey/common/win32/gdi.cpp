#include "gdi.h"
#include <Windows.h>
#include <gdiplus.h>
#include "../str.h"

#pragma comment(lib, "gdiplus.lib")

namespace grey::common::win32
{
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
        UINT  num = 0;          // number of image encoders
        UINT  size = 0;         // size of the image encoder array in bytes

        Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

        Gdiplus::GetImageEncodersSize(&num, &size);
        if(size == 0)
            return -1;  // Failure

        pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
        if(pImageCodecInfo == NULL)
            return -1;  // Failure

        GetImageEncoders(num, size, pImageCodecInfo);

        for(UINT j = 0; j < num; ++j) {
            if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return j;  // Success
            }
        }

        free(pImageCodecInfo);
        return -1;  // Failure
    }

    unsigned char* get_raw_bytes(Gdiplus::Image* img, size_t& size, CLSID* clsid) {
        IStream* istream = nullptr;
        HRESULT hr = ::CreateStreamOnHGlobal(NULL, TRUE, &istream);
        if(!SUCCEEDED(hr)) return nullptr;

        Gdiplus::Status save_status = img->Save(istream, clsid, NULL);

        // get memory handle associated with istream
        HGLOBAL hg;
        HRESULT hgok = ::GetHGlobalFromStream(istream, &hg);

        //copy IStream to buffer
        size = ::GlobalSize(hg);
        unsigned char* buffer = new unsigned char[size];

        // lock & unlock memory
        LPVOID ptr = ::GlobalLock(hg);
        memcpy(buffer, ptr, size);
        GlobalUnlock(hg);

        istream->Release();
        return buffer;
    }

    HICON extract_exe_main_icon(const std::string& exe_path) {
        HICON hIcon;
        // https://github.com/zumoshi/BrowserSelect/blob/master/BrowserSelect/IconExtractor.cs
        UINT piconid;
        UINT xt_r = ::PrivateExtractIcons(str::to_wstr(exe_path).c_str(), 0, 256, 256, &hIcon, &piconid, 1, 0);
        return hIcon;
    }

    gdi::gdi() {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplus_token, &gdiplusStartupInput, NULL);

        GetEncoderClsid(L"image/png", &png_clsid);
    }

    gdi::~gdi() {
        Gdiplus::GdiplusShutdown(gdiplus_token);
    }

    void gdi::ico_to_png(const std::string& ico_path, const std::string& png_path) {
        Gdiplus::Image* ico = Gdiplus::Image::FromFile(str::to_wstr(ico_path).c_str());

        ico->Save(str::to_wstr(png_path).c_str(), &png_clsid, NULL);

        delete ico;
    }

    HICON extract_ico_highest_res(const std::string& ico_path) {
        const int sizes[] = {256, 128, 96, 64, 48, 32, 16};
        HICON bestIcon = nullptr;
        int bestSize = 0;

        for(int size : sizes) {
            HICON hIcon = nullptr;
            UINT piconid = 0;
            UINT result = ::PrivateExtractIcons(
                str::to_wstr(ico_path).c_str(),
                0, size, size,
                &hIcon, &piconid, 1, 0
            );

            if(result > 0 && hIcon) {
                ICONINFO iconInfo;
                if(::GetIconInfo(hIcon, &iconInfo)) {
                    BITMAP bm;
                    ::GetObject(iconInfo.hbmColor ? iconInfo.hbmColor : iconInfo.hbmMask, sizeof(bm), &bm);

                    if(iconInfo.hbmColor) ::DeleteObject(iconInfo.hbmColor);
                    if(iconInfo.hbmMask) ::DeleteObject(iconInfo.hbmMask);

                    if(bm.bmWidth > bestSize) {
                        if(bestIcon) ::DestroyIcon(bestIcon);
                        bestIcon = hIcon;
                        bestSize = bm.bmWidth;
                    } else {
                        ::DestroyIcon(hIcon);
                    }
                } else {
                    ::DestroyIcon(hIcon);
                }
            }
        }

        return bestIcon;
    }

    unsigned char* gdi::ico_to_png(const std::string& ico_path, size_t& buf_size) {
        HICON hIcon = extract_ico_highest_res(ico_path);

        if(!hIcon) {
            // Fallback: use GDI+ (may be low-res)
            Gdiplus::Image* ico = Gdiplus::Image::FromFile(str::to_wstr(ico_path).c_str());
            if(!ico) return nullptr;
            unsigned char* buf = get_raw_bytes(ico, buf_size, &png_clsid);
            delete ico;
            return buf;
        }

        Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromHICON(hIcon);
        unsigned char* buf = get_raw_bytes(bmp, buf_size, &png_clsid);
        delete bmp;
        ::DestroyIcon(hIcon);

        return buf;
    }

    void gdi::exe_to_png(const std::string& exe_path, const std::string& png_path) {
        HICON hIcon = extract_exe_main_icon(exe_path);

        Gdiplus::Bitmap* x = Gdiplus::Bitmap::FromHICON(hIcon);

        x->Save(str::to_wstr(png_path).c_str(), &png_clsid, NULL);

        delete x;

        ::DestroyIcon(hIcon);
    }

    unsigned char* gdi::exe_to_png(const std::string& exe_path, size_t& buf_size) {
        HICON hIcon = extract_exe_main_icon(exe_path);

        Gdiplus::Bitmap* x = Gdiplus::Bitmap::FromHICON(hIcon);

        unsigned char* buf = get_raw_bytes(x, buf_size, &png_clsid);

        delete x;

        ::DestroyIcon(hIcon);

        return buf;
    }
}
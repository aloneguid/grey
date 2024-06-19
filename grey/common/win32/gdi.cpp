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

    unsigned char* gdi::ico_to_png(const std::string& ico_path, size_t& buf_size) {
        Gdiplus::Image* ico = Gdiplus::Image::FromFile(str::to_wstr(ico_path).c_str());
        if(!ico) return nullptr;

        unsigned char* buf = get_raw_bytes(ico, buf_size, &png_clsid);

        delete ico;

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
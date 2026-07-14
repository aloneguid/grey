#include "clipboard.h"
#include "platform.h"
#include "str.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#elif PLATFORM_LINUX
#include <cstdio>
#endif

using namespace std;

namespace grey::common::clipboard {

#if PLATFORM_WINDOWS
    void set_text(const std::string& text) {
        if(!::OpenClipboard(nullptr)) return;
        if(!::EmptyClipboard()) {
            ::CloseClipboard();
            return;
        }
        HGLOBAL gh = ::GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if(!gh) {
            ::CloseClipboard();
            return;
        }
        memcpy(::GlobalLock(gh), text.c_str(), text.size() + 1);
        ::GlobalUnlock(gh);
        HANDLE hResult = ::SetClipboardData(CF_TEXT, gh);
        ::CloseClipboard();
        if(!hResult) {
            ::GlobalFree(gh);
        }
    }

    std::string get_text() {
        if(!::IsClipboardFormatAvailable(CF_UNICODETEXT)) return "";
        if(!OpenClipboard(nullptr)) return "";
        wstring r;

        HGLOBAL gh = ::GetClipboardData(CF_UNICODETEXT);
        if(gh != NULL) {
            LPWSTR lpstr = (LPWSTR)::GlobalLock(gh);
            if(lpstr != NULL) {

                r = lpstr;

                ::GlobalUnlock(lpstr);
            }
        }

        ::CloseClipboard();

        return str::to_str(r);
    }

#elif PLATFORM_LINUX

    void set_text(const std::string& text) {
        FILE* pipe = popen("xclip -selection clipboard", "w");
        if (pipe) {
            fwrite(text.c_str(), 1, text.size(), pipe);
            pclose(pipe);
        }
    }

    std::string get_text() {
        std::string out;
        FILE* pipe = popen("xclip -selection clipboard -o", "r");
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                out += buffer;
            }
            pclose(pipe);
        }
        return out;
    }

#endif

}
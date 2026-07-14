#include "clipboard.h"
#include "platform.h"
#include "str.h"

#if PLATFORM_WINDOWS
#include <windows.h>
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

#endif

}
#include "user.h"
#include <WinUser.h>
#include "../str.h"
#include "reg.h"

using namespace std;

#define MAX_LOADSTRING 1024

// Version.lib is required to call ::GetFileVersionInfoSize etc.
#pragma comment(lib, "Version.lib")

namespace grey::common::win32::user {
    std::wstring load_string(HINSTANCE hInst, UINT id) {
        WCHAR szs[MAX_LOADSTRING];
        ::LoadString(hInst, id, szs, MAX_LOADSTRING);
        return wstring(szs);
    }

    std::wstring get_file_version_info_string(
        const std::string& module_path,
        const std::string& string_name) {

        wstring wmp = str::to_wstr(module_path);
        DWORD v_size = ::GetFileVersionInfoSize(wmp.c_str(), nullptr);
        unique_ptr<char[]> v_data(new char[v_size]);    // can be vector<char> as well

        if (::GetFileVersionInfo(wmp.c_str(), 0, v_size, v_data.get())) {

            struct LANGANDCODEPAGE {
                WORD wLanguage;
                WORD wCodePage;
            } *lpTranslate;

            UINT cbTranslate;
            ::VerQueryValue(v_data.get(), L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate);

            wchar_t buffer[256];
            swprintf_s(buffer, 
                L"\\StringFileInfo\\%04x%04x\\%S",
                lpTranslate[0].wLanguage, lpTranslate[0].wCodePage, string_name.c_str());
            wchar_t* value;

            UINT dwBytes;
            if (::VerQueryValue(v_data.get(), buffer, (LPVOID*)&value, &dwBytes)) {
                return wstring(value);
            }
        }

        return L"";
    }

    std::wstring load_version(bool full) {
        wstring r;

        // get current module's filename
        TCHAR szVersionFile[MAX_PATH];
        ::GetModuleFileName(nullptr, szVersionFile, MAX_PATH);

        DWORD hVer = 0;
        DWORD verSize = ::GetFileVersionInfoSize(szVersionFile, &hVer);
        LPSTR verData = new char[verSize];

        if (::GetFileVersionInfo(szVersionFile, hVer, verSize, verData)) {
            LPBYTE lpBuffer = 0;
            UINT size = 0;
            if (VerQueryValue(verData, L"\\", (VOID FAR * FAR*) & lpBuffer, &size)) {
                if (size) {
                    VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd) {
                        // Doesn't matter if you are on 32 bit or 64 bit,
                        // DWORD is always 32 bits, so first two revision numbers
                        // come from dwFileVersionMS, last two come from dwFileVersionLS

                        r = to_wstring((verInfo->dwFileVersionMS >> 16) & 0xffff) + L"." +
                            to_wstring((verInfo->dwFileVersionMS >> 0) & 0xffff);

                        if (full) {
                            r += L"." +
                                to_wstring((verInfo->dwFileVersionLS >> 16) & 0xffff) + L"." +
                                to_wstring((verInfo->dwFileVersionLS >> 0) & 0xffff);
                        }
                    }
                }
            }

            delete[] verData;
        }

        return r;

    }

    void set_window_pos(HWND hwnd, int x, int y, int width, int height) {
        UINT flags{};

        if (x == -1 || y == -1) flags |= SWP_NOMOVE;
        if (width == -1 || height == -1) flags |= SWP_NOSIZE;

        ::SetWindowPos(hwnd, NULL, x, y, width, height, flags);
    }

    bool is_kbd_ctrl_down() {
        // https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

        return ::GetKeyState(VK_CONTROL) & 0x8000;
    }

    bool is_kbd_alt_down() {
        return (::GetKeyState(VK_LMENU) & 0x8000) || (::GetKeyState(VK_RMENU) & 0x8000);
    }

    bool is_kbd_shift_down() {
        return ::GetKeyState(VK_SHIFT) & 0x8000;
    }

    bool is_kbd_caps_locks_on() {
        return (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    }

    bool is_app_light_theme() {

        string s = win32::reg::get_value(win32::reg::hive::current_user,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            "AppsUseLightTheme");

        return s.empty() || s == "1";
    }

    bool is_system_light_theme() {
        string s = win32::reg::get_value(win32::reg::hive::current_user,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            "SystemUsesLightTheme");

        return s.empty() || s == "1";
    }

    void message_box(const std::string& caption, const std::string& text) {
        ::MessageBoxA(nullptr, text.c_str(), caption.c_str(), MB_OK);
    }
}
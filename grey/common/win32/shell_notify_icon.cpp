#include "shell_notify_icon.h"
#include <commctrl.h>
#include <strsafe.h>
#include "../str.h"

// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

using namespace std;

namespace grey::common::win32 {
    shell_notify_icon::shell_notify_icon(HWND hwnd, GUID icon_guid, UINT callback_message, const string& tooltip_text) 
        : hwnd{ hwnd }, callback_message{ callback_message } {

        if(icon_guid == GUID_NULL)
            if(!SUCCEEDED(::CoCreateGuid(&icon_guid)))
                return;

        this->icon_guid = icon_guid;

        // create icon
        // https://www.codeproject.com/Articles/4768/Basic-use-of-Shell-NotifyIcon-in-Win32
        // https://github.com/microsoft/Windows-classic-samples/blob/67a8cddc25880ebc64018e833f0bf51589fd4521/Samples/Win7Samples/winui/shell/appshellintegration/NotificationIcon/NotificationIcon.cpp#L88

        HINSTANCE hModule = ::GetModuleHandle(NULL);

        NOTIFYICONDATA nid = { sizeof(nid) };
        nid.hWnd = hwnd;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
        nid.guidItem = this->icon_guid;
        nid.uCallbackMessage = callback_message;
        ::StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), str::to_wstr(tooltip_text).c_str());
        ::LoadIconMetric(hModule, L"IDI_ICON1", LIM_SMALL, &nid.hIcon);
        if(!::Shell_NotifyIcon(NIM_ADD, &nid)) {
            return;
        }

        nid.uVersion = NOTIFYICON_VERSION_4;
        ::Shell_NotifyIcon(NIM_SETVERSION, &nid);
    }

    shell_notify_icon::~shell_notify_icon() {
        NOTIFYICONDATA nid = { sizeof(nid) };
        nid.uFlags = NIF_GUID;
        nid.guidItem = icon_guid;
        ::Shell_NotifyIcon(NIM_DELETE, &nid);
    }

    /*
    bool shell_notify_icon::set_tooptip(const std::string& text) {
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd;
        nid.uFlags = NIF_GUID | NIF_TIP;
        nid.guidItem = icon_guid;
        ::StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), str::to_wstr(text).c_str());
        return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
    }*/

    void shell_notify_icon::display_notification(const std::string& title, const std::string& text) {

        // Display a balloon notification. The szInfo, szInfoTitle, dwInfoFlags, and uTimeout members are valid.

        NOTIFYICONDATA nid = {sizeof(nid)};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uFlags = NIF_GUID | NIF_INFO;
        nid.guidItem = icon_guid;
        ::StringCchCopy(nid.szInfo, ARRAYSIZE(nid.szInfo), str::to_wstr(text).c_str());
        ::StringCchCopy(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), str::to_wstr(title).c_str());
        nid.dwInfoFlags = NIIF_INFO;
        ::Shell_NotifyIcon(NIM_MODIFY, &nid);
    }

    void shell_notify_icon::update_icon(LPCWSTR pszName) {
        NOTIFYICONDATA nid = {sizeof(nid)};
        nid.hWnd = hwnd;
        nid.uFlags = NIF_GUID | NIF_ICON;
        nid.guidItem = icon_guid;

        HINSTANCE hModule = ::GetModuleHandle(NULL);
        ::LoadIconMetric(hModule, pszName, LIM_SMALL, &nid.hIcon);
        ::Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
}
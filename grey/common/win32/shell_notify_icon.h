#pragma once
#include <Windows.h>
#include <string>

namespace grey::common::win32 {
    class shell_notify_icon {
    public:
        /**
         * @brief Creates and shows shell notification icon
         * @param hwnd Window that owns this icon
         * @param icon_guid Unique icon GUID. If you don't set it to the same one every time, windows shell history will grow with duplicate icons!
         * @param callback_message Message to receive back to owning window.
        */
        shell_notify_icon(HWND hwnd,
            GUID icon_guid = GUID_NULL,
            UINT callback_message = WM_APP + 1,
            const std::string& tooltip_text = "");
        ~shell_notify_icon();

        //bool set_tooptip(const std::string& text);

        void display_notification(const std::string& title, const std::string& text);

        /**
         * @brief Updates icon by specifying a new icon name. When constructed, icon is loaded with "IDI_ICON1" name;
         * @param pszName 
         */
        void update_icon(LPCWSTR pszName);


    private:
        HWND hwnd;
        GUID icon_guid{GUID_NULL};
        UINT callback_message;
    };
}
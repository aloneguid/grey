#pragma once
#include <string>

namespace grey::common::win32 {
    class uwp {
    public:
        uwp();
        ~uwp();

        // IApplicationActivationManager

        bool start_app(const std::wstring& app_user_mode_id, const std::wstring& arg);
        bool open_url(const std::wstring& app_user_mode_id, const std::wstring& uri);
        bool open_file(const std::wstring& app_user_mode_id, const std::wstring& file_path);

        // ILauncherStatics

        /**
         * @brief Launches the URI using the ILauncherStatics COM interface.
         * @param app_package_family_name The package family name of the application to launch the URI with. Example: "TheBrowserCompany.Arc_ttt1ap7aakyb4"
         * @param uri The URI to launch. Example: "https://www.google.com"
         * @return 
         */
        bool launch_uri(const std::string& app_package_family_name, const std::string& uri) const;

    private:
        bool initialised;
    };
}
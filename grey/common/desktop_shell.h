#pragma once
#include <string>

namespace grey::common {
    class desktop_shell {
    public:
        static unsigned int get_current_monitor_dpi();

        static void open_default_apps_configuration();

        /**
         * @brief Call file open dialog system-native to select a file
         * @param file_type_name Display name of the file type, for example "Word Document"
         * @param extension Extension of the file type, for example "*.doc". You can pass multiple extensions, for instance "*.doc;*.docx"
         * @return Path to the selected file, or empty string if the user canceled the dialog
         */
        static std::string file_open_dialog(const std::string &file_type_name, const std::string &extension);

        static bool file_open_dialog_supported();


    private:
        desktop_shell() = delete;
    };
}
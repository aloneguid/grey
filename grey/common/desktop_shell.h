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

        /**
         * @brief Call directory open dialog system-native to select a folder
         * @return Path to the selected folder, or empty string if the user canceled the dialog
         */
        static std::string directory_open_dialog();

        static bool directory_open_dialog_supported();

        /**
         * Opens URL or path with the default UI shell.
         * Uses:
         * - Linux: xdg-open path
         * - Windows: ShellExecute::open path
         * @param path Program name, URL or path, whatever the shell can handle.
         * @return
         */
        static bool open(const std::string& path, const std::string& args = "");

    private:
        desktop_shell() = delete;
    };
}
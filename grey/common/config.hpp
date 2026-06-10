#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <SimpleIni.h> // https://github.com/brofield/simpleini
#include "fss.h"

namespace grey::common {

    /**
     * Specifically designed for immediate mode
     */
    class config {
    public:
        config(const std::string& application_name, const std::string& file_name = "config.ini") {
            bool is_portable = std::filesystem::exists(std::filesystem::path{fss::get_current_dir()} / ".portable");
            if(is_portable) {
                file_path = (std::filesystem::path{fss::get_current_dir()} / file_name).string();
            } else {
                file_path = (std::filesystem::path{fss::get_config_dir()} / application_name / file_name).string();
            }

            // create dir if not exists
            std::filesystem::path abs{file_path};
            std::filesystem::create_directories(abs.parent_path());

            ini.SetMultiKey(true);
            ini.SetMultiLine(true);
        }

#define X(type, name, default_val) type name{default_val};
    #include "config.def"
#undef X

        ~config() {
            commit();
        }

        void tick() {}

        void reload() {

        }

        void commit() {

        }

    private:
        std::string file_path;
        CSimpleIni ini;
    };
}
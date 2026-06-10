#pragma once
#include <string>
#include <filesystem>
#include <SimpleIni.h> // https://github.com/brofield/simpleini

#include "fss.h"

namespace grey::common {

    /**
     * Specifically designed for immediate mode. To use, define lines of
     *
     * X(type, name, default_val)
     *
     * in config.def in the same folder you are including config.hpp.
     */
    class config {
    public:
        using string = std::string;

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

            reload();
        }

#define X(type, name, default_val) type name{default_val};
    #include "config.def"
#undef X

        ~config() {
            commit();
        }

        void tick() {}

        void reload() {
            ini.Reset();
            ini.LoadFile(file_path.c_str());

#define X(type, name, default_val) name = get_##type##_value(#name, default_val);
    #include "config.def"
#undef X
        }

        void commit() {
#define X(type, name, default_val) set_##type##_value(#name, name);
    #include "config.def"
#undef X
            ini.SaveFile(file_path.c_str());
        }

    private:
        std::string file_path;
        CSimpleIni ini;

        void set_string_value(const std::string& key, const std::string& value, const std::string& section = "") {
            ini.Delete(section.c_str(), key.c_str());
            if(!value.empty()) {
                ini.SetValue(section.c_str(), key.c_str(), value.c_str());
            }
        }

        string get_string_value(const std::string& key, const std::string& default_value, const std::string& section = "") {
            const char* v = ini.GetValue(section.c_str(), key.c_str());
            return v ? v : "";
        }

        void set_bool_value(const std::string& key, bool value, const std::string& section = "") {
            ini.SetBoolValue(section.c_str(), key.c_str(), value, nullptr, true);
        }

        bool get_bool_value(const std::string& key, bool default_value, const std::string& section = "") {
            string value = get_string_value(key, "", section);
            if(value.empty()) return default_value;
            return value == "true" || value == "y" || value == "1";
        }

    };
}
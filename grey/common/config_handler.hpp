#pragma once
#include <SimpleIni.h> // https://github.com/brofield/simpleini
#include <list>
#include "imgui.h"
#include "fss.h"
#include <filesystem>

namespace grey::common {
    class config_handler {
    public:
        using uint = unsigned int;
        using string = std::string;
        using strings = std::vector<std::string>;
        using colour = unsigned int;

        config_handler(const std::string& application_name, const std::string& file_name = "config.ini") {
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

        void reload() {
            ini.Reset();
            ini.LoadFile(file_path.c_str());
        }

        void commit() {
            ini.SaveFile(file_path.c_str());
        }

        std::vector<std::string> list_sections() const {
            std::vector<string> r;
            std::list<CSimpleIni::Entry> ir;
            ini.GetAllSections(ir);
            for (auto& s : ir) {
                r.push_back(s.pItem);
            }
            return r;
        }

        void delete_section(const std::string& section) {
            ini.Delete(section.c_str(), nullptr, true);
        }


        // save/ load functions
        void save_string_value(const std::string &key, const std::string &value, const std::string &section = "") {
            ini.Delete(section.c_str(), key.c_str());
            if(!value.empty()) {
                ini.SetValue(section.c_str(), key.c_str(), value.c_str());
            }
        }

        string load_string_value(const std::string &key, const std::string &default_value,
                                const std::string &section = "") {
            const char *v = ini.GetValue(section.c_str(), key.c_str());
            return v ? v : default_value;
        }

        void save_strings_value(const std::string &key, const strings &value, const std::string &section = "") {
            ini.Delete(section.c_str(), key.c_str());
            for(const auto &s: value) {
                ini.SetValue(section.c_str(), key.c_str(), s.c_str());
            }
        }

        strings load_strings_value(const std::string &key, const strings &default_value,
                                  const std::string &section = "") {
            strings r;
            std::list<CSimpleIni::Entry> ir;
            ini.GetAllValues(section.c_str(), key.c_str(), ir);
            for(auto &e: ir) {
                r.push_back(e.pItem);
            }
            return r;
        }

        void save_int_value(const std::string &key, int value, const std::string &section = "") {
            ini.SetLongValue(section.c_str(), key.c_str(), value, nullptr, false, true);
        }

        void save_uint_value(const std::string &key, uint value, const std::string &section = "") {
            ini.SetLongValue(section.c_str(), key.c_str(), value, nullptr, false, true);
        }

        int load_int_value(const std::string &key, int default_value, const std::string &section = "") {
            return ini.GetLongValue(section.c_str(), key.c_str(), default_value);
        }

        uint load_uint_value(const std::string &key, uint default_value, const std::string &section = "") {
            return static_cast<uint>(ini.GetLongValue(section.c_str(), key.c_str(), default_value));
        }

        void save_float_value(const std::string &key, float value, const std::string &section = "") {
            ini.SetDoubleValue(section.c_str(), key.c_str(), value, nullptr, true);
        }

        float load_float_value(const std::string &key, float default_value, const std::string &section = "") {
            return ini.GetDoubleValue(section.c_str(), key.c_str(), default_value);
        }

        void save_bool_value(const std::string &key, bool value, const std::string &section = "") {
            ini.SetBoolValue(section.c_str(), key.c_str(), value, nullptr, true);
        }

        bool load_bool_value(const std::string &key, bool default_value, const std::string &section = "") {
            string value = load_string_value(key, "", section);
            if(value.empty()) return default_value;
            return value == "true" || value == "y" || value == "1";
        }

        unsigned int load_colour_value(const std::string &key, unsigned int default_value,
                                      const std::string &section = "") {
            string value = load_string_value(key, "", section);
            if(value.empty()) return default_value;
            return hex_to_imcol(value);
        }

        void save_colour_value(const std::string &key, unsigned int value, const std::string &section = "") {
            save_string_value(key, imcol_to_hex(value), section);
        }

    private:
        std::string file_path;
        CSimpleIni ini;

        unsigned int hex_to_imcol(const std::string &hex) {
            std::string h = hex;

            // Remove # prefix if present
            if(!h.empty() && h[0] == '#') {
                h = h.substr(1);
            }

            // Parse hex string (expects RRGGBB or RRGGBBAA format)
            if(h.length() == 6) {
                try {
                    unsigned long value = std::stoul(h, nullptr, 16);
                    // RGB format - add full alpha
                    float r = ((value >> 16) & 0xFF) / 255.0f;
                    float g = ((value >> 8) & 0xFF) / 255.0f;
                    float b = (value & 0xFF) / 255.0f;

                    ImColor ic{r, g, b};
                    return ic;
                } catch(...) {
                    return 0;
                }
            }

            return 0;
        }

        std::string imcol_to_hex(unsigned int color) {
            ImColor ic{color};
            return std::format("#{:02X}{:02X}{:02X}", (int) (ic.Value.x * 255), (int) (ic.Value.y * 255),
                               (int) (ic.Value.z * 255));
        }
    };
}

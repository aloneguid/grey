#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include "config_handler.hpp"

#if __has_include("model.h")
#include "model.h"
#endif

#if __has_include("config_custom.h")
#include "config_custom.h"
#endif

namespace grey::common {

    /**
     * Specifically designed for immediate mode. To use, define lines of
     *
     * opt(type, name, default_val)
     * or for sections
     * opt_sect(type, name, default_val, sect_name, sect_val_name). sect_val_name is value name inside the section
     *
     * in config.def in the same folder you are including config.hpp. Types supported:
     * int, float, string, bool, colour (saved as unsigned int).
     */
    class config {
    public:
        using uint = unsigned int;
        using string = std::string;
        using strings = std::vector<std::string>;
        using colour = unsigned int;

        config(const std::string& application_name, const std::string& file_name = "config.ini") : ini{application_name, file_name} {
            reload();
        }

        // declare config members
#define opt(type, name, default_val) type name{default_val};
#define opt_sect(type, name, default_val, sect_name, sect_val_name) type name{default_val};
#define opt_custom(type, name) type name;
#define opt_enum(type, name, default_val, base_type) type name{default_val};
    #include "config.def"
#undef opt_enum
#undef opt_custom
#undef opt_sect
#undef opt

        ~config() {
            commit();
        }

        void tick() {}

        void reload() {
            ini.reload();

            // loading logic
#define opt(type, name, default_val) name = ini.load_##type##_value(#name, default_val);
#define opt_sect(type, name, default_val, sect_name, sect_val_name) name = ini.load_##type##_value(#sect_val_name, default_val, #sect_name);
#define opt_custom(type, name) load_custom(ini, browsers);
#define opt_enum(type, name, default_val, base_type) base_type base_##name = ini.load_##base_type##_value(#name, default_val); name = static_cast<type>(base_##name);
    #include "config.def"
#undef opt_enum
#undef opt_custom
#undef opt_sect
#undef opt
        }

        void commit() {
            // saving logic
#define opt(type, name, default_val) ini.save_##type##_value(#name, name);
#define opt_sect(type, name, default_val, sect_name, sect_val_name) ini.save_##type##_value(#sect_val_name, name, #sect_name);
#define opt_custom(type, name) save_custom(ini, browsers);
#define opt_enum(type, name, default_val, base_type) ini.save_##base_type##_value(#name, static_cast<base_type>(name));
    #include "config.def"
#undef opt_enum
#undef opt_custom
#undef opt_sect
#undef opt
            ini.commit();
        }

    private:
        config_handler ini;

    };
}
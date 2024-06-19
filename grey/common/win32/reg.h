#pragma once
#include <string>
#include <vector>
#include <map>

namespace grey::common::win32::reg
{
    enum class hive {
        classes_root = 0,

        local_machine,

        current_user,

    };

    std::vector<std::string> enum_subkeys(hive h, std::string path);

    std::vector<std::string> get_value_names(hive h, std::string path);

    std::string get_value(hive h, const std::string& path, const std::string& value_name = "");

    std::vector<std::string> get_multi_value(hive h, const std::string& path, const std::string& value_name = "");

    void delete_key(hive h, const std::string& path);

    void delete_value(hive h, const std::string& path, const std::string& value_name = "");

    void set_value(hive h,
       const std::string& path,
       const std::string& value,
       const std::string& value_name = "");

    void set_value(hive h,
       const std::string& path,
       int32_t value,
       const std::string& value_name = "");

    void set_value(hive h,
       const std::string& path,
       const std::vector<std::string>& value,
       const std::string& value_name = "");
}
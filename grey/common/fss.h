#pragma once
#include <string>

namespace grey::common::fss {
    std::string get_current_dir();

    /**
     * Gets user home directory
     * @return
     */
    std::string get_home_dir();

    /**
     *
     * @return Get a configuration directory that can be used to store user settings
     */
    std::string get_config_dir();

    /**
     * Get a path to a configuration file in the configuration directory
     */
    std::string get_config_file_path(const std::string& application_name, const std::string& filename);

    std::string get_current_exec_path();

    bool file_exists(const std::string &name);

    size_t get_file_size(const std::string &name);

    std::string get_full_path(const std::string &path);

    bool read_binary_file(const std::string &name, unsigned char *buffer);

    /**
     * @brief Reads file as string, returns empty string on error.
     * @param name
     * @return
     */
    std::string read_all_text(const std::string &name);

    // renamed to above function, for consistency
    //std::string read_file_as_string(const std::string& name) { return read_all_text(name); }

    /**
     * @brief Writes all text to file, overwriting if exists.
     * @param filename
     * @param contents
     */
    void write_all_text(const std::string &filename, const std::string &contents);

    /**
     * @brief Writes all text to file, appending if exists.
     * @param filename
     * @param contents
     */
    void append_all_text(const std::string &filename, const std::string &contents);

    /**
     * @brief Calculates file age in seconds, from the creation time.
     * @param filename
     * @return
     */
    unsigned int get_age_in_seconds(const std::string &filename);

    std::string get_temp_file_path(const std::string &prefix = "TMP");

    bool delete_file(const std::string &path);
}

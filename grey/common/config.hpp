#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <type_traits>
#include <fkYAML/node.hpp>
#include "fss.h"
#include "imgui.h"
#include <iostream>

namespace grey::common {

    /**
     *
     * @tparam T Value type
     * @param node YAML node
     * @param key key to read
     * @param default_value default value to return if key is not found.
     * @return The value of the key in the yaml node.
     */
    template<typename T>
    T read(const fkyaml::node &node, const std::string &key, const T& default_value) {
        if(!node.contains(key)) {
            return default_value;
        }
        const fkyaml::node &value = node[key];

        try {
            return value.get_value<T>();
        } catch(const fkyaml::exception &ex) {
            return default_value; // wrong type, or conversion failed
        }
    }

    template<typename T>
    T read(const fkyaml::node &node, const std::string &key) {
        T result{};
        try {
            const fkyaml::node &value = node.contains(key) ? node[key] : fkyaml::node::mapping();
            from_node(value, result);
            return result;
        } catch(...) {

        }
        return result;
    }

    template<typename T>
    static void write(fkyaml::node &n, const std::string &key, const T &value) {
        n[key] = value;
    }

    static unsigned int hex_str_to_imgui_col(const std::string &hex) {
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

    static std::string imgui_col_to_hex_str(unsigned int color, bool prepend_hash = true) {
        ImColor ic{color};
        std::string hex = std::format("{}{:02X}{:02X}{:02X}",
            prepend_hash ? "#" : "",
            (int) (ic.Value.x * 255), (int) (ic.Value.y * 255),
                                      (int) (ic.Value.z * 255));
        return hex;
    }

    template<typename TState>
    class config {
    public:
        explicit config(TState &state, const std::string &application_name, float flush_interval = 1.0f) : state{state},
            flush_interval{flush_interval} {
            file_path = (std::filesystem::path{fss::get_config_dir(application_name)} / "config.yml").
                    string();

            // create dir if not exists
            const std::filesystem::path abs{file_path};
            std::filesystem::create_directories(abs.parent_path());

            deserialize();
            prev_state = state;
            last_write_time = get_last_write_time();
        }

        ~config() {
            serialize();
        }

        std::filesystem::file_time_type get_last_write_time() const {
            return std::filesystem::exists(file_path)
                       ? std::filesystem::last_write_time(file_path)
                       : std::filesystem::file_time_type{};
        }

        void serialize() {
            to_node(root, state);
            std::ofstream ofs{file_path, std::ios::out};
            ofs << root;
        }

        void deserialize() {
            std::ifstream ifs{file_path, std::ios::in};
            const bool is_valid = static_cast<bool>(ifs);
            root = is_valid ? fkyaml::node::deserialize(ifs) : fkyaml::node::mapping();
            if(root.get_type() != fkyaml::node_type::MAPPING)
                root = fkyaml::node::mapping();
            from_node(root, state);
        }

        TState &get_state() {
            return state;
        }

        bool tick(float delta_time) {
            last_flushed_ago += delta_time;
            bool changed{false};

            if(last_flushed_ago > flush_interval) {
                // check if our version is old
                auto new_last_write_time = get_last_write_time();
                if(new_last_write_time > last_write_time) {
                    from_node(root, state);
                    last_write_time = new_last_write_time;
                    prev_state = state;
                    changed = true;
                } else {
                    // otherwise check if we need to dump the state
                    if(state != prev_state) {
                        to_node(root, state);
                        prev_state = state;
                        last_write_time = get_last_write_time();
                        changed = true;
                    }
                }
                last_flushed_ago = 0.f; // otherwise we'll keep comparing forever
            }

            return changed;
        }

    private:
        std::string file_path;
        fkyaml::node root;
        TState &state;
        TState prev_state;
        float flush_interval;
        std::filesystem::file_time_type last_write_time;
        float last_flushed_ago{0.f};
    };
}

#pragma once
#include <string>
#include <format>
#include <filesystem>
#include <fstream>
#include <fkYAML/node.hpp>
#include "fss.h"
#include "imgui.h"
#include <iostream>
#include <type_traits>

#include "widgets.h"
#include "magic_enum/magic_enum.hpp"

namespace grey::common {

    template<typename T>
    bool read(const fkyaml::node& node, const std::string& key, T& value_ref) {
        if(!node.contains(key)) return false;

        const fkyaml::node &value = node[key];

        try {
            value_ref = value.get_value<T>();
            return true;
        } catch(const fkyaml::exception&) {
            // wrong type, or conversion failed
        }
        return false;
    }

    template<typename T>
    static void write(fkyaml::node &n, const std::string &key, const T &value) {
        n[key] = value;
    }

    template<typename T>
    bool read_enum(const fkyaml::node& node, const std::string& key, T& value_ref) {
        if(!node.contains(key)) return false;
        
        static_assert(std::is_enum_v<T>, "T must be an enum");

        const fkyaml::node &value = node[key];

        try {
            auto str_val = value.get_value<std::string>();
            auto enum_val = magic_enum::enum_cast<T>(str_val);
            if (enum_val.has_value()) {
                value_ref = enum_val.value();
                return true;
            }
        } catch(const fkyaml::exception&) {
            // Not a string, or magic_enum failed
        }
        return false;
    }

    template<typename T>
    static void write_enum(fkyaml::node &n, const std::string &key, const T &value) {
        static_assert(std::is_enum_v<T>, "T must be an enum");
        n[key] = std::string(magic_enum::enum_name(value));
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
                    deserialize();
                    last_write_time = new_last_write_time;
                    prev_state = state;
                    changed = true;
                } else {
                    // otherwise check if we need to dump the state
                    if(state != prev_state) {
                        serialize();
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

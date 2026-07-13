#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <fkYAML/node.hpp>
#include "fss.h"

namespace grey::common {

#define fky(node, type, name, default_value) node.contains(name) ? node[name].get_value<type>() : default_value;

    class state_handler {
    public:
        using ynode = fkyaml::node;

        explicit state_handler(const std::string &application_name) {
            file_path = (std::filesystem::path{grey::common::fss::get_config_dir(application_name)} / "config.yml").
                    string();

            // create dir if not exists
            const std::filesystem::path abs{file_path};
            std::filesystem::create_directories(abs.parent_path());
        }

        template<typename T>
        void read(ynode &n, const std::string &key, T &value, const T default_value) {
            const ynode &value_node = n[key];
            value = default_value;

            if constexpr(std::is_same_v<T, std::string>) {
                if(value_node.is_string())
                    value = value_node.get_value<std::string>();
            } else if constexpr(std::is_same_v<T, bool>) {
                if(value_node.is_boolean())
                    value = value_node.get_value<bool>();
            }
        }

        template<typename T>
        void read(const std::string &key, T &value, const T default_value) {
            read(root, key, value, default_value);
        }

        template<typename T>
        void write(ynode &n, const std::string &key, const T &value) {
            n[key] = value;
        }

        template<typename T>
        void write(const std::string &key, const T &value) {
            write(root, key, value);
        }

        ynode root;

        void deserialize() {
            std::ifstream ifs{file_path, std::ios::in};
            root = ifs ? fkyaml::node::deserialize(ifs) : fkyaml::node::mapping();
            if(root.get_type() != fkyaml::node_type::MAPPING)
                root = fkyaml::node::mapping();
        }

        void serialize() const {
            std::ofstream ofs{file_path, std::ios::out};
            ofs << root;
        }

        std::filesystem::file_time_type get_last_write_time() const {
            return std::filesystem::exists(file_path)
                ? std::filesystem::last_write_time(file_path)
                : std::filesystem::file_time_type{};
        }

    private:
        std::string file_path;
    };

    template<typename TState>
    class app_state_container {
    public:
        virtual ~app_state_container() = default;

        virtual void serialize() = 0;
        virtual void deserialize() = 0;
        virtual std::filesystem::file_time_type get_last_write_time() const = 0;
        virtual TState& get_state() = 0;
    };

    template<typename TState>
    class state_ticker {
    public:
        state_ticker(app_state_container<TState>& state_container, float flush_interval = 1.0f)
        : state_container{state_container}, flush_interval{flush_interval} {
            last_write_time = state_container.get_last_write_time();
            prev_state = state_container.get_state();
        }

        bool tick(float delta_time) {
            last_flushed_ago += delta_time;
            bool changed{false};

            if(last_flushed_ago > flush_interval) {
                // check if our version is old
                auto new_last_write_time = state_container.get_last_write_time();
                if(new_last_write_time > last_write_time) {
                    state_container.deserialize();
                    last_write_time = new_last_write_time;
                    prev_state = state_container.get_state();
                    changed = true;
                } else {
                    // otherwise check if we need to dump the state
                    bool are_same = state_container.get_state() == prev_state;
                    if(!are_same) {
                        state_container.serialize();
                        prev_state = state_container.get_state();
                        last_write_time = state_container.get_last_write_time();
                        changed = true;
                    }
                }
                last_flushed_ago = 0.f; // otherwise we'll keep comparing forever
            }

            return changed;
        }
    private:
        const float flush_interval;
        app_state_container<TState>& state_container;
        TState prev_state;   // copy of the state (state needs to support copy constructor
        float last_flushed_ago{0.f};
        std::filesystem::file_time_type last_write_time;
    };
}

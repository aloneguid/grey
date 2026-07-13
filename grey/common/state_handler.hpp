#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <fkYAML/node.hpp>
#include "fss.h"
#include "imgui.h"

namespace grey::common {

    template<typename T>
    static T read(const fkyaml::node &n, const std::string &key, const T& default_value) {
        const fkyaml::node &value_node = n[key];
        T value = default_value;

        if constexpr(std::is_same_v<T, std::string>) {
            if(value_node.is_string())
                value = value_node.get_value<std::string>();
        } else if constexpr(std::is_same_v<T, bool>) {
            if(value_node.is_boolean())
                value = value_node.get_value<bool>();
        } else if constexpr(std::is_same_v<T, float>) {
            if(value_node.is_float_number())
                value = value_node.get_value<float>();
        } else if constexpr(std::is_same_v<T, int>) {
            if(value_node.is_integer())
                value = value_node.get_value<int>();
        } else if constexpr(std::is_same_v<T, unsigned int>) {
            if(value_node.is_integer())
                value = value_node.get_value<unsigned int>();
        }

        return value;
    }

    template<typename T>
    static void write(fkyaml::node &n, const std::string &key, const T &value) {
        n[key] = value;
    }

    static unsigned int shex_to_imcol(const std::string& hex) {
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

    static std::string imcol_to_shex(unsigned int color) {
        ImColor ic{color};
        std::string hex = std::format("#{:02X}{:02X}{:02X}", (int) (ic.Value.x * 255), (int) (ic.Value.y * 255),
                           (int) (ic.Value.z * 255));
        return hex;
    }

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
        void read(const std::string &key, T &value, const T default_value) {
            read(root, key, value, default_value);
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
        app_state_container(TState& state, const std::string &application_name, float flush_interval = 1.0f) : state{state}, h{application_name},
            flush_interval{flush_interval} {
            deserialize();
            prev_state = state;
            last_write_time = h.get_last_write_time();
        }

        ~app_state_container() {
            serialize();
        }

        void serialize() {
            state.serialize(h.root);
            h.serialize();
        }

        void deserialize() {
            h.deserialize();
            state.deserialize(h.root);
        }

        std::filesystem::file_time_type get_last_write_time() const {
            return h.get_last_write_time();
        }

        TState& get_state() {
            return state;
        }

        bool tick(float delta_time) {
            last_flushed_ago += delta_time;
            bool changed{false};

            if(last_flushed_ago > flush_interval) {
                // check if our version is old
                auto new_last_write_time = get_last_write_time();
                if(new_last_write_time > last_write_time) {
                    state.deserialize();
                    last_write_time = new_last_write_time;
                    prev_state = state;
                    changed = true;
                } else {
                    // otherwise check if we need to dump the state
                    bool are_same = state == prev_state;
                    if(!are_same) {
                        state.serialize();
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
        TState& state;
        TState prev_state;
        state_handler h;
        float flush_interval;
        std::filesystem::file_time_type last_write_time;
        float last_flushed_ago{0.f};
    };
}

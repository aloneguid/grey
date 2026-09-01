#pragma once
#include "imgui.h"
#include <string>

namespace grey {
    struct sz;

    enum class emphasis : int32_t {
        none = 0,
        primary = 1,
        secondary = 2,
        success = 3,
        error = 4,
        warning = 5,
        info = 6
    };

    enum class show_delay : int32_t {
        immediate = 0,
        quick,
        normal,
        slow
    };

    enum class font_weight : int32_t {
        regular = 0,
        fixed_size = 1,
        bold = 2
    };

    /**
     * @brief Trivial point struct for storing 2D coordinates
     */
    struct point {
        float x;
        float y;

        point() : x{0}, y{0} {
        }

        point(const float x, const float y) : x{x}, y{y} {
        }

        point(const ImVec2& pos) : x{pos.x}, y{pos.y} {
        }

        point operator+(const sz& dimensions) const;

        point operator+(float offset_both) const;

        operator ImVec2() const { return ImVec2{x, y}; }
    };

    /**
     * @brief Trivial size struct for storing 2D dimensions
     */
    struct sz {
        float width;
        float height;

        sz() : width{0}, height{0} {
        }

        sz(const float width, const float height) : width{width}, height{height} {
        }

        sz(const ImVec2& dim) : width{dim.x}, height{dim.y} {
        }

        operator ImVec2() const { return ImVec2{width, height}; }
    };

    inline point point::operator+(const sz& dimensions) const {
        return point{x + dimensions.width, y + dimensions.height};
    }

    inline point point::operator+(const float offset_both) const {
        return point{x + offset_both, y + offset_both};
    }

    /**
     * @brief Trivial rectangle struct for storing item bounds.
     */
    struct rect {
        float x_min;
        float y_min;
        float x_max;
        float y_max;

        constexpr rect() : x_min{0}, y_min{0}, x_max{0}, y_max{0} {
        }

        constexpr rect(float x_min, float y_min, float x_max, float y_max)
            : x_min{x_min}, y_min{y_min}, x_max{x_max}, y_max{y_max} {
        }

        constexpr rect(const point& min, const point& max)
            : x_min{min.x}, y_min{min.y}, x_max{max.x}, y_max{max.y} {
        }

        constexpr rect(const ImVec2& min, const ImVec2& max)
            : x_min{min.x}, y_min{min.y}, x_max{max.x}, y_max{max.y} {
        }

        /**
         * @brief Left-top corner
         * @return
         */
        [[nodiscard]] point lt() const { return point{x_min, y_min}; }

        /**
         * @brief Right-bottom corner
         * @return
         */
        [[nodiscard]] point rb() const { return point{x_max, y_max}; }

        [[nodiscard]] float width() const { return x_max - x_min; }

        [[nodiscard]] float height() const { return y_max - y_min; }

        [[nodiscard]] point centre() const { return point{(x_min + x_max) / 2, (y_min + y_max) / 2}; }
    };

    class rgb_colour {
    public:
        float r;
        float g;
        float b;
        float o;

        rgb_colour() : r{0}, g{0}, b{0}, o{0} {
        }

        rgb_colour(float r, float g, float b, float o = 1.0f) : r{r}, g{g}, b{b}, o{o} {
        }

        /**
         * Construct from hex representation in the following format: RRGGBB or RRGGBBAA.
         * Can be optionally prefixed with a #.
         */
        rgb_colour(const std::string& hex);

        explicit rgb_colour(const ImColor& ic) {
            r = ic.Value.x;
            g = ic.Value.y;
            b = ic.Value.z;
            o = ic.Value.w;
        }

        explicit rgb_colour(const ImVec4& vec) {
            r = vec.x;
            g = vec.y;
            b = vec.z;
            o = vec.w;
        }

        explicit rgb_colour(ImGuiCol_ col) {
            ImColor ucol = ImGui::GetColorU32(col);
            r = ucol.Value.x;
            g = ucol.Value.y;
            b = ucol.Value.z;
            o = ucol.Value.w;
        }

        operator ImColor() const {
            return {r, g, b, o};
        }

        operator ImU32() const {
            return (ImU32) ImColor(r, g, b, o);
        }

        /**
         * @brief Returns true if color has any opacity at all
        */
        operator bool() const { return o > 0; }

        bool operator==(const rgb_colour& other) const {
            return r == other.r && g == other.g && b == other.b && o == other.o;
        }

        [[nodiscard]] const std::string to_hex(bool prepend_hash = true) const;
    };

    struct font_config {
        bool load_icons{false};
        bool load_fixed{false};
        bool load_bold{false};

        void load_all() {
            load_icons =
                    load_fixed =
                    load_bold = true;
        }
    };
}

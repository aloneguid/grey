#pragma once
#include "imgui.h"
#include <string>
#include <memory>
#include <optional>
#include <format>

namespace grey {
    struct sz;

    struct texture {
        void* data;
        size_t width;
        size_t height;

        texture(void* data) : data{data}, width{0}, height{0} {}

        /**
         * @brief Disposing the texture is platform specific.
         */
        virtual ~texture() = default;
    };

    /**
     * @brief Represents different styles of system window chrome.
     */
    enum class system_chrome {
        /**
         * Make the window look like a native window.
         */
        native = 0,

        /**
         * Looks like a native window, but without header.
         */
        headerless = 1,

        /**
         * No decorations at all, looks like a boring rectangle.
         */
        none = 2
    };

    /**
     * Functions to work with textures (i.e. images)
     */
    class texture_loader {
    public:
        virtual std::shared_ptr<texture> get_texture(const std::string& key) = 0;

        virtual bool preload_texture(const std::string& key, const unsigned char* buffer, unsigned int len) = 0;

        virtual bool preload_texture(const std::string& key, const std::string& path) = 0;
    };

    enum class emphasis : int32_t {
        none = 0,
        primary = 1,
        secondary = 2,
        success = 3,
        error = 4,
        warning = 5,
        info = 6,
        disabled = 7
    };

    enum class sub_emphasis : int32_t {
        normal = 0,
        normal_text = 1,
        hovered = 2,
        active = 3
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

    enum class point_pivot {
        top_left,
        top_center,
        top_right,
        center_left,
        center,
        center_right,
        bottom_left,
        bottom_center,
        bottom_right,
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

        point(point_pivot pivot) {
            switch (pivot) {
                case point_pivot::top_left:      x = 0.0f; y = 0.0f; break;
                case point_pivot::top_center:    x = 0.5f; y = 0.0f; break;
                case point_pivot::top_right:     x = 1.0f; y = 0.0f; break;
                case point_pivot::center_left:   x = 0.0f; y = 0.5f; break;
                case point_pivot::center:        x = 0.5f; y = 0.5f; break;
                case point_pivot::center_right:  x = 1.0f; y = 0.5f; break;
                case point_pivot::bottom_left:   x = 0.0f; y = 1.0f; break;
                case point_pivot::bottom_center: x = 0.5f; y = 1.0f; break;
                case point_pivot::bottom_right:  x = 1.0f; y = 1.0f; break;
                default:                         x = 0.0f; y = 0.0f; break;
            }
        }

        point operator+(const sz& dimensions) const;

        point operator-(const sz& dimensions) const;

        point operator+(float offset_both) const;

        point operator*(const float mult) const { return point{x * mult, y * mult}; }

        point operator/(const float div) const { return point{x / div, y / div}; }

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

        sz operator*(const float mult) const { return sz{width * mult, height * mult}; }

        sz operator/(const float div) const { return sz{width / div, height / div}; }
    };

    inline point point::operator+(const sz& dimensions) const {
        return point{x + dimensions.width, y + dimensions.height};
    }

    inline point point::operator-(const sz& dimensions) const {
        return point{x - dimensions.width, y - dimensions.height};
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

        [[nodiscard]] bool empty() const { return x_min == x_max && y_min == y_max; }
    };

    struct monitor {
        rect area;
        rect work_area;

        /**
         * DPI scale factor. 96 DPI == 1.0f
         */
        float dpi_scale;
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

    struct style {
        emphasis emp{emphasis::none};

        /**
         * When set, emphasis will probably be ignored.
         */
        rgb_colour colour{};

        float text_wrap_pos{.0f};

        bool center_x{false};

        bool center_y{false};

        float font_size{.0f};

        font_weight font_w{font_weight::regular};
    };

    enum class act_condition {
        never,
        once,
        always
    };

    inline ImGuiCond to_imgui_cond(act_condition cond) {
        switch(cond) {
            case act_condition::once: return ImGuiCond_Once;
            case act_condition::always: return ImGuiCond_Always;
            default: return ImGuiCond_None;
        }
    }

    struct wnd_opts {
        bool* open_ptr{nullptr};
        bool fill_viewport{false};
        float opacity{1.0f};
        bool show_title_bar{true};
        bool always_on_top{false};
        point pos{};
        point_pivot pos_pivot{point_pivot::top_left};
        act_condition pos_cond{act_condition::never};
        sz size{};
        act_condition size_cond{act_condition::never};
        float border{.0f};
        bool scrollable{true};

        /**
         * When true, user can manually resize the window.
         */
        bool resizeable{true};

        /**
         * Window will auto-resize automatically based on the rendered content. Does not apply to main application window.
         */
        bool auto_resize{false};

        /**
         * Prevents screen capture of this window (Windows only for now).
         */
        std::optional<bool> screen_capture_allowed{false};
        bool native_decorations{true};
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

namespace std {
    template <> struct std::formatter<grey::point> : std::formatter<std::string> {
        auto format(const grey::point& p, std::format_context& ctx) const {
            return std::formatter<std::string>::format(
                std::format("({:.1f}, {:.1f})", p.x, p.y), ctx);
        }
    };

    template <> struct std::formatter<grey::sz> : std::formatter<std::string> {
        auto format(const grey::sz& s, std::format_context& ctx) const {
            return std::formatter<std::string>::format(
                std::format("({:.1f}, {:.1f})", s.width, s.height), ctx);
        }
    };

    template <> struct std::formatter<grey::rect> : std::formatter<std::string> {
        auto format(const grey::rect& r, std::format_context& ctx) const {
            return std::formatter<std::string>::format(
                std::format("({} - {})", r.lt(), r.rb()), ctx);
        }
    };
}

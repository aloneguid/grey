#pragma once
#include "imgui.h"
#include "themes.h"
#include "fonts/MaterialIcons.h"
#include <string>
#include <vector>
#include <functional>
#include "app.h"

namespace grey::widgets {

    enum class emphasis {
        none = 0,
        primary = 1,
        error = 2
    };

    class rgb_colour {
    public:
        float r;
        float g;
        float b;
        float o;

        rgb_colour() : r{0}, g{0}, b{0}, o{0} {

        }

        rgb_colour(const ImColor& ic) {
            r = ic.Value.x;
            g = ic.Value.y;
            b = ic.Value.z;
            o = ic.Value.w;
        }

        rgb_colour(const ImVec4& vec) {
            r = vec.x;
            g = vec.y;
            b = vec.z;
            o = vec.w;
        }

        operator ImColor() {
            return ImColor(r, g, b, o);
        }

        operator ImU32() {
            return (ImU32)ImColor(r, g, b, o);
        }

        /**
         * @brief Returns true if color has any opacity at all
        */
        operator bool() { return o > 0; }
    };

    class guardable {
    public:
        virtual void enter() = 0;
        virtual void leave() = 0;
    };

    class window : public guardable {
    public:
        window(const std::string& title, bool* p_open = nullptr);

        window& size(int width, int height, float scale);
        window& has_menubar();
        window& fullscreen();
        window& no_resize();
        window& no_focus();
        window& no_scroll();
        window& center(void* monitor_handle = nullptr);

        void enter() override;
        void leave() override;

        ~window();

    private:
        bool rendered_once{false};
        bool capture_size{false};
        ImVec2 init_size;
        bool init_center{false};
        void* init_center_monitor{nullptr};
        bool init_center_done{false};
        ImVec2 size_in;
        const std::string title;
        bool* p_open{nullptr};
        ImGuiWindowFlags flags{0};
        ImGuiWindowClass wc;
    };

    class guard {
    public:
        guard(guardable& g) : g{g} {
            g.enter();
        }

        ~guard() {
            g.leave();
        }

    private:
        guardable& g;
    };

    class container : public guardable {
    public:
        /**
         * @brief Creates container for other controls, which can be scrollable.
         * @param width Width of the container, if zero, it will be taking the remaining space.
         * @param height Height of the container, if zero, it will be taking the remaining space.
         */
        container(float width = 0.0F, float height = 0.0F);
        container(const std::string& id, float width = 0.0F, float height = 0.0F);

        container& border() {
            has_border = true; return *this; }

        void enter() override;
        void leave() override;

    private:
        std::string id;
        ImVec2 size;
        bool has_border{false};
    };

    class group {
    public:
        group();

        group& background(size_t colour_index) {
            this->bg_ci = colour_index; return *this; }

        group& background_hover(size_t colour_index) {
            this->bg_hover_ci = colour_index; return *this; }

        group& border(size_t colour_index) {
            this->bdr_ci = colour_index; return *this; }

        group& border_hover(size_t colour_index) {
            this->bdr_hover_ci = colour_index; return *this; }

        group& spread_horizontally() {
            this->full_width = true; return *this; }

        void render();

        ~group();

    private:
        size_t bdr_ci{0};
        size_t bdr_hover_ci{0};
        size_t bg_ci{0};
        size_t bg_hover_ci{0};
        bool full_width{false};
    };

    class menu_item {
    public:
        std::string id;
        std::string text;
        std::vector<menu_item> children;
        std::string icon;

        menu_item(const std::string& id, const std::string& text, const std::string& icon = "") : id{id}, text{text}, icon{icon} {}

        menu_item(const std::string text, const std::vector<menu_item>& children, const std::string& icon = "") : id{""}, text{text}, children{children}, icon{icon} {}

        // utility
        static std::vector<menu_item> make_ui_theme_items();
    };

    class menu_bar {
    public:
        menu_bar();
        menu_bar(const std::vector<menu_item>& items, std::function<void(const std::string&)> clicked);
        ~menu_bar();

        operator bool() const {
            return rendered;
        }

    private:
        bool rendered{false};
        void render(const std::vector<menu_item>& items, std::function<void(const std::string&)> clicked);
    };

    class status_bar {
    public:
        status_bar();
        ~status_bar();

    private:
        ImGuiStyle& style;
        ImVec2 cursor_before;
    };

    class tab_bar_item {
    public:
        tab_bar_item(const std::string& id);
        ~tab_bar_item();

        operator bool() const {
            return rendered;
        }

    private:
        std::string id;
        bool rendered{false};
    };

    class tab_bar {
    public:
        tab_bar(const std::string& id);
        ~tab_bar();

        tab_bar_item next_tab(const std::string& title);

    private:
        bool rendered{false};
        std::string id;
        size_t tab_index{0};
        ImGuiTabBarFlags flags{0};
    };

    /**
     * @brief Set absolute position. If value is less than zero, no positioning is done for that axis.
     * @param x 
     * @param y 
     */
    void set_pos(float x, float y);

    /**
     * @brief Move relatively. Supports positive and negative values.
     * @param x 
     * @param y 
     */
    void move_pos(float x, float y);

    void label(const std::string& text, size_t text_wrap_pos = 0, bool enabled = true);

    void label(const std::string& text, emphasis emp, size_t text_wrap_pos = 0, bool enabled = true);

    bool input(std::string& value, const std::string& label = "", bool enabled = true, float width = 0, bool is_readonly = false);

    void tooltip(const std::string& text);

    void image(app& app, const std::string& key, size_t width, size_t height);

    void spc(size_t repeat = 1);
    void sl(float offset = 0);
    void sep();

    bool button(const std::string& text, emphasis emp = emphasis::none, bool is_enabled = true, bool is_small = false);

    bool icon_checkbox(const std::string& icon, bool is_checked);

    void icon_list(const std::vector<std::pair<std::string, std::string>>& options, size_t& selected);

    bool accordion(const std::string& header);

    void combo(const std::string& label, const std::vector<std::string>& options, size_t& selected, float width = 0);

    // mouse helpers

    bool is_leftclicked();
}
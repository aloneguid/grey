#pragma once
#include "imgui.h"
#include "themes.h"
#include "fonts/forkawesome.h"
#include <string>
#include <vector>
#include <functional>
#include "app.h"

namespace grey::widgets {
    class window {
    public:
        window(const std::string& title, bool* p_open = nullptr);

        window& size(int width, int height, float scale);
        window& has_menubar();
        window& fullscreen();
        window& no_resize();
        window& no_focus();

        void render();

        ~window();

    private:
        std::string title;
        bool* p_open{nullptr};
        ImGuiWindowFlags flags{0};
        bool rendered{false};
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

    void label(const std::string& text, size_t text_wrap_pos = 0);

    void tooltip(const std::string& text);

    void image(app& app, const std::string& key, size_t width, size_t height);

    void sp(size_t repeat = 1);
    void sl();

}
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

    class guardable {
    public:
        virtual void enter() = 0;
        virtual void leave() = 0;
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
         * @brief Creates container for other controls
         * @param width Width of the container, if zero, it will be taking the remaining space.
         * @param height Height of the container, if zero, it will be taking the remaining space.
         */
        container(float width = 0.0F, float height = 0.0F);

        void enter() override;
        void leave() override;

    private:
        std::string id;
        ImVec2 size;
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

    void label(const std::string& text, size_t text_wrap_pos = 0, bool enabled = true);

    void label(const std::string& text, emphasis emp, size_t text_wrap_pos = 0, bool enabled = true);

    void tooltip(const std::string& text);

    void image(app& app, const std::string& key, size_t width, size_t height);

    void spc(size_t repeat = 1);
    void sl();
    void sep();

    bool button(const std::string& text, emphasis emp = emphasis::none, bool is_enabled = true, bool is_small = false);
}
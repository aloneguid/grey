#pragma once
#include "imgui.h"
#include "themes.h"
#include "fonts/MaterialIcons.h"
#include <string>
#include <vector>
#include <functional>
#include "app.h"

// 3rdparty
//#ifdef GREY_INCLUDE_IMNODES
#include "3rdparty/imgui-node-editor/imgui_node_editor.h"
namespace ed = ax::NodeEditor;
//#endif // GREY_INCLUDE_IMNODES
#include "3rdparty/ImGuiColorTextEdit/TextEditor.h"

namespace grey::widgets {

    extern float scale;

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

        /**
         * @brief Set initial window size. The size will be scaled.
         * @param width 
         * @param height 
         * @return 
         */
        window& size(int width, int height);
        window& resize(float width = 0, float height = 0);
        window& has_menubar();
        window& fullscreen();
        window& no_resize();
        window& no_collapse();
        window& no_titlebar();
        window& border(float width);
        window& no_scroll();
        window& center(void* monitor_handle = nullptr);
        window& fill_viewport();

        void enter() override;
        void leave() override;

        ~window();

    private:
        ImVec2 init_size{0, 0};
        ImVec2 resize_to{0, 0};

        // centering
        bool init_center{false};    // whether to center window
        ImVec2 init_center_pos;     // position to center at (calculated)
        void* init_center_monitor{nullptr}; // monitor to center at (native handle)
        ImGuiPlatformMonitor init_center_imgui_monitor; // monitor to center at (imgui handle)

        const std::string title;
        bool* p_open{nullptr};
        ImGuiWindowFlags flags{0};
        ImGuiWindowClass wc;
        float border_size{-1};
        bool fill_viewport_enabled{false};

        bool win32_brought_forward{false};
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

    const std::string SetThemeMenuPrefix{"set_theme_"};

    /**
     * @brief Menu item with optional icon space reservation and icon.
     * @param text 
     * @param reserve_icon_space 
     * @param icon 
     * @return 
     */
    bool mi(const std::string& text, bool reserve_icon_space = false, const std::string& icon = "");

    void mi_themes(std::function<void(const std::string&)> on_changed);

    class menu {
    public:
        menu(const std::string& title, bool reserve_icon_space = false, const std::string& icon = "");
        ~menu();

        operator bool() const {
            return rendered;
        }

    private:
        bool rendered;
        ImVec2 cp;
        std::string icon;
    };

    class menu_bar {
    public:
        menu_bar();
        ~menu_bar();

        operator bool() const {
            return rendered;
        }

    private:
        bool rendered{false};
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
        tab_bar_item(const std::string& id, bool unsaved);
        ~tab_bar_item();

        operator bool() const {
            return rendered;
        }

    private:
        std::string id;
        ImGuiTabItemFlags flags{0};
        bool rendered{false};
    };

    class tab_bar {
    public:
        tab_bar(const std::string& id);
        ~tab_bar();

        tab_bar_item next_tab(const std::string& title, bool unsaved = false);

    private:
        bool rendered{false};
        std::string id;
        size_t tab_index{0};
        ImGuiTabBarFlags flags{0};
    };

    /**
     * @brief Popup is a child element that can display extra items on top.
     *        But unlike raw implementation, this allow to open popups from anywhere in ID-stack.
     */
    class popup : public guardable {
    public:
        popup(const std::string& id);

        void enter() override;
        void leave() override;

        void open();
        void open(float x, float y);

        operator bool() const {
            return rendered;
        }

    private:
        std::string id;
        bool do_open{false};
        float open_x, open_y;
        bool rendered{false};
    };

    /**
     * @brief Set absolute position. If value is less than zero, no positioning is done for that axis.
     * @param x 
     * @param y 
     */
    void set_pos(float x, float y);

    void get_pos(float& x, float& y);

    /**
     * @brief Move relatively. Supports positive and negative values.
     * @param x 
     * @param y 
     */
    void move_pos(float x, float y);

    void label(const std::string& text, size_t text_wrap_pos = 0, bool enabled = true);

    void label(const std::string& text, emphasis emp, size_t text_wrap_pos = 0, bool enabled = true);

    bool input(std::string& value, const std::string& label = "", bool enabled = true, float width = 0, bool is_readonly = false);

    void input_ml(std::string& value, const std::string& label = "", unsigned int line_height = 10, bool autoscroll = false);

    void tooltip(const std::string& text);

    void image(app& app, const std::string& key, size_t width, size_t height);

    void icon_image(app& app, const std::string& key);

    void rounded_image(app& app, const std::string& key, size_t width, size_t height, float rounding);

    bool icon_selector(app& app, const std::string& path, size_t square_size);

    void spc(size_t repeat = 1);
    void sl(float offset = 0);
    void sep(const std::string& text = "");

    bool button(const std::string& text, emphasis emp = emphasis::none, bool is_enabled = true, bool is_small = false);

    bool icon_checkbox(const std::string& icon, bool& is_checked, bool reversed = false);

    bool checkbox(const std::string& label, bool& is_checked);

    /**
     * @brief 
     * Small (less height) checkbox, similar to small button.
     * @param label 
     * @param is_checked 
     * @return 
     */
    bool small_checkbox(const std::string& label, bool& is_checked);

    void icon_list(const std::vector<std::pair<std::string, std::string>>& options, size_t& selected);

    bool accordion(const std::string& header);

    void combo(const std::string& label, const std::vector<std::string>& options, size_t& selected, float width = 0);

    bool radio(const std::string& label, bool is_active);

    bool small_radio(const std::string& label, bool is_active);

    void notify_info(const std::string& message);
    void notify_render_frame();

    // mouse helpers

    bool is_leftclicked();

    bool is_rightclicked();

    bool is_hovered();

    bool tree_node(const std::string& label, ImGuiTreeNodeFlags flags = 0, emphasis emp = emphasis::none);

    // colour helpers

    ImU32 imcol32(ImGuiCol idx);

//#ifdef GREY_INCLUDE_IMNODES

    // Node editor

    class node_editor_node {
    public:
        ImVec2 size;

        node_editor_node(int id);
        ~node_editor_node();

    private:
        int id;
    };

    /**
     * @brief Node editor based on https://github.com/thedmd/imgui-node-editor
     */
    class node_editor : public guardable {
    public:
        node_editor(bool select_on_hover = false);
        ~node_editor();

        void enter() override;
        void leave() override;

        node_editor_node node(int id) {
            return node_editor_node{id};
        }

        void set_node_pos(int node_id, float x, float y);
        void pin_in(int pin_id, const std::string& text);
        void pin_out(int pin_id, const std::string& text);
        void link(int link_id, int from_pin_id, int to_pin_id, bool flow = false);

        void get_node_size(int node_id, float& width, float& height);

        /**
         * @brief If a node is selected, returns node ID, otherwise returns -1.
         * @return 
         */
        int get_selected_node_id();

        /**
         * @brief If a node is hovered, returns node ID, otherwise returns -1.
         * @return 
         */
        int get_hovered_node_id();

    private:
        std::string id;
        bool select_on_hover;
        ed::Config config;
        ed::EditorContext* context{nullptr};
    };

//#endif

    // ImGuiColorTextEdit
    class text_editor {
    public:
        text_editor();

        void set_text(const std::string& text);
        std::string get_text();

        /**
         * @brief Renders text editor and returns true if text has changed.
         * @return 
         */
        bool render();
    private:
        std::string id;
        TextEditor editor;
        const TextEditor::LanguageDefinition& lang;
    };

}
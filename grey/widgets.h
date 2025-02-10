#pragma once
#include "imgui.h"
#include "themes.h"
#include "fonts/MaterialIcons.h"
#include <string>
#include <vector>
#include <functional>
#include "app.h"

// 3rdparty
#include "3rdparty/ImGuiColorTextEdit/TextEditor.h"

namespace grey::widgets {

    extern float scale;

    enum class emphasis : int32_t {
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
            flags |= ImGuiChildFlags_Border;
            return *this;
        }

        container& auto_size_y() {
            flags |= ImGuiChildFlags_AutoResizeY;
            return *this;
        }

        container& resize_y() {
            flags |= ImGuiChildFlags_ResizeY;
            return *this;
        }

        container& resize_x() {
            flags |= ImGuiChildFlags_ResizeX;
            return *this;
        }

        void enter() override;
        void leave() override;

    private:
        std::string id;
        ImVec2 size;
        ImGuiChildFlags flags{0};
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
        tab_bar(const std::string& id, bool tab_list_popup = false, bool scroll = false);
        ~tab_bar();

        tab_bar_item next_tab(const std::string& title, bool unsaved = false);

        /**
         * @brief Increments tab index
         * @return Index before increment 
         */
        size_t increment_tab_index() { return tab_index++; }

    private:
        bool rendered{false};
        std::string id;
        size_t tab_index{0};
        ImGuiTabBarFlags flags{ImGuiTabBarFlags_DrawSelectedOverline};
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

    bool input(int& value, const std::string& label = "", bool enabled = true, float width = 0, bool is_readonly = false);

    bool input(char* value, int value_length, const std::string& label = "", bool enabled = true, float width = 0, bool is_readonly = false);

    bool input_ml(const std::string& id, std::string& value, unsigned int line_height = 10, bool autoscroll = false, bool enabled = true);

    /**
     * @brief Multiline edit
     * @param id 
     * @param value 
     * @param height Height in pixels, if zero, it will be taking the remaining space. If negative, it will be taking the remaining space minus the value.
     * @param autoscroll Whether to scroll to the bottom when new text is added.
     * @param enabled Controls whether the input is enabled.
     * @param use_fixed_font Use fixed font for the text editor if possible.
     * @return 
     */
    bool input_ml(const std::string& id, std::string& value, float height = 0, bool autoscroll = false, bool enabled = true, bool use_fixed_font = false);

    bool input_ml(const std::string& id, char* value, int value_length, float height = 0, bool autoscroll = false, bool enabled = true, bool use_fixed_font = false);


    bool slider(float& value, float min, float max, const std::string& label = "");

    bool slider(int& value, int min, int max, const std::string& label = "");

    void tooltip(const std::string& text);

    void tooltip(const char* text);

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

    bool hyperlink(const std::string& text, const std::string& url_to_open = "");

    /**
     * @brief 
     * Small (less height) checkbox, similar to small button.
     * @param label 
     * @param is_checked 
     * @return 
     */
    bool small_checkbox(const std::string& label, bool& is_checked);

    bool icon_list(const std::vector<std::pair<std::string, std::string>>& options, size_t& selected);

    bool accordion(const std::string& header, bool default_open = false);

    /**
     * @brief 
     * @param label 
     * @param options 
     * @param selected 
     * @param width Unscaled width
     * @return 
     */
    bool combo(const std::string& label, const std::vector<std::string>& options, size_t& selected, float width = 0);

    bool list(const std::string& label, const std::vector<std::string>& options, size_t& selected, float width = 0);

    //bool list(const std::string& label, std::ranges::range auto&& options, size_t& selected, size_t& hovered, float width = 0);

    bool radio(const std::string& label, bool is_active);

    bool small_radio(const std::string& label, bool is_active);

    /**
     * @brief Progress spinner in "HBO" style.
     * @param radius 
     * @param thickness 
     * @param speed 
     * @param dot_count 
     */
    void spinner_hbo_dots(float radius = 16, float thickness = 4, float speed = 1.0f, size_t dot_count = 6);

    void notify_info(const std::string& message);
    void notify_render_frame();

    // mouse helpers

    bool is_leftclicked();

    bool is_rightclicked();

    bool is_hovered();

    bool tree_node(const std::string& label, ImGuiTreeNodeFlags flags = 0, emphasis emp = emphasis::none);

    // colour helpers

    ImU32 imcol32(ImGuiCol idx);

    // system debug info
    void label_debug_info();

    // Node editor

    class node_editor_node {
    public:
        ImVec2 size;

        node_editor_node(int id);
        ~node_editor_node();

    private:
        int id;
    };

    // ImGuiColorTextEdit
    class text_editor {
    public:
        text_editor(bool border = false);

        void set_text(const std::string& text);
        std::string get_text();

        /**
         * @brief Renders text editor and returns true if text has changed.
         * @return 
         */
        bool render(float width = 0.0f, float height = 0.0f);
    private:
        std::string id;
        bool border;
        TextEditor editor;
        const TextEditor::LanguageDefinition& lang;
    };

    // tables

    class table {
    public:
        table(const std::string& id, int column_count, float outer_width = 0, float outer_height = 0);
        ~table();

        std::vector<std::string> columns;

        operator bool() const {
            return rendered;
        }

        void begin_data();
        void begin_row();
        void begin_col();

    private:
        bool rendered{false};
        bool header_rendered{false};
        ImGuiTableFlags flags{
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_NoBordersInBodyUntilResize |
            ImGuiTableFlags_HighlightHoveredColumn |
            ImGuiTableFlags_Hideable |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_ScrollX };

        void render_headers();
    };

    /**
     * @brief Table that will allow displaying enormous amounts of data.
              Experimental, needs refining the clipping functionality
     */
    class big_table {
    public:
        big_table(const std::string& id, int column_count, int row_count, ImVec2 outer_size);
        ~big_table();

        operator bool() const {
            return rendered;
        }

        void col(const std::string& label, bool stretch = false);
        void headers_row();

        bool step(int& display_start, int& display_end);
        void next_row();
        void to_col(int i);

    private:
        bool rendered{false};
        ImGuiTableFlags flags {
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_NoBordersInBodyUntilResize |
            ImGuiTableFlags_HighlightHoveredColumn |
            ImGuiTableFlags_Hideable |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_ScrollX };
        ImGuiListClipper clipper;
    };
}
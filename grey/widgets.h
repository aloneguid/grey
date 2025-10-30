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

#if GREY_INCLUDE_IMPLOT
#include "implot.h"
#endif

namespace grey::widgets {

    extern float scale;

    enum class emphasis : int32_t {
        none = 0,
        primary = 1,
        secondary = 2,
        success = 3,
        error = 4,
        warning = 5,
        info = 6
    };

    class rgb_colour {
    public:
        float r;
        float g;
        float b;
        float o;

        rgb_colour() : r{0}, g{0}, b{0}, o{0} { }

        rgb_colour(float r, float g, float b, float o = 1) : r{r}, g{g}, b{b}, o{o} { }

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
        window& no_background();
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

#define with_window(w, ...) { { grey::widgets::guard wg{w}; __VA_ARGS__ }}

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

        container& background(bool present) {
            if(present) {
                window_flags &= ~ImGuiWindowFlags_NoBackground;
            } else {
                window_flags |= ImGuiWindowFlags_NoBackground;
            }
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

        container& horizontal_scrollbar() {
            window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
            return *this;
        }

        container& resize(float x, float y) {
            size = ImVec2(x, y);
            return *this;
        }

        container& padding(float x, float y) {
            window_flags |= ImGuiWindowFlags_AlwaysUseWindowPadding;
            pad = ImVec2{x, y};
            return *this;
        }

        void enter() override;
        void leave() override;

    private:
        std::string id;
        ImVec2 size;
        ImVec2 pad{0, 0};
        ImGuiChildFlags flags{0};
        ImGuiWindowFlags window_flags{0};
    };

#define with_container(c, ...) { { grey::widgets::guard cg{c}; __VA_ARGS__ }}

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

#define with_menu_bar(...) { { grey::widgets::menu_bar mb; if(mb) { __VA_ARGS__ } } }
#define with_menu_item(title, ...) { { grey::widgets::menu mi{title}; if(mi) { __VA_ARGS__ } }}

    class status_bar {
    public:
        status_bar();
        ~status_bar();

    private:
        ImGuiStyle& style;
        ImVec2 cursor_before;
    };

#define with_status_bar(...) { grey::widgets::status_bar sb; __VA_ARGS__ }

    class tab_bar_item {
    public:
        tab_bar_item(const std::string& id, bool unsaved, bool selected);
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

        tab_bar_item next_tab(const std::string& title, bool unsaved = false, bool selected = false);

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

#define with_tab(tb, title, ...) { auto tab = tb.next_tab(title); if(tab) { __VA_ARGS__  } }

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
     * @brief Get cursor position
     * @param x 
     * @param y 
     */
    void cur_get(float& x, float& y);
    ImVec2 cur_get();
    void cur_set(float x, float y);
    void cur_set(const ImVec2& pos);

    float avail_x();
    float avail_y();

    void label(const std::string& text, size_t text_wrap_pos = 0, bool enabled = true);

    void label(const std::string& text, rgb_colour colour);

    void label(const std::string& text, emphasis emp, size_t text_wrap_pos = 0, bool enabled = true);

    /**
     * @brief Selectable item
     * @param text Text to display
     * @param span_columns Whether to span all columns in a table (only applicable inside tables)
     * @return True if selection changes
     */
    bool selectable(const std::string& text, bool span_columns = false);

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

    /**
     * @brief Renders markdown text using https://github.com/enkisoftware/imgui_markdown. This is not ready for public consumption yet.
     * @param text 
     */
    void markdown(const std::string& text);

    bool slider(float& value, float min, float max, const std::string& label = "");

    bool slider(int& value, int min, int max, const std::string& label = "");

    /**
     * @brief Checks if the last rendered item is hovered, and if so, shows a tooltip with the given text.
     * @param text 
     */
    void tooltip(const std::string& text);

    /**
     * @brief Checks if the last rendered item is hovered, and if so, shows a tooltip with the given text.
     * @param text 
     */
    void tooltip(const char* text);

    void image(app& app, const std::string& key, size_t width, size_t height);

    void icon_image(app& app, const std::string& key);

    void rounded_image(app& app, const std::string& key, size_t width, size_t height, float rounding);

    bool icon_selector(app& app, const std::string& path, size_t square_size);

    void spc(size_t repeat = 1);
    void sl(float offset = 0);
    void sep(const std::string& text = "");

    bool button(const std::string& text, emphasis emp = emphasis::none, bool is_enabled = true, bool is_small = false, const std::string& tooltip_text = "");

    bool icon_checkbox(const std::string& icon, bool& is_checked, bool reversed = false, const std::string& tooltip = "");

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
     * @brief Combo box selection widget
     * @param label Label to display
     * @param options List of options
     * @param selected Selected index 
     * @param width Unscaled width
     * @return True if selection has changed.
     */
    bool combo(const std::string& label, const std::vector<std::string>& options, unsigned int& selected, float width = 0);

    bool list(const std::string& label, const std::vector<std::string>& options, unsigned int& selected, float width = 0);

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

    enum class mouse_cursor_type {
        none = -1,
        arrow = 0,
        text_input,
        resize_all,
        resize_ns,
        resize_ew,
        resize_nesw,
        resize_nwse,
        hand,
        not_allowed
    };

    void mouse_cursor(mouse_cursor_type mct);

    class tree_node {
    public:
        tree_node(const std::string& label, bool open_by_default = false, bool is_leaf = false);
        ~tree_node();

        operator bool() const {
            return opened;
        }
    private:
        const std::string label;
        bool opened{false};
    };

#define with_window(w, ...) { { grey::widgets::guard wg{w}; __VA_ARGS__ }}

    //bool tree_node(const std::string& label, ImGuiTreeNodeFlags flags = 0, emphasis emp = emphasis::none);

    // colour helpers

    ImU32 imcol32(ImGuiCol idx);

    // system debug info
    void label_debug_info();

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

    /**
     * @brief Table that allows displaying enormous amounts of data.
     */
    class big_table {
    public:
        big_table(const std::string& id, const std::vector<std::string>& columns, size_t row_count,
            float outer_width = 0.0f, float outer_height = 0.0f,
            bool alternate_row_bg = false);
        ~big_table();

        operator bool() const {
            return rendered;
        }

        /**
         * @brief Call to initialize table data rendering. Accepts lambda callback to be invoked for each cell.
         * @param cell_render Callback that will be called for each cell in the table. Row and column indices are passed as parameters.
         */
        void render_data(std::function<void(int, int)> cell_render);

    private:
        size_t columns_size;
        bool rendered{false};
        ImVec2 outer_size;
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

    class small_table {
    public:
        
    };

    /*
    class tree_table {
        public:
        tree_table(const std::string& id, const std::vector<std::string>& columns,
            float outer_width = 0.0f, float outer_height = 0.0f);
        ~tree_table();

        void render(
            std::function<bool(std::string&, int, int&, bool&)> row_render,
            std::function<void(int, int)> column_render);

        operator bool() const {
            return rendered;
        }

    private:
        size_t columns_size;
        bool rendered{false};
        ImVec2 outer_size;
        ImGuiTableFlags flags{
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_NoBordersInBodyUntilResize |
            ImGuiTableFlags_HighlightHoveredColumn |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_ScrollX };
    };
    */

    // plotting

#if GREY_INCLUDE_IMPLOT
    void plot_demo();
#endif
}
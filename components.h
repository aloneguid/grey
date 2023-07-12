#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include "imgui.h"
#include "grey_context.h"
#include "3rdparty/memory_editor.h"

namespace grey {

    const size_t MAX_TABLE_COLUMNS = 64;

    const ImColor emphasis_primary_colour = ImColor::HSV(2 / 7.0f, 0.6f, 0.6f);
    const ImColor emphasis_primary_colour_hovered = ImColor::HSV(2 / 7.0f, 0.6f, 0.7f);
    const ImColor emphasis_primary_colour_active = ImColor::HSV(2 / 7.0f, 0.6f, 0.8f);

    const ImColor emphasis_error_colour = ImColor::HSV(0.0f, 0.6f, 0.6f);
    const ImColor emphasis_error_colour_hovered = ImColor::HSV(2 / 7.0f, 0.6f, 0.7f);
    const ImColor emphasis_error_colour_active = ImColor::HSV(2 / 7.0f, 0.6f, 0.8f);

    const ImColor emphasis_warning_colour = ImColor::HSV(7 / 7.0f, 0.6f, 0.6f);
    const ImColor emphasis_warning_colour_hovered = ImColor::HSV(7 / 7.0f, 0.6f, 0.7f);
    const ImColor emphasis_warning_colour_active = ImColor::HSV(7 / 7.0f, 0.6f, 0.8f);

    static int generate_int_id();

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

    enum class emphasis {
        none,
        primary,
        error,
        warning
    };

    enum class cursor {
        normal,

        hand
    };

    class component {
    public:
        std::string id;
        bool is_enabled{ true };
        bool is_visible{ true };
        std::string tooltip;
        component* parent{ nullptr };
        float width{0};
        float height{0};
        float alpha{1};
        bool bg_draw{false};    // when set to true, does not change cursor coordinates (draws as background)

        // paddings
        float padding_left{0};
        float padding_top{0};

        void* tag{ nullptr };
        float tag_float{0};

        component(const std::string& id = "");
        virtual ~component() {}

        virtual const void render();
        virtual const void render_visible() = 0;
        virtual void post_render() {}
        void set_emphasis(emphasis em);
        emphasis get_emphasis() { return em; }

        std::function<void(component&, bool)> on_hovered;
        std::function<void(component&)> on_click;
        std::function<void(component&)> on_frame;
        std::function<void(component&)> on_visibility_changed;

    protected:
        //emphasis
        ImColor em_normal;
        ImColor em_hovered;
        ImColor em_active;
        emphasis em{ emphasis::none };

        bool is_window{false};

        inline std::string sys_label(const std::string& label) { return label + "##" + id; }

    private:
        bool is_visible_prev_frame{ true };
        ImVec2 start_cursor_pos;

        //click state
        bool state_click_pressed{ false };
        bool on_click_sent{ false };

        const void cursor_move(float x, float y);
    };

    class menu_bar;
    class label;
    class listbox;
    class tree;
    class input;
    class input_int;
    class checkbox;
    class button;
    class toggle;
    class image;
    class big_table;
    template<class TRowState> class complex_table;
    class tabs;
    class slider;
    class progress_bar;
    class imgui_raw;
    class modal_popup;
    class plot;
    class metrics_plot;
    class status_bar;
    class accordion;
    class child;
    class group;
    class selectable;
    template<class TDataElement> class repeater_bind_context;
    template<class TDataElement> class repeater;
#if _DEBUG
    class demo;
#endif

    class container : public component {
    public:
        container(grey_context& mgr) : tmgr{mgr}, scale{mgr.get_system_scale()} {}

        virtual const void render() override;
        virtual const void render_visible() = 0;
        void post_render() override;

        bool empty() { return managed_children.empty() && owned_children.empty(); }
        void clear();
        void assign_child(std::shared_ptr<component> child);
        void assign_managed_child(std::shared_ptr<component> child);
        std::shared_ptr<component> get_child(int index);

        // general helpers
        std::shared_ptr<menu_bar> make_menu_bar();
        std::shared_ptr<label> make_label(const std::string& text, bool is_bullet = false);
        std::shared_ptr<label> make_label(std::string* text, bool is_bullet = false);
        std::shared_ptr<listbox> make_listbox(const std::string& label);
        std::shared_ptr<tree> make_tree();
        std::shared_ptr<input> make_input(const std::string& label, std::string* value = nullptr);
        std::shared_ptr<input_int> make_input_int(const std::string& label, int* value = nullptr);
        std::shared_ptr<checkbox> make_checkbox(const std::string& label, bool* value = nullptr);
        std::shared_ptr<button> make_button(const std::string& label, bool is_small = false, emphasis e = emphasis::none);
        std::shared_ptr<toggle> make_toggle(bool* value);
        std::shared_ptr<image> make_image_from_file(const std::string& path,
            size_t desired_width = std::string::npos,
            size_t desired_height = std::string::npos);
        std::shared_ptr<image> make_image_from_memory(const std::string& tag,
            unsigned char* buffer, size_t buffer_length,
            size_t desired_width = std::string::npos,
            size_t desired_height = std::string::npos);
        std::shared_ptr<big_table> make_big_table(std::vector<std::string> columns, size_t row_count);
        template<class TRowState>
        std::shared_ptr<complex_table<TRowState>> make_complex_table(std::vector<std::string> columns) {
            auto r = std::make_shared<complex_table<TRowState>>(tmgr, columns);
            assign_child(r);
            return r;
        }
        std::shared_ptr<tabs> make_tabs(bool tab_list_popup = false);
        std::shared_ptr<slider> make_slider(const std::string& label, float* value, float min = 0, float max = 1);
        std::shared_ptr<progress_bar> make_progress_bar(float* value, const char* overlay_text = nullptr, float height = 10);
        std::shared_ptr<modal_popup> make_modal_popup(const std::string& title);
        std::shared_ptr<plot> make_plot(const std::string& title, size_t max_values);
        std::shared_ptr<metrics_plot> make_metrics_plot(size_t max_points);
        std::shared_ptr<status_bar> make_status_bar();
        std::shared_ptr<accordion> make_accordion(const std::string& label);
        // child windows have their own scrolling/clipping area.
        std::shared_ptr<child> make_child_window(size_t width = 0, size_t height = 0, bool horizonal_scroll = false);
        std::shared_ptr<group> make_group();
        std::shared_ptr<selectable> make_selectable(const std::string& value);
        template<class TDataElement>
        std::shared_ptr<repeater<TDataElement>> make_repeater(
            std::function<void(repeater_bind_context<TDataElement>)> element_factory,
            bool track_selection = false) {
            auto r = std::make_shared<repeater<TDataElement>>(tmgr, element_factory, track_selection);
            assign_child(r);
            return r;
        }

#if _DEBUG
        std::shared_ptr<demo> make_demo();
#endif

        // edge case helpers
        void same_line(float offset_left = 0);
        void spacer();
        void separator();
        std::shared_ptr<imgui_raw> make_raw_imgui();
        void set_pos(float x, float y, bool is_movement = false);


    protected:
        grey_context& tmgr;
        // managed children are not owned, but you are responsible for rendering them
        std::vector<std::shared_ptr<component>> managed_children;
        float scale;

        const void render_children();


    private:
        bool is_dirty{false};
        std::vector<std::shared_ptr<component>> owned_children;
        std::vector<std::shared_ptr<component>> owned_children_new;
    };

    class common_component : public component {
    public:
        common_component(int c, float arg1 = 0) : c{c}, arg1{arg1} {}
        virtual const void render_visible() override;

    private:
        int c;
        float arg1;
    };

    class button : public component {
    public:
        button(const std::string& label, bool is_small = false, emphasis e = emphasis::none);

        virtual const void render_visible() override;

        std::function<void(button&)> on_pressed;

        std::string get_label() { return label; }
        void set_label(const std::string& label);

    private:
        std::string label;
        bool is_small;
    };

    class toggle : public component {
    public:
        toggle(bool* value) : value{ value } {}

        std::string label_on;
        std::string label_off;

        virtual const void render_visible() override;

        std::function<void(bool)> on_toggled;

    private:
        bool* value;
    };

    /// <summary>
    /// The most basic (but advanced) label control
    /// </summary>
    class label : public component {
    public:
        size_t text_wrap_pos{ 0 };

        label(const std::string& value, bool is_bullet) : value{value}, value_ptr{nullptr}, is_bullet{is_bullet} {}
        label(const std::string* value, bool is_bullet) : value_ptr{value}, is_bullet{is_bullet} {}

        virtual const void render_visible() override;

        std::string get_value();
        void set_value(const std::string& value);

    private:
        std::string value;
        const std::string* value_ptr;
        bool is_bullet;
    };

    class selectable : public component {
    public:
        selectable(const std::string& value);

        virtual const void render_visible() override;

        std::function<void(selectable&)> on_selected;

        void set_size(float x, float y) { size = ImVec2(x, y); }

        /// <summary>
        /// Alighment is proportional and is a value between 0 and 1 for both x and y.
        /// 0 is left/top, 1 is right/bottom.
        /// 0.5 is in the middle.
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        void set_alignment(float x, float y) { alignment = ImVec2(x, y); }

    private:
        std::string value;
        bool is_selected{ false };
        ImGuiSelectableFlags flags{};
        ImVec2 size{};
        ImVec2 alignment{};
    };

    class menu_item {
    public:
        std::string id;
        std::string label;
        std::string icon;
        bool is_selected{ false };
        bool is_enabled{ true };
        std::string shortcut_text;
        void* tag{ nullptr };
        std::vector<std::shared_ptr<menu_item>> children;

        std::shared_ptr<menu_item> add(const std::string& id, const std::string& label, const std::string& icon = "");
    };

    class menu_bar : public component {
    public:
        virtual const void render_visible() override;

        bool is_main_menu{ false };

        std::function<void(menu_item&)> clicked;

        std::shared_ptr<menu_item> items() { return root; }

    private:
        std::shared_ptr<menu_item> root{std::make_shared<menu_item>()};

        void render(std::shared_ptr<menu_item> mi, bool icon_pad);
    };

    class list_item {
    public:
        std::string text;
        std::string tooltip;
        void* tag{ nullptr };

        list_item(const std::string& text, const std::string& tooltip) : text{text}, tooltip{tooltip} {};
    };

    enum class listbox_mode {
        list = 0,
        combo,
        icons
    };

    class listbox : public component {
    public:
        listbox(const std::string& title);

        std::size_t selected_index = -1;
        bool is_full_width = false;
        listbox_mode mode = listbox_mode::list;
        std::vector<list_item> items;
        int items_tall{ 5 };

        /// <summary>
        /// Calls back with index of selected item and the item itself
        /// </summary>
        std::function<void(std::size_t, list_item&)> on_selected;

        virtual const void render_visible() override;

    private:
        std::string title;

    };

    class slider : public component {
    public:
        std::string label;

        std::function<void(slider&)> on_value_changed;

        slider(const std::string& label, float* value, float min = 0.0f, float max = 1.0f);
        ~slider();

        virtual const void render_visible() override;

        void set_limits(float min, float max);

    private:
        float min;
        float max;
        float* value;
        bool value_changed{ false };
        const char* format{ "%.2f" };
        ImGuiSliderFlags flags{ ImGuiSliderFlags_NoInput };
    };

    class progress_bar : public component {
    public:
        progress_bar(float* value, const char* overlay_text = nullptr, float height = 10);

        virtual const void render_visible() override;

    private:
        float* value;
        const char* overlay_text;
        ImVec2 size;
    };

    class tree_node : public container {
    public:
        std::string label;
        bool is_bold{ false };

        tree_node(grey_context& mgr, const std::string& label, bool is_expanded, bool is_leaf)
            : container{ mgr }, label{ label }, is_expanded{ is_expanded }, is_leaf{ is_leaf } {}

        std::vector<std::shared_ptr<tree_node>> nodes;

        virtual const void render_visible() override;

        std::shared_ptr<tree_node> add_node(const std::string& label, bool is_expanded, bool is_leaf);

    private:
        bool is_expanded;
        bool is_leaf;

        bool x_tree_node();
    };

    class tree : public component {
    public:
        tree(grey_context& mgr) : tmgr{ mgr } {}

        std::vector<std::shared_ptr<tree_node>> nodes;

        virtual const void render_visible() override;

        std::shared_ptr<tree_node> add_node(const std::string& label, bool is_expanded, bool is_leaf);
        void remove_node(size_t index);

        void post_render() override;


    private:
        grey_context& tmgr;
    };

    class input : public component {
    public:
        size_t lines = 1;
        bool fill = false;
        size_t lines_bottom_padding{ 0 };
        // temp helper - if true, sets to alternative visual style
        bool is_in_error_state{ false };

        input(const std::string& label, std::string* value = nullptr);
        ~input();

        std::function<void(std::string&)> on_value_changed;

        virtual const void render_visible() override;

        void fire_changed();

        std::string& get_value() { return *value; }
        void set_value(const std::string& value) { *this->value = value; fire_changed(); }

        void set_is_readonly() { flags |= ImGuiInputTextFlags_ReadOnly; }
        void set_select_all_on_focus() { flags |= ImGuiInputTextFlags_AutoSelectAll; }

    private:
        std::string label;
        bool owns_mem;
        std::string* value;
        ImGuiInputTextFlags flags{ImGuiInputTextFlags_AllowTabInput};
    };

    class input_int : public component {
    public:
        input_int(const std::string& label, int* value = nullptr);
        ~input_int();

        virtual const void render_visible() override;

        std::function<void(int&)> on_value_changed;

        int get_value() { return *value; }
        void set_value(int v) { *value = v; }

        /**
         * @brief Sets how much to increment/decrement with step buttons. Setting it to 0 hides step buttons completely.
        */
        void set_step_button_step(int step = 1) { this->step = step; }

    private:
        bool owns_mem;
        std::string label;
        int* value;
        int step{1};
    };

    class checkbox : public component {
    public:
        std::string text;
        bool is_checked() { return *value; };
        void set_checked(bool checked) { *value = checked; }
        bool is_highlighted{ false };
        bool render_as_icon{false};

        checkbox(const std::string& text, bool* value = nullptr);
        ~checkbox();

        std::function<void(bool)> on_value_changed;

        virtual const void render_visible() override;

    private:
        bool owns_mem;
        bool* value;
        bool mouse_was_down{false};
    };

    class image : public component {
    public:
        image(const grey_context& mgr,
            const std::string& file_path,
            unsigned char* buffer = nullptr, unsigned int len = 0,
            size_t desired_width = std::string::npos,
            size_t desired_height = std::string::npos);

        size_t desired_width;
        size_t desired_height;
        size_t actual_width;
        size_t actual_height;
        float rounding{0};

        virtual const void render_visible() override;

    private:
        const grey_context& mgr;
        std::string file_path_or_tag;
        unsigned char* buffer;
        unsigned int len;
        void* texture;

        void ensure_texture();
    };

    class table : public component {
    public:
        virtual const void render_visible() override;

        void set_data(std::vector<std::vector<std::string>> data, bool has_header);

    private:
        std::vector<std::vector<std::string>> data;
        bool has_header{ false };


    };

    class big_table : public component {
    public:
        big_table(std::vector<std::string> columns, size_t row_count)
            :columns{ columns }, row_count{ row_count } {};

        std::vector<std::string> columns;
        size_t row_count;

        virtual const void render_visible() override;

        std::function<void(big_table&, size_t, std::vector<std::string>&)> get_row_data;
    };

    class table_cell : public container {
    public:
        table_cell(grey_context& tmgr) : container{ tmgr } {

        }

        virtual const void render_visible() override;
    };

    template<class TState>
    class table_row {
    public:
        table_row(std::shared_ptr<TState> state) : state{ state } {}

        size_t size() { return cells.size(); }

        std::vector<std::shared_ptr<table_cell>> cells;
        std::shared_ptr<TState> state;
    };

    template<class TRowState>
    class complex_table : public component {
    public:
        complex_table(grey_context& mgr, std::vector<std::string> columns) : mgr{ mgr }, columns{ columns } {}

        bool stretchy{false};

        table_row<TRowState> make_row(std::shared_ptr<TRowState> state) {

            table_row<TRowState> row(state);

            // create cell containers
            for (auto& col : columns) {
                row.cells.push_back(std::make_shared<table_cell>(mgr));
            }

            rows.push_back(row);
            return row;

        }

        virtual const void render_visible() {
            if (columns.empty()) return;

            ImGuiTableFlags flags = ImGuiTableFlags_RowBg |
                ImGuiTableFlags_BordersV |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;

            if(stretchy) flags |= ImGuiTableFlags_SizingStretchSame;

            if (ImGui::BeginTable(id.c_str(), columns.size(), flags)) {
                // render header

                ImGui::TableSetupScrollFreeze(0, 1);   // freeze top row

                for (std::string& col : columns) {
                    ImGui::TableSetupColumn(col.c_str());
                }
                ImGui::TableHeadersRow();

                // render data

                for (auto& row : rows) {
                    ImGui::TableNextRow();
                    for (int i = 0; i < row.size(); i++) {
                        ImGui::TableSetColumnIndex(i);
                        row.cells[i]->render_visible();
                    }
                }

                ImGui::EndTable();
            }
        }

        void post_render() override {
            for(auto& row : rows) {
                for(auto& cell : row.cells) {
                    cell->post_render();
                }
            }
        }

        void clear() { rows.clear(); }

        std::vector<table_row<TRowState>>& get_rows() { return rows; }

        void erase(int index) { if (index < rows.size()) rows.erase(rows.begin() + index); }

    private:
        grey_context& mgr;
        std::vector<std::string> columns;
        std::vector<table_row<TRowState>> rows;
    };

    class tab : public container {
    public:
        tab(grey_context& tmgr) : container{ tmgr } {}

        virtual const void render_visible() override { render_children(); };
    };

    class tabs : public component {
    public:
        tabs(grey_context& mgr, bool tab_list_popup = false);

        size_t get_selected_idx() { return rendered_selected_idx; }

        void clear();
        std::shared_ptr<tab> make(const std::string& title);
        virtual const void render_visible() override;
        void post_render() override;

        std::function<void(size_t)> on_tab_changed;

    private:
        bool tabs_dirty{false};
        int rendered_selected_idx{-1};
        grey_context& mgr;
        ImGuiTabBarFlags flags;
        std::vector<std::string> tab_headers;
        std::vector<std::string> tab_headers_new;
        std::vector<std::shared_ptr<tab>> tab_containers;
        std::vector<std::shared_ptr<tab>> tab_containers_new;
    };

    /// <summary>
    /// Memory Editor by ocornut himself, found at https://github.com/ocornut/imgui_club
    /// </summary>
    class hex_editor : public component {
    public:
        hex_editor(const std::string& title, std::string& data) : title{ title }, data{ data } {}

        virtual const void render_visible() override;

    private:
        MemoryEditor me;
        const std::string title;
        const std::string& data;
    };

    class plot : public component {
    public:

        float sticky_min = -1;
        std::string plot_overlay_text;

        plot(const std::string& label, size_t max_values = 10)
            : label{label}, max_values{max_values} {
            values.resize(max_values);
        }

        virtual const void render_visible() override;

        void add(float value);

        void set_label(const std::string& label);

    private:
        std::string label;
        size_t max_values;
        std::vector<float> values;
        float min{0};
        float max{0};
    };

    class rolling_buffer {
    public:
        double y_max_ever{0};

        rolling_buffer(size_t size) : max_size{size} {
            //data.reserve(size);
            //xs.resize(size);
            //ys.resize(size);
        }

        double* x_begin() { return &xs[0]; }

        double* y_begin() { return &ys[0]; }

        size_t size() { return xs.size(); }

        void add_point(double x, double y) {
            if(xs.size() == max_size) {
                xs.erase(xs.begin());
                ys.erase(ys.begin());
            }

            xs.push_back(x);
            ys.push_back(y);

            if(y > y_max_ever) y_max_ever = y;
        }

        void add_point(double y) {
            add_point(0, y);
        }

    private:
        size_t max_size;
        std::vector<double> xs;
        std::vector<double> ys;
    };

    class metrics_plot_definition {
    public:
        metrics_plot_definition(std::string label, size_t max_points) : label{label}, data{max_points} {

        }

        std::string label;
        rolling_buffer data;
    };

    /**
     * @brief General metric plot for the following:
     * - percentages (%)
     * - memory (mb, gb etc.)
     * - number counter
    */
    class metrics_plot : public component {
    public:
        metrics_plot(size_t max_points) : max_points{max_points} {
        }

        virtual const void render_visible() override;

        void add_number_plot(std::string label) {
            number_plots.emplace_back(label, max_points);
        }

        void add_memory_plot(std::string label) {
            memory_plots.emplace_back(label, max_points);
        }

        void add_perc_plot(std::string label) {
            perc_plots.emplace_back(label, max_points);
        }

        void add_number_point(size_t plot_idx, double x, double y) {
            metrics_plot_definition& def = number_plots[plot_idx];
            def.data.add_point(x, y);
        }

        void add_memory_point(size_t plot_idx, double x, double y) {
            metrics_plot_definition& def = memory_plots[plot_idx];
            def.data.add_point(x, y);
        }

        void add_perc_point(size_t plot_idx, double x, double y) {
            metrics_plot_definition& def = perc_plots[plot_idx];
            def.data.add_point(x, y);
        }

    private:
        size_t max_points;
        std::vector<metrics_plot_definition> number_plots;
        std::vector<metrics_plot_definition> memory_plots;
        std::vector<metrics_plot_definition> perc_plots;
    };

    /// <summary>
    /// Allows to put any raw imgui code conveniently
    /// </summary>
    class imgui_raw : public component {
    public:
        imgui_raw() {}

        virtual const void render_visible() override { if (raw_function) raw_function(); }

    private:
        std::function<void()> raw_function;
    };

    /// <summary>
    /// Helps to set drawing position
    /// </summary>
    class positioner : public component {
    public:
        positioner(float x, float y, bool is_movement);

        virtual const void render_visible() override;

    private:
        ImVec2 pos;
        bool is_movement;
    };

    /// <summary>
    /// To show the pop-up, just set is_visible to true
    /// </summary>
    class modal_popup : public container {
    public:
        modal_popup(grey_context& mgr, const std::string& title);

        virtual const void render() override;
        virtual const void render_visible() override;

    private:
        bool was_visible{ false };
        std::string title;
        bool sys_open{true};
    };

#ifdef _DEBUG
    class demo : public component {
    public:
        virtual const void render_visible() override;
    };
#endif

    class status_bar : public container {
    public:
        status_bar(grey_context& mgr) : container{ mgr } {}

        virtual const void render_visible() override;

    private:
        ImGuiWindowFlags flags{ ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar };
    };

    class accordion : public container {
    public:
        accordion(grey_context& mgr, const std::string& label, bool is_closeable);

        virtual const void render_visible() override;

    private:
        std::string label;
        bool is_closeable;
        bool is_open{ true };
    };

    /**
     * @brief Child (or child window) allows for independent scrollable/clippable area
    */
    class child : public container {
    public:
        child(grey_context& mgr, size_t width, size_t height, bool horizontal_scroll);

        virtual const void render_visible() override;

        bool has_border{false};

        //experimental
        size_t padding_bottom{0};

    private:
        ImVec2 size;
        ImGuiWindowFlags flags{ 0 };
    };

    /**
     * @brief Group allows for treating all the items inside as one so you can use item functions like on_hover etc.
    */
    class group : public container {
    public:
        group(grey_context& mgr);

        virtual const void render_visible() override;

        bool spread_horizontally{false};
        rgb_colour hover_border_colour{};
        rgb_colour hover_bg_colour{};
        rgb_colour border_colour{};
    };

    class window : public container {
    public:
        bool has_menu_space{ false };
        //size_t left{0}, top{0};
        bool can_resize{true};
        bool detach_on_close{false};

        /**
         * @brief 
         * @param ctx 
         * @param title 
         * @param width width, will be scaled
         * @param height height, will be scaled
        */
        window(grey_context& ctx, std::string title, float width, float height);
        ~window() override;

        virtual const void render_visible() override;

        std::function<void(bool&)> on_open_changed;

        void close();

        float get_system_scale() { return container::tmgr.get_system_scale(); }

        void center();
        void bring_to_top();

    private:
        grey_context& ctx;
        ImGuiWindowFlags flags;
        const std::string title;
        const std::string id_title;
        const bool can_close{true};
        bool initialised{false};
        bool do_center{false};
        bool do_top{false};

        bool change_pos{false};
        ImVec2 change_pos_point{0, 0};


        //const bool is_dockspace;
        //ImGuiID dockspace_id{};
        //ImGuiDockNodeFlags dockspace_flags{ ImGuiDockNodeFlags_PassthruCentralNode };

        bool was_visible{true};
    };

    template<class TDataElement>
    struct repeater_bind_context {
        repeater<TDataElement>& rpt;
        std::shared_ptr<container> container;
        std::shared_ptr<TDataElement> data;
        size_t idx;
    };

    template<class TDataElement>
    class repeater : public container {
    public:
        repeater(grey_context& ctx,
            std::function<void(repeater_bind_context<TDataElement>)> element_factory,
            bool track_selection = false)
            : container{ctx}, ctx{ctx}, make_element{element_factory}, track_selection{track_selection} {
        }

        virtual const void render_visible() override {
            render_children();
        }

        void bind(std::vector<std::shared_ptr<TDataElement>>& data) {
            clear();
            bound_groups.clear();
            ImGuiStyle& style = ImGui::GetStyle();
            size_t idx{0};
            for(auto data_element : data) {
                auto ec = std::make_shared<group>(ctx);
                assign_child(ec);
                bound_groups.push_back(ec);
                make_element(repeater_bind_context<TDataElement>{*this, ec, data_element, idx++});
                ec->spread_horizontally = true;
                if(track_selection) {
                    ec->hover_border_colour = ec->hover_bg_colour = rgb_colour{style.Colors[ImGuiCol_FrameBgHovered]};

                    ec->on_click = [this, ec, data_element](component&) {
                        ImGuiStyle& style = ImGui::GetStyle();

                        for(auto bg : bound_groups) {
                            bg->border_colour = rgb_colour{
                                style.Colors[bg->id == ec->id ? ImGuiCol_FrameBgActive : ImGuiCol_WindowBg]};
                        }

                        if(on_item_clicked) {
                            on_item_clicked(ec, data_element);
                        }
                    };

                    ec->on_hovered = [this, ec, data_element](component&, bool is_hovered) {
                        if(on_item_hovered && is_hovered) {
                            on_item_hovered(ec, data_element);
                        }
                    };
                }
            }
            this->data = data;
        }

        std::function<void(std::shared_ptr<container>, std::shared_ptr<TDataElement>)> on_item_hovered;
        std::function<void(std::shared_ptr<container>, std::shared_ptr<TDataElement>)> on_item_clicked;

        void set_selected_index(size_t idx) {
            ImGuiStyle& style = ImGui::GetStyle();

            for(int i = 0; i < bound_groups.size(); i++) {
                bound_groups[i]->border_colour = rgb_colour{
                    style.Colors[i == idx ? ImGuiCol_FrameBgActive : ImGuiCol_WindowBg]};
            }
        }

    private:
        grey_context& ctx;
        bool track_selection;
        std::vector<std::shared_ptr<group>> bound_groups;   // only to track visual state i.e. selection
        std::function<void(repeater_bind_context<TDataElement>)> make_element;
        std::vector<std::shared_ptr<TDataElement>> data;
    };
}
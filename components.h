#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include "common/containers.hpp"
#include "grey_context.h"
#include "3rdparty/memory_editor.h"

namespace grey {
    static int generate_int_id();

    struct rgb_colour {
        float r;
        float g;
        float b;
        float o;
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
        void* tag{ nullptr };

        component(const std::string& id = "");
        virtual const void render();
        virtual const void render_visible() = 0;
        void set_emphasis(emphasis em);
        emphasis get_emphasis() { return em; }

        std::function<void(component&)> on_hovered;
        std::function<void(component&)> on_click;
        std::function<void(component&)> on_frame;
        std::function<void(component&)> on_visibility_changed;

    protected:
        //emphasis
        ImVec4 em_normal;
        ImVec4 em_hovered;
        ImVec4 em_active;
        emphasis em{ emphasis::none };

        inline std::string sys_label(const std::string& label) { return label + "##" + id; }

    private:
        bool is_visible_prev_frame{ true };

        //click state
        bool state_click_pressed{ false };
        bool on_click_sent{ false };
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
    class status_bar;
    class accordion;
    class child;
    class group;
    class selectable;
#if _DEBUG
    class demo;
#endif

    class container : public component {
    public:
        container(grey_context& mgr) : tmgr{ mgr } {}

        virtual const void render() override;
        virtual const void render_visible() = 0;

        bool empty() { return managed_children.empty() && owned_children.empty(); }
        void clear() { owned_children.clear(); }
        void assign_child(std::shared_ptr<component> child);
        void assign_managed_child(std::shared_ptr<component> child);
        std::shared_ptr<component> get_child(int index);

        // general helpers
        std::shared_ptr<menu_bar> make_menu_bar();
        std::shared_ptr<label> make_label(const std::string& text);
        std::shared_ptr<label> make_label(std::string* text);
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
            auto r = make_shared<complex_table<TRowState>>(tmgr, columns);
            assign_child(r);
            return r;
        }
        std::shared_ptr<tabs> make_tabs(bool tab_list_popup = false);
        std::shared_ptr<slider> make_slider(const std::string& label, float* value, float min = 0, float max = 1);
        std::shared_ptr<progress_bar> make_progress_bar(float* value, const char* overlay_text = nullptr, float height = 10);
        std::shared_ptr<modal_popup> make_modal_popup(const std::string& title);
        std::shared_ptr<plot> make_plot(const std::string& title, std::vector<float>& values);
        std::shared_ptr<status_bar> make_status_bar();
        std::shared_ptr<accordion> make_accordion(const std::string& label);
        // child windows have their own scrolling/clipping area.
        std::shared_ptr<child> make_child_window(size_t width = 0, size_t height = 0, bool horizonal_scroll = false);
        std::shared_ptr<group> make_group();
        std::shared_ptr<selectable> make_selectable(const std::string& value);

#if _DEBUG
        std::shared_ptr<demo> make_demo();
#endif

        // edge case helpers
        void same_line();
        void spacer();
        void separator();
        std::shared_ptr<imgui_raw> make_raw_imgui();
        void set_pos(float x, float y, bool is_movement = false);


    protected:
        grey_context& tmgr;
        // managed children are not owned, but you are responsible for rendering them
        std::vector<std::shared_ptr<component>> managed_children;
        std::vector<std::shared_ptr<component>> owned_children;

        const void render_children();
    };

    class common_component : public component {
    public:
        common_component(int c) : c{ c } {}
        virtual const void render_visible() override;

    private:
        int c;
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

        label(const std::string& value) : value{ value }, value_ptr{ nullptr } {}
        label(const std::string* value) : value_ptr{ value } {}

        virtual const void render_visible() override;

        std::string get_value();
        void set_value(const std::string& value);

    private:
        std::string value;
        const std::string* value_ptr;
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
        bool is_selected{ false };
        bool is_enabled{ true };
        std::string shortcut_text;
        void* tag{ nullptr };
        std::vector<std::shared_ptr<menu_item>> children;

        std::shared_ptr<menu_item> add(const std::string& id, const std::string& label);
    };

    class menu_bar : public component {
    public:
        virtual const void render_visible() override;

        bool is_main_menu{ false };

        std::function<void(menu_item&)> clicked;

        std::shared_ptr<menu_item> items() { return root; }

    private:
        std::shared_ptr<menu_item> root{std::make_shared<menu_item>()};

        void render(std::shared_ptr<menu_item> mi);
    };

    class list_item {
    public:
        std::string text;
        std::string tooltip;
        void* tag{ nullptr };

        list_item(const std::string& text) : text{ text } {};
    };

    enum class listbox_mode {
        list = 0,
        combo
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
        void set_value(const std::string& value) { *this->value = value; }

    private:
        std::string label;
        bool owns_mem;
        std::string* value;
    };

    class input_int : public component {
    public:
        input_int(const std::string& label, int* value = nullptr);
        ~input_int();

        virtual const void render_visible() override;

        std::function<void(int&)> on_value_changed;

        int get_value() { return *value; }
        void set_value(int v) { *value = v; }

    private:
        bool owns_mem;
        std::string label;
        int* value;
    };

    class checkbox : public component {
    public:
        std::string text;
        bool is_checked() { return *value; };
        void set_checked(bool checked) { *value = checked; }
        bool is_highlighted{ false };

        checkbox(const std::string& text, bool* value = nullptr);
        ~checkbox();

        std::function<void(bool)> on_value_changed;

        virtual const void render_visible() override;

    private:
        bool owns_mem;
        bool* value;
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

            if (ImGui::BeginTable(id.c_str(), columns.size(),
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_BordersV |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            )) {
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

        void clear();
        std::shared_ptr<tab> make(const std::string& title);

        virtual const void render_visible() override;

    private:
        grey_context& mgr;
        ImGuiTabBarFlags flags;
        std::vector<std::string> tab_headers;
        std::vector<std::shared_ptr<tab>> tab_containers;
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
        plot(const std::string& label, std::vector<float>& values)
            : label{ label }, values{ values } {

        }

        int height{ 50 };
        int width{ -1 };
        bool stick_to_bottom{ false }; // todo: should be some kind of a layout manager?

        virtual const void render_visible() override;

    private:
        std::string label;
        std::vector<float>& values;
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
    /// To show the pop-up, just set is_viible to true
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
    };

    class window : public container {
    public:
        bool has_menu_space{ false };

        window(grey_context& mgr,
            std::string title,
            bool is_maximized = false,
            bool can_close = true,
            bool is_dockspace = false);

        virtual const void render_visible() override;

        std::function<void(bool&)> on_open_changed;

        void close();

        float get_system_scale() { return container::tmgr.get_system_scale(); }

    private:
        bool is_open{ true };
        bool user_is_open{ true };  // user-controlled flag
        bool is_maximized;
        ImGuiWindowFlags flags;
        std::string title;
        bool can_close;
        bool was_open{ true };
        bool first_time{ true };

        bool is_dockspace;
        ImGuiID dockspace_id{};
        ImGuiDockNodeFlags dockspace_flags{ ImGuiDockNodeFlags_PassthruCentralNode };
    };

    template<class TDataElement>
    class repeater : public container {
    public:
        repeater(grey_context& ctx,
            std::function<void(std::shared_ptr<container>, std::shared_ptr<TDataElement>)> element_factory) 
            : container{ctx}, ctx{ctx}, make_element{element_factory} {
        }

        virtual const void render_visible() override {
            //ImGui::Text("repeater begin");

            render_children();

            //ImGui::Text("repeater end");
        }

        void bind(std::vector<std::shared_ptr<TDataElement>>& data) {
            clear();
            for(auto e : data) {
                auto ec = std::make_shared<group>(ctx);
                assign_child(ec);
                //ec->make_label("repeater element");
                make_element(ec, e);
            }
        }

    private:
        grey_context& ctx;
        std::function<void(std::shared_ptr<container>, std::shared_ptr<TDataElement>)> make_element;
    };
}
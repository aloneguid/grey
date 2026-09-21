//#include "main_wnd.h"
#include "../grey/app.h"
#include "../grey/widgets.h"
#include "../grey/x/graph.h"
#include "res.inl"
#include <imgui_internal.h>
#include <vector>
#include <iostream>

#include "grey.h"
#include "common/clipboard.h"
#include "common/desktop_shell.h"

using namespace std;
using namespace grey;
namespace w = widgets;

bool app_open{true};
bool show_demo{false};
string window_title = "Demo app";
string text;
bool ned_initialised{false};
bool selected{false};
// multiline string with sample for text editor
string text_editor_text = R"(
-- This is a comment
function foo()
    print("Hello, world!")
end
)";

grey::x::graph gr;

void plot_demo() {
}

#if WIN32
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
#else
    int main(int argc, char* argv[]) {
#endif

    //auto backend = grey::backend::make_platform_default(APP_LONG_NAME);
    //auto wnd = backend->make_window<demo::main_wnd>();
    //backend->run();

    auto app = grey::app::make("demo", sz{700, 800});

    app->on_initialised = [&app]() {
        app->preload_texture("luna", luna_jpg, luna_jpg_len);
    };

    app->main_window_opts().has_menu_bar = true;
    app->fonts.load_all();
    app->center_on_screen = true;

    gr.add_node(1);
    gr.add_node(2);
    gr.add_node(3);
    gr.add_node(4);
    gr.add_edge(1, 2);
    gr.add_edge(2, 3);
    gr.add_edge(1, 4);


    app->run([&app]() {
            // menu
            {
                if(w::menu_bar menu; menu) {
                    {
                        if(w::menu m("File"); m) {
                            w::mi("New", true, ICON_MD_DONUT_LARGE);
                            if(w::mi("Exit", true)) {
                                app_open = false;
                            }
                        }
                    }

                    {
                        if(w::menu m("View"); m) {
                            w::mi_themes([&app](const std::string& id) {
                                w::toast(emphasis::info, "theme changed to " + id);
                                app->set_theme(id);
                            });
                            w::small_checkbox("Show ImGui demo", show_demo);
                        }
                    }

                    {
                        if(w::menu m("Help"); m) {
                            w::mi("About");
                        }
                    }
                }
            }

            // top tabs
            {
                w::tab_bar tabs{"topTabs", true, true};

                // basics
                if(auto tab = tabs.next_tab("Basics"); tab) {
                    if(w::accordion("Icons")) {


                        const int columns = 16;
                        int i = 0;
                        for (unsigned cp = ICON_MIN_MD; cp < ICON_MAX_16_MD; ++cp, ++i) {
                            char buf[5] = {};
                            ImTextCharToUtf8(buf, cp);          // built into ImGui (imgui_internal.h), no custom encoder needed

                            if (i % columns != 0) ImGui::SameLine();
                            ImGui::PushID((int)cp);
                            ImGui::Button(buf, ImVec2(40, 40));
                            if (ImGui::IsItemHovered())
                                ImGui::SetTooltip("U+%04X", cp);
                            ImGui::PopID();
                        }



                    }

                    if(w::accordion("Label styles")) {
                        w::lbl("");
                        for(pair<emphasis, string_view> emp: magic_enum::enum_entries<emphasis>()) {
                            string title = format("emp: {}", emp.second);
                            w::sl();
                            w::lbl(title, {.emp = emp.first});
                        }
                    }

                    if(w::accordion("Windows")) {
                        static bool title_bar{true};
                        static bool is_open{false};
                        static bool use_is_open{true};
                        static float opacity{1.0f};
                        static float border{.0f};
                        static bool scrollable{true};

                        w::checkbox("show window", is_open);
                        w::checkbox("display close button", use_is_open);
                        w::checkbox("title bar", title_bar);
                        w::slider(opacity, 0.0f, 1.0f, "opacity", 0.1f);
                        w::checkbox("scrollable", scrollable);
                        w::slider(border, 0.0f, 10.0f, "border", 0.1f);

                        if(is_open) {
                            if(w::wnd w1{
                                "windows 1", {
                                    .open_ptr = use_is_open ? &is_open : nullptr,
                                    .opacity = opacity,
                                    .show_title_bar = title_bar,
                                    .border = border,
                                    .scrollable = scrollable
                                }
                            }) {
                                w::lbl("DPI: ", {.emp = emphasis::primary});
                                w::sl();
                                w::lbl(format("{}", w::scale));

                                w::lbl("Position:", {.emp = emphasis::primary});
                                w::sl();
                                w::lbl(format("{}", w1.pos()));

                                w::lbl("Size: ", {.emp = emphasis::primary});
                                w::sl();
                                w::lbl(format("{}", w1.size()));
                            }
                        }
                    }

                    if(w::accordion("Image")) {
                        static bool img_rounded{false};
                        static float img_rounding{5.0f};
                        static float img_scale{0.5f};

                        auto texture = app->get_texture("luna");
                        if(texture) {
                            w::checkbox("rounded", img_rounded);
                            w::slider(img_scale, 0.1f, 3.0f, "scale");
                            if(img_rounded) {
                                w::slider(img_rounding, 1, 50, "rounding");
                                w::image_rounded(*app, "luna", texture->size * img_scale, img_rounding);
                            } else {
                                w::image(*app, "luna", texture->size * img_scale);
                            }
                        }
                    }

                    if(w::accordion("Tooltips")) {
                        static string tooltip_text{"tooltip text"};
                        static show_delay sd{show_delay::normal};

                        w::input(tooltip_text, "text");

                        w::enum_combo("show delay", sd);

                        w::hyperlink("simple");
                        w::tt(tooltip_text, sd);

                        w::sl();
                        w::hyperlink("rich");
                        if(w::rich_tt tt{sd}; tt) {
                            w::lbl("rich one", {.emp = emphasis::primary});
                            w::sep();
                            w::lbl(tooltip_text);
                        }
                    }

                    if(w::accordion("Combo")) {
                        static unsigned selected{0};
                        w::combo("elements", {"one", "two", "three"}, selected);
                        w::sl();
                        w::lbl(format("current: {}", selected));

                        static show_delay selected_delay{show_delay::normal};
                        w::enum_combo<show_delay>("enum", selected_delay);
                        w::sl();
                        w::lbl(format("current: {}", magic_enum::enum_name(selected_delay)));
                    }

                    if (w::accordion("List")) {
                        static vector<string> items = {"item1", "item2", "item3"};
                        static unsigned int current_item = 0;

                        w::list("list", items, current_item);
                        w::lbl(format("selected item: {} ({})", current_item, items[current_item]));
                    }

                    if(w::accordion("Buttons")) {
                        w::lbl("Clicking a button also opens a toast with the same emphasis");
                        for(pair<emphasis, string_view> emp: magic_enum::enum_entries<emphasis>()) {
                            w::sl();
                            if(w::button(string{emp.second}, emp.first)) {
                                w::toast(emp.first, format("Toast of {} emphasis", emp.second));
                            }
                        }
                    }

                    if(w::accordion("Radios")) {
                        w::radio("radio1", selected);
                        w::sl();
                        w::radio("radio2", !selected);
                        w::small_radio("small radio1", selected);
                        w::sl();
                        w::small_radio("small radio2", !selected);
                    }

                    if(w::accordion("Checkboxes")) {
                        w::checkbox("basic", selected);
                        w::sl();
                        w::small_checkbox("small", selected);
                    }

                    if(w::accordion("Hyperlinks")) {
                        if(w::hyperlink("click me")) {
                            w::toast(emphasis::info, "hyperlink clicked");
                        }
                        w::sl();
                        w::hyperlink("blog", "https://www.aloneguid.uk/posts/");
                    }

                    if(w::accordion("Input text")) {
                        w::input(text, "##input1");
                    }

                    if(w::accordion("Selectables")) {
                        w::selectable("default selectable");
                    }

                    if(w::accordion("Sliders")) {
                        static float slider_value_float = 0.5f;
                        static int slider_value_int = 1;
                        static bool slider_ticks = false;
                        w::checkbox("ticks", slider_ticks);
                        w::slider(slider_value_float, 0.0f, 1.0f, "slider float", 0.1f, slider_ticks);
                        w::slider(slider_value_float, 0.0f, 1.0f, "slider float (small)", 0.1f, slider_ticks,
                                  emphasis::none, true);
                        w::slider(slider_value_float, 0.0f, 1.0f, "slider float secondary", 0.1f, slider_ticks,
                                  emphasis::secondary);
                        w::slider_classic(slider_value_float, 0.0f, 1.0f, "slider classic");

                        w::slider(slider_value_int, 0, 10, "slider int", 2, slider_ticks);
                    }

                    if(w::accordion("IDs and collisions")) {
                        w::lbl("Creates 10 buttons with the same id, but they don't collide");
                        for(int i = 0; i < 10; i++) {
                            w::id_frame f{i}; // demonstrates collision avoidance when using id_frame
                            if(i > 0) w::sl();
                            w::button("collide");
                        }
                    }

                    if(w::accordion("Fonts")) {
                        static float fa_delta{0.0f};
                        w::slider(fa_delta, -100.0f, 100.0f, "size delta");

                        {
                            widgets::texter adj{fa_delta, font_weight::regular};
                            w::lbl("Fonts: regular, ");
                        }

                        {
                            widgets::texter adj{fa_delta, font_weight::bold};
                            w::sl(0, false);
                            w::lbl("bold, ");
                        }

                        {
                            widgets::texter adj{fa_delta, font_weight::fixed_size};
                            w::sl(0, false);
                            w::lbl("and monospace");
                        }
                    }

                    if(w::accordion("Div")) {
                        static div_opts opts;
                        static int line_count = 10;
                        static int line_length = 10;

                        w::sep("Content");;
                        w::slider(line_count, 1, 1000, "Line count");
                        w::slider(line_length, 1, 1000, "Line length");

                        w::sep("Options");
                        w::input(opts.size.width, "width");
                        w::input(opts.size.height, "height");
                        w::checkbox("horizontally resizeable", opts.user_resizeable_horizontal);
                        w::checkbox("vertically resizeable", opts.user_resizeable_vertical);
                        w::checkbox("has background", opts.has_background);
                        w::checkbox("auto resize X", opts.auto_resize_x);
                        w::checkbox("auto resize Y", opts.auto_resize_y);
                        w::checkbox("style like other widgets", opts.style_like_widget);
                        if(w::button("reset", emphasis::warning)) {
                            opts = {};
                        }

                        w::div ddiv{"demo_div", opts};

                        w::lbl(format("div rendered (may not be if going out of view, so you can optimise): {}", (bool)ddiv));

                        if(ddiv) {
                            string line;
                            line.resize(line_length);
                            for(int i = 0; i < line_length; i++) {
                                line[i] = 'A';
                            }

                            for(int i = 0; i < line_count; i++) {
                                w::lbl("Line " + std::to_string(i));
                                w::sl();
                                w::lbl(line);
                            }
                        }
                    }

                    if(w::accordion("Tree node")) {
                        if(w::tree_node p("Tree Node", true); p) {
                          if(w::tree_node c1("Child 1"); c1) {
                              if(w::tree_node l0("Leaf 0", false, true); l0) {
                                  w::lbl("leaf content");
                              }
                          }
                      }
                    }

                    if(w::accordion("Popup")) {
                        static bool open{false};

                        if(w::button("pop")) open = true;

                        if(w::popup pop{"status_pop", open}; pop) {
                            w::button("button inside popup");
                            w::lbl("label inside popup");
                        }
                    }

                    if(w::accordion("Spinners")) {
                        static spinner_type type{spinner_type::hbo_dots};
                        static spinner_style style{};

                        // options: first general, then speicific
                        w::enum_combo<emphasis>("emphasis", style.emp);
                        w::slider(style.radius, 5, 500, "radius");
                        w::slider(style.thickness, 1, 50, "thickness");
                        w::slider(style.speed, 0.1, 10, "speed");
                        w::slider(style.hbo.dot_count, 1, 100, "dot count");

                        // enumerate all enum members of spinner_type
                        for(const pair<spinner_type, string_view>& st: magic_enum::enum_entries<spinner_type>()) {
                            w::lbl(string{st.second});
                            w::spinner(st.first, style);
                        }

#if _DEBUG
                        if(w::accordion("Original ImSpinner demo")) {
                            w::sep("Demo");
                            w::spinner_demo();
                        }
#endif
                    }
                }

                // table
                {
                    auto tab = tabs.next_tab("Table");
                    if(tab) {
                        static int row_count = 100;
                        w::slider(row_count, 0, 1000, "row count");
                        int rows_rendered = 0;
                        if(w::table tbl{"tbl", {"col 1", "col 2", "col 3"}, .0f, -40 * w::scale}; tbl) {
                            for(int i = 0; i < row_count; i++) {
                                if(tbl.begin_row()) {
                                    rows_rendered++;
                                    w::lbl("row " + to_string(i));
                                    for(int c = 1; c < 3; c++) {
                                        if(tbl.next_column()) {
                                            w::lbl(to_string(i) + " x " + to_string(c));
                                        }
                                    }
                                }
                            }
                        }
                        w::lbl("rows rendered: ");
                        w::sl();
                        w::lbl(to_string(rows_rendered));
                    }
                }

                // big table
                {
                    auto tab = tabs.next_tab("Big table");
                    if(tab) {
                        static int row_count = 1000;
                        static int col_count = 3;
                        static bool row_selectable = false;
                        static bool row_selectable_span = false;

                        w::slider(row_count, 0, 1000000000, "row count");
                        w::slider(col_count, 1, 100, "col count");
                        w::checkbox("row selectable", row_selectable);
                        if(row_selectable) {
                            w::sl();
                            w::checkbox("span all columns", row_selectable_span);
                        }

                        static vector<string> columns;
                        if(col_count != columns.size()) {
                            columns.clear();
                            for(int i = 0; i < col_count; i++) {
                                columns.push_back("col " + to_string(i));
                            }
                        }

                        static bool row_bg = false;
                        w::checkbox("alternate row bg", row_bg);

                        w::big_table t{"table2", columns, (size_t) row_count, 0.0f, -20 * w::scale, row_bg};
                        if(t) {
                            t.render_data([](int row, int col) {
                                if(col == 0) {
                                    if(row_selectable) {
                                        if(w::selectable(to_string(row), row_selectable_span)) {
                                            w::toast(emphasis::info, "row " + to_string(row) + " selected");
                                        }
                                    } else {
                                        w::lbl(to_string(row));
                                    }
                                } else {
                                    w::lbl(to_string(row) + "x" + to_string(col));
                                }
                            });
                        }
                    }
                }

                // Multiline edit
                {
                    auto tab = tabs.next_tab("Editor");
                    if(tab) {
                        static float height = 0;
                        static bool autoscroll = false;
                        static bool enabled = true;
                        static bool use_fixed_font = false;
                        static bool use_rich_editor = false;
                        static bool ted_initialised = false;
                        w::slider(height, -500, 500, "height");
                        w::checkbox("autoscroll", autoscroll);
                        w::checkbox("enabled", enabled);
                        w::checkbox("fixed font", use_fixed_font);
                        w::checkbox("use rich editor", use_rich_editor);

                        if(use_rich_editor) {
                            //ted.render(height == 0 ? -FLT_MIN : height);
                            static w::code_editor ted;
                            if(!ted_initialised) {
                                ted.lng = w::code_editor::language::lua;
                                ted.set_text(text_editor_text);
                                ted_initialised = true;
                            }

                            ted.render();
                        } else {
                            w::input_ml("##ml", text_editor_text, height == 0 ? -FLT_MIN : height, autoscroll, enabled,
                                        use_fixed_font);
                        }
                    }
                }

                // ImPlot
                with_tab(tabs, "Plots",
                         //w::plot_demo();
                         plot_demo();)

                // markdown
                {
                    auto tab = tabs.next_tab("Markdown");
                    static w::markdown_config md_config;
                    static bool preview_only{false};
                    if(tab) {
                        float w = w::avail_x();

                        w::slider(md_config.h1_size_delta, 0.0f, 100.0f, "h1");
                        w::slider(md_config.h2_size_delta, 0.0f, 100.0f, "h2");
                        w::slider(md_config.h3_size_delta, 0.0f, 100.0f, "h3");
                        w::checkbox("preview only", preview_only);

                        static string md_text = R"(# Built-in markdown support

Uses [MD4C](https://github.com/mity/md4c) parser and takes ideas from [imgui_md](https://github.com/mekhontsev/imgui_md) (but does *not* include it).

## Emphasis

Supports **bold**, _underlined_, ~strikethrough~. However, *italic* is not there yet.

In tables, formatting is supported as well, as well as **content alignment**. In the example below, first column is right-aligned, second is centered, and third is left-aligned.

id | name | description
---: | :---: | :---
1 | aloneguid | Tables are supported.
2 | uncle | For**matting** inside _tables_.

## Lists

1. First ordered list item
2. Another item
   * Unordered sub-list 1.
   * Unordered sub-list 2.
1. Actual numbers don't matter, just that it's a number
   1. **Ordered** sub-list 1
   2. **Ordered** sub-list 2
4. And another item with minuses.
   - __sub-list with underline__
   - sub-list with escapes: \[looks like\]\(a link\)
5. ~~Item with pluses and strikethrough~~.
   + sub-list 1
   + sub-list 2
   + [Just a link](https://github.com/mekhontsev/imgui_md).
      * Item with [link1](#link1)
      * Item with bold [**link2**](#link1)

Supports `inline code` text and code fences like these:

```bash
sudo apt update
```

quoted text:

> this is a quote.

Also, [GitHub alerts](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax#alerts) are supported as well:

> [!NOTE]
> Useful information that users should know, even when skimming content.

> [!TIP]
> Helpful advice for doing things better or more easily.

> [!IMPORTANT]
> Key information users need to know to achieve their goal.

> [!WARNING]
> Urgent info that needs immediate user attention to avoid problems.

> [!CAUTION]
> Advises about risks or negative outcomes of certain actions.

)";
                        if(preview_only) {
                            w::markdown(md_text, md_config);
                        } else {
                            {
                                w::div dw{"md_left", {.size = sz{w / 2, 0}}};
                                w::input_ml("##md_input", md_text, 0.0f);
                            }
                            w::sl();
                            {
                                w::div dw{"md_right"};
                                w::markdown(md_text, md_config);
                            }
                        }
                    }
                }

                // graph
                {
                    auto tab = tabs.next_tab("X");
                    if(tab) {
                        static bool gr_auto_fr{false};
                        static string gr_connect_to;

                        if(w::button("add node")) {
                            int id = gr.get_nodes().size() + 1;
                            gr.add_node(id);
                            if(!gr_connect_to.empty()) {
                                gr.add_edge(id, std::stoi(gr_connect_to));
                            }
                        }
                        w::sl();
                        w::input(gr_connect_to, "connect to", true, 50 * w::scale);
                        w::sl();
                        w::lbl("|");
                        w::sl();
                        if(w::button("circle")) {
                            gr.layout_circle();
                        }
                        w::sl();
                        w::checkbox("auto FR", gr_auto_fr);
                        if(gr_auto_fr) {
                            gr.layout_fruchterman_reingold();
                        }

                        //gr.loop_fr_re();
                        gr.render();
                    }
                }

                // system
                {
                    if(auto tab = tabs.next_tab("sys")) {
                        w::slider(app->opacity, 0.1, 1, "Main opacity", 0.1f);

                        bool fps_control = app->fps != -1;
                        if(w::checkbox("FPS control", fps_control)) {
                            app->fps = fps_control ? 10.0f : -1;
                        }
                        if(fps_control) {
                            w::slider(app->fps, 0.0f, 500.0f, "FPS", 0.1f);
                        }

                        if(w::accordion("Monitors")) {
                            w::lbl(format("mouse pos: {}", w::mouse_pos()));
                            for(int i = 0; i < w::mon_count(); i++) {
                                auto mm = w::mon(i).value();
                                w::lbl(format("{:2d}: ", i));
                                w::sl(40);
                                w::lbl(format("scale: {}", mm.dpi_scale));
                                w::sl(160);
                                w::lbl(format("{} (work: {})", mm.area, mm.work_area));
                            }
                        }

                        if(w::button("center on screen")) {
                            app->center();
                        }

                        if(w::accordion("File Dialogs")) {

                            w::lbl(format("Support flags:\n  File open: {}\n  File save: {}\n  Directory open: {}",
                                common::desktop_shell::file_open_dialog_supported(),
                                common::desktop_shell::file_save_dialog_supported(),
                                common::desktop_shell::directory_open_dialog_supported()));

                            static string file_path;

                            if(w::button("open file")) {
                                file_path = common::desktop_shell::file_open_dialog("Text File", "*.txt");
                            }
                            w::sl();
                            if(w::button("save file")) {
                                file_path = common::desktop_shell::file_save_dialog("Text File", "*.txt");
                            }

                            if(w::button("directory open")) {
                                file_path = common::desktop_shell::directory_open_dialog();
                            }

                            w::lbl("File path: " + file_path);
                        }

                        if(w::accordion("Clipboard")) {
                            static string clip_text;
                            w::input_ml("clip", clip_text, w::scaled(200));
                            if(w::button("read"))
                                clip_text = common::clipboard::get_text();
                            w::sl();
                            if(w::button("write"))
                                common::clipboard::set_text(clip_text);
                        }
                    }
                }
            }


            with_status_bar(
                w::lbl(ICON_MD_HEAT_PUMP, {.emp=emphasis::primary});
                w::sl();
                w::lbl("|", {.emp=emphasis::disabled});

                auto sbi = [](string s, bool sep = true) {
                if(sep) {
                w::sl(); w::lbl("|", {.emp=emphasis::disabled});
                }
                w::sl();
                w::lbl(s);
                };

                sbi(format("{:.2f} FPS", ImGui::GetIO().Framerate), false);
                sbi(format("x{:.2f}", w::scale));
                sbi(ImGui::GetVersion());
            )


            if(show_demo)
                ImGui::ShowDemoWindow();

            w::toast_render_frame();

            return app_open;
        }

    );

    return
            0;
}

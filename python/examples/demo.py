"""Comprehensive Grey Python Desktop Demo Application.

Recreates dotnet/ConsoleDemo/Program.cs showcasing all widgets, containers,
styling, code editor, and Material Icons.
"""

import sys
from pathlib import Path
from datetime import datetime

# Allow running directly from repository root or python/examples directory
pkg_root = Path(__file__).resolve().parent.parent
if str(pkg_root) not in sys.path:
    sys.path.insert(0, str(pkg_root))

import grey
from grey import Icon, Emphasis, Style, ProgrammingLanguage, SpinnerType


def main() -> None:
    is_running = True
    is_checked = True
    message = "1"
    number = 42

    spinner_radius = 16.0
    spinner_thickness = 4.0
    spinner_speed = 1.0
    spinner_dot_count = 8

    multiline_text = "{\n  \"name\": \"Grey\",\n  \"language\": \"Python\",\n  \"awesome\": true\n}"
    ml_height = 0.0
    ml_enabled = True
    editor_rich = False
    ce: grey.CodeEditor | None = None
    ml_use_fixed_font = False

    alternate_table_row_bg = False
    table_selectables = False
    table_selectable_row = False

    choices = ["one", "two", "three"]
    current_choice = 0

    # Window configuration states
    window_has_title_bar = True
    window_is_open = False
    window_use_is_open = True
    window_opacity = 1.0
    window_border = 0.0
    window_scrollable = True

    def basics_tab() -> None:
        nonlocal is_running, is_checked, message, number
        nonlocal window_has_title_bar, window_is_open, window_use_is_open, window_opacity, window_border, window_scrollable

        if grey.accordion("Icons"):
            grey.lbl(Icon.Access_alarms)

        if grey.accordion("Label styles"):
            grey.lbl("")
            for emp in [
                Emphasis.NONE,
                Emphasis.PRIMARY,
                Emphasis.SECONDARY,
                Emphasis.SUCCESS,
                Emphasis.ERROR,
                Emphasis.WARNING,
                Emphasis.INFO,
                Emphasis.DISABLED,
            ]:
                grey.sl()
                grey.lbl(emp.name, Style(emp=emp))

        if grey.accordion("Windows"):
            _, window_is_open = grey.checkbox("show window", window_is_open)
            _, window_use_is_open = grey.checkbox("display close button", window_use_is_open)
            _, window_has_title_bar = grey.checkbox("title bar", window_has_title_bar)
            _, window_opacity = grey.slider_float("opacity", window_opacity, 0.0, 1.0, 0.1)
            _, window_scrollable = grey.checkbox("scrollable", window_scrollable)
            _, window_border = grey.slider_float("border", window_border, 0.0, 10.0, 0.1)
            grey.lbl("window API is not marshalled yet", Style(emp=Emphasis.ERROR))

        if grey.accordion("Image"):
            grey.lbl("Image API is not marshalled yet.")

        grey.lbl("Buttons:")
        for emp in [
            Emphasis.NONE,
            Emphasis.PRIMARY,
            Emphasis.SECONDARY,
            Emphasis.SUCCESS,
            Emphasis.ERROR,
            Emphasis.WARNING,
            Emphasis.INFO,
            Emphasis.DISABLED,
        ]:
            grey.sl()
            grey.button(emp.name, emp)

        grey.lbl(message)

        if grey.button("get current date (has tooltip)"):
            message = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        grey.lbl("Tooltips:")
        grey.sl()
        grey.lbl("simple")
        grey.tt("simple tooltip")

        grey.sl()
        grey.lbl("rich")

        def render_rich_tt():
            grey.sep("rich")
            grey.lbl("rich tooltip content")
            grey.button("close")

        grey.tt(render_rich_tt)

        grey.sl()
        if grey.button("Quit"):
            is_running = False

        grey.sl()
        grey.button("small button", is_small=True)

        _, is_checked = grey.checkbox("checkbox", is_checked)
        grey.sl()
        _, is_checked = grey.small_checkbox("small checkbox", is_checked)

        if grey.accordion("Normal accordion"):
            grey.lbl("accordion content")

        if grey.accordion("Open accordion", default_open=True):
            grey.lbl("accordion content (open)")

        if grey.hyperlink("click me"):
            grey.toast(Emphasis.INFO, "hyperlink clicked")

        grey.hyperlink("blog", "https://www.aloneguid.uk/posts/")

        chg, message = grey.input("default input", message)
        if chg:
            grey.toast(Emphasis.INFO, "input changed")

        grey.input("disabled input", message, enabled=False)
        grey.input("readonly input", message, is_readonly=True)
        grey.input("width explicitly set to 400", message, width=400.0)
        _, number = grey.input_int("number input", number)

        grey.sep("Mouse helpers")

        grey.lbl("hovered: ")
        grey.sl()
        grey.lbl("yes" if grey.is_hovered() else "no")

        grey.lbl("left clicked: ")
        grey.sl()
        grey.lbl("yes" if grey.is_leftclicked() else "no")

        grey.lbl("right clicked: ")
        grey.sl()
        grey.lbl("yes" if grey.is_rightclicked() else "no")

        # Collision demo
        for i in range(5):
            if i > 0:
                grey.sl()
            grey.id_frame(i, lambda: grey.button("collide"))

    def render_frame() -> bool:
        nonlocal is_running, current_choice, choices
        nonlocal spinner_radius, spinner_thickness, spinner_speed, spinner_dot_count
        nonlocal ml_height, ml_enabled, ml_use_fixed_font, editor_rich, ce, multiline_text
        nonlocal alternate_table_row_bg, table_selectables, table_selectable_row

        # Top menu bar
        def render_menu():
            def render_file():
                nonlocal is_running
                if grey.menu_item("Quit", reserve_icon_space=True, icon=Icon.Exit_to_app):
                    is_running = False

            grey.menu("File", render_file)

        grey.menu_bar(render_menu)

        # Tab bar
        def render_tabs(tb: grey.TabBarActions):
            nonlocal current_choice, choices
            nonlocal spinner_radius, spinner_thickness, spinner_speed, spinner_dot_count
            nonlocal ml_height, ml_enabled, ml_use_fixed_font, editor_rich, ce, multiline_text
            nonlocal alternate_table_row_bg, table_selectables, table_selectable_row

            # Tab 1: Basics
            tb.tab_item("Basics", basics_tab)

            # Tab 2: Lists
            def lists_tab():
                nonlocal current_choice
                chg_c, current_choice = grey.combo("combo", choices, current_choice)
                if chg_c:
                    grey.toast(Emphasis.INFO, f"COMBO choice changed to {choices[current_choice]}")

                chg_l, current_choice = grey.list_box("list", choices, current_choice)
                if chg_l:
                    grey.toast(Emphasis.INFO, f"LIST choice changed to {choices[current_choice]}")

                grey.lbl(f"current: {current_choice}")

            tb.tab_item("Lists", lists_tab)

            # Tab 3: Icons
            tb.tab_item(
                "Icons",
                lambda: grey.lbl(f"{Icon.Num10k} {Icon.Fireplace} {Icon.Access_alarm}"),
            )

            # Tab 4: Spinner
            def spinner_tab():
                nonlocal spinner_radius, spinner_thickness, spinner_speed, spinner_dot_count
                _, spinner_radius = grey.slider_float("radius", spinner_radius, 1.0, 100.0)
                _, spinner_thickness = grey.slider_float("thickness", spinner_thickness, 1.0, 50.0)
                _, spinner_speed = grey.slider_float("speed", spinner_speed, 0.1, 10.0)
                _, spinner_dot_count = grey.slider_int("dot count", spinner_dot_count, 1, 100)
                grey.spinner(SpinnerType.HBO_DOTS)

            tb.tab_item("Spinner", spinner_tab)

            # Tab 5: Editor
            def editor_tab():
                nonlocal ml_height, ml_enabled, ml_use_fixed_font, editor_rich, ce, multiline_text
                _, ml_height = grey.slider_float("height", ml_height, -1000.0, 1000.0)
                _, ml_enabled = grey.checkbox("enabled", ml_enabled)
                _, ml_use_fixed_font = grey.checkbox("use fixed font", ml_use_fixed_font)
                _, editor_rich = grey.checkbox("rich editor", editor_rich)

                if editor_rich:
                    if ce is None:
                        ce = grey.CodeEditor(ProgrammingLanguage.JSON, text=multiline_text)
                    ce.render()
                else:
                    chg, multiline_text = grey.input_multiline(
                        "multiline",
                        multiline_text,
                        ml_height,
                        autoscroll=False,
                        enabled=ml_enabled,
                        use_fixed_font=ml_use_fixed_font,
                    )
                    if chg:
                        grey.toast(Emphasis.INFO, "multiline changed")

            tb.tab_item("Editor", editor_tab)

            # Tab 6: Collapsibles
            def collapsibles_tab():
                def render_parent(is_open: bool):
                    if is_open:
                        grey.tree_node("child 1", is_leaf=True)

                        def render_child2(open2: bool):
                            if open2:
                                grey.tree_node("grand 1", is_leaf=True)
                                grey.tree_node("grand 2", is_leaf=True)

                        grey.tree_node("child 2", render_child2, open_by_default=False)

                grey.tree_node("parent", render_parent, open_by_default=True)

            tb.tab_item("Collapsibles", collapsibles_tab)

            # Tab 7: Table
            def table_tab():
                def render_tbl(ta: grey.TableActions):
                    with ta.row():
                        with ta.column():
                            grey.lbl("1")
                        with ta.column():
                            grey.lbl("Grey")
                        with ta.column():
                            grey.lbl("A cross-platform immediate mode GUI library for Python and .NET")

                    # Nested tree inside table
                    with ta.row():
                        def render_root(is_open: bool):
                            with ta.column():
                                grey.lbl("--")
                            with ta.column():
                                grey.lbl("--")
                            if is_open:
                                with ta.row():
                                    with ta.column():
                                        grey.lbl("2")
                                    with ta.column():
                                        grey.lbl("Two")
                                    with ta.column():
                                        grey.lbl("Two is a good number")

                        grey.tree_node("root", render_root, open_by_default=True)

                grey.table("t0", ["id", "name", "description+"], render_tbl)

            tb.tab_item("Table", table_tab)

            # Tab 8: Big table
            def big_table_tab():
                nonlocal alternate_table_row_bg, table_selectables, table_selectable_row
                _, alternate_table_row_bg = grey.checkbox("alg bg", alternate_table_row_bg)
                _, table_selectables = grey.checkbox("selectable", table_selectables)
                if table_selectables:
                    grey.sl()
                    _, table_selectable_row = grey.checkbox("entire row", table_selectable_row)

                def cell_render(row: int, column: int):
                    if column == 0:
                        if table_selectables:
                            if grey.selectable(f"{row}", table_selectable_row):
                                grey.toast(Emphasis.INFO, f"clicked {row}x{column}")
                        else:
                            grey.lbl(str(row))
                    else:
                        grey.lbl(f"{row}x{column}")

                grey.big_table(
                    "t0",
                    ["id", "name", "description+"],
                    100000,
                    cell_render,
                    alternate_row_bg=alternate_table_row_bg,
                )

            tb.tab_item("Big table", big_table_tab)

        grey.tab_bar("1", render_tabs)

        # Status bar
        def render_status():
            grey.lbl(f"{grey.get_fps():.1f} FPS | Dear ImGui {grey.get_version()}")

        grey.status_bar(render_status)

        return is_running

    grey.run(
        "Grey Python Demo",
        render_frame,
        width=1000,
        height=750,
        has_menu_bar=True,
        can_scroll=False,
        center_on_screen=True,
    )


if __name__ == "__main__":
    main()

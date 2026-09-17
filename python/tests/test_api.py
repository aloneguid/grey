"""Unit tests for high-level Python API, context helpers, and widgets."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import grey
from grey.enums import Emphasis, ShowDelay, ProgrammingLanguage, SpinnerType
from grey.models import Style
from grey.icons import Icon
from grey.code_editor import CodeEditor
from grey._loader import find_library_path


class TestApi(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.has_native = find_library_path() is not None

    def test_icons_present(self):
        self.assertEqual(Icon.Exit_to_app, "\ue879")
        self.assertEqual(Icon.Access_alarms, "\ue191")
        self.assertIsInstance(Icon.Num10k, str)

    def test_version_retrieval(self):
        if not self.has_native:
            self.skipTest("Native library not available")
        version = grey.get_version()
        self.assertIsInstance(version, str)
        self.assertTrue(len(version) > 0)
        # Test alias
        self.assertEqual(grey.GreyVersion, version)

    def test_clipboard_operations(self):
        if not self.has_native:
            self.skipTest("Native library not available")
        test_msg = "Grey Framework Python Binding Test"
        grey.set_clipboard_text(test_msg)
        read_msg = grey.get_clipboard_text()
        self.assertEqual(read_msg, test_msg)

    def test_code_editor_object_lifecycle(self):
        with CodeEditor(ProgrammingLanguage.PYTHON) as editor:
            self.assertEqual(editor.language, ProgrammingLanguage.PYTHON)
            editor.text = "print('Hello, Grey!')"
            self.assertEqual(editor.text, "print('Hello, Grey!')")
        # Disposed editor cannot be rendered
        with self.assertRaises(RuntimeError):
            editor.render()

    def test_full_widget_render_loop(self):
        """Runs one full render frame testing all widgets, containers, and callbacks."""
        if not self.has_native:
            self.skipTest("Native library not available")

        frame_ran = False
        tested_widgets = {}

        def render_frame() -> bool:
            nonlocal frame_ran
            frame_ran = True

            # Metrics
            fps = grey.get_fps()
            self.assertGreaterEqual(fps, 0.0)

            # Labels and styling
            grey.lbl("Basic Label")
            grey.lbl("Styled Label", Style(Emphasis.SUCCESS))

            # Buttons & Selectable
            grey.button("Button 1", Emphasis.PRIMARY)
            grey.button("Small Disabled Button", Emphasis.DISABLED, is_enabled=False, is_small=True)
            grey.selectable("Selectable Item")

            # Checkbox
            chg1, chk1 = grey.checkbox("Check 1", True)
            chg2, chk2 = grey.small_checkbox("Check 2", False)
            tested_widgets["checkbox"] = (chk1, chk2)

            # Text & numeric inputs
            chg_s, str_val = grey.input("Input text", "sample")
            chg_i, int_val = grey.input_int("Input int", 42)
            chg_m, ml_val = grey.input_multiline("ml_id", "multi\nline")
            tested_widgets["inputs"] = (str_val, int_val, ml_val)

            # Sliders
            chg_si, s_int = grey.slider_int("Slider Int", 5, 0, 10)
            chg_sf, s_flt = grey.slider_float("Slider Float", 2.5, 0.0, 5.0)
            tested_widgets["sliders"] = (s_int, s_flt)

            # Combo and List
            chg_c, c_sel = grey.combo("Combo Box", ["Option A", "Option B"], 0)
            chg_l, l_sel = grey.list_box("List Box", ["Item 1", "Item 2"], 1)
            tested_widgets["selections"] = (c_sel, l_sel)

            # Tooltip
            grey.tt("Plain Tooltip", ShowDelay.QUICK)
            grey.tt(lambda: grey.lbl("Rich Tooltip Content"), ShowDelay.NORMAL)

            # Separator, Accordion, Hyperlink, Toast, Spinner
            grey.sep("Section")
            grey.accordion("Collapsible Group", default_open=False)
            grey.hyperlink("Grey Website", "https://github.com/aloneguid/grey")
            grey.toast(Emphasis.INFO, "Notification message")
            grey.spinner(SpinnerType.ROTATED_HEART)

            # Scoped ID frame
            grey.id_frame(999, lambda: grey.lbl("Scoped element"))

            # Menu bar & menu items
            def render_menu():
                def render_items():
                    grey.menu_item("New File", reserve_icon_space=True)
                    grey.menu_item("Save")
                grey.menu("File", render_items)
            grey.menu_bar(render_menu)

            # Big Table
            grey.big_table(
                "big_tbl",
                ["C1", "C2"],
                row_count=2,
                cell_render=lambda r, c: grey.lbl(f"{r}:{c}"),
            )

            # Regular Table with TableActions and Context Managers
            def render_table(actions: grey.TableActions):
                with actions.row():
                    with actions.column():
                        grey.lbl("Row 0 Col 0")
                    with actions.column():
                        grey.lbl("Row 0 Col 1")

            grey.table("reg_tbl", ["Header 1", "Header 2"], render_table)

            # Tab Bar with TabBarActions
            def render_tabs(tab_bar: grey.TabBarActions):
                tab_bar.tab_item("Tab 1", lambda: grey.lbl("Inside Tab 1"))
                tab_bar.tab_item("Tab 2", lambda: grey.lbl("Inside Tab 2"))

            grey.tab_bar("my_tabbar", render_tabs)

            # Tree Node
            grey.tree_node("Tree Root", lambda is_open: grey.lbl("Tree Child"))
            grey.tree_node("Leaf Item", is_leaf=True)

            # Status Bar
            grey.status_bar(lambda: grey.lbl("Ready"))

            # Single frame exit
            return False

        grey.run("Test Frame", render_frame, width=400, height=300)
        self.assertTrue(frame_ran)
        self.assertIsNone(grey.app._active_frame_callback)
        self.assertEqual(tested_widgets["checkbox"], (True, False))
        self.assertEqual(tested_widgets["inputs"], ("sample", 42, "multi\nline"))
        self.assertEqual(tested_widgets["sliders"], (5, 2.5))
        self.assertEqual(tested_widgets["selections"], (0, 1))


if __name__ == "__main__":
    unittest.main()

"""Unit tests for ctypes signatures and native function declarations."""

import ctypes
import os
import sys
import unittest
from pathlib import Path

# Ensure package is on sys.path
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from grey.enums import Emphasis, SubEmphasis, ShowDelay, ProgrammingLanguage, SpinnerType
from grey.models import CStyle, Style, DebugInfo
from grey._native import (
    FUNCTION_SIGNATURES,
    RenderFrameCallback,
    RenderCallback,
    RenderTreeNodeCallback,
    RenderTableCellCallback,
    RenderPtrCallback,
    get_native,
)
from grey._loader import find_library_path


class TestNativeSignatures(unittest.TestCase):
    def test_cstyle_structure(self):
        cs = CStyle(emp=int(Emphasis.PRIMARY))
        self.assertEqual(cs.emp, 1)
        self.assertEqual(ctypes.sizeof(CStyle), 4)

    def test_style_conversion(self):
        s = Style(emp=Emphasis.ERROR)
        cs = s.to_cstyle()
        self.assertEqual(cs.emp, 4)
        s2 = Style.from_cstyle(cs)
        self.assertEqual(s2.emp, Emphasis.ERROR)

    def test_enums_values(self):
        self.assertEqual(Emphasis.NONE, 0)
        self.assertEqual(Emphasis.PRIMARY, 1)
        self.assertEqual(Emphasis.DISABLED, 7)

        self.assertEqual(SubEmphasis.NORMAL, 0)
        self.assertEqual(SubEmphasis.ACTIVE, 3)

        self.assertEqual(ShowDelay.IMMEDIATE, 0)
        self.assertEqual(ShowDelay.SLOW, 3)

        self.assertEqual(ProgrammingLanguage.NONE, 0)
        self.assertEqual(ProgrammingLanguage.PYTHON, 4)
        self.assertEqual(ProgrammingLanguage.MARKDOWN, 8)

        self.assertEqual(SpinnerType.HBO_DOTS, 0)
        self.assertEqual(SpinnerType.ROTATED_HEART, 1)

    def test_callbacks_types(self):
        self.assertTrue(issubclass(RenderFrameCallback, ctypes._CFuncPtr))
        self.assertTrue(issubclass(RenderCallback, ctypes._CFuncPtr))
        self.assertTrue(issubclass(RenderTreeNodeCallback, ctypes._CFuncPtr))
        self.assertTrue(issubclass(RenderTableCellCallback, ctypes._CFuncPtr))
        self.assertTrue(issubclass(RenderPtrCallback, ctypes._CFuncPtr))

    def test_function_signatures_count(self):
        # All 40 functions from grey/x.h must be present
        self.assertEqual(len(FUNCTION_SIGNATURES), 40)
        expected_functions = [
            "app_run",
            "id_frame",
            "sl",
            "lbl",
            "selectable",
            "checkbox",
            "button",
            "sep",
            "accordion",
            "hyperlink",
            "toast",
            "input_string",
            "input_int",
            "input_multiline",
            "slider_int",
            "slider_float",
            "tt",
            "rich_tt",
            "combo",
            "list",
            "tab_bar",
            "tab",
            "spinner",
            "status_bar",
            "is_hovered",
            "is_leftclicked",
            "is_rightclicked",
            "big_table",
            "table",
            "table_begin_row",
            "table_next_column",
            "tree_node",
            "menu_bar",
            "menu",
            "menu_item",
            "code_editor",
            "clipboard_get_text",
            "clipboard_set_text",
            "get_fps",
            "get_version",
        ]
        for name in expected_functions:
            self.assertIn(name, FUNCTION_SIGNATURES, f"Missing signature for {name}")

    def test_native_bindings_load(self):
        path = find_library_path()
        if path is not None:
            native = get_native()
            for func_name in FUNCTION_SIGNATURES:
                func = getattr(native, func_name, None)
                self.assertIsNotNone(func, f"Function {func_name} was not bound on NativeBindings")


if __name__ == "__main__":
    unittest.main()

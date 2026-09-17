"""Low-level ctypes declarations and function bindings for xgrey C ABI (grey/x.h)."""

import ctypes
from typing import Dict, Tuple, List, Optional
from .models import CStyle
from ._loader import load_library

# Callback function pointer types matching grey/x.h
RenderFrameCallback = ctypes.CFUNCTYPE(ctypes.c_bool)
RenderCallback = ctypes.CFUNCTYPE(None)
RenderTreeNodeCallback = ctypes.CFUNCTYPE(None, ctypes.c_bool)
RenderTableCellCallback = ctypes.CFUNCTYPE(None, ctypes.c_int32, ctypes.c_int32)
RenderPtrCallback = ctypes.CFUNCTYPE(None, ctypes.c_void_p)

# Complete dictionary of exported function signatures: name -> (argtypes, restype)
FUNCTION_SIGNATURES: Dict[str, Tuple[List[object], object]] = {
    "app_run": (
        [
            ctypes.c_char_p,
            ctypes.c_int32,
            ctypes.c_int32,
            ctypes.c_bool,
            ctypes.c_bool,
            ctypes.c_bool,
            RenderFrameCallback,
        ],
        None,
    ),
    "id_frame": (
        [ctypes.c_int32, RenderCallback],
        None,
    ),
    "sl": (
        [ctypes.c_float],
        None,
    ),
    "lbl": (
        [ctypes.c_char_p, ctypes.POINTER(CStyle)],
        None,
    ),
    "selectable": (
        [ctypes.c_char_p, ctypes.c_bool],
        ctypes.c_bool,
    ),
    "checkbox": (
        [ctypes.c_char_p, ctypes.POINTER(ctypes.c_bool), ctypes.c_bool],
        ctypes.c_bool,
    ),
    "button": (
        [ctypes.c_char_p, ctypes.c_int32, ctypes.c_bool, ctypes.c_bool],
        ctypes.c_bool,
    ),
    "sep": (
        [ctypes.c_char_p],
        None,
    ),
    "accordion": (
        [ctypes.c_char_p, ctypes.c_bool],
        ctypes.c_bool,
    ),
    "hyperlink": (
        [ctypes.c_char_p, ctypes.c_char_p],
        ctypes.c_bool,
    ),
    "toast": (
        [ctypes.c_int32, ctypes.c_char_p],
        None,
    ),
    "input_string": (
        [
            ctypes.c_char_p,
            ctypes.c_int32,
            ctypes.c_char_p,
            ctypes.c_bool,
            ctypes.c_float,
            ctypes.c_bool,
        ],
        ctypes.c_bool,
    ),
    "input_int": (
        [
            ctypes.POINTER(ctypes.c_int32),
            ctypes.c_char_p,
            ctypes.c_bool,
            ctypes.c_float,
            ctypes.c_bool,
        ],
        ctypes.c_bool,
    ),
    "input_multiline": (
        [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_int32,
            ctypes.c_float,
            ctypes.c_bool,
            ctypes.c_bool,
            ctypes.c_bool,
        ],
        ctypes.c_bool,
    ),
    "slider_int": (
        [
            ctypes.POINTER(ctypes.c_int32),
            ctypes.c_int32,
            ctypes.c_int32,
            ctypes.c_char_p,
            ctypes.c_int32,
            ctypes.c_bool,
            ctypes.c_int32,
        ],
        ctypes.c_bool,
    ),
    "slider_float": (
        [
            ctypes.POINTER(ctypes.c_float),
            ctypes.c_float,
            ctypes.c_float,
            ctypes.c_char_p,
            ctypes.c_float,
            ctypes.c_bool,
            ctypes.c_int32,
        ],
        ctypes.c_bool,
    ),
    "tt": (
        [ctypes.c_char_p, ctypes.c_int32],
        None,
    ),
    "rich_tt": (
        [RenderCallback, ctypes.c_int32],
        None,
    ),
    "combo": (
        [
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_char_p),
            ctypes.c_int32,
            ctypes.POINTER(ctypes.c_uint32),
            ctypes.c_float,
        ],
        ctypes.c_bool,
    ),
    "list": (
        [
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_char_p),
            ctypes.c_int32,
            ctypes.POINTER(ctypes.c_uint32),
            ctypes.c_float,
        ],
        ctypes.c_bool,
    ),
    "tab_bar": (
        [ctypes.c_char_p, RenderPtrCallback],
        None,
    ),
    "tab": (
        [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_bool,
            ctypes.c_bool,
            RenderCallback,
        ],
        None,
    ),
    "spinner": (
        [ctypes.c_int32],
        None,
    ),
    "status_bar": (
        [RenderCallback],
        None,
    ),
    "is_hovered": (
        [],
        ctypes.c_bool,
    ),
    "is_leftclicked": (
        [],
        ctypes.c_bool,
    ),
    "is_rightclicked": (
        [],
        ctypes.c_bool,
    ),
    "big_table": (
        [
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_char_p),
            ctypes.c_int32,
            ctypes.c_int32,
            ctypes.c_float,
            ctypes.c_float,
            ctypes.c_bool,
            RenderTableCellCallback,
        ],
        None,
    ),
    "table": (
        [
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_char_p),
            ctypes.c_int32,
            ctypes.c_float,
            ctypes.c_float,
            ctypes.c_bool,
            RenderPtrCallback,
        ],
        None,
    ),
    "table_begin_row": (
        [ctypes.c_void_p],
        ctypes.c_bool,
    ),
    "table_next_column": (
        [ctypes.c_void_p],
        ctypes.c_bool,
    ),
    "tree_node": (
        [
            ctypes.c_char_p,
            ctypes.c_bool,
            ctypes.c_bool,
            ctypes.c_bool,
            RenderTreeNodeCallback,
        ],
        None,
    ),
    "menu_bar": (
        [RenderCallback],
        None,
    ),
    "menu": (
        [ctypes.c_char_p, RenderCallback],
        None,
    ),
    "menu_item": (
        [ctypes.c_char_p, ctypes.c_bool, ctypes.c_char_p],
        ctypes.c_bool,
    ),
    "code_editor": (
        [ctypes.c_int32, ctypes.c_bool, ctypes.c_int32, ctypes.c_char_p],
        ctypes.c_int32,
    ),
    "clipboard_get_text": (
        [ctypes.c_char_p, ctypes.c_int32],
        ctypes.c_int,
    ),
    "clipboard_set_text": (
        [ctypes.c_char_p],
        None,
    ),
    "get_fps": (
        [],
        ctypes.c_float,
    ),
    "get_version": (
        [ctypes.c_char_p, ctypes.c_int32],
        ctypes.c_int,
    ),
}


class NativeBindings:
    """Encapsulates ctypes function bindings for xgrey."""

    def __init__(self, cdll: ctypes.CDLL):
        self._cdll = cdll
        for func_name, (argtypes, restype) in FUNCTION_SIGNATURES.items():
            func = getattr(cdll, func_name, None)
            if func is not None:
                func.argtypes = argtypes
                func.restype = restype
                setattr(self, func_name, func)
            else:
                raise AttributeError(f"Function '{func_name}' not found in native library.")


_native_instance: Optional[NativeBindings] = None


def get_native() -> NativeBindings:
    """Returns the singleton NativeBindings instance, loading the library if needed."""
    global _native_instance
    if _native_instance is None:
        cdll = load_library()
        _native_instance = NativeBindings(cdll)
    return _native_instance

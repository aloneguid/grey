"""High-level Pythonic wrapper for the Grey framework."""

import ctypes
from typing import Callable, Sequence, Optional, Union, Tuple
from .enums import Emphasis, ShowDelay, SpinnerType
from .models import Style, CStyle
from .context import TableActions, TabBarActions
from ._native import (
    get_native,
    RenderFrameCallback,
    RenderCallback,
    RenderTreeNodeCallback,
    RenderTableCellCallback,
    RenderPtrCallback,
)

# Reference table for callbacks to guarantee they remain referenced during native execution
_active_frame_callback: Optional[RenderFrameCallback] = None
_cached_version: Optional[str] = None


def run(
    title: str,
    render_frame: Callable[[], bool],
    width: int = 800,
    height: int = 600,
    has_menu_bar: bool = False,
    can_scroll: bool = True,
    center_on_screen: bool = False,
) -> None:
    """Initializes the window and runs the main immediate-mode render loop."""
    global _active_frame_callback
    native = get_native()
    c_title = title.encode("utf-8")
    _active_frame_callback = RenderFrameCallback(render_frame)
    try:
        native.app_run(
            c_title,
            int(width),
            int(height),
            bool(has_menu_bar),
            bool(can_scroll),
            bool(center_on_screen),
            _active_frame_callback,
        )
    finally:
        _active_frame_callback = None


def id_frame(scope_id: int, render: Callable[[], None]) -> None:
    """Creates an ID scope frame to avoid ID collisions in repeated UI blocks."""
    native = get_native()
    cb = RenderCallback(render)
    native.id_frame(int(scope_id), cb)


def sl(offset: float = 0.0) -> None:
    """Same-line layout helper: places the next widget on the same horizontal line."""
    native = get_native()
    native.sl(float(offset))


def lbl(text: str, style: Optional[Style] = None) -> None:
    """Renders a text label with optional visual emphasis style."""
    native = get_native()
    c_text = text.encode("utf-8")
    if style is not None:
        cs = style.to_cstyle()
        native.lbl(c_text, ctypes.byref(cs))
    else:
        native.lbl(c_text, None)


def selectable(text: str, span_columns: bool = False) -> bool:
    """Renders a selectable item. Returns True if clicked."""
    native = get_native()
    return bool(native.selectable(text.encode("utf-8"), bool(span_columns)))


def checkbox(label: str, is_checked: bool, is_small: bool = False) -> Tuple[bool, bool]:
    """Renders a checkbox. Returns (changed, new_checked_value)."""
    native = get_native()
    c_checked = ctypes.c_bool(is_checked)
    changed = bool(
        native.checkbox(label.encode("utf-8"), ctypes.byref(c_checked), bool(is_small))
    )
    return changed, bool(c_checked.value)


def small_checkbox(label: str, is_checked: bool) -> Tuple[bool, bool]:
    """Renders a small checkbox. Returns (changed, new_checked_value)."""
    return checkbox(label, is_checked, is_small=True)


def button(
    text: str,
    emphasis: Emphasis = Emphasis.NONE,
    is_enabled: bool = True,
    is_small: bool = False,
) -> bool:
    """Renders a push button. Returns True if clicked."""
    native = get_native()
    return bool(
        native.button(
            text.encode("utf-8"), int(emphasis), bool(is_enabled), bool(is_small)
        )
    )


def sep(text: str = "") -> None:
    """Renders a horizontal separator with an optional label."""
    native = get_native()
    native.sep(text.encode("utf-8"))


def accordion(header: str, default_open: bool = False) -> bool:
    """Renders a collapsible accordion section. Returns True if open."""
    native = get_native()
    return bool(native.accordion(header.encode("utf-8"), bool(default_open)))


def hyperlink(text: str, url_to_open: Optional[str] = None) -> bool:
    """Renders a clickable hyperlink. Returns True if clicked."""
    native = get_native()
    c_url = url_to_open.encode("utf-8") if url_to_open is not None else None
    return bool(native.hyperlink(text.encode("utf-8"), c_url))


def toast(emphasis: Emphasis, message: str) -> None:
    """Displays a transient toast notification overlay."""
    native = get_native()
    native.toast(int(emphasis), message.encode("utf-8"))


def input(
    label: str,
    value: str,
    enabled: bool = True,
    width: float = 0.0,
    is_readonly: bool = False,
    max_length: int = 1024,
) -> Tuple[bool, str]:
    """Renders a single-line text input field. Returns (changed, new_string_value)."""
    native = get_native()
    val_bytes = value.encode("utf-8")
    buf_size = max(len(val_bytes) * 2 + 64, max_length)
    buf = ctypes.create_string_buffer(val_bytes, buf_size)
    changed = bool(
        native.input_string(
            buf,
            ctypes.sizeof(buf),
            label.encode("utf-8"),
            bool(enabled),
            float(width),
            bool(is_readonly),
        )
    )
    if changed:
        return True, buf.value.decode("utf-8", errors="replace")
    return False, value


def input_int(
    label: str,
    value: int,
    enabled: bool = True,
    width: float = 0.0,
    is_readonly: bool = False,
) -> Tuple[bool, int]:
    """Renders an integer input field. Returns (changed, new_int_value)."""
    native = get_native()
    c_val = ctypes.c_int32(value)
    changed = bool(
        native.input_int(
            ctypes.byref(c_val),
            label.encode("utf-8"),
            bool(enabled),
            float(width),
            bool(is_readonly),
        )
    )
    return changed, int(c_val.value)


def input_multiline(
    id: str,
    value: str,
    height: float = 0.0,
    autoscroll: bool = False,
    enabled: bool = True,
    use_fixed_font: bool = False,
    max_length: int = 65536,
) -> Tuple[bool, str]:
    """Renders a multi-line text editing area. Returns (changed, new_string_value)."""
    native = get_native()
    val_bytes = value.encode("utf-8")
    buf_size = max(len(val_bytes) * 2 + 256, max_length)
    buf = ctypes.create_string_buffer(val_bytes, buf_size)
    changed = bool(
        native.input_multiline(
            id.encode("utf-8"),
            buf,
            ctypes.sizeof(buf),
            float(height),
            bool(autoscroll),
            bool(enabled),
            bool(use_fixed_font),
        )
    )
    if changed:
        return True, buf.value.decode("utf-8", errors="replace")
    return False, value


def slider_int(
    label: str,
    value: int,
    min_val: int,
    max_val: int,
    step: int = 0,
    ticks: bool = False,
    emphasis: Emphasis = Emphasis.NONE,
) -> Tuple[bool, int]:
    """Renders an integer slider. Returns (changed, new_int_value)."""
    native = get_native()
    c_val = ctypes.c_int32(value)
    changed = bool(
        native.slider_int(
            ctypes.byref(c_val),
            int(min_val),
            int(max_val),
            label.encode("utf-8"),
            int(step),
            bool(ticks),
            int(emphasis),
        )
    )
    return changed, int(c_val.value)


def slider_float(
    label: str,
    value: float,
    min_val: float,
    max_val: float,
    step: float = 0.0,
    ticks: bool = False,
    emphasis: Emphasis = Emphasis.NONE,
) -> Tuple[bool, float]:
    """Renders a floating point slider. Returns (changed, new_float_value)."""
    native = get_native()
    c_val = ctypes.c_float(value)
    changed = bool(
        native.slider_float(
            ctypes.byref(c_val),
            float(min_val),
            float(max_val),
            label.encode("utf-8"),
            float(step),
            bool(ticks),
            int(emphasis),
        )
    )
    return changed, float(c_val.value)


def slider(
    label: str,
    value: Union[int, float],
    min_val: Union[int, float],
    max_val: Union[int, float],
    step: Union[int, float] = 0,
    ticks: bool = False,
    emphasis: Emphasis = Emphasis.NONE,
) -> Tuple[bool, Union[int, float]]:
    """Generic slider dispatcher for either integer or float values."""
    if isinstance(value, int) and not isinstance(value, bool):
        return slider_int(
            label, value, int(min_val), int(max_val), int(step), ticks, emphasis
        )
    return slider_float(
        label,
        float(value),
        float(min_val),
        float(max_val),
        float(step),
        ticks,
        emphasis,
    )


def tt(
    text_or_render: Union[str, Callable[[], None]],
    delay: ShowDelay = ShowDelay.NORMAL,
) -> None:
    """Renders a tooltip on hover. Accepts either plain text or a rendering callable."""
    native = get_native()
    if isinstance(text_or_render, str):
        native.tt(text_or_render.encode("utf-8"), int(delay))
    else:
        cb = RenderCallback(text_or_render)
        native.rich_tt(cb, int(delay))


def combo(
    label: str,
    items: Sequence[str],
    selected: int,
    width: float = 0.0,
) -> Tuple[bool, int]:
    """Renders a dropdown combo box. Returns (changed, selected_index)."""
    native = get_native()
    c_selected = ctypes.c_uint32(selected)
    encoded = [item.encode("utf-8") for item in items]
    c_arr = (ctypes.c_char_p * len(encoded))(*encoded)
    changed = bool(
        native.combo(
            label.encode("utf-8"),
            c_arr,
            len(encoded),
            ctypes.byref(c_selected),
            float(width),
        )
    )
    return changed, int(c_selected.value)


def list_box(
    label: str,
    items: Sequence[str],
    selected: int,
    width: float = 0.0,
) -> Tuple[bool, int]:
    """Renders a selectable list box. Returns (changed, selected_index)."""
    native = get_native()
    c_selected = ctypes.c_uint32(selected)
    encoded = [item.encode("utf-8") for item in items]
    c_arr = (ctypes.c_char_p * len(encoded))(*encoded)
    changed = bool(
        native.list(
            label.encode("utf-8"),
            c_arr,
            len(encoded),
            ctypes.byref(c_selected),
            float(width),
        )
    )
    return changed, int(c_selected.value)


def spinner(spinner_type: SpinnerType = SpinnerType.HBO_DOTS) -> None:
    """Renders an animated loading spinner."""
    native = get_native()
    native.spinner(int(spinner_type))


def menu_bar(render: Callable[[], None]) -> None:
    """Renders the top application menu bar."""
    native = get_native()
    cb = RenderCallback(render)
    native.menu_bar(cb)


def menu(label: str, render: Callable[[], None]) -> None:
    """Renders a dropdown menu inside a menu bar."""
    native = get_native()
    cb = RenderCallback(render)
    native.menu(label.encode("utf-8"), cb)


def menu_item(
    label: str,
    reserve_icon_space: bool = False,
    icon: str = "",
) -> bool:
    """Renders an item inside a dropdown menu. Returns True if clicked."""
    native = get_native()
    return bool(
        native.menu_item(
            label.encode("utf-8"),
            bool(reserve_icon_space),
            icon.encode("utf-8"),
        )
    )


def big_table(
    id: str,
    columns: Sequence[str],
    row_count: int,
    cell_render: Callable[[int, int], None],
    outer_width: float = 0.0,
    outer_height: float = 0.0,
    alternate_row_bg: bool = False,
) -> None:
    """Renders a high-performance virtualized table."""
    native = get_native()
    encoded = [col.encode("utf-8") for col in columns]
    c_arr = (ctypes.c_char_p * len(encoded))(*encoded)
    cb = RenderTableCellCallback(cell_render)
    native.big_table(
        id.encode("utf-8"),
        c_arr,
        len(encoded),
        int(row_count),
        float(outer_width),
        float(outer_height),
        bool(alternate_row_bg),
        cb,
    )


def table(
    id: str,
    columns: Sequence[str],
    render: Callable[[TableActions], None],
    outer_width: float = 0.0,
    outer_height: float = 0.0,
    alternate_row_bg: bool = False,
) -> None:
    """Renders an immediate-mode table yielding TableActions."""
    native = get_native()
    encoded = [col.encode("utf-8") for col in columns]
    c_arr = (ctypes.c_char_p * len(encoded))(*encoded)

    def on_render(ptr):
        render(TableActions(ptr))

    cb = RenderPtrCallback(on_render)
    native.table(
        id.encode("utf-8"),
        c_arr,
        len(encoded),
        float(outer_width),
        float(outer_height),
        bool(alternate_row_bg),
        cb,
    )


def tab_bar(id: str, render: Callable[[TabBarActions], None]) -> None:
    """Renders a tab bar container yielding TabBarActions."""
    native = get_native()

    def on_render(ptr):
        render(TabBarActions(ptr))

    cb = RenderPtrCallback(on_render)
    native.tab_bar(id.encode("utf-8"), cb)


def tree_node(
    label: str,
    render: Optional[Callable[[bool], None]] = None,
    open_by_default: bool = True,
    is_leaf: bool = False,
    span_all_cols: bool = False,
) -> None:
    """Renders a tree node. If render callback is provided, calls it with is_open."""
    native = get_native()
    if render is None:
        cb = RenderTreeNodeCallback(lambda is_open: None)
        native.tree_node(
            label.encode("utf-8"),
            bool(open_by_default),
            bool(is_leaf),
            bool(span_all_cols),
            cb,
        )
    else:
        cb = RenderTreeNodeCallback(render)
        native.tree_node(
            label.encode("utf-8"),
            bool(open_by_default),
            bool(is_leaf),
            bool(span_all_cols),
            cb,
        )


def status_bar(render: Callable[[], None]) -> None:
    """Renders the application status bar at the bottom."""
    native = get_native()
    cb = RenderCallback(render)
    native.status_bar(cb)


def is_hovered() -> bool:
    """Returns True if the current widget is hovered."""
    native = get_native()
    return bool(native.is_hovered())


def is_leftclicked() -> bool:
    """Returns True if the current widget was left-clicked."""
    native = get_native()
    return bool(native.is_leftclicked())


def is_rightclicked() -> bool:
    """Returns True if the current widget was right-clicked."""
    native = get_native()
    return bool(native.is_rightclicked())


def get_fps() -> float:
    """Returns the current rendering frames per second (FPS)."""
    native = get_native()
    return float(native.get_fps())


def get_version() -> str:
    """Returns the underlying Dear ImGui version."""
    global _cached_version
    if _cached_version is not None:
        return _cached_version

    native = get_native()
    buf = ctypes.create_string_buffer(64)
    length = native.get_version(buf, 64)
    if length > 64:
        buf = ctypes.create_string_buffer(length)
        native.get_version(buf, length)

    _cached_version = buf.value.decode("utf-8", errors="replace")
    return _cached_version


def get_clipboard_text() -> str:
    """Retrieves plain text from the system clipboard."""
    native = get_native()
    buf = ctypes.create_string_buffer(1024)
    length = native.clipboard_get_text(buf, 1024)
    if length > 1024:
        buf = ctypes.create_string_buffer(length)
        native.clipboard_get_text(buf, length)
    return buf.value.decode("utf-8", errors="replace")


def set_clipboard_text(text: str) -> None:
    """Stores plain text in the system clipboard."""
    native = get_native()
    native.clipboard_set_text(text.encode("utf-8"))


# Aliases matching .NET naming conventions
Run = run
IdFrame = id_frame
SL = sl
Lbl = lbl
Selectable = selectable
Checkbox = checkbox
SmallCheckbox = small_checkbox
Button = button
Sep = sep
Accordion = accordion
Hyperlink = hyperlink
Toast = toast
Input = input
InputMultiline = input_multiline
Slider = slider
TT = tt
Combo = combo
List = list_box
MenuBar = menu_bar
Menu = menu
MenuItem = menu_item
BigTable = big_table
Table = table
TabBar = tab_bar
TreeNode = tree_node
StatusBar = status_bar
IsHovered = is_hovered
IsLeftClicked = is_leftclicked
IsRightClicked = is_rightclicked
GetFps = get_fps
GetVersion = get_version
GetClipboardText = get_clipboard_text
SetClipboardText = set_clipboard_text


def __getattr__(name: str):
    """Provides property-like access for version and fps metrics."""
    if name in ("version", "GreyVersion"):
        return get_version()
    elif name in ("fps", "Fps"):
        return get_fps()
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

"""Context helper classes and actions for containers like tables and tab bars."""

from typing import Callable
from contextlib import contextmanager
from ._native import get_native, RenderCallback


class TableActions:
    """Action helper passed to table rendering callbacks."""

    def __init__(self, table_ptr: int):
        self._table_ptr = table_ptr
        self._native = get_native()

    def begin_row(self) -> bool:
        """Starts a new row in the table."""
        return bool(self._native.table_begin_row(self._table_ptr))

    def next_column(self) -> bool:
        """Advances to the next column in the current row."""
        return bool(self._native.table_next_column(self._table_ptr))

    @contextmanager
    def row(self):
        """Context manager for a table row."""
        if self.begin_row():
            try:
                yield self
            finally:
                pass

    @contextmanager
    def column(self):
        """Context manager for a table column."""
        if self.next_column():
            try:
                yield
            finally:
                pass

    # PascalCase aliases matching .NET API
    BeginRow = begin_row
    NextColumn = next_column


class TabBarActions:
    """Action helper passed to tab bar rendering callbacks."""

    def __init__(self, tabbar_ptr: int):
        self._tabbar_ptr = tabbar_ptr
        self._native = get_native()

    def tab_item(
        self,
        title: str,
        render: Callable[[], None],
        is_unsaved: bool = False,
        is_selected: bool = False,
    ) -> None:
        """Renders an individual tab within the tab bar."""
        c_title = title.encode("utf-8")
        cb = RenderCallback(render)
        self._native.tab(self._tabbar_ptr, c_title, is_unsaved, is_selected, cb)

    # PascalCase alias matching .NET API
    TabItem = tab_item

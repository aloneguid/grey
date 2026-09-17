"""Python wrapper for the Grey CodeEditor widget."""

from typing import Optional
from .enums import ProgrammingLanguage
from ._native import get_native


class CodeEditor:
    """Immediate mode code editor widget with syntax highlighting."""

    def __init__(self, language: ProgrammingLanguage = ProgrammingLanguage.NONE, text: Optional[str] = None):
        self._id: int = -1
        self.language: ProgrammingLanguage = language
        self._set_text: Optional[str] = text
        self._disposed: bool = False

    @property
    def text(self) -> Optional[str]:
        """Returns the pending text to be set in the editor."""
        return self._set_text

    @text.setter
    def text(self, value: str) -> None:
        """Sets the text of the code editor on the next render pass."""
        self._set_text = value

    def render(self) -> None:
        """Renders the code editor widget in the current frame."""
        if self._disposed:
            raise RuntimeError("Cannot render a disposed CodeEditor.")
        native = get_native()
        c_text = self._set_text.encode("utf-8") if self._set_text is not None else None
        self._id = native.code_editor(self._id, False, int(self.language), c_text)
        self._set_text = None

    def dispose(self) -> None:
        """Frees the native resources associated with this code editor."""
        if not self._disposed:
            if self._id != -1:
                native = get_native()
                native.code_editor(self._id, True, 0, None)
                self._id = -1
            self._disposed = True

    def __enter__(self) -> "CodeEditor":
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        self.dispose()

    def __del__(self) -> None:
        try:
            self.dispose()
        except Exception:
            pass

    # PascalCase aliases matching .NET API
    Render = render
    Dispose = dispose

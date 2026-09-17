"""Enumerations matching Grey framework data models."""

from enum import IntEnum


class Emphasis(IntEnum):
    """Visual emphasis styles for widgets and text."""
    NONE = 0
    PRIMARY = 1
    SECONDARY = 2
    SUCCESS = 3
    ERROR = 4
    WARNING = 5
    INFO = 6
    DISABLED = 7

    # Aliases
    None_ = 0
    Primary = 1
    Secondary = 2
    Success = 3
    Error = 4
    Warning = 5
    Info = 6
    Disabled = 7


class SubEmphasis(IntEnum):
    """Sub-emphasis state modifier."""
    NORMAL = 0
    NORMAL_TEXT = 1
    HOVERED = 2
    ACTIVE = 3

    # Aliases
    Normal = 0
    NormalText = 1
    Hovered = 2
    Active = 3


class ShowDelay(IntEnum):
    """Delay before displaying a tooltip."""
    IMMEDIATE = 0
    QUICK = 1
    NORMAL = 2
    SLOW = 3

    # Aliases
    Immediate = 0
    Quick = 1
    Normal = 2
    Slow = 3


class ProgrammingLanguage(IntEnum):
    """Supported syntax highlighting programming languages for the code editor."""
    NONE = 0
    CPP = 1
    C = 2
    CS = 3
    PYTHON = 4
    LUA = 5
    JSON = 6
    SQL = 7
    MARKDOWN = 8

    # Aliases
    None_ = 0
    Cpp = 1
    Cs = 3
    Python = 4
    Lua = 5
    Json = 6
    Sql = 7
    Markdown = 8


class SpinnerType(IntEnum):
    """Visual animation spinner types."""
    HBO_DOTS = 0
    ROTATED_HEART = 1

    # Aliases
    HboDots = 0
    RotatedHeart = 1

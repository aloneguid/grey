"""Data models and ctypes structures for the Grey framework."""

import ctypes
from dataclasses import dataclass
from typing import Optional
from .enums import Emphasis


class CStyle(ctypes.Structure):
    """C ABI compatible struct for widget styling."""
    _fields_ = [
        ("emp", ctypes.c_int32),
    ]


@dataclass
class Style:
    """High-level styling configuration."""
    emp: Emphasis = Emphasis.NONE

    def to_cstyle(self) -> CStyle:
        """Converts to a CStyle ctypes structure."""
        return CStyle(emp=int(self.emp))

    @classmethod
    def from_cstyle(cls, cs: CStyle) -> "Style":
        """Constructs a Style instance from a CStyle ctypes structure."""
        return cls(emp=Emphasis(cs.emp))


@dataclass
class DebugInfo:
    """Debug metrics information."""
    fps: float = 0.0

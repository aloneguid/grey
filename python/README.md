# Grey Python Bindings

First-class Python bindings for the [Grey](https://github.com/aloneguid/grey) framework — a blazing fast, small, disposable UI for developers that makes their grey matter to be spent on actual problems.

## Highlights

- **Pure Python with `ctypes`**: Zero third-party runtime dependencies. Uses Python standard library only.
- **Cross-Platform**: Seamlessly runs on Windows, Linux, and macOS (x64 and ARM64).
- **Hybrid Dynamic Library Discovery**: Transparently auto-discovers `xgrey.dll` / `libxgrey.so` / `libxgrey.dylib` from bundled packages, local build directories (`out/`, `xbin/`), custom `GREY_LIBRARY_PATH`, or system paths.
- **Pythonic & Ergonomic**: Supports both Python callbacks and context managers (`TableActions`, `TabBarActions`, `CodeEditor`).
- **Comprehensive Material Icons**: Access thousands of Material Icons glyphs via `grey.Icon`.
- **Integrated Code Editor**: Syntax-highlighted code editor for C++, C, C#, Python, JSON, Markdown, and more.

## Installation

```bash
pip install grey
```

Or for development / building from source:

```bash
git clone https://github.com/aloneguid/grey.git
cd grey/python
pip install -e .
```

Ensure `xgrey` dynamic library is built or accessible. If running in a custom directory, you can set the `GREY_LIBRARY_PATH` environment variable:

```bash
# Windows
$env:GREY_LIBRARY_PATH="path/to/xgrey.dll"

# Linux / macOS
export GREY_LIBRARY_PATH="/path/to/libxgrey.so"
```

## Quick Start

```python
import grey
from datetime import datetime

name = "World"
counter = 0

def render_frame() -> bool:
    global name, counter

    grey.lbl(f"Hello, {name}!", grey.Style(grey.Emphasis.SUCCESS))

    changed, name = grey.input("Your name", name)
    if grey.button(f"Click me! ({counter})", grey.Emphasis.PRIMARY):
        counter += 1

    grey.sep()

    if grey.button("Show Current Time"):
        grey.toast(grey.Emphasis.INFO, f"Current time: {datetime.now().strftime('%H:%M:%S')}")

    return True  # Return False to close the window

if __name__ == "__main__":
    grey.run("My Grey App", render_frame, width=500, height=350, center_on_screen=True)
```

## Interactive Demo

To launch the full interactive desktop demo displaying all widgets, tables, code editor, and styling:

```bash
python -m examples.demo
```
or run `python python/examples/demo.py`.

## License

MIT License. See [LICENSE](../LICENSE) for details.

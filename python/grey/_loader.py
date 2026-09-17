"""Cross-platform dynamic library discovery and loader for xgrey."""

import os
import platform
import sys
import ctypes
from typing import Optional, List
from pathlib import Path


def get_platform_rid() -> str:
    """Returns the .NET-style Runtime Identifier (RID) for current OS and architecture."""
    system = platform.system().lower()
    machine = platform.machine().lower()

    # Normalize architecture
    if machine in ("x86_64", "amd64"):
        arch = "x64"
    elif machine in ("arm64", "aarch64"):
        arch = "arm64"
    else:
        arch = machine

    if system == "windows":
        return f"win-{arch}"
    elif system == "linux":
        return f"linux-{arch}"
    elif system == "darwin":
        return f"osx-{arch}"
    else:
        return f"{system}-{arch}"


def get_library_filename() -> str:
    """Returns the platform-specific library name for xgrey."""
    system = platform.system().lower()
    if system == "windows":
        return "xgrey.dll"
    elif system == "darwin":
        return "libxgrey.dylib"
    else:
        return "libxgrey.so"


def _candidate_search_paths() -> List[Path]:
    """Generates prioritized candidate paths to search for xgrey library."""
    paths: List[Path] = []
    lib_name = get_library_filename()
    rid = get_platform_rid()

    # 1. Environment variable override
    env_path = os.environ.get("GREY_LIBRARY_PATH")
    if env_path:
        p = Path(env_path)
        if p.is_dir():
            paths.append(p / lib_name)
            paths.append(p / "runtimes" / rid / "native" / lib_name)
        else:
            paths.append(p)

    # 2. Package-relative bundled paths
    package_dir = Path(__file__).resolve().parent
    paths.append(package_dir / lib_name)
    paths.append(package_dir / "runtimes" / rid / "native" / lib_name)

    # 3. Development tree paths (look upwards for repo root)
    curr = package_dir
    repo_root: Optional[Path] = None
    for _ in range(5):
        if (curr / "grey" / "x.h").exists() or (curr / "CMakeLists.txt").exists():
            repo_root = curr
            break
        curr = curr.parent

    if repo_root:
        # Check out/build locations
        out_build = repo_root / "out" / "build"
        if out_build.exists():
            # Check debug and release directories
            paths.append(out_build / "x64-debug" / "grey" / lib_name)
            paths.append(out_build / "x64-release" / "grey" / lib_name)
            paths.append(out_build / rid / "grey" / lib_name)
            # Search immediate subdirectories of out/build
            for sub in out_build.iterdir():
                if sub.is_dir():
                    paths.append(sub / "grey" / lib_name)

        # Check xbin directory
        xbin = repo_root / "xbin"
        if xbin.exists():
            paths.append(xbin / "runtimes" / rid / "native" / lib_name)
            paths.append(xbin / lib_name)

        # Check build directory
        build_dir = repo_root / "build"
        if build_dir.exists():
            paths.append(build_dir / "grey" / lib_name)
            paths.append(build_dir / lib_name)
            paths.append(build_dir / "Release" / "grey" / lib_name)
            paths.append(build_dir / "Debug" / "grey" / lib_name)

    return paths


def find_library_path() -> Optional[str]:
    """Resolves and returns the full path to xgrey shared library, or None if not found."""
    candidates = _candidate_search_paths()
    for candidate in candidates:
        try:
            if candidate.is_file():
                return str(candidate.resolve())
        except OSError:
            continue
    return None


_loaded_cdll: Optional[ctypes.CDLL] = None


def load_library(custom_path: Optional[str] = None) -> ctypes.CDLL:
    """Loads and returns the xgrey dynamic library via ctypes.CDLL.
    
    Raises:
        FileNotFoundError: If the library cannot be found in search paths.
        OSError: If the library cannot be loaded by the operating system.
    """
    global _loaded_cdll
    if _loaded_cdll is not None and custom_path is None:
        return _loaded_cdll

    path_to_load: Optional[str] = custom_path
    if path_to_load is None:
        path_to_load = find_library_path()

    if path_to_load is not None and os.path.isfile(path_to_load):
        # On Windows, ensure directory containing the dll is added to dll directory if supported
        if sys.platform == "win32" and hasattr(os, "add_dll_directory"):
            try:
                os.add_dll_directory(str(Path(path_to_load).parent.resolve()))
            except OSError:
                pass
        cdll = ctypes.CDLL(path_to_load)
        if custom_path is None:
            _loaded_cdll = cdll
        return cdll

    # Fallback to system-level loader
    lib_name = get_library_filename()
    try:
        cdll = ctypes.CDLL(lib_name)
        if custom_path is None:
            _loaded_cdll = cdll
        return cdll
    except OSError as err:
        searched = "\n".join(f" - {p}" for p in _candidate_search_paths())
        raise FileNotFoundError(
            f"Could not find or load '{lib_name}'. Searched paths:\n{searched}\n"
            f"You can specify the location using the GREY_LIBRARY_PATH environment variable."
        ) from err

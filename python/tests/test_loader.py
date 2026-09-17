"""Unit tests for native library resolution and loading."""

import os
import sys
import unittest
from pathlib import Path

# Ensure package is on sys.path
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from grey._loader import (
    get_platform_rid,
    get_library_filename,
    find_library_path,
    load_library,
    _candidate_search_paths,
)


class TestLoader(unittest.TestCase):
    def test_platform_rid_format(self):
        rid = get_platform_rid()
        self.assertIsInstance(rid, str)
        self.assertTrue(
            rid.startswith("win-") or rid.startswith("linux-") or rid.startswith("osx-") or "-" in rid
        )

    def test_library_filename(self):
        filename = get_library_filename()
        self.assertIn(filename, ("xgrey.dll", "libxgrey.so", "libxgrey.dylib"))

    def test_candidate_search_paths(self):
        candidates = _candidate_search_paths()
        self.assertGreater(len(candidates), 0)
        # Verify candidate paths point to the platform-specific library name
        lib_name = get_library_filename()
        self.assertTrue(any(p.name == lib_name for p in candidates))

    def test_env_var_override(self):
        original_env = os.environ.get("GREY_LIBRARY_PATH")
        dummy_path = r"C:\fake\path\xgrey.dll" if os.name == "nt" else "/fake/path/libxgrey.so"
        try:
            os.environ["GREY_LIBRARY_PATH"] = dummy_path
            candidates = _candidate_search_paths()
            self.assertEqual(str(candidates[0]), dummy_path)
        finally:
            if original_env is not None:
                os.environ["GREY_LIBRARY_PATH"] = original_env
            else:
                os.environ.pop("GREY_LIBRARY_PATH", None)

    def test_load_existing_library(self):
        path = find_library_path()
        if path is not None:
            self.assertTrue(os.path.isfile(path))
            cdll = load_library()
            self.assertIsNotNone(cdll)


if __name__ == "__main__":
    unittest.main()

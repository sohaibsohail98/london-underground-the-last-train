#!/usr/bin/env python3
"""Run the whole assertion suite.

Deliberately stdlib `unittest` rather than pytest, so the CI job is
`python3 tools/zombie-surfaces/tests/run_assertions.py` and matches the shape
of the existing gates in `tools/ci/`, and so the only third party dependencies
are the two the tool itself needs.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

TESTS_DIR = Path(__file__).resolve().parent


def main() -> int:
	loader = unittest.TestLoader()
	suite = loader.discover(str(TESTS_DIR), pattern="test_*.py", top_level_dir=str(TESTS_DIR))
	result = unittest.TextTestRunner(verbosity=2).run(suite)
	return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
	sys.exit(main())

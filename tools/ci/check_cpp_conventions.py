#!/usr/bin/env python3
"""Static checks for the Unreal module.

The engine is not available in CI, so this cannot compile. It catches the
mistakes that do not need a compiler: naming, missing generated headers,
include order and the project's own style rules.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

SOURCE = Path("Source")

FAILURES: list[str] = []


def fail(path: Path, line: int | None, message: str) -> None:
    where = f"{path}:{line}" if line else str(path)
    FAILURES.append(f"{where}: {message}")


def check_header(path: Path, text: str, lines: list[str]) -> None:
    if not text.startswith("#pragma once"):
        fail(path, 1, "header must begin with #pragma once")

    includes = [i for i, line in enumerate(lines) if line.startswith("#include")]
    if includes:
        last = lines[includes[-1]]
        expected = f'#include "{path.stem}.generated.h"'
        if "generated.h" in text and last != expected:
            fail(path, includes[-1] + 1, f"{path.stem}.generated.h must be the final include")

    for i, line in enumerate(lines, start=1):
        for match in re.finditer(r"\b(class|struct)\s+(?:LASTTRAIN_API\s+)?([A-Za-z_]\w*)", line):
            kind, name = match.group(1), match.group(2)
            if kind == "class" and not re.match(r"^(A|U|F|I|S|T|E)[A-Z]", name):
                fail(path, i, f"class {name} needs an Unreal prefix")

        if re.search(r"\bTArray<\w+\*>", line):
            fail(path, i, "use TObjectPtr in containers, not raw UObject pointers")


def check_common(path: Path, lines: list[str]) -> None:
    for i, line in enumerate(lines, start=1):
        if "\u2014" in line or "\u2013" in line:
            fail(path, i, "em or en dash in source; use plain punctuation")
        if line.rstrip() != line:
            fail(path, i, "trailing whitespace")
        if re.search(r"\b(TODO|FIXME|HACK|XXX)\b", line):
            fail(path, i, "unfinished marker; finish it or open an issue")
        if "UE_LOG(" in line and "LogTemp" in line:
            fail(path, i, "use LT_LOG rather than LogTemp")
        if re.search(r'"[^"]*\b(organiz|color|behavior|customiz)\w*"', line, re.IGNORECASE):
            fail(path, i, "user facing string uses US spelling")


def main() -> int:
    if not SOURCE.is_dir():
        print("no Source directory; nothing to check")
        return 0

    files = sorted([*SOURCE.rglob("*.h"), *SOURCE.rglob("*.cpp")])
    if not files:
        print("no C++ files found")
        return 0

    for path in files:
        text = path.read_text(encoding="utf-8")
        lines = text.splitlines()

        check_common(path, lines)
        if path.suffix == ".h":
            check_header(path, text, lines)

    if FAILURES:
        print(f"{len(FAILURES)} convention problem(s):\n")
        for failure in FAILURES:
            print(f"  {failure}")
        return 1

    print(f"{len(files)} files pass convention checks")
    return 0


if __name__ == "__main__":
    sys.exit(main())

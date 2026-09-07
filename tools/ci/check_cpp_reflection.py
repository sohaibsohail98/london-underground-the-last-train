#!/usr/bin/env python3
"""Static stand-in for the compiler that CI cannot run.

Unreal Engine is not installable on a hosted runner, so there is no hosted
compile gate: `tools/ci/compile.sh` on a developer machine, or the optional
self-hosted `compile` job, is the only real one. This catches the mistakes a
compiler or Unreal Header Tool would have caught, cheaply and without an engine:

1. A reflected type with no GENERATED_BODY().
2. A header declaring reflected types without including its own generated.h.
3. A translation unit that does not include its own header first, which is how
   a header that only compiles because of an earlier include goes unnoticed.
4. An LT_LOG or UE_LOG whose format specifiers and arguments disagree, which
   prints rubbish at best and reads off the stack at worst.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

SOURCE = Path("Source")

FAILURES: list[str] = []

REFLECTED_MACROS = re.compile(r"^\s*(UCLASS|USTRUCT|UINTERFACE)\s*\(")
ANY_REFLECTED = re.compile(r"^\s*(UCLASS|USTRUCT|UENUM|UINTERFACE|UDELEGATE)\s*\(")

# %[flags][width][.precision][length]conversion, with %% excluded by the caller.
FORMAT_SPECIFIER = re.compile(r"%[-+ #0]*[\d*]*(?:\.[\d*]+)?(?:hh|h|ll|l|L|z|j|t|I64|I32)?([diuoxXeEfgGaAcspSn%])")

TEXT_LITERAL = re.compile(r'TEXT\(\s*"((?:[^"\\]|\\.)*)"\s*\)')


def fail(path: Path, line: int | None, message: str) -> None:
    where = f"{path}:{line}" if line else str(path)
    FAILURES.append(f"{where}: {message}")


def check_generated_body(path: Path, lines: list[str]) -> None:
    """Every UCLASS, USTRUCT and UINTERFACE needs GENERATED_BODY within a few lines."""
    for index, line in enumerate(lines):
        if not REFLECTED_MACROS.match(line):
            continue

        window = "\n".join(lines[index : index + 8])
        if "GENERATED_BODY()" not in window and "GENERATED_UCLASS_BODY()" not in window:
            fail(path, index + 1, "reflected type has no GENERATED_BODY() within 8 lines")


def check_generated_include(path: Path, text: str, lines: list[str]) -> None:
    """A header with reflected types must include its own generated.h."""
    if not any(ANY_REFLECTED.match(line) for line in lines):
        return

    expected = f'#include "{path.stem}.generated.h"'
    if expected not in text:
        fail(path, None, f"header declares reflected types but never includes {path.stem}.generated.h")


def check_own_header_first(path: Path, lines: list[str]) -> None:
    """The first include of a .cpp is its own header, so the header stands alone."""
    for index, line in enumerate(lines):
        if not line.startswith("#include"):
            continue

        first = line.strip()
        if not first.endswith(f'{path.stem}.h"'):
            fail(path, index + 1, f"first include should be this file's own header, {path.stem}.h")
        return


def split_arguments(call: str) -> list[str]:
    """Top level comma split, ignoring commas inside strings and brackets.

    Angle brackets are deliberately not tracked: `->` and comparisons are far
    more common in a log argument than a template with a comma in it, and
    treating `>` as a closer miscounts every `Something->Member`.
    """
    arguments: list[str] = []
    depth = 0
    in_string = False
    escaped = False
    current: list[str] = []

    for char in call:
        if escaped:
            current.append(char)
            escaped = False
            continue

        if char == "\\":
            current.append(char)
            escaped = True
            continue

        if char == '"':
            in_string = not in_string
            current.append(char)
            continue

        if in_string:
            current.append(char)
            continue

        if char in "([{":
            depth += 1
        elif char in ")]}":
            depth = max(0, depth - 1)

        if char == "," and depth == 0:
            arguments.append("".join(current).strip())
            current = []
            continue

        current.append(char)

    if current:
        arguments.append("".join(current).strip())

    return arguments


def find_calls(text: str, macro: str) -> list[tuple[int, str]]:
    """Every macro call in the file, as (line number, argument text)."""
    calls: list[tuple[int, str]] = []
    pattern = re.compile(rf"\b{macro}\s*\(")

    for match in pattern.finditer(text):
        start = match.end()
        depth = 1
        index = start
        in_string = False
        escaped = False

        while index < len(text) and depth > 0:
            char = text[index]

            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = not in_string
            elif not in_string:
                if char == "(":
                    depth += 1
                elif char == ")":
                    depth -= 1

            index += 1

        if depth == 0:
            calls.append((text.count("\n", 0, match.start()) + 1, text[start : index - 1]))

    return calls


def check_log_arity(path: Path, text: str) -> None:
    """Format specifiers must match the arguments that follow the format string."""
    for macro, format_position in (("LT_LOG", 1), ("UE_LOG", 2)):
        for line_number, call in find_calls(text, macro):
            arguments = split_arguments(call)
            if len(arguments) <= format_position:
                continue

            format_argument = arguments[format_position]
            pieces = TEXT_LITERAL.findall(format_argument)
            if not pieces:
                # Not a literal format string, so nothing can be counted.
                continue

            literal = "".join(pieces)
            specifiers = [m.group(1) for m in FORMAT_SPECIFIER.finditer(literal)]
            expected = len([s for s in specifiers if s != "%"])
            supplied = len(arguments) - format_position - 1

            if expected != supplied:
                fail(
                    path,
                    line_number,
                    f"{macro} has {expected} format specifier(s) but {supplied} argument(s)",
                )


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

        check_log_arity(path, text)

        if path.suffix == ".h":
            check_generated_body(path, lines)
            check_generated_include(path, text, lines)
        else:
            check_own_header_first(path, lines)

    if FAILURES:
        print(f"{len(FAILURES)} reflection or format problem(s):\n")
        for failure in FAILURES:
            print(f"  {failure}")
        return 1

    print(f"{len(files)} files pass reflection and log format checks")
    return 0


if __name__ == "__main__":
    sys.exit(main())

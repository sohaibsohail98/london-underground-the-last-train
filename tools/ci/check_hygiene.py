#!/usr/bin/env python3
"""Repository hygiene: secrets, absolute paths and trademark leakage.

The trademark rules in docs/art-direction.md are a hard constraint, so they are
enforced here rather than left to review.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

SKIP_DIRS = {
    ".git", "node_modules", "dist", "Binaries", "Intermediate",
    "Saved", "DerivedDataCache", ".github",
}

TEXT_SUFFIXES = {
    ".ts", ".js", ".json", ".html", ".css", ".md", ".cpp", ".h",
    ".cs", ".ini", ".yml", ".yaml", ".py", ".uproject",
}

SECRETS = [
    (re.compile(r"github_pat_[A-Za-z0-9_]{20,}"), "GitHub fine grained token"),
    (re.compile(r"\bghp_[A-Za-z0-9]{30,}"), "GitHub classic token"),
    (re.compile(r"\bAKIA[0-9A-Z]{16}\b"), "AWS access key id"),
    (re.compile(r"-----BEGIN [A-Z ]*PRIVATE KEY-----"), "private key"),
    (re.compile(r"\bBearer\s+[A-Za-z0-9\-._~+/]{24,}"), "bearer token"),
    (re.compile(r"(?i)\b(api[_-]?key|secret|password)\s*[:=]\s*['\"][^'\"]{8,}"), "hardcoded credential"),
]

PATHS = [
    (re.compile(r"[A-Za-z]:\\\\Users\\\\"), "absolute Windows user path"),
    (re.compile(r"/(?:home|Users)/[a-z0-9_.-]+/"), "absolute home path"),
]

# Trademarks and third party names that must never appear in shipped content.
# Prose is exempt: the constraint has to be discussable to be enforceable, so
# markdown, the checker itself and lines that state a rule negatively pass.
TRADEMARKS = [
    (re.compile(r"(?i)\bjohnston\b"), "Johnston typeface"),
    (re.compile(r"(?i)\broundel\b"), "TfL roundel"),
    (re.compile(r"(?i)\btransport for london\b"), "TfL name"),
    (re.compile(r"(?i)\b(treyarch|activision)\b"), "third party publisher"),
    (re.compile(r"(?i)\bcall of duty\b"), "third party title"),
]


# A line that forbids a mark is not a use of it.
NEGATION = re.compile(r"(?i)\b(no|not|never|avoid|without|forbid\w*|instead of)\b")


def iter_files() -> list[Path]:
    files: list[Path] = []
    for path in Path(".").rglob("*"):
        if not path.is_file() or path.suffix not in TEXT_SUFFIXES:
            continue
        if any(part in SKIP_DIRS for part in path.parts):
            continue
        files.append(path)
    return sorted(files)


def main() -> int:
    problems: list[str] = []
    files = iter_files()

    for path in files:
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except UnicodeDecodeError:
            continue

        exempt = (
            path.suffix == ".md"
            or path.parts[:2] == ("tools", "ci")
            or path.parts[0] == "docs"
        )

        for number, line in enumerate(lines, start=1):
            for pattern, label in SECRETS:
                if pattern.search(line):
                    problems.append(f"{path}:{number}: {label}")

            for pattern, label in PATHS:
                if pattern.search(line):
                    problems.append(f"{path}:{number}: {label}")

            if exempt or NEGATION.search(line):
                continue

            for pattern, label in TRADEMARKS:
                if pattern.search(line):
                    problems.append(f"{path}:{number}: {label} referenced outside prose")

    if problems:
        print(f"{len(problems)} hygiene problem(s):\n")
        for problem in problems:
            print(f"  {problem}")
        return 1

    print(f"{len(files)} files pass hygiene checks")
    return 0


if __name__ == "__main__":
    sys.exit(main())

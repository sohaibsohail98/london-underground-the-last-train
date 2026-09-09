#!/usr/bin/env python3
"""Documentation checks.

The design docs carry the same house rules as the source: British spelling,
no em or en dashes. They also carry structured data (open-questions.json) and
cross references between files that go stale silently. This catches all of
that without a human reading every diff.

Scope: tracked .md and .json files under docs/, plus the top level CLAUDE.md
and README.md. Any vendored tool skill is out of scope. Only tracked files are
checked, so this matches what CI sees.
"""

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

# Only our own documentation. Not any vendored skill or agent directory.
INCLUDE_PREFIXES = ("docs/",)
INCLUDE_EXACT = {"CLAUDE.md", "README.md", "Content/README.md", "Content/ATTRIBUTION.md"}

US_SPELLING = re.compile(
    r"(?<![A-Za-z])("
    r"organiz\w*|"
    r"customiz\w*|"
    r"colou?r(?:s|ed|ing)?|"  # matched then filtered: British "colour" is allowed
    r"behavior(?:s)?|"
    r"favor(?:s|ed|ing|ite|ites)?|"
    r"defense|offense|"
    r"traveled|traveling|"
    r"catalog|dialog(?!ue)"
    r")(?![A-Za-z])",
    re.IGNORECASE,
)
# The pattern above intentionally matches British "colour" so we can allow it
# explicitly here and only reject the US "color".
ALLOWED_TOKENS = re.compile(r"^colou?r", re.IGNORECASE)
US_COLOR = re.compile(r"(?<![A-Za-z])colors?(?![A-Za-z])|(?<![A-Za-z])colored(?![A-Za-z])|(?<![A-Za-z])coloring(?![A-Za-z])", re.IGNORECASE)

NEGATION = re.compile(
    r"(?i)\b(no|not|never|avoid|reject(?:s|ed)?|forbid(?:s|den)?|"
    r"instead of|rather than|british spelling|us spelling|american spelling|"
    r"the checker rejects)\b"
)

CSS_COLOR = re.compile(r"(color\s*:|--[\w-]*colou?r|color-scheme|background-color|[A-Za-z-]+-color\b|\bcolou?r\b)")

FAILURES: list[str] = []


def fail(path: str, line: int | None, message: str) -> None:
    where = f"{path}:{line}" if line else path
    FAILURES.append(f"{where}: {message}")


def tracked_files(suffix: str) -> list[str]:
    out = subprocess.run(
        ["git", "ls-files", f"*{suffix}"],
        capture_output=True, text=True, check=True,
    ).stdout.splitlines()
    kept = []
    for rel in out:
        if rel.startswith(INCLUDE_PREFIXES) or rel in INCLUDE_EXACT:
            kept.append(rel)
    return sorted(kept)


def check_markdown(rel: str) -> None:
    text = Path(rel).read_text(encoding="utf-8")
    lines = text.splitlines()

    in_fence = False
    for i, line in enumerate(lines, start=1):
        if line.strip().startswith("```"):
            in_fence = not in_fence
            continue

        if "—" in line or "–" in line:
            fail(rel, i, "em or en dash in a doc; use plain punctuation")

        if line != line.rstrip():
            fail(rel, i, "trailing whitespace")

        if in_fence or NEGATION.search(line):
            continue

        for match in US_SPELLING.finditer(line):
            token = match.group(0)
            if ALLOWED_TOKENS.match(token):
                # "colour" family is British and allowed; only reject the US form.
                if US_COLOR.search(line) and not CSS_COLOR.search(line):
                    fail(rel, i, "US spelling 'color' family; use 'colour'")
                continue
            fail(rel, i, f"US spelling {token!r}; use British spelling")


def check_json(rel: str):
    try:
        return json.loads(Path(rel).read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        fail(rel, exc.lineno, f"invalid JSON: {exc.msg}")
        return None


def check_open_questions(rel: str) -> None:
    data = check_json(rel)
    if data is None:
        return
    if not isinstance(data, dict):
        fail(rel, None, "must be a JSON object")
        return

    total = data.get("totalItems")
    sections = data.get("sections")
    if not isinstance(sections, list) or not sections:
        fail(rel, None, "needs a non-empty sections array")
        return

    seen: set[str] = set()
    counted = 0
    for section in sections:
        if not isinstance(section, dict):
            fail(rel, None, "each section must be an object")
            continue
        items = section.get("items")
        if not isinstance(items, list):
            fail(rel, None, f"section {section.get('id')!r} has no items array")
            continue
        for item in items:
            counted += 1
            if not isinstance(item, dict):
                fail(rel, None, "each item must be an object")
                continue
            item_id = item.get("id")
            if not item_id:
                fail(rel, None, "an item is missing its id")
                continue
            if item_id in seen:
                fail(rel, None, f"duplicate item id {item_id!r}")
            seen.add(item_id)
            for key in ("title", "isProposal"):
                if key not in item:
                    fail(rel, None, f"item {item_id!r} is missing {key!r}")

    if isinstance(total, int) and total != counted:
        fail(rel, None, f"totalItems says {total} but {counted} items are present")


def check_cross_links(md_files: list[str]) -> None:
    link = re.compile(r"\[[^\]]+\]\(([^)]+)\)")
    for rel in md_files:
        base = Path(rel).parent
        for i, line in enumerate(Path(rel).read_text(encoding="utf-8").splitlines(), start=1):
            for match in link.finditer(line):
                target = match.group(1).strip()
                if target.startswith(("http://", "https://", "#", "mailto:")):
                    continue
                target = target.split("#", 1)[0].split(" ", 1)[0]
                if not target:
                    continue
                if (base / target).exists() or (Path(".") / target).exists():
                    continue
                fail(rel, i, f"link target does not exist: {target}")


def main() -> int:
    md_files = tracked_files(".md")
    json_files = tracked_files(".json")

    for rel in md_files:
        check_markdown(rel)
    for rel in json_files:
        if Path(rel).name == "open-questions.json":
            check_open_questions(rel)
        else:
            check_json(rel)
    check_cross_links(md_files)

    if FAILURES:
        print(f"{len(FAILURES)} documentation problem(s):\n")
        for failure in FAILURES:
            print(f"  {failure}")
        return 1

    print(f"{len(md_files)} markdown and {len(json_files)} json file(s) pass documentation checks")
    return 0


if __name__ == "__main__":
    sys.exit(main())

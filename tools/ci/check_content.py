#!/usr/bin/env python3
"""Content asset integrity.

Unreal .uasset and .umap files are binary and must go through Git LFS. If one
is committed as raw binary (a smudge failure or a tooling mistake), the repo
still clones but diffs become unreadable and the file bloats history.

This inspects the COMMITTED blob for each tracked asset (git cat-file against
HEAD), not the working tree. Locally the working tree holds the smudged real
file; in CI without an LFS pull it holds a pointer. The committed blob is a
pointer in both cases when LFS is set up correctly, so checking it is
environment independent.

Also verifies .gitattributes still routes the binary suffixes through LFS.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

LFS_SUFFIXES = {".uasset", ".umap", ".fbx", ".tga", ".wav", ".ttf", ".otf"}

POINTER_PREFIX = "version https://git-lfs.github.com/spec/v1"
# A committed LFS pointer is always small. A raw asset blob is not.
POINTER_MAX_BYTES = 1024

FAILURES: list[str] = []


def fail(path: str, message: str) -> None:
    FAILURES.append(f"{path}: {message}")


def run(args: list[str]) -> str:
    return subprocess.run(args, capture_output=True, text=True, check=True).stdout


def have_head() -> bool:
    return subprocess.run(
        ["git", "rev-parse", "--verify", "HEAD"], capture_output=True
    ).returncode == 0


def tracked() -> list[str]:
    return run(["git", "ls-files"]).splitlines()


def committed_blob(rel: str) -> str | None:
    try:
        return run(["git", "cat-file", "-p", f"HEAD:{rel}"])
    except subprocess.CalledProcessError:
        return None


def check_gitattributes() -> None:
    ga = Path(".gitattributes")
    if not ga.is_file():
        fail(".gitattributes", "missing; LFS routing is not configured")
        return
    text = ga.read_text(encoding="utf-8")
    for suffix in (".uasset", ".umap"):
        pattern = f"*{suffix}"
        line = next((l for l in text.splitlines() if l.strip().startswith(pattern)), None)
        if line is None:
            fail(".gitattributes", f"no rule for {pattern}")
        elif "filter=lfs" not in line:
            fail(".gitattributes", f"{pattern} is not routed through filter=lfs")


def check_assets(files: list[str]) -> int:
    if not have_head():
        print("no commits yet; skipping committed-blob check")
        return 0
    count = 0
    for rel in files:
        if Path(rel).suffix.lower() not in LFS_SUFFIXES:
            continue
        count += 1
        blob = committed_blob(rel)
        if blob is None:
            fail(rel, "tracked but not present in HEAD")
            continue
        if blob.startswith(POINTER_PREFIX):
            continue
        if len(blob.encode("utf-8", "surrogateescape")) > POINTER_MAX_BYTES:
            fail(
                rel,
                "committed as raw binary, not a Git LFS pointer; "
                "run: git rm --cached <file> && git add <file> && git commit",
            )
        else:
            fail(rel, "committed blob is neither an LFS pointer nor a recognised asset")
    return count


def main() -> int:
    check_gitattributes()
    count = check_assets(tracked())

    if FAILURES:
        print(f"{len(FAILURES)} content problem(s):\n")
        for failure in FAILURES:
            print(f"  {failure}")
        return 1

    print(f"{count} tracked content asset(s) pass integrity checks")
    return 0


if __name__ == "__main__":
    sys.exit(main())

# LAST TRAIN  -  repo setup and git workflow

This file was written for the browser build. The token safety rules, the
credential helper setup and the branch strategy still apply to the Unreal
Engine 5 project. The old `.gitignore` and `.gitattributes` templates, the
Fable prompt addenda and the Project Knowledge upload instructions have been
removed because they described the superseded Vite build. The UE5 project uses
the standard Unreal `.gitignore` and Git LFS for binary content instead.

## 1. Token scoping

Use a **fine-grained** PAT, not a classic one.

GitHub → Settings → Developer settings → Personal access tokens → Fine-grained tokens.

- **Resource owner:** your account
- **Repository access:** Only select repositories → the one repo, nothing else
- **Permissions:** Contents → Read and write. Metadata → Read-only (added automatically). Nothing else. Not Actions, not Workflows, not Packages, not Admin.
- **Expiry:** 30 days. Renew rather than extend.

### Rules

- Never paste the token into a chat with any model, including Fable, Claude Code or Claude.
- Never put it in a file, a tracked `.env`, a commit message, or a code comment.
- Never put it in the remote URL (`https://TOKEN@github.com/...`), because that writes it into `.git/config` in plaintext.
- Let the credential helper hold it. It is the only thing that should ever store it.

## 2. Local bootstrap

Run these yourself in a terminal, not through a model.

```bash
mkdir last-train && cd last-train
git init -b main

# store credentials in the OS keychain, not a plaintext file
# macOS:
git config --global credential.helper osxkeychain
# Windows:
git config --global credential.helper manager
# Linux:
git config --global credential.helper libsecret

git remote add origin https://github.com/YOUR_USERNAME/last-train.git
```

On the first `git push`, git prompts for a username and password. Enter your
GitHub username, and paste the **token** as the password. The helper stores it
and never asks again. The token never touches the repo.

Alternative, cleaner if you use the GitHub CLI:

```bash
gh auth login --with-token < /path/to/token.txt
rm /path/to/token.txt
```

Delete that file immediately afterwards.

## 3. Branch strategy

Simplest thing that still protects you:

- `main` holds working builds only.
- One branch per phase: `phase/04-combat-slice`, and so on.
- Merge to `main` only after you have approved that phase's gate.
- Tag each merge: `git tag gate-b && git push --tags`.

If a gate looks wrong and you fall back to the tagged `phase-03` browser build,
you want a clean tagged point to branch from rather than an unpickable history.

## 4. CI

`.github/workflows/ci.yml` runs on every push to `main` and the working
branch prefixes, and on every pull request into `main`. Four jobs, all fast,
none needs the engine:

- **cpp**: clang-format 20 in `--Werror` mode over `Source/`, then
  `tools/ci/check_cpp_conventions.py` (Unreal prefixes, generated-header
  order, `TObjectPtr` in containers, no `LogTemp`, no unfinished markers, no
  dashes, British spelling in strings).
- **docs**: `tools/ci/check_docs.py` over `docs/`, `CLAUDE.md`, `README.md`:
  British spelling, no em or en dashes, no trailing whitespace, every
  tracked `.json` parses, and every relative markdown link points at a file
  that exists.
- **hygiene**: `tools/ci/check_hygiene.py`: secret patterns, absolute local
  paths, trademark leakage (roundel, Johnston, operator name, third party
  titles).
- **content**: `tools/ci/check_content.py`: every tracked `.uasset` and
  `.umap` is committed as a Git LFS pointer, not raw binary, and
  `.gitattributes` still routes those suffixes through LFS. It inspects the
  committed blob, so it passes the same locally and in CI.

The engine is not available in CI, so C++ is never compiled there. Compile
locally after every source change with the batch build in `CLAUDE.md`.

Run all four before a push:

    python3 tools/ci/check_cpp_conventions.py
    python3 tools/ci/check_docs.py
    python3 tools/ci/check_hygiene.py
    python3 tools/ci/check_content.py

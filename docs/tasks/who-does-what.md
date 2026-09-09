# Who does what: the sessions and how they hand off

Written 2026-09-09. Three Claude sessions run against this repo. This file says
what each does and how work moves between them, so nobody steps on anybody.

## The sessions

| Session | Model | Editor access | Does |
|---|---|---|---|
| **Terminal** (this repo, plain `claude`) | Sonnet | no | git, pushes, CI, branch merges, writing task specs, reviewing what comes back, this file |
| **CC-in-Unreal** (terminal docked in the editor, talks over MCP port 9315) | Opus | **yes, exclusive** | all interactive editor work: placing actors, Blueprint graphs, materials, PIE, screenshots, running Python `execute_script` payloads |
| **Remote** (a remote Claude Code session on the repo) | Opus | no | `Source/` C++ edits, Python editor-script authoring, CI gate runs, asset-research subagents, specs, balance numbers, review |

Only **CC-in-Unreal** may drive the editor, and only one process can hold the
MCP connection at a time. Everything editor-shaped funnels through it.

## Hard rules (all sessions)

- **British spelling** everywhere. **No em or en dashes** in source or docs.
- Palette is fixed: `#16161C` `#6C4C9C` `#E0A030` `#B02030`.
- Legal constraints in `CLAUDE.md` are non negotiable: no roundel, no Johnston,
  no official line diagram, no operator livery / logo / announcement recordings,
  no Call of Duty names.
- **CC-in-Unreal never edits `Source/` and never runs git** (except one
  `git pull` to sync). It commits through its own trailer.
- **`MAP CHECK` as a console exec is banned** since it crashed the editor
  2026-09-08. Use `MAP CHECKDEP NOCLEARLOG`. See `editor-crash-endplaymap.md`.
- **Never mutate an actor or asset while PIE is running** (transacted edit
  during PIE pins the PIE GameInstance and crashes the editor on teardown).
- Compile C++ locally after every change: `./tools/ci/compile.sh` (needs the
  external Xcode mount). The remote session cannot compile: it writes C++, the
  human or the self-hosted runner compiles.
- `.uasset` / `.umap` are Git LFS. Verify a commit is a pointer:
  `git show <ref>:<path> | head -c 45` -> `version https://git-lfs...`.
- `_incoming_assets/`, `/Content/SubwayTrain/`, `/Content/UrbanSubway/`,
  `/Content/CitySample/` are gitignored. Never `git add` a `.uasset` from them.

## How work hands off

### Remote writes C++ that CC-in-Unreal needs

1. Remote edits `Source/`, runs the CI gates (`python3 tools/ci/check_*.py`),
   commits to a `claude/<topic>` branch, pushes.
2. Human runs `./tools/ci/compile.sh` locally, confirms it builds.
3. Terminal session merges the branch to `main`, pushes.
4. CC-in-Unreal does `git pull`, restarts the editor if the module changed, and
   the new hooks / actors are available.

### Remote writes a Python editor script

1. Remote writes the `execute_script` payload as
   `tools/editor-scripts/<name>.py` with a header comment saying what it does
   and what it expects, commits, pushes.
2. Terminal merges. CC-in-Unreal pulls and runs the script verbatim over MCP,
   reports what happened, iterates in place if it needs to.

### Asset research

1. Remote spins up a research subagent per `asset-research-phase-f.md` (hard
   constraints in that file). Output lands in `_incoming_assets/` with
   `SOURCES.txt` files. Nothing is imported.
2. CC-in-Unreal triages: imports the useful files into `Content/LastTrain/`,
   commits them via LFS.

### CC-in-Unreal finishes an editor task

1. Commits the `.uasset` / `.umap` / `.ini` changes via LFS with its trailer.
2. Updates the relevant row in `handover.md`.
3. Terminal session reviews and pushes.

## Current queue

| Order | Task | Session | State |
|---|---|---|---|
| 1 | S13 main menu | CC-in-Unreal | in flight |
| 2 | F5 fix S11 signage (readable, sodium, cookable) | CC-in-Unreal | queued |
| 3 | asset research for F1/F3/menu | Remote | queued |
| 4 | F1 modular kit | CC-in-Unreal | queued, needs nothing |
| 5 | F2 lighting + atmosphere | CC-in-Unreal (Fable guidance) | needs F1 |
| 6 | F3 train exterior | CC-in-Unreal | needs F1 |
| 7 | F4 train interior | CC-in-Unreal | needs F3 |
| 8 | F6 zombie bodies | CC-in-Unreal | do after F1 |
| 9 | F7 perf pass | CC-in-Unreal (Fable guidance) | needs F1 to F6 |
| 10 | Phase G (audio, HUD, balance) | mixed | after F |

Perks / bench / lost property (Phase E leftovers): decide at the start of Phase
G whether v1 ships them. If not, v1's only survival-extension is the existing
downed / auto-revive.

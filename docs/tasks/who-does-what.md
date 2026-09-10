# Who does what: the sessions and how they hand off

Written 2026-09-09. Three Claude sessions run against this repo. This file says
what each does and how work moves between them, so nobody steps on anybody.

## The sessions

| Session | Model | Editor access | Does |
|---|---|---|---|
| **Terminal** (this repo, plain `claude`) | Sonnet | no | git, pushes, CI, branch merges, writing task specs, reviewing what comes back, this file |
| **CC-in-Unreal** (terminal docked in the editor, talks over MCP port 8000) | Opus | **yes, exclusive** | all interactive editor work: placing actors, Blueprint graphs, materials, PIE, screenshots, running Python `execute_script` payloads |
| **Remote** (a remote Claude Code session on the repo) | Opus | no | `Source/` C++ edits, Python editor-script authoring, CI gate runs, asset-research subagents, specs, balance numbers, review |

Only **CC-in-Unreal** may drive the editor, and only one process can hold the
MCP connection at a time. Everything editor-shaped funnels through it.

## Hard rules (all sessions)

- **Read `docs/known-issues.md` first.** It carries what each lane cannot do,
  what is committed but unverified, and the open legal flags. Update it when
  an issue opens or closes.
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

**Corrected 2026-09-09 after the first run.** The remote session cannot download:
its egress proxy refuses every asset host, and `_incoming_assets/` is gitignored
and dies with the container, so nothing staged there could reach this machine.
Web search still works, so the research is real. The lane is therefore:

1. Remote spins up research subagents per `asset-research-phase-f.md` (hard
   constraints in that file) and commits a **source manifest** to `docs/`:
   sources, asset IDs, licences, what each feeds, what needs inspecting.
2. A human or CC-in-Unreal runs `tools/asset-fetch/fetch-phase-f.sh` here. It
   creates `_incoming_assets/<category>/` with a `SOURCES.txt` per category,
   fetches what has a public API, and prints a manual checklist for the rest.
   It imports nothing and runs no git.
3. CC-in-Unreal triages: imports the useful files into `Content/LastTrain/`,
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
| 3 | asset research for F1/F3/menu | Remote | **done 2026-09-09.** Result is `docs/reference/asset-sources-phase-f.md` plus `tools/asset-fetch/fetch-phase-f.sh`. Nothing downloaded: the remote container cannot reach any asset host, and `_incoming_assets/` would not survive it anyway. Run the script here. |
| 4 | F1 modular kit | CC-in-Unreal | **done for Canary Wharf 2026-09-09** (`a0fcdba`, `47f4bac`). 12 kit meshes, the shell rebuilt, navmesh and a PIE round verified. `L_GreyboxTest` still on Phase B cubes. |
| 5 | F2 lighting + atmosphere | CC-in-Unreal (Fable guidance) | **unblocked.** F1 left the hall badly blown out against the new near-white tile, so exposure and the sodium balance are the first job. `r.Shadow.Virtual.Enable` is already on. |
| 6 | F3 train exterior | CC-in-Unreal | **unblocked.** The train is currently two scaled kit panels standing in. |
| 7 | F4 train interior | CC-in-Unreal | needs F3 |
| 8 | F6 zombie bodies | CC-in-Unreal | do after F1 |
| 9 | F7 perf pass | CC-in-Unreal (Fable guidance) | needs F1 to F6 |
| 10 | Phase G (audio, HUD, balance) | mixed | after F |

Perks / bench / lost property (Phase E leftovers): decide at the start of Phase
G whether v1 ships them. If not, v1's only survival-extension is the existing
downed / auto-revive.

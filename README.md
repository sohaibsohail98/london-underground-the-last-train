# Last Train

First person round-based zombie survival on a fictionalised Crossrail-scale
London rail line, built in Unreal Engine 5.8. A station is the arena. A train
arrives every 100 seconds and dwells for 25, and boarding it is an optional
escape to an adjacent station. Staying raises the station's heat.

Not affiliated with, endorsed by, or connected to Transport for London. No TfL
trademarks are used: no roundel, no official logo, no Johnston typeface, no
reproduction of the official line diagram, no recorded announcements. Station
names and geography are factual and used as such.

## Layout

```
LastTrain.uproject      Unreal Engine 5.8 project, the active target
Source/LastTrain/       C++ module: Player Weapons Zombies Rounds Economy Interaction Core
Content/                Unreal assets, Git LFS, see Content/README.md
Config/                 Engine and project configuration
docs/                   Briefs, plan, design canon, art direction, setup
tools/ci/               Static checks run by CI
tools/asset-fetch/      Stages the Phase F asset list into _incoming_assets/
```

The Unreal target is the project. An earlier Three.js browser build was removed
from the tree on 2026-09-09; it is preserved at the `web-threejs-final` tag as a
historical reference and is not developed. Note that a fresh clone may not fetch
tags: check with `git ls-remote --tags origin`, not `git tag -l`.

## Running the Unreal target

Unreal Engine 5.8, macOS. The `LastTrain` C++ module compiles with the batch
build in `CLAUDE.md` (external Xcode on `/Volumes/DriveSohaib` must be mounted).
The editor assets for the first playable grey box are built; follow
`docs/unreal-setup.md` for how they were made and `docs/tasks/handover.md` for
where the work stands. The task spec that drove that build,
`phase-a4-editor-setup.md`, was retired once it was done and is in git history at
commit `bcd947a`.

## Checks

CI (`.github/workflows/ci.yml`) runs five gates on every push and pull request.
None of them compile: Unreal Engine cannot be installed on a hosted runner, so
compiling is a local step, `./tools/ci/compile.sh`, after every source change.
The workflow also carries an opt-in `compile` job that runs that same script on a
self-hosted macOS runner (labels `self-hosted, macOS, unreal`, enabled by setting
the repository variable `UNREAL_SELF_HOSTED` to `true`); without the variable it
is skipped rather than queued against a runner that does not exist.

| Gate | What it does |
|---|---|
| `check_hygiene.py` | secret patterns, absolute local paths, TfL trademark leakage |
| `check_cpp_conventions.py` + clang-format | Unreal prefixes, `#pragma once`, generated-header order, `TObjectPtr` in containers, no `LogTemp`, no unfinished markers, British spelling |
| `check_cpp_reflection.py` | the compiler stand-in: missing `GENERATED_BODY()`, a reflected header not including its own `generated.h`, a `.cpp` that does not include its own header first, `LT_LOG` format specifiers that disagree with their arguments |
| `check_docs.py` | British spelling, no em or en dashes, JSON validity, dead relative links |
| `check_content.py` | every tracked `.uasset` / `.umap` is a Git LFS pointer, not raw binary |

Run all five locally:

```bash
python3 tools/ci/check_hygiene.py
python3 tools/ci/check_cpp_conventions.py
python3 tools/ci/check_cpp_reflection.py
python3 tools/ci/check_docs.py
python3 tools/ci/check_content.py
```

And the compile, which is the one gate CI cannot run for you:

```bash
./tools/ci/compile.sh
```

## Current state

Grey box phase. The C++ is now feature complete for Phases A to C plus the
downed state: player character with a bleed-out and revive loop, weapon component
with hip fire and aim down sights, five data-driven zombie types off one mesh, the
round manager with a roster and the sprinter and brute special rounds, points
economy, the interaction system with a wall buy, station heat, the train with its
arrive, dwell, depart cycle and boarding, the departure board, and travel between
two stations carrying points and weapon across the level load.

**Everything after the train has been written but never compiled**: it was
authored in remote sessions with no engine to hand, so the first local build is
the gate on it. In the editor the game still plays as the Phase B grey box,
because the data assets and Blueprints that turn the new systems on do not exist
yet. Both jobs are written up: `docs/tasks/handover.md` for the state and the
order, `docs/tasks/neostack.md` for the editor work. Not built at all: perks, the
art pass, audio.

Full plan and status in `docs/tasks/README.md`; the resume point for a fresh
session is `docs/tasks/handover.md`.

## Documents

- `CLAUDE.md` - build specifics, module layout, conventions, model split
- `docs/tasks/` - the phase plan and one bounded task per file; `handover.md` is the resume point
- `docs/brief-v3-unreal.md` - current brief: engine, camera, phases, model split
- `docs/brief-v2.md` - superseded for engine, still authoritative on game design
- `docs/design/gameplay-canon.md` - the settled design stated tight
- `docs/art-direction.md` - palette, composition, and the trademark substitutions
- `docs/unreal-setup.md` - editor steps the source cannot do for you
- `docs/reference/` - the reference frame, its notes, and the Canary Wharf research
- `docs/known-issues.md` - open problems, unverified work, and what the remote session cannot do
- `Content/ATTRIBUTION.md` - provenance and licence for every imported asset

## Licence

See `LICENSE`.

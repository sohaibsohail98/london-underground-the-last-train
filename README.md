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
web/                    Discarded Three.js build, tagged phase-03, not part of this work
```

The Unreal target is the project. `web/` is the earlier browser build, kept on
`main` under the `phase-03` tag as a historical reference and a last-resort
fallback, and is not developed.

## Running the Unreal target

Unreal Engine 5.8, macOS. The `LastTrain` C++ module compiles with the batch
build in `CLAUDE.md` (external Xcode on `/Volumes/DriveSohaib` must be mounted).
The editor assets for the first playable grey box are built; follow
`docs/unreal-setup.md` and `docs/tasks/phase-a4-editor-setup.md` for how they
were made and `docs/tasks/NEXT.md` for where the work stands.

## Checks

CI (`.github/workflows/ci.yml`) runs four gates on every push and pull request.
The engine is not available in CI, so C++ is never compiled there; compile
locally after every source change.

| Gate | What it does |
|---|---|
| `check_hygiene.py` | secret patterns, absolute local paths, TfL trademark leakage |
| `check_cpp_conventions.py` + clang-format | Unreal prefixes, `#pragma once`, generated-header order, `TObjectPtr` in containers, no `LogTemp`, no unfinished markers, British spelling |
| `check_docs.py` | British spelling, no em or en dashes, JSON validity, dead relative links |
| `check_content.py` | every tracked `.uasset` / `.umap` is a Git LFS pointer, not raw binary |

Run all four locally:

```bash
python3 tools/ci/check_hygiene.py
python3 tools/ci/check_cpp_conventions.py
python3 tools/ci/check_docs.py
python3 tools/ci/check_content.py
```

## Current state

Grey box phase. The C++ combat chassis compiles on UE 5.8: player character,
weapon component with hip fire and aim down sights, zombie with head hitboxes and
round scaling, round manager and spawn points, points economy, the interaction
system with a wall buy, station heat and the run-state backbone. The Phase A grey
box map plays and the Phase B code (throttled repath, interaction, HUD) has
landed with acceptance still being verified in PIE. Not yet built: the train,
the five zombie types, perks, travel between stations, the art pass, audio.

Full plan and status in `docs/tasks/README.md`; the resume point for a fresh
session is `docs/tasks/NEXT.md`.

## Documents

- `CLAUDE.md` - build specifics, module layout, conventions, model split
- `docs/tasks/` - the phase plan and one bounded task per file; `NEXT.md` is the resume point
- `docs/brief-v3-unreal.md` - current brief: engine, camera, phases, model split
- `docs/brief-v2.md` - superseded for engine, still authoritative on game design
- `docs/design/gameplay-canon.md` - the settled design stated tight
- `docs/art-direction.md` - palette, composition, and the trademark substitutions
- `docs/unreal-setup.md` - editor steps the source cannot do for you
- `docs/reference/` - the reference frame, its notes, and the Canary Wharf research
- `Content/ATTRIBUTION.md` - provenance and licence for every imported asset

## Licence

See `LICENSE`.

# Tasks

One bounded task per file. Point a fresh session at the relevant file rather than
re typing the spec. The plan below supersedes the roadmap in `brief-v3-unreal.md`
Part 3 where they differ, because the C++ combat slice now compiles and the
sequencing has moved on.

The visual target for the whole project is `docs/reference/reference-frame.png`.
Read `docs/reference/reference-frame-notes.md` before any art or layout task.

## Status

| Phase | Goal | Gate | State |
|---|---|---|---|
| A | Foundation and first playable grey box | `unreal-setup.md` section 8 checklist passes | **done 2026-09-04**. `L_GreyboxTest` plays, A5 passed on every testable check (5, 8, 9, 10 not reached due to NeoStack input harness degradation, not code) |
| B | Engine core hardening: throttled repath, interaction system, hit markers | 24 to 40 zombies on the grey box platform stays stable at 60fps | B1, B2, B3 all landed. Acceptance in progress. Two PIE findings open: zombie attack lands nothing, spawn count plateaued at 6 (a per instance override, not a code bug). Diagnostics compiled, one more PIE pass needed. See `NEXT.md`. |
| C | Rounds, five zombie types, the train, the departure board, station heat | a train arrives on schedule, you can board during dwell, staying raises pressure, rounds 1 to 10 play untouched | not started. All C++. Numbers from `brief-v2.md`. |
| D | Grey box Canary Wharf | it is fun to train zombies around in grey boxes | blockout mostly built early (2026-09-05 NeoStack run, 17 of 18 items). `L_CanaryWharf_Greybox.umap`. Level Blueprint wiring and horde smoke test still needed. |
| E | Perks, upgrade bench, lost property, downed and revive | a full survival session start to death is possible | not started |
| F | Art pass, Fable led | a screenshot of the platform stands next to the reference frame without embarrassment | not started |
| G | Audio, restrained HUD, second station, balance | play well using only what is on screen | not started |

## Phase A tasks

| File | Task | Who | State |
|---|---|---|---|
| `phase-a1-commit-foundation.md` | Fix `.uproject`, land `CLAUDE.md` and this scaffold, commit the pending source changes | done | done |
| `phase-a4-editor-setup.md` | The in editor work: trace channel, Input assets, Blueprints, data asset, grey box test map | NeoStack agent, `Weapon` channel by hand | done 2026-09-04. All assets built under `Content/LastTrain/`. Weapon `DisplayName` is "Stag Compact". |
| `phase-a5-acceptance.md` | Run the ten point acceptance test, record failures | NeoStack agent | done. Checks 1 to 4, 6, 7 pass with numbers. 5, 8, 9, 10 not reached due to harness input degradation, no code failures found. |

## Phase B tasks

Written ahead. Run them in order; each assumes the one before it landed.

| File | Task | Who | State |
|---|---|---|---|
| `phase-b1-throttled-repath.md` | Throttle per zombie `MoveToActor` to a jittered cadence | Opus | C++ landed. RVO radius 90 to 45 and acceptance `0.75f` to `0.5f` fixes on top (supersedes the "do not change the movement setup" line in the spec). Crowd frame check still blocked on a round manager instance override, see `NEXT.md`. |
| `phase-b2-interaction.md` | Interaction component, `Interact` input, first wall buy | Opus or Sonnet | C++ landed, compiles. `L_GreyboxTest` has the `LTWallBuy`. Acceptance not signed off, PIE pass pending. |
| `phase-b3-feedback-widgets.md` | Hit marker, crosshair, prompt, restrained HUD. No C++ | NeoStack | `WBP_HUD` built and in git. Bug 1 (prompt fade branch) fixed. Open: `HealthBar.Percent` has no binding at all, needs wiring to `GetHealthFraction()` or `OnHealthChanged`. |

## Building the editor assets

`docs/tasks/neostack-build.md` is the brief for a NeoStack agent driving the
Unreal editor through `execute_script`: the concrete asset list for A4 and B3,
the C++ parent classes and property names, the constraints, and what NeoStack
cannot do (custom trace channels and other bespoke Project Settings UI stay
human only). Phase C editor work is stubbed there, pending the Phase C C++.

## Free assets to fill the art gap

`docs/reference/free-assets.md` is a vetted, licence checked list of free
Unreal content for Phases D to G: a modular subway kit, City Sample Crowds for
distinct zombie types, animation sample projects, Niagara FX, Sonniss audio,
CC0 surfaces and OFL fonts. Two filters run on everything: no real TfL trade
dress, and no re hosting Epic or Fab raw assets in a public repo. Imported
packs are gitignored and fetched per that file; our own work under
`Content/LastTrain/` is committed via LFS.

## How to resume in a fresh context window

Start with `docs/tasks/NEXT.md`. It carries the current state, what was just done,
and the exact next action, written so a cold session can pick up without
re reading the whole history.

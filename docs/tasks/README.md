# Tasks

One bounded task per file. Point a fresh session at the relevant file rather than
re typing the spec. The plan below supersedes the roadmap in `brief-v3-unreal.md`
Part 3 where they differ, because the C++ combat slice now compiles and the
sequencing has moved on.

The visual target for the whole project is `docs/reference/reference-frame.png`.
Read `docs/reference/reference-frame-notes.md` before any art or layout task.

**Committed scope numbers.** v1 ships **2 stations**. Any "41 stations"
reference in the docs is an aspirational expansion list, not a plan. The
fictional line is a fictionalised Crossrail-scale line (main-line loading
gauge, modelled on the Elizabeth line, NOT a deep-level tube); the train is a
Class 345 "Aventra" silhouette. See `docs/brief-v3-unreal.md` Part 1.

## Status

| Phase | Goal | Gate | State |
|---|---|---|---|
| A | Foundation and first playable grey box | `unreal-setup.md` section 8 checklist passes | **done 2026-09-04**. `L_GreyboxTest` plays, A5 passed on every testable check (5, 8, 9, 10 not reached due to NeoStack input harness degradation, not code) |
| B | Engine core hardening: throttled repath, interaction system, hit markers | 24 to 40 zombies on the grey box platform stays stable at 60fps | B1, B2, B3 code all landed. Zombie attack fixed (`DriveTowardsTarget`) and verified. HUD health bar fixed and verified. **Gate not yet measured**: the `GreyboxTest_RoundManager` instance still caps every PIE run at 6 zombies (a per-instance `OpeningRoundCounts` override that needs clearing by hand in the Details panel), so the 24 to 40 crowd frame check has never run. A corridor stall-recovery edge case and the B2 wall-buy PIE acceptance are also open. See `NEXT.md`. |
| C | Rounds, five zombie types, the train, the departure board, station heat | a train arrives on schedule, you can board during dwell, staying raises pressure, rounds 1 to 10 play untouched | in progress. All C++. `LTStationHeat`, `LTGameMode` and `LTGameState` landed as groundwork. C1 `ALTTrain` (`phase-c1-train.md`), C2 `ALTDepartureBoard` (`phase-c2-departure-board.md`) and C3 travel (`phase-c3-travel.md`) are all written. Their PIE acceptance is not yet run, and C2 and C3 have not been compiled: they were written in a remote session with no engine, so the first local build is the compile gate. Still to do: `phase-c-zombie-types.md`, then `phase-c-special-rounds.md`. Numbers from `brief-v2.md` and `docs/design/gameplay-canon.md`. |
| D | Grey box Canary Wharf | it is fun to train zombies around in grey boxes | blockout built early (NeoStack, 17 of 18 items), `L_CanaryWharf_Greybox.umap`. Level Blueprint `BeginRounds` wiring is now done and rounds start; a spawn-point fall-through bug (zombies drop through the floor on spawn) blocks the horde smoke test. See the 2026-09-06 run log. |
| E | Perks, upgrade bench, lost property, downed and revive | a full survival session start to death is possible | started. E1 downed and revive (`phase-e1-downed-revive.md`) is written into `ALTPlayerCharacter`: zero health downs rather than kills, a bleed-out clock runs, a solo auto-revive stands the player back up at half health, and `Revive()` is the seam for an item or a co-op revive. Not compiled (written with no engine to hand) and PIE acceptance not run. Perks, the bench and lost property are untouched. |
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
| `phase-b1-throttled-repath.md` | Throttle per zombie `MoveToActor` to a jittered cadence | Opus | C++ landed. RVO radius 90 to 45, acceptance `0.75f` to `0.5f`, then a `DriveTowardsTarget` rework with `ContactRange` and a stall-recovery nudge, all on top (supersedes the "do not change the movement setup" line in the spec). The 24 to 40 crowd frame check is still blocked on the `GreyboxTest_RoundManager` instance override, see `NEXT.md`. |
| `phase-b2-interaction.md` | Interaction component, `Interact` input, first wall buy | Opus or Sonnet | C++ landed, compiles. `L_GreyboxTest` has the `LTWallBuy`. Acceptance not signed off: the wall-buy prompt and purchase flow have not been exercised in PIE (blocked on scriptable first-person aim; needs an interactive pass). |
| `phase-b3-feedback-widgets.md` | Hit marker, crosshair, prompt, restrained HUD. No C++ | NeoStack | `WBP_HUD` built and in git. Prompt fade branch fixed. Health bar fixed (a zero-height `SizeBox`, not a binding bug) and verified draining in PIE. Hit marker, crosshair spread and prompt-anchor polish not yet reviewed. |

## Phase C tasks

Run C1 and the zombie-type mechanism first; the rest are independent except
special rounds, which needs the zombie types. Each file is self-contained and can
be handed to a fresh session cold.

| File | Task | Who | State |
|---|---|---|---|
| `phase-c1-train.md` | `ALTTrain` timing state machine, presentation hooks, boarding interact, `NotifyPlayerBoarded` | Opus | **C++ landed** (`7cb7c6d`), compiles clean, 4 CI gates pass. 12-point PIE acceptance pending a `BP_Train` (editor task). |
| `phase-c-zombie-types.md` | `ULTZombieTypeData`, `ApplyTypeData`, roster on the round manager | Opus, then Sonnet for the 5 stat blocks | Not started. Blocks `phase-c-special-rounds.md`. |
| `phase-c2-departure-board.md` | `ALTDepartureBoard` actor reading the train countdown getters | Opus | **C++ written**, new files only (`Train/LTDepartureBoard.{h,cpp}`), 4 CI gates pass, clang-format 20 clean. **Not compiled**: written in a remote session with no engine. Runtime acceptance needs a `BP_DepartureBoard` with a text render child driven by `OnCountdownChanged`, placed near the `BP_Train`. |
| `phase-c3-travel.md` | `ULTGameInstance` travel payload, `OpenLevel` on board, rehydrate on arrival, 2 stations | Opus | **C++ written**: `Core/LTGameInstance.{h,cpp}`, `ALTGameMode::NextStationMap` plus the payload build in `NotifyPlayerBoarded` and `RehydrateFromTravel` on arrival, and `GameInstanceClass` in `Config/DefaultEngine.ini`. 4 CI gates pass. **Not compiled.** Editor step left: set `NextStationMap` on each station's game mode. Two design choices recorded: rounds restart at 1 on travel, and the reserve carries to full rather than to the exact count. |
| `phase-c-special-rounds.md` | Sprinter round every 5th, brute pair every 10th, as a plan layer on `ALTRoundManager` | Opus | Spec ready but still **blocked** until `phase-c-zombie-types.md` lands. |

## Phase E tasks

| File | Task | Who | State |
|---|---|---|---|
| `phase-e1-downed-revive.md` | Solo downed state, bleed-out timer, auto-revive at half health, in `ALTPlayerCharacter` only | Opus | **C++ written**: `Down()`, `Revive()`, `Die()`, the bleed-out and auto-revive clocks in `Tick`, input locked while down with `Look` left free, and the `OnDowned` / `OnRevived` / `OnBleedOutExpired` hooks. 4 CI gates pass. **Not compiled.** Seams left open: `Revive()` for an item or a co-op revive, the two hooks for a last-stand weapon swap, and the downed screen treatment for the HUD and art pass. |

## Building the editor assets

`docs/tasks/neostack-build.md` is the brief for a NeoStack agent driving the
Unreal editor through `execute_script`: the concrete asset list for A4 and B3,
the C++ parent classes and property names, the constraints, and what NeoStack
cannot do (custom trace channels and other bespoke Project Settings UI, actor
instance-property resets, and the level Blueprint EventGraph all stay human
only). Phase C editor work is stubbed there, pending the Phase C C++. The dated
run logs (`neostack-run-2026-09-06.md`) are the live checklists for a session.

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

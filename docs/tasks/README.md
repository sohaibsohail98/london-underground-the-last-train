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
| B | Engine core hardening: throttled repath, interaction system, hit markers | 24 to 40 zombies on the grey box platform stays stable at 60fps | B1, B2, B3 code all landed. Zombie attack fixed (`DriveTowardsTarget`) and verified. HUD health bar fixed and verified. **Gate not yet measured**, but no longer blocked: `bb0ec42` made `OpeningRoundCounts` `EditAnywhere` and the 2026-09-07 run proved a placed instance takes the write, so the crowd check is a matter of setting index 0 to 30, measuring, and putting it back. The corridor stall recovery tested clean on 2026-09-07, leaving one flagged anomaly rather than a defect. The B2 wall-buy acceptance and the B3 polish review are still open. See `handover.md`. |
| C | Rounds, five zombie types, the train, the departure board, station heat | a train arrives on schedule, you can board during dwell, staying raises pressure, rounds 1 to 10 play untouched | **all five Phase C specs are written as C++**: C1 `ALTTrain`, C2 `ALTDepartureBoard`, C3 travel, the five zombie types and the special rounds. C1 compiled locally; everything after it was written in a remote session with no engine and **has never been compiled**, so the first local build is the gate on the lot. No PIE acceptance has been run. What is left is editor work, specified in `neostack.md`: the five type assets, the tintable material, `BP_Train`, `BP_DepartureBoard`, the roster wiring, and reparenting `BP_GameMode` to `ALTGameMode`. Numbers from `brief-v2.md` and `docs/design/gameplay-canon.md`. |
| D | Grey box Canary Wharf | it is fun to train zombies around in grey boxes | blockout built (NeoStack, 17 of 18 items), `L_CanaryWharf_Greybox.umap`, rounds start, and as of `bf1878c` **the spawn fall-through is fixed and the horde smoke test passes**: all ten points placed on the platform, zombies spawn on the floor, converge and reach melee, nothing drops through at any of nine tested positions. `L_GreyboxTest`'s five points turned out to be at world origin too and were placed as well. Left: lighting rebuild, and the space has never been walked for fun. |
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
| `phase-b1-throttled-repath.md` | Throttle per zombie `MoveToActor` to a jittered cadence | Opus | C++ landed. RVO radius 90 to 45, acceptance `0.75f` to `0.5f`, then a `DriveTowardsTarget` rework with `ContactRange` and a stall-recovery nudge, all on top (supersedes the "do not change the movement setup" line in the spec). The 24 to 40 crowd frame check is still unmeasured: the write to raise the count works, but the editor throttles its tick while unfocused, so it needs a human with the window focused or a packaged build. The stall recovery itself tested clean on 2026-09-07. See `handover.md`. |
| `phase-b2-interaction.md` | Interaction component, `Interact` input, first wall buy | Opus or Sonnet | C++ landed, compiles. `L_GreyboxTest` has the `LTWallBuy`. Acceptance not signed off: the wall-buy prompt and purchase flow have not been exercised in PIE (blocked on scriptable first-person aim; needs an interactive pass). |
| `phase-b3-feedback-widgets.md` | Hit marker, crosshair, prompt, restrained HUD. No C++ | NeoStack | `WBP_HUD` built and in git. Prompt fade branch fixed. Health bar fixed (a zero-height `SizeBox`, not a binding bug) and verified draining in PIE. Hit marker, crosshair spread and prompt-anchor polish not yet reviewed. |

## Phase C tasks

Run C1 and the zombie-type mechanism first; the rest are independent except
special rounds, which needs the zombie types. Each file is self-contained and can
be handed to a fresh session cold.

| File | Task | Who | State |
|---|---|---|---|
| `phase-c1-train.md` | `ALTTrain` timing state machine, presentation hooks, boarding interact, `NotifyPlayerBoarded` | Opus | **C++ landed** (`7cb7c6d`), compiles clean, 4 CI gates pass. 12-point PIE acceptance pending a `BP_Train` (editor task). |
| `phase-c-zombie-types.md` | `ULTZombieTypeData`, `ApplyTypeData`, roster on the round manager | Opus, then Sonnet for the 5 stat blocks | **C++ written**: the data asset with both enums, `ApplyTypeData` with the capsule, navigation and behaviour application, the armour plate, the sprinter lunge, the screamer line of sight and summon, and the weighted roster with the heat-3 shift on `ALTRoundManager`. 4 CI gates pass. **Not compiled.** The five stat blocks and the tintable material are editor work, now specified in `neostack.md`. |
| `phase-c2-departure-board.md` | `ALTDepartureBoard` actor reading the train countdown getters | Opus | **C++ written**, new files only (`Train/LTDepartureBoard.{h,cpp}`), 4 CI gates pass, clang-format 20 clean. **Not compiled**: written in a remote session with no engine. Runtime acceptance needs a `BP_DepartureBoard` with a text render child driven by `OnCountdownChanged`, placed near the `BP_Train`. |
| `phase-c3-travel.md` | `ULTGameInstance` travel payload, `OpenLevel` on board, rehydrate on arrival, 2 stations | Opus | **C++ written**: `Core/LTGameInstance.{h,cpp}`, `ALTGameMode::NextStationMap` plus the payload build in `NotifyPlayerBoarded` and `RehydrateFromTravel` on arrival, and `GameInstanceClass` in `Config/DefaultEngine.ini`. 4 CI gates pass. **Not compiled.** Editor steps left: reparent `BP_GameMode` to `ALTGameMode`, then fill `StationRoutes` with both directions. Two design choices recorded: rounds restart at 1 on travel, and the reserve carries to full rather than to the exact count. |
| `phase-c-special-rounds.md` | Sprinter round every 5th, brute pair every 10th, as a plan layer on `ALTRoundManager` | Opus | **C++ written** on top of the roster: `FLTRoundPlan`, `BuildRoundPlan`, the three-way branch in `TrySpawnOne`, `IsSpecialRound` and `GetSpecialRoundTag`. 4 CI gates pass. **Not compiled.** Deviates from the spec on two points where `gameplay-canon.md` says otherwise: the brute pair lands at roughly 30 and 70 per cent through the round rather than as a group up front, and a round that is both (20, 30) is a sprinter round carrying the pair rather than brutes taking precedence. |

## Phase E tasks

| File | Task | Who | State |
|---|---|---|---|
| `phase-e1-downed-revive.md` | Solo downed state, bleed-out timer, auto-revive at half health, in `ALTPlayerCharacter` only | Opus | **C++ written**: `Down()`, `Revive()`, `Die()`, the bleed-out and auto-revive clocks in `Tick`, input locked while down with `Look` left free, and the `OnDowned` / `OnRevived` / `OnBleedOutExpired` hooks. 4 CI gates pass. **Not compiled.** Seams left open: `Revive()` for an item or a co-op revive, the two hooks for a last-stand weapon swap, and the downed screen treatment for the HUD and art pass. |

## Building the editor assets

`docs/tasks/neostack.md` is the single brief for every outstanding editor task,
whether a NeoStack agent drives it through `execute_script` or a human does it by
hand: the ground rules, the C++ parent classes and property names, the delegate
table, the five zombie type assets with their full value tables, the tintable
material, `BP_Train`, `BP_DepartureBoard`, the `BP_GameMode` reparent, the Canary
Wharf spawn point placement and the Phase B leftovers. It also lists what cannot
be scripted and has to be done by hand: custom trace channels, other bespoke
Project Settings UI, and the level Blueprint EventGraph.

The dated NeoStack run logs are gone: their live findings are folded into
`neostack.md` and `handover.md`, and the rest was history.

## Free assets to fill the art gap

`docs/reference/free-assets.md` is a vetted, licence checked list of free
Unreal content for Phases D to G: a modular subway kit, City Sample Crowds for
distinct zombie types, animation sample projects, Niagara FX, Sonniss audio,
CC0 surfaces and OFL fonts. Two filters run on everything: no real TfL trade
dress, and no re hosting Epic or Fab raw assets in a public repo. Imported
packs are gitignored and fetched per that file; our own work under
`Content/LastTrain/` is committed via LFS.

## How to resume in a fresh context window

Start with `docs/tasks/handover.md`. It carries the current state, what was just done,
and the exact next action, written so a cold session can pick up without
re reading the whole history.

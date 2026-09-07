# NEXT - resume point for a fresh context window

Last updated 2026-09-06 (C1 train landed). Update this whenever you finish a task so a cold
session can pick up without re-reading the whole history.

## The one line

Phase A is DONE. Phase B code (B1 throttled repath, B2 interaction, B3 HUD) has
all landed and compiled. The zombie attack fix and the HUD health bar are
verified working in PIE. Phase B is **not signable**: the 24 to 40 zombie crowd
frame gate has never been measured because the `GreyboxTest_RoundManager`
instance caps every run at 6, a corridor stall-recovery edge case is still open
in C++, and the B2 wall-buy flow has not been exercised in PIE. Phase C has
started: **C1 `ALTTrain` has landed and compiles clean**, but its 12-point PIE
acceptance list has not been run and no `BP_Train` exists yet. The five zombie
types and the departure board are specced and waiting.

## State of the tree

- Branch `main`, C++ binary compiled clean through the 2026-09-06 fixes
  (`bb0ec42`). If the editor is open, confirm it is on the fresh binary by
  checking `LTZombieCharacter` reflects `StallSpeedThreshold`, `ContactRange`
  etc before trusting PIE behaviour.
- Design canon is `docs/design/gameplay-canon.md`. The old 167-item
  open-questions sweep is deleted; it is in git history at commit `0b7e35f`.
  All its items were resolved to the suggested default.
- The Phase C specs (`phase-c1-train.md`, `phase-c-zombie-types.md`) have had
  their `[DECISION NEEDED]` markers resolved against the accept-all-defaults
  decision and the settled line identity. Read them as written.

## Exact next actions, in order

1. **Clear the round manager instance override.** In `L_GreyboxTest`, select
   `GreyboxTest_RoundManager` in the World Outliner, find `Opening Round Counts`
   in Details, hit the yellow reset arrow so it inherits the class default. This
   must be done by hand in the Details panel: `OpeningRoundCounts` is
   `EditDefaultsOnly`, so no scripted write path and no CDO edit can reach the
   placed instance. For the crowd test, set index 0 to 30 on the instance.

2. **Measure the Phase B gate.** PIE `L_GreyboxTest`, console `stat unit`, filter
   the Output Log for `LogLastTrain`. Confirm `Spawned zombie. Alive N ...`
   climbs past 6 toward the `MaximumAlive` cap of 24 (about 38s at the current
   spawn interval), and record the game thread ms with 24+ alive over 10s. Pass
   is the game thread holding near 16.6 ms. Then revert index 0 to 6 and save.

3. **Fix the corridor stall edge case.** The 2026-09-06 run found a single zombie
   a short distance outside `AttackRange` (about 19 units) can sit frozen at zero
   velocity for 20+ seconds while `UpdateStallRecovery`'s nudge never fires. Check
   the order of the gates in `UpdateStallRecovery`, the ground-speed sample it
   reads, and whether `StallTimer` is reset before `StallGraceSeconds` can
   accumulate. Bounded C++ change to `LTZombieCharacter.cpp`, compile, re-verify
   in PIE with 6+ zombies queued.

4. **Strip the diagnostic logs.** Remove the `// DIAGNOSTIC` `LT_LOG` lines from
   `TryAttack` and `TrySpawnOne` if they are still present. Keep the deliberate
   `LT_LOG(Verbose, "Spawned zombie. Alive %d ...")` in `TrySpawnOne` (it is kept
   for the intermittent-spawn investigation). Keep the `BlueprintReadOnly` field
   exposure on the zombie. Recompile.

5. **B2 wall-buy PIE acceptance.** Needs an interactive pass (mouse plus WASD in a
   real PIE session; the NeoStack bridge cannot aim the first-person camera). Walk
   to the `GreyboxTest_WallBuy_SMG` plate: prompt fades in, `E` under 500 points
   no-ops, `E` with 500+ buys and swaps the weapon and flashes the points crimson,
   `E` again offers ammunition at 250. Then the vignette and regen checks from the
   B3 acceptance list.

6. **Canary Wharf spawn fall-through.** In `L_CanaryWharf_Greybox`, rounds now
   start but spawned zombies free-fall through the level. Check `RecastNavMesh-
   Default` coverage and floor collision under the `CW_SpawnPoint_*` actors,
   most likely the top clusters. Then re-run the horde smoke test.

7. **Mark Phase B done** in `docs/tasks/README.md` and update this file once the
   gate is measured and 3 to 5 pass.

8. **C1 train PIE acceptance.** Editor work, no C++ needed. Make `BP_Train` from
   `ALTTrain`, give it a box mesh child in the trackbed, place it at the platform
   edge, size `BoardingVolume` over the door aperture, add a `ULTStationHeat`
   component to `GreyboxTest_RoundManager`, and put print or log nodes on the nine
   presentation hooks and `OnTrainPhaseChanged`. Then walk the 12-point list in
   `docs/tasks/phase-c1-train.md`.

## Phase C, the next real code work

All C++. Numbers from `docs/design/gameplay-canon.md` and `docs/brief-v2.md`.

- `ALTTrain` (`phase-c1-train.md`): **C++ DONE**, compiled clean. The arrive /
  dwell / depart / away state machine on the 100s interval and 25s dwell, the
  `BlueprintImplementableEvent` presentation hooks, the "Board train" interact,
  `ALTGameMode::NotifyPlayerBoarded`, and the heat increment on a not-boarded
  departure. New files `Source/LastTrain/Public/Train/LTTrain.h` and
  `Source/LastTrain/Private/Train/LTTrain.cpp`. Travel to another station stayed
  out of scope: `NotifyPlayerBoarded` is the seam and logs that travel is unwired.
  **Outstanding**: build a `BP_Train` from `ALTTrain` with a box mesh child, place
  it at the platform edge in `L_GreyboxTest` with `BoardingVolume` over the door
  aperture, put a `ULTStationHeat` component on the round manager, and run the
  12-point acceptance list in `phase-c1-train.md`. Drop the tuneables right down
  (`FirstTrainStopSeconds`, `TrainInterval`, `DwellDuration`) while testing so a
  cycle takes seconds, then restore 30 / 100 / 25.
- `ULTZombieTypeData` (`phase-c-zombie-types.md`): the data asset, `ApplyTypeData`
  on the zombie, the roster on `ALTRoundManager` (keep `ZombieClass` as the
  empty-roster fallback). Data entry of the five stat blocks is a separate
  Sonnet task once the mechanism lands.
- Then the departure board actor (`ALTDepartureBoard`) reading the train's
  countdown getters, and the travel transition that fills the
  `NotifyPlayerBoarded` seam.

## Editor tooling notes

- `Plugins/NeoStackAI/` is a gitignored third-party editor plugin (`.uasset` /
  `.umap` building through `execute_script`). Its trial expired around
  2026-09-07; assume it is not available and that editor asset work is by hand.
- NeoStack could not do: custom trace and object channels, other bespoke Project
  Settings UI, resetting an actor instance property to its class default, and
  the level Blueprint EventGraph. The `Weapon` trace channel (slot 1,
  `ECC_GameTraceChannel1`, response Ignore) is in `Config/DefaultEngine.ini`.
- `CommonUI` is in `LastTrain.uproject` `Plugins` explicitly because NeoStack
  enabled it as a dependency without its modules built, which SIGSEGV'd PIE.
  Do not remove that entry.
- The `EndPlayMap` SIGSEGV on messy PIE teardown is documented in
  `docs/tasks/editor-crash-endplaymap.md`. Never mutate actors or assets while
  PIE is running; stop PIE with `playtest.stop` or Escape, not by closing the
  window.

## Build and toolchain

- Editor build, run after any C++ change:
  `"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"`
- External Xcode on `/Volumes/DriveSohaib` must be mounted.
- No CI compile. Compile locally after every C++ change, keep changes small.
- **Pending: engine move to the external drive**, `docs/tasks/drive-migration.md`.
  Do it when the editor is fully closed. Frees about 43 GiB.

## Content in git

Our own assets under `Content/LastTrain/` are committed via Git LFS per
`.gitattributes`. Imported third-party packs are gitignored by their landing
folders because UE-Only Content and Fab Standard licences forbid re-hosting raw
assets in a public repo. Vetted free asset list: `docs/reference/free-assets.md`.
Canary Wharf reference research: `docs/reference/canary-wharf-research/`. If a
pack imports to a new folder, add it to `.gitignore` before committing.

## Phase plan in brief

A foundation and first playable grey box (done) -> **B engine core hardening
(repath, interaction, HUD), gate not yet measured** -> C rounds, five zombie
types, the train, the departure board, station heat -> D grey box Canary Wharf
(blockout built, spawn fall-through to fix) -> E perks, bench, lost property,
revive -> F art pass, Fable led, against the reference frame -> G audio, HUD,
second station, balance. Full table in `docs/tasks/README.md`.

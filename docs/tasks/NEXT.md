# NEXT - resume point for a fresh context window

Last updated 2026-09-07 (C2 departure board, C3 travel and E1 downed/revive written). Update
this whenever you finish a task so a cold session can pick up without re-reading the whole
history.

## The one line

Phase A is DONE. Phase B code (B1 throttled repath, B2 interaction, B3 HUD) has
all landed and compiled. The zombie attack fix and the HUD health bar are
verified working in PIE. Phase B is **not signable**: the 24 to 40 zombie crowd
frame gate has never been measured because the `GreyboxTest_RoundManager`
instance caps every run at 6, a corridor stall-recovery edge case is still open
in C++, and the B2 wall-buy flow has not been exercised in PIE. Phase C has
started: **C1 `ALTTrain` has landed and compiles clean**, but its 12-point PIE
acceptance list has not been run and no `BP_Train` exists yet.

**Three more specs are now written as C++ but have never been compiled**: C2
`ALTDepartureBoard`, C3 travel (`ULTGameInstance` plus the `ALTGameMode` payload
and rehydrate path, plus the `GameInstanceClass` ini line) and E1 downed and
revive in `ALTPlayerCharacter`. They were written in a remote Linux session with
no Unreal engine and no Xcode, so all four `tools/ci/` gates and clang-format 20
pass but nothing has been through the compiler. **The first local editor build is
the gate on all three.** The five zombie types and the special rounds that depend
on them are still specced and waiting.

## State of the tree

- Branch `claude/docs-tasks-implementation-vd3wm7` carries the C2, C3 and E1
  work on top of `main`. The last binary compiled clean was the 2026-09-06 fixes
  (`bb0ec42`) plus C1; nothing since C1 has been compiled. If the editor is open,
  confirm it is on the fresh binary by checking `LTZombieCharacter` reflects
  `StallSpeedThreshold`, `ContactRange` etc before trusting PIE behaviour.
- `LastTrain.uproject` on `main` still carries the `NeoStackAI` plugin entry
  while `Plugins/NeoStackAI/` is gitignored, so a clone without the plugin gets a
  missing-plugin prompt. The editor also rewrites this file on launch (spaces for
  tabs, and a `PixelStreaming2` entry). Neither is committed on this branch. It
  wants a decision now that the NeoStack trial has expired: drop the entry, or
  keep it and accept the prompt.
- Design canon is `docs/design/gameplay-canon.md`. The old 167-item
  open-questions sweep is deleted; it is in git history at commit `0b7e35f`.
  All its items were resolved to the suggested default.
- The Phase C specs (`phase-c1-train.md`, `phase-c-zombie-types.md`) have had
  their `[DECISION NEEDED]` markers resolved against the accept-all-defaults
  decision and the settled line identity. Read them as written.

## Exact next actions, in order

1. **Compile.** Mount `/Volumes/DriveSohaib`, confirm `xcode-select -p` points
   inside it, then build the editor target:
   `"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"`
   Three tasks (C2, C3, E1) have never seen a compiler. Fix anything it finds
   before touching the editor, because the C3 ini line changes the game instance
   class for every map: a `ULTGameInstance` that fails to compile takes PIE with
   it. New files that must appear in the build:
   `Source/LastTrain/{Public,Private}/Train/LTDepartureBoard.*` and
   `Source/LastTrain/{Public,Private}/Core/LTGameInstance.*`. Then verify in the
   editor that Project Settings shows the game instance class as
   `LTGameInstance`, from
   `[/Script/EngineSettings.GameMapsSettings] GameInstanceClass=/Script/LastTrain.LTGameInstance`
   in `Config/DefaultEngine.ini`.

2. **Set the travel destinations.** On the `L_GreyboxTest` game mode set
   `NextStationMap` to `L_CanaryWharf_Greybox`, and on the Canary Wharf game mode
   set it back to `L_GreyboxTest`. Unset means boarding ends the run where it
   stands, which is the pre-C3 behaviour, so nothing breaks if this is skipped.

3. **Clear the round manager instance override.** In `L_GreyboxTest`, select
   `GreyboxTest_RoundManager` in the World Outliner, find `Opening Round Counts`
   in Details, hit the yellow reset arrow so it inherits the class default. This
   must be done by hand in the Details panel: `OpeningRoundCounts` is
   `EditDefaultsOnly`, so no scripted write path and no CDO edit can reach the
   placed instance. For the crowd test, set index 0 to 30 on the instance.

4. **Measure the Phase B gate.** PIE `L_GreyboxTest`, console `stat unit`, filter
   the Output Log for `LogLastTrain`. Confirm `Spawned zombie. Alive N ...`
   climbs past 6 toward the `MaximumAlive` cap of 24 (about 38s at the current
   spawn interval), and record the game thread ms with 24+ alive over 10s. Pass
   is the game thread holding near 16.6 ms. Then revert index 0 to 6 and save.

5. **Fix the corridor stall edge case.** The 2026-09-06 run found a single zombie
   a short distance outside `AttackRange` (about 19 units) can sit frozen at zero
   velocity for 20+ seconds while `UpdateStallRecovery`'s nudge never fires. Check
   the order of the gates in `UpdateStallRecovery`, the ground-speed sample it
   reads, and whether `StallTimer` is reset before `StallGraceSeconds` can
   accumulate. Bounded C++ change to `LTZombieCharacter.cpp`, compile, re-verify
   in PIE with 6+ zombies queued.

6. **Strip the diagnostic logs.** Remove the `// DIAGNOSTIC` `LT_LOG` lines from
   `TryAttack` and `TrySpawnOne` if they are still present. Keep the deliberate
   `LT_LOG(Verbose, "Spawned zombie. Alive %d ...")` in `TrySpawnOne` (it is kept
   for the intermittent-spawn investigation). Keep the `BlueprintReadOnly` field
   exposure on the zombie. Recompile.

7. **B2 wall-buy PIE acceptance.** Needs an interactive pass (mouse plus WASD in a
   real PIE session; the NeoStack bridge cannot aim the first-person camera). Walk
   to the `GreyboxTest_WallBuy_SMG` plate: prompt fades in, `E` under 500 points
   no-ops, `E` with 500+ buys and swaps the weapon and flashes the points crimson,
   `E` again offers ammunition at 250. Then the vignette and regen checks from the
   B3 acceptance list.

8. **Canary Wharf spawn fall-through.** In `L_CanaryWharf_Greybox`, rounds now
   start but spawned zombies free-fall through the level. Check `RecastNavMesh-
   Default` coverage and floor collision under the `CW_SpawnPoint_*` actors,
   most likely the top clusters. Then re-run the horde smoke test.

9. **Mark Phase B done** in `docs/tasks/README.md` and update this file once the
   gate is measured and items 5 to 7 pass.

10. **C1 train PIE acceptance.** Editor work, no C++ needed. Make `BP_Train` from
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
  out of scope: `NotifyPlayerBoarded` is the seam, and C3
  has now wired travel to it.
  **Outstanding**: build a `BP_Train` from `ALTTrain` with a box mesh child, place
  it at the platform edge in `L_GreyboxTest` with `BoardingVolume` over the door
  aperture, put a `ULTStationHeat` component on the round manager, and run the
  12-point acceptance list in `phase-c1-train.md`. Drop the tuneables right down
  (`FirstTrainStopSeconds`, `TrainInterval`, `DwellDuration`) while testing so a
  cycle takes seconds, then restore 30 / 100 / 25.
- `ALTDepartureBoard` (`phase-c2-departure-board.md`): **C++ written, not
  compiled.** New files only, `Train/LTDepartureBoard.{h,cpp}`. It finds the
  level's `ALTTrain` on `BeginPlay` (or takes the `TrainOverride` set on the
  instance), polls the countdown getters on `Tick`, and fires
  `OnCountdownChanged(WholeSeconds, Phase)` only when the whole second moves, plus
  `OnPhaseChanged` forwarded from the train's delegate. `GetDisplaySeconds`,
  `GetDisplayPhase`, `IsBoardingOpen` and `HasTrain` are the `BlueprintPure`
  reads. `ALTTrain` was not touched. **Outstanding**: compile, then a
  `BP_DepartureBoard` with a text render child driven by `OnCountdownChanged`,
  placed near the `BP_Train` on the platform.
- Travel (`phase-c3-travel.md`): **C++ written, not compiled.**
  `Core/LTGameInstance.{h,cpp}` holds `FLTTravelPayload` (carried points, weapon,
  reserve, and the visited-station list) and `BeginStationTravel`, which stores
  the payload, fires `OnTravelStarted` for a Blueprint fade and calls `OpenLevel`.
  `ALTGameMode::NotifyPlayerBoarded` builds the payload and travels to
  `NextStationMap`; `RehydrateFromTravel` grants the carry on the far side. Two
  design choices: rounds restart at 1 on travel, and the reserve carries **to
  full** rather than to the exact count, because `ULTWeaponComponent` has no
  reserve setter (only `RefillAmmunition`). `CarriedReserve` is recorded in the
  payload for when a setter lands. The rehydrate runs on the first tick after
  `StartRun`, not inside it, because the pawn's components stamp their own
  starting values in their `BeginPlay` and that order against the game mode's is
  not guaranteed. `Config/DefaultEngine.ini` now sets
  `GameInstanceClass=/Script/LastTrain.LTGameInstance`. **Outstanding**: compile,
  then set `NextStationMap` on both stations' game modes and board a train. A
  station-select picker for a third station onwards replaces the single
  `NextStationMap` name; a disk save game is a separate later task.
- `ULTZombieTypeData` (`phase-c-zombie-types.md`): the data asset, `ApplyTypeData`
  on the zombie, the roster on `ALTRoundManager` (keep `ZombieClass` as the
  empty-roster fallback). Data entry of the five stat blocks is a separate
  Sonnet task once the mechanism lands. **This is the next real code task**, and
  `phase-c-special-rounds.md` stays blocked until it lands.

## Phase E, what has landed

- Downed and revive (`phase-e1-downed-revive.md`): **C++ written, not compiled.**
  Two files, `Player/LTPlayerCharacter.{h,cpp}`. Zero health now calls `Down()`,
  never the death path: the pawn stops dead (`DisableMovement`), `Move`,
  `StartFire`, `StartAim`, `Reload`, `Interact` and `StartSprint` no-op while
  down, `Look` stays free, and `NotifyPlayerDowned()` flips the run state.
  `BleedOutSeconds` (30) counts down in `Tick`; expiry fires
  `OnBleedOutExpired()` then the extracted `Die()`, which is the only route to
  `NotifyPlayerDied()` now. `bSoloAutoRevive` (on) stands the player back up
  after `SoloReviveDelaySeconds` (8) at `ReviveHealthFraction` (0.5) of
  `MaxHealth`. Damage while down is ignored so it cannot shorten the clock.
  Seams left open: `Revive()` is `BlueprintCallable` for a self-revive item or a
  co-op revive, `OnDowned` / `OnRevived` are the hooks for a last-stand weapon
  swap, and the downed screen treatment belongs to the HUD and art pass.
  **Outstanding**: compile, then the PIE pass. Take damage to zero and check the
  player goes down rather than dies, the camera still turns, and the auto-revive
  stands them up at half health after 8s; then set `bSoloAutoRevive` false and
  check the run ends after 30s with `OnDied` and run state `Dead`.

## Specs written and ready for a fresh session to pick up

Each of these is a bounded, self-contained spec with its own build command,
constraints, and acceptance list. They can be handed cold to a separate Claude
session (no context from the session that wrote them needed). Rough order:

1. `phase-c-zombie-types.md` - `ULTZombieTypeData`, `ApplyTypeData` on the
   zombie, the roster on `ALTRoundManager`. **Ready now**, and the only thing
   blocking special rounds.
2. `phase-c-special-rounds.md` - sprinter round every 5th, brute pair every 10th,
   as a small plan layer on `ALTRoundManager`. **BLOCKED on
   `phase-c-zombie-types.md`** landing first - it hooks into the type-data path
   that task establishes.

`phase-c2-departure-board.md`, `phase-c3-travel.md` and
`phase-e1-downed-revive.md` have all been executed as C++ and are waiting on a
local compile plus their editor and PIE steps, described above.

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

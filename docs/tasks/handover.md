# Handover - the resume point

Last updated 2026-09-07 evening. One place for where the project stands and what
is left. **Update it whenever a task finishes**, so a cold session can pick up
without re-reading the history.

## The one line

Phase A is done. Phase B is one measurement from signable. **Phase C is code
complete, compiled, and verified in PIE for everything except the screamer
scream trigger.** The C++ that was written in remote sessions has been compiled
locally and reviewed twice. NeoStack's editor pass on 2026-09-07 finished all
five milestones: reparented `BP_GameMode`, built the five zombie type assets,
the tintable material, `BP_Train` and `BP_DepartureBoard`, wired the roster and
`StationRoutes`, and verified station travel both directions with exact
carry-over. What is left is small: force one screamer spawn to verify the
scream, take a frame-rate reading in a focused or packaged build, and run the
remaining Phase B acceptance.

## State of play, `origin/main` at `2e0f59a`

| Layer | State |
|---|---|
| Phase C C++ | **Compiled clean under `-Werror`, all 5 CI gates green.** Train, departure board, station travel, five zombie types, special rounds, downed and revive. Reviewed in `phase-c-review-2026-09-07.md`. |
| Phase C editor assets | **Committed** (`2e0f59a`). `BP_GameMode` reparented to `ALTGameMode`, so the run state machine is now live in every level. Five `DA_Zombie_*` type assets, `M_Zombie_Tintable` plus five `MI_Zombie_*` instances, `BP_Train` boarding, `BP_DepartureBoard`, roster wired on both round managers, spawn points placed on both greybox maps. |
| Verified in PIE | Boarding end to end, downed then auto-revive, station heat 0 to 3 one step per departure, train full cycle with all ten hooks on schedule, types spawn with correct stats and silhouette, sprinter round at 5, brute pair at 10, armour plate absorbs 200 body damage. |
| Not verified | The screamer scream trigger (the data applies, but no natural round-12 spawn was reached to test the sight line, summon and cancel window). The Phase B crowd frame gate (the editor pins its tick to 3 fps while unfocused). |
| Station travel | **PASS both directions** 2026-09-07. `StationRoutes` wired on `BP_GameMode`, `CW_Train` and a heat component added to Canary Wharf. Boarded out of `L_GreyboxTest` with 1280 points / 240 reserve, arrived in Canary Wharf with the exact carry, rounds from 1, heat 0. Boarded back with 2060 / 240, same. |

## You can play the full loop right now

Open `L_GreyboxTest` or `L_CanaryWharf_Greybox` and press Play. Working: rounds,
five zombie types, special rounds at 5 and 10, the wall buy, points, the train
cycling with a live board prompt, boarding out to the other station carrying
points and weapon (rounds restart at 1, heat resets), downed plus auto-revive at
half health, station heat 0 to 3. The only thing not exercised in a run is the
screamer's scream (it does not spawn until round 12).

## Exact next actions, in order

Everything here is editor work or a packaged build. No `Source/` change is
expected; if one seems needed, stop and write it up as a spec rather than
editing C++ in this pass.

### 1. Verify the screamer scream (~20 min)

The screamer's data applies correctly (`Type SCREAMER`, `Behaviour SCREAM`, walk
speed 110.5) but the scream was never triggered because reaching round 12 by hand
was out of budget, and runtime spawning is not exposed to the editor Python API.

Force a natural spawn: on `DA_Zombie_Screamer`, temporarily set
`FirstRoundAvailable` from 12 to **1**, and drop `OpeningRoundCounts` on the
placed round manager to something small like `(3,3,3,3,3)`. Play, let a screamer
spawn, stand in its line of sight with a clear floor between you for more than
`ScreamLineOfSightSeconds` (2.0s). Expect:

- A `screamed after` log line, `OnScream` fires, `OnZombieScreamed` broadcasts.
- Four extra walkers spawn (`ScreamSummonCount` 4).
- If you kill the screamer within `ScreamCancelWindowSeconds` (0.5s) of the
  scream, the summon is cancelled (`OnZombieScreamed` broadcasts with count 0).

**Revert `FirstRoundAvailable` to 12 and `OpeningRoundCounts` to
`(6,8,10,12,14)` afterwards** and confirm by read-back. If the scream does not
fire with a natural spawn either, that is a `Source/` bug in
`ALTZombieCharacter::UpdateScream` / `TickScream` - write it up as a spec, do not
fix it in this pass. Per `CLAUDE.md` a bug that survives is Fable's.

### 2. Frame-rate reading for the Phase B gate

The gate is 60 fps with 24 to 40 zombies alive. It has never been measured
because every delta sample over the editor bridge returns exactly 0.33333s: the
editor throttles its tick to 3 fps while the window is unfocused, and every
bridge command runs unfocused.

Two ways to get a real number:

- **Focused editor**: play in a New Editor Window, click into it so it has
  focus, force a dense round (`OpeningRoundCounts` high, `BreatherSeconds` low),
  type `stat unit` in the console, read the ms on screen by eye. Revert the test
  values after.
- **Packaged build** (the honest number): `RunUAT BuildCookRun` for a Mac
  Development or Shipping build, both maps in the cook set. `Config/DefaultEngine.ini`
  pins `GameInstanceClass`, and `OpenLevel` by name means `L_CanaryWharf_Greybox`
  is referenced by no asset - **add both maps to the packaging map list** or the
  travel target will not cook. This is also the first real end-to-end test of a
  shipped artifact, which is the Sunday goal.

Record the number in `README.md` and here, then mark Phase B done if B2 and B3
also pass (below).

### 3. Run the acceptance lists that are still open

| What | Where | Status |
|---|---|---|
| Crowd frame gate, 24 to 40 at 60fps | `phase-b1-throttled-repath.md` | action 2 above |
| Wall buy prompt and purchase | `phase-b2-interaction.md` steps 3 to 7 | never done in PIE |
| HUD polish: hit marker, spread, prompt anchor | `phase-b3-feedback-widgets.md` | never reviewed |
| Train, 12 points | `phase-c1-train.md` | PASS 2026-09-07 |
| Live countdown on a sign | `phase-c2-departure-board.md` | PASS 2026-09-07 |
| Board and arrive with the carry | `phase-c3-travel.md` | PASS both directions 2026-09-07 |
| Zombie types, 11 points | `phase-c-zombie-types.md` | PASS except screamer, action 1 |
| Sprinters on 5 and 15, brutes on 10 and 20 | `phase-c-special-rounds.md` | PASS at 5 and 10 |
| Down, bleed out, auto revive | `phase-e1-downed-revive.md` | PASS 2026-09-07 |
| Corridor stall no longer freezes a zombie | open code item 1 | fixed in code, unverified in PIE |

Also fill **`StationDisplayNames`** on `BP_GameMode` (two entries, key by map
asset name: `L_GreyboxTest` -> `Greybox Test`, `L_CanaryWharf_Greybox` ->
`Canary Wharf`) so the HUD and board have a station label. The C++ stamps it on
BeginPlay; blank until this is filled. Not blocking anything, but visible.

### 4. Full playthrough

Once 1 to 3 pass: play a complete session on each map, boarding between them at
least once, to round 12+ so a screamer and the special rounds all appear. Note
the top three feel-breakers (pacing, readability, a zombie doing something
visibly wrong) and write them up. That list is the next work session's input.

## What has landed

### Phase A, done 2026-09-04

`L_GreyboxTest` plays end to end. A5 acceptance passed on every testable check.
Records: `phase-a4-editor-setup.md`, `phase-a5-acceptance.md`.

### Phase B, code complete, one measurement short

B1 throttled repath, B2 interaction and the first wall buy, B3 the HUD widget.
The zombie attack fix and the HUD health bar are verified in PIE. **Open**: the
crowd frame gate (action 2), the wall-buy flow in PIE, the HUD polish review.
The corridor stall edge case has a verdict and a fix as of 2026-09-07,
unverified in PIE: open code item 1.

### Phase C, code complete and compiled, editor pass done

- **C1 `ALTTrain`** (`phase-c1-train.md`): arrive, dwell, depart, away on the
  100s interval and 25s dwell, ten presentation hooks, the boarding interact,
  the heat increment on a not-boarded departure. `BP_Train` built and its
  12-point PIE list PASSED 2026-09-07.
- **C2 `ALTDepartureBoard`** (`phase-c2-departure-board.md`): finds the level's
  train or takes an instance override, polls the countdown getters, fires
  `OnCountdownChanged` only on a whole-second move, forwards `OnPhaseChanged`.
  `BP_DepartureBoard` built and the countdown tracked the train exactly in PIE.
- **C3 travel** (`phase-c3-travel.md`): `ULTGameInstance` carries points, weapon,
  reserve and the visited-station list across an `OpenLevel`;
  `NotifyPlayerBoarded` snapshots the carry and travels after
  `TravelDelaySeconds`; `RehydrateFromTravel` grants it on the far side. Uses
  `ULTPointsComponent::SetPoints` so the carry is not a HUD spend. Rounds restart
  at 1 on travel; reserve carries to full. Editor wiring done 2026-09-07 (StationRoutes).
- **Five zombie types** (`phase-c-zombie-types.md`): `ULTZombieTypeData` plus
  `ApplyTypeData`, the brute's front armour plate, the sprinter's lunge, the
  screamer's sight line and summon, the weighted roster with
  `FirstRoundAvailable`, per-type live caps and the heat-3 weight shift. Five
  `DA_Zombie_*` assets built from the `neostack.md` table, five `MI_Zombie_*`
  material instances, `BP_Zombie` swaps its instance on a 0.05s delay after
  BeginPlay off `GetZombieType()` (the delay is needed because `ApplyTypeData`
  runs after BeginPlay). PIE confirmed types, stats, silhouette, armour plate.
  **Screamer scream unverified: action 1.**
- **Special rounds** (`phase-c-special-rounds.md`): `FLTRoundPlan` decided once
  per round, a three-way branch in `TrySpawnOne`, `GetSpecialRoundTag` for a
  banner. Reviewed in `phase-c-review-2026-09-07.md`. Brute pair lands at ~30 and
  ~70 per cent through the round (canon section 6). Round 10 is a walker round
  plus the pair, not an all-sprinter horde, controlled by
  `bBruteRoundOverridesSprinterRound` (default set); clear it on
  `BP_RoundManager` to stack the two. Canon section 6 contradicts itself here and
  wants settling. PIE confirmed sprinters at 5, two brutes at 10.

### Phase E, started

**E1 downed and revive** (`phase-e1-downed-revive.md`): zero health calls
`Down()`, never the death path. Movement disabled, fire/aim/reload/interact/sprint
no-op while down, `Look` stays free, and the interaction sweep is disabled so no
stale prompt hangs on screen. `BleedOutSeconds` (30) runs in `Tick`; expiry fires
`OnBleedOutExpired` then `Die()`, now the only route to `NotifyPlayerDied`.
`bSoloAutoRevive` stands the player up at half health after 8s. Damage while down
is ignored. `Down()` is a no-op once the run state is `Boarded` or `Dead`. PIE
confirmed 2026-09-07. Seams open: `Revive()` for an item or co-op, `OnDowned`/
`OnRevived` for a last-stand weapon, the downed screen for the HUD and art pass.

Perks, the upgrade bench and lost property are untouched.

## Open code items

All eight from the 2026-09-07 review are closed in `main` and **compiled**.
Kept here for the record; only item 1 still needs a PIE check and item 5 is a
design call.

1. **Corridor stall edge case: fixed, unverified in PIE.** It was a gate-order
   bug: the repath in `Tick` re-issued the AI move 0.35s after
   `BeginStallRecovery` cancelled it, handing velocity back to path following.
   Plus `StallTimer` was wiped by any single jittery frame, and recovery exited
   on one jittery frame without the zombie moving. Now the repath is held while
   recovering, the timer decays, exit needs 40 units of ground closed, and
   `StallRecoverySeconds` (1.5) caps a shove. **Check in PIE**: a corridor queue
   should fan out, not freeze; no zombie sits still outside `AttackRange` for
   more than ~2s; "entering stall recovery" logs at most every couple of seconds
   per zombie, not every frame. If it survives, it goes to Fable.
2. `NavProjectionExtent` Z tightened 500 to 150, `NavProjectionWarnDistance`
   (200) logs which spawn point snapped and how far. Fixed.
3. Arrival no longer flashes the points HUD as a spend: `SetPoints` broadcasts a
   zero delta. Fixed.
4. Interaction prompts while downed: `SetInteractionEnabled(false)` on `Down`
   and `Die`, `true` on `Revive`. Fixed.
5. **Solo death is unreachable on the shipped defaults** (auto-revive at 8s vs a
   30s bleed-out, damage ignored while down, no down cap). Exactly what E1
   specified, but Phase E's own gate ("a full survival session start to death is
   possible") needs a revive item, a per-run cap, or `bSoloAutoRevive` cleared.
   Clearing it on `BP_PlayerCharacter` is the one-click version. **Design call.**
6. `ALTGameState::SetStationName` now has a caller: `ALTGameMode` stamps
   `StationDisplayNames` / `StationDisplayName` on BeginPlay. Fixed. **Editor
   follow-up: fill StationDisplayNames, action 3.**
7. `ALTTrain`'s class comment updated. Fixed.
8. `ALTPlayerCharacter::TakeDamage` subtracts what `Super::TakeDamage` returns,
   and a fully absorbed hit no longer restarts the regen delay. Fixed.

## Build, CI and toolchain

- **Compile:** `./tools/ci/compile.sh` (checks the external Xcode mount, then
  runs the engine `Build.sh` for `LastTrainEditor Mac Development`). Override the
  engine path with `LASTTRAIN_ENGINE_ROOT`. No hosted compile: Unreal is not on
  CI. An opt-in `compile` job runs the same script on a self-hosted macOS runner
  (labels `self-hosted, macOS, unreal`, repo variable `UNREAL_SELF_HOSTED` set
  to `true`); skipped until a runner is registered.
- **Five CI gates**, all run locally, on every push to `main`, `claude/**` and
  the phase branches, and on every PR: `check_hygiene.py`,
  `check_cpp_conventions.py` plus clang-format 20, `check_cpp_reflection.py`
  (the compiler stand-in: `GENERATED_BODY()`, `generated.h` include, TU includes
  its own header first, `LT_LOG`/`UE_LOG` format specifier vs argument
  agreement), `check_docs.py`, `check_content.py` (LFS pointer integrity).
- **External Xcode** on `/Volumes/DriveSohaib` must be mounted to compile.
  `xcode-select -p` must point inside it.
- **clang-format 20** governs `.h`/`.cpp`/`.cs`. If not on PATH:
  `python3 -m venv /tmp/cf-venv && /tmp/cf-venv/bin/pip install clang-format==20.1.0`.
- **Pending: move the engine to the external drive**, `drive-migration.md`. Do it
  with the editor fully closed. Frees ~43 GiB.
- **`LastTrain.uproject`** lists only `EnhancedInput`, `ModelingToolsEditorMode`
  and `CommonUI`. The editor rewrites the file on launch: it reformats it and
  re-adds plugin entries for whatever is installed locally (`NeoStackAI`,
  `ModelContextProtocol`, `AllToolsets`, `Terminal`, `EditorToolset`). **Check
  `git diff LastTrain.uproject` before committing after an editor session** and
  revert it if the only change is plugin re-additions. `CommonUI` must stay:
  NeoStack once enabled it as a dependency without its modules built, which
  SIGSEGV'd PIE.
- **Local tooling that is gitignored, not project code**: `Plugins/NeoStackAI/`,
  `.neostack/`, `.agents/`, `.mcp.json`, `.codex/`, `.cursor/`, `.gemini/`. The
  last four are MCP client configs the in-editor Claude Code terminal and the
  toolset plugins drop; they point at a local bridge.
- **`.uasset` and `.umap` are Git LFS.** Working-tree copies are raw binary
  locally (smudged); the committed blob must be a pointer. `check_content.py`
  enforces this. LFS objects upload on push.
- **Never mutate actors or assets while PIE is running.** Stop PIE with
  `playtest.stop` or Escape before any edit.
- **British spelling everywhere**, no em or en dashes in source or docs. The
  palette is `#16161C` charcoal, `#6C4C9C` violet, `#E0A030` sodium, `#B02030`
  crimson. No roundel, no Johnston, no official line diagram, no operator livery,
  no Call of Duty names.

## The docs a fresh session needs

- **This file** - the resume point.
- `neostack.md` - every editor task in detail, including the full zombie stat
  table and the `BP_GameMode` travel section.
- `phase-c-review-2026-09-07.md` - what the review pass changed and why.
- `neostack-run-2026-09-07.md` - NeoStack's own run logs for the day, milestone
  by milestone, with the PIE evidence.
- `docs/design/gameplay-canon.md` - the settled design, coded values marked
  authoritative.
- `CLAUDE.md` - conventions, module layout, legal constraints, the model split.

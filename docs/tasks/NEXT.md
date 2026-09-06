# NEXT - resume point for a fresh context window

Last updated 2026-09-05. Update this whenever you finish a task so a cold
session can pick up without re reading the whole history.

## The one line

Phase A is DONE and pushed. Phase B code (B1 throttled repath, B2 interaction,
B3 HUD) is all landed. Phase B **acceptance is not signed off yet**: two PIE
findings are still open (zombie attack lands nothing, round 1 spawns plateau at
6). Diagnostic instrumentation is compiled and waiting for one more PIE pass.
After that clears, Phase B is done and Phase C (the train, station heat, five
zombie types, the departure board) begins as C++ work.

## What just happened (2026-09-05)

- A long autonomous NeoStack run worked the checklist in
  `docs/tasks/neostack-run-2026-09-05.md`. Results:
  - Section 1 (verify the two fixes): only 1.1 passed. 1.2 to 1.6 BLOCKED.
  - Section 2 (B1 crowd frame check): 2.1 and 2.4 passed (count set to 30, then
    reverted). 2.2 and 2.3 BLOCKED, spawns never climbed past 6.
  - Section 3 (Canary Wharf grey box blockout, Phase D pulled forward): 17 of
    18 items PASS. The whole blockout is built in
    `Content/LastTrain/Maps/L_CanaryWharf_Greybox.umap`. Only 3.14 is left, the
    Level Blueprint `BeginPlay` wiring, which is a NeoStack API gap not a
    decision.
  - Section 4 wrap up complete. No crashes, no git run, all levels saved.
- A follow up NeoStack diagnosis chat pinned both blocked findings:
  - **1.2 zombie attack lands nothing.** Two independent problems.
    (a) `WBP_HUD` `HealthBar.Percent` has *zero* bindings, confirmed
    structurally, so the bar can never move regardless of damage. (b) 15+
    seconds of point blank contact produced no economy tick and no damage
    vignette, which points to the attack genuinely not firing, not just a
    display bug. Could not confirm from tooling because `Health`,
    `AttackCooldown`, `CurrentTarget` were plain private members with no
    reflection.
  - **2.2 spawn plateau.** The placed `GreyboxTest_RoundManager` instance in
    `L_GreyboxTest` carries a per instance `OpeningRoundCounts` override still
    at `(6,8,10,12,14)`. NeoStack edited the asset, not the instance, so PIE
    kept spawning 6. `MaximumAlive = 24` is correct and intended. Not a code
    bug.
- This agent then, without the editor:
  - Exposed `Health` and `AttackCooldown` on `ALTZombieCharacter` as
    `VisibleInstanceOnly, BlueprintReadOnly, Transient`, and `CurrentTarget`
    the same with `AllowPrivateAccess`. So the attack state is now inspectable
    in PIE.
  - Added temporary `LT_LOG` diagnostics to `ALTZombieCharacter::TryAttack`
    (logs every early out and every `ApplyDamage` call) and to
    `ALTRoundManager::TrySpawnOne` (logs alive count, pending, cap, round on
    each spawn). Both marked `// DIAGNOSTIC, remove after ... confirmed`.
  - Compiled clean with the batch build. `LTZombieCharacter.cpp` and
    `LTRoundManager.cpp` rebuilt and relinked. **The running editor needs a
    restart or Live Coding to pick up the new binary.**
  - Noted the RVO fix supersedes the "do not change the movement setup" line in
    `docs/tasks/phase-b1-throttled-repath.md`.
- Uncommitted before this session: the two earlier fixes
  (`LTZombieCharacter.cpp` RVO 90 to 45 and acceptance `0.75f` to `0.5f`,
  `WBP_HUD.uasset` Bug 1 branch rewire), plus the new docs and the new map.

## Exact next actions

1. **Restart the editor** so it loads the recompiled binary. Fresh NeoStack
   chat after (its `execute_script` connector goes stale on every restart).
2. **Fix the round manager instance override.** In `L_GreyboxTest`, select
   `GreyboxTest_RoundManager` in the World Outliner, find `Opening Round
   Counts` in Details, hit the yellow reset arrow so it inherits the asset. For
   the B1 test, set index 0 to 30 *on the instance*. This unblocks 2.2 and 2.3.
3. **PIE `L_GreyboxTest`, watch the Output Log.**
   - Filter for `LogLastTrain`. Confirm `Spawned zombie. Alive N ...` lines
     climb past 6 toward the `MaximumAlive` cap of 24. That closes the B1
     crowd check once `stat unit` holds near 60fps at 24+ alive.
   - Let a zombie reach the player and stand in contact. Watch for
     `TryAttack firing ApplyDamage ...` lines. If they appear and the player
     health does not drop, the bug is downstream of `ApplyDamage`. If they do
     not appear, read the early out lines to see which gate is failing
     (`cooldown`, `target null`, or `out of range`), and read the zombie's now
     visible `AttackCooldown` and `CurrentTarget` in the Details panel while
     PIE is paused.
4. **Act on what the log shows.** Likely one of:
   - Attack fires, health drops, only the HUD bar is dead. Then it is a pure
     `WBP_HUD` fix, bind `HealthBar.Percent` to `GetHealthFraction()` or drive
     it from `OnHealthChanged`. NeoStack territory.
   - Attack never fires. Then fix `ALTZombieCharacter` in C++ (compile), most
     likely the repath acceptance still parks the zombie just outside
     `AttackRange`, or `CurrentTarget` is being lost.
5. **Strip the diagnostics.** Remove the `// DIAGNOSTIC` `LT_LOG` lines from
   `TryAttack` and `TrySpawnOne`. Keep the `BlueprintReadOnly` field exposure,
   it is worth keeping. Recompile.
6. **Re run B2/B3 acceptance** (checklist 1.2 to 1.6): attack plus health
   drain, regen refills, vignette on real hits, interaction prompt in and out,
   wall buy under 500 no ops then buys then re buys ammo at 250. Use direct
   editor input, not simulated, the simulated input was unreliable all run.
7. **Wire the Level Blueprint in `L_CanaryWharf_Greybox`** (checklist item
   3.14): `Event BeginPlay`, `Get All Actors Of Class BP_RoundManager`, index
   0, `BeginRounds`. Then place one `BP_RoundManager` in that map if 3.14 did
   not. Re run the PIE smoke test as a real horde playtest, only composition
   and no fall through are proven so far.
8. **Commit.** When 3 to 6 pass: the two earlier fixes, the new C++ field
   exposure (diagnostics stripped), the `WBP_HUD` binding fix, the new docs
   (`canary-wharf-grid.md`, `neostack-run-2026-09-05.md`, `drive-migration.md`,
   `canary-wharf-research/`), and `L_CanaryWharf_Greybox.umap` via LFS. Confirm
   before pushing.
9. **Mark Phase B done** in `docs/tasks/README.md` and update the one line
   here. Then Phase C begins.

## Phase C, the next real code work

All C++, no NeoStack dependency. Numbers from `docs/brief-v2.md`.

**Rolling stock / line identity: RESOLVED.** The fictional line is a
fictionalised Crossrail-scale line (main-line loading gauge, modelled on the
Elizabeth line, NOT a deep-level tube). The train is a Class 345 "Aventra"
silhouette: walk-through, 9-car ~205 m, near-vertical box sides, curved
wraparound cab front, 3 plug doors per side per car. This supersedes the
earlier "1996 Stock deep-tube vs Class 345" open question (open-questions 4.11 /
16.10). Details: `docs/brief-v3-unreal.md` Part 1,
`docs/reference/canary-wharf-research/rolling-stock.md`.

- `ALTTrain` actor: 100s arrival interval, 25s dwell, doors open and close on
  the dwell, boarding the train is the optional escape that ends the run.
- Station heat component: staying in the station raises heat over time,
  boarding releases it. Heat drives spawn pressure or roster.
- Five zombie types: either five `ALTZombieCharacter` subclasses or a data
  driven variant on a `LTZombieData` asset. Decide the approach first.
- Departure board actor showing the countdown to the next train, tied to
  `ALTTrain`.

Phase D grey box Canary Wharf was pulled forward and is mostly built already
(see above), so Phase C has a second arena to test in beyond `L_GreyboxTest`.

## Editor tooling note

- `Plugins/NeoStackAI/` is a third party Unreal editor plugin that lets a
  NeoStack agent build `.uasset` and `.umap` through `execute_script`. It is
  gitignored (`Plugins/NeoStackAI/`, `.neostack/`, `.agents/`), not ours to
  redistribute. Agent brief is `docs/tasks/neostack-build.md`.
- **NeoStack trial expires around 2026-09-07.** Spend remaining chats on the
  PIE verification above and the `L_CanaryWharf_Greybox` Level Blueprint wiring,
  not on new building.
- NeoStack cannot do: custom trace and object channels, other bespoke Project
  Settings UI (`write_config` silently no ops), and it could not reach the
  Level Blueprint EventGraph in `L_CanaryWharf_Greybox` (API gap, item 3.14
  needs a human).
- The `Weapon` trace channel (slot 1, `ECC_GameTraceChannel1`, response Ignore)
  was created by hand and is in `Config/DefaultEngine.ini`. Do not expect
  NeoStack to recreate it.
- `CommonUI` is in `LastTrain.uproject` `Plugins` explicitly because NeoStack
  enabled it as a dependency without its modules built, which SIGSEGV'd PIE in
  `UCommonInputSubsystem::Initialize`. Do not remove that entry.
- NeoStack's `execute_script` connector goes stale on every editor restart.
  Fix: restart the editor fully, start a fresh NeoStack chat, first message
  "list your tools".
- NeoStack's simulated movement input (`playtest_key`, `playtest_axis`)
  intermittently stops producing player displacement mid session, reproduced on
  clean sessions with no zombies. Treat it as unreliable, drive PIE by hand.

## Build and toolchain

- Editor build, run after any C++ change:
  `"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"`
- External Xcode on `/Volumes/DriveSohaib` must be mounted. `xcode-select -p`
  points at `/Volumes/DriveSohaib/Applications/Xcode.app/Contents/Developer`.
- No CI compile. Compile locally after every C++ change, keep changes small.
- **Pending: engine move to the external drive.** See
  `docs/tasks/drive-migration.md`. Do it when NeoStack is idle and the editor
  is fully closed, not mid run. Frees ~43 GiB on the internal SSD.

## Content in git

Our own assets under `Content/LastTrain/` are committed via Git LFS per
`.gitattributes`. Imported third party packs are gitignored by their landing
folders (`Content/ThirdPerson/`, `Content/Characters/`, `Content/Megascans/`,
`Content/Lyra/`, etc.) because UE-Only Content and Fab Standard licences forbid
re hosting raw assets in a public repo. Vetted free asset list and fetch
instructions: `docs/reference/free-assets.md`. Canary Wharf reference research
(architecture, materials, signage, rolling stock, licence notes):
`docs/reference/canary-wharf-research/`. If a pack imports to a new folder, add
it to `.gitignore` before committing.

## Phase plan in brief

A foundation and first playable grey box → **B engine core hardening (repath,
interaction, hit markers), acceptance in progress** → C rounds, five zombie
types, the train, the departure board, station heat → D grey box Canary Wharf
(blockout mostly built early) → E perks, bench, lost property, revive → F art
pass, Fable led, against the reference frame → G audio, HUD, second station,
balance. Full table in `docs/tasks/README.md`.

Design numbers (round loop, 100s train interval, 25s dwell, station heat, the
five zombie types, economy) come from `docs/brief-v2.md`. Engine and structure
come from `docs/brief-v3-unreal.md`.

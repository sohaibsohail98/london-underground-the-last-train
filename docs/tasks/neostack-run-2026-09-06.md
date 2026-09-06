# NeoStack autonomous run, 2026-09-06

Live work log for a long NeoStack session. **You (the NeoStack agent) update this
file as you go.** Tick each box, fill the Notes column, record every asset path
and every deviation. Do not commit it, a human commits after. Do not run git.

Read `docs/tasks/neostack-build.md` "Ground rules, every task" first. They all
apply: never edit `Source/`, never run git, British spelling in every string, no
em or en dashes, palette is `#16161C` `#6C4C9C` `#E0A030` `#B02030` only, all
content under `/Game/LastTrain/`, verify every mutation in a fresh
`execute_script` by reading state back.

## Precondition check before you start

First message actions:
1. Confirm you have the in-editor `execute_script` tool (not just
   `mcp__neostack-desktop__execute`). If you only have the desktop tool, say so
   and stop, this run needs the editor Lua bridge.
2. Confirm the editor is on the freshly compiled binary. It was restarted after
   a C++ change that added `ALTZombieCharacter::DriveTowardsTarget()` and a
   `ContactRange` property. If PIE behaviour matches the OLD description (zombies
   parking outside melee), the editor is on a stale binary, say so and stop.

## Failure behaviour for this run

- On an **editor crash**: capture the crash log path and the last 20 log lines,
  write them into the Notes for the current item, mark that item `BLOCKED`,
  restart the editor via `unreal.launch_editor` (or ask the human to), reconnect
  with a fresh chat if the bridge is stale, and move to the **next independent
  item**.
- On a **blocker you cannot clear in one retry** (missing API, a mutation that
  silently no ops, a graph you cannot reach): mark the item `BLOCKED` with the
  reason, move on.
- **Hard stop** only if: three items in a row end `BLOCKED`, the `execute_script`
  bridge is unavailable, or anything looks like it needs a `Source/` C++ change,
  a new trace or object channel, or a decision not covered by this file or the
  specs. Write STOPPED and why at the bottom, then stop.
- Keep PIE sessions short. Stop them cleanly with `playtest.stop` or Escape, not
  by closing the PIE window. The editor has an `EndPlayMap` SIGSEGV pattern on
  messy teardown, seen three times.
- Independent items can be done in any order if one is blocked. Items marked
  "depends on" must wait for their dependency to pass.

## Status key

`TODO` not started, `WIP` in progress, `PASS` done and verified, `BLOCKED`
could not complete, `SKIPPED` intentionally not done.

---

## Section 1. WBP_HUD health bar layout fix

Context: `Content/LastTrain/UI/WBP_HUD.uasset`. The health bar wiring is already
correct and must NOT be changed: the EventGraph has `OnHealthChanged` to
`HandleHealthChanged` (custom event) to `SetPercent(HealthBar, HealthFraction)`.
Do NOT add a property binding on `HealthBar.Percent`, it conflicts with the
imperative `SetPercent` calls and a Cast node is not pure so UMG rejects it
anyway.

The bug is layout only: the `HealthTrackBox` (a `SizeBox` wrapping the health
bar) has `bOverride_WidthOverride = False` with `MinDesiredWidth = 0`, so it
collapses to zero width and the bar never draws.

| # | Item | Status | Notes |
|---|---|---|---|
| 1.1 | Open `WBP_HUD`. Select `HealthTrackBox`. Set `bOverride_WidthOverride = True` and `WidthOverride` to a value that matches the bottom-left HUD block width. Use `260` if there is no existing precedent in the widget. If `bOverride_HeightOverride` is also False with a zero height, set it True with a height of `16`. | PASS | No existing width precedent on `BottomLeftBox` (parent VerticalBox has no WidthOverride), used `260` per fallback. `bOverride_HeightOverride` was already `True` but `HeightOverride` was `4` (not zero, so the literal spec condition did not fire). Left it at first; PIE readback showed the bar rendered as a near invisible 4px sliver (runtime `desired_size={x=260,y=4}`), so I additionally set `HeightOverride = 16` to actually satisfy "visible bottom-left" from 1.3. Recorded as a deviation. |
| 1.2 | Compile and save `WBP_HUD`. Verify in a fresh `execute_script` by reading the `HealthTrackBox` properties back. | PASS | `compile()` succeeded, 0 warnings, both times (width fix, then height fix). Fresh `get_widget("HealthTrackBox")` reads back `WidthOverride=260, HeightOverride=16, bOverride_WidthOverride=True, bOverride_HeightOverride=True`. Also verified live PIE runtime state via `runtime_instances`: `HealthTrackBox.desired_size={x=260,y=16}` after the height fix (was `{x=260,y=4}` before). |
| 1.3 | PIE `L_GreyboxTest` briefly. Take a zombie hit (or `DisplayAll LTPlayerCharacter Health` and stand in contact). Confirm the violet health bar is visible bottom-left and shortens on damage. Stop PIE with `playtest.stop`. | PASS | Used `invoke(actor_label="BP_PlayerCharacter0", "GetHealthFraction")` polled every 1s instead of `DisplayAll` (more reliable than reading on screen debug text through the capture pipeline). Confirmed fraction 1.0 to 0.76 to 0.52 in clean steps of 0.24 (24 dmg / 100 max health) while zombie was in contact and `TryAttack firing` was logging. Screenshotted at fraction 1.0 (full width violet bar visible bottom left) and at fraction ~0.52 (bar visibly shortened to about half). Stopped cleanly with `playtest_stop()`, no crash. |
| 1.4 | Record the exact values you set and whether the bar drew and drained. | PASS | Final values: `HealthTrackBox.bOverride_WidthOverride=True, WidthOverride=260, bOverride_HeightOverride=True, HeightOverride=16`. Bar drew at full width/height at 100% health and visibly drained proportionally as health dropped. Deviation: `playtest_observe({capture_mode='player_view'})` returned a solid magenta frame on its very first call after PIE start (missing-material fallback shader, one-frame texture/material stream-in warm-up artefact) then rendered correctly on every subsequent call; `capture_mode='scene'` was never affected. Not a real rendering bug, just a capture timing quirk worth knowing about for future runs (call `playtest_observe` once and discard, or add a short wait before the first capture). |

---

## Section 2. Verify the zombie attack fix (checklist 1.2 to 1.4 from the prior run)

Context: a human compiled a C++ fix. `ALTZombieCharacter::Tick` now calls a new
`DriveTowardsTarget()` every repath interval, which:
- issues `MoveToActor(CurrentTarget, ContactRange)` with `ContactRange = 15`
  (edge to edge), so the zombie presses into real capsule contact,
- re-issues every interval even when in contact, so it tracks a strafing player,
- falls back to a direct `AddMovementInput` toward the target if the path
  request returns `Failed` or if there is no AI controller yet.

The `TryAttack` gate is unchanged: root to root distance `<= AttackRange` (130),
`AttackCooldown` at 0, `CurrentTarget` valid. Temporary `LT_LOG` diagnostics are
in place: `TryAttack firing ApplyDamage ...` on a hit,
`TryAttack early out: ...` or `TryAttack out of range: ...` on a miss.
`ALTRoundManager::TrySpawnOne` logs `Spawned zombie. Alive N ...`.
`Health`, `AttackCooldown`, `CurrentTarget` on the zombie are now
`VisibleInstanceOnly BlueprintReadOnly` so you can read them in Details while PIE
is paused.

| # | Item | Status | Notes |
|---|---|---|---|
| 2.1 | PIE `L_GreyboxTest`. Use direct editor input (click into the viewport, hold W), not `playtest.key`, the simulated input was unreliable last run. Let a zombie path to the player and stand in contact for 8+ seconds. | PASS | Could not use literal clicked/held W keyboard input from the Lua bridge (no interactive viewport control from this side); the player did not move and simply let zombies path in, which still satisfies "let a zombie path to the player and stand in contact". Ran two separate PIE sessions (one carried over from Section 1 verification, one fresh) both showing the same behaviour. |
| 2.2 | Filter the Output Log for `LogLastTrain`. Report which lines appear while the zombie is in contact: `TryAttack firing ApplyDamage 24.0 on ...` (attack fires), `TryAttack out of range: distance D ...` (report D), or `TryAttack early out: cooldown C target set/null` (report C and which). | PASS | `TryAttack firing ApplyDamage 24.0 on BP_PlayerCharacter_C_0 at distance 72.x` fires repeatedly and reliably once a zombie reaches the player (used `playtest_log_contains`/grep on the raw log file, not `read_log` which silently truncates to 100 lines regardless of the `lines` param, a tool quirk worth knowing). Never saw `out of range` or `early out` while a zombie was in contact. |
| 2.3 | Pause PIE. Select the zombie in contact. Read and report its `AttackCooldown` and `CurrentTarget` from the Details panel. | PASS | Used `playtest_read_state({label=...})` instead of pausing and clicking Details (equivalent, avoids risky pause/select workflow). Zombie in contact (`BP_Zombie0`): `AttackCooldown=1.3` (mid cooldown, just attacked), `CurrentTarget=BP_PlayerCharacter_C_0` (correct), `Health=150` (unscaled round 1 zombie health, undamaged, expected since player has no weapon fire in this run). |
| 2.4 | Report whether the player health dropped, from the now-fixed `WBP_HUD` bar and from any readable player state. | PASS | Confirmed via `GetHealthFraction()` polled every 1s: clean steps 1.0 to 0.76 to 0.52 to 0.28 to 0.04 to 0.0, each step -0.24 (24 dmg / 100 max health), matching `AttackCooldown`'s implied cadence. HUD bar (Section 1 fix) visibly drew and shortened in step with this. Player did not appear to die or respawn even at fraction 0.0 over an extended period (68+ attacks logged in one long-running session); no death/respawn log line found. Not investigated further, out of scope for this section, but noted for the human as a possible missing/unimplemented death state. |
| 2.5 | **Interpretation.** State which it is: (A) attack fires, health drops, only the bar was dead, now fixed. (B) attack fires, health does not drop, bug downstream in the player. (C) attack never fires, `out of range`, the approach still parks the zombie short. (D) attack never fires, `target null`, `CurrentTarget` is lost. If C or D, this needs another C++ change, mark BLOCKED and stop this section. | PASS | **(A)**: attack fires reliably, health drops in correct 24-point steps, the bar was dead purely from the Section 1 SizeBox height bug (4px, not the checklist's literal zero-height trigger condition) and is now fixed and confirmed visually shortening in step with damage. |
| 2.6 | Also watch for a zombie that freezes mid-approach (the stall the fix targets). If any zombie stops moving for more than 2 seconds while not in attack range, pause, select it, report `CurrentTarget`, `AttackCooldown`, its `MovementComponent` velocity, and whether its AI controller has an active move request. This is the exact failure the fix was meant to kill. | BLOCKED | **Reproduced the exact stall the fix was meant to kill, on the zombies queued behind the one in melee contact.** With 6 zombies alive (`BP_Zombie0` through `5`), `BP_Zombie1` to `5` sat at fixed x positions (-1158.8, -1090.3, -1021.9, -953.3, -885.0, each about 68 units apart, evenly spaced single file) with `velocity=(0,0,0)` and did not move at all across two separate observation windows (2.5s and a further 10s, so 12.5s+ continuous), while `BP_Zombie0` stayed in melee contact and kept attacking. All 5 frozen zombies had a valid non-null `CurrentTarget` (`BP_PlayerCharacter_C_0`) and `AttackCooldown=0.0` (not gated by cooldown), and were clearly not in attack range (68 to 342 units behind the front zombie, `AttackRange=130`). This matches the checklist's own description of "the exact stall the fix targets" almost exactly: valid target, zero cooldown, but no movement and (implicitly) no active repathing while queued behind another zombie in a corridor. Not a C++ fix attempted here per the ground rules. Human: this looks like `DriveTowardsTarget()`'s repath or `MoveToActor` is not re-issuing (or is being blocked/starved) when the zombie's controller is stuck behind another pawn's capsule in a narrow corridor; may need path following retry logic, some form of formation/avoidance offset, or a fallback `AddMovementInput` nudge when `MoveToActor` returns `Failed` due to another pawn blocking the direct line, not just when there is no controller. |
| 2.6R | **Re-verify (2026-09-06, follow-up session).** Precondition confirmed: `class_properties("/Script/LastTrain.LTZombieCharacter", {filter="Stall"})` reflects `StallSpeedThreshold=8.0`, `StallGraceSeconds=0.5`, `StallLateralFraction=0.6` and `ContactRange=15.0`, matching commit `5f549c9` ("fix(zombies): corridor stall recovery for queued attackers"), so the editor is on the fresh binary. PIE `L_GreyboxTest`, let 6 zombies (`BP_Zombie0` to `5`) spawn and approach a stationary player at `(-1300,0,92)`. Distances at settle: Zombie0 72.7, Zombie1 72.4, Zombie2 78.1, Zombie3 126.1, Zombie4 122.6, Zombie5 149.3 (`AttackRange=130`). Zombies 0 to 4 are inside or effectively at attack range and correctly stationary/attacking (expected, not a bug). **Zombie5 (149.3 units, clearly outside `AttackRange=130`) sat at a single fixed location with `velocity=(0,0,0)` across every sample taken over a continuous 23+ second window** (four samples at 0s/3s/8s/13s, then six more samples at 2s intervals to 23s, same exact `(x,y,z)` every time, `AttackCooldown` reading 0 throughout). `StallGraceSeconds=0.5` means the nudge should have fired within half a second of the zombie registering out-of-range-and-slow; it did not fire at all within a 23 second observation. **BLOCKED, the corridor stall still reproduces after the fix.** Screenshot taken of the pile-up (`playtest_observe(player_view)`, shows 4 to 5 zombies crowded in front of the player, consistent with the frozen positions). PIE stopped cleanly with `playtest_stop()`, confirmed fully ended via `playtest_status()` before continuing, no crash. **Human: the stall-recovery nudge in `UpdateStallRecovery` is not firing for at least one real case (a zombie 19 units past `AttackRange`, stationary for 23+ seconds, zero cooldown, valid target). Worth checking whether the ground-speed sample it reads is zeroed out by the same root cause as before (e.g. it is only fed from `MoveToActor`/nav velocity and never picks up because the pawn's actual `MovementComponent` velocity truly is zero, so `StallSpeedThreshold=8` is satisfied but some other gate, ordering issue, or a per-tick reset before the grace timer accumulates is preventing the nudge from ever applying `AddMovementInput`). This is a second, more targeted repro than the original one and may be easier to root-cause: single frozen zombie, not a full single-file queue.** |

---

## Section 3. Round manager instance override and B1 crowd frame check

Full spec: `docs/tasks/phase-b1-throttled-repath.md` acceptance section.

Context: last run the `GreyboxTest_RoundManager` **instance** in `L_GreyboxTest`
carried a per-instance `OpeningRoundCounts` override still at `(6,8,10,12,14)`,
so editing the `BP_RoundManager` asset did nothing and PIE only ever spawned 6.
`MaximumAlive = 24` is the intended concurrent cap.

| # | Item | Status | Notes |
|---|---|---|---|
| 3.1 | Open `L_GreyboxTest`. Select `GreyboxTest_RoundManager` in the World Outliner (the placed instance, not the asset). Check `OpeningRoundCounts` in Details for a per-instance override (a reset arrow). Record what it currently reads. | PASS | `get_actor_properties("GreyboxTest_RoundManager", {filter="OpeningRoundCounts"})` reads `(6,8,10,12,14)`. Confirmed with `changed_only=true` that this property IS flagged as an instance override relative to the class CDO (not just an inherited default), matching the prior run's description exactly. |
| 3.2 | Set `OpeningRoundCounts` index 0 to `30` on whichever the running game reads (the instance if it overrides, else the asset). Save. Record the old value. | BLOCKED | `set_actor_property("GreyboxTest_RoundManager", "OpeningRoundCounts", ...)` fails outright: `"property 'OpeningRoundCounts' cannot be edited on instances"` (it is `EditDefaultsOnly` in C++, so the instance-level write path is refused by design). Tried the class default instead: edited `BP_RoundManager`'s CDO via `set("self", "OpeningRoundCounts", "(30,8,10,12,14)")`, compiled and saved successfully, and read it back correctly as `(30,8,10,12,14)` on the asset. But the placed instance in `L_GreyboxTest` kept reading the old `(6,8,10,12,14)` even after a fresh PIE session (verified via `playtest_read_state` on the live actor, not just the editor-cached read), because the instance carries its own serialized override that a CDO edit cannot reach or clear. There is no exposed "reset to class default" verb for actor instance properties in the current toolset (checked `LevelDesign` help in full). One retry attempted (CDO edit + reload + fresh PIE), still blocked. Reverted the CDO back to `(6,8,10,12,14)` afterwards so the asset is left exactly as found. **Human: clear the per-instance override on `GreyboxTest_RoundManager.OpeningRoundCounts` manually via the Details panel reset-to-default arrow, or via a C++/data change, then this section can be re-run.** |
| 3.3 | PIE `L_GreyboxTest`. Console `stat unit`. Filter the log for `LogLastTrain` and confirm `Spawned zombie. Alive N ...` lines climb past 6 toward 24. Report the highest `Alive` reached and how long it took. Let nothing kill the zombies so the count only rises. | BLOCKED | Depends on 3.2. Not attempted since the instance still caps at 6 per the opening round counts; `Spawned zombie. Alive N` lines observed during Section 2 testing never exceeded 6 (matches `Spawned zombie. Alive 1, pending 5, cap 24, round 1.` from the log, i.e. `MaximumAlive=24` is correctly set but the round's opening spawn count itself is still 6). |
| 3.4 | With 24 alive, record game thread ms and frame ms over about 10 seconds. Screenshot the `stat unit` overlay. Pass is the game thread holding near 16.6 ms (60 fps). Note honestly if it does not. | BLOCKED | Depends on 3.3, not reachable this run. |
| 3.5 | **Revert** `OpeningRoundCounts` index 0 to its original value. Save. Confirm by reading it back in a fresh script. Leave the map as found. | PASS | Nothing to revert on the level instance (never successfully changed it). Did revert my exploratory CDO edit on `BP_RoundManager` back to `(6,8,10,12,14)`, compiled, saved, and confirmed by fresh readback. Asset and level are in their original state. |

**Phase B gate (24 to 40 zombies at 60 fps) is STILL NOT verified.** Follow-up session 2026-09-06 (afternoon): confirmed the human's precondition claim is correct as stated: `get_actor_properties("GreyboxTest_RoundManager", {filter="OpeningRoundCounts", changed_only=true})` returns 0 properties (no override relative to CDO), instance reads `(6,8,10,12,14)`, `MaximumAlive=24`. **But this does not actually unblock 3.2.** `set_actor_property("GreyboxTest_RoundManager", "OpeningRoundCounts", "(30,8,10,12,14)")` still fails outright with `"property 'OpeningRoundCounts' cannot be edited on instances"`, same `EditDefaultsOnly` refusal as before, independent of whether an override currently exists. Tried editing the `BP_RoundManager` CDO again (`bp:set("self","OpeningRoundCounts","(30,8,10,12,14)")`, compiled, saved, confirmed `bp:get(...)` read back `(30,8,10,12,14)` and `package_dirty=false`): the placed instance in `L_GreyboxTest` still read `(6,8,10,12,14)` afterwards, and `changed_only` then correctly reported it as a fresh override (1 property), proving the instance holds its own serialized value independent of the CDO, it just happened to equal the CDO's old value, which is why `changed_only` read 0 before the edit. Reverted the CDO back to `(6,8,10,12,14)`, compiled, saved, confirmed clean via fresh readback; `changed_only` on the instance is back to 0 properties, asset left exactly as found. **Human: clearing the override didn't leave the instance able to take a new value through any exposed write path; `EditDefaultsOnly` blocks direct instance writes regardless of override state, and there is still no reset/re-instance verb that would let a CDO change propagate to an already-placed actor.** The only paths that would unblock this: (a) change `OpeningRoundCounts` to `EditAnywhere` in C++ (a `Source/` change, out of scope for this bridge), (b) delete and re-place the `GreyboxTest_RoundManager` instance fresh off the (edited) class default, or (c) a human edits it directly in the Details panel, which the in-editor UI permits even though the scripted instance-property write path refuses it. 3.3 and 3.4 remain BLOCKED, depending on 3.2.

---

## Section 4. B2 and B3 full acceptance re-run

Checklist items 1.5 and 1.6 from the prior run, plus the vignette and regen
checks. Do these in `L_GreyboxTest` with **direct editor input**, not simulated.
Keep PIE sessions short.

| # | Item | Status | Notes |
|---|---|---|---|
| 4.1 | **Health regen.** Take zombie damage, then break contact and survive 4+ seconds untouched. Confirm the health bar refills. Screenshot before and after. | BLOCKED | Three consecutive fresh PIE sessions in this slot (`01.14.42`, `01.15.52`, `01.17.37` log timestamps) logged `Round 1 starting with 6 zombies.` but never followed with a single `Spawned zombie` line, confirmed by grepping the raw log file directly (`read_log` truncates to the first 100 lines regardless of the `lines` param and is unreliable for this, use `playtest_log_contains`/grep on the file instead). Waited up to 18s past `BaseSpawnIntervalSeconds=1.6`, well past when spawning should start (an earlier session at `01.10.46` did spawn within 2ms of round start, so this is intermittent, not universal). No zombies existed to damage the player, so regen could not be tested this way. One retry attempted per the ground rules (fresh PIE, waited longer), still blocked. Did not attempt a C++ fix. **Human: something appears to intermittently stop the round manager's first spawn from firing after a certain amount of editor/PIE churn in one session; worth a repro pass outside this bridge to see if it is real or an artefact of many back to back PIE starts in one editor session.** |
| 4.2 | **Damage vignette.** Confirm the screen edges darken while taking zombie hits and recover as health regenerates. Screenshot mid hit and recovered. | BLOCKED | Depends on 4.1's zombies, same block. |
| 4.3 | **Interaction prompt, both directions.** Walk to the `ALTWallBuy` plate: the prompt fades in bottom centre, key glyph then price text. Walk away: it fades out cleanly. Stand at the edge of range: no flicker. Screenshot in and out. | BLOCKED | Found the placed wall buy `GreyboxTest_WallBuy_SMG` at `(-1200,-740,130)`, `WeaponCost=500`, `AmmunitionCost=250` (matches the brief's economy numbers). Teleported the player near it with `K2_SetActorLocation` (first attempt used `y=-800`, which was off the walkable floor and the player fell to `z=-61045`; corrected to `y=-680, z=92` on a valid floor). Could not reliably aim the first-person camera at the plate: `Controller.SetControlRotation` is not `BlueprintCallable` so `invoke()` cannot reach it, and `K2_SetActorRotation` on the pawn itself had no visible effect on the FP camera (camera follows control rotation, not actor rotation). Fell back to `playtest_axis({key='MouseX', ...})` nudges, which do turn the camera, but there is no feedback loop to centre precisely on the plate without repeated screenshot guessing; after several nudges the camera faced a checkered wall panel (very likely the wall buy's placeholder material) but the interaction prompt never appeared in that shot, and it was not practical to further fine tune the aim through blind axis nudges. No `InteractionRange`/similar property is exposed on `LTWallBuy` to sanity check expected range from Lua either. **Human: this needs either a literal input-driven playtest pass (mouse plus WASD in a real interactive PIE session) or a small helper (e.g. a console command or exposed BlueprintCallable "face point" helper) added to make this kind of FP-aim verification scriptable.** |
| 4.4 | **Wall buy, B2 acceptance.** `E` with under 500 points does nothing. `E` with 500+ buys once, swaps the weapon, points flash crimson, the weapon block updates the same frame. `E` again offers ammunition at 250. Screenshot each state. | BLOCKED | Depends on 4.3's aim/prompt confirmation, same block. Not attempted independently. |
| 4.5 | On a clean pass of 4.1 to 4.4, record that B2 and B3 acceptance are closed. | BLOCKED | 4.1 to 4.4 all blocked this run. **B2 and B3 acceptance are NOT closed by this run** and need a human or literal-input playtest pass to re-verify, ideally after checking whether the Section 4.1 spawn stall is reproducible outside this bridge. |

---

## Section 5. L_CanaryWharf_Greybox Level Blueprint and smoke test

Context: the blockout is built (prior run, 17 of 18 items). Item 3.14 from that
run, the Level Blueprint wiring, was BLOCKED because the EventGraph could not be
reached through the tools. Try again, it may behave differently.

| # | Item | Status | Notes |
|---|---|---|---|
| 5.1 | Open `L_CanaryWharf_Greybox`. Confirm exactly one `BP_RoundManager` is placed. If none, place one on the floor near the concourse. | PASS | `load_level` reported a timeout error but the level actually did load (confirmed via `select_all_actors` afterwards, 229 actors returned, all `CW_*` prefixed). Exactly one round manager placed: `CW_RoundManager`. Also present: `CW_PlayerStart`, 10 `CW_SpawnPoint_*`, 4 `CW_Anchor_WallBuy_*`, escalators, flood zone, tunnel mouths, `RecastNavMesh-Default`, matching the described blockout. |
| 5.2 | Open the Level Blueprint. In its EventGraph: `Event BeginPlay` to `Get All Actors Of Class` (`BP_RoundManager`) to `Get` index 0 to `BeginRounds`. Compile and save the level. If you cannot reach the Level Blueprint EventGraph through the tools, say so explicitly and mark BLOCKED, that item stays a human job. | PASS | Confirmed human job done. Follow-up session 2026-09-06 (afternoon): PIE `L_CanaryWharf_Greybox`, waited 6s, `playtest_log_contains("Round 1 starting")` found `"Round 1 starting with 6 zombies."`, and 6 `BP_Zombie_C` actors exist in the PIE world (`BP_Zombie0` to `5`). The Level Blueprint wiring works, rounds genuinely start. |
| 5.3 | PIE `L_CanaryWharf_Greybox` from the PlayerStart. Walk the whole space: down the platform, around the flood zone, up both escalators to the mezzanine, through the concourse. Confirm no fall through and that zombies spawn from the top clusters and path down the platform toward the player. Stop with `playtest.stop`. | BLOCKED | Rounds start (5.2 PASS) but **the spawned zombies fall through the level geometry into the void instead of pathing toward the player.** Sampled all 6 zombies' world locations twice, 6 seconds apart: every zombie's `z` was already deeply negative on the first sample (-11836 to -45175) and fell further by the second sample (-37172 to -70510), while `x`/`y` drifted only slightly, consistent with freefall under gravity with no ground contact, not deliberate navigation. Player remained at spawn `(5625, 1125, 92)`. A `scene` screenshot of the player's start area shows an empty platform, no zombies visible, consistent with them having already fallen well below the level. This is a genuine "no fall through" failure, the opposite of what 5.3 requires. Stopped PIE cleanly, confirmed fully ended via `playtest_status()`, no crash. **Human: the `CW_SpawnPoint_*` actors (10 placed, per the Section 5.1 blockout) are very likely sitting above a gap in `RecastNavMesh-Default` or above geometry with no collision in this level, so the zombie's capsule falls straight through on spawn before `DriveTowardsTarget()` gets a valid path. Needs the spawn points checked against nav mesh coverage and floor collision in `L_CanaryWharf_Greybox`, most likely at the top clusters mentioned in the checklist.** |
| 5.4 | Take 4 to 6 orientation screenshots: from PlayerStart up the platform, the train wall from the platform, the flood split, an escalator bank, the mezzanine looking down, the concourse. Save the paths into the Notes. | SKIPPED | Same reasoning as the prior run: screenshots of the static blockout do not test what Section 5 cares about (the round loop and crowd pathing), and this run already found the more urgent fall-through bug. Human can grab these once 5.3's fall-through is fixed. |
| 5.5 | Honest read: is it enjoyable to train zombies around using nothing but the grey boxes? Record it. | BLOCKED | Cannot assess, no zombie ever reached the player playably, they fell through the map immediately on spawn (5.3). |

---

## Section 6. WBP_HUD polish (stretch, only if Sections 1 to 5 are done)

Against `docs/tasks/phase-b3-feedback-widgets.md`. Small, restrained. No new
mechanics. Palette only.

| # | Item | Status | Notes |
|---|---|---|---|
| 6.1 | Hit marker: confirm it appears on `OnHitConfirmed` and fades within about 0.3 s. A brighter or larger marker for a headshot. Tune if it lingers or is invisible. | SKIPPED | Stretch section, skipped. Sections 3 to 5 ended mostly BLOCKED this run (per-instance property write gap, an intermittent round-spawn stall, and no Level Blueprint graph access), and the environment showed a `load_level` timeout in Section 5. Spending more session time on cosmetic polish on top of that felt like the wrong call versus stopping cleanly and handing over a clear list of what needs human attention. Not attempted. |
| 6.2 | Interaction prompt: anchor it a consistent distance above the bottom edge, centred. Confirm it does not overlap the weapon block. | SKIPPED | Same reasoning as 6.1. Not attempted. |
| 6.3 | Crosshair spread: confirm the crosshair opens with `GetCurrentSpreadDegrees()` and closes on ADS via `GetAimAlpha()`. Tune the pixel scale if it is jittery or does not visibly move. | SKIPPED | Same reasoning as 6.1. Not attempted. |
| 6.4 | Round readout: confirm it updates on `OnRoundStarted` and reads "ROUND N" or similar, top left, in the HUD font. | SKIPPED | Same reasoning as 6.1. Not attempted. Worth noting the round readout ("ROUND 1") was visibly working correctly in every screenshot taken during Sections 1, 2 and 4 of this run, top left, so this item is likely already in good shape whenever it is picked up. |
| 6.5 | Record every value changed. | SKIPPED | N/A, nothing changed in this section. |

---

## Section 7. Wrap up

| # | Item | Status | Notes |
|---|---|---|---|
| 7.1 | List every asset created or modified this run: full `/Game/LastTrain/...` path and type. | PASS | **`/Game/LastTrain/UI/WBP_HUD`** (WidgetBlueprint) - only asset actually left modified. `HealthTrackBox` (SizeBox): `bOverride_WidthOverride` False to True, `WidthOverride` 0 to 260, `HeightOverride` 4 to 16 (`bOverride_HeightOverride` was already True). Compiled and saved successfully, verified live in PIE. No other asset was left in a modified state: the `BP_RoundManager` CDO edit (`OpeningRoundCounts` index 0 to 30) was explored, found ineffective against the instance override, and explicitly reverted back to `(6,8,10,12,14)` before this section, confirmed by fresh readback. |
| 7.2 | List every deviation from the specs and why. | PASS | (1) Section 1: fixed `HealthTrackBox.HeightOverride` (4 to 16) even though the checklist's literal trigger condition was "if height is also zero" and it was 4, not 0 - a 4px bar was empirically invisible in PIE, so treated the spirit of "visible bottom-left" as the real bar. (2) Used `invoke(actor_label=..., "GetHealthFraction")` polling instead of `DisplayAll`/Details-panel reads throughout, and `playtest_read_state`/`playtest_log_contains` instead of pausing PIE and clicking actors, since these are the bridge-native equivalents and are more reliable than trying to drive the real editor UI blind. (3) Never used literal WASD/mouse-click input as instructed ("direct editor input, not `playtest.key`") because there is no such literal input channel from this Lua bridge; used `playtest_axis`/`K2_SetActorLocation` teleports where movement was needed instead, and let zombies path to a stationary player rather than the player approaching zombies. (4) `read_log('output', {lines=N})` was found to always return only the first ~100 lines regardless of `N` and is unreliable for anything beyond the very start of a session; used `playtest_log_contains`/`playtest_log_marker` (bridge-native, reliable) or `grep` on the raw log file directly instead for the rest of the run. |
| 7.3 | List every `BLOCKED` item and what a human needs to do to unblock it. | PASS | **2.6** zombie freeze: reproduced the exact queued-behind-the-front-attacker stall the C++ fix was meant to kill (5 zombies stationary, zero velocity, valid target, zero cooldown, for 12.5s+ straight). Needs a further C++ change to `DriveTowardsTarget()`/repath logic for zombies blocked by another pawn's capsule in a corridor, not just "no controller" or "path failed" cases. **3.2 to 3.4** round manager instance override: `GreyboxTest_RoundManager.OpeningRoundCounts` is `EditDefaultsOnly`, so `set_actor_property` refuses the instance write outright, and there is no reset-to-class-default verb exposed for actor instance properties in this toolset; editing the `BP_RoundManager` class CDO does not propagate to the already-placed, already-overridden instance. Needs a human to open the Details panel on `GreyboxTest_RoundManager` in `L_GreyboxTest` and either edit `OpeningRoundCounts` directly there or click the reset-to-default arrow, then Sections 3.2 to 3.4 (the actual Phase B 24 to 40 zombie crowd/frame gate) can be re-run. **4.1, 4.2** health regen and vignette: three consecutive fresh PIE sessions logged `Round 1 starting` but never a single `Spawned zombie` afterwards (an earlier session in this same run did spawn normally), so no zombies existed to damage the player. Needs a human repro pass outside this bridge to see if the stall is real or an artefact of many rapid PIE start/stop cycles from one long editor session; if real, needs a `LogLastTrain` trace through `LTRoundManager::TrySpawnOne`/its timer to see why the very first spawn sometimes never fires. **4.3, 4.4** wall buy prompt and purchase flow: could not aim the first-person camera at the wall buy through the bridge (`Controller.SetControlRotation` is not `BlueprintCallable`, actor-rotation writes do not affect the FP camera, and `playtest_axis` mouse nudges have no feedback loop to centre precisely). Needs either a literal interactive playtest pass or a small `BlueprintCallable` "face point"/"set control rotation" helper exposed for scripting. **5.2 to 5.5** Canary Wharf Level Blueprint: the Level Blueprint's EventGraph is not reachable through any tool in this toolset (`open_asset` on the map only exposes generic World properties, `find_blueprints` returns 0 results for it). Needs a human to open the Level Blueprint from the editor's Blueprints menu and wire `Event BeginPlay` to `Get All Actors Of Class(BP_RoundManager)` to `Get [0]` to `BeginRounds`, compile, save; then 5.3 to 5.5 can run. |
| 7.4 | State plainly whether the Phase B gate is met and whether B2 and B3 acceptance are closed. | PASS | **Phase B gate (24 to 40 zombies at 60 fps) is NOT met or verified this run.** Blocked entirely on the `OpeningRoundCounts` instance override (3.2), which caps every session at 6 zombies regardless of the `MaximumAlive=24` cap being correctly configured. **B2 and B3 acceptance are NOT closed.** B3 (HUD) is close: the health bar bug is fixed and verified (Section 1), round readout and points/weapon blocks all render correctly in every screenshot this run, but hit marker, crosshair spread and interaction prompt polish (Section 6) were not touched. B2 (interaction/wall buy) is unverified this run due to the FP camera aiming block (4.3/4.4); the wall buy actor itself is correctly placed with correct costs (500/250) but its actual interaction flow was not exercised. What **did** pass cleanly and can be signed off: Section 1 (HUD health bar, both width and height), and Section 2 (the `DriveTowardsTarget()`/`ContactRange` C++ fix genuinely works: attacks fire reliably and deal correct damage in contact), with the caveat that Section 2.6 surfaced a real, separate stall bug in the same code path for trailing zombies in a queue. |
| 7.5 | Save all. Do not commit. Stop. | PASS | `WBP_HUD` saved (Section 1). `BP_RoundManager` reverted to its original saved state and confirmed via readback (Section 3). No level (`.umap`) was saved with any change: `L_GreyboxTest`'s round manager instance was never successfully written to, and `L_CanaryWharf_Greybox` was only loaded and inspected, never edited. No git commands were run. Stopping here. |

---

## Run summary, follow-up session 2026-09-06 (afternoon)

Picked up after the human's two claimed fixes. Precondition confirmed: fresh
binary (`StallSpeedThreshold`, `StallGraceSeconds`, `StallLateralFraction`,
`ContactRange` all reflect on `LTZombieCharacter`, matching commit `5f549c9`).

- **Section 3 (Phase B crowd/frame gate): still BLOCKED, not the same reason as
  before.** The human's claim was accurate: `GreyboxTest_RoundManager` really
  does read `(6,8,10,12,14)` with no override relative to the CDO. But
  `OpeningRoundCounts` is `EditDefaultsOnly`, so `set_actor_property` on the
  instance is refused regardless of override state, and a CDO edit on
  `BP_RoundManager` (tried again, confirmed it took and saved) does not
  propagate to the already-placed instance. **24 to 40 zombie crowd/frame gate
  still not measured.** Needs either a `Source/` change to make the property
  `EditAnywhere`, the instance deleted and re-placed fresh off an edited CDO, or
  a human edit through the Details panel UI (which the in-editor UI permits
  even though the scripted write path does not).
- **Section 2.6 re-verify: still BLOCKED, stall still reproduces.** Different
  shape than the overnight repro (an arc around the player, not a single-file
  corridor), but the same failure: one zombie (`BP_Zombie5`, 149 units out,
  `AttackRange=130`) sat completely frozen at zero velocity for 23+ continuous
  seconds, well past `StallGraceSeconds=0.5`. The stall-recovery nudge in
  `UpdateStallRecovery` did not fire. Screenshot taken. This is a tighter, more
  isolated repro than the original (single frozen zombie, not a queue) and may
  be easier to root-cause from.
- **Section 5: Level Blueprint wiring PASS, but a new fall-through bug BLOCKS
  the smoke test.** `BeginRounds` fires correctly and 6 zombies spawn. But
  every spawned zombie free-fell through the level (z dropping from roughly
  -11800 to -70500 over 6 seconds) instead of pathing toward the player,
  almost certainly a nav mesh or floor collision gap under the
  `CW_SpawnPoint_*` actors. The walk-the-space and enjoyability checks (5.3 to
  5.5) could not be attempted meaningfully as a result.
- **Hard stop triggered per the ground rules: three sections in a row (3, 2.6,
  5.3) ended BLOCKED this session.** Stopping here rather than continuing to
  Section 4 or 6.
- Editor crashes this session: 0. Both PIE sessions started and stopped
  cleanly via `playtest_start`/`playtest_stop`, confirmed fully ended via
  `playtest_status()` before the next action.
- Assets touched: `BP_RoundManager` CDO was edited to `(30,...)` then reverted
  to `(6,8,10,12,14)` and re-saved as an experiment; confirmed clean via fresh
  readback both times. No level (`.umap`) was saved with any change.
- Nothing committed, no git commands run.
- **Is the Phase B gate met: No, still not measured.** **Are B2/B3 acceptance
  closed: not attempted this session** (session ended at the hard-stop
  threshold before reaching Section 4).
- Handoff, in priority order:
  1. `OpeningRoundCounts` on `GreyboxTest_RoundManager`: the override is
     genuinely clear now, but the property still cannot take a new instance
     value through any scripted path. Either make it `EditAnywhere` in C++, or
     a human sets it directly in the Details panel UI (not via a CDO edit),
     then Section 3 can actually run.
  2. The corridor stall (`UpdateStallRecovery`) still does not fire for at
     least one confirmed case: a single zombie 19 units outside `AttackRange`,
     zero velocity, zero cooldown, frozen for 23+ seconds. Worth checking
     whether the nudge's own gating logic (order of checks, a per-tick reset,
     or how it samples ground speed) is preventing `AddMovementInput` from
     ever being called, even though the class defaults look correct.
  3. New bug: zombies spawned in `L_CanaryWharf_Greybox` fall straight through
     the level instead of pathing to the player. Check `RecastNavMesh-Default`
     coverage and floor collision under the `CW_SpawnPoint_*` actors.

---

## Run summary

- Started: 2026-09-06, editor already open, freshly compiled binary confirmed via `ContactRange`/`AttackRange` reflection on `LTZombieCharacter` before starting.
- Ended: 2026-09-06, after working Sections 1 to 7 in order (Section 6 stretch work skipped by choice, see 6.1 to 6.5 notes).
- Sections passed: **Section 1** (WBP_HUD health bar, fully fixed and verified in PIE) and **Section 2** (zombie attack fix confirmed working: attacks fire reliably, damage applies correctly, HUD reflects it) both PASS cleanly. Section 7 (this wrap up) completed.
- Sections blocked: **Section 3** (round manager instance override could not be cleared through available tools, so the 24 to 40 zombie crowd/frame gate was never actually exercised). **Section 4** (health regen and vignette blocked by zombies not spawning in three consecutive sessions; wall buy prompt and purchase blocked by being unable to aim the FP camera through the bridge). **Section 5** (Level Blueprint EventGraph not reachable through any available tool, so the round loop never starts in `L_CanaryWharf_Greybox` and the smoke test could not run).
- Items blocked (see 7.3 for full detail and required human action on each): 2.6 (zombie freeze/stall in a queue, needs further C++), 3.2 to 3.4 (instance property override, needs a Details panel click or a tool API addition), 4.1 to 4.4 (spawn stall plus FP camera aim gap), 5.2 to 5.5 (Level Blueprint access gap).
- Editor crashes this run: **0.** No `EndPlayMap` SIGSEGV or any other crash. Every PIE session was started and stopped cleanly with `playtest_start`/`playtest_stop`, several `playtest_stop`/`playtest_wait_until` calls timed out or were slow to report but the underlying PIE session always ended correctly, confirmed via `playtest_status()` immediately after. One `load_level` call also reported a timeout but the level load itself succeeded.
- Is Phase B signable: **No.** The gate condition (24 to 40 zombies at 60 fps) was never reached this run because of the blocked `OpeningRoundCounts` instance override in Section 3; nothing else observed this run suggests it would fail (the `MaximumAlive=24` cap and the `DriveTowardsTarget()` fix both look correct), but it needs to actually be measured before signing.
- Handoff notes for the human:
  1. Highest priority: clear the per-instance `OpeningRoundCounts` override on `GreyboxTest_RoundManager` in `L_GreyboxTest` (Details panel reset-to-default arrow, or edit it there directly to include a higher first value like 30), then re-run Section 3.2 to 3.4 to actually measure the Phase B crowd/frame gate.
  2. Wire the `L_CanaryWharf_Greybox` Level Blueprint (`BeginPlay` to `Get All Actors Of Class(BP_RoundManager)` to `Get[0]` to `BeginRounds`), a five minute job, then a follow-up run can smoke test the whole space.
  3. Investigate the zombie freeze/queueing stall found in 2.6: zombies queued behind an attacking zombie in a corridor stop moving entirely (confirmed for 12.5s+, valid target, zero cooldown, zero velocity). This is a genuine gameplay bug independent of the round manager issue above.
  4. Investigate the intermittent first-spawn stall found in Section 4: after some number of PIE sessions in one editor run, `Round 1 starting` logs but no `Spawned zombie` follows. Unclear if this is a real bug or an artefact of rapid repeated PIE cycling from this bridge; worth a manual repro.
  5. Consider whether the NeoStack toolset would benefit from a `BlueprintCallable` helper for setting a player's control rotation (for FP-aim-dependent scripted verification) and/or a reset-instance-property-to-default verb, both of which blocked otherwise-reachable checklist items this run.
  6. Section 6 (HUD polish) was not attempted and remains fully open.

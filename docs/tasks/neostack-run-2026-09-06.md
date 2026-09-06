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
| 1.1 | Open `WBP_HUD`. Select `HealthTrackBox`. Set `bOverride_WidthOverride = True` and `WidthOverride` to a value that matches the bottom-left HUD block width. Use `260` if there is no existing precedent in the widget. If `bOverride_HeightOverride` is also False with a zero height, set it True with a height of `16`. | TODO | |
| 1.2 | Compile and save `WBP_HUD`. Verify in a fresh `execute_script` by reading the `HealthTrackBox` properties back. | TODO | depends on 1.1 |
| 1.3 | PIE `L_GreyboxTest` briefly. Take a zombie hit (or `DisplayAll LTPlayerCharacter Health` and stand in contact). Confirm the violet health bar is visible bottom-left and shortens on damage. Stop PIE with `playtest.stop`. | TODO | depends on 1.2 |
| 1.4 | Record the exact values you set and whether the bar drew and drained. | TODO | depends on 1.3 |

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
| 2.1 | PIE `L_GreyboxTest`. Use direct editor input (click into the viewport, hold W), not `playtest.key`, the simulated input was unreliable last run. Let a zombie path to the player and stand in contact for 8+ seconds. | TODO | |
| 2.2 | Filter the Output Log for `LogLastTrain`. Report which lines appear while the zombie is in contact: `TryAttack firing ApplyDamage 24.0 on ...` (attack fires), `TryAttack out of range: distance D ...` (report D), or `TryAttack early out: cooldown C target set/null` (report C and which). | TODO | depends on 2.1 |
| 2.3 | Pause PIE. Select the zombie in contact. Read and report its `AttackCooldown` and `CurrentTarget` from the Details panel. | TODO | depends on 2.1 |
| 2.4 | Report whether the player health dropped, from the now-fixed `WBP_HUD` bar and from any readable player state. | TODO | depends on 2.1, Section 1 |
| 2.5 | **Interpretation.** State which it is: (A) attack fires, health drops, only the bar was dead, now fixed. (B) attack fires, health does not drop, bug downstream in the player. (C) attack never fires, `out of range`, the approach still parks the zombie short. (D) attack never fires, `target null`, `CurrentTarget` is lost. If C or D, this needs another C++ change, mark BLOCKED and stop this section. | TODO | depends on 2.2, 2.3, 2.4 |
| 2.6 | Also watch for a zombie that freezes mid-approach (the stall the fix targets). If any zombie stops moving for more than 2 seconds while not in attack range, pause, select it, report `CurrentTarget`, `AttackCooldown`, its `MovementComponent` velocity, and whether its AI controller has an active move request. This is the exact failure the fix was meant to kill. | TODO | depends on 2.1 |

---

## Section 3. Round manager instance override and B1 crowd frame check

Full spec: `docs/tasks/phase-b1-throttled-repath.md` acceptance section.

Context: last run the `GreyboxTest_RoundManager` **instance** in `L_GreyboxTest`
carried a per-instance `OpeningRoundCounts` override still at `(6,8,10,12,14)`,
so editing the `BP_RoundManager` asset did nothing and PIE only ever spawned 6.
`MaximumAlive = 24` is the intended concurrent cap.

| # | Item | Status | Notes |
|---|---|---|---|
| 3.1 | Open `L_GreyboxTest`. Select `GreyboxTest_RoundManager` in the World Outliner (the placed instance, not the asset). Check `OpeningRoundCounts` in Details for a per-instance override (a reset arrow). Record what it currently reads. | TODO | |
| 3.2 | Set `OpeningRoundCounts` index 0 to `30` on whichever the running game reads (the instance if it overrides, else the asset). Save. Record the old value. | TODO | depends on 3.1 |
| 3.3 | PIE `L_GreyboxTest`. Console `stat unit`. Filter the log for `LogLastTrain` and confirm `Spawned zombie. Alive N ...` lines climb past 6 toward 24. Report the highest `Alive` reached and how long it took. Let nothing kill the zombies so the count only rises. | TODO | depends on 3.2 |
| 3.4 | With 24 alive, record game thread ms and frame ms over about 10 seconds. Screenshot the `stat unit` overlay. Pass is the game thread holding near 16.6 ms (60 fps). Note honestly if it does not. | TODO | depends on 3.3 |
| 3.5 | **Revert** `OpeningRoundCounts` index 0 to its original value. Save. Confirm by reading it back in a fresh script. Leave the map as found. | TODO | depends on 3.4 |

On a clean pass of 3.3 and 3.4, the Phase B gate (24 to 40 zombies at 60 fps)
is met. Note that here.

---

## Section 4. B2 and B3 full acceptance re-run

Checklist items 1.5 and 1.6 from the prior run, plus the vignette and regen
checks. Do these in `L_GreyboxTest` with **direct editor input**, not simulated.
Keep PIE sessions short.

| # | Item | Status | Notes |
|---|---|---|---|
| 4.1 | **Health regen.** Take zombie damage, then break contact and survive 4+ seconds untouched. Confirm the health bar refills. Screenshot before and after. | TODO | depends on Section 1, Section 2 passing A |
| 4.2 | **Damage vignette.** Confirm the screen edges darken while taking zombie hits and recover as health regenerates. Screenshot mid hit and recovered. | TODO | depends on Section 2 passing A |
| 4.3 | **Interaction prompt, both directions.** Walk to the `ALTWallBuy` plate: the prompt fades in bottom centre, key glyph then price text. Walk away: it fades out cleanly. Stand at the edge of range: no flicker. Screenshot in and out. | TODO | |
| 4.4 | **Wall buy, B2 acceptance.** `E` with under 500 points does nothing. `E` with 500+ buys once, swaps the weapon, points flash crimson, the weapon block updates the same frame. `E` again offers ammunition at 250. Screenshot each state. | TODO | depends on 4.3 |
| 4.5 | On a clean pass of 4.1 to 4.4, record that B2 and B3 acceptance are closed. | TODO | depends on 4.1, 4.2, 4.3, 4.4 |

---

## Section 5. L_CanaryWharf_Greybox Level Blueprint and smoke test

Context: the blockout is built (prior run, 17 of 18 items). Item 3.14 from that
run, the Level Blueprint wiring, was BLOCKED because the EventGraph could not be
reached through the tools. Try again, it may behave differently.

| # | Item | Status | Notes |
|---|---|---|---|
| 5.1 | Open `L_CanaryWharf_Greybox`. Confirm exactly one `BP_RoundManager` is placed. If none, place one on the floor near the concourse. | TODO | |
| 5.2 | Open the Level Blueprint. In its EventGraph: `Event BeginPlay` to `Get All Actors Of Class` (`BP_RoundManager`) to `Get` index 0 to `BeginRounds`. Compile and save the level. If you cannot reach the Level Blueprint EventGraph through the tools, say so explicitly and mark BLOCKED, that item stays a human job. | TODO | depends on 5.1 |
| 5.3 | PIE `L_CanaryWharf_Greybox` from the PlayerStart. Walk the whole space: down the platform, around the flood zone, up both escalators to the mezzanine, through the concourse. Confirm no fall through and that zombies spawn from the top clusters and path down the platform toward the player. Stop with `playtest.stop`. | TODO | depends on 5.2 |
| 5.4 | Take 4 to 6 orientation screenshots: from PlayerStart up the platform, the train wall from the platform, the flood split, an escalator bank, the mezzanine looking down, the concourse. Save the paths into the Notes. | TODO | depends on 5.3 |
| 5.5 | Honest read: is it enjoyable to train zombies around using nothing but the grey boxes? Record it. | TODO | depends on 5.3 |

---

## Section 6. WBP_HUD polish (stretch, only if Sections 1 to 5 are done)

Against `docs/tasks/phase-b3-feedback-widgets.md`. Small, restrained. No new
mechanics. Palette only.

| # | Item | Status | Notes |
|---|---|---|---|
| 6.1 | Hit marker: confirm it appears on `OnHitConfirmed` and fades within about 0.3 s. A brighter or larger marker for a headshot. Tune if it lingers or is invisible. | TODO | |
| 6.2 | Interaction prompt: anchor it a consistent distance above the bottom edge, centred. Confirm it does not overlap the weapon block. | TODO | |
| 6.3 | Crosshair spread: confirm the crosshair opens with `GetCurrentSpreadDegrees()` and closes on ADS via `GetAimAlpha()`. Tune the pixel scale if it is jittery or does not visibly move. | TODO | |
| 6.4 | Round readout: confirm it updates on `OnRoundStarted` and reads "ROUND N" or similar, top left, in the HUD font. | TODO | |
| 6.5 | Record every value changed. | TODO | |

---

## Section 7. Wrap up

| # | Item | Status | Notes |
|---|---|---|---|
| 7.1 | List every asset created or modified this run: full `/Game/LastTrain/...` path and type. | TODO | |
| 7.2 | List every deviation from the specs and why. | TODO | |
| 7.3 | List every `BLOCKED` item and what a human needs to do to unblock it. | TODO | |
| 7.4 | State plainly whether the Phase B gate is met and whether B2 and B3 acceptance are closed. | TODO | |
| 7.5 | Save all. Do not commit. Stop. | TODO | |

---

## Run summary

Filled in at the end.

- Started:
- Ended:
- Sections passed:
- Items blocked:
- Editor crashes this run:
- Is Phase B signable:
- Handoff notes for the human:

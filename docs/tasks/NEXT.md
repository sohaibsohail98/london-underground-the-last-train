# NEXT — resume point for a fresh context window

Written 2026-09-04. Update this whenever you finish a task so a cold session can
pick up without re reading the whole history.

## The one line

B1 and B2 C++ is written and compiles on UE 5.8. There is still no playable map.
Every remaining task is editor work and nothing can be verified until the grey
box map exists. The next action is `docs/tasks/phase-a4-editor-setup.md` in the
Unreal editor, done by the user.

## What just happened (2026-09-04, later session)

- Implemented `docs/tasks/phase-b1-throttled-repath.md`. `LTZombieCharacter`
  now throttles its `MoveToActor` repath to a jittered 0.35s cadence instead of
  once per tick. Two files, compiles, unverified until there is a crowd to test.
- Implemented `docs/tasks/phase-b2-interaction.md`. New
  `ULTInteractionComponent` on the player traces on `ECC_Visibility` for an
  interactable and holds it; `Interact()` and an `InteractAction` input slot on
  the player; `ALTWallBuy` is the first concrete `ILTInteractableInterface`
  actor. Six new files plus the two player files, compiles clean.
- Nothing committed yet in this session. Diff is confined to the eight source
  files B1 and B2 name.
- Still no `Content/` assets. B1, B2, B3 acceptance and the Phase A5 test all
  wait on Phase A4 building `L_GreyboxTest`.

## What just happened (earlier)

- Fixed four build blockers so `LastTrain` compiles and the editor opens on
  UE 5.8: target settings to `V7` and `Unreal5_8`, renamed a shadowed
  `Instigator` parameter, moved `LastTrain.h/.cpp` into `Private/`, installed the
  Metal toolchain. External Xcode is on `/Volumes/DriveSohaib`.
- Added the reference frame at `docs/reference/reference-frame.png` with notes in
  `docs/reference/reference-frame-notes.md`. This is the Phase F art target and
  should stay in mind for every layout and art decision.
- Wrote `CLAUDE.md` at the repo root: engine facts, module layout, conventions,
  legal line, model split, working rules.
- Wrote this `docs/tasks/` scaffold: `README.md` with the phase plan, Phase A
  task specs, this file.
- On branch `phase/04-combat-slice`. Pending source changes from the build fix
  are committed in this branch.

## Repo state to be aware of

- `Content/` is empty apart from markdown. No maps, Blueprints, Input assets or
  data assets. Phase A4 creates the first ones.
- `LastTrain.uproject` `EngineAssociation` is now `5.8`.
- `web/` is the discarded Three.js build, tagged `phase-03`. Not part of this
  work. Do not touch it.
- CI has no compile step. Compile locally after every C++ change.

## Exact next action

Everything left is in the Unreal editor. Do it in this order:

1. `docs/tasks/phase-a4-editor-setup.md`: trace channel, seven input actions
   plus `IMC_Default`, `DA_Weapon_SMG`, the four Blueprints, `L_GreyboxTest`.
   Add an eighth input action `IA_Interact` bound to `E` in `IMC_Default` and
   assign it to `BP_PlayerCharacter`'s `InteractAction` slot (this is the B2
   editor step). Place one `ALTWallBuy` in the map with a cube on its Plate and
   `DA_Weapon_SMG` in its Weapon slot.
2. `docs/tasks/phase-a5-acceptance.md`: the ten point combat test.
3. B1 crowd check: raise `BP_RoundManager` `OpeningRoundCounts[0]` to 30, open
   `stat unit`, confirm the game thread holds near 60fps with 24 or more alive.
4. B2 acceptance: checks 3 to 7 in `phase-b2-interaction.md` (look at the plate,
   prompt appears; `E` with too few points does nothing; `E` with enough buys
   once; `E` again offers ammunition).
5. `docs/tasks/phase-b3-feedback-widgets.md`: `WBP_HUD`. No C++.
6. If A5 passes, mark Phase A done in `docs/tasks/README.md`. If anything fails,
   record it in a new `docs/tasks/phase-b-bugs.md` with check number, observed
   behaviour, the likely file, and whether it blocks the next phase.

There is a click by click walkthrough of steps 1 to 5 written for a first time
UE5 user. If it is still around it was produced as an HTML artifact in the
session that wrote B1 and B2.

## Written ahead, code landed, editor and acceptance pending

- `docs/tasks/phase-b1-throttled-repath.md` (2026-09-04). CODE DONE, compiles.
  `LTZombieCharacter` throttles its `MoveToActor` repath to a jittered 0.35s
  cadence. Acceptance is step 3 above and needs the map.
- `docs/tasks/phase-b2-interaction.md` (2026-09-04). CODE DONE, compiles.
  `ULTInteractionComponent` on the player, `Interact()` plus `InteractAction`,
  and `ALTWallBuy`. Eight source files. Editor wiring and acceptance are steps
  1 and 4 above.
- `docs/tasks/phase-b3-feedback-widgets.md` (2026-09-04). Hit marker, crosshair,
  prompt and the restrained HUD block. Editor only. Note that
  `OnHitConfirmed`, `OnAmmoChanged`, `OnPointsChanged` and `OnHealthChanged` all
  already exist and already fire, so this task needs no C++ at all. The spec
  carries a table of every existing delegate; if a session proposes new C++ for
  feedback, it has not read the headers.

## Phase plan in brief

A foundation and first playable grey box → B engine core hardening (repath,
interaction, hit markers) → C rounds, five zombie types, the train, the
departure board → D grey box Canary Wharf → E perks, bench, lost property,
revive → F art pass, Fable led, against the reference frame → G audio, HUD,
second station, balance. Full table in `docs/tasks/README.md`.

Design numbers (round loop, 100s train interval, 25s dwell, station heat, the
five zombie types, economy) come from `docs/brief-v2.md`. Engine and structure
come from `docs/brief-v3-unreal.md`.

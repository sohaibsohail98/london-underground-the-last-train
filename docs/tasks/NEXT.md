# NEXT — resume point for a fresh context window

Written 2026-09-04. Update this whenever you finish a task so a cold session can
pick up without re reading the whole history.

## The one line

The C++ combat slice compiles on UE 5.8. There is no playable map yet. The next
action is the in editor setup in `docs/tasks/phase-a4-editor-setup.md`, done by
the user in the Unreal editor.

## What just happened

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

1. User: work through `docs/tasks/phase-a4-editor-setup.md` in the editor.
2. User: run `docs/tasks/phase-a5-acceptance.md`.
3. If it passes: mark Phase A done in `docs/tasks/README.md`, then hand a session
   `docs/tasks/phase-b1-throttled-repath.md`. That spec is already written and
   ready. If it fails: failures go in a new `docs/tasks/phase-b-bugs.md` and are
   brought to a session one at a time.

## Written ahead, not yet started

- `docs/tasks/phase-b1-throttled-repath.md` (2026-09-04). Phase B's first task:
  `LTZombieCharacter::Tick` issues a `MoveToActor` pathfind every frame per
  zombie, which is the load the Phase B 60fps gate measures. The spec throttles
  it to a jittered ~0.35s cadence in the two zombie files. Opus, small diff,
  compile after. It does not depend on the editor work, only on Phase A having
  passed so there is a map to test the crowd on.

## Phase plan in brief

A foundation and first playable grey box → B engine core hardening (repath,
interaction, hit markers) → C rounds, five zombie types, the train, the
departure board → D grey box Canary Wharf → E perks, bench, lost property,
revive → F art pass, Fable led, against the reference frame → G audio, HUD,
second station, balance. Full table in `docs/tasks/README.md`.

Design numbers (round loop, 100s train interval, 25s dwell, station heat, the
five zombie types, economy) come from `docs/brief-v2.md`. Engine and structure
come from `docs/brief-v3-unreal.md`.

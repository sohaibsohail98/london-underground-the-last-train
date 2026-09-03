# Phase A5 — combat slice acceptance test

**Prerequisite:** `docs/tasks/phase-a4-editor-setup.md` complete, `L_GreyboxTest`
plays.

Run every check in `L_GreyboxTest`, play in editor. A failure here is a Phase A
or Phase 1 bug, not something to defer.

| # | Check | Pass condition |
|---|---|---|
| 1 | Move and look | WASD moves, mouse looks, Space jumps, Left Shift sprints and is faster |
| 2 | Fire | Left Mouse fires, magazine decreases, reloads automatically when empty, R reloads early |
| 3 | Body hit | Shooting a zombie's torso lowers its health and points increase by 10 |
| 4 | Body kill | Killing with body shots adds 60 points on the kill |
| 5 | Headshot kill | Killing with a head shot adds 130 points |
| 6 | Aim | Right Mouse narrows FOV, slows movement, tightens the spread cone |
| 7 | Sprint cancels aim | Sprinting while aimed drops the aim |
| 8 | Recoil bloom | Holding the trigger widens the cone, which recovers a second or so after release |
| 9 | Zombie attack and regen | A zombie reaches you and damages you, health regenerates about four seconds after the last hit |
| 10 | Round transition | Killing every zombie ends the round exactly once, the next starts after ten seconds with more zombies |

## On failure

Record each failure in `docs/tasks/phase-b-bugs.md` as: check number, observed
behaviour, the file most likely responsible, and whether it blocks Phase B.
Then bring the specific failures to a fresh session one at a time.

## On pass

Phase A gate is met. Update `docs/tasks/README.md` status to mark Phase A done,
update `docs/tasks/NEXT.md`, and begin Phase B with
`docs/tasks/phase-b1-throttled-repath.md` (to be written when Phase A passes).

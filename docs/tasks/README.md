# Tasks

One bounded task per file. Point a fresh session at the relevant file rather than
re typing the spec. The plan below supersedes the roadmap in `brief-v3-unreal.md`
Part 3 where they differ, because the C++ combat slice now compiles and the
sequencing has moved on.

The visual target for the whole project is `docs/reference/reference-frame.png`.
Read `docs/reference/reference-frame-notes.md` before any art or layout task.

## Status

| Phase | Goal | Gate | State |
|---|---|---|---|
| A | Foundation and first playable grey box | `unreal-setup.md` section 8 checklist passes | in progress |
| B | Engine core hardening: throttled repath, interaction system, hit markers | 24 to 40 zombies on the grey box platform stays stable at 60fps | not started |
| C | Rounds, five zombie types, the train, the departure board | a train arrives on schedule, you can board during dwell, staying raises pressure, rounds 1 to 10 play untouched | not started |
| D | Grey box Canary Wharf | it is fun to train zombies around in grey boxes | not started |
| E | Perks, upgrade bench, lost property, downed and revive | a full survival session start to death is possible | not started |
| F | Art pass, Fable led | a screenshot of the platform stands next to the reference frame without embarrassment | not started |
| G | Audio, restrained HUD, second station, balance | play well using only what is on screen | not started |

## Phase A tasks

| File | Task | Who |
|---|---|---|
| `phase-a1-commit-foundation.md` | Fix `.uproject`, land `CLAUDE.md` and this scaffold, commit the pending source changes | done in this branch |
| `phase-a4-editor-setup.md` | The in editor work: trace channel, Input assets, Blueprints, data asset, grey box test map | you, in the editor |
| `phase-a5-acceptance.md` | Run the ten point acceptance test, record failures | you |

## How to resume in a fresh context window

Start with `docs/tasks/NEXT.md`. It carries the current state, what was just done,
and the exact next action, written so a cold session can pick up without
re reading the whole history.

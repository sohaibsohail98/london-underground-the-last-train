# Tasks

One bounded task per file. Point a fresh session at the relevant file rather than
re typing the spec.

The visual target for the whole project is `docs/reference/reference-frame.png`.
Read `docs/reference/reference-frame-notes.md` before any art or layout task.

**Committed scope numbers.** v1 ships **2 stations**. Any "41 stations"
reference in the docs is an aspirational expansion list, not a plan. The
fictional line is a fictionalised Crossrail-scale line (main-line loading
gauge, modelled on the Elizabeth line, NOT a deep-level tube); the train is a
Class 345 "Aventra" silhouette. See `docs/brief-v3-unreal.md` Part 1.

**Who runs what:** `who-does-what.md`. Three sessions (terminal, CC-in-Unreal,
remote), only CC-in-Unreal can drive the editor.

## Status

| Phase | Goal | State |
|---|---|---|
| A | Foundation and first playable grey box | **done**. `L_GreyboxTest` plays. |
| B | Throttled repath, interaction system, hit markers | **code done, PIE-verified**. The 60 fps crowd gate is measured and currently FAILED (S6): ~26 to 32 fps with a horde on an M4, GPU-bound, not the crowd logic. Deferred to Phase F7. |
| C | Rounds, five zombie types, train, departure board, station heat | **done**. Compiled, PIE-verified: rounds, five types, sprinter round at 5, brute pair at 10, the train cycle, the departure board countdown, station travel both directions carrying points and weapon, station heat 0 to 3. |
| D | Grey box Canary Wharf | **done**. Both greybox maps play the full loop. Spawn points placed, horde funnels down the platform. |
| E | Downed and revive | **done** for the solo path: zero health downs, bleed-out clock, auto-revive at half health. Perks / bench / lost property NOT done: decide at the start of Phase G whether v1 ships them. |
| F | Art pass: kit, lighting, train, signage, zombie bodies, perf | **in progress**. Assets staged (`_incoming_assets/`), S11 signage started but broken. Specs: `phase-f-art-pass.md` and `phase-f1` to `phase-f7`. |
| G | Audio, restrained HUD, balance | **not started**. Spec: `phase-g-audio-hud-balance.md`. |

The full mechanical game (A to E) is code-complete and playable in greybox
today. Phase F is the gap between "greybox that plays" and "looks like the
reference frame". Phase G makes it a finished thing.

## Active task files

### Front end

| File | Task | Session | State |
|---|---|---|---|
| tracked as **S13** in `handover.md` | Main menu: `L_MainMenu` map, `BP_MenuGameMode`, `WBP_MainMenu`, set as launch map | CC-in-Unreal | in flight |

### Phase F

| File | Task | Session | Depends on |
|---|---|---|---|
| `phase-f-art-pass.md` | The Phase F plan and lane split | reference | - |
| `phase-f1-modular-kit.md` | Modular kit for the station shell, replace greybox cubes | CC-in-Unreal | nothing |
| `phase-f2-lighting-atmosphere.md` | Lumen, post process, fog, the sodium rig, crimson depth cue, wet floor | CC-in-Unreal, Fable guidance | F1 |
| `phase-f3-train-exterior.md` | Class 345 silhouette exterior, original livery, doors on the `ALTTrain` hooks | CC-in-Unreal | F1 |
| `phase-f4-train-interior.md` | The visible-through-doors interior slice | CC-in-Unreal | F3 |
| `phase-f5-signage-wayfinding.md` | Finish and fix S11: readable name boards, wayfinding, the hanging departure board face | CC-in-Unreal | F1 |
| `phase-f6-zombie-bodies.md` | Varied clothed bodies (City Sample Crowds), keep the five-type split | CC-in-Unreal | do after F1 |
| `phase-f7-perf-pass.md` | Profile the finished art with a horde, claw back the 60 fps gate | CC-in-Unreal, Fable guidance | F1 to F6 |
| `asset-research-phase-f.md` | Research subagents: kit reference, trim sheets, train reference, menu assets | Remote | **done 2026-09-09** |
| `../reference/asset-sources-phase-f.md` | The result: sources, licences, legal flags. `tools/asset-fetch/fetch-phase-f.sh` stages it | reference | - |

### Phase G

| File | Task | Session |
|---|---|---|
| `phase-g-audio-hud-balance.md` | The Phase G plan: G1 audio, G2 restrained HUD, G3 balance | mixed |
| `phase-g2-hud.md` | G2 in full: binding table, layout, downed and run-over states, the 5 open S7 findings | CC-in-Unreal |

### Still-relevant references

| File | What |
|---|---|
| `handover.md` | The live state ledger and the CC-in-Unreal session log (S0 to S13). Resume point for a cold session. |
| `neostack.md` | The editor-task brief: C++ parent classes, property names, the delegate table, what cannot be scripted. Still the reference for how the editor session works against the C++. |
| `editor-crash-endplaymap.md` | The PIE-teardown crash hazard, and the 2026-09-08 `MAP CHECK` crash. Read before any editor session. |
| `who-does-what.md` | The three sessions, the hard rules, how work hands off. |

## Design canon (not tasks, but authoritative)

- `docs/brief-v3-unreal.md` - engine, camera, phases, line identity, model split.
- `docs/brief-v2.md` - superseded for engine, still authoritative for the round
  loop, train timing (100s interval, 25s dwell), heat, the five zombie types,
  the mechanic library, the economy.
- `docs/design/gameplay-canon.md` - the settled design, tight, with coded values
  marked authoritative.
- `docs/art-direction.md` - palette, composition, trademark substitutions.
- `docs/reference/reference-frame-notes.md` - the visual target, what to lift,
  what must not be copied.
- `docs/reference/canary-wharf-research/` - station architecture, rolling stock,
  signage, materials research.
- `docs/unreal-setup.md` - the editor steps the C++ cannot do for itself.

## How to resume in a fresh context window

Start with `handover.md`. It carries the current state, what was just done, and
the exact next action, written so a cold session can pick up without re reading
the whole history.

# Phase F, the art pass: from playable greybox to the reference frame

Written 2026-09-09. This is the plan for the work between "the greybox plays the
full loop" (true today, Phases A to E all landed) and "a screenshot of the
platform stands next to `docs/reference/reference-frame.png` without
embarrassment" (the Phase F gate in `README.md`).

Read first: `docs/reference/reference-frame-notes.md`, `docs/art-direction.md`,
`docs/brief-v3-unreal.md` Part 1 (line identity and rolling stock) and Part 3
Phase 7. The palette is fixed: `#16161C` charcoal, `#6C4C9C` violet, `#E0A030`
sodium, `#B02030` crimson. Legal constraints in `CLAUDE.md` are non negotiable.

## Where the work splits

| Lane | Who | Why |
|---|---|---|
| Lighting, Lumen, post process volume, volumetric fog, emissive tuning | **Fable** | subtle, expensive to diagnose, does not announce itself when wrong |
| Material graphs, trim sheets, decal setup, the wet-floor material | **Fable** | same |
| Crowd performance once there is a profile (the 60 fps gate) | **Fable** | needs a profile, not a guess |
| Modular kit geometry, station blockout to finished shell, kit piece placement | **Opus in the editor** (the CC-in-Unreal session) or by hand | being wrong is visible in ten seconds |
| The train exterior and interior mesh and its original livery | **Opus in the editor** | geometry and material-instance parameter work against a fixed spec |
| Signage, wayfinding, departure board face, station-name boards | **Opus in the editor** | layout and text, already half-done in S11, needs finishing and fixing |
| Zombie body variation (City Sample Crowds or MetaHuman bases) | **Opus in the editor**, Fable if the anim blending misbehaves | asset wiring first, blend logic only if it breaks |
| Every task spec in this folder, git, CI, review of what comes back | **this terminal session** | no editor needed |

## The bounded tasks

Each is its own file so a fresh session can be pointed at it cold. Run them
roughly in this order; F1 and F2 unblock everything visual, F7 is last because
it needs the rest in place to profile against.

| File | Task | Lane | Depends on |
|---|---|---|---|
| `phase-f1-modular-kit.md` | A disciplined modular kit for the station shell: wall, floor, ceiling, pillar, platform-edge, tunnel-mouth, trim. Nanite. Replace the greybox cubes in `L_CanaryWharf_Greybox` with kit pieces on grid. | Opus editor | nothing |
| `phase-f2-lighting-atmosphere.md` | Lumen settings, a tuned post process volume, exponential height fog + volumetric, the sodium/white functional lighting rig, the crimson emergency lights receding down the tunnel as a depth cue. | Fable | F1 (needs real geometry to light) |
| `phase-f3-train-exterior.md` | The Class 345 "Aventra" silhouette exterior mesh, original charcoal/sodium/violet livery, doors that read, sitting in the platform as a wall filling one side of frame. | Opus editor | F1 |
| `phase-f4-train-interior.md` | Walk-through articulated interior: full open gangways end to end, LED strip lighting, PIS screens, longitudinal + transverse seating. Only the part visible through the doors needs finishing for v1. | Opus editor | F3 |
| `phase-f5-signage-wayfinding.md` | Finish and fix S11: readable station-name boards (the S12 review found them edge-on and off-colour), wayfinding pictogram signs, the hanging departure board face driven by the existing `ALTDepartureBoard` hooks. All original, no roundel, no Johnston. | Opus editor | F1 |
| `phase-f6-zombie-bodies.md` | Replace the single tinted mannequin (S9 minimal pass) with varied clothed bodies. City Sample Crowds is the intended route; MetaHuman bases are the fallback. Keep the five-type tint/behaviour split working. | Opus editor | nothing, but do after F1 so it is tested in the real space |
| `phase-f7-perf-pass.md` | Profile `L_CanaryWharf_Greybox` with 24 to 40 zombies and the finished art. Claw the frame back toward the 60 fps gate. GPU-bound today (S6): Lumen quality, VSM, Nanite, material cost, light count, LODs. | Fable | F1 to F6 (profile the real thing) |

## The gate

Take a screenshot of the `L_CanaryWharf_Greybox` platform from roughly the
reference-frame camera angle. Put it next to `docs/reference/reference-frame.png`.
It does not need to be equal. It needs to not be embarrassing: the lighting
reads as an underground station, the train reads as modern London rail without
being the Elizabeth line, the signage is legible and original, the floor is wet
and reflective, the space has depth. Then F7 gets the frame rate honest.

## Not in Phase F

Audio (Phase G), the final restrained HUD (Phase G), balance to round 12 to 15
(Phase G), a third station (out of v1 scope). The main menu is tracked
separately as S13 in `handover.md`.

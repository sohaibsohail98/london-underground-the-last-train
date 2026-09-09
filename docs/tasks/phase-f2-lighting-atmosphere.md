# F2: lighting, Lumen, atmosphere

Lane: **Fable** (subtle, expensive to diagnose when wrong). No `Source/` change.
Depends on: F1 (needs real geometry to light). Blocks the Phase F gate.

Read `docs/reference/reference-frame-notes.md` and `docs/art-direction.md`
sections 1 and 2. Palette fixed: `#16161C` / `#6C4C9C` / `#E0A030` / `#B02030`.

## Editor-access note

Only one process can hold the NeoStack MCP connection (port 9315) at a time. If
the CC-in-Unreal Opus session is live, either coordinate a handover window, or
author the post process volume settings and the wet-floor material graph against
a scratch map and hand the final parameter values back as a table for the Opus
session to apply. Do not both drive the editor at once.

## Goal

The lighting and atmosphere that carries most of the reference frame's
impression. Per the brief this is "mostly a tuning problem" that Lumen gives you
"with effort".

## Scope

1. **Lumen** GI and reflections on. Tune quality against frame cost (F7 will
   profile, but do not ship something obviously too expensive). Hardware ray
   tracing off by default.
2. **Post process volume**, unbound, covering the station. Exposure locked or
   tightly ranged so the sodium reads warm and the shadows stay deep. Slight
   bloom on the emissive fittings. Subtle grain. No heavy vignette, no chromatic
   aberration, no film-emulation LUT that fights the palette.
3. **Exponential height fog + volumetric fog** for the tunnel haze. Light,
   directional-ish, thicker down the tunnel mouths so they read as depth.
4. **Functional lighting rig:** sodium `#E0A030` and warm white. Ceiling
   fittings down the platform, brighter pools under them, falloff between. The
   space should have rhythm, not flat fill.
5. **Emergency lighting as a depth cue:** crimson `#B02030` units receding down
   each tunnel mouth as a gradient. Not general illumination. The rest of the
   map is not red. Copy the reference precisely on this.
6. **Wet floor material:** the platform floor reads wet with real Lumen
   reflections. This "does more for the look than any other single decision"
   (art-direction.md). Puddle mask, not a uniform mirror. Wear where people
   walk.

## Rules

- No gameplay actor moves.
- `MAP CHECK` console exec is banned (crashed the editor 2026-09-08, see
  `editor-crash-endplaymap.md`). Use `MAP CHECKDEP NOCLEARLOG`.
- Never mutate anything while PIE is running.

## Accept

- Screenshot from the reference-frame camera next to `reference-frame.png`: the
  lighting reads as an underground station, the floor is wet and reflective, the
  tunnel mouths recede into crimson-tinged haze, the sodium is warm and the
  shadows are deep.
- Not embarrassing next to the reference. Not required to equal it.
- Frame cost noted (rough fps at the camera with a round running) so F7 has a
  baseline.
- New material and lighting assets committed via LFS.

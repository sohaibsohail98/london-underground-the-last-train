# F7: performance pass

Lane: **Fable** (needs a profile, not a guess). Depends on: F1 to F6 in place.
May touch `Source/` only if a genuine logic cost is found; default is no code
change.

Read the S6 rows in `handover.md` (the frame readings) and `docs/brief-v3-unreal.md`
Part 1 "Rendering" (the 60 fps at 1080p with 24 zombies target).

## Context

S6 measured `L_CanaryWharf_Greybox` on an Apple M4:

- Editor PIE, 30 zombies: ~40 fps / ~24 ms, GPU-bound, game thread ~2.5 ms.
- Packaged Development build, `-game` 1280x720, natural spawns (6 zombies):
  ~32 fps, GPU 29 ms.
- Packaged, forced horde 24 to 36 alive: ~26.5 fps, GPU ~36 ms, game thread
  ~5 ms.

**GPU-bound in every configuration. The crowd logic (B1) is not the cause.**
B3's 60 fps gate is currently failed. This was deferred to Phase F because the
art was not in yet and there was nothing meaningful to profile.

## Goal

Get `L_CanaryWharf_Greybox` with 24 to 40 zombies and the finished F1 to F6 art
as close to 60 fps as the hardware allows, honestly measured in a packaged
Development build.

## Where to look (GPU cost, in rough priority)

- **Lumen** GI and reflection quality. The wet floor (F2) is a reflection cost.
  Screen-space vs software vs hardware trade. Final gather quality.
- **Virtual shadow maps** resolution and page cost, especially with many
  emissive fittings and the train's interior strip lights.
- **Nanite** on the kit: is it actually helping, or are there non-Nanite meshes
  (skeletal crowd bodies, foliage-style clutter) dominating.
- **Crowd**: LOD distances and screen sizes on the F6 bodies, animation update
  rate off-screen, `URO` (update rate optimisation), draw call count for 40
  distinct bodies. Try the engine's **Animation Budget Allocator** before hand
  tuning URO per component: it throttles skeletal mesh tick rate by a
  significance evaluator (screen size, distance, visibility), which is a
  closer match to "many zombies behind a pillar or far down the platform"
  than a flat URO setting, and needs no `Source/` change, only a project
  setting and a significance function. Confirm it is actually cheaper than
  URO for this scene rather than assuming it, both are on the table.
- **Light count** and overlap down the platform. Shadowed vs unshadowed.
- **Translucency / decals**: the blood, the wet-floor puddle mask, any glass.
- **Post process** cost (bloom, grain).
- Resolution scaling / TSR settings as a last lever.

## Rules

- Profile first, change one thing, re-measure. No blind toggling.
- A change that costs visible quality has to be weighed against the reference
  frame: do not sacrifice the wet floor or the sodium atmosphere to hit a
  number. If 60 is not reachable without gutting the look, land on the best
  honest compromise and record it.
- `MAP CHECK` console exec banned (see `editor-crash-endplaymap.md`).
- If a `Source/` change is genuinely warranted (a real logic cost surfaces),
  write it as its own spec, compile locally, do not slip it in.

## Accept

- A packaged Development build, `L_CanaryWharf_Greybox`, 24 to 40 zombies alive,
  measured fps recorded with the settings that produced it.
- A short before/after table of what was changed and what each change bought.
- If 60 fps is met: the gate passes. If not: the honest number, the reason, and
  the recommended target hardware / resolution stated plainly.

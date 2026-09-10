# F-Greybox: L_GreyboxTest becomes the second station

Lane: Opus in the editor (CC-in-Unreal session). No `Source/` change.
Depends on: F1 to F7 all landed on `L_CanaryWharf_Greybox`. Queued behind them
deliberately, so there is a finished reference to reuse from rather than a
moving target. Do not start this until the Canary Wharf row in
`docs/tasks/handover.md` says F7 is done.

Read `docs/reference/reference-frame-notes.md` and `docs/art-direction.md`
sections 1 to 3 first, same as every other Phase F task. This file only adds
what is different for a second, distinct station: it does not repeat the
palette, livery substitutions or legal rules, which apply project wide and are
already stated there.

## Goal

`L_GreyboxTest` is v1's second station (the brief commits to exactly two).
It is already fully playable: rounds, the five zombie types, boarding, station
travel, signage and the HUD all pass there today. What it lacks is every art
pass Canary Wharf has had (F1 to F7): it is still on Phase B scaled cube
geometry with S4's original flat dressing lighting, and has no train mesh.

This task brings it to the same finished state as Canary Wharf, reusing the
kit meshes, materials, train mesh and lighting approach already built rather
than authoring any of it again from scratch, **and gives it a distinct real
world identity so the two stations do not read as the same place twice: this
is Paddington.**

## Why Paddington

Both v1 stations are on the fictionalised line described in
`docs/brief-v3-unreal.md` Part 1: a fictionalised Crossrail-scale line, main
line loading gauge, modelled on the Elizabeth line, not a deep level tube.
Paddington is a real Elizabeth line station, so the choice is factual and
needs no invented geography, and its actual platform architecture is a
deliberate contrast to Canary Wharf's glass box island platforms: long,
gently curved side platforms in a wide below ground box, not a straight
island hall. That contrast is the point. Station names and real geography are
factual and fine per `CLAUDE.md`; nothing else about Paddington (no roundel,
no Great Western branding, no heritage ironwork replica, no real signage) is
being reproduced, only the platform's general proportions and layout logic.

**Reference facts to build from** (public knowledge, dimensions approximate,
used for proportion only, not for a literal reconstruction):

- Elizabeth line Paddington sits in a deep box station below the main line
  terminus, opened 2022. Two island-adjacent side platforms either side of a
  central through wall/services core, not the single wide island Canary Wharf
  uses: build `L_GreyboxTest` as a **single side platform against one closed
  wall**, the opposite side open to the trackbed and tunnel, same as the
  existing greybox composition and the reference frame's "one side closed, one
  side open" rule. Do not attempt the twin platform pair, that is out of scope
  and does not change the single player arena.
- Platforms are long and gently curved in plan, not dead straight: a slight
  arc down the platform's length reads as distinctly different from Canary
  Wharf's straight run, and costs nothing extra to greybox with the same kit
  pieces on a curved spline instead of a straight line.
- Finish is cooler and more clinical than Canary Wharf's warmer tile: pale
  grey/off white wall panelling with darker accent bands, exposed concrete
  soffit, a wide flat ceiling rather than Canary Wharf's coffered vault. Keep
  this as a material swap on the existing kit (new tint on `MI_LT_Wall` style
  instances), not a new mesh set.
- Wide, unobstructed platforms with fewer columns than Canary Wharf's centre
  colonnade: place the existing pillar piece sparingly along the back wall
  rather than as a platform centre line, or omit it and rely on the wall
  modules alone.

## Scope

Apply the equivalent of F1 through F4 to `L_GreyboxTest`, reusing existing
assets:

1. **Kit (F1 equivalent).** Replace the Phase B scaled cubes with the same 12
   kit meshes from `Content/LastTrain/Kit/Meshes/` (wall panel, floor panel,
   ceiling panel, pillar, platform edge, tunnel portal, stairs, bench, bin,
   handrail). New material instances for the cooler Paddington palette
   (children of the same masters F1/F2 used, `MI_LT_Wall` / `MI_LT_Floor` /
   `MI_LT_Enclosure` lineage), not new meshes. Lay the wall/floor/ceiling run
   on a gentle curve rather than straight.
2. **Lighting (F2 equivalent).** Same three part rig as Canary Wharf: sodium
   overhead spots, cooled neutral fill, emergency crimson gradient into the
   tunnel mouths, manual exposure with the physical camera stop off, wet floor
   material. Reuse `MI_LT_FloorWet` and the same post process volume settings
   as a starting point, retuned for the paler wall tone so it does not blow
   out the way the first F1 pass did on Canary Wharf's near white tile (see
   known-issues 2.x and the F2 handover row for exactly what that looked like
   and how it was fixed, so it is not rediscovered here).
3. **Train (F3 equivalent).** Place the same train mesh and materials already
   authored for Canary Wharf (`Content/LastTrain/Train/Meshes/`,
   `M_LT_TrainEmissive`, the door leaf setup and `BP_Train`'s door timeline).
   No new train geometry. Reposition and rescale the placement to
   `L_GreyboxTest`'s platform length and the existing `GBX_Train` actor
   transform; do not move `GreyboxTest_PlayerStart` or any spawn point.
4. **Interior (F4 equivalent).** Same interior approach F4 lands on Canary
   Wharf (continuous lit gangway, LED ceiling strip, seating, grab poles):
   reuse the components as a child of `BP_Train`, which already carries them
   once F4 is committed, so this is largely automatic once the train is placed
   correctly.
5. **Signage.** Already done (F5 covered both maps). Only revisit if the new
   wall geometry moves the sign mounting points; if so keep the same panel
   geometry and typefaces, change only placement.

## Rules

- Every original gameplay actor stays exactly where it is at its current
  transform: `GreyboxTest_PlayerStart`, `GBX_Train`, the round manager, wall
  buy, all spawn points, the departure board once placed. Confirm each by
  read-back before and after, same discipline as every prior F task.
- Reuse assets, do not duplicate them: point at the existing
  `Content/LastTrain/Kit/`, `Content/LastTrain/Train/` and
  `Content/LastTrain/Materials/Dressing/` assets and create new **material
  instances** for the cooler palette variant, not new meshes or masters.
- `MAP CHECK` as a console exec is banned, use `MAP CHECKDEP NOCLEARLOG`.
- Never mutate an actor or asset while PIE is running: stop PIE cleanly first.
- No roundel, no Johnston, no Great Western or Elizabeth line branding, same
  legal rules as every other station. Station name factual: "Paddington" as
  the display text is fine (`StationDisplayNames` on `BP_GameMode` already
  has an entry to update), a reproduced GWR or TfL roundel/wordmark is not.

## Accept

- Open `L_GreyboxTest`, stand at the equivalent reference camera spot: the
  shell reads as a real, finished station, on the same kit and lighting
  language as Canary Wharf but visibly a different, cooler, curved space, not
  a reskin and not the old grey box.
- The train reads as a continuous lit wall through the doors and window band,
  same as Canary Wharf post F4.
- A round plays: horde spawns from the tunnel mouths, funnels down the curved
  platform, boarding still works.
- Map Check 0 errors, 0 warnings.
- Every gameplay actor confirmed unmoved.
- `StationDisplayNames` entry for `L_GreyboxTest` reads "Paddington" in the HUD
  and on the departure board.
- New material instances committed via LFS; no new mesh or master assets
  expected under this task.

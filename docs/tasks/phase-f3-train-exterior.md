# F3: train exterior mesh and livery

Lane: Opus in the editor. No `Source/` change. Depends on: F1.

Read `docs/reference/canary-wharf-research/rolling-stock.md` in full (it is the
physical spec), plus `docs/brief-v3-unreal.md` Part 1 "line identity and rolling
stock" and `docs/art-direction.md` section 1.

## Goal

The train that fills one whole side of the `L_CanaryWharf_Greybox` platform as a
wall. Class 345 "Aventra" silhouette, the project's own livery, doors that read
and that line up with the existing `ALTTrain` door hooks.

## The silhouette (model precisely, it sells "modern London rail")

- Rounded-rectangle cross section: near-vertical sides, slight tumblehome, flat
  roof, generous corner radii at cant rail and solebar. A box, not a tube.
- Body width 2.77 m, rail-to-roof 3.76 m, floor 1.145 m above rail.
- Driving cars 23.6 m, intermediate cars 22.5 m. For v1 the platform only needs
  the length that fills frame: model 3 to 4 cars, one a driving car with the
  cab, articulated together with full open gangways (no black gaps between
  cars, one continuous lit interior read).
- Full-width curved wraparound cab windscreen (the "smiling" front), headlight
  and marker clusters low on the nose, nose cone over a coupler cover.
- Flat roof with air-conditioning pods. Deep skirt (side valance) below the
  solebar hiding the underframe.
- Tall near-continuous glazed window band, large windows, softly radiused
  corners.
- 3 double-leaf sliding plug doors per side per car, each leaf pair ~1.45 m,
  flush when closed, a plain tall rectangular doorway. Lit passenger open
  buttons both sides of every doorway at ~1.0 to 1.1 m. Perished grey rubber
  edge seals.

## Livery (the project's own, NOT the Elizabeth line)

- Charcoal `#16161C` bodyshell.
- Sodium `#E0A030` cab band.
- Violet `#6C4C9C` door surrounds. Violet is an accent, not the bodyshell.
- An original wayfinding typeface on any car-side text (not Johnston).
- A made-up operator mark. Small, on the cab side. Original geometry.

**Off limits, restate every time:** the purple ELIZABETH LINE roundel; the TfL
grey/white bodyshell with a single purple sole-bar stripe as a copied livery;
New Johnston on blinds, numbering or signage; the operator name or "MTR
Elizabeth line" branding; TfL purple as the sole livery colour. A
purple-and-white unmarked train still reads as Elizabeth line: avoid it. The
silhouette says "modern London rail", the dressing says "not TfL".

## Wiring

- The train sits at the existing `BP_Train` / `ALTTrain` actor transform. If
  `BP_Train` currently uses placeholder geometry, swap the mesh, keep the actor,
  keep every hook.
- The nine presentation hooks on `ALTTrain` (arrive, dwell, depart, doors, etc.)
  drive door animation and light state. The door mesh must be riggable to the
  door-open hook. Confirm the hook names against `Source/LastTrain/Public/Train/
  LTTrain.h` before wiring.
- Doors open to the platform side during dwell, matching `ALTTrain`'s dwell
  window (25 s).

## Rules

- No other gameplay actor moves.
- `MAP CHECK` console exec banned (see `editor-crash-endplaymap.md`). Use
  `MAP CHECKDEP NOCLEARLOG`.
- Never mutate anything while PIE is running.

## Accept

- From the reference-frame camera the train reads as a continuous flat-roofed
  box wall, one lit interior end to end, curved cab at the leading end, deep
  dark skirt, violet door surrounds, sodium cab band. Not the Elizabeth line.
- Play a train cycle: doors open on the dwell hook to the platform side and
  close on depart. Boarding still works.
- Headless Map Check 0/0. Gameplay actors unmoved.
- Mesh and materials committed via LFS.

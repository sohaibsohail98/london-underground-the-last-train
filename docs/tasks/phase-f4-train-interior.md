# F4: train interior (visible-through-doors slice)

Lane: Opus in the editor. No `Source/` change. Depends on: F3.

Read `docs/reference/canary-wharf-research/rolling-stock.md` (interior section).

## Goal

The part of the walk-through interior that is visible from the platform through
the open doors and the window band. Not a full boardable interior for v1: the
player boards via the `ALTTrain` interact and the level travels, so the inside
only needs to read correctly from outside.

## Scope

- One continuous articulated interior with full open gangways: looking down the
  train through the glass you see end to end, no black gaps between cars.
- LED strip lighting the length of the ceiling each side: flat, bright, even.
  This is the lit strip that makes the train read as "wall" at night.
- Ceiling / cant-rail LCD strip displays above the windows, plus screens at car
  ends. Model the fittings, not the content (a dim generic glow is enough).
- Seating: longitudinal benches against the near-vertical wall by the doors,
  transverse bays further in. Straight benches, no splayed diagonal end seats.
- Grab poles, handrails, the door vestibule area.
- Wear: scuffed floor, worn seat moquette (original pattern, not a real
  operator's), grime at hand height.

## Rules

- The interior is a child of the F3 train mesh / `BP_Train`, moves with it.
- No gameplay actor moves. `MAP CHECK` console exec banned. No mutation during
  PIE.

## Accept

- Stand on the platform at the open doors: the interior reads as a bright, high,
  airy suburban train, one continuous lit space, not a dark box.
- From the reference camera the window band glows as a continuous strip.
- Map Check 0/0. Committed via LFS.

## Outcome, 2026-09-10

Built in `L_CanaryWharf_Greybox`. **Half accepted, half blocked by an F3 mesh
defect found in the course of the work.**

### Done

One continuous articulated interior from `x 1050` to `10050`, cavity
`y 5136` to `5393`, floor `z 12`, ceiling `z 286`, as loose actors under
`TrainShell/F4Shell`, `F4Light`, `F4Seats` and `F4Fittings`, all labelled
`CW_F4_*`. Nothing is parented to `BP_Train`: the F3 shell it sits inside is
itself static level geometry and only the 24 door leaves move, so the interior
matches the shell's own convention.

- Shell: floor, ceiling, far wall, two end caps, and near-wall panels in the
  13 runs between doorways, so the saloon is a closed lit box.
- LED strip lighting: two unbroken emissive runs the full length, one each
  side at the ceiling edge (`z 272` to `284`), plus 52 movable point lights on
  the ceiling line. Flat and even, both sides.
- Cant-rail LCD strip fittings above the windows both sides, and car-end
  screens flanking each gangway. Fittings only, a dim generic glow.
- Seating: 48 longitudinal bench runs against the wall either side of every
  doorway, straight, no splayed ends; 44 transverse bay seats in facing pairs
  at the 11 mid-points further in. A sodium colourway marks priority seats.
- Grab poles floor to ceiling flanking all 12 vestibules both sides (48), plus
  horizontal grab rails down both sides.
- Gangway pilasters and head at the 5 car joints: open, never a bulkhead, so
  the lit space stays continuous end to end.
- Wear: scuffed sheet floor, an original violet moquette weave (no operator's
  design), and a grime band at hand height down the far wall.
- 11 new material instances under `Content/LastTrain/Train/Materials/`,
  `MI_TrainInt_*`, off the existing `M_LT_PBRSurface` and `M_LT_TrainEmissive`
  masters. No new master was authored.
- The dead F3 fake interior plane `CW_F3_Interior` was removed, and
  `BP_Train`'s placeholder `CarriageMesh` was hidden (**visibility only**,
  collision untouched, so the boarding trace of 2.11 still resolves).

**Accepted:** from the reference camera the window band glows as one continuous
strip receding to the vanishing point, not isolated rectangles. Verified by
viewport capture in session; the frame is not committed, because `*.png` is
still outside the LFS filter (see `docs/known-issues.md` 3.4) and F2's two
acceptance frames already cost 2.7 MB of raw blob.

### Blocked

**The open-door half of the acceptance cannot pass.** The F3 door apertures are
recesses, not cut through the bodyside, so with the leaves fully open a player
at the doorway sees solid skin. Full evidence and the fix in
`docs/known-issues.md` 2.12. This needs a geometry-script re-author of
`SM_Train_DoorBay`, which the MCP bridge in this session cannot do, so it was
diagnosed and written up rather than guessed at.

Once the apertures are cut, the vestibule pieces already placed
(`CW_F4_Vest*`: ceiling, floor, reveals and a lit back wall at each doorway)
should light the opening with no further work, and the open-door view can be
re-shot to close the acceptance.

### Not verified

**Map Check was not run.** The MCP bridge this session used exposes no Map
Check tool (`EditorAppToolset` and `SceneTools` both lack one) and `MAP CHECK`
as a console exec is banned. The 0/0 figure in the acceptance list is therefore
outstanding, not passed.

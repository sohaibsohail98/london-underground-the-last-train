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

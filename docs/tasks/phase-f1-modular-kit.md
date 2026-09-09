# F1: modular kit for the station shell

Lane: Opus in the editor (CC-in-Unreal session) or by hand. No `Source/` change.
Depends on: nothing. Unblocks: F2, F3, F5.

Read `docs/reference/reference-frame-notes.md` and `docs/art-direction.md`
sections 1 and 2 first.

## Goal

A disciplined modular kit that builds the `L_CanaryWharf_Greybox` station shell,
replacing the greybox cubes with real geometry on a consistent grid. The
composition must not change: train as a wall down one side, platform receding to
a vanishing point, one side closed and one side open, tunnel mouths at each end
for the horde to arrive through.

## The kit

Grid: 100 uu base, kit pieces on a 200 or 400 uu module. Nanite on every static
piece. Pivot at a predictable corner so pieces snap.

| Piece | Notes |
|---|---|
| Wall panel | Dirty tile to about 2.2 m, painted concrete above. Trim strip at the tile line. A plain variant and a variant with a service-box recess. |
| Floor panel | Platform floor. Flat, will take the wet material in F2. A variant with the tactile paving strip along the platform edge (yellow-equivalent in sodium, rubber). |
| Ceiling panel | Coffered or ribbed concrete, service runs. Mount points for hanging light fittings and the departure board. |
| Pillar | Square section, tile-clad base, steel collar. Down the platform centreline, matching the greybox column spacing. |
| Platform edge | The lip, the fall to track level, the under-edge shadow gap. |
| Tunnel mouth | Outer wall with the arched or square portal, a lintel, ring detail receding a few metres into black. One at each end of the platform. |
| Stairs / ramp up | To the existing mezzanine deck. Keep the deck where the greybox has it. |
| Bench, bin, handrail | Brushed steel. Original design language, main-line station furniture. |

## Rules

- Every original gameplay actor stays exactly where it is at its current
  transform: the train, `BP_DepartureBoard` if placed, the wall buy, the round
  manager, all spawn points, the player start, the heat component. Confirm each
  by read-back before and after.
- The kit replaces `GreyboxTest/Station` and the Canary Wharf blockout geometry.
  Delete a greybox piece only after its kit replacement is placed in the same
  spot.
- Do the greybox map `L_GreyboxTest` too if time allows, but Canary Wharf is the
  one that gets photographed against the reference. Canary Wharf first.
- Do not run `MAP CHECK` as a console exec (it crashed the editor 2026-09-08,
  see `editor-crash-endplaymap.md`). Use `MAP CHECKDEP NOCLEARLOG` or the Map
  Check subsystem.
- Never mutate an actor or asset while PIE is running.

## Accept

- Open `L_CanaryWharf_Greybox`, stand at the reference-frame camera position,
  and the shell reads as a tiled main-line station hall, not a box of cubes.
- The horde still spawns from the tunnel mouths and funnels down the platform
  (play a round, watch it).
- Headless dependency Map Check: 0 errors, 0 warnings.
- Every gameplay actor confirmed unmoved.
- New kit meshes and materials committed via LFS (verify each `.uasset` is a
  pointer: `git show HEAD:<path> | head -c 45` -> `version https://git-lfs...`).

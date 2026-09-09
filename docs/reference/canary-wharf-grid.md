# Canary Wharf, grey box layout grid

The layout sketch for the Phase D grey box blockout of Canary Wharf. This is a
sketch to block out geometry against, not an authoring format for a generator.
It follows the same tile legend the discarded web build used, so the vocabulary
is shared. That build's source is at the `phase-03` tag (`legend.ts` and
`stations/debug-yard.ts`); nothing here depends on reading it, because the
legend and every constant are restated in full below.

Station facts, from `docs/brief-v2.md`: Canary Wharf, tier 4, mechanics `flood`
and `interchange`. The reference frame `docs/reference/reference-frame.png` and
its notes are the composition target: the train fills one whole long side as a
wall, the platform recedes to a vanishing point down a tiled corridor, the horde
funnels down that corridor toward the player, one side is closed and one open.

## Scale

`TILE = 1.5 m`, matching the legend. The grid below is 74 wide by 40 tall, so
about 111 m by 60 m. `WALL_HEIGHT = 3.6 m`. Platform lip `0.2 m` above floor,
trackbed `1.1 m` below floor.

## Tile legend, unchanged from the web build

| Char | Meaning | Grey box treatment |
|---|---|---|
| `#` | Wall | Cube, 3.6 m tall, static |
| `.` | Floor | Walkable slab at Z 0 |
| `=` | Platform edge | Floor with a 0.2 m lip cube along the track side |
| `T` | Tunnel mouth | Opening in the wall, dark box 6 m deep, a spawn route feeds from here |
| `B` | Boarded | Walkable floor with a low plank cube on top, blocks line of sight not path |
| `D` | Debris door | A cube filling the gap now, becomes a purchasable clear in Phase E |
| `W` | Wall buy | Floor, anchor for an `ALTWallBuy`, plate cube on the adjacent wall |
| `P` | Perk | Floor, anchor for a perk machine cube, Phase E |
| `L` | Lost property | Floor, anchor for the mystery box cube, Phase E |
| `U` | Upgrade bench | Floor, anchor for the bench cube, Phase E |
| `X` | Barrier | Waist high cube, walkable but slow, a ticket line or handrail run |
| `E` | Escalator | Sloped slab up to a mezzanine landing, pitch about 30 degrees |
| `S` | Spawn | Floor, anchor for an `LTSpawnPoint` |
| `~` | Water | Shallow flooded floor. Mark the zone now, no rising mechanic. Distinct flat cube 0.1 m proud, or just a marked material zone. Movement cost is higher, path still allowed |

## The grid

Row 0 is the far end down the platform, the vanishing point the horde walks out
of. The train wall is the full right edge, rows 2 to 24. The player and the
concourse are lower rows. Two tunnel mouths at the top feed the spawn routes.

```
##########################TT##################TT##########################
#........................#..#................#..#........................#
#..SSS....................#..#................#..#....................SSS..#
#..S......................D..D................D..D......................S..#
#........................#..#................#..#........................#
#........................#..#................#..#........................#
#....B..........B.........#..#....B.....B.....#..#.........B..........B...#
#........................#..#................#..#........................#
#========================================================================
#                        T R A I N   V O L U M E   (rows 2 to 24)        =
#========================================================================
#........................................................................#
#..W..................................................................W..#
#........................................................................#
#........X..X..X..X..X..X..X..X..X..X..X..X..X..X..X..X..X..X..X..X.......#
#........................................................................#
#........................................................................#
#........................................................................#
#..........~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~..........#
#..........~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~..........#
#..W.......~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.......W..#
#..........~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~..........#
#..........~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~..........#
#........................................................................#
#========================================================================
############D###################.........#####################D###########
#..........#...................#.........#...................#...........#
#..P.......#...EEEEEEEE.........#.........#.........EEEEEEEE..#........P..#
#..........#...EEEEEEEE.........#.........#.........EEEEEEEE..#..........#
#..........#...EEEEEEEE.........#.........#.........EEEEEEEE..#..........#
#..........#...EEEEEEEE.........#.........#.........EEEEEEEE..#..........#
#..........#...................#.........#...................#..........#
#..........#####...#############.........#############...#####..........#
#..........B...B...#...........................#...#....B...B...........#
#..L...............#...........................#...#...............U...#
#..................#....B.................B....#...#....................#
#..................#...........................#...#....................#
#..................#####################B#######...#....................#
#........................................................................#
#..S..........................B................................B......S..#
#........................................................................#
########################################################################
```

## How it maps to the reference frame

- **Train as a wall.** Rows 8 to 10 mark the platform edge and the reserved
  train volume down the whole width. In the blockout, place a static blocked out
  train shell here, carriage boxes with door gaps, so the platform reads as an
  arena in screenshots. No movement, no boarding logic, no `BP_Train`, that is
  Phase C. Boarding zone is the platform edge strip, rows 8 to 10, marked but
  not functional.
- **Platform recedes to a vanishing point.** The long platform hall is rows 2
  to 24. The horde spawns at the top (rows 2 to 4, `S` clusters, fed by the two
  `T` tunnel mouths) and walks down the corridor toward the player, exactly the
  reference framing.
- **One side closed, one open.** The train volume is the closed right side down
  the platform. The concourse and escalator bank is the open left, giving the
  player somewhere to fall back to.
- **Interchange.** Two escalator banks (`E`) drop from a mezzanine into the
  lower concourse, rows 26 to 31. This is the interchange mechanic's geometry,
  a second level and a traversal loop.
- **Flood.** The `~` block, rows 17 to 22, is the flooded centre of the
  platform. Mark the zone now as a distinct floor, no rising water. It splits
  the platform into two lanes so the horde cannot all come straight down the
  middle.
- **Debris doors.** The `D` gaps seal the tunnel approaches and the two
  concourse entrances. Cubes now, purchasable clears in Phase E, so early rounds
  are a smaller fight and the map opens up as you spend.

## Spawn routes

Four `S` clusters. Top two (rows 2 to 4) are the main horde source, funnelled
through the `T` tunnel mouths and the `D` debris gaps down the platform. Bottom
two (rows 37 to 38) are a smaller pincer from behind the concourse, only active
from a later round. Vary `LTSpawnPoint` `Weight`, top clusters heavier (2, 2),
bottom lighter (1, 1), and set `FirstRound` on the bottom pair to about 4.

## What Phase D builds from this

Primitives only. Floor slabs, wall cubes, the lip along the platform edge, the
train shell boxes, the sloped escalator slabs and a mezzanine landing, the
flooded zone as a marked slab, debris cubes in the `D` gaps, waist high barrier
cubes on the `X` run, `LTSpawnPoint` actors on the `S` tiles, anchor cubes on
`W` `P` `L` `U` for later. A `NavMeshBoundsVolume` over the whole thing, built,
with the flood zone still navigable at higher cost if the API allows setting
that, otherwise just navigable. It should be enjoyable to train zombies around
using nothing but these boxes. If it is not fun grey, materials will not save
it.

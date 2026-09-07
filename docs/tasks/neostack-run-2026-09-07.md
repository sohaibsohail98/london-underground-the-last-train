# NeoStack editor run, 2026-09-07

Iterative editor pass against `main` at `871f062`. Editor assets only, no
`Source/` changes, no commits.

## Step 0, sync and rebuild

- `git pull` on `main`: already up to date. All four required commits were
  present in the checkout: `871f062`, `7cb7c6d`, `bb0ec42`, `f76e0a4`.
- `Build.sh LastTrainEditor Mac Development`: `Result: Succeeded`. Only
  `WriteMetadata` ran, so the binaries already matched `871f062`. The spawn
  point root component fix is live in the running editor.
- Editor relaunched and the bridge reconnected.

## Milestone 1, Canary Wharf spawn points

### Root component check

All ten `CW_SpawnPoint` actors now report two components:

- `Root` (`SceneComponent`)
- `DirectionArrow` (`ArrowComponent`)

`RootComponent` reads back as `Root` on every one of them. Before placement all
ten were confirmed piled at `(0, 0, 0)` with zero rotation, which matches the
described symptom. The `871f062` fix took.

### Level geometry actually present

The level is further along than the task brief assumed, and that changed the
placement. Measured from the placed actors:

- `CW_Floor` top surface is at Z 0, spanning X 0..11100, Y 0..6300.
- A barrier line runs the full length of the platform at Y 4102..4148
  (`CW_Barrier_1` through `CW_Barrier_20`, X 1380..10020). The brief's suggested
  Y 4200 row would have sat on top of it.
- `CW_FloodZone` occupies Y 2850..3600.
- Platform lips sit at Y 4725..5025 and Y 2625.
- Concourse wall columns enclose the Y 600..2550 band.

So the usable open platform is the Y 2600..4700 band, split by the barrier line.
Points were placed in two rows either side of it: Y 3800 between the flood zone
and the barriers, and Y 4400 between the barriers and the platform lip.

### Final spawn point placement

All at Z 110. `bEnabled` true and `AreaTag` empty on all ten, unchanged.

| Actor | Location | Yaw | Weight | FirstRound | Placement |
|---|---|---|---|---|---|
| CW_SpawnPoint_1 | (1500, 3800, 110) | 0 | 2.0 | 1 | platform west end, faces east |
| CW_SpawnPoint_2 | (3300, 3800, 110) | -45 | 2.0 | 1 | platform west centre, faces south east |
| CW_SpawnPoint_3 | (5550, 3800, 110) | -90 | 2.0 | 1 | platform centre, faces south |
| CW_SpawnPoint_4 | (7800, 3800, 110) | -135 | 2.0 | 1 | platform east centre, faces south west |
| CW_SpawnPoint_5 | (9600, 3800, 110) | 180 | 2.0 | 1 | platform east end, faces west |
| CW_SpawnPoint_6 | (2200, 4400, 110) | -60 | 2.0 | 1 | rear platform west, faces south east |
| CW_SpawnPoint_7 | (4400, 4400, 110) | -90 | 2.0 | 1 | rear platform west centre, faces south |
| CW_SpawnPoint_8 | (6700, 4400, 110) | -90 | 2.0 | 1 | rear platform east centre, faces south |
| CW_SpawnPoint_9 | (8900, 4400, 110) | -120 | 1.0 | 4 | rear platform east, faces south west |
| CW_SpawnPoint_10 | (10400, 3800, 110) | 180 | 1.0 | 4 | far east platform, faces west |

Points 9 and 10 were left at `FirstRound` 4 as instructed. The other eight were
already `FirstRound` 1, so no property writes were needed on any of them, only
transforms.

Closest point to `CW_PlayerStart` (5625, 1125) is `CW_SpawnPoint_3` at 2675
units, comfortably clear of the 400 unit exclusion.

### Navigation

- `CW_NavBounds` measured at X -50..11150, Y -50..6350, Z -150..450, covering the
  whole level. A second stray 200 unit `NavMeshBoundsVolume` sits at
  (5240, 2490) fully inside the large one; left alone as harmless and outside
  the scope of this task.
- Full navigation rebuild run, `is_building` false and `is_locked` false after.
- Every spawn point projects onto navmesh directly beneath itself, X and Y
  unchanged, Z snapping to 10..16. This is the specific thing that was broken
  last session: no point projects to a bogus origin poly any more.
- Synchronous pathfind from all ten points to the player start succeeds, lengths
  3447..6030 units over 2 to 6 waypoints. No failed or empty paths.
- Both escalator landings project onto navmesh at Z 360.

### PIE result

| Check | Result |
|---|---|
| Spawn | Yes. `Round 1 starting with 6 zombies.` then Alive 1..6, pending 5..0, cap 24 |
| On platform | Yes. All six spawned at the points, Z 90..120 throughout |
| Path to player | Yes. All six converged and reached melee |
| Fall through | No. Nothing dropped anywhere tested |

Zombies were sampled at 130 u/s converging on the player, then three arrived at
distance 72..77 with speed 0, which is melee stop, while the rest closed. A
screenshot at that moment shows them upright on the platform pressing in on the
player.

Re-path was tested by moving the player 1500 units mid round. All six closed
monotonically, roughly 2750 down to 1460 units over 18 seconds.

Fall through was tested at nine positions. Player settles at Z 92 on all floor
positions, Z 280 on both escalator ramps, Z 449 on both escalator landings.
No spot dropped an actor.

### Stall recovery note

`BP_Zombie_C_1` logged `entering stall recovery` four times inside 1.6 seconds
while at melee range on the player. Not a frozen zombie, and not the
outside-melee-range failure the fix targets, but the repeated re-entry at the
attack position is worth a look. Carried into Milestone 2 for a proper read
under a dense crowd.

### Map check

One pre-existing error, `WorldSettings Maps need lighting rebuilt`. Unrelated to
spawns or navigation, not addressed.

Level saved. Not committed.

## Milestone 2, the two open C++ fixes on L_GreyboxTest

### Unplanned finding, the same spawn point bug on this map

`L_GreyboxTest` was described as the known good map, but its five
`GreyboxTest_SpawnPoint` actors were also all sitting at `(0, 0, 0)`. They had
never been placed either. Zombies reached the player here only because world
origin happens to sit in the middle of this small arena, so the bug was hidden
rather than absent.

The arena is a plain box: floor X -1500..1500, Y -750..750, player start at
(-1300, 0). With all five points stacked on one spot there was no funnel to test
a choke stall against, so the points were spread across the east half to make the
crowd converge westward on the player:

| Actor | Location | Yaw |
|---|---|---|
| GreyboxTest_SpawnPoint_1 | (1300, 0, 110) | 180 |
| GreyboxTest_SpawnPoint_2 | (1300, 550, 110) | -160 |
| GreyboxTest_SpawnPoint_3 | (1300, -550, 110) | 160 |
| GreyboxTest_SpawnPoint_4 | (600, 600, 110) | -150 |
| GreyboxTest_SpawnPoint_5 | (600, -600, 110) | 150 |

Navigation rebuilt after the move. This is a real placement, not a test change,
so it was kept and saved.

### Crowd gate write, bb0ec42

Succeeded. `OpeningRoundCounts` was written to `(20,8,10,12,14)` on the placed
`GreyboxTest_RoundManager` instance, stored and read back cleanly with no revert
to default arrow blocking it.

Confirmed at runtime as well, not just in the details panel. The log line
`Round 1 starting with 20 zombies.` appears in the PIE run, against
`Round 1 starting with 6 zombies.` from the Canary Wharf run earlier in the
session. The fix has not regressed.

### Stall recovery, bb0ec42

No zombie was found frozen for more than 2 seconds outside melee range while the
crowd was actively pathing.

Getting to a trustworthy answer took three runs, because the naive measurement is
misleading:

1. First dense run. Many zombies read speed 0 outside 250 units. That looked like
   the bug, but it is the outer ranks of a pile queued behind roughly fifteen
   bodies packed around a stationary player. Moving the player proved it: all 20
   of 20 resumed movement within 2 seconds and closed steadily. Blocked, not
   broken.
2. Second run. The player died early, so the whole crowd idled around a corpse
   and every zombie read as frozen. Not a valid sample.
3. Third run, player repeatedly relocated to keep the crowd genuinely pathing
   across open floor. Sustained speeds of 80 to 130 for every zombie, distances
   swinging as the player moved corner to corner, and **zero** stall recovery
   events logged for the whole run.

So the state machine behaves. The distinguishing test is whether a stalled zombie
frees itself once the obstruction clears, and it always did.

One anomaly worth recording. In the first dense run, `BP_Zombie_C_19` logged
`entering stall recovery` 122 times, once per second, continuously from 12:22:56
to 12:26:03, while every other zombie logged between 6 and 12 times. Sampled at
the end it sat at (345, 98) with speed 0 while the player was 359 units away.
That run had a dead player for part of its length, which accounts for a crowd
standing still, but the 1 Hz re-entry over three minutes is a distinct pattern
from the handful of events the other nineteen produced. It did not reproduce in
the clean third run. Flagged rather than called a defect.

### Attack behaviour under a stacked crowd

They press in and attack rather than forming a frozen ring. The player was killed
outright by the pile during the first dense run, health fraction reaching 0.00,
which is 100 damage delivered by zombies at 72 to 77 units. The screenshot of the
pile up shows roughly a dozen zombies packed shoulder to shoulder around the
player, upright and closed in.

### Frame rate at 20 to 24 alive, NOT MEASURED

This number could not be obtained through this harness and is deliberately not
reported.

Every delta sample returned exactly 0.33333 seconds, that is 3 fps, at a
precision that indicates a fixed background tick rather than a real measurement.
`t.MaxFPS` reads 0, `r.VSync` 0, `Slate.SleepWhenAppIdle` 0 and
`editor.LowerFPSWhenNotForeground` 0, and clearing them changed nothing. Game
time advanced at 0.95x wall clock, so the simulation ran at roughly real speed
while ticking about three times a second. The editor throttles its tick while the
window is unfocused, and every command in this session is driven over the bridge
with the window unfocused.

Reporting 3 fps against a gate of 60 would be actively misleading. The Phase B
crowd gate needs a measurement taken with the editor window focused and
`stat unit` read on screen by a human, or a packaged build. Left open.

### Clean up

PIE stopped. `OpeningRoundCounts` reverted to `(6,8,10,12,14)` and the revert
verified by readback. Note that the first revert attempt was lost when the editor
bridge socket dropped, and the value was still `(20,8,10,12,14)` on reconnect; it
was rewritten and confirmed. `L_GreyboxTest` saved.

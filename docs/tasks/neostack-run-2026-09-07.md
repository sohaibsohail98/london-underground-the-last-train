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

## Milestone 3, BP_Train and the C1 train cycle

### Editor crash mid milestone

The editor died once during this milestone, taking an unsaved in-memory
`BP_Train` with it. No crash dump was produced and the log simply ends after a
HotReload completed at 16:02:58, that is, a C++ rebuild landed from outside this
session while the editor was open. Both maps had been saved at 17:02 and came
back intact: the Milestone 2 revert and all spawn point placements survived.
`BP_Train` was rebuilt from scratch and saved after every step from then on.

### What was built

`Content/LastTrain/Blueprints/BP_Train`, parented to `ALTTrain`.

- `CarriageMesh`, a `StaticMeshComponent` on the engine cube, scaled
  (20, 3, 3) for a 2000 x 300 x 300 carriage, offset to (0, 250, 150) so it sits
  in the trackbed beside the platform rather than on it.
- `BoardingVolume` sized to extent (120, 90, 120) at (0, 80, 110), a door sized
  aperture at the platform edge and reachable by the player.
- All ten presentation events overridden, each with a Print String:
  `OnInboundAnnouncement`, `OnArrivalStarted`, `OnArrivalComplete`,
  `OnDoorsOpen`, `OnDoorsClose`, `OnDepartureAnnouncement`,
  `OnDepartureStarted`, `OnTrainAway`, `OnPlayerBoarded`,
  `OnTrainDeparted_NotBoarded`.
- `OnTrainPhaseChanged` bound on BeginPlay to a `HandlePhaseChanged` custom
  event that prints `TRAIN phase OLD -> NEW`.

Compiles clean, zero errors and zero warnings.

One deviation from the task text, forced by the code. Every timing property on
`ALTTrain` is `EditDefaultsOnly`, so it cannot be set on a placed instance. The
test timings were set on the Blueprint class defaults instead, which is the only
available route and achieves the same test.

Placed as `GreyboxTest_Train` at (0, 640, 0) in `L_GreyboxTest`, along the north
wall so the carriage sits beyond it. A `ULTStationHeat` component was added to
`GreyboxTest_RoundManager`, which the level did not have and
`ALTTrain::FindStationHeat` needs.

### Acceptance checklist

| Acceptance point | Result |
|---|---|
| Phase starts Away, first stop at load plus 8s | PASS, Away to Approaching at 8s |
| Cycle Away, Approaching, Dwelling, Departing, Away | PASS, full cycle observed twice |
| OnInboundAnnouncement once per cycle, 5s before the stop | PASS, fired 16:09:44.1, stop 16:09:49.1 |
| OnDoorsOpen about 1s into the dwell | PASS, dwell began 49.1, doors 50.1 |
| OnDoorsClose about 2s before departure | PASS, doors shut 57.1, departure 59.1 |
| Doors events strictly inside Dwelling | PASS, both between 49.1 and 59.1 |
| Board prompt only while doors are open | PASS, see below |
| OnPlayerBoarded fires | PASS |
| Rounds stop, GetZombiesRemaining stops climbing | PASS, held at 6 |
| Run state goes Boarded | PASS, `Run state 1 to 4` |
| Weapon reserve to full | PASS, per the engine log line |
| Station heat reads 0 after boarding | PASS |
| Train frozen after boarding | PASS, held Dwelling 12s with no phase logs |
| Not boarding: OnTrainDeparted_NotBoarded once, heat 0 to 1 | PASS |
| Second cycle without boarding: heat to 2, not more | PASS, `Heat now 1` then `Heat now 2` |

Hook ordering and timing came from the log, cross checked against the native
`Train phase X -> Y` lines the C++ already emits, so both the Blueprint hooks and
the underlying state machine are confirmed rather than just the prints.

The boarding gate was checked in both directions. While the doors were shut, in
Dwelling and again in Departing, `CanInteract` returned false and `TryBoard`
returned false. The moment the doors opened, `CanInteract` returned true. The
prompt text reads `Board train`.

### Blocking defect found, BP_GameMode has the wrong parent class

`TryBoard` initially returned false even with the doors open, the player at full
health and the run active. The cause is not in the train.

**`BP_GameMode`'s parent class is `GameModeBase`, not `ALTGameMode`.**

So `GetAuthGameMode<ALTGameMode>()` returns null and `ALTTrain::TryBoard` bails
out with `TryBoard with no ALTGameMode, cannot board`, which is in the log twice.
The knock on effect is wider than boarding: the level also runs with a plain
`GameStateBase` rather than `ALTGameState`, so there is no run state at all and
`ALTTrain::IsRunLive` silently takes its permissive test-level fallback.

To prove the train itself is correct, the world settings game mode was temporarily
pointed at the native `ALTGameMode` for one PIE run. With that in place the game
state became `ALTGameState`, the run state read Active, and boarding worked end to
end with every consequence firing. That override was a diagnostic only and has
been reverted; the level is back on `BP_GameMode_C`.

`BP_GameMode` was not reparented. That is an edit to an existing asset outside
this task's scope, and it changes behaviour for every level that uses it, so it
is the project owner's call. It is the single most important thing to fix: until
it is, boarding cannot work in any level using `BP_GameMode`, and the run
lifecycle is effectively absent.

### Clean up

PIE stopped. The world settings game mode override reverted to `BP_GameMode_C`
and confirmed. All eight `BP_Train` timing values restored to the class defaults
(30, 100, 25, 4, 4, 1, 3, 15) and confirmed by readback. `BP_Train` and
`L_GreyboxTest` saved.

## Summary

Milestone 1 and Milestone 2 both pass. Milestone 3 passes every acceptance point
for the train's own timing, hooks and heat, and passes boarding only once the
game mode is a real `ALTGameMode`.

What is now playable: Canary Wharf spawns six zombies on the platform that path
to the player and fight, with no origin spawning and no fall through anywhere
tested, and the same on the grey box test map. The train runs its full arrive,
dwell, depart cycle with every presentation hook firing on schedule, raises
station heat when it leaves without you, and completes a board when the game mode
is correct.

The single most important thing still broken: `BP_GameMode` is parented to
`GameModeBase` instead of `ALTGameMode`, so no level using it has a run state and
boarding always fails.

Also left open: the Phase B crowd frame rate gate was not measurable through this
harness, because the editor pins its tick to 3 fps while the window is unfocused.
It needs a focused editor or a packaged build.

Nothing was committed. One `git pull` was run in Step 0, as instructed.

# NeoStack autonomous editor pass, 2026-09-07 evening

Second session the same day, against `main` at `ab9cc8e`. Editor assets only, no
`Source/` changes, no commits.

## Step 0, sync and rebuild

- `git pull` on `main`: already up to date, HEAD at `ab9cc8e`. All three required
  commits present: `ab9cc8e` (BP_Train), `bf1878c` (Canary Wharf spawn points),
  `c4a6369` (the C2/C3 merge).
- Rebuild: `Result: Succeeded`, "Target is up to date". The merged C++ was
  already compiled locally.
- Editor relaunched. `LTGameMode`, `LTDepartureBoard`, `LTZombieTypeData` and
  `LTGameInstance` all confirmed reflected.
- `Config/DefaultEngine.ini` has `GameInstanceClass=/Script/LastTrain.LTGameInstance`
  but **no `GlobalDefaultGameMode`**, so levels must resolve the game mode through
  their own World Settings override. Both do.

## Milestone 1, reparent BP_GameMode: PASS

This was the blocker the previous run identified, and the reparent fixes it.

### Reparent

`BP_GameMode` parent class changed from `GameModeBase` to `LTGameMode`.
Compiled clean, zero errors and zero warnings, saved.

Defaults after the reparent, all intact:

| Property | Value |
|---|---|
| bAutoStart | True |
| GameStateClass | `LTGameState`, inherited from the ALTGameMode constructor |
| DefaultPawnClass | `BP_PlayerCharacter_C`, survived the reparent |
| NextStationMap | None, filled in Milestone 5 |
| StationRoutes | empty, filled in Milestone 5 |

### Both levels resolve to an ALTGameMode

`L_GreyboxTest` and `L_CanaryWharf_Greybox` were both already overriding to
`BP_GameMode_C` in World Settings, so no override change was needed; the
reparent alone is what makes them resolve to an `ALTGameMode`. Confirmed by CDO
check: `isinstance LTGameMode: True`. Both levels saved.

### PIE verification

| Check | Result |
|---|---|
| Game mode resolves as ALTGameMode | PASS, `BP_GameMode_C isLT: True` |
| Game state is a real ALTGameState | PASS, `LTGameState`, was `GameStateBase` before |
| Run state exists | PASS, reads Active at start |
| Round manager started by the game mode | PASS, `Round 1 starting with 6 zombies.` and `Run state 0 to 1.` |
| Player goes down, not dead | PASS, 500 damage left health 0 with `is_downed` true |
| Downed log line | PASS, `Player downed. Bleed-out in 30s.` |
| Solo auto revive | PASS, `Player revived at 50% health.` and health fraction 0.50 |
| Run state through the loop | PASS, `Run state 1 to 2.` then `Run state 2 to 1.` |

After the revive the normal 4s-delay regen carried health 0.50 to 1.00 over about
six seconds, which is the existing player regen behaving correctly on top of the
new revive.

The board test was skipped here as instructed; `BP_Train` from the previous
session is still placed in this level and was observed cycling normally during
the run, and station heat incremented to 1 on a departure without boarding.

This one change also switches on the run state machine for every level using
`BP_GameMode`, which boarding, downed and travel all depend on.

## Milestone 2, the five zombie type assets and the tintable material: PASS

Values came from the table in `docs/tasks/neostack.md`, which the merge rewrote
for exactly this. That table is authoritative and differs from the task brief in
a few places, so the doc was followed: brute `MeshScale` 1.5 not 1.4, crawler 0.5,
the material parameter is `TintColour` not `Tint`, and the walker and screamer
tints are white rather than charcoal.

### The five assets

Created under `Content/LastTrain/Zombies/`. Every value below was read back from
disk after saving, not just written.

| Field | Walker | Sprinter | Brute | Crawler | Screamer |
|---|---|---|---|---|---|
| Type | Walker | Sprinter | Brute | Crawler | Screamer |
| Behaviour | None | Sprint | ArmourPlate | LowProfile | Scream |
| HealthMultiplier | 1.00 | 0.55 | 5.00 | 0.35 | 0.80 |
| WalkSpeedMultiplier | 1.00 | 3.85 | 0.73 | 1.15 | 0.85 |
| AttackDamageOverride | 0 | 18 | 45 | 20 | 10 |
| AttackCooldownOverride | 0 | 1.0 | 2.2 | 1.1 | 1.5 |
| MeshScale | 1.00 | 0.95 | 1.50 | 0.50 | 1.00 |
| AnimPlayRate | 1.00 | 1.35 | 0.80 | 1.00 | 1.00 |
| RepathIntervalOverride | 0 | 0.2 | 0 | 0.4 | 0 |
| CapsuleHalfHeightOverride | 0 | 0 | 130 | 45 | 0 |
| AvoidanceConsiderationRadiusOverride | 0 | 0 | 70 | 0 | 0 |
| ContactRangeOverride | 0 | 25 | 0 | 0 | 0 |
| SpawnWeightNormalRound | 100 | 0 | 0 | 12 | 6 |
| FirstRoundAvailable | 1 | 5 | 10 | 8 | 12 |
| HighHeatWeightMultiplier | 1.0 | 2.0 | 1.0 | 2.0 | 2.0 |
| MaxAliveOfThisType | 0 | 0 | 0 | 0 | 1 |
| SprintLungeImpulse | 0 | 200 | 0 | 0 | 0 |
| ArmourBodyDamageToBreak | 0 | 0 | 200 | 0 | 0 |
| CorpseLifetimeOverride | 0 | 0 | 10 | 0 | 0 |
| DeathScreenShakeRadius | 0 | 0 | 600 | 0 | 0 |
| bRagdollOnDeath | false | false | false | false | false |
| ColourTint | white | sodium 0.88, 0.63, 0.19 | crimson 0.69, 0.12, 0.19 | violet 0.42, 0.30, 0.61 | white |

Screamer also carries `ScreamLineOfSightSeconds` 2.0, `ScreamSummonCount` 4 and
`ScreamCancelWindowSeconds` 0.5.

Worth recording: the first write pass silently did nothing. A Python exception on
the `bRagdollOnDeath` property name (it reflects as `ragdoll_on_death`, without
the Hungarian prefix) aborted the script before `save_asset`, so every value it
had set was discarded when the assets reloaded. The read-back check caught it and
the pass was redone. Verifying by reading from disk rather than trusting the write
is what made the difference.

### The material

`Content/LastTrain/Materials/M_Zombie_Tintable`, a default-lit material: a
`TintColour` vector parameter multiplied into a flat 0.35 grey base colour, with
roughness 0.85. Grey box appropriate, no texture needed.

Five `UMaterialInstanceConstant` assets, `MI_Zombie_Walker` through
`MI_Zombie_Screamer`, all parented to it, each with `TintColour` set to its
type's `ColourTint` and confirmed by read-back.

### How BP_Zombie picks its instance

This needed care because of an ordering constraint in the C++. In
`ALTRoundManager::TrySpawnOne`, `SpawnActor` completes, and therefore BeginPlay
runs, **before** `ApplyTypeData` is called. A material swap on BeginPlay alone
would read the default Walker type on every zombie regardless of its real type.

So the graph is: `Event BeginPlay` to a `Delay` of 0.05s, then a
`Switch on ELTZombieType` whose Selection comes from the BlueprintPure
`GetZombieType()`, and one `SetMaterial` per branch on the inherited character
mesh with the matching `MI_Zombie_*` instance at element index 0.

One shared master, five constant instances, one swap per spawn. No
`CreateDynamicMaterialInstance`, per the rule in the type asset header.

BP_Zombie compiles clean, zero errors and zero warnings.

### Verify

No PIE check for this milestone on its own, as specified. All five data assets
and all five material instances open without error and their values match the
table; the read-backs above are the evidence. The types actually spawning is
Milestone 4.

## Milestone 3, boarding end to end and BP_DepartureBoard: PASS

### Boarding works without the override

This is the headline result. The previous run could only make boarding work by
pointing World Settings at the native `ALTGameMode` as a diagnostic. With
`BP_GameMode` reparented in Milestone 1, boarding works through the normal path:

```
TRAIN OnPlayerBoarded
Run state 1 to 4.
Player boarded. Run state Boarded, rounds stopped, reserve refilled, heat reset.
```

`TryBoard` returned true, and `TryBoard with no ALTGameMode` does not appear
anywhere in the run.

### BP_Train and the round manager

`BP_Train` was still placed in `L_GreyboxTest` at (0, 640, 0) from the previous
session, and `GreyboxTest_RoundManager` still had its `ULTStationHeat` component,
so neither needed rebuilding.

### BP_DepartureBoard

Created from `ALTDepartureBoard` with a `TextRenderComponent` named `BoardText`,
world size 48, sodium `#E0A030`, centre aligned. Placed at (0, 560, 260) facing
the platform.

`OnCountdownChanged(WholeSeconds, Phase)` is overridden and builds the display
string: the whole seconds through `To String (Integer)`, appended to `"s "`, then
appended to a phase word from a `Select` on `ELTTrainPhase`. Away and Approaching
both read `inbound`, Dwelling reads `at platform`, Departing reads `departing`.
The result goes to `SetText` on `BoardText`; the editor inserted the string to
text conversion automatically. `OnPhaseChanged` is overridden and left empty for
a later recolour. Compiles clean.

One deviation. The board shows `8s inbound` rather than `0:08 inbound`. The M:SS
form needs integer divide and modulo nodes, and the fuzzy node search kept
resolving `Divide` and `Percent` to `FrameNumber / FrameNumber` and `As Percent`.
Since every countdown in the coded timings is well under a minute, raw seconds
with an `s` suffix is correct and readable, so that was kept rather than fight
the node picker. Worth revisiting when a real board mesh and font arrive.

### Departure board tracking, observed live

| Time | Phase | Doors | Board text |
|---|---|---|---|
| 2s | Away | shut | `8s inbound` |
| 6s | Away | shut | `4s inbound` |
| 8s | Approaching | shut | `2s inbound` |
| 10s | Dwelling | shut | `10s at platform` |
| 12s | Dwelling | **open** | `8s at platform` |
| 16s | Dwelling | open | `4s at platform` |
| 18s | Dwelling | shut | `2s at platform` |
| 20s | Departing | shut | `0s departing` |
| 22s | Away | shut | `17s inbound` |

The countdown tracks the train exactly and the wording switches on phase.

### C1 acceptance checklist

| Point | Result |
|---|---|
| Phase Away to Approaching at load plus 8s | PASS |
| Full cycle Away, Approaching, Dwelling, Departing, Away | PASS, observed twice |
| Doors open about 1s into the dwell | PASS, dwell at 10s, doors open by 12s |
| Doors close about 2s before departure | PASS, shut by 18s, departure 20s |
| Doors events strictly inside Dwelling | PASS |
| Board prompt only while doors open | PASS, `Board train`, `CanInteract` true only when open |
| OnPlayerBoarded fires | PASS |
| Run state goes Boarded | PASS, `Run state 1 to 4.` |
| Rounds stop, zombies remaining stops climbing | PASS, held at 6 |
| Weapon reserve to full | PASS, per the engine log line |
| Station heat reads 0 after boarding | PASS, went 1 to 0 |
| Train frozen after boarding | PASS, held Dwelling 10s, zero phase logs after |
| Not boarded: hook fires, heat 0 to 1 | PASS |
| Second cycle not boarded: heat to 2, no more | PASS, observed 0, 1, 2, 3 across four departures, one increment each |

Heat also drove the round manager as designed: live cap bonus 6, 12, 18 and spawn
rate x1.12, x1.24, x1.36 at heat 1, 2, 3.

An incidental observation: boarding succeeded while the run state was Downed.
`ALTTrain::IsRunLive` only excludes Dead and Boarded, so this is what the code
says, but whether a downed player should be able to board is a design question
worth a decision.

### Clean up

PIE stopped. All eight `BP_Train` timing values restored to the class defaults
(30, 100, 25, 4, 4, 1, 3, 15) and confirmed by read-back. `BP_Train`,
`BP_DepartureBoard` and `L_GreyboxTest` saved.

## Milestone 4, roster, types and special rounds: MOSTLY PASS

### Roster wiring

`Roster` is a `TArray<FLTZombieRosterEntry>`, each entry a struct with one
`TypeData` pointer, and it is `EditAnywhere` so it can be set per instance. Both
`GreyboxTest_RoundManager` and `CW_RoundManager` now carry all five entries in
order Walker, Sprinter, Brute, Crawler, Screamer, with `ZombieClass` left as
`BP_Zombie_C`. Both levels saved.

### Types spawn with correct stats and silhouette

| Round | Special | Tag | Composition observed |
|---|---|---|---|
| 1 to 4 | no | None | all Walker, the only type with FirstRoundAvailable 1 |
| 5 | **yes** | **Sprinters** | Sprinter |
| 6, 7 | no | None | Walker |
| 8 | no | None | **Crawler** appears, exactly its FirstRoundAvailable |
| 9 | no | None | 4 Walker plus 2 Crawler, a real weighted mix |
| 10 | **yes** | **SprintersAndBrutes** | Sprinters plus exactly **2 Brutes** |

Round 10 carries the tag `SprintersAndBrutes` because 10 is divisible by both the
sprinter interval 5 and the brute interval 10, and the brute count was exactly 2,
matching `BruteRoundBruteCount`.

Per-type values read off a live brute in PIE:

- armour remaining 200
- capsule half height 130, radius 34
- anim play rate 0.80
- walk speed 204.9, which is the round-scaled base times the 0.73 multiplier
- mesh component relative scale 1.50, 1.50, 1.50

and off a live screamer: walk speed exactly 110.5, which is 130 x 0.85.

Note that `MeshScale` lands on the **mesh component**, not the actor: actor scale
stays 1.0 while the mesh reads 1.5. That is correct, just worth knowing when
checking a brute in the outliner.

### Armour plate: PASS

Four consecutive 50 damage body shots on a brute:

```
body shot 1 -> armour 150 dead=False
body shot 2 -> armour 100 dead=False
body shot 3 -> armour 50 dead=False
body shot 4 -> armour 0   dead=False
```

200 damage absorbed by the plate with no health loss and no death, exactly as
`ArmourBodyDamageToBreak` 200 specifies.

### Tint via material instance: PASS, after a fix

The swap works: a live brute read `MI_Zombie_Brute` on material slot 0.

The first version only covered slot 0. `SKM_Quinn_Simple` has **two** material
slots, so half the body kept the default `MI_Quinn_02`. A second `SetMaterial`
per branch at element index 1 was added, and a fresh PIE confirmed
`slot0=MI_Zombie_Walker slot1=MI_Zombie_Walker`. Ten `SetMaterial` nodes now, two
per type. Still one shared master and five constant instances, no dynamic
instances.

### Screamer: NOT VERIFIED

The screamer's data applies correctly. A live one reads
`Type SCREAMER`, `Behaviour SCREAM` and walk speed 110.5, so `ApplyTypeData` is
doing its job and the asset values are right.

But it never screamed. Tested at 700 units and at 600 units, directly in front of
the player with the camera pointed at it and unobstructed floor between, both
frozen and walking, for eight to ten seconds each time against a
`ScreamLineOfSightSeconds` of 2.0. No `screamed after` log line, so
`OnZombieScreamed` never broadcast and the extra wave and the cancel window could
not be tested either.

Two caveats on that result, stated plainly rather than dressed up:

1. The screamer under test was a **retyped walker**, not one the roster spawned.
   `FirstRoundAvailable` is 12 and reaching round 12 by hand was beyond the time
   available. `ApplyTypeData` is documented as idempotent and the behaviour flag
   did apply, but a retyped actor is not the same as a natural spawn.
2. Runtime actor spawning is not exposed to the editor Python API here, and
   `EditorAssetLibrary.load_asset` returns None during PIE, so the type asset had
   to be fetched through the Asset Registry instead. That cost several attempts
   and is worth knowing for future runs.

So the scream path is **unverified, not proven broken**. It needs either a real
round 12 or a temporary `FirstRoundAvailable` of 1 on the screamer asset to force
a natural spawn. Since the trigger lives in `ALTZombieCharacter::TickScream`,
under `Source/`, this milestone stops here rather than working around it.

### Clean up

PIE stopped. `OpeningRoundCounts` reverted from the test `(4,4,4,4,4)` to
`(6,8,10,12,14)` on the placed instance and confirmed. `BreatherSeconds` reverted
from the test 2.0 to 10.0 on `BP_RoundManager` and confirmed; it is
`EditDefaultsOnly` so it could only be changed on the class, not the instance.
`BP_Zombie`, `BP_RoundManager` and both levels saved.

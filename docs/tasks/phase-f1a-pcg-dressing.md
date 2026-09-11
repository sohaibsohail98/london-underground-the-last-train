# F1a: PCG surface dressing, authoring time only

Lane: Opus in the editor (CC-in-Unreal). No `Source/` change. UE 5.8.
Depends on: F1 (the kit meshes and the Canary Wharf shell) and F2 (lighting),
both done for `L_CanaryWharf_Greybox`. Unblocks nothing: this is polish that
can be skipped without stalling the queue, and it sits behind the H1 items in
`README.md`'s priority order.

Five steps with a **verification gate** after step 3. Steps 0 to 3 build and
prove the smaller of the two scatters end to end (plugin, graph, spline, spawn,
bake, Map Check, navmesh unchanged) before step 4 builds the larger one. Do not
build step 4 on an unproven bake, for the same reason
`phase-f6a-zombie-surface-masks.md` puts its gate where it does: this project
has twice shipped a subsystem that reported success and was not working.

## The boundary, read this before anything else

PCG is used here for **authoring time visual dressing only**: cables, grime,
small litter, decals, scattered onto surfaces that are already fixed by the
time the graph runs.

A broader "generate the game with PCG and MassEntity" proposal was considered
for this project and rejected. This spec is the narrow, surviving part of it.
It must not grow back. Specifically, and permanently out of scope:

- **No PCG for zombie behaviour.** `ALTZombieCharacter` is a full `ACharacter`
  on a jittered repath cadence, already built and already PIE verified,
  deliberately not a behaviour tree and deliberately not MassEntity. The crowd
  cap is 24 and the measured bottleneck is GPU bound, not CPU bound (S6, and
  `../known-issues.md` 2.8), so a rewrite would not even address the real cost.
- **No PCG for zombie spawning.** `ALTSpawnPoint` placement and weighting are
  hand set and are being balance passed in `phase-h1-zombie-pacing-balance.md`.
- **No PCG for station layout or room connectivity.** Both stations are hand
  composed around fixed gameplay beats and a real station identity. See
  `../reference/canary-wharf-grid.md`.
- **No PCG for any gameplay relevant actor placement.** No wall buy, no spawn
  point, no interactable, no light that the readability floor depends on, no
  navmesh affecting geometry of any kind.

Anything scattered by this spec must be removable in one action with no effect
on how the game plays.

## Read first

- `../art-direction.md` section 1, the Materials and Dressing paragraphs, and
  section 6. The house position is restraint: litter drifts in corners rather
  than scattered evenly, wear concentrates where people touch things, fluid
  staining sits near the horde and nowhere else. A PCG graph will happily cover
  a station uniformly, which is exactly the look this project has rejected.
- `../reference/reference-frame-notes.md` sections 1 and 3. Section 1 is what
  the target frame actually shows. Section 3 is the protected material: cable
  runs and grime are fair game, printed matter is not.
- `phase-f1-modular-kit.md` and `Content/LastTrain/Kit/KIT-NOTES.txt` for the
  kit module contract and the two authoring traps F1 hit.
- `../reference/canary-wharf-grid.md` for the tile legend, in particular `W`
  (wall buy anchor), `S` (spawn), `T` (tunnel mouth) and `D` (debris door).
- `phase-h1-zombie-pacing-balance.md` steps 3 and 5. Its finding is that a
  zombie which is hard to read reads as unfair. Clutter that breaks a zombie
  silhouette at range does the same damage as a badly placed spawn point.
- `../known-issues.md` 2.4 (geometry scripting traps), 2.7 (a material can be
  fully wired, save, and still not compile), 2.8 (no frame rate baseline
  exists), 2.9 (three more geometry scripting traps).
- `editor-crash-endplaymap.md`. `MAP CHECK` as a console exec is banned.

## Context: what exists to scatter onto, and what does not exist to scatter

`L_CanaryWharf_Greybox` is 849 actors, of which 701 carry a kit mesh. The
surfaces this spec targets are all F1 kit pieces:

| Surface | Piece | Module |
|---|---|---|
| Ceiling | `SM_Kit_CeilingPanel_400` | 400x400x40, coffered, **two service channels** |
| Wall | `SM_Kit_WallPanel_400` and `_Recess` | 400x150x360, tile line at 220 |
| Floor | `SM_Kit_FloorPanel_400` and `_Tactile` | 400x400x20 |

Room height is 360 uu. The map itself is built on a 150 uu grid, not the kit's
400, so the panels are scaled to the grid (F1 handover row). Do not assume 400
uu spacing anywhere in the map.

The coffered ceiling panel already carries two service channels. That is where
the cable runs belong: in the channel, not bolted across the coffers.

### The cable and junction box mesh audit

Asked and answered, so nobody repeats the search:

- **Nothing cable shaped is committed to this repository.**
  `Content/LastTrain/Kit/Meshes/` holds exactly 12 pieces (wall, floor, ceiling,
  pillar, platform edge, tunnel portal, stair run, bench, bin, handrail and two
  variants) and `Content/LastTrain/Train/Meshes/` holds 6 train pieces. No
  cable, no conduit, no junction box, no pipe.
- **`/Game/UrbanSubway/` does hold wire shaped meshes**, and it is the only
  pool in the project that does. The S3 import listed `wall_wires`,
  `roof_wires`, the `Wall_wire_*`, `Roof_wire_holder_*` and
  `metallic_wire_holder_*` sets, `Pipe_holder_*`, `underground_pipe1` and `2`,
  `wall_metal_gas_pipe`, and a `wires` material instance.
- **They cannot be shipped.** `/Content/UrbanSubway/` is gitignored (line 41),
  as is `/Content/SubwayTrain/`. A baked instance referencing one of them puts a
  reference to an uncommitted asset into a committed `.umap`, which is broken
  for everyone but this machine. They also came in low poly, in a register that
  does not match the F1 kit, and with no lightmap UVs generated (S3 caveat).
- **Conclusion: three new cable meshes and three new litter meshes must be
  modelled.** This is greybox work, not hero work: bundled cylinders and a box.
  Author them the way F1 authored the kit, in engine with geometry scripting,
  and commit them through LFS. Use the `/Game/UrbanSubway/` wire meshes as a
  same day stand in while proving the graph if that is quicker, then swap them
  out before the bake in step 3. Nothing from `/Game/UrbanSubway/` may survive
  into a committed map.

Textures are already here. `Content/LastTrain/Surfaces/SurfaceImperfections001/`
and `SurfaceImperfections013/` are committed CC0 ambientCG sets with BaseColor,
Normal and Opacity, credited in `Content/LastTrain/Surfaces/SURFACE-SOURCES.txt`.
They are the grime decal source. No new download is needed for this task, which
also means `../known-issues.md` 1.1 and the open licence flags in 2.2 and 2.3
do not block it.

---

## Step 0: enable the PCG plugin

1. `LastTrain.uproject` currently enables three plugins: `EnhancedInput`,
   `ModelingToolsEditorMode`, `CommonUI`. Add the Procedural Content Generation
   Framework plugin (`PCG`) and nothing else. Do not enable the MassEntity
   plugins, the PCG geometry script interop or the PCG external data interop:
   none is needed and the first is on the far side of the boundary above.
2. Restart the editor, then confirm from script that the plugin is enabled and
   that `UPCGGraph`, `UPCGComponent` and the `PCGVolume` actor class all
   resolve. A plugin that is listed in the `.uproject` but failed to load will
   let every later step look like it is working until nothing spawns.
3. The `.uproject` is a `Source/` adjacent file, not `Source/` itself, so
   CC-in-Unreal may edit it, but record the change in the handover row so the
   terminal session sees it in the diff.

Read back: the `.uproject` Plugins array has 4 entries, and the class resolve
check returns three valid classes.

---

## Step 1: the six dressing meshes and three materials

All new assets live under `Content/LastTrain/Dressing/`, a new folder, so the
whole pass is one directory that can be deleted wholesale if it is rejected.

### 1.1 Meshes, `Content/LastTrain/Dressing/Meshes/`

Geometry scripting, per `Content/LastTrain/Kit/KIT-NOTES.txt`. Do **not** call
`geometry_uvs(mode="recompute")`: it reports success, leaves `uv_channels` at 1
and renders every surface flat and untextured (`../known-issues.md` 2.4).
Remember that `geometry_create` boxes are centred in X and Y but sit **on**
Z=0, and that `geometry_transform` translate is relative and cumulative
(`../known-issues.md` 2.9).

| Mesh | Size, uu | Notes |
|---|---|---|
| `SM_Dress_CableBundle_200` | 200 long, about 14 across | 5 cylinders of 4 to 6 uu diameter, bundled and slightly irregular. Pivot at the -X end so segments butt end to end with no gap. |
| `SM_Dress_CableSag_200` | 200 long, 14 across, 26 deep | The same bundle with a catenary droop of about 26 uu at mid span. Same pivot rule. |
| `SM_Dress_JunctionBox` | 60 x 40 x 90 | Plain box, a lid lip, a conduit stub at each end sized to swallow the bundle. Pivot on the mounting face. |
| `SM_Dress_Litter_A` | under 30 tall | A drift of paper and card. |
| `SM_Dress_Litter_B` | under 30 tall | Crushed cups and a flattened carton. |
| `SM_Dress_Litter_C` | under 30 tall | Fine scatter: tickets, leaves, grit. |

**The 30 uu height cap on the litter is a gameplay constraint, not an art
preference.** Nothing scattered on the floor may reach a height where it can
break a zombie silhouette at range, per `phase-h1-zombie-pacing-balance.md`.

Every one of the six:

- Nanite on and built, as the kit pieces are.
- **Collision explicitly set to none.** F1 found that geometry scripted meshes
  ship with zero collision shapes, and for the kit that was a defect that made
  the station unwalkable. Here it is the desired state, so make it explicit
  rather than incidental: `CollisionEnabled = NoCollision` on the mesh and, in
  step 2 and step 4, `bCanEverAffectNavigation = false` on the spawner
  descriptor. Dressing that can be shot, walked into, pathed around or swept by
  the interaction trace is a gameplay change, and commit `af0a152` already
  records cosmetic geometry hiding an interactable once.
- A real material slot, created by writing `static_materials` in Python before
  any Lua `configure("material", 0, ...)` call, which fails with "index out of
  range" on a mesh that has none (`../known-issues.md` 2.9).

Legal: original geometry throughout, per `../reference/reference-frame-notes.md`
section 3. The litter carries no legible printed matter. No masthead, no real
ticket design, no operator mark, no wordmark.

### 1.2 Materials, `Content/LastTrain/Materials/Dressing/`

- `MI_LT_Cable`: an instance of the proven `M_LT_PBRSurface` (379 pixel shader
  instructions), tinted to a dull charcoal sheath with a sodium tinged dust
  build up. No new master is needed, so `../known-issues.md` 2.7 cannot bite.
  The junction box takes a second instance, `MI_LT_JunctionBox`, off the
  committed `Metal032` or `PaintedMetal` set.
- `M_LT_GrimeDecal`: this one **does** need a new master, because a deferred
  decal cannot be an instance of a surface material. Material Domain
  `Deferred Decal`, Blend Mode `Translucent`, driven by
  `T_SurfaceImperfections013_Opacity` as the mask and
  `T_SurfaceImperfections013_Normal` for the normal, with `GrimeColour`,
  `GrimeOpacity` and `RoughnessRise` as parameters. **Author it exactly to the
  2.7 procedure**: build and save in one call, compile in a **separate** call,
  then read
  `unreal.MaterialEditingLibrary.get_statistics(mat).num_pixel_shader_instructions`
  and confirm it is **not zero** before wiring it to anything. Zero means the
  shader does not exist whatever the graph looks like, and it has already cost
  this project two sessions.

Read back: 6 meshes exist, all Nanite, all reporting `NoCollision` and
`uv_channels = 1` with a real material slot; `M_LT_GrimeDecal` reports non zero
pixel shader instructions.

---

## Step 2: ceiling cable runs

### 2.1 The splines are hand drawn, not generated

One or more spline actors per station, drawn by hand along the ceiling
following the hall's actual run. **The spline is placement, and placement stays
hand authored.** PCG only decorates it.

There is no stock spline actor class worth relying on here, so make one: a
trivial Blueprint, `BP_LT_DressSpline`, carrying a `USplineComponent` and a
`UPCGComponent`, with no logic and no tick. That also gives the graph its own
component to generate from and a single class to find every dressing spline by.

- Outliner folder `Dressing/F1a`. Names `CW_DressSpline_Cable_01` upward.
- Height: the room is 360 uu and the ceiling panel soffit sits at the top of
  it, so run the spline at **z 320 to 335**, inside the ceiling panel's service
  channel rather than across the coffers.
- **Keep the spline clear of the train's swept volume.** The train roof sits at
  **z 340 with 75 uu of ceiling clearance** (F3 handover row), so a cable at
  z 330 anywhere over the track corridor is inside the volume the train sweeps
  through on arrival and departure. The rule is simple and absolute: **no
  spline point sits over the track corridor at any x.** Cables run over the
  platform, the concourse and the mezzanine only. Confirm by reading `ALTTrain`
  bounds at both its arrived and its away transform and checking every spline
  point against the union of the two.
- **Keep the spline clear of every interactable's collision.** No spline point
  within 300 uu of any actor implementing `ILTInteractableInterface`. The
  interaction component sweeps to `InteractionRange = 250` uu with a 12 uu
  radius, so 300 is 250 plus margin.
- Do not run a spline across a tunnel portal arch. The arch is 330 uu wide and
  it is the doorway the horde walks through: F1 already had to move both portals
  because a jamb was standing in that doorway.

### 2.2 The graph, `Content/LastTrain/Dressing/PCG/PCG_CableRun`

Fed by a `UPCGComponent` on the spline actor itself, not by a volume.

1. **Input** the spline as spline data.
2. **Spline Sampler**, Distance mode, **150 uu** between points. 150 is the
   map's real grid, so the cable joints line up with the panel joints rather
   than beating against them. The bundle mesh is 200 long, so consecutive
   segments overlap by 50 uu and the run reads continuous with no visible butt
   joint.
3. **Transform Points** for the variation. Keep it small: a cable run is
   installed, not strewn.
   - Roll jitter plus or minus 3 degrees, pitch and yaw plus or minus 1.5.
   - Z offset minus 6 to plus 6 uu.
   - Uniform scale 0.97 to 1.03.
   - Absolute position offset along the spline normal, 0 to 8 uu.
4. **Branch the points three ways** off the sampled set:
   - **Bundle segments.** Every point. `Static Mesh Spawner` with
     `SM_Dress_CableBundle_200`.
   - **Junction boxes.** A `Point Filter` or `Density Filter` keeping roughly
     one point in seven, so a box every 900 to 1200 uu. Junction boxes get
     **no yaw randomisation**: a box bolted to a ceiling is aligned to the
     structure, and a randomly rotated one reads as debris. Snap yaw to the
     spline tangent.
   - **Sag spans.** Roughly one point in eight, `SM_Dress_CableSag_200`,
     excluded within 200 uu of a junction box point so a sag never starts
     inside a box.
5. **Static Mesh Spawner settings**, on all three: Nanite on, `Collision
   Enabled = NoCollision`, `Can Ever Affect Navigation = false`, `Cast Shadow`
   on but `Cast Dynamic Shadow` considered against F7's budget, and the mesh
   descriptor set to instanced rather than one actor per point.
6. **Seed.** Set the `Seed` on the PCG component explicitly and write it into
   the handover row. Generate twice and confirm identical instance counts and
   identical first and last instance transforms before going further. A scatter
   that cannot be reproduced cannot be reviewed.

### 2.3 Budget

Hard caps for the whole of `L_CanaryWharf_Greybox`:

| Kind | Cap |
|---|---|
| Cable bundle segments | 400 instances |
| Junction boxes | 60 instances |
| Sag spans | 80 instances |

The map is 849 actors and B3's 60 fps gate is currently **failed** at 26 to 32
fps, GPU bound (S6). There is no frame rate baseline to measure a regression
against (`../known-issues.md` 2.8), so the discipline here is a budget agreed in
advance rather than a measurement taken afterwards. If the pass wants more than
the caps, that is a conversation with the owner, not a number to quietly raise.
Record the final counts for F7.

---

## Step 3: bake, and the verification gate

**Do not go on to step 4 until every line of this passes.**

### 3.1 Bake to static instances, do not ship a generative component

PCG can either regenerate at load and at runtime, or be baked once into fixed
instanced static mesh data that is saved in the map. **Bake.** State it plainly
so nobody leaves it ambiguous:

- A shipped station is placed once. Regenerating its dressing on every level
  load buys nothing, costs load time on a project that already fails its frame
  rate gate, and makes what ships differ from what was reviewed.
- It matches the instinct this project has already applied elsewhere.
  `phase-f6a-zombie-surface-masks.md`'s step 2 is "variation inside a type",
  and its first instruction is to do it **with more static material instances,
  not with a dynamic one**, for the same reason: pay the cost at author time,
  not per frame.
- A baked scatter is reviewable in a diff and in a screenshot. A generative one
  is only reviewable by running it.

Procedure:

1. Set the PCG component's generation trigger to the on demand setting, never
   the runtime one. Confirm the property value by read back rather than by the
   checkbox looking right.
2. Generate in the editor.
3. Bake the generated instances to instanced static mesh actors. Confirm the
   exact affordance name in this engine build before relying on it: in 5.8 it
   is on the PCG component's detail panel, and if it is not, the fallback is to
   select the generated instanced components and convert them to static mesh
   actors by hand. Either route is acceptable; a runtime generative component
   left in the map is not.
4. Clean up the PCG component's own generated output so the baked actors are
   the only copy, then **disable regeneration in the editor** on the component.
   Two copies of a cable run at the same transform is the obvious failure here
   and it is invisible in a screenshot.
5. Keep the graph asset and the spline actors in the level, hidden in game and
   left in the `Dressing/F1a` folder, so the pass is reproducible. Keeping them
   is what makes the bake an authoring step rather than a one way door.

### 3.2 Gate

| Check | Pass condition |
|---|---|
| Duplicate bake | Instance counts after cleanup equal the counts before the bake, not double them |
| Map Check | `MAP CHECKDEP NOCLEARLOG`: 1 error, 0 warnings, the error being the stock "Maps need lighting rebuilt" that every map here carries under Lumen |
| Gameplay actors unmoved | Dump the gameplay actor transform set before and after and hash it with this session's own fold. The two hashes must match. Do not compare against another session's number: F2's fold and F3's fold are different algorithms and gave 787847072 and 3974687351 for overlapping sets |
| Navmesh unchanged | All 10 spawn points return a **complete, non partial** navmesh path to the player start, at the same 2676 to 9452 lengths F1, F2 and F3 all recorded |
| Collision | Every baked instanced component reports `NoCollision` and `bCanEverAffectNavigation = false` |
| Train clearance | `ALTTrain` at its arrived transform and at its away transform intersects no baked instance. Test both, not just the arrived one |
| Interaction | Stand at each of the 4 wall buys and at the boarding point in PIE and confirm the prompt still appears. This is the exact failure `af0a152` fixed once already |
| Budget | Instance counts within the step 2.3 caps |
| Determinism | Regenerate from the recorded seed onto a scratch copy and get identical counts and transforms |
| LFS | Every new `.uasset` is a pointer: `git show <ref>:<path>` piped into `head -c 45` starts `version https://git-lfs...` |
| CI | All five `tools/ci/check_*.py` gates pass |

If any line fails, fix it here. Step 4 multiplies whatever is wrong by an order
of magnitude.

---

## Step 4: wall to floor seam grime and litter

Only after the gate. This is the larger scatter and the one with the real
readability risk, because it sits at floor level in the space the horde walks
through.

The graph is `Content/LastTrain/Dressing/PCG/PCG_SeamDressing`, driven by a
`UPCGComponent` on a single `APCGVolume` covering the station, not by a
per room component.

### 4.1 Which sampler, and why

The target is a band along the wall to floor seam, not a whole floor and not a
single line. Three ways to get it, and the call is the first with the second as
a named fallback:

- **Chosen: `Get Actor Data` on the wall panel actors, bounds driven.** Select
  the wall panel actors, take each as a point, `Bounds Modifier` to extend the
  bounds **60 uu out from the wall face along the panel's local +Y** and clamp
  Z to 0 to 40, then run a `Surface Sampler` bounded by those points and
  projected down onto the floor. The band then follows the walls exactly, in
  every room, with no hand painting and nothing to redo when a wall moves.
  Canary Wharf has 405 wall panel actors, so the `Get Actor Data` query is not
  cheap, but this is authoring time and it runs once.
- **Fallback if that query is too slow to iterate on: hand drawn seam splines
  plus a `Spline Sampler`.** Deterministic and cheap, at the cost of drawing
  the seam by hand in every room and redrawing it whenever the shell changes.
- **Rejected: an unbounded `Surface Sampler` over the whole floor with a
  density falloff.** It scatters the middle of the platform, which is precisely
  the look `../art-direction.md` rejects and precisely the floor the horde walks
  down.

Density: aim for **litter drifting into corners**, per `../art-direction.md`.
Weight density up where two wall runs meet and down along open straight runs.
An even band is the wrong answer even though it is the easy one.

### 4.2 Exclusions, and the mechanism

Every exclusion is layered, because the two available mechanisms fail in
different directions:

- **Class and tag driven exclusion, via `Get Actor Data` into a `Bounds
  Modifier` into a `Difference` node.** Follows the actor. If
  `phase-h1-zombie-pacing-balance.md` repositions a spawn point next week, a
  regenerate puts the exclusion in the right place with no further work. Prefer
  **class** selection over tag selection wherever the class identifies the
  actor, because a tag has to be added to a gameplay actor and this task
  changes no gameplay actor at all. `ALTWallBuy`, `ALTSpawnPoint`, `ALTTrain`
  and `ALTDepartureBoard` are all selectable by class.
- **Hand placed exclusion volumes, tagged `LTDressingExclude`.** Needed for the
  zones that are shaped rather than radial: the boarding strip, the tunnel
  doorways, a sightline corridor. Visible in the outliner, reviewable, and they
  do not touch any gameplay actor. The cost is that they are manual and they
  drift if the thing they protect moves, which is why they carry the shapes and
  not the point anchors.

Do **not** use a hand painted exclusion mask. It is invisible in a diff, it
cannot be read back from script, and it silently goes stale.

The exclusions for `L_CanaryWharf_Greybox`:

| Zone | Mechanism | Buffer |
|---|---|---|
| Every `ALTWallBuy` (the 4 `W` tiles) | Class | 300 uu radius, floor to 220 uu. `InteractionRange` is 250, plus margin |
| Every `ALTSpawnPoint` (all 10) | Class | 450 uu radius. A zombie must read cleanly the instant it appears |
| `ALTTrain` swept volume and the boarding strip | Class, plus a hand volume | The whole track corridor, plus a 300 uu band inboard of the platform edge along its full length |
| `ALTDepartureBoard` and its two panels | Class | 200 uu radius |
| Both tunnel portal doorways | Hand volume | The full 330 uu arch plus 250 uu either side, full height, at the measured gap centres x 4125 and x 7125 on the y 6225 wall row |
| Every `D` debris door gap | Hand volume | The gap plus 200 uu either side. These open up as the run progresses and become walked routes |
| Player start | Class | 300 uu radius |
| The horde approach lanes down the platform | Hand volume | Nothing over 30 uu tall anywhere in the lanes from either tunnel mouth down the platform. This is the `phase-h1-zombie-pacing-balance.md` constraint made geometric |
| Any actor implementing `ILTInteractableInterface` not already listed | Class | 300 uu radius |

Build the exclusion set **first**, generate with the spawners disabled, and
screenshot the surviving point cloud from above. Look at where the points are
before any mesh is placed. That picture is the review, and it is much easier to
read than the finished scatter.

### 4.3 What gets spawned

- **Litter meshes**, `SM_Dress_Litter_A/B/C`, via `Static Mesh Spawner` with
  the same instanced, `NoCollision`, no navigation settings as step 2.2 item 5.
  Random yaw is correct here: litter is strewn. Cap **500 instances**.
- **Grime decals**, `M_LT_GrimeDecal` instances, spawned along the same seam at
  a much lower density. Decals are per instance draws and they are not free, so
  cap them at **120 across the map** and set a Fade Screen Size so the small
  ones drop out at distance. Concentrate them under the tile line, where
  `../reference/asset-sources-phase-f.md` section 1.2 already says grime reads
  best, and near the flooded zone.
- Nothing else. No props, no signage, no furniture, no lights. Furniture is
  hand placed kit, per F1.

Then run step 3's bake procedure and step 3.2's gate again, in full, on the
combined scatter. The gate is not a one time check.

---

## Explicitly not in this spec: dressing keyed to gameplay data

The original discussion included an idea for **medical debris scattered near
Screamer spawn points**. It is not in this spec and it must not be added to it.

It ties dressing density to gameplay data (`ULTZombieTypeData`, the round
roster, which spawn points are live at which round) at a point where the graph
would have to run at runtime or per round. That is exactly the runtime coupling
this whole task is scoped to avoid: a station whose dressing changes with the
round composition is a gameplay adjacent system, not authoring time set
dressing, and it lands straight back on the far side of the boundary at the top
of this file.

It is also not free. A per round regenerate on a map that already fails its
frame rate gate, with no baseline to measure against, is a cost nobody can
currently price.

If it is genuinely wanted, it is a separate and larger proposal needing an
owner decision, in the same class as the standing rule in `CLAUDE.md` about
pausing before a new architectural dependency. It is not something to fold into
a dressing pass.

---

## Rules

- No `Source/` change. The only non content file this task touches is
  `LastTrain.uproject`, for the plugin, in step 0.
- No gameplay actor is moved, retagged, reparented or edited. Prove it with the
  transform hash, both before and after.
- `MAP CHECK` as a console exec is banned. Use `MAP CHECKDEP NOCLEARLOG`. See
  `editor-crash-endplaymap.md`.
- Never mutate an actor or asset while PIE is running. Stop PIE cleanly first.
- Nothing from `/Game/UrbanSubway/`, `/Game/SubwayTrain/` or `/Game/CitySample/`
  survives into a committed map. All three are gitignored.
- Do not commit anything from `_incoming_assets/`.
- Commit the new `.uasset` files and the changed `.umap` through LFS, and verify
  each is a pointer.
- British spelling, no em or en dashes, in any new asset note or text file.
- Update the F1a row in `handover.md` and the Phase F table in `README.md`.

## Accept

- The PCG plugin is enabled and the three PCG classes resolve.
- 6 new dressing meshes and 3 new materials exist under
  `Content/LastTrain/Dressing/` and `Content/LastTrain/Materials/Dressing/`,
  `M_LT_GrimeDecal` reporting non zero pixel shader instructions.
- Cable runs read as installed services in the ceiling channel from the
  reference frame camera position, with junction boxes at structural intervals
  and no cable anywhere over the track corridor.
- Litter drifts into corners and along the wall seam and does not cover the
  open platform.
- Every exclusion zone in 4.2 is visibly clear: photograph each wall buy, each
  tunnel doorway and the boarding strip.
- A zombie silhouette is unbroken at 3 distances down each approach lane.
- Everything in the step 3.2 gate passes on the final, combined, baked state.
- Total new instances within budget: 540 for the cables, 620 for the seam, and
  the actor count rises by the small number of baked instanced actors, not by
  a thousand.
- A full round plays in `L_CanaryWharf_Greybox` with rounds, spawning, the wall
  buys, the train cycle and boarding all behaving exactly as they did before.

## Then, and only then, Paddington

`L_GreyboxTest` becomes Paddington in `phase-f-greybox-station.md`, and that
pass reuses this graph rather than rebuilding it: new splines, new exclusion
volumes, the same `PCG_CableRun` and `PCG_SeamDressing` assets, the same six
meshes. Reuse is the point of having built it as a graph at all. Do not start
it before Canary Wharf's bake has passed the gate.

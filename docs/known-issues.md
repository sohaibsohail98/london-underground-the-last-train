# Known issues

Open problems, environment limits and unverified work. Kept so a cold session
does not rediscover them and so nothing quietly rots.

**Update this whenever an issue is closed or a new one is found.** An issue that
is fixed gets deleted from here, not ticked: git history is the record.

Last updated 2026-09-11.

---

## 1. What the remote session cannot do

These are hard limits of the remote Claude Code lane, not bugs. They shape what
that lane can be asked for. See `docs/tasks/who-does-what.md` for the split.

### 1.1 No network route to any asset host

The remote container's egress proxy allows GitHub, the Anthropic API and the
language package registries. **Everything else is refused at CONNECT with a
403:** ambientcg.com, polyhaven.com, quaternius.com, kenney.nl, freesound.org,
itch.io, wikimedia.org. Both `curl` and the agent's own page-fetch tool hit the
same wall.

Web search still works. So the remote lane can **research** assets and cannot
**download** them.

**Consequence for task specs:** never ask the remote session to fetch, stage or
download a file. Ask it for a manifest, and run the fetch here.

### 1.2 Nothing staged in the remote container can reach this machine

`_incoming_assets/` is gitignored (`.gitignore` line 80), committing assets is
forbidden, and the container is reclaimed when the session ends. A file the
remote session downloads has no route out **even if 1.1 were fixed.**

This is the more fundamental of the two, and it applies to any gitignored
output, not just assets.

**It holds in the other direction too, which is easy to forget when writing a
task spec.** Nothing staged on the Mac under `_incoming_assets/` is visible to
a remote session: it is gitignored, so it is not in the clone the container
gets. A spec that tells the remote lane to "read the staged pack and its
`SOURCES.txt`" cannot be followed. Phase G1 hit exactly this: the audio spec
had to be written from the repo's own research docs instead, and
`docs/tasks/phase-g1-audio.md` step 0 is a reconciliation pass against the real
`SOURCES` files, to be run here. **Consequence for task specs:** anything the
remote lane must read has to be in git.

### 1.3 No compile

Unreal cannot be installed in the container, and the external Xcode mount is not
there. The remote session writes C++, and the human or the self-hosted runner
compiles it with `./tools/ci/compile.sh`. Already stated in `CLAUDE.md`;
repeated here because it is the same class of limit as the two above.

**Consequence:** the remote lane should not push C++ it cannot at least reason
its way to confidence about, and a small change beats a large one. Where a C++
addition would be nice but is not required, propose it in the spec rather than
writing it blind. `docs/tasks/phase-g2-hud.md` does this with
`GetReloadProgress()`.

### 1.4 The five CI gates are not a compile

`tools/ci/check_*.py` catch style, spelling, dashes, LFS pointer integrity and a
stand-in subset of reflection errors. **A green gate run says nothing about
whether the module builds.** `check_cpp_reflection.py` is explicitly the
compiler stand-in and covers only missing `GENERATED_BODY()`, a reflected header
not including its own `generated.h`, a `.cpp` not including its own header
first, and `LT_LOG` format specifier mismatches.

---

## 2. Open items from the 2026-09-09 remote session

### 2.1 `fetch-phase-f.sh` has never run against a live host

`tools/asset-fetch/fetch-phase-f.sh` was written in the remote container, which
cannot reach ambientcg.com or polyhaven.com. It is verified for **syntax, dry
run, and graceful failure against the blocked proxy** and nothing else.

Specifically unproven:

- ~~The ambientCG zip URL shape `https://ambientcg.com/get?file=<ID>_2K-JPG.zip`.~~
  **Confirmed working 2026-09-09 during F1.** `Tiles036` and `Tiles133B` were
  both fetched from the live host with exactly that URL shape, HTTP 200 and
  about 14 MB each. The v2 CSV API fallback is still unproven, and has not been
  needed.
- Whether all 36 ambientCG asset IDs resolve. `TactilePaving003`, `004` and
  `005` were **inferred from the family naming pattern**, not seen.
- The Poly Haven API response shape the script parses, and whether all 22 slugs
  exist.

**Owner: whoever runs it first.** It fails per-asset and reports what failed, so
a wrong URL costs a line in the failure list rather than the run. Expect to fix
something on the first go.

### 2.2 Every licence in the Phase F manifest is unverified

`docs/reference/asset-sources-phase-f.md` rests on web-search snippets. **No page
was opened and no file was looked at.** Only two per-file licences in the whole
document were confirmable from search evidence, both Wikimedia Commons files in
section 4.3.

Confirm the licence on the page before any file is used. Wikimedia Commons and
Freesound in particular are mixed-licence: the licence is a per-file fact, never
a per-site fact.

### 2.3 Four unresolved legal flags on Phase F assets

Listed in full in section 6 of `docs/reference/asset-sources-phase-f.md`. In
brief: the signboards and posters in the Modular Underground Metro pack; the bus
stop in City Environment Pack #2; photogrammetry scans that may carry painted
markings (ambientCG `Road001` named specifically); and the FOI re-use notice
attached to the Class 345 drawings, which nobody has read.

None is likely to be a problem. All four need eyes before the file is staged.

### 2.4 Two NeoStack geometry-scripting traps, found in F1

Both were hit, diagnosed and worked around during F1 on 2026-09-09. They are
recorded here because they are properties of the tooling, not of this map, and
the next kit or prop task will hit them again.

- **`geometry_uvs(mode="recompute")` destroys usable UVs.** It returns success
  and the built asset still reports `uv_channels = 1`, but every surface
  renders flat and untextured. `geometry_create` already emits good UVs, so the
  fix is simply not to call it. Proved by A/B test: a raw `geometry_create` box
  and an `/Engine/BasicShapes/Cube` both textured correctly under the same
  material instance while the recomputed kit panel did not.
- **Geometry-scripted static meshes ship with zero collision shapes.** With the
  default `SimpleAndComplex` trace flag that makes them invisible to navmesh
  generation, so a map built from them is entirely unwalkable and no spawn
  point can path anywhere. Add simple box collision per piece, or set
  `CTF_UseComplexAsSimple` where a bounding box would seal an opening (the
  tunnel portal) or flatten a walkable profile (the stair run, the platform
  edge).

Neither is a NeoStack bug report yet: no minimal reproduction has been filed.

### 2.5 `docs/art-direction.md` section 7 is stale

It says Overpass "is not present in the project or the engine". The OFL faces
were imported in the S9 haul and are in `Content/LastTrain/UI/Fonts/`
(`Font_UI_Overpass`, `Font_UI_OverpassMono`, `Font_UI_Barlow`,
`Font_UI_BarlowCondensed`, `Font_UI_PublicSans`).

Fixing it is in the accept list of `docs/tasks/phase-g2-hud.md`, so it closes
with G2. Flagged here in case anything reads section 7 before then.

### 2.6 Two owner decisions are blocking nothing yet, but will

Both are stated in `docs/tasks/phase-g2-hud.md`:

- **The typeface split.** `WBP_MainMenu` uses Barlow plus Public Sans, the HUD
  spec calls for the Overpass pair. Three families across two screens is not
  defensible. The spec recommends one display face (Barlow Condensed, titles
  only) and one functional pair (Overpass, Overpass Mono).
- **The station schematic.** The Phase G plan lists it as the HUD's map
  substitute. The G2 spec argues it should be cut from the HUD entirely while
  v1 ships two stations, and built as diegetic platform signage in F5 instead,
  where it also does the legal job of replacing the official line diagram.

### 2.7 A material can be fully wired, save, and still not compile

Found in F2 on 2026-09-09. `M_LT_PlatformWet` was authored through
`MaterialEditingLibrary` with all four outputs connected and 31 nodes present,
and `read_graph` plus `get_material_property_input_node` both reported it
healthy. It rendered as an untextured default anyway.

The cause: a `recompile_material` call **hit the NeoStack 60 second
`execute_script` timeout mid-build** and left the material compiling to **0
pixel-shader instructions**. The symptom that identified it was setting a
deliberately near-black tint on the instance and seeing **no change at all** in
the render.

**The check:** `unreal.MaterialEditingLibrary.get_statistics(mat)` and read
`num_pixel_shader_instructions`. A healthy comparable material,
`M_LT_PBRSurface`, reports 379. Zero means the shader does not exist, whatever
the graph looks like.

**Consequence for authoring:** do not call `recompile_material` in the same
`execute_script` payload that builds the graph. Build, save, then compile in a
separate call, and verify the statistic before wiring the material to anything.

**Closed 2026-09-09 by F3.** The broken asset was deleted from disk as F3's
first editor action on a fresh editor start, exactly as prescribed, and the
working tree was clean afterwards. The trap itself stands: `delete_asset`
refuses an asset held in memory, so a dead material still has to be removed
from disk after an editor restart.

F3 authored one new master, `M_LT_TrainEmissive`, to this procedure and it
reports **96** pixel-shader instructions. Its eight sibling livery materials
avoided the risk entirely by instancing the already-proven `M_LT_PBRSurface`
(379), which is the cheaper move whenever an existing master will do.

### 2.8 No frame-rate baseline exists for Phase F

F2 was asked for a rough fps figure at the reference camera and **could not
produce a trustworthy one, so none is recorded.** Guessing was not an option and
a wrong baseline is worse than none.

In PIE driven over the MCP bridge the world delta reported to Blueprint pins to
a dead-flat `0.33333 s`, which is exactly 3 fps. Neither `t.MaxFPS 0` nor
`r.Editor.ThrottleCPUWhenNotForeground 0` shifts it. But game real-time advances
`8.001 s` over an `8.0 s` wall-clock wait, so **the game clock is running at true
speed and it is the smoothed delta that is capped**, not the renderer. `stat
unit` and `stat fps` do not render into either the level-viewport or the PIE
screenshot path, so the overlay cannot be read back either.

**Owner: F7.** Measure with the editor focused, from the reference camera, with
a round running. Until then Phase F has no perf number and nothing should claim
one.


### 2.9 Three geometry-scripting traps F3 hit

Found in F3 on 2026-09-09, building the train exterior. All three cost real time
and none is obvious from `help()`.

**Primitive origin and cumulative transforms.** `geometry_create` boxes are
centred in x and y but sit **on** z=0, base at the origin, not centred. And
`geometry_transform` translate is **relative and cumulative**, so applying an
absolute-looking offset to an already-placed mesh moves it again. Getting both
wrong at once silently put a body section 168 uu too high.

**A recessed opening shows nothing behind it.** The first train body recessed
the window band 10 uu into the skin instead of cutting through, so the lit
interior behind it was invisible and the cause was not obvious from any query:
the mesh reported closed, 1 component, correct bounds. If something must be seen
through, the cut tool has to be **thicker than the wall**. When it is, the
collision then has to be **two side-wall boxes rather than one body box**, or
the aperture is sealed to the player even though it renders as open.

**A geometry rebuild drops the material slot.** Re-saving a mesh through
`geometry_boolean` leaves it with zero material slots, and
`configure("material", 0, ...)` then fails with "index out of range". The slot
has to be created by writing `static_materials` through Python first; only then
does the Lua `configure` path work. This is the same family as the F1 finding
that geometry-scripted meshes ship with **zero collision shapes**.

### 2.10 Two Blueprint traps that cost F3 its door animation

Found in F3c on 2026-09-10, finishing the train doors. F3 shipped the doors
inert because of the first of these; the second was only found in PIE.

**A Static mesh component silently ignores `Set Actor Location`.** The 24 door
leaves were placed as ordinary `StaticMeshActor`s, so their mesh components
defaulted to **`Static` mobility**. At runtime a Static component cannot be
moved: the call is dropped with **no error, no warning, and no log line**. The
Blueprint compiled 0/0, the timeline ran to completion, the arrays held correct
values, and the leaves did not move. Set the component to `Movable` for anything
that has to move in play. This is the same family as the F2 Static-mobility
ceiling-fitting note, and it is worth checking mobility **first** whenever a
transform write appears to do nothing.

**The plain float add is only reachable through `|Utilities|Operators`.**
`find_nodes` returns **0 results** for `Add_FloatFloat`, `Add_DoubleDouble`,
`float + float` and `Adds two floating point`. The node that works is result
**1** of `find_nodes("Add")`, in category `|Utilities|Operators`, reported
against `TimeManagementBlueprintLibrary` and spawning with the title
**"FrameNumber + Int"** and three **wildcard** pins. It is the promotable
operator: connect a `real` pin to `A` and the node retitles itself
**`float + float`** with every pin resolved to `real`. F3 recorded this lookup
as a blocker; it is not, but the entry is easy to read past.

**Diagnosing a silent Blueprint failure: use probe variables, not the log.**
`playtest_log_contains` returned `found=false` even for the string "PIE", so
`PrintString` output could not be used as evidence of anything. What worked was
adding temporary Blueprint variables and reading them back with
`playtest_read_state`: an int set on the C++ hook proved the hook fires, and a
real set from the timeline's `Update` proved the timeline runs. That narrowed a
five-link chain to the one failing link in two PIE rounds. Remove the probes and
recompile once the cause is found.

---

### 2.11 The F3 bodyshell blocks the boarding interact

Found on 2026-09-10 in the F3-completion follow-up, the PIE session that
finally exercised boarding. **Boarding was unreachable in
`L_CanaryWharf_Greybox`.** This was a real gameplay defect, not a test artefact.

**Primary fault fixed 2026-09-10 (`Source/`).** `ULTInteractionComponent` now
sweeps multi and takes the nearest hit that implements
`ULTInteractableInterface`, not the nearest hit of any kind, so a cosmetic
Visibility blocker in front of an interactable (the F3 bodyshell panel, a
pillar in front of a wall buy) is skipped rather than ending the search.
Compiled clean.

**Re-verified 2026-09-10, terminal session, automation test.** A new editor
test module (`Source/LastTrainTests/`, test
`LastTrain.Boarding.TrainInteractableAndBoard`) drives a real PIE session on
`L_CanaryWharf_Greybox`, waits for the train to reach `Dwelling` with doors
open, stands the player at the platform standing spot facing the train, and
asserts `ULTInteractionComponent::GetCurrentInteractable()` resolves the train,
then calls `TryInteract()` and asserts the run state flips to `Boarded`. Run via
`AutomationTestToolset.RunTests` (Epic's own `ModelContextProtocol` plugin, not
NeoStack, see `neostack.md`): **passed=1, failed=0.** `CurrentInteractable`
correctly resolves `BP_Train_C_0`. The range caveat below did not need a
spawn-point nudge: the test's stand spot (150 uu out along the train's right
vector from its centre) sat within range once the occluder stopped blocking the
sweep. C1 and C3 are now verified working end to end.

**Symptom.** Standing on the platform facing the train through a full 22 second
dwell, `ULTInteractionComponent::CurrentInteractable` never populates. The
"Board train" prompt never appears, so the interact cannot be taken and the C3
travel path never runs. Sampled every 1.2 s from the first frame of `Dwelling`
to `Departing`: empty on every sample.

**Cause.** `ULTInteractionComponent` sweeps a 12 uu sphere on `ECC_Visibility`
for `InteractionRange` 250 uu from the pawn's eyes, takes the **single blocking
hit**, and only then asks whether that actor implements
`ULTInteractableInterface`. The first thing the sweep hits is
`CW_F3_Body_07`, an F3 bodyshell panel, whose face sits at **y 5126**. It is a
plain `StaticMeshActor` and implements nothing, so the sweep stops there and the
train is never considered. `ALTTrain` is on `CW_Train`, whose only
Visibility-blocking component is `CarriageMesh` at **y 5200**. The F3 shell was
placed 74 uu in front of the actor that owns the interact, and hides it.

Measured from the player's real eye point, control rotation forward:

| Ignore list | First blocking hit |
|---|---|
| nothing | reach ~205 uu, `CW_F3_Body_07` |
| `CW_F3_*` | reach ~275 uu, `CW_Train` |

Ignoring `CW_F3_Body_07` alone takes the hit count to 0; ignoring `CW_Train`
alone leaves it at 1. That is the proof of which actor blocks.

**The second half of the problem.** With the shell ignored the train is at
**275 uu**, which is still outside `InteractionRange` 250. So even removing the
occluder is not sufficient on its own from where the player can comfortably
stand. The player can close to roughly y 5076 before the shell stops the
capsule, which puts `CarriageMesh` 124 uu away and inside range, so the shell is
the primary fault, but the margin is thin.

**`BoardingVolume` cannot help.** `ALTTrain::ALTTrain` sets it `QueryOnly` with
`ECR_Overlap` on all channels. An overlap response does not block a sweep, so
the volume is invisible to `SweepSingleByChannel` and contributes nothing to
finding the interact. Its comment says it only scopes where the aperture is.

**Fix applied.** The interaction component now sweeps multi and picks the first
hit that implements the interface rather than the first hit of any kind
(`SweepMultiByChannel` in `ULTInteractionComponent::TickComponent`). This was
the most general of the candidates and survives any future geometry in front of
the train. The three editor-side alternatives (strip Visibility collision from
the doorway panels, move the interact proxy to the doorway plane, make
`BoardingVolume` block Visibility) were not needed and were not done.

**Still open: the range caveat.** With the occluder no longer stopping the
sweep, the train sits at ~275 uu from a comfortable standing spot, just outside
`InteractionRange` 250. The player can close to ~124 uu before the shell stops
the capsule, so boarding is reachable, but the margin is thin. If the PIE
re-verification finds the prompt flickers or needs the player pressed against
the shell, nudge the platform spawn point forward or move the interact proxy to
the doorway plane.

**Also seen, unrelated and minor.** The HUD points label renders an unlocalised
text key, for example `Mac-5F869E425B10C8`, where the station or player name
should be. Visible in every PIE screenshot from this session.

---

### 2.12 The F3 door apertures were recesses, fixed 2026-09-10

Opened building F4, **closed the same day** by re-authoring the mesh. Kept as a
short record because the fix changed which asset the 12 bays point at.

**What was wrong.** `SM_Train_DoorBay` recessed the door bay into the bodyside
but never cut the aperture through, so with the leaves open a player at the
doorway looked at solid skin. Traces at door height stopped at `y 5126` while a
control trace through a window reached past it: the window band was cut, the
doorways were not.

**Fix.** The bodyside turned out to be a **zero-thickness surface** at `y 5126`,
not a solid wall, so no boolean was needed: the near skin was re-tessellated as
a ring of panels around the aperture. The mesh was authored as an OBJ and
brought in with `StaticMeshTools.import_file`, which this bridge does expose
even though it has no geometry-scripting or boolean tools. New asset
`SM_Train_DoorBay_Open`; all 12 `CW_F3_Body_*` actors now point at it and the
old `SM_Train_DoorBay` is left in the tree untouched as the record.

Collision was rebuilt as **four boxes**: the far wall full length, two near-wall
pieces flanking the aperture, and a header above it. The original had two boxes,
one per wall, but each spanned the whole bay **including the doorway**, which is
2.9's "sealed to the player even though it renders as open" in its exact form.
Auto-generated convex collision from the import was discarded first.

**Verified.** Past the leaf plane, traces through all 12 doorways reach the F4
interior (`y 5200` to `5372`); the control trace at solid bodyside beside each
doorway still stops at `y 5136`. Confirmed visually in PIE with the train
dwelling and the leaves parted: the aperture is a clear opening with the violet
surround and reveals, no skin behind it.

**Import traps worth keeping.** `import_file` will not overwrite an existing
asset (it errors, so a re-author needs a new name), and an OBJ with no normals
or UVs imports with "degenerate tangent bases" and "nearly zero bi-normals"
warnings. Writing one normal per face plus planar UVs clears both.

**Still open after a lighting pass on 2026-09-11.** F4's "bright, high, airy"
half of the acceptance is **still not met**. What the pass established:

- **Light placement is not the cause.** Every one of the 12 vestibules has a
  ceiling point light within 25 to 175 in x, all inside the 520 attenuation
  radius, and the 6 `CW_F4_LED_*` runs per side tile `x 1050` to `10050` with no
  break at any doorway. Coverage over the vestibule zone was never missing.
- **`CarriageMesh` is ruled out as a visual cause, verified not assumed.** It
  reads `bVisible false` with `bCastHiddenShadow false`, so it casts no shadow
  while hidden. It still blocks traces, as 2.11 needs. It spans `x 4550` to
  `6550`, straddling vestibules 05 to 07, so those three are poor probe sites:
  use 01, 02 or 12 instead.
- **All 151 F4 surfaces had `bEmissiveLightSource` false**, so the `*Glow`
  instances rendered bright but contributed nothing to Lumen. The saloon looked
  lit only because 52 point lights at 1400 cd lit it directly. The flag was set
  true on the 24 fittings that should emit (12 `CW_F4_LED_*` runs, 12
  `CW_F4_VestCeil_*`), read back true on all 24, and the level is saved. The
  other 127 pieces (walls at `Brightness` 3.4, floors at 1.2) were deliberately
  left non-emitting: turning the whole shell into emitters is what produced the
  blown-out white of the previous attempt.
- **That change did not fix the acceptance.** Verified in PIE, train `Dwelling`,
  leaf fully open (`CW_F3_DoorL_01` `xmin 1281` against a closed 1353), phase
  re-read as `Dwelling` after the capture.

**Geometry occlusion is ruled out, checked 2026-09-11.** The question was whether
the door leaf or the bodyside occludes the aperture at platform eye height. It
does not, on four independent measurements:

- `SM_Train_DoorBay_Open`'s asset thumbnail shows the doorway aperture cut
  through the bodyside panel, with the window band either side.
- Traces from `y 5128` (just past the bodyside plane) inward reach the far wall
  at `y 5372`, at bays 01, 02, 03 and 12, at `z 100`, `150` and `200`. 2.12's
  original verification reproduces exactly.
- The mesh's four collision boxes map to world `x 1050` to `1353` and `x 1497`
  to `1800` on the near wall, leaving a **144-wide gap at `x 1353` to `1497`**
  with only a header above `z 266`. Collision does not seal the doorway.
- Leaf travel is 72 (`doorClosedX` 1389, `doorOpenX` 1317). Open, the leaves sit
  at `x 1281` to `1353` and `x 1497` to `1569`, each still overlapping the
  `1324` to `1526` reveal span by 29, leaving 144 of 202 clear. That matches the
  collision gap. The leaves slide clear of the opening.

**What the doorway actually looks like during dwell.** A close capture from the
platform at `x 1425, y 5080, z 150`, doors open, shows the two leaves as dark
slabs left and right with the opening between them running floor to ceiling, the
lit window band above, and **the interior visible through the gap but dim and
brown**, not the near-white the saloon reads as from inside. The earlier
"solid dark slab" reading in this section was taken from `y 4990`, too far out
and too wide a framing to resolve the opening, and was wrong.

So the aperture is open and the interior is visible through it.

**Direct light reach is ruled out too, measured 2026-09-11. The vestibule is
better lit than the saloon.** Correcting an error earlier in this section
first: **there are no vestibule lights.** All 52 are `CW_F4_SaloonP_*` and
`CW_F4_SaloonF_*` at `y 5185` and `y 5345`, two continuous rows the length of
the carriage. The earlier claim of "a light within 25 to 175 of each vestibule"
measured **x-distance only** and ignored y, which made the saloon rows passing
overhead look like dedicated local fittings. They are not.

Contribution at `z 150` under one falloff model, same 52 lights:

| sample point | reach | lights in radius |
|---|---|---|
| vestibule mid `1425, 5200` | 553 | 6 |
| vestibule back `1425, 5300` | 565 | 6 |
| vestibule mouth `1425, 5150` | 463 | 6 |
| saloon centre `1700, 5265` | 366 | 6 |
| saloon platform side `1700, 5185` | 332 | 5 |

The vestibule gets about 1.5x the saloon mid-panel's direct contribution, and
every vestibule centre has its nearest light 142 to 224 away in true 3D, well
inside the 520 radius. Line of sight is clear on all 8 traces from the two
nearest lights into 4 vestibule sample points, while the saloon control traces
hit bench backs and near-wall panels at 32 and 48. The vestibule is the least
obstructed part of the carriage, being the gap between seating bays.

**No change was made.** There is no underpowered vestibule light to raise, and
raising the saloon rows would brighten the already-brighter region and blow out
the saloon.

**Exposure is ruled out, measured 2026-09-11. F4 closes here, unfixed.**
The suspicion was that the platform's hard `CW_F3_TrainWash_*` spots were
driving auto-exposure to adapt and crushing the interior by comparison. They are
not, because nothing in this level adapts.

There is exactly **one** post process volume in the map, `PostProcessVolume_0`
(the actor labelled `CW_D_PostProcess`), found both by label and by a
class-wide sweep, so there is no second volume and no camera-component override
competing with it. It reads `bUnbound true`, `blendWeight 1`, `bEnabled true`,
`priority 0`: it applies to every camera in the level, platform and interior
alike, through one identical transfer curve.

Its exposure settings, read off the `Settings` struct with the override flags
that decide whether a value does anything at all:

| property | value | overridden |
|---|---|---|
| `autoExposureMethod` | `AEM_Manual` | true |
| `autoExposureMinBrightness` | 1 | true |
| `autoExposureMaxBrightness` | 1 | true |
| `autoExposureBias` | -1.35 | true |
| `autoExposureApplyPhysicalCameraExposure` | false | true |
| `bOverride_LocalExposure*` | all defaults | all false |
| `bOverride_HistogramLogMin/Max` | -8 / 4 | both false |

Manual metering does not read scene luminance. Min and max brightness are both
pinned to 1, so adaptation is frozen even arithmetically. Local exposure, the
one feature that could darken a dim region against a bright one inside a single
frame, is entirely unoverridden. `r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange`
is `False` in `Config/DefaultEngine.ini`, so those brightness numbers are not
EV100-scaled.

**The measurement that settles it.** Two editor viewport captures, same map,
same unbound volume, same 90 degree FOV, seconds apart. Mean and median of
Rec. 709 luma over the frame:

| capture | pose | whole-frame mean | median | middle band mean |
|---|---|---|---|---|
| saloon, from inside | `1700, 5265, 150` yaw 180 | 164.4 | 226.2 | 178.8 |
| doorway, from platform | `1425, 5080, 150` yaw 90 | 14.7 | 1.2 | **1.88** |

The aperture region reads mean 1.88, median 1.21, max 11.91 out of 255: not
"dim brown", effectively black. The same geometry from inside sits at median
226. A single global exposure setting cannot produce that split, because a
uniform bias scales both frames together; the interior-to-platform ratio within
a frame is what is wrong, and no global exposure control touches a ratio. The
doorway frame does show a blown platform floor strip (lower band p95 233) and a
lit band at the top, so the tone mapper is passing bright surfaces through
normally. Almost no interior light is reaching the camera through the aperture.

**No change was made.** Raising `autoExposureBias` off -1.35 would lift the
whole frame, platform included, and make a capture look brighter without
touching the mechanism. That is a cosmetic edit to a setting that is not the
cause, so it was not done. The volume is untouched and the map was not saved.

**What is left**, now two candidates rather than three:

1. **Glow parameter levels.** `MI_TrainInt_FloorGlow` `Brightness` 1.2 and
   `MI_TrainInt_WallGlow` 3.4 against `MI_TrainInt_LEDStrip` 34. The surfaces
   facing the doorway may simply be dim by parameter.
2. **Lumen indirect** in the recess. The saloon's brightness is carried by 51
   direct point lights that do not throw toward the doorway plane; whatever
   should fill the vestibule is indirect, and the measurement above is
   consistent with that indirect contribution being near zero.

Owner: F4 follow-up. Do not re-run the light-placement, `CarriageMesh`,
geometry-occlusion, direct-light-reach, or exposure checks; all five are
settled above.

**Count correction.** A label sweep for `CW_F4_Saloon` returns **51** point
lights, not the 52 stated further up this section. One sampled component reads
`intensity 1400`, `Unitless`, radius 520, `Movable`, matching the recorded
figures exactly, so the map has not diverged: the 52 is a miscount, not a
missing light.

**Trace caution.** `SceneTools.trace_world` tests collision, not visibility. A
platform-side trace at door height stops at `y 5126` across the whole bay even
with the doors open, because it meets the near-wall collision boxes and the
bodyside plane. Do not read a stopped trace as "the view is blocked": the mesh
renders the aperture open where collision reports solid. Confirm every visual
claim with a capture.

There is no 2600 cd light in the level. All 52 point lights read exactly
`1400 cd`, `Movable`, radius 520. An earlier interrupted attempt is recorded as
having pushed one to 2600 cd, but that was never saved and is not present.

Map Check remains unavailable: no Map Check tool on any registered toolset, no
`LogMapCheck` category, no MapCheck automation test. Unchanged from F4 and F3f.

### 2.13 Two editor-scripting traps F4 hit

Found on 2026-09-10 building the train interior. Both cost real time.

**`/Engine/BasicShapes/Cube` is centred on all three axes.** 2.9 records that
`geometry_create` boxes sit **on** z=0 with the base at the origin. The engine
basic-shape Cube does not: it is centred in x, y **and** z, so placing a slab by
its intended base z puts it half its own height too low. Place by the centre
(`(z0+z1)/2`) and scale by `(z1-z0)/100`. The first five F4 shell pieces all
landed half-height low and the walls landed catastrophically low, because the
z convention was assumed from 2.9.

**`ObjectTools.set_properties` takes `values`, not `properties`.** The read side
is `get_properties(instance, properties=[...])` but the write side is
`set_properties(instance, values="<json string>")`, and the value is a JSON
**string**, not an object. Passing `properties=` fails with a schema error that
helpfully prints the real schema, which is the fastest way to discover any of
these signatures.

### 2.14 The save and settings C++ has never been compiled

Written 2026-09-11 in a remote session with no engine, per 1.3. All five gates
pass, which per 1.4 says nothing about whether it builds. New files
`Source/LastTrain/Public/Core/LTSaveGame.h`, `LTSettingsSaveGame.h` and their
two `.cpp`, plus wiring in `LTGameInstance`, `LTGameMode` and
`LTPlayerCharacter`. No `LastTrain.Build.cs` change was needed: `USaveGame`,
`UGameplayStatics` and `FAudioDevice` are all in `Engine`, and
`UEnhancedInputUserSettings` is in `EnhancedInput`, both already in
`PublicDependencyModuleNames`.

**Three calls to check first if it does not build**, all in
`Private/Core/LTGameInstance.cpp`, and all written from memory of the 5.x API
rather than from a header:

1. `World->GetAudioDevice()` into an `FAudioDeviceHandle`, then
   `SetTransientPrimaryVolume`. The UE4 spelling was
   `SetTransientMasterVolume`; if 5.8 has moved it again, this is the line.
2. `UEnhancedInputLocalPlayerSubsystem::GetUserSettings()`, and
   `ApplySettings()` and `SaveSettings()` on what it returns.
3. `#include "UserSettings/EnhancedInputUserSettings.h"`, the path to that
   class inside the EnhancedInput module.

None of the three is load bearing for progression saving: cutting all of them
leaves the round records, the field of view setting and both slots working.

## 3. Repo hygiene

### 3.1 The discarded web build was removed

Done 2026-09-09. The Three.js tree that preceded the Unreal build, 66 files and
568K, is out of the working tree and preserved at the `web-threejs-final`
tag (`adeb32d`). The `phase-03` tag does not contain it.

**Recovering it:** `git ls-remote --tags origin`, not `git tag -l`. A fresh
clone may not fetch tags, which made the tag look missing during the survey.

`docs/reference/canary-wharf-grid.md` cited two files in that tree as the
provenance of its tile legend. It restates the legend and every constant in full
itself, so the citation was repointed at the tag and nothing was lost. `README.md`,
`CLAUDE.md`, `docs/brief-v3-unreal.md` and the now-dead skip in
`tools/ci/check_docs.py` were updated with it.

### 3.2 Dead references to retired task specs

Commit `bcd947a` retired 14 completed task files. Two live documents still point
readers at them as if they were readable:

- `docs/tasks/neostack.md`, the outstanding-editor-task list, cites
  `phase-c1-train.md` (twice), `phase-c-zombie-types.md`,
  `phase-b2-interaction.md` ("steps 3 to 7"), `phase-b3-feedback-widgets.md` and
  `phase-a4-editor-setup.md`. These are instructions to go and read a file that
  is not there.
- `README.md` pointed at `docs/tasks/phase-a4-editor-setup.md` as a how-to.

Both fixed 2026-09-09 by pointing at the retiring commit instead. Listed here so
the pattern is recognised: **when a task file is retired, grep for its name
first.**

Commit `bf04ca1` retired six more: the `remote-brief-*.md` handoff prompts for
the seven branches merged into `main` on 2026-09-11 (`docs(handover):
session close 2026-09-12` and the commits before it). Each was a one-time
prompt for a remote session; once its branch merged, the brief described
nothing still open. Grepped clean before removal, nothing else pointed at
them.

### 3.3 Not dead, do not "fix"

Two families of reference look broken to a link checker and are deliberate:

- **The `00` to `08` series and `phase-1-plan.md`.** External Project Knowledge
  uploads, never committed. `docs/brief-v3-unreal.md` and
  `docs/art-direction.md` both say so in their own text.
- **`open-questions.md`, `open-questions-review.md`, `open-questions.json`.**
  Removed deliberately; `docs/design/gameplay-canon.md` replaced them and names
  commit `0b7e35f` as where the full sweep lives.

Also fine: `_incoming_assets/ASSET-RESEARCH.md` (gitignored by design), `.mcp.json`
(gitignored), and the engine's own `BaseGame.ini` / `MacGame.ini` / `IOSGame.ini`
cited in `handover.md`, which are engine-side and correctly qualified.

### 3.4 Images are committed raw, not through LFS

`.gitattributes` marks `*.png` as `binary` but **not** as an LFS filter, so PNGs
go into the pack as raw blobs. There is one such file left,
`docs/reference/reference-frame.png` at 2.3 MB, and it is the project's visual
target so it has to be somewhere.

`UI.png`, a byte-identical duplicate of it sitting at the repo root and
referenced by nothing, was removed 2026-09-09.

Worth deciding, low priority: move `*.png` onto the LFS filter like `*.tga` and
`*.exr`, or accept the 2.3 MB. Moving it rewrites nothing already committed, so
the existing blob stays in history either way.

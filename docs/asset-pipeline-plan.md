# Asset pipeline plan

A gap analysis and a tooling plan for generating LAST TRAIN's own 2D and 3D
assets by script, in the remote lane, deterministically.

Written 2026-09-09 by the remote session. **No tooling code exists yet.** This
document is the proposal that has to be corrected and signed off before any
track is built. Nothing here modifies `Source/`, `Content/` or `docs/tasks/`.

Read `docs/known-issues.md` and `docs/tasks/who-does-what.md` first. This plan
lives or dies on the lane limits recorded there.

---

## 0. Corrections to the brief that commissioned this

The prompt behind this document was written from memory and four of its
premises are wrong. Correcting them changes the plan, so they go first.

| Premise in the brief | Reality |
|---|---|
| Read `NEXT.md` | There is no `NEXT.md`. The resume point is `docs/tasks/handover.md`, and the live queue is in `docs/tasks/who-does-what.md`. |
| Read "the archived `web/` build" in the tree | Removed from the tree on 2026-09-09. It is at the `web-threejs-final` tag, and `git tag -l` will not show it in a fresh clone until `git fetch --tags`. |
| Its station data encodes decisions "about station lists" | It does not. The tag holds exactly **one** station file, `web/src/data/stations/debug-yard.ts`. The 41 station list was always aspirational. It does encode a tile legend, a scale and a `StationDef` schema, and those are useful. |
| "Once the system exists, all 41 stations' signage is a data-entry job" | v1 ships **2 stations** (`docs/tasks/README.md`, `docs/brief-v3-unreal.md` Part 5 risk 4). Sizing a data format for 41 stations is designing for a scope the project has explicitly dropped. |
| "the exact UE5.4 import settings" | The project is on **UE 5.8** (`LastTrain.uproject`, both target files). `docs/brief-v3-unreal.md` says "5.4 or later" and is the stale one. |

One more, and it is the important one. The brief argues for generated 2D on the
grounds that a generated system stays consistent where 200 hand-made signs do
not. True, but there is a stronger argument specific to this repo, and it is in
section 1.4.

---

## 1. State of play

### 1.1 What is built

Cited from the code and the committed content, not from the plans.

| Area | State | Evidence |
|---|---|---|
| Phases A to E, the whole mechanical game | Code complete, PIE verified, playable in greybox | `docs/tasks/README.md` status table; 34 files under `Source/LastTrain/` |
| Modular station kit (F1) | **Done for Canary Wharf.** 12 meshes, shell rebuilt, navmesh and a PIE round verified | `Content/LastTrain/Kit/Meshes/`, `Content/LastTrain/Kit/KIT-NOTES.txt` |
| Lighting, Lumen, fog, wet floor (F2) | **Done.** Post process fixed, 32 sodium spots, 8 crimson tunnel units, volumetric fog, `MI_LT_FloorWet` | handover F2 row; `docs/reference/f2-acceptance-platform.png` |
| Signage and wayfinding (F5) | **Mostly done, and the task table has not caught up.** Six station name sign triples rebuilt and legible on both maps, 26 pictograms moved to a cookable group, the hanging departure board placed and ticking in PIE | handover F5 row |
| Surfaces | 23 ambientCG CC0 surface sets committed, 19 with material instances, over one master material | `Content/LastTrain/Surfaces/`, `SURFACE-SOURCES.txt` |
| Typefaces | 5 OFL families imported as `.uasset`, plus one offline atlas font | `Content/LastTrain/UI/Fonts/` |
| Pictograms | 26 ISO 7010 textures imported | `Content/LastTrain/UI/Pictograms/` |
| HDRIs | 2 Poly Haven CC0 cubemaps | `Content/LastTrain/Lighting/HDRI/` |
| Main menu (S13) | Done, and it is the launch map | `Content/LastTrain/Maps/L_MainMenu.umap` |
| HUD | Phase B3 widget shipped and reviewed; G2 spec written, not built | `Content/LastTrain/UI/WBP_HUD.uasset`, `docs/tasks/phase-g2-hud.md` |

Specced but unbuilt: F3 train exterior (the train is currently two scaled kit
panels), F4 train interior, F6 zombie bodies (one tinted mannequin repeated),
F7 perf pass (the 60 fps gate is measured FAILED and has no trustworthy
baseline), all of Phase G, and the four Phase E leftovers (perks, bench, lost
property, equipment).

Neither specced nor built: advertising artwork, notices and ephemera, decal
masks, the original network schematic, wayfinding panels beyond the station
name boards, and any audio at all.

### 1.2 Where the docs and the code disagree, and the code wins

- **`docs/tasks/README.md`** says of Phase F, "S11 signage started but broken".
  Superseded. F5 fixed it the same day. The same table's "Active task files"
  section still lists F5 as queued for CC-in-Unreal.
- **`docs/art-direction.md` section 7** says Overpass "is not present in the
  project or the engine". It is: five families are in
  `Content/LastTrain/UI/Fonts/`. Already flagged as `known-issues.md` 2.5 and
  scheduled to close with G2.
- **`docs/brief-v3-unreal.md`** says UE 5.4 or later; the project is pinned to
  5.8 in `LastTrain.uproject` and both `.Target.cs` files.
- **`docs/reference/canary-wharf-grid.md`** cites the `phase-03` tag for the
  legend; `known-issues.md` 3.1 records that `phase-03` does **not** contain the
  web build and `web-threejs-final` does. The grid restates every constant
  itself, so nothing is lost either way.

None of these is load-bearing for this plan. They are listed because a fresh
session reading the task table would conclude signage is broken, and it is not.

### 1.3 The three questions the brief asked me to settle

**Which skeleton the character work uses, and its bone naming.** The UE5
Manny/Quinn mannequin skeleton, with stock UE5 bone naming. The evidence is
`ALTZombieCharacter::HeadBoneNames`, defaulted in
`Source/LastTrain/Public/Zombies/LTZombieCharacter.h:150` to
`{"head", "Head", "neck_01"}`. `neck_01` is the UE5 convention specifically, not
the UE4 one. The handover S9 row confirms the mesh is Quinn and that
`M_Zombie_Tintable` samples the Quinn D/N/MRA texture set. Locomotion is the
stock `ABP_Unarmed`. Consequence for any character track: conform to the UE5
skeleton exactly, or the head hitbox and therefore the 130 point headshot stop
working.

**What the interaction and HUD work needs from assets.** Less than you would
think, and this matters for track ordering. `ULTInteractionComponent` drives a
prompt delegate and `WBP_HUD` already renders it; the prompt needs a key glyph
and nothing else. `docs/tasks/phase-g2-hud.md` is explicit that every value the
HUD needs is already Blueprint-visible, that G2 needs no `Source/` change, and
that its element list is round, points, health, weapon, reticle, hit marker,
prompt. It also **recommends cutting the station network schematic from the HUD
entirely** while v1 ships two stations, and rehoming it as diegetic platform
signage. So the HUD's asset appetite is a small glyph set, and the schematic
belongs to the signage track, not to a HUD track. The brief's candidate list
gets this backwards.

**Which station geometry conventions are already fixed.** Two layers of them,
and they agree.

From the web build (`web-threejs-final`, `web/src/data/legend.ts`) and restated
in full in `docs/reference/canary-wharf-grid.md`: `TILE` 1.5 m,
`WALL_HEIGHT` 3.6 m, `PLATFORM_LIP` 0.2 m, `TRACK_DROP` 1.1 m,
`SKIRTING_HEIGHT` 0.15 m, and a closed 14 character tile legend.

From F1, which is what actually shipped
(`Content/LastTrain/Kit/KIT-NOTES.txt`): base grid 100 uu, kit module 400 uu,
wall thickness 150 uu, room height 360 uu, and a **tile line at 220 uu** with
tiled wainscot below, painted concrete above and a proud trim at the join.
Pivots stand on Z=0 centred in X and Y, except the platform edge, whose deck top
is Z=0 with the fall running to Z=-130.

Unreal units are centimetres, so room height 360 uu is the legend's 3.6 m and
the two layers are the same decision. **Any generated asset that mounts on a
wall must respect the 220 uu tile line**, because a sign or poster straddling
it will read as a mistake. This single number constrains more of the 2D output
than anything else in the repo, and no existing document says so.

### 1.4 The environment, measured

Everything in this table was run in this container today, not assumed. The
brief asked specifically that `bpy` be verified before any 3D track was
proposed, so that row is the load-bearing one.

| Capability | Result |
|---|---|
| Python | 3.11.15, stdlib only for our purposes (the preinstalled packages are conan, yq and system glue) |
| PyPI | reachable through the proxy |
| Pillow | 12.3.0, installs clean |
| pycairo | 1.29.1, builds from source; system cairo and pango are present |
| CairoSVG | 2.9.1, installs clean |
| fontTools | 4.64.0, installs clean |
| numpy | 2.4.6, installs clean |
| **`bpy`** | **5.0.1 installs, imports, reports `bpy.app.background = True`, builds a mesh, runs `uv.smart_project`, and exports both GLB and FBX.** Verified by running it, not by reading about it |
| `blender` binary | absent. `bpy` as a Python module is the route, and it is sufficient |
| Fonts on the machine | 59 faces, all DejaVu, Liberation, FreeFont and CJK. **None of the project's typefaces** |
| The project's `.ttf` sources | **not in the repo.** `Content/LastTrain/UI/Fonts/` holds imported `.uasset` plus `OFL.txt` licence text only. The `.ttf` files live in `_incoming_assets/fonts/` on the Mac, which is gitignored |
| Fetching the typefaces | **works.** `raw.githubusercontent.com` serves Barlow from `jpt/barlow`, Public Sans from `uswds/public-sans`, and Overpass plus its `OFL.txt` from `google/fonts` at `ofl/overpass/` |
| Google Fonts API and `fonts.gstatic.com` | refused |
| Asset hosts (ambientCG, Poly Haven, Kenney, Freesound, Wikimedia) | still refused at CONNECT, exactly as `known-issues.md` 1.1 records |
| GitHub API (`api.github.com`) | 403 for repositories outside this session's scope |

Four findings from that table change the plan.

**Finding 1: glTF is byte-deterministic and FBX is not.** Exporting the same
cube twice produced an identical GLB both times and two different FBX files,
differing first at byte 340. FBX embeds a creation timestamp and a file id. The
brief requires byte-identical output, so **every 3D track emits glTF**, and any
FBX is a convenience export excluded from the determinism assertion. UE 5.8
imports glTF through Interchange, so this costs nothing.

**Finding 2: Pillow's PNG output is byte-deterministic.** Confirmed by
rendering the same sign twice and comparing bytes. The 2D tracks can assert
determinism on file bytes directly.

**Finding 3: the variable Overpass renders here, and can be made static.**
`Overpass[wght].ttf` has one `wght` axis, 100 to 900, default 400. Pillow
renders it and `set_variation_by_axes([600])` works. fontTools
`instantiateVariableFont` produced a 170 KB static SemiBold with `fvar`
dropped. This matters beyond the pipeline: the F5 handover row records that
`TrueTypeFontFactory` **failed to rasterise the variable Overpass** and Barlow
had to be substituted for the sign atlas font. A static instance is the fix for
that too, and it is a by-product of the signage track rather than extra work.

**Finding 4, and this is the real argument for the whole pipeline.** The remote
lane cannot download an asset and cannot get a gitignored file out of the
container (`known-issues.md` 1.1 and 1.2). Everything the remote lane has ever
produced for Phase F is therefore a *document* telling somebody else to go and
fetch something. A generator whose output is committed text and images is the
only asset-producing work this lane can deliver end to end: the tool, its data,
its output and its manifest all travel by git. That, not consistency, is why
this is worth building, and it is why the 2D tracks come first.

### 1.5 One prerequisite that blocks every track

`.gitattributes` puts `*.uasset`, `*.umap`, `*.fbx`, `*.tga`, `*.exr`, `*.hdr`
and `*.wav` on the LFS filter. It marks `*.png` as `binary` **and nothing
else**, so PNGs go into the pack as raw blobs. `known-issues.md` 3.4 flags this
as an open decision worth low priority, on the basis that there is one 2.3 MB
PNG in the repo.

A generated pipeline changes that calculus completely. Determinism means a
regeneration with unchanged data adds no new blob, but every content edit
rewrites a texture, and every rewrite is a new permanent blob in history. A
signage and poster set is tens of megabytes per revision.

**Recommendation, and it is cheap: put `*.png` and `*.glb` on the LFS filter
before the first track commits any output.** It rewrites nothing already in
history, so the existing `reference-frame.png` blob stays where it is either
way. Doing it after the first track ships means the bloat is already permanent.

---

## 2. Asset inventory

Structured as the brief asked, by what is visible in one frame of a finished
station, working outward from the surfaces. Classification is one of:

- **SOURCEABLE** a free or cheap download exists and authoring it is wasted
  effort.
- **AUTHORABLE** must be made, and can be made deterministically by script in
  this container with no GPU and no display.
- **HANDMADE** must be made, and needs a human, a GPU, the editor, or a
  judgement a script cannot supply.

Where something is already done, it says so, because the point of the exercise
is the gap and not the list.

### 2.1 Surfaces

| Asset | Class | Note |
|---|---|---|
| Tile, concrete, metal, rubber, tactile paving, plaster PBR sets | SOURCEABLE | **Done.** 23 ambientCG CC0 sets in `Content/LastTrain/Surfaces/` |
| Trim sheet for the kit's wainscot, trim and concrete band | AUTHORABLE | The one surface gap. F1's tile line at 220 uu wants a single trim sheet so the join is one authored decision instead of three tiling materials meeting |
| Wet floor variant | HANDMADE | Done in F2 as a material instance. Roughness remapping is a material decision, not a texture |
| Material graphs and instances | HANDMADE | In-engine only, and `known-issues.md` 2.7 is the reason to be careful: a graph can be fully wired, save, and compile to zero pixel shader instructions |

### 2.2 Architecture and the kit

| Asset | Class | Note |
|---|---|---|
| Wall, floor, ceiling, pillar, platform edge, tunnel portal, stair run, bench, bin, handrail | SOURCEABLE then HANDMADE, now **done** | There is no CC0 modular station kit, established by the F1 research. F1 modelled 12 pieces in-engine |
| Escalator bank, 30 degree pitch, parallel runs | AUTHORABLE | Not in the kit. Pure parametric geometry: tread, riser, balustrade, comb plate, all from a pitch and a rise. The single highest-value 3D piece, because the research calls a wide parallel bank "instantly big London interchange" |
| Gate line and barrier pedestals | AUTHORABLE | Parametric extrusions |
| Poster frames, 4-sheet, 16-sheet, 48-sheet, cross-track band, escalator panel | AUTHORABLE | Chamfered border round a plane, optionally a backlit inner plane. `advertising-and-dressing.md` calls a 4-sheet frame "a five minute mesh" and gives every real dimension |
| Sign frames, blades, drop rods, flag brackets | AUTHORABLE | Same argument |
| Platform edge screen doors | AUTHORABLE | Repeating glazed bay from a bay width and a head height |
| Tunnel rings and sprayed lining sections | AUTHORABLE | Lathe of a profile |
| Help point, fire cabinet, ticket machine, vending machine | AUTHORABLE for the box, HANDMADE for the read | Boxes with panel insets are scriptable; whether one reads as a help point is a silhouette judgement |
| Cove and coffer detail, the acoustic baffle soffit | AUTHORABLE | Repeating profile at a spacing |
| Timber lattice roof for an entrance box | HANDMADE | 564 unique steel nodes in the real thing. Anything that reads as this needs an artist |

### 2.3 Wayfinding and signage

| Asset | Class | Note |
|---|---|---|
| Station name boards | AUTHORABLE | Shipped in F5 as engine `TextRenderComponent` on primitives. Regenerating them as **textures** removes both F5 defects at once: no offline font atlas needed, and no dependence on the engine's lit default text material, which is the unfixed dimming problem in the F5 row |
| Directional signs, way out, to trains, no exit, staff only | AUTHORABLE | Not built. `signage-and-wayfinding.md` gives proportions, 3:1 to 6:1, and mounting heights |
| Platform number panels | AUTHORABLE | Not built |
| Frieze bands | AUTHORABLE | Not built |
| Large platform wall station mark, repeated every 10 to 20 m | AUTHORABLE | Not built. This is the roundel's slot and the place the identity is either solved or fudged |
| Safety and byelaw notices | AUTHORABLE | Not built |
| The original network schematic | AUTHORABLE | Not built. Data driven from station adjacency. Does the legal job of replacing the official line diagram, and G2 wants it here rather than in the HUD |
| ISO 7010 pictograms | SOURCEABLE | **Done.** 26 committed. Not TfL IP, an international standard |
| Arrow and chevron set | AUTHORABLE | Not built, and must come from the same source as the HUD arrows or the two will drift |

### 2.4 Display content

| Asset | Class | Note |
|---|---|---|
| Departure board face and enclosure | AUTHORABLE for the art, done for the mechanism | `ALTDepartureBoard` polls the train and drives its text. F5 placed it and verified the countdown live in PIE |
| Dot matrix glyph atlas, amber on charcoal | AUTHORABLE | The board currently draws with an offline atlas font. A real dot matrix atlas is what makes it read as transit hardware |
| Static board states, service disruption, no service, last train | AUTHORABLE | Strong environmental storytelling, and the brief is right that a board showing something wrong is stronger than one showing something plausible |
| Platform indicator and station clock faces | AUTHORABLE | |
| Live runtime content | HANDMADE | A render target the Blueprint drives. Already wired |

### 2.5 Advertising

| Asset | Class | Note |
|---|---|---|
| Fictional poster artwork, 4-sheet through 96-sheet and cross-track | AUTHORABLE | Nothing exists. Layout templates plus seeded palettes plus a campaign data file. `art-direction.md` section 4 gives the editorial rules: two or three recurring fictional companies, one advert in four carrying lore, the rest mundane |
| Ageing and damage passes, torn corners, water damage, overpasting, dead backlight tubes | AUTHORABLE | Compositing over the same artwork, deterministic per seed |
| Digital advert panel content | AUTHORABLE | |
| Anything that needs to look photographed, a face, a product shot | HANDMADE | A generated poster can be typographic and graphic. It cannot contain a photograph, and a campaign that needs one has to be designed around that limit |

### 2.6 Props, dressing and ephemera

| Asset | Class | Note |
|---|---|---|
| Litter, newspapers, cups, bottles, wrappers, tickets | SOURCEABLE for the meshes, AUTHORABLE for their printed faces | A folded freesheet with an original masthead is a texture problem |
| Bins, benches, barriers, A-frames, cable trays, conduit | SOURCEABLE, partly done | F1 has bench and bin; the local packs cover the rest |
| Abandoned personal items, umbrella, single shoe, suitcase | SOURCEABLE | |
| Timetable cases, staff notices, improvement works apologies, missing person notices | AUTHORABLE | Same track as the notices |
| Graffiti tags | HANDMADE | A generated tag looks generated. This is the clearest AUTHORABLE trap in the list |

### 2.7 Rolling stock

| Asset | Class | Note |
|---|---|---|
| Body shell, near-vertical sides, flat roof, deep skirt, door apertures | AUTHORABLE | The research is explicit that the dimension table is complete enough to box-model from, and gives every number: 23 m driving car, 22.5 m intermediate, 2.77 m width, 3.76 m rail to roof, 1.145 m floor height, three double-leaf doors per side per car |
| **The cab, full-width curved wraparound windscreen** | **HANDMADE** | Compound double curvature, and the single feature the whole silhouette is judged on. Scripting this is the way to get a train that is dimensionally correct and looks wrong |
| Interior fittings, longitudinal and transverse seating, poles, gangway rings, LED strips | AUTHORABLE | Repeating parametric furniture along a datum, which is exactly what a script is good at |
| Livery, cab band, door surrounds, car numbering, destination blinds | AUTHORABLE | Texture and decal work off the identity system. `#16161C` shell, `#E0A030` cab band, `#6C4C9C` door surrounds |
| Bogies, couplers, underframe equipment | SOURCEABLE or HANDMADE | Mechanically fiddly, barely seen, not worth a track |

### 2.8 Characters

| Asset | Class | Note |
|---|---|---|
| Base zombie body | SOURCEABLE | City Sample Crowds is local and gitignored; MetaHuman is the fallback. F6's spec already chooses this |
| Garments and wardrobe | SOURCEABLE | Buying beats scripting, decisively |
| Body variation, the seeded wardrobe combination | AUTHORABLE **only once a wardrobe exists** | Gated on a purchase or download, so it cannot be a first track |
| The five type reads, armour plate, brute bulk, screamer silhouette | HANDMADE | Silhouette judgement, and the C++ already carries the mechanics |
| Shamble and lunge animation set | SOURCEABLE then HANDMADE | `ULTZombieTypeData::AnimPlayRate` is plumbed in C++ and the stock ABP does not read it. That is an animation blueprint job |
| Player hands and view model | SOURCEABLE | |

### 2.9 Weapons

All **SOURCEABLE or HANDMADE**, and explicitly not a track. A weapon needs a
sculpted mesh, a first-person animation set and a recoil feel. The C++ side is
done and tuned; the art is a purchase. Only the **original weapon names** are
ours, and those are a data decision, not an asset.

### 2.10 Decals and VFX

| Asset | Class | Note |
|---|---|---|
| Grime, water staining, scuffs, salt bloom, rust runs as greyscale masks | AUTHORABLE | Cheap to generate, tedious to source consistently, and they carry the wet lived-in look the art direction asks for |
| Blood pools and drips | AUTHORABLE | A fluid boundary is a solvable procedural problem |
| Hero gore spatter | HANDMADE | Directional spatter that reads as an impact is a judgement call, and `art-direction.md` section 6 asks for restraint and high-value moments, which is the opposite of a generated set |
| Chewing gum discs, tyre marks, painted floor markings | AUTHORABLE | The floor markings are typography, so they come from the identity system |
| Muzzle flash, impact sparks, dust, blood mist | HANDMADE | Niagara |

### 2.11 HUD

| Asset | Class | Note |
|---|---|---|
| Ammunition, equipment and interaction glyphs | AUTHORABLE | Small, and must share a source with the signage arrows |
| Reticle and hit marker | HANDMADE, done | Widget geometry, not art |
| Perk icons | AUTHORABLE, blocked | No perks exist. Phase G decides whether v1 ships them |
| Station schematic | AUTHORABLE, rehomed | G2 recommends cutting it from the HUD. It belongs in signage |

### 2.12 Audio

| Asset | Class | Note |
|---|---|---|
| Ambience, room tone, train hum, drone | **AUTHORABLE** | The brief's candidate list misses this. A synthesised drone is arithmetic over a sample buffer: numpy plus the stdlib `wave` module, no GPU, fully deterministic. `asset-sources-phase-f.md` section 5 independently recommends synthesis over a field recording, on the grounds that a synthesised drone **provably cannot contain an announcement**. That is a legal argument for generating rather than sourcing, and it is the strongest one in this document |
| Weapon, impact, foley, zombie vocals | SOURCEABLE | |
| Original announcements | HANDMADE | Needs a voice. Original phrasing is a writing job, and no recording may be transcribed |
| Music and stingers | SOURCEABLE | |

---

## 3. Tooling plan

Nine candidate tracks, ordered by value per unit of effort. The ordering
principle: 2D before 3D, because 2D is cheap, reliable, needs no sourced input,
and contributes more to the setting reading as the Underground than any single
mesh does.

### 3.1 The contract every track obeys

Stated once here rather than repeated nine times.

- **Data driven.** Content lives in committed data files under the track's own
  `data/` directory. The tool is a renderer over that data. Adding a sign means
  editing data, never code.
- **Deterministic and seeded.** Same inputs, byte-identical output. Asserted by
  building twice and comparing bytes, except for 3D FBX convenience exports,
  which are excluded for the reason in finding 1.
- **CLI.** Subset selection, `--seed`, `--dry-run`, `--out`.
- **A manifest**, emitted alongside the output, with a documented schema treated
  as a contract with the Unreal side. See 3.2.
- **A real headless assertion suite** in the track's `tests/`, asserting on
  properties that matter for the output type, not that a file appeared.
- **A contact sheet**, because the author of the tool cannot see its output and
  the human has to be able to judge it in one image.
- **A CI job** matching the existing five, added without touching them.
- **A README** covering how to run it, how to add content, the manifest schema,
  the assumptions made and where they came from, and the UE 5.8 import settings
  for its output.
- **Pinned dependencies**, in a per-track requirements file. Note that `bpy`
  5.0.1 pins `numpy` to 1.26.4 while a 2D track will want 2.x, so the 2D and 3D
  tracks need separate virtual environments. Do not fight this.
- **No new paid dependency.**

Two repo-specific traps that are not obvious:

- `tools/ci/check_hygiene.py` scans every `.py` file for trademark terms and
  exempts only `.md` files, `docs/`, and `tools/ci/` itself. A generator under
  `tools/` explaining in a comment that its mark is deliberately not a certain
  circular device will **fail the gate**, unless the same line also carries a
  negation word. It does not exempt `tools/` generally.
- The same checker rejects absolute home paths in any text file. Tests must use
  `tmp_path` or a repo-relative directory, never a hardcoded one.

### 3.2 The manifest contract

One schema shape for every track, so the Unreal side learns it once.

Top level: the tool name and version, the git commit, the seed, the ISO date,
and an `entries` array. Each entry carries a stable `id`, the output `path`
relative to the manifest, a `sha256`, a `kind`, the source `data` record it came
from, and a `unreal` block.

The `unreal` block is the part that earns its keep, because it is what lets
CC-in-Unreal import a hundred files without deciding a hundred times. For a
texture it carries the target content path, the texture group, the compression
setting, whether sRGB is on, and the addressing mode. For a mesh it carries the
target path, whether Nanite is on, the collision treatment, and the material
slot names.

That last field is not decoration. `KIT-NOTES.txt` records that
geometry-scripted meshes ship with **zero collision shapes**, which made the
whole station unwalkable until each piece was given box collision, with
`CTF_UseComplexAsSimple` on the portal, the stair run and the platform edge. A
mesh manifest that does not state the collision treatment per piece will
reproduce that bug. A glTF authored in `bpy` can carry an explicit collision
mesh, which is a real advantage over the in-engine route.

### 3.3 World-space texel density, fixed now

Consistency across tracks needs one rule, and the brief's audit prompt asks for
it, so it is settled here rather than discovered later.

Unreal units are centimetres. Two density classes:

- **Class A, read at 1 to 3 m: 512 px per metre.** Posters, notices, small
  wayfinding, ephemera, HUD source art. A 4-sheet at 1016 by 1524 mm becomes
  520 by 780 px, packed into a 1024 page.
- **Class B, read at 5 to 30 m: 128 px per metre.** Station name boards,
  friezes, cross-track bands, the large platform wall mark. F5's 7 m main board
  becomes 896 px and fits a 1024 page with room for the bleed.

Every manifest entry states its class and its real-world size in millimetres.
A checker material then reads at the same size on every piece, which is the
cross-track consistency the audit prompt looks for.

### 3.4 The tracks

**Track 1: signage and wayfinding.** Emits Class B and Class A PNG atlas pages
plus a manifest mapping sign ids to page UVs. Reads a station data file (name,
platform designations, directions, exit routes, interchanges) and a sign
vocabulary file. Deterministic inputs: the data, the identity spec constants,
the seed for wear variants only. Validated headlessly by asserting page
dimensions and budget, that every sign id in the data appears in the manifest
and every manifest entry has a file, that no glyph overflows its box (measured
from the font metrics, which is the assertion that actually catches bad
layouts), contrast ratio between text and field, palette conformance to the four
hex values, and byte determinism across two builds. Effort: **medium**, and the
largest part of it is the identity spec in section 4 rather than the code.

**Track 2: departure board and display content.** Emits a dot matrix glyph
atlas, static board state faces, clock and platform indicator faces. Reads a
service pattern data file. Validated by round-tripping the glyph atlas, that
every character in the declared set is present and non-empty, and that a
rendered board row matches an expected pixel signature. Effort: **small**,
because the mechanism is already built and verified in PIE. Highest ratio of
visible payoff to work in the whole plan.

**Track 3: advertising artwork.** Emits Class A poster PNGs plus a manifest.
Reads a campaigns data file, one record per advert: fictional brand, product,
copy lines, layout template id, palette key, lore flag, age and damage seed.
Validated by asserting every campaign renders, that text fits, that the
palette derives from the four project colours, that the lore ratio in the data
is roughly one in four per `art-direction.md`, that no real brand string appears
against a blocklist, and determinism. Effort: **medium**. This is where the
lore lives.

**Track 4: notices and ephemera.** Emits Class A notice and ticket and
newspaper faces. Same machinery as track 3 with different templates, so it is
**small** once track 3 exists and should reuse its renderer rather than fork it.

**Track 5: decal masks.** Emits greyscale and packed masks for grime, water
staining, scuffs, rust runs, gum, blood pools. Reads a generator parameter data
file per mask family. Validated by asserting histogram properties, mean and
variance inside a declared band, tileability where declared (the wrap seam
checked by comparing opposite edges), alpha coverage budget, and distinctness
across seeds measured as a minimum pixel difference. Effort: **small to
medium**, and it is the cheapest way to buy the wet lived-in look.

**Track 6: HUD and schematic.** Emits SVG plus rasterised PNG for glyphs and
arrows, and the network schematic, from the station adjacency data. Shares the
arrow and grid definitions with track 1 by importing them, not by copying them.
Validated by asserting SVG parses, viewBox and grid conformance, that the
schematic includes every station in the adjacency data exactly once and no edge
crosses another, and determinism. Effort: **small**, but blocked in part: perk
icons cannot be designed before Phase G decides whether perks ship.

**Track 7: station kit additions.** Headless `bpy`, verified working. Emits
glTF plus a manifest with explicit collision and Nanite settings. Reads a piece
parameter data file keyed to the F1 module contract: 400 uu module, 220 uu tile
line, 360 uu room height, the pivot convention. Priority pieces are the
escalator bank, the gate line, the poster and sign frames, the platform edge
screen door bay, and the tunnel ring. Validated by asserting the mesh is closed
and manifold where declared, triangle budget, UV channel present with no
overlap beyond a tolerance, world-space texel density from the UV area against
the surface area, bounding box matching the declared module, that a collision
mesh exists, and GLB byte determinism. Effort: **large**. It is the gap that
most affects whether the station reads as London rather than as New York or
sci-fi, and it is still third in line behind the 2D because the 2D is cheaper
and F1 already covers the shell.

**Track 8: ambience synthesis.** Emits seamless looping WAV room tone, train
hum and tunnel drone from a spectral recipe data file. numpy plus `wave`.
Validated by asserting loop seamlessness at the wrap point, RMS and peak inside
a declared band, spectral centroid inside a band per recipe, no DC offset, and
byte determinism. Effort: **small**. Add it to the plan because it is cheap, it
unblocks Phase G's first item, and the "a synthesised drone cannot contain an
announcement" argument makes it legally safer than any download.

**Track 9: rolling stock detailing.** Headless `bpy` for the interior fittings
and the body extrusion, with the cab left as a HANDMADE hole in the middle of
it. Gated on F3 settling the base shell. Effort: **large**, and it should not
start until F3 exists to detail.

### 3.5 Not tracks, and why

The base zombie body, garments, weapons, anything sculpted, graffiti, hero gore
spatter, Niagara VFX, material graphs, Lumen and post process tuning, and the
train cab. Source, buy, or hand these to a human. The brief's own list says the
same for most of them, and sections 2.7 and 2.10 above add the cab, the
graffiti and the hero spatter.

---

## 4. The identity question

The hard constraint: station names and geography are factual and fine; the
identity system laid over them is not. No roundel, no Johnston or New Johnston,
no reproduction of the official line diagram, no operator livery or logo, no
transcribed announcements. `docs/reference/branding-precedent.md` establishes
that this is the normal route and not an unusually cautious one, citing a major
publisher shipping a recognisable Underground level with zero TfL marks on it.

What follows is a specification, written to be read as a transport authority's
design manual. It is meant to stand on its own: if the Underground did not
exist, this should still look like a coherent system somebody drew on purpose.

It also deliberately **formalises what has already shipped** rather than
replacing it. F5 built a violet bar over a charcoal field with the station name
in sodium, and `WBP_MainMenu` echoes that geometry. That is the marque, whether
or not anyone wrote it down. Overturning it now would waste committed work.

### 4.1 The operator and the line

**The line is the Meridian line. The operator is Meridian Rail.**

The reasoning is geographic and it is the kind of reasoning a real naming
exercise produces. The line is a fictionalised main-line east-west route
through Canary Wharf. Canary Wharf sits a little over a mile west of the prime
meridian at Greenwich, and an east-west railway through that part of London
crosses it. The name is therefore descriptive rather than decorative, it is
original, it carries no operator's mark, and it survives translation into a
mark (see 4.2) because a meridian is a line.

Two alternates if the owner dislikes it: **the Reach line** (the river's
reaches, Blackwall Reach and Bugsby's Reach, both adjacent to the station) and
**the Isle line** (the Isle of Dogs). Both are equally original and neither
translates into a mark as cleanly.

**One clearance action before any commercial release**, consistent with the
last section of `branding-precedent.md`: check "Meridian Rail" against the
trade mark register and against active rail sector companies. A fictional
passenger line in a game is low risk, but the check is cheap and the answer
should be recorded before the name is on a hundred assets.

**Service pattern rule.** Destinations on boards and blinds are **real London
place names arranged into a service pattern that is not any real line's**. Real
geography is factual and fine; reproducing a real timetable would be
reproducing a real service. The archived web build reached the same conclusion
and used invented destinations for the same reason. Candidate termini, offered
for correction: eastbound **Barking Riverside**, westbound **Park Royal**. Both
are real places, neither is a terminus of the real line, and the pair spans
London plausibly.

**In-world voice.** Signs are issued by Meridian Rail. Byelaw and enforcement
notices are issued by a separate fictional authority so the world has more than
one institution in it: **the Regional Transport Board**. Improvement works
apologies are signed by Meridian Rail; prohibitions are signed by the Board.
This split is free and it does a lot of work for the notices track.

### 4.2 The marque

**The Meridian mark.** Constructed on an 8 by 8 unit grid:

- A charcoal `#16161C` square field, 8 by 8.
- A violet `#6C4C9C` vertical bar, 1 unit wide, full height, its left edge at
  x = 3.
- A sodium `#E0A030` horizontal bar, 1 unit tall, full width, its top edge at
  y = 5.
- Where they cross, sodium is drawn over violet.

It is a meridian crossing a line of latitude. Three rectangles, no curves.

Why this and not something else, in the terms the constraint demands:

- **No circle anywhere, and no enclosing form.** The prohibited device is a bar
  across a ring. This has no ring.
- **Asymmetric on both axes.** The vertical bar is at 3/8 and the horizontal at
  5/8, so it never reads as a centred cross, a plus, or a medical or national
  symbol. The prohibited device's whole character is centred symmetry, and this
  is the opposite.
- **It survives 16 px.** Three rectangles on an 8 unit grid means the mark
  rasterises exactly at any multiple of 8, which is why it can be the same
  drawing in a HUD glyph and on a train side.
- **It uses two palette colours in their assigned roles**, violet for identity
  and sodium for function, so it is a member of the system rather than a logo
  bolted on.

**The station plate**, which is the large architectural form and is what F5
already built: a charcoal field with a violet bar across the **top 22 per cent**
of its height, full width, and the station name in sodium, centred, in the
remaining field. At Class B sizes the mark sits in the bar at the left, one bar
height square, with one bar height of clearance. Below 1200 mm wide the mark is
dropped and the bar carries nothing, because a 90 mm mark is a smudge.

Aspect ratios: **5:1** for the main platform board, **4:1** for the repeated
wall boards. F5 shipped 700 by 130 cm and 500 by 100 cm, which are 5.4:1 and
5:1, so this ratifies what is on the wall.

### 4.3 Typeface

**Overpass** for all architectural wayfinding and platform numerals.
**Overpass Mono** for the departure board, the dot matrix and any tabular
numeral. **Public Sans** for dense institutional notices, byelaws and
prohibitions. All three are SIL OFL 1.1, all three are already imported as
`.uasset`, and all three are fetchable as source `.ttf` with their licence text,
verified in 1.4.

Overpass is the right primary for a reason that is about the constraint and not
just about taste: it descends from US Federal Highway Administration signage
lettering, so its heritage is road signage rather than transit lettering, and
its tells are the opposite of the prohibited face's. That face's signature is a
perfectly circular O and diamond-shaped tittles and full stops; Overpass has
slightly squared curves, plain round tittles and a taller x-height. It reads as
signage while being traceably descended from somewhere else entirely, which is
exactly the defence the project wants. `signage-and-wayfinding.md` reaches the
same recommendation independently.

Public Sans as the notice face is a second deliberate choice: it was made for US
government notices, so it looks institutional and generic, which is what a
byelaw notice should look like. Using a different face for notices than for
wayfinding is also how real systems work.

**Barlow Condensed stays confined to the menu title**, where `WBP_MainMenu`
already uses it. It is a display face and it does not appear in the world.
This resolves the typeface split flagged as `known-issues.md` 2.6 in the
direction G2 recommends: one display face for titles, one functional pair
everywhere else.

**Weights.** Overpass SemiBold, `wght` 600, for sign text; Regular, 400, for
body copy on notices. Both taken as **static instances** produced by fontTools
from the variable file, per finding 3, which also gives the engine an
importable file for the offline atlas it needs.

### 4.4 Colour, one colour one job

The four project colours are fixed. What has not been written down is what each
one means, and without that rule a generated set will use them decoratively.

| Colour | Hex | Job | Never |
|---|---|---|---|
| Charcoal | `#16161C` | The default field for all information | As a text colour, except on a sodium field |
| Violet | `#6C4C9C` | Identity only: the mark, the plate bar, structural leading edges | As a text colour at any size, and never as a large field |
| Sodium | `#E0A030` | Function: names, arrows, live values, and the exit field | On a crimson field |
| Crimson | `#B02030` | Danger, prohibition, emergency | As general illumination, and never on more than one sign in a sightline |
| Off-white | text only | Neutral body copy and destinations | As a field |

**The exit rule, and it is the one real invention here.** The generic
convention for exit routing is a yellow field, and generic safety practice is
fair game, but copying the operator's exit panel is not. Sodium `#E0A030` is
already the project's amber. So: **way out and exit signage inverts the system.
A sodium field with charcoal text.** Everything else in the station is text on
charcoal; the exit is the only thing that is charcoal on light. That carries the
exit meaning through inversion rather than through anyone's artwork, it needs no
fifth colour, and under the sodium lighting rig the exits will be the brightest
things on the wall, which is what an exit should be.

### 4.5 Hierarchy and grid

Six layers, coarsest first, each answering one question, following the structure
in `signage-and-wayfinding.md`:

1. **Network identity.** The mark and the line name. The large platform wall
   plate, repeated every 10 to 20 m.
2. **Direction of travel.** Platform designated by direction and destination,
   not only by number.
3. **Way out and interchange.** The inverted sodium panels. The most repeated
   sign in the system.
4. **Platform number.** Large numeral, high, at the entrance and repeated.
5. **Local and safety information.** Denser, lower, Public Sans, Board issued.
6. **Service information.** The board and the digital panels, dynamic.

**The grid.** One unit, `c`, is the cap height of the panel's primary text.
Everything else is a multiple:

| Measure | Value |
|---|---|
| Margin, all sides | 1c |
| Gap between a mark and the text it labels | 1c |
| Gap between an arrow and its destination | 0.75c |
| Leading, multi-line | 1.5c |
| Plate bar height | 1.5c, or 22 per cent of panel height, whichever is greater |
| Secondary text cap height | 0.6c |
| Minimum clear space around the mark | 1 mark unit, that is 1/8 of the mark's width |
| Stroke weight for arrows and rules | 1/8 c |

**Mounting**, taken from the research and reconciled with the F1 kit, which is
the reconciliation nothing in the repo has done yet: the tile line sits at
220 uu, so wall-mounted directional signs centre at **170 to 200 uu** and sit
wholly **below** the trim, friezes run at **240 uu and up** wholly above it, and
nothing straddles 220. Hanging blades clear **210 to 260 uu** at their bottom
edge. The large wall plate centres at **150 to 220 uu**. Notices sit at
**140 to 170 uu**.

### 4.6 Arrows and pictograms

**The arrow.** One arrow, drawn once on the 8 unit grid: a shaft 1 unit thick
and a head 3 units across at 45 degrees, total length 6 units. Eight rotations
at 45 degree steps, plus two doglegs, left-then-up and right-then-up, for
routes that turn. Sodium on charcoal, or charcoal on sodium on exit panels. It
is generated as SVG and rasterised, and **track 6 imports the same definition
rather than redrawing it**, which is how the HUD ends up belonging to the same
world instead of merely resembling it.

**Pictograms.** ISO 7010, already imported as 26 textures. An international
standard, not anyone's trade dress, and the correct answer. The identity system
supplies only the field and the frame around them. One live defect worth
recording: `MI_Picto_no_access_unauthorised` renders blank because its source
PNG is near-empty, and F5 substituted `general_warning`. A generated pictogram
frame set should not silently inherit that hole.

### 4.7 What this system deliberately does not have

Stated so a later pass does not add them back: no circular or elliptical device
of any kind; no bar crossing an enclosing form; no diamond tittles; no
perfectly circular O; no geographic line diagram with the real line's topology;
no operator name resembling a real one; no fifth colour; no photograph on any
sign.

---

## 5. Recommended first track

**Track 2, departure board and display content.**

This is not the answer the brief expected, and the argument is about leverage
rather than about scope. Track 1 signage is the larger prize and it is where the
identity is proved, but it is also the track whose design half is the entire
section 4 above, which the owner has not yet corrected. Building the renderer
before the spec is signed off means building it twice.

Track 2 is the same machinery at a fifth of the size, and it lands against a
mechanism that is already finished and verified. `ALTDepartureBoard` polls the
train's countdown and drives its text, F5 placed the board mid-platform, and the
handover records it reading "30s inbound" down to "19s inbound" in PIE against
real `ALTTrain` state. `art-direction.md` calls the board "the single most
valuable thing to lift from the image", and `reference-frame-notes.md` calls it
"the best idea in the frame" and a mechanic rather than a decoration. The one
thing it lacks is a face that looks like transport hardware, because it
currently draws with a general-purpose offline font atlas on a charcoal quad.

So track 2 exercises every piece of the shared contract, the data format, the
manifest, the texel density rule, the determinism assertion, the contact sheet,
the CI job, the UE 5.8 import settings, against a target where the payoff is
immediate and visible in a screenshot the owner can judge in five seconds. It
proves the identity system's typography and colour rules on a small surface
before they are committed to a hundred signs. It produces a dot matrix atlas
that track 1 then reuses. And if the whole approach turns out to be wrong, it is
a small loss rather than a large one.

Then track 1 signage, once section 4 has been corrected, because the corrected
spec is its actual input.

---

## 6. Owner decisions this plan needs

Ranked by how much downstream work each one blocks.

1. **The AUTHORABLE versus HANDMADE split in section 2.** The one thing here
   that can waste a whole build cycle. The calls most worth arguing with are the
   train cab as HANDMADE with the body as AUTHORABLE (section 2.7), graffiti as
   HANDMADE (2.6), hero gore as HANDMADE with pools as AUTHORABLE (2.10), and
   ambience synthesis as AUTHORABLE (2.12), which the brief's own list did not
   have as a track at all.
2. **The identity spec in section 4**, in particular the operator and line name,
   the mark's construction, and the sodium-field exit inversion in 4.4. Every
   downstream track inherits these, so a change after track 1 ships is a
   regeneration of everything.
3. **`*.png` and `*.glb` onto the LFS filter**, per section 1.5. Cheap now,
   permanent later.
4. **Whether v1 ships perks**, which is already an open Phase G question and
   which blocks part of track 6.
5. **Commit the source `.ttf` files** for the three OFL faces plus their licence
   text, so the generator and the engine rasterise from one source of truth
   instead of the generator fetching from GitHub and the engine reading a
   font installed on one particular Mac. Roughly 600 KB, and it closes the
   fragility recorded in the F5 handover row.
6. **The service pattern and termini** in 4.1, which the boards and blinds need
   before track 2 can emit a plausible service.

## 7. What I did not do

No tooling code, per the brief. No change to `Source/`, `Content/` or
`docs/tasks/`. I did not update the stale rows in `docs/tasks/README.md` or
section 7 of `docs/art-direction.md`, both noted in 1.2, because `docs/tasks/`
is out of scope for this pass and the art direction fix is already scheduled to
close with G2.

I did not verify that UE 5.8's Interchange glTF importer handles a `bpy` 5.0
export with the collision and Nanite settings this plan assumes. That needs the
editor, so it is a CC-in-Unreal check, and it should happen before track 7
rather than before track 2.

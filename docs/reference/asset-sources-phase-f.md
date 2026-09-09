# Phase F asset sources

Researched 2026-09-09 by the remote session, per `docs/tasks/asset-research-phase-f.md`.

This is a **shopping list, not a haul.** Read the next section before using it.

## Why this is a document and not a folder of files

`asset-research-phase-f.md` asks for downloads staged in `_incoming_assets/`
with `SOURCES.txt` beside them. The remote session cannot produce that, for two
independent reasons, and the second one would still apply even if the first were
fixed:

1. **The remote container has no route to any asset host.** Its egress proxy
   allows GitHub, the Anthropic API and the language package registries.
   Everything else is refused at CONNECT with a 403: ambientcg.com,
   polyhaven.com, quaternius.com, kenney.nl, freesound.org, itch.io,
   wikimedia.org. Web search still works, so the research is real; the
   downloading is not possible.
2. **Nothing staged in the remote container could reach the human's machine
   anyway.** `_incoming_assets/` is gitignored (`.gitignore` line 80), the task
   forbids committing assets, and the container is reclaimed when the session
   ends. A downloaded file would have no route out.

So the transferable half of the job is the part that survives: **which files,
from where, under what licence, for which task.** That is what this document is.
`tools/asset-fetch/fetch-phase-f.sh` turns it back into a folder of files with
`SOURCES.txt` on a machine that can actually reach the hosts.

**Every entry rests on search-result evidence only.** No page was opened, no
licence text was read on the page itself, no mesh or texture was looked at.
Entries marked `UNVERIFIED` are the ones where search did not settle the
question. Confirm the licence on the page before staging anything, and treat the
legal flags below as work to do rather than as findings.

## How to use it

1. Run `tools/asset-fetch/fetch-phase-f.sh` on the machine with the editor. It
   creates `_incoming_assets/<category>/`, fetches what can be fetched from a
   script, and writes a `SOURCES.txt` per category from this document.
2. Work the manual checklist it prints at the end: itch.io, Wikimedia Commons
   and anything else that needs a human click.
3. Clear the legal flags in the "Inspect before staging" section below.
4. Append what actually landed to `_incoming_assets/ASSET-RESEARCH.md`, which
   lives outside git and is the record of the real haul.
5. Then, and only then, the editor session triages into `Content/LastTrain/`.

## Headline findings

**There is no CC0 modular station kit, and F1's budget should assume that.**
Nothing on Quaternius, Kenney, Poly Haven, ambientCG, OpenGameArt or the CC0
corners of itch.io ships wall panels, ceiling sections, platform-edge sections,
escalators and tunnel portals as one public-domain set. Every realistic modular
subway kit found in the sweep is paid. The closest free thing is CC-BY, not CC0,
and is a cramped metro bore rather than a tall main-line hall. What the CC0
world gives this project free is **surfacing, small props and proportion
reference**. The station shell, the platform edge, the escalator, the gate line
and the tunnel portal are in-house modelling work.

---

## 1. Surface textures (F1)

The most actionable section here, and the one the fetch script can mostly
automate. Every entry is a plain generic material: ceramic tile, concrete,
plaster, metal, rubber, gravel, timber. None contains lettering, so none can
carry a mark. **The one thing to watch:** ambientCG and Poly Haven both hold
photogrammetry scans of real street and station surfaces, and a scan can capture
painted markings. Reject on sight anything with text, numerals or a logo baked
into the base colour map.

### 1.0 Sources and their licences

| Source | Licence | Account | Verdict |
|---|---|---|---|
| ambientCG | CC0 1.0, stated at https://docs.ambientcg.com/license/ | No | Use freely |
| Poly Haven | CC0, stated at https://polyhaven.com/license, no attribution | No | Use freely |
| cgbookcase | CC0 1.0 per snippet and CG Channel's launch write-up. **Confirm the per-asset line on the page.** Medium confidence. | No, for single downloads | Use with the check |
| TextureCan | CC0 1.0 per https://www.texturecan.com/terms/ | No | Use freely |
| 3dtextures.me | CC0 per https://3dtextures.me/about/ | No | Use freely |
| ShareTextures | **Not pure CC0.** A "custom CC0-based" licence that forbids redistributing the asset in collections or plugins without written permission. | No | **Excluded.** Not unrestricted. |

Excluded so nobody re-researches them: CGTrader and Poliigon (paid), Adobe
Substance 3D community assets (Adobe account), BlenderKit (account and
subscription tiers), LotPixel and TextureMax (account plus premium),
Architextures (a service, licence not established), CGMood, rawcatalog.

### 1.1 Download URL patterns, for the fetch script

**Every constructed URL below is UNVERIFIED**, since nothing could be fetched to
confirm it. Both sites also publish a keyless API, which is the reliable route
and is what the script uses as a fallback.

- **ambientCG.** Page `https://ambientcg.com/view?id=<AssetID>`. Zip at
  `https://ambientcg.com/get?file=<AssetID>_2K-JPG.zip` (swap 2K for 1K/4K/8K,
  JPG for PNG). A 2K JPG zip runs roughly 8 to 40 MB and contains base colour,
  normal in DX and GL, roughness, displacement and usually ambient occlusion as
  separate files. Fallback API, documented at https://docs.ambientcg.com/api/v2/ :
  `https://ambientcg.com/api/v2/downloads_csv?id=<AssetID>` returns exact file
  URLs as CSV.
- **Poly Haven.** Page `https://polyhaven.com/a/<slug>`. Serves individual maps,
  not a zip:
  `https://dl.polyhaven.org/file/ph-assets/Textures/jpg/2k/<slug>/<slug>_diff_2k.jpg`
  with siblings `_nor_gl_2k`, `_rough_2k`, `_ao_2k`, `_arm_2k`, `_disp_2k`. Map
  suffixes vary per asset, so the reliable route is the keyless public API
  (https://polyhaven.com/our-api): `https://api.polyhaven.com/files/<slug>`.
- **cgbookcase, TextureCan, 3dtextures.me.** Per-map buttons, no documented URL
  pattern. Download by hand.

### 1.2 Dirty subway tile, the hero surface

Carries more screen area than anything else in the kit. **Take a clean base and
a dirty variant and blend them with a vertex-painted or world-height mask.**
Grime concentrated at the skirting and under the tile line reads far better than
a uniformly dirty sheet.

| Asset | Page | Notes |
|---|---|---|
| **ambientCG Tiles036** | https://ambientcg.com/view?id=Tiles036 | **Hero pair, clean half.** The canonical clean white subway tile. Procedural, so it tiles cleanly with no baked lighting. Tags: Bathroom, Bright, Clean, Light, Smooth, Subway, Tiles, White. 129,232 downloads. |
| **ambientCG Tiles133B** | https://ambientcg.com/view?id=Tiles133B | **Hero pair, dirty half. Highest priority in this manifest alongside Tiles036.** Same white family, so it blends without a hue shift, but grimed and aged. Tags: Dirty, Kitchen, Old, Tiles, White. |
| ambientCG Tiles010 | https://ambientcg.com/view?id=Tiles010 | A second white subway tile at a different bond and grout width. Two whites at different scales stop the corridor reading as one repeated sheet. |
| ambientCG Tiles032 | https://ambientcg.com/view?id=Tiles032 | Dark green accent band. Reflective and shiny out of the box, which is what a glazed accent course needs under sodium. Procedural, 86,213 downloads. |
| ambientCG Tiles033 | https://ambientcg.com/view?id=Tiles033 | Yellow, same procedural series. Sits against the `#E0A030` sodium; pull the hue toward cream in the material rather than downloading another. |
| ambientCG Tiles141 | https://ambientcg.com/view?id=Tiles141 | Tagged Urban, Dirty and Stained together, which is unusual and exactly the brief. Beige reads as aged cream. For the worst-maintained stretch of wall. |
| cgbookcase Subway Tiles 01 | https://www.cgbookcase.com/textures/subway-tiles-01 | A different author, so it does not share Substance Designer DNA with the ambientCG set. Ships a channel-packed ARM set as well as metalness/roughness, which saves a packing step in the UE material. Up to 4K. |
| 3dtextures.me Subway Tiles 001 | https://3dtextures.me/2018/02/17/subway-tiles-001/ | A third variation, useful as a decal or trim rather than the main sheet. |

### 1.3 Painted concrete, above the tile line and on ceilings

| Asset | Page | Notes |
|---|---|---|
| **ambientCG PaintedPlaster017** | https://ambientcg.com/view?id=PaintedPlaster017 | **First pick above the tile line.** Photogrammetry rather than procedural, so the roller texture and subtle undulation are real. 242,932 downloads, so it is well behaved and seamless. |
| ambientCG PaintedPlaster003 | https://ambientcg.com/view?id=PaintedPlaster003 | Variation between bays so the ceiling does not read as one plane. |
| ambientCG Concrete020 | https://ambientcg.com/view?id=Concrete020 | Bare structural concrete under the paint, for where paint has failed and for the soffit. Ships base colour, displacement, normal DX and GL, roughness. |
| ambientCG Concrete012 and Concrete040 | https://ambientcg.com/view?id=Concrete012 , https://ambientcg.com/view?id=Concrete040 | Grab both, keep whichever has the flattest albedo: a flat albedo tints better to the `#16161C` charcoal. |
| Poly Haven `painted_concrete` | https://polyhaven.com/a/painted_concrete | Painted concrete already failing. Weathered, worn, dirty, chipped, cracked. 2 m tile. Pairs with PaintedPlaster017 as the damaged half of a blend. |
| Poly Haven `concrete_wall_008` | https://polyhaven.com/a/concrete_wall_008 | Chipped plaster, hairline cracks, **bolt holes** and subtle stains. The bolt holes are the detail worth having: they read as former fixings and sell an old station without a single decal. |

**Coffered and precast:**

- **Poly Haven `precast_concrete_wall`**, https://polyhaven.com/a/precast_concrete_wall .
  **The closest CC0 answer to the coffered requirement.** A stamped rectangular
  pattern that reads as coffering at ceiling scale.
- Poly Haven `concrete_panels`, https://polyhaven.com/a/concrete_panels .
  Corrugated grooves, the nearest CC0 thing to board marking.
- Poly Haven `preconcrete_wall_001` and `preconcrete_wall_001_long`,
  https://polyhaven.com/a/preconcrete_wall_001 . The `_long` version is a
  non-square tile, useful for a long platform wall run.

### 1.4 Brushed steel and painted metal

| Asset | Page | Notes |
|---|---|---|
| **ambientCG Metal009** | https://ambientcg.com/view?id=Metal009 | **The brushed steel pick.** The only ambientCG metal confirmed to carry Brushed alongside Steel and Scratches. Handrails, bench frames, door surrounds, bin bodies. |
| ambientCG Metal032 | https://ambientcg.com/view?id=Metal032 | Clean smooth metal for anything recently replaced. 242,011 downloads, so safe and seamless. |
| ambientCG Metal036 | https://ambientcg.com/view?id=Metal036 | Sibling of Metal032 at a different roughness. Differentiates the handrail from the door surround without a second material graph. |
| ambientCG PaintedMetal001 | https://ambientcg.com/view?id=PaintedMetal001 | Scratched yellow. **Retints straight to sodium `#E0A030` or crimson `#B02030`,** because the paint layer is flat colour with the wear held in the masks. One download, three palette colours. |
| ambientCG PaintedMetal012 | https://ambientcg.com/view?id=PaintedMetal012 | White with rust blooming through. Bin bodies, cable trunking, the back of the departure board housing. |
| ambientCG PaintedMetal006 | https://ambientcg.com/view?id=PaintedMetal006 | Green with rust. Retints to violet `#6C4C9C` cleanly; the rust masks are independent of hue. |
| Poly Haven `metal_plate_02` | https://polyhaven.com/a/metal_plate_02 | Photogrammetry, so it has the surface irregularity the procedural metals lack. Floor hatch, equipment cabinet door, the plated section at a ramp. |
| ambientCG MetalPlates003 | https://ambientcg.com/view?id=MetalPlates003 | Panelised metal for a service door or cable riser. Lower priority. |

### 1.5 Tactile paving and rubber

| Asset | Page | Notes |
|---|---|---|
| **ambientCG TactilePaving001** | https://ambientcg.com/view?id=TactilePaving001 | **The platform-edge strip, first pick.** ambientCG has a dedicated TactilePaving family, which is unusual and means the blister spacing is purpose-built rather than improvised from a stud plate. |
| ambientCG TactilePaving003 / 004 / 005 | https://ambientcg.com/view?id=TactilePaving004 and siblings | Variants of the blister pattern. UK platform edge uses an offset blister grid: pick the closest stud pitch and take one or two only. IDs inferred from the family pattern, medium confidence they all resolve. |
| Poly Haven `rubber_tiles` | https://polyhaven.com/a/rubber_tiles | **Plain rubber matting, first pick.** Smooth dark matte with shallow seams and subtle scuffs. Holds the charcoal end of the palette and gives the eye somewhere to rest between the wet floor and the tile. |
| TextureCan tactile pavement | https://www.texturecan.com/details/47/ | A different blister profile, with friction crosses on top of each dot. Good second option if the ambientCG pitch is wrong. |
| ambientCG Rubber003, Rubber004 | https://ambientcg.com/view?id=Rubber003 | Handrail grip, door seals, stair nosings. |
| ambientCG RubberSubstance001 | https://ambientcg.com/view?id=RubberSubstance001 | **Skip unless someone has Substance.** It ships `.sbsar` and `.sbs`, not a flat texture set. |

### 1.6 Floor: wet, polished, terrazzo

**The important thing here is the roughness map, not the albedo.** None of the
CC0 sets is a wet floor, and that is the right outcome: wetness should come from
the UE5 material, a roughness floor near zero inside a puddle mask plus a
flattened normal, driven by a mask painted per instance. Pick the set with the
cleanest, highest-contrast roughness map and drive it.

| Asset | Page | Notes |
|---|---|---|
| **Poly Haven `smooth_concrete_floor`** | https://polyhaven.com/a/smooth_concrete_floor | **First pick for the platform floor.** Smooth and indoor, so the roughness map is not fighting a rough aggregate surface. A smooth base is what a wet puddle mask needs to look convincing. |
| **ambientCG Terrazzo004** | https://ambientcg.com/view?id=Terrazzo004 | **The polished floor answer.** Terrazzo is period-correct for a main-line concourse and polishes to a genuine specular sheen, which is what the art target's wet reflective look wants. Whole category at https://ambientcg.com/list?category=Terrazzo |
| Poly Haven `concrete_floor_02` | https://polyhaven.com/a/concrete_floor_02 | The dirty variant to blend against the first pick along the platform edge and under benches. Moss staining reads outdoor: desaturate it. |
| Poly Haven `hangar_concrete_floor` | https://polyhaven.com/a/hangar_concrete_floor | Large-scale industrial floor, which is what a main-line hall actually has. Bigger feature scale, so it does not repeat visibly across a wide platform. |
| Poly Haven `concrete_floor_worn_001`, `worn_concrete_floor` | https://polyhaven.com/a/worn_concrete_floor | Wear detail for the boarding zone where footfall is heaviest. |
| ambientCG Terrazzo001, Terrazzo005 | https://ambientcg.com/view?id=Terrazzo001 | Two more aggregate sizes. Pick one for the concourse and one for the platform so the transition between spaces reads. |
| ambientCG Concrete047A | https://ambientcg.com/view?id=Concrete047A | Candidate polished concrete if terrazzo is too decorative. Verify visually first: the evidence it is smooth is weak. |

### 1.7 Secondary

- **Ceramic floor tile and dirty grout.** Poly Haven `dirty_tiles`
  (https://polyhaven.com/a/dirty_tiles) is the **best single answer for dirty
  grout**, and its reddish brown doubles as the oxblood accent. Also
  `interior_tiles`, `worn_tile_floor`, `tiled_floor_001`, `floor_tiles_06`,
  `floor_tiles_09`, `square_tiles`. ambientCG Tiles093 (Black, Dark, Old,
  Random, 2.45 m tile) for a dark utility floor.
- **Peeling paint.** Poly Haven `peeling_painted_wall` is the first pick. Also
  `cracked_concrete_wall`, `concrete_wall_003`, `concrete_wall_006`, and
  `blue_plaster_wall` with the blue retinted.
- **Rust and water staining.** ambientCG Rust001 and Rust004 are **tiling
  surfaces, not alpha decals.** Every dedicated leak and rust decal library found
  is account-gated. Do the vertical streaking procedurally in the UE material
  from world position and a noise texture: cheaper, and it avoids a download.
- **Trackbed.** ambientCG Gravel023 (photoscanned pebbles) is the closest thing
  to ballast; push the normal strength and desaturate, because real ballast is
  angular crushed rock. Also Gravel042, Gravel001, the Gravel category, Asphalt002
  / 006 / 018, Poly Haven `asphalt_01`. **Check ambientCG Road001's preview for
  painted lane markings before using it** in a trackbed.
- **Sleepers and rail.** ambientCG Planks010 has the right shape for a sleeper;
  Wood049 / 076 / 017 / 039, take a weathered grey and darken to creosote. Steel
  rail needs no download: reuse Metal009 with roughness pulled low on the
  railhead and high on the web, plus Rust001 masked into the web and foot.

### 1.8 Surface needs with no CC0 answer

1. **Oxblood or dark red subway tile.** None exists on any CC0 source checked.
   Retint Tiles036 or Tiles032 toward crimson `#B02030` in the material: glazed
   tile albedo is almost flat colour with the interest in the normal and
   roughness, so a hue shift holds up.
2. **True board-marked concrete.** Every board-formed search returned paid or
   licence-unestablished sources. `concrete_panels` is the nearest substitute.
   If genuine board marking is needed, generate it: a plank height map from
   Planks010 driving a concrete albedo produces it in the material graph.
3. **Rust and water streaks as alpha decals.** Do it procedurally, as above.
4. **A genuinely wet floor set.** Correctly absent. A baked wet albedo would
   fight Lumen and would not respond to puddle placement. Drive it from
   roughness.
5. **Angular track ballast.** ambientCG's gravel is rounded pebble
   photogrammetry. Gravel023 with a strengthened normal is usable but not exact.


---

## 2. Meshes

### 2.1 A modular station kit: the gap

| Pack | Source | Licence | Verdict |
|---|---|---|---|
| Modular Underground Metro, loafbrr | https://loafbrr.itch.io/modular-underground-metro | **CC-BY 1.0, not CC0.** Creator permits commercial use and modification; attribution required. | The single closest match. Walls, floors, ceilings, hand rails, signboards, posters, a modular shutter door, 1K textures. Name your own price, zero accepted, no account. But it is a cramped metro bore, not a tall hall, and it carries an attribution obligation. Best used for props and kitbash, not as the shell. Format UNVERIFIED. |
| Modular Concrete Interior, Axel Kulomaa | https://axel-kulomaa.itch.io/modular-concrete-interior and https://opengameart.org/content/modular-concrete-interior-3d | CC0, stated on the listing. | The only genuinely CC0 shell found. Painted concrete walls, floors, ceilings, 2K textures, 3 materials, 48 MB. No tile, no platform edge, no portal. Good for back-of-house corridors. Format UNVERIFIED. |
| Modular Sci-Fi MegaKit, Quaternius | https://quaternius.com/packs/modularscifimegakit.html | CC0. | 270+ grid-disciplined modules, FBX/OBJ/glTF/Blend. Thematically wrong. Proportion reference and grey-box-plus stand-ins only. Do not ship as the station. |
| Modular Buildings, Kenney | https://kenney.nl/assets/modular-buildings | CC0 1.0. | Exterior building modules, not station interior. Listed so it is not re-checked. |
| Downtown City MegaKit, Quaternius | https://quaternius.com/packs/downtowncitymegakit.html | CC0. | Only relevant if F ever builds a street-level station entrance. |

**Nobody gives these away free, in any licence:** an escalator (Kenney's
Conveyor Kit rails are a kitbash stand-in, nothing more), a ticket barrier or
gate line, a platform-edge section with tactile paving, a tunnel-mouth portal, a
passenger help point, a station-grade display housing, and a flat-fronted
suburban EMU nose. All in-house modelling for F1 and F3.

### 2.2 Props and furniture: good coverage

All CC0, all no account, all state their formats unless noted.

| Pack | Source | Licence | Serves |
|---|---|---|---|
| Retro Urban Kit, Kenney | https://kenney.nl/assets/retro-urban-kit | CC0 1.0 | 120+ street props, OBJ/FBX/glTF. PSX-resolution textures, far below the F art target: block-out and proportion reference, not shippable meshes. |
| City Kit (Commercial), Kenney | https://kenney.nl/assets/city-kit-commercial | CC0 1.0 | 50 assets. Hanging sign frames and brackets, awning and fascia shapes. Invented signage only. Siblings on the same terms: city-kit-suburban, city-kit-industrial, city-kit-roads. |
| RPG Urban Pack, Kenney | https://kenney.nl/assets/rpg-urban-pack | CC0 1.0 | 480 assets of bulk urban clutter and litter. Substantially 2D sprites too: confirm the 3D portion before staging. |
| Furniture Kit, Kenney | https://kenney.nl/assets/furniture-kit | CC0 1.0 | 140 assets. Bench and seating proportion reference. |
| Conveyor Kit, Kenney | https://kenney.nl/assets/conveyor-kit | CC0 1.0 | 90 assets. Runs, side rails and support legs are the nearest CC0 stand-in for an escalator truss and balustrade. |
| City Environment Pack, 3DModelsCC0 | https://3dmodelscc0.itch.io/city-environment-pack | CC0, stated. Format UNVERIFIED. | 11 props. Security camera covers the CCTV need, street bench the platform bench, ATM and payphone are the nearest help-point and ticket-machine shapes, street light a wall-fitting kitbash base. |
| City Environment Pack #2, 3DModelsCC0 | https://3dmodelscc0.itch.io/free-cc0-city-environment-pack-2 | CC0, stated. Format UNVERIFIED. | 9 props. Plastic trash bin, parking meter (ticket machine or help point base), bus stop (hanging sign frame and shelter), electrical enclosure (platform-end kit boxes). |
| Industrial 3D Models, 3DModelsCC0 | https://3dmodelscc0.itch.io/free-cc0-industrial-3d-models | CC0, stated. Format UNVERIFIED. | 12 props. Electrical boxes, work lights, platform trolley for engineering clutter. |
| Industrial Props Pack #2, 3DModelsCC0 | https://3dmodelscc0.itch.io/free-cc0-3d-industrial-props-pack-2 | CC0, stated. Format UNVERIFIED. | 9 props. Fire extinguisher is the exact need; wet-floor caution sign is a strong atmosphere prop for a tiled hall; locker. |
| Electronics and Gadgets Pack, 3DModelsCC0 | https://3dmodelscc0.itch.io/free-cc0-electronics-gadgets-pack | CC0, stated. Format UNVERIFIED. | Security camera and display housings. Full contents not established. |
| Poly Haven models | https://polyhaven.com/models and /models/props, /models/industrial/props, /models/furniture | CC0 sitewide, no login. | **The best source here for a small number of good meshes.** Blend/FBX/glTF with full PBR sets at several resolutions. Pick a handful by hand for hero placement. Confirmed present: https://polyhaven.com/a/fire_hydrant. Other slugs UNVERIFIED. |
| Poly Pizza | https://poly.pizza and https://poly.pizza/search/CC0 | **Mixed.** Hosts CC0 and CC-BY side by side; the Google Poly archive content on it is largely CC-BY. | Fast search surface for one-off props: bins, benches, signs, turnstiles. Licence-check every model on its own page. Do not bulk-take. |
| KayKit, Kay Lousberg | https://kaylousberg.com/game-assets/complete-kaykit-collection | CC0, with a request not to resell unmodified or claim authorship (does not restrict game use). Format UNVERIFIED. | Furniture Bits and City Builder Bits. Chunky stylised low-poly, a long way from the reference frame. Proportion reference only. |
| CC0 3D | https://cc03d.com and https://cc03d.com/license/ | CC0, dedicated licence page: no rights reserved, commercial use, no credit. Format UNVERIFIED. | Small photoreal-leaning catalogue. Confirmed: a worn rusty barrier. Worth a look rather than a plan. |

### 2.3 The menu diorama (S13)

Fully served by the above, no separate research needed:

- Tiled wall panel: Modular Concrete Interior (CC0) or Modular Underground Metro
  (CC-BY), surfaced with an ambientCG tile material.
- Bench: 3DModelsCC0 street bench, or a Poly Haven prop.
- Bin: 3DModelsCC0 plastic trash bin.
- Hanging sign frame: Kenney City Kit (Commercial) frames, or the bus stop frame.
  **The sign face must be original artwork made in project, never a downloaded
  texture.**
- Train nose: Modular Train Pack, Quaternius,
  https://quaternius.com/packs/modulartrain.html, CC0, FBX/OBJ/Blend, 11 models.
  **Caveat:** none is a suburban or main-line EMU. The "high speed front" is a
  tapered nose, wrong for a flat-fronted Aventra. Silhouette blocking at menu
  distance only, wrong as a shipping mesh.

### 2.4 Meshes excluded, and why

- **herio low poly subway pack**, https://herio.itch.io/low-poly-subway-pack.
  Free, but bespoke terms ("you cannot sell the assets"), ambiguous about a
  commercial game build. Not worth the argument.
- **BitSoft Japan Subway Train + Tunnel / Station.** Paid.
- **RetroStyleGames modular metro platform and subway tunnel sets.** Paid,
  though the ArtStation store pages are worth a look purely as reference for
  what a good kit's part breakdown looks like.
- **Sketchfab "FREE // Subway Station & R46 Subway".** The R46 is a real New
  York City Subway car, so it almost certainly carries MTA livery and markings.
  Fails the project's legal rule regardless of licence. Licence UNVERIFIED and
  Sketchfab downloads are generally account-gated.
- **Sketchfab generally.** Mostly account-gated, mostly CC-BY or CC-BY-SA.
- **Meshy.ai.** AI-generated, account-gated, and the provenance of generated
  meshes is not a licence position this project should lean on.
- **BlenderKit.** Account-gated through the add-on. Go to Poly Haven directly.
- **CGTrader, TurboSquid, Free3D, 3DModels.org.** Mixed, mostly
  royalty-free-with-terms, per-model licences that each need reading.

---

## 3. Architectural reference photography (F1)

**Standing rule for this whole section.** Photographs of real stations are
legitimate modelling reference. Take architecture, proportion, material, joint
spacing, lighting position and grime distribution. Do not trace the signage
dress. Where a reference shows the operator's marks, read them as "a sign of
roughly this size, mounted this way, at this height" and redraw in the project's
own wayfinding language.

**Licence is a per-file fact, never a per-site fact.** Wikimedia Commons hosts
CC0, CC-BY, CC-BY-SA and public-domain files side by side. Geograph is
predominantly CC-BY-SA 2.0 but states it per photograph. Record for every file
saved: source page URL, file name, author, exact licence, capture date.

### 3.1 Tall main-line halls (the primary target)

Jubilee Line Extension and Elizabeth line coverage on Commons is genuinely
strong, so this need is well served.

| Category | URL | Why |
|---|---|---|
| Canary Wharf | https://commons.wikimedia.org/wiki/Category:Canary_Wharf_tube_station | The Foster JLE box: full-length concourse, escalator banks, platform level, the repeating structural bay. Best single match for "tall main-line loading-gauge hall", and it is the station the project is already building. |
| Canary Wharf interior sequence | File:Canary_Wharf_tube_station,_Jubilee_Line_-_interior_(2)_-_geograph.org.uk_-_4759001.jpg on Commons, plus siblings _(5)_ 4759003 and _(6)_ 4759004 | CC-BY-SA 2.0 Geograph transfers (UNVERIFIED per file). A numbered sequence of one hall from several positions is worth more than scattered singles: it lets a modeller reconstruct the bay rhythm. |
| Southwark | https://commons.wikimedia.org/wiki/Category:Southwark_tube_station | Upper concourse is a 16 m space with a glass roof. The tall-volume-with-toplight variant. |
| Bermondsey | https://commons.wikimedia.org/wiki/Category:Bermondsey_tube_station | Daylight driven down into a deep box. |
| Westminster | https://commons.wikimedia.org/wiki/Category:Westminster_tube_station | The Hopkins escalator box: exposed structure, raw grey concrete, extreme height. A vertical, structural, non-tiled reading. |
| North Greenwich | https://commons.wikimedia.org/wiki/Category:North_Greenwich_tube_station | Wide cobalt-clad cavern with a central row of raking columns. The closest JLE station to a genuinely wide platform hall. |
| Farringdon (Elizabeth line) | https://commons.wikimedia.org/wiki/Category:Farringdon_station_(Elizabeth_line) | Platform, escalators, gatelines, sprayed-concrete lining with panelled cladding. |
| The Elizabeth line station set | https://commons.wikimedia.org/wiki/Category:Railway_stations_served_by_MTR_Elizabeth_line | Fastest route into the whole set from one page: Liverpool Street, Paddington (the long box with the deep escalator run), Woolwich (smaller, quieter box). |
| Stockholm Metro | https://commons.wikimedia.org/wiki/Category:Stockholm_Metro_stations | Sprayed-concrete caverns, wide rather than a bore. Also serves the ceiling need. |
| Featured pictures | https://commons.wikimedia.org/wiki/Category:Featured_pictures_of_railway_stations | Best signal-to-noise starting page on Commons for this whole job: high resolution, well exposed. |
| Empty stations, 2020 | https://commons.wikimedia.org/wiki/Category:2020_on_the_London_Underground | Deserted platforms and concourses. Unusually valuable, because the arena has no crowd and this is the largest body of empty-station imagery that exists. |

Geograph search terms: "Canary Wharf Jubilee", "North Greenwich station
platform", "Westminster Underground escalator", "Crossrail platform",
"Elizabeth line platform". Example: https://www.geograph.org.uk/photo/1034493
(Canary Wharf Jubilee Line station, Rob Farrow, CC-BY-SA 2.0).

### 3.2 Tiled corridors

| Source | URL | Licence | What it shows |
|---|---|---|---|
| Geograph, Stephen Craven | https://www.geograph.org.uk/photo/3848392 | CC-BY-SA 2.0, stated inline | London Bridge lift-to-platform access tunnel. The archetype grimy tiled corridor with a curved ceiling. |
| Geograph, Rossographer | https://www.geograph.org.uk/photo/6069242 | CC-BY-SA 2.0 | Bakerloo platform at Piccadilly Circus: tiling, advertising frames, the tile-to-ceiling junction. |
| Geograph, Rossographer | https://www.geograph.org.uk/photo/6156552 | CC-BY-SA 2.0 | Central line at Bank. Older, grimier tiling and cable runs. |
| Commons | File:Jubilee_Line_Platform,_London_Bridge_Underground_Station_-_geograph.org.uk_-_5660750.jpg | CC-BY-SA 2.0 transfer | JLE platform with platform-edge doors in shot. Serves the platform-edge need too. |
| Commons | File:St_Paul's_Station_-_geograph.org.uk_-_7044844.jpg | CC-BY-SA 2.0 transfer | Clean modern relining over an older bore. |
| Paris Metro | https://commons.wikimedia.org/wiki/Paris_Metro and https://commons.wikimedia.org/wiki/Category:History_of_the_Paris_Metro | Mixed; older scans often public domain | Continuous white bevelled corridors, the reference standard for a tiled passage at wide pedestrian scale, plus the 1950s orange-tile and 1970s Motte relining phases. |

**Known gap:** a wide, grimy, tall tiled corridor at main-line loading gauge was
not found. The London Bridge, Bank and Piccadilly Circus photos are deep-level
tube bores, which is the wrong scale for the arena. Paris Metro corridors and
the older New York subway are the nearest substitutes and neither is a match.

### 3.3 Tunnel-mouth portals: the thinnest need

There is no Commons category for the in-station portal condition.

| Source | URL | Licence | What it shows |
|---|---|---|---|
| Geograph, Richard Sutcliffe | https://www.geograph.org.uk/photo/4727255 | CC-BY-SA 2.0 | Queen Street Low Level, the view west from a platform end straight into the tunnel mouth. **The best single hit for this need.** |
| Commons | https://commons.wikimedia.org/wiki/Category:Tunnel_portals | Mixed | Ring and headwall geometry, mostly open-air main-line rather than in-station. |
| Geograph tag page | https://www.geograph.org.uk/tagged/railway+tunnel+portals | Predominantly CC-BY-SA 2.0, per file | About 125 images of UK portal ring construction, brick and concrete, with wing walls. |
| Commons | File:Mouth_of_disused_railway_tunnel_-_geograph.org.uk_-_996056.jpg | CC-BY-SA 2.0, Shazz | Disused, weathered, overgrown. Derelict-portal mood. |

The practical route is to harvest platform-end shots out of the station
categories in 2.1. That works but needs a human eye going through them.

### 3.4 Coffered concrete, board-marked concrete, sprayed linings

| Source | URL | What it shows |
|---|---|---|
| Washington Metro, via https://commons.wikimedia.org/wiki/Category:Coffered_ceilings_in_the_United_States | Search evidence names files such as "Silhouettes in Metro Center.jpg" | **The strongest precedent anywhere for this project's ceiling.** Harry Weese's single-template coffered concrete barrel vault with concealed indirect lighting behind the platform edge. Worth a dedicated pass. Background: https://en.wikipedia.org/wiki/Metro_Center_station and https://ggwash.org/view/68565/metro-has-11-types-of-station-architecture |
| https://commons.wikimedia.org/wiki/Category:Coffered_ceilings | About 51 files, mixed licence | Coffer grid proportion, mostly historic plaster rather than concrete. |
| https://commons.wikimedia.org/wiki/Category:Concrete_ceilings | About 63 files, mixed licence | Concrete soffits, waffle slabs, ribbed slabs. |
| https://commons.wikimedia.org/wiki/Category:Brutalist_architecture | Mixed licence | Board-marked beton brut, timber shuttering grain legible in the finished face, hammered aggregate. UK exemplars with their own categories: the National Theatre, the Barbican Estate. |
| https://commons.wikimedia.org/wiki/Category:Concrete_textures | About 98 files, mixed licence | Flat concrete surfaces. Reference only, not a shipped texture without a licence check. |

### 3.5 Platform edge

| Source | URL | What it shows |
|---|---|---|
| https://commons.wikimedia.org/wiki/Category:Platform_screen_doors_on_the_London_Underground | About 23 files | The Jubilee line platform-edge doors, cantilevered off the platform rather than a full-height screen. Directly matches the arena concept. |
| https://commons.wikimedia.org/wiki/Category:Tactile_paving_in_train_stations | About 83 files | The tactile strip: blister and lozenge patterns, width, setback from the coping, contrast against the platform floor. |
| https://commons.wikimedia.org/wiki/Category:Platform_markings | About 84 files | Painted edge lines, stopping marks, boarding position marks. **Any lettering in these must be redrawn in the project's own type.** |
| https://commons.wikimedia.org/wiki/Category:Tactile_paving | National subcategories, Japan and Hong Kong among them | Standards differ visibly between countries, which widens the vocabulary. |
| https://commons.wikimedia.org/wiki/Category:Train_station_platforms | Mixed | Coping stones, the platform-to-train gap, drainage, the platform-to-track level difference. |

### 3.6 Escalators, gatelines, service greebles

| Source | URL | What it shows |
|---|---|---|
| https://commons.wikimedia.org/wiki/Category:Escalators_on_the_London_Underground | Pitch angle, balustrade section, incline soffit, advertising frame rhythm along the incline. |
| https://commons.wikimedia.org/wiki/Category:Stairs_on_the_London_Underground | Emergency stair and cross-passage details. |
| https://commons.wikimedia.org/wiki/Category:London_Underground_ticket_gates and https://commons.wikimedia.org/wiki/Category:Ticket_barriers_in_London | Gateline geometry: pedestal spacing, aisle widths, the wide accessible gate. Redraw all graphics. |
| https://commons.wikimedia.org/wiki/Category:London_Underground_infrastructure | **The category to mine for the greebles that sell a real station:** wall-mounted service equipment, cable trays, cabinets, emergency points, ventilation grilles. |
| https://commons.wikimedia.org/wiki/Category:Vauxhall_tube_station | One station covered end to end, escalators plus ticket hall. Useful for how the parts connect. |

Hanging sign gantries have no dedicated category anywhere. Harvest from the
station categories and take only the mounting hardware, the gantry section, the
hanger spacing and the height above platform.

### 3.7 Measured drawings, the highest-value item for a modeller

**Library of Congress HABS/HAER/HALS.** More than 40,000 structures with
large-format photography and, crucially, **measured drawings**, which give
dimensions rather than impressions.
https://www.loc.gov/collections/historic-american-buildings-landscapes-and-engineering-records/about-this-collection/technical-information/

Rights advisory is "no known restrictions on images made by the U.S.
Government; images copied from other sources may be restricted", so it is
effectively public domain for the survey's own photography, but the per-item
rights field still has to be read. Example: Pennsylvania Railroad Station
Rotunda, Pittsburgh, https://www.loc.gov/pictures/item/pa0065/, 7 photographs
plus 6 measured drawings. Search for "railroad station", "terminal",
"concourse", "train shed".

Note on Flickr: "Flickr Commons" means "no known copyright restrictions" as
asserted by the holding institution, which is a weaker statement than a licence.
Ordinary Flickr is per-photo licensed and frequently NonCommercial, so it is not
a safe bulk source.

---

## 4. Train shape reference (F3)

### 4.0 Shape is fair game. Livery is not.

The Class 345 Aventra's exterior geometry, proportions, panel lines and
published dimensions are factual engineering. Copying that shape is the point of
this section. What must never reach the shipped asset, restated because it
creeps back in: any roundel; the operator's grey and white bodyshell with a
single purple sole-bar stripe copied as a livery; New Johnston or a lookalike on
destination blinds, car numbering or any bodyside text; the operator name or
mark; and **purple as the dominant or sole livery colour, because a purple and
white unmarked train still reads as the real thing.**

The substitute dress, from `CLAUDE.md`: charcoal `#16161C` bodyshell, sodium
`#E0A030` cab band, violet `#6C4C9C` door surrounds, an original typeface, a
made-up operator mark.

Entries are tagged SHAPE or LIVERY. A LIVERY tag does not mean useless: it means
the value is mostly in the paint and branding, so read the silhouette and panel
lines from it and never let it drive colour or graphics.

**The standing recommendation, and it is the most useful thing in this section:
model from Class 720 or Class 701 reference plus the Class 345 numbers, and
touch the Elizabeth line photograph pool as little as possible.** The Aventra
siblings share the platform, the body section and the cab architecture but wear
completely different liveries and are run by other operators. The shape is
identical where it matters, the door count comes from the numbers rather than a
photograph, and it removes the risk of the livery bleeding into the asset by
osmosis. The safest thing is for the modeller to have spent their reference time
looking at trains that are not purple.

### 4.1 Published dimensions: the fastest route to a correct blockout

Dimensions are factual and always safe to build to. Primary source
https://en.wikipedia.org/wiki/British_Rail_Class_345 , which draws on
manufacturer and operator data.

| Property | Value |
|---|---|
| Car length, driving cars | 23.615 m |
| Car length, intermediate cars | 22.500 m |
| Body width | 2.772 m |
| Body height above rail | 3.760 m |
| Floor height above rail | 1.145 m (platform edge is nominally 1.100 m, so the floor sits marginally above the platform) |
| Unit length, 9 car | 204.73 m |
| Doors | 3 double-leaf sliding plug doors per side per car |
| Door clear width | 1.450 m each |
| Bogie wheelbase | 2.25 m |
| Bogie centres | 16.00 m |
| Bogie type | FLEXX Eco 5011 |
| Weight | 319 t |
| Capacity | 1,500 total, 454 seated |
| Maximum speed | 90 mph (145 km/h) |
| Electrification | 25 kV 50 Hz AC overhead. **A pantograph well breaks the roof line on the pan car.** |

**Derived, for the blockout:**

- Width to height is 2.772 to 3.760, roughly **1 : 1.36**. The car is taller than
  it is wide, which is exactly what makes it read as a full-height wall down the
  right of the frame.
- **2.615 m of bodyside above floor level** (3.760 minus 1.145). The skirt is the
  1.145 m below the floor minus wheel clearance, and that band hides the running
  gear.
- On an intermediate car, 22.500 m carries 3 doors of 1.450 m: **4.350 m of door
  aperture and 18.150 m of bodyside** across four panels, two ends and two
  between doors.
- Bogie centres 16.00 m on a 22.500 m car gives **3.25 m of overhang each end**.
- The table is internally consistent: (2 x 23.615) + (7 x 22.500) = 204.73 m
  exactly, matching the published unit length. Good accuracy signal.
- **A station platform does not need 204.73 m.** Model one intermediate car at
  22.500 m and one driving car at 23.615 m as two modules, then instance.

Sibling cross-check: Class 720 car length 24.2 m, body width 2.77 m; Class 701
body width 2.77 m. Both have **two** doors per side per car, not three: the three
doors are the Crossrail high-throughput variant. Body width is effectively
identical across the family, confirming the shared cross section. **So take the
section and cab architecture from 720 or 701 photographs, and the three-door
spacing from the 345 numbers.** Sources:
https://en.wikipedia.org/wiki/British_Rail_Class_720 ,
https://en.wikipedia.org/wiki/British_Rail_Class_701 ,
https://en.wikipedia.org/wiki/Alstom_Aventra .

### 4.2 The best single find: FOI-released orthographic drawings

Dimensioned orthographic views beat any photograph for a blockout. Three FOI
requests archived on WhatDoTheyKnow were answered with Class 345 drawings.

| Request | URL | What it holds |
|---|---|---|
| Data sheet for the Class 345 | https://www.whatdotheyknow.com/request/data_sheet_for_the_class_345_tra | A data sheet with **top, side and front views** plus a statistics block. Requested precisely because it carries height, length, width and weight. **Drop this into the modelling package as a backdrop plate.** |
| Technical drawings of Class 345 trains | https://www.whatdotheyknow.com/request/technical_drawings_of_class_345_2 | Drawings released by the project company, file reference `Q234 BMB R1 XMO CR001 50011 Rev 3.0.pdf`. Front, side and top views with dimensions. |
| Specification of Class 345 trains | https://www.whatdotheyknow.com/request/specification_of_class_345_train | Written specification. A cross-check on the dimension table rather than a drawing. |

**Licence UNVERIFIED on all three, and this matters.** FOI disclosures are
normally released with a re-use statement, and the second request explicitly
mentions an attached sheet covering "copyright and how to re-use the disclosed
information". **Read that notice before relying on the drawings. Do not assume
the Open Government Licence.** The dimensions extracted from them are facts and
are safe regardless of how the drawing itself is licensed.

Also https://tfl.gov.uk/cdn/static/cms/documents/elizabeth-line-fleet-information.pdf ,
licence UNVERIFIED, numbers safe.

### 4.3 Drawings and photographs on Wikimedia Commons

Only two licences in this section were confirmable from search evidence.
Everything else is UNVERIFIED and must be read off the file page.

| File or category | URL | Licence | Tag |
|---|---|---|---|
| File:British Rail Class 345 front portion.svg | https://commons.wikimedia.org/wiki/File:British_Rail_Class_345_front_portion.svg | **CC-BY-SA 4.0, verified.** Share-alike: do **not** trace it into a shipped texture or mesh outline without accepting that consequence. On-screen proportion check only. | LIVERY, read geometry only |
| File:Elizabeth Line Class 345 at Reading.jpg | https://commons.wikimedia.org/wiki/File:Elizabeth_Line_Class_345_at_Reading.jpg | **CC-BY 4.0, verified.** Author David Maj. | LIVERY |
| File:Crossrail Class 345.png | https://commons.wikimedia.org/wiki/File:Crossrail_Class_345.png | UNVERIFIED | 6,699 x 188 px, roughly 36:1: a **full-length side elevation strip of a whole unit.** Ideal backdrop plate. Use the outline, door pitch and glazing band only. |
| File:Class 345 Aventra.png | https://commons.wikimedia.org/wiki/File:Class_345_Aventra.png | UNVERIFIED | 1,346 x 164 px, another side profile strip, probably a part unit. |
| Category:Drawings of Elizabeth line rolling stock | https://commons.wikimedia.org/wiki/Category:Drawings_of_Elizabeth_line_rolling_stock | Per file | 4 files. Small, but drawings beat photographs for a blockout. |
| Category:Drawings of rolling stock of the United Kingdom | https://commons.wikimedia.org/wiki/Category:Drawings_of_rolling_stock_of_the_United_Kingdom | Per file | 198 files. **Worth a sweep for modern EMU side elevations carrying no Elizabeth line branding at all, which would be the legally cleanest reference possible.** |

**The Aventra siblings, the recommended pool:**

- **Class 701 Arterio**, https://commons.wikimedia.org/wiki/Category:British_Rail_Class_701 ,
  51 files. **Legally the cleanest photographic pool found:** a non-TfL operator.
  Locations include Long Marston, a storage site, which often means units shot in
  the open with clear side-on access and no platform in the way. Best untried lead
  for a straight-on side elevation and for the running gear.
- **Class 720**, https://commons.wikimedia.org/wiki/Category:British_Rail_Class_720 ,
  42 files. Same cab family, no Elizabeth line dress at all. Station shots at
  Spellbrook, Manningtree, Marks Tey, Sawbridgeworth, Shenfield.
- **Class 710**, https://commons.wikimedia.org/wiki/Category:British_Rail_Class_710 .
  124 files in the London Overground subcategory. Geometry only: **this is still a
  TfL-operated train, so its livery is also off limits** even though it is not
  purple.
- Bogies: the Commons FLEXX **Urban** 1000/2000/3000 categories are the **wrong
  sub-family**, since the 345 runs FLEXX Eco 5011. Useful only as generic modern
  bogie reference for something that will be 90 per cent hidden behind the skirt.

Full-size cab mock-up: File:Crossrail train mock-up London Transport Museum
Acton Depot.jpg on Commons, useful for the cab front read at close range.

### 4.4 Coverage per shape feature

| # | Feature | Best source | Confidence |
|---|---|---|---|
| 1 | Cab front: raked, wraparound windscreen, lower valance | FOI front elevation; the CC-BY-SA 4.0 front-portion SVG; the Acton mock-up; Class 720 and 701 categories | Good |
| 2 | Flush plug doors and surrounds | Published numbers (3 per side per car, 1.450 m); side elevation strips for the pitch | Good |
| 3 | Deep skirt over the bogies | Any platform-level side photograph; the elevation strips give the skirt line height | Good |
| 4 | **Flat roof with air-conditioning pods** | **Poor. The real hole.** The FOI top view is the only solid lead, and it is unseen. | Poor |
| 5 | Continuous window band and its break at door apertures | Elevation strips plus any side photograph | Good |
| 6 | Bogies, couplers, gangways, car-end connection | Low-angle platform shots; Class 701 at Long Marston; FLEXX categories as generic. Coupler is Dellner, a Scharfenberg derivative (confirmed for 710 and 720, reasonable but UNVERIFIED for the 345) | Moderate |
| 7 | Section and dimensions | Solved, see 4.1 | Excellent |

**On feature 4.** No Commons roof view, aerial view or depot overhead of any
Aventra was found; searches kept returning generic building rooftop HVAC. Leads
for whoever picks it up: search the sibling categories for units shot from a road
overbridge or a multi-storey car park next to a station, which is how roof shots
of trains usually get taken; try the Class 701 at Long Marston, since stored
units are often shot from raised ground. Note the 345 is 25 kV overhead, so the
roof carries a **pantograph well on one car**, a large geometry feature for which
no reference was found at all.

### 4.5 CC0 kitbash meshes: skip the kitbash

**Nothing genuinely good exists.** Everything CC0 found is stylised, low poly,
toy-proportioned, or the wrong body shape, or account-gated.

- **BlendSwap "Japanese Subway Train"**, https://blendswap.com/blend/21859 .
  **CC0, verified on the model page.** The closest shape match found: Japanese
  commuter EMUs are main-line gauge with a rounded-rectangle box section, a flat
  roof carrying AC pods, a continuous window band and sliding doors, which is the
  correct family, unlike every tube-train model. **But BlendSwap requires a free
  account to download,** and poly count, topology, UVs and game-readiness are all
  unknown. The cab would need reworking entirely. The only candidate worth
  opening. The CC0 filter crossed with the train tag
  (https://blendswap.com/blends/blicense/CC-0 , 8,591 models) is the one search
  worth running by hand.
- **Kenney Train Kit**, https://kenney.nl/assets/train-kit . CC0, verified, no
  account, 5.3 MB. Roughly 50 models plus spline-ready track. **Not a kitbash
  base:** deliberately stylised, chunky, flat-shaded, toy-proportioned. Useful for
  exactly one thing, a five-minute scale sanity check in the grey box, plus
  possibly the track furniture, which is far less shape-critical than the body.
- **Quaternius Modular Train Pack**, https://poly.pizza/bundle/Modular-Train-Pack-jYEybkFVr1 .
  CC0, verified, no login, FBX/OBJ/blend. Same verdict: wrong fidelity target.
- Rejected: **Meshy** (AI-generated, 1,499 train models claimed CC0, provenance
  not something to stake a shipped asset on, and generated topology is typically
  unusable); **Sketchfab** (mostly account-gated, mostly CC-BY, the named EMU
  models all licence UNVERIFIED); **CGTrader, TurboSquid, Free3D** (bespoke EULA,
  not CC0, the EMU models found are priced around 139 USD); **OpenGameArt "toy
  train lowpoly"** (CC0 but, as the name says, a toy); **Miziziziz
  Retro3DGraphicsCollection** (deliberately PS1-fidelity).

**Recommendation: box-model it.** The dimension table in 4.1 is complete enough
that a rounded-rectangle extrusion with a chamfered roof edge, a raked cab and
three door apertures per car is a short job, and it will be more accurate than
anything above. Kitbashing a toy-shaped CC0 mesh into an Aventra takes longer and
ends up worse.


---

## 5. Menu ambient audio (S13)

### 5.1 The exclusion rule, restated concretely

Anything containing an announcement, a station chime, a jingle, a spoken station
name, or a recognisable operator door tone is banned, whatever licence label it
carries. **A CC0 tag on Freesound is an uploader's assertion, not a provenance
check, and this project has already been bitten by exactly that:** files labelled
CC0 and described as tube chimes turned out to be real operator recordings.
Those are permanently excluded and are not re-listed here.

Filter for whoever downloads:

1. Listen to the whole file at raised gain, not the first few seconds.
   Announcements are sparse.
2. Reject any file whose title, tags or description mention announcement, chime,
   PA, tannoy, "mind the gap", a station name, an operator name or a line name.
3. Prefer somewhere with no PA at all: a plant room, basement, warehouse, car
   park, stairwell, road tunnel. A generic industrial hum is safer than any
   genuine station recording and, filtered and looped, is indistinguishable at
   menu volume.
4. Prefer synthesised or heavily processed drones over field recordings. A
   synthesised drone cannot contain an announcement at all.

### 5.2 Recommendation

**Take the bed from a synthesised CC0 drone, not a field recording.** Base:
https://opengameart.org/content/sci-fi-drone-loop, a low sine-built drone
described as suiting a computer hum; pitch it down and it reads as distant plant
machinery. Layer under a hall or basement tone from the Signature Sounds CC0
Room Tones pack, https://signaturesounds.org/store/p/room-tones (CC0, royalty
free, no attribution, high-resolution WAV). Both halves are CC0 and both are
provably free of announcement content.

Check https://freesound.org/people/Kinoton/sounds/353159/ first only because if
it is genuinely CC0 it is a one-file solution: "Room Tone, Sci Fi, Large Hall",
soft ventilation and hollow rumble, explicitly loopable and for a large space.
Licence UNVERIFIED.

### 5.3 Other CC0 candidates, verify the per-file licence block

OpenGameArt submissions can list several licences at once, and some are marked
CC0 in the title but carry a different block. Verify on the page.

- https://opengameart.org/content/background-space-track - drone loop, synthesised, uneasy edge that suits the game.
- https://opengameart.org/content/loopable-dungeon-ambience - low-frequency wind with water drips. Drips may be too eventful for a menu; test.
- https://opengameart.org/content/cc0-background-ambience - general ambience beds.
- https://opengameart.org/content/30-cc0-sfx-loops - 30 loops, 3 ambient.
- https://opengameart.org/content/cc0-sounds-library and https://opengameart.org/content/cc0-sound-effects - bulk sweep only.

Freesound CC0 filtering, which is worth knowing because the facet is easy to
miss: in the web interface use the "licenses" facet in the right-hand column and
click "Creative Commons 0". The URL form is
`https://freesound.org/search/?q=room+tone&f=license:%22Creative+Commons+0%22`.
Via the API the filter string is `license:"Creative Commons 0"`. Freesound only
permits three licence values on upload, so that exact string is reliable.

Freesound candidates, all licence UNVERIFIED, all needing a per-file check:
sons_pt 255522 ("Empty (large) Room - Tone", mid-side, 48 kHz 24 bit),
Juane170058 407553 ("Room Tone Ambiance in Empty Warehouse", loop-ready; a
warehouse has no PA), Kinoton 558840 ("Room Tone, Corridor, Howling Wind"),
ProductionNow 455045 ("Train Rumble and Rattle", loopable, but it is a train
recording so listen for a horn, bell or PA bleed).

### 5.4 Audio excluded, and why

| Source | Why |
|---|---|
| Coghezzi "Horror Metro Train Pack", https://freesound.org/people/Coghezzi/packs/45881/ | Explicitly a metro pack, which is exactly the category where an operator chime or announcement hides. Given this project's history, skip entirely. |
| swelltoe77 "Sounds of NYC Subway", https://freesound.org/people/swelltoe77/packs/9918/ | A real operator's system. NYC subway recordings routinely carry announcements and door chimes. Listed so nobody rediscovers it and thinks it is new. |
| klankbeeld room tones, Freesound 212137 and 212202 | Excellent hall tones with dimensions in the title, but klankbeeld's library is largely CC-BY, not CC0. Outside the stated constraint. |
| BBC Sound Effects archive, https://sound-effects.bbcrewind.co.uk/ | The RemArc licence is personal, educational and research use only. Commercial use needs a separate licence. Also full of genuine operator recordings. |
| Pixabay audio, https://pixabay.com/service/license-summary/ | **No longer CC0.** Pixabay replaced its pre-2019 CC0 arrangement with the Pixabay Content License: royalty free and commercial-use-permitted, but the content stays copyrighted and it is not a public dedication. Fails a strict CC0-only test. |
| ZapSplat | Site default is a bespoke licence with attribution conditions on the free tier. Only its explicitly CC0-tagged section would qualify, and OpenGameArt is cleaner. |
| Sonniss GDC Game Audio Bundle, https://sonniss.com/gameaudiogdc/ | Royalty free worldwide, commercial use, no attribution, but a bespoke licence rather than CC0. **Legally safe for a shipped game and an excellent library.** Flagged for the owner in case the CC0-only rule is a preference rather than a hard constraint. |
| Kenney audio | Already staged, and the catalogue is UI, interface, impact and digital packs. It has no ambient or room-tone loop, so it cannot fill this need. |

---

## 6. Inspect before staging: the open legal flags

Four items in this document carry a legal question that search could not settle.
None is likely to be a problem; all four need eyes before the file is staged.

1. **Modular Underground Metro (2.1) ships signboards and posters.** A hobbyist
   metro pack is unlikely to carry a real operator's mark, but it is unverified.
   Inspect mesh by mesh and texture by texture. Anything carrying a real operator
   mark is unusable. It is also CC-BY, so using any of it means carrying a credit
   line: decide that before it goes in.
2. **The bus stop in City Environment Pack #2 (2.2).** A generic bus stop is
   fine; one carrying a real operator's mark or typeface is not.
3. **Photogrammetry scans on ambientCG and Poly Haven** can capture painted
   markings. Reject on sight anything with text, numerals or a logo in the base
   colour map. Specifically named: check ambientCG Road001 for lane markings
   before using it in a trackbed.
4. **The FOI drawings (4.2) come with a re-use notice that has not been read.**
   Read it before relying on the drawings themselves. The dimensions are facts
   and are safe either way.

Two licence positions to decide rather than inspect:

- **cgbookcase** is CC0 at medium confidence, from two secondary sources rather
  than its own licence page. Confirm on the page.
- **Sonniss GDC Game Audio Bundle** (5.4) is not CC0 but is permissive enough for
  a shipped game, no attribution required. Excluded here because the brief says
  CC0 or public domain only. Worth raising if that was a preference rather than a
  hard rule, because it is a much better library than anything in 5.3.

## 7. What this research could not settle

Beyond the per-item flags above:

- **Every download URL is unverified**, because nothing could be fetched. Both
  scriptable sites publish a keyless API that returns authoritative file URLs,
  and the fetch script uses those rather than the constructed patterns.
- **Every per-file licence on Wikimedia Commons and Geograph.** Commons is
  explicitly mixed-licence and the licence is a per-file fact. Only two files in
  this whole document had their licence confirmed from search evidence, both in
  section 4.3.
- **File formats for the five 3DModelsCC0 packs, Modular Concrete Interior,
  KayKit and cc03d.** The licence statements came through clearly; the formats
  did not.
- **Whether the OpenGameArt audio entries are CC0 alone.** OpenGameArt
  submissions can list several licences at once, and some are marked CC0 in the
  title but carry a different block.
- **The Signature Sounds CC0 statement (5.2)** rests on one unread page, and it
  is the strongest audio recommendation here. Read it before relying on it.
- **Commons category file counts** were reported by search summaries and will
  drift.

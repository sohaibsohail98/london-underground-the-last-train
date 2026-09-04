# Free asset acquisition list

Vetted 2026-09-04. Free, licence checked, UE 5.8 and macOS Apple Silicon usable.
The project has a working C++ chassis and no art. This is the list to close that
gap through Phases D to G without buying anything.

## Two filters every asset must pass

1. **TfL filter.** No roundel. No Johnston or New Johnston typeface. No
   reproduction of the official Tube line diagram. No real operator livery, logo
   or wordmark in its house style. No transcribed real announcement recordings.
   Real station names and real geography are fine and are the point. See
   `docs/reference/branding-precedent.md` for why real London plus invented
   signage is the normal path, not an unusually cautious one.
2. **Repo filter.** Almost all Epic and Fab content is UE-Only Content or Fab
   Standard licence: we may ship it compiled into the game and sell that game,
   but we may not re host the raw `.uasset`, FBX or textures in a public repo.
   So imported packs are gitignored (see `.gitignore`) and fetched per this
   file. Only CC0 and MIT assets, and our own work under `Content/LastTrain/`,
   are committed.

## Where imported packs land, and the gitignore

`.gitignore` excludes the folders third party packs import into:
`Content/ThirdPerson/`, `Content/FirstPerson/`, `Content/Characters/`,
`Content/Megascans/`, `Content/Lyra/`, `Content/CitySample/`,
`Content/GameAnimationSample/`, `Content/NiagaraExamples/`, `Content/Subway/`,
`Content/Audio/Sonniss/`, and a few others. If a pack imports somewhere new, add
that folder to `.gitignore` before committing anything.

Our own assets stay under `Content/LastTrain/` and are committed via Git LFS as
`.gitattributes` already sets up.

## If you only fetch five things

| # | Asset | Source | Licence | For |
|---|---|---|---|---|
| 1 | Game Animation Sample Project | Fab, publisher Epic Games | UE-Only Content, free | Player and zombie animation backbone. 500+ retargetable clips on the UE5 Mannequin skeleton, motion matching included. Native 5.8. |
| 2 | City Sample Crowds | Fab, Epic Games | UE-Only Content, free | The route to 5 to 8 distinct undead Londoner types. MetaHuman derived civilians, 6 bodies, 12 heads, swappable clothing and accessories, mannequin compatible skeleton. |
| 3 | Subway Environment | Fab, Epic Games, was the old Permanently Free collection | UE-Only Content, free | Grey box to recognisable Tube platform. Modular platform, tiled walls, columns, a train, grime decals, steam and water Niagara, ambience. Generic NYC and Toronto styling, no real network IP. UE4 origin, re save materials in 5.8. |
| 4 | Niagara Examples Pack (UE 5.7) | Fab, Epic Games | Standard, free | Combat FX. 50+ systems: bullet impacts, muzzle sparks, blood hit dissolves, footstep effects, smoke. Built for 5.7, opens in 5.8. |
| 5 | Sonniss GDC Game Audio Bundle | gdc.sonniss.com | Royalty free, commercial OK, no attribution. Not for AI training, no reselling individual sounds. | Sound. Tens of GB of pro WAV across many years: gunfire, concrete footsteps, machinery hum, ambience, crowd, creatures. |

---

## A. Environment, the London Underground look

Highest priority. Get the platform reading as a Tube station first.

1. **Subway Environment** (Epic, Fab, free, UE-Only Content). Modular subway
   inspired by NYC and Toronto. Platform, tiled walls, columns, ceiling, stairs,
   a detailed train, graffiti and grunge decals, water and steam Niagara, sound.
   UE4 origin, forward ports to 5.x, expect to re save materials. The single
   highest impact grab.
2. **Downtown West Modular Pack** (Epic, PurePolygons, Fab, free, UE-Only
   Content). City street kit. Not subterranean, but the tiling, trims, concrete
   and brick materials and empty signage boards suit a ticket hall or station
   entrance and share a register with Subway Environment.
3. **Downtown Alley / Back Alley pack** (Epic, Fab, free, Standard, released May
   2025). About 50 modular assets: wet asphalt, brick, dumpsters, pipes,
   cabling, grime decals, puddle materials. The wet ground and grime layer you
   want on a platform.
4. **Stack O Bot** (Epic, Fab, free, UE-Only Content, 5.6 or 5.8). Not a Tube
   kit. Worked PCG examples for environment generation, Level Instances, commented
   materials. Use as the template for procedurally lining a tunnel section with
   sleepers, cable trays and lights.
5. **Content Examples** (Epic, Fab, free, UE-Only Content, 5.8 native). One
   level per engine feature: decals, materials, Niagara, lighting, PCG, audio.
   Reference for correct setup, not shippable content.

### Surfaces and decals, repo safe

Use these instead of Megascans. Megascans stopped being blanket free in 2025;
only a rotating subset on Fab is free now, so check the price flag on each one.

- **AmbientCG** (ambientcg.com). CC0, no account, direct download, 2K to 8K PBR.
  Deep catalogue of concrete, tile, metal, rubber, painted surfaces, grime,
  blood and stain decal maps. Safest surface source: CC0 means you can commit
  the textures if you want. Import as textures, build the material.
- **Poly Haven** (polyhaven.com). CC0. PBR textures and HDRIs. A sodium lit
  night HDRI is useful for entrance exteriors.
- **Megascans free subset** on Fab: filter Category Surfaces, Price Free. Free
  with an Epic account, Standard licence, permanent once claimed. Check each
  one's price first.

---

## B. Characters and costume, unique zombies

Path to 5 to 8 distinct undead Londoner looks, cheapest effort first.

1. **City Sample Crowds** (Epic, Fab, free, UE-Only Content). Start here. Rigged
   characters adapted from MetaHumans: 6 bodies, 12 heads, 10 hair grooms, a
   wardrobe of clothing and accessories, a blueprint that randomises the
   combination. Mannequin compatible skeleton. Recolour and dirty the materials,
   add a wound decal layer, drive with zombie anims, and you have your commuter,
   tourist, cleaner, office worker and so on. Lighter than full MetaHumans but
   still groom plus high res heads, so budget it.
2. **MetaHuman** (Creator plus plugin, ships with 5.8, runs on Apple Silicon).
   Standard UE EULA, free under the revenue threshold. For 2 or 3 hero zombie
   faces only, a boss commuter or a station staff type. Each high LOD MetaHuman
   is expensive: groom hair, 8K face textures, complex material. Force low LODs
   for anything not a hero. MetaHuman Animator markerless facial capture is
   Windows only, but zombies do not need facial mocap so that does not block
   anything.
3. **Modular clothing.** The free options are thin. Most civilian clothing packs
   on Fab that layer garments onto MetaHuman and Mannequin skeletons, including
   ones with orange coveralls, hi-vis and worker kit, are paid. Verify each in
   the launcher and only pull ones flagged Free. The reliable free path is the
   City Sample Crowds wardrobe plus small bespoke pieces. For hi-vis
   specifically, model a plain vest in Blender, about 10 minutes, or grab a CC0
   safety vest from itch.io, and give it an emissive trim material in the sodium
   or crimson palette.
4. **Mixamo** (Adobe, free with an Adobe ID, royalty free commercial, cannot
   resell raw data). 2000+ mocap animations and some rigged everyday person
   characters. Source of zombie shuffle, lunge, melee, hit react and death
   clips that retarget to the mannequin via IK Rig. Adobe has mothballed
   Mixamo: works today, unsupported, had a multi day outage in 2025. Download
   what you need now, do not build a live pipeline on it.
5. **itch.io CC0 humans.** Search itch.io game assets, filter Free, tag
   character or zombie, licence CC0. Low poly and stylised, not the grimy
   realism of the reference, but fully repo safe. Useful as far horde LODs.
6. **Realistic zombie pipeline.** Take a City Sample or MetaHuman body, apply
   blood and wound decals (Niagara Examples has a hit dissolve, AmbientCG has
   blood and gore decal maps under CC0), desaturate the skin material. This is
   the intended free route to a believable zombie.

### Animation sets for the mannequin skeleton

- **Game Animation Sample Project** (Epic, Fab, free, 5.8 native). 500+
  animations, motion matching, traversal, on the UE5 Mannequin with runtime
  retargeting. Best free locomotion. Base layer for player and zombies.
- **Animation Starter Pack** (Epic, Fab, free, UE-Only Content). 62 classic
  animations including simple melee swings usable for zombie attacks. Lighter
  and older than GASP.
- **Lyra Starter Game** (Epic, Fab, free, UE-Only Content, updated to 5.8). Mine
  the modular weapon system, Linked Anim Layers architecture and locomotion, not
  the sci-fi art.
- **Mixamo** for the specifically undead clips GASP does not have.

---

## C. Weapons and FX

1. **Niagara Examples Pack (UE 5.7)** (Epic, Fab, free). Bullet impacts, sparks,
   footstep effects, hit dissolves, smoke. Built for 5.7, opens in 5.8. Pair
   with a crimson blood decal layer for gore.
2. **Lyra weapon system** (Epic, Fab, free). Complete hitscan and projectile
   framework with recoil, spread, reload, ammo, first person arms, muzzle flash
   and impact hookups. Our C++ already does hitscan, so use Lyra as the FX and
   anim wiring reference, and optionally its FP arms and generic weapon meshes.
   Names are generic, no third party weapon IP.
3. **First person weapon models, CC0, repo safe:**
   - Free CC0 Guns and Explosives Pack by 3dmodelscc0 on itch.io. 19 firearms,
     CC0.
   - Low Poly Gun Models by chilly_durango on itch.io. Pistols, shotguns, SMGs,
     rifles, CC0 1.0.
   - Kenney Blaster Kit, kenney.nl, CC0. Stylised, zero licensing risk.
   - These are static meshes. Animate with reused Lyra or Mixamo FP anims, or
     accept simple procedural recoil.
   - Fab free FPS weapon and animation packs exist but most are paid. Verify
     each in the launcher, only pull Free with a Standard licence.
4. **Decals.** Niagara Examples covers hit dissolves. For persistent decals use
   AmbientCG blood and scorch maps (CC0) and build a material with the palette
   crimson `#B02030`.
5. **Sound:**
   - **Sonniss GDC Game Audio Bundle**, gdc.sonniss.com. Royalty free,
     commercial OK, no attribution. Not for AI training, no reselling individual
     sounds. Multiple years, each hundreds to thousands of WAVs. Gunfire,
     concrete footsteps, machinery hum, ambience, distant rumble, creatures.
   - **Freesound.org**. Filter licence CC0, or CC-BY and keep the attribution
     file the repo already has. One offs: train brake squeal, turnstile clack,
     crowd murmur, zombie vocals.
   - **Kenney audio packs**, kenney.nl, CC0. Interface and impact sounds, good
     for UI and placeholder hits.
   - Per CLAUDE.md: no transcribed real announcements, no operator audio
     branding. Generate station announcements with TTS or a friend's voice.

---

## D. Tooling and systems

Epic first party sample projects to mine wholesale. All free, all UE-Only
Content: usable in UE based products including commercial ones, no re
distributing the raw assets or source as standalone content, no royalty beyond
the standard UE EULA.

1. **Lyra Starter Game** (Fab, free, 5.8). GAS weapon system, ability system,
   modular Anim Blueprint architecture, Common UI framework, Enhanced Input
   config, equipment and inventory, game phase flow. Large, several GB. Pattern
   reference for our AnimBP and weapon FX wiring.
2. **City Sample split packs** (Fab, free). Crowds, Vehicles, Buildings.
   Mass Entity crowd system, MetaHuman crowd LOD setup, modular building kit.
   Prefer the split packs over the full City Sample project, which is tens of GB
   and historically rough on Mac.
3. **Game Animation Sample Project** (Fab, free, 5.8). Motion matching config,
   pose search databases, the modern locomotion AnimBP, traversal system,
   physics additive layer. Build character movement on this.
4. **Stack O Bot** (Fab, free, 5.6 or 5.8). Worked PCG examples, Level
   Instances, commented Blueprints and materials. Template for procedurally
   dressing tunnel and platform modules.
5. **Content Examples** (Fab, free, 5.8). One level per feature. Reference.

### Built in engine plugins, macOS safe, no download

Mass Entity and MassAI for crowd. Smart Objects. StateTree, a modern
replacement for Behaviour Trees: the zombies currently do a naive `MoveToActor`
every tick even after the B1 throttle, and a StateTree or Behaviour Tree plus
EQS would be a cheap upgrade later. PCG and PCG Geometry Script for tunnel
lining. Motion Matching and Pose Search, used by GASP. Common UI for the HUD.
MetaSounds for procedural station hum and train audio. All ship with 5.8, all
run on Apple Silicon.

### Third party open source plugins, MIT, repo safe

Can be added as git submodules, unlike Fab content.

- **PCGExtendedToolkit** (github.com/PCGEx/PCGExtendedToolkit, MIT). Graph
  theory, pathfinding, spatial ops and filtering for PCG. Verify it compiles
  against 5.8 on Apple Silicon.
- **PCGPathfinding** (github.com/spood/PCGPathfinding). A* over PCG to generate
  splines between two points. Verify licence.
- **PCG Pro Tools** (github.com/eXeViruZ/PCGProTools). Custom PCG nodes for 5.7
  and 5.8. Verify licence before committing to a public repo.
- Caution: many free Fab code plugins ship Win64 binaries only and will not
  build on Mac. Check Supported Platforms includes Mac, or that full C++ source
  is included.

---

## E. Fonts and signage

Use a SIL OFL geometric or grotesque sans that reads as clean modern wayfinding
without being a Johnston clone. Johnston's defining tics are the diamond tittles
on i and j and full stops, and the perfectly circular O. Avoid fonts that
reproduce those, and avoid anything marketed as a Johnston replacement or
Underground revival, for example P22 Underground, which is a paid Johnston
revival and wrong on both cost and IP grounds.

All three below are OFL, free for commercial use, and can be committed with the
OFL text kept alongside them.

1. **Overpass** (Red Hat, Google Fonts). OFL 1.1. Full weight range plus
   Overpass Mono, good for platform number displays and departure boards.
   Derived from US Highway Gothic, so it reads unmistakably as transit signage.
   Tall x-height, open apertures. Its heritage is American highway, which
   sidesteps the LU IP question while still saying transit. Best fit for
   concourse and platform signage.
2. **Public Sans** (US government, USWDS). OFL 1.1. A strict neutral grotesque
   with no personality quirks, designed for official and administrative
   contexts, distinct 1 l I and 0 O. Ideal for byelaw posters and safety notices
   where the text should look institutional but generic.
3. **Barlow** (Google Fonts). OFL 1.1. A slightly rounded low contrast grotesk
   explicitly inspired by California highway signage. Another safe transit
   flavoured choice, very large family. Note: the NeoStack plugin also bundles
   Barlow for its own UI, unrelated to game use.

Record the final choice in `docs/art-direction.md` once picked, since every
downstream UI matches it.

### Signage decals and pictograms

No confirmed free and confirmed clean signage decal pack found. Build your own:
ISO 7010 safety pictograms, the running person exit symbol, no entry, warning
triangles, are an international standard and are not TfL IP. ISO 7010 symbol
SVGs are on Wikimedia Commons under public domain or CC. Compose way out, line
name and platform signage with Overpass on a violet `#6C4C9C` or sodium
`#E0A030` bar. This is both the safest legal route and the most on brand.

---

## Watch out for

- **Megascans is not blanket free any more.** Only a rotating subset on Fab is
  free since 2025. Check the price flag on every surface. For guaranteed free
  repo safe surfaces use AmbientCG and Poly Haven, both CC0.
- **Fab first party, publisher Epic Games, is reliably free.** Third party Fab
  listings need the price checked in the launcher. Several ideal looking subway
  kits, civilian clothing packs and FPS weapon packs are almost certainly paid.
- **UE-Only Content and Fab Standard block re hosting raw assets in a public
  repo.** Ship them compiled into the game, do not commit the source. This is
  why imported packs are gitignored and this file exists.
- **City Sample full project is a macOS pain point.** Tens of GB, aggressive
  World Partition and Nanite, historically unstable on Mac. Use the split packs.
- **MetaHumans are heavy.** Fine for 2 or 3 hero zombies, not a 30 strong horde.
  Use City Sample Crowds for the bulk and force low LODs.
- **MetaHuman Animator markerless facial capture is Windows only.** Creator and
  the plugin run on Apple Silicon in 5.8. Not blocking for zombies.
- **Mixamo is abandonware.** Works today, unsupported by Adobe, had a multi day
  outage in 2025. Download what you need now.
- **Sonniss licence.** Royalty free and commercial OK, but not for training AI
  models and no reselling individual sounds.
- **Windows only plugins.** Many free Fab code plugins ship Win64 binaries only.
  Check Supported Platforms or that full C++ source is included.
- **Real TfL IP traps.** Any pack literally themed London Underground that ships
  the roundel, the Johnston typeface, the real line diagram or LU rolling stock
  livery. Generic metro and subway is abundant and is what to use. Do not
  substitute a London branded pack.
- **Engine version drift.** Niagara Examples is 5.7. Subway Environment and
  Animation Starter Pack are 4.x origin. Lyra, GASP, Stack O Bot and Content
  Examples are 5.8 current. Older packs generally forward port but expect to re
  save materials and fix the odd broken reference. Test each in a throwaway 5.8
  project before integrating.

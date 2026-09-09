# Free asset sweep, 2026-09

A one-time extensive sweep for free game assets, run 2026-09-09 by a research
subagent. Research only. Nothing was downloaded, no account was created, no
licence gate was clicked, and no repo file was changed except this one.

## What this deliberately does not repeat

This sweep excludes everything already covered, so it is additive by
construction. It does not re-list:

- Anything staged in `_incoming_assets/`: the ambientCG surface zips, the four
  Poly Haven night HDRIs, the Quaternius Universal Animation Library and
  Universal Base Characters, the OFL fonts (Barlow, Overpass, Public Sans,
  Hammersmith One), the Kenney CC0 audio packs, the CC0 industrial-prop, gun and
  city-kit archives, the 26 ISO 7010 pictogram SVGs, and the three CC0 Freesound
  synth drones.
- Anything in `docs/reference/asset-sources-phase-f.md`: the full ambientCG and
  Poly Haven surface shopping list, the Wikimedia and Geograph reference
  photography, the Class 345 dimension work and FOI drawings, the menu ambient
  audio recommendation, the "no CC0 modular station kit" finding.
- Anything on the `claude/free-game-assets-5xgiow` branch: the de-branded S
  Stock carriage at `SourceArt/ThirdParty/SStock/`, `tfl-dimensional-reference.md`,
  `rejected-assets.md` (Paddington Plain font, the stripped S Stock textures),
  `free-assets.md`, `free-asset-sourcing-guide.md`, and the
  `canary-wharf-research/` set including `free-3d-and-kits.md`.
- Anything in the commissioned guide at
  `~/Downloads/Free Asset Sourcing Guide for London Underground Zombie Shooter.md`:
  the timblewee Sketchfab trains, Dekogon City Subway (Fab), Kaiju Station,
  Mixamo, Lyra and Low Poly Shooter Pack, the Sonniss bundle as a concept, the
  CodeLikeMe blood pack, MeloTTS.

Where this sweep touches a source already named above, it adds a specific new
item from it that the earlier docs did not list.

## Licence key, used in every table

- **yes CC0-MIT**: CC0, MIT, public domain, or a stated "free for commercial use,
  no attribution" custom licence. Raw file may be committed.
- **attribution**: CC-BY, OGA-BY, OFL. May be committed if the credit and
  licence text travel with it (`Content/LastTrain/CREDITS` per repo discipline).
- **UE-only**: Fab or Epic licence. May be shipped compiled into the game, never
  committed as raw `.uasset` or source. Listed only where relevant.
- Anything CC-BY-NC, CC-BY-SA where it would viral the project, or
  personal/educational-only is in the Skipped list with the reason, not in a
  table.

---

## 1. Environment / station art

### 1.1 Modular kits and station sections

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| CC0 Metro Station Prototype | https://plewr.itch.io/cc0-metro-station-prototype | CC0 1.0, stated on page ("no generative AI was used") | yes CC0-MIT | F1 blockout, tunnel and platform massing | One `MetroPrototypeV1.blend`, 2 MB. A Metro 2033 inspired low-poly prototype, not a finished kit. Blockout and proportion reference for a bore-scale tunnel. No signage on the page but open the .blend before use. Name-your-price, zero accepted, itch account to download. |
| KayKit Prototype Bits 1.0 | https://github.com/KayKit-Game-Assets/KayKit-Prototype-Bits-1.0 | CC0 1.0 Universal, in `LICENSE.txt` | yes CC0-MIT | Grey-box geometry, grid-disciplined blockout modules | GitHub mirror of the itch pack, direct `git archive` or zip download, no account. About 5.5 MB. Stylised grid blocks, block-out only, wrong fidelity for the hero platform. |
| KayKit City Builder Bits 1.0 | https://github.com/KayKit-Game-Assets/KayKit-City-Builder-Bits-1.0 | CC0 1.0 Universal, in `LICENSE.txt` | yes CC0-MIT | Street-level station entrance massing, menu diorama background silhouettes | GitHub mirror, direct download, about 4.8 MB. Chunky stylised low-poly, a long way from `reference-frame.png`. Proportion and silhouette only. |
| Blend Swap "Subway Station Entrance" (scene 19305) | https://blendswap.com/blend/19305 | Per-blend licence, filter to CC0; this one needs the page checked | attribution or yes, check page | A street-to-station entrance box for F1 | Blend Swap requires a free account to download. Licence is per file: confirm CC0 or CC-BY on the blend page before staging. Reference for the entrance transition, which no other free source covers. |
| ambientCG Atlas assets (rubble, debris, kit pieces) | https://ambientcg.com/list?type=Atlas | CC0 1.0, https://docs.ambientcg.com/license/ | yes CC0-MIT | Debris and rubble scatter for the derelict dressing | Not a kit, but ambientCG's Atlas type ships small modelled clusters (bricks, planks, debris) as OBJ plus PBR. Fills the "engineering clutter on the trackbed" need. Keyless API at `https://ambientcg.com/api/v2/full_json?id=<AssetID>`. |

**Standing finding still holds:** there is no CC0 modular main-line station
shell, platform edge, escalator, gate line or tunnel portal. Nothing in this
sweep changes that. The new items above are blockout aids, not shippable
geometry.

### 1.2 Individual CC0 station props

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| Poly Haven "Security Camera 01" | https://polyhaven.com/a/security_camera_01 | CC0, https://polyhaven.com/license | yes CC0-MIT | CCTV camera on platform walls and the concourse | 14k tris, full PBR, Blend / glTF / FBX / USD at several resolutions. 22,500+ downloads, well behaved. Keyless API `https://api.polyhaven.com/files/security_camera_01`. |
| Poly Haven "Security Camera 02" | https://polyhaven.com/a/security_camera_02 | CC0 | yes CC0-MIT | A second CCTV body so cameras do not all read identical | 12k tris, same format spread. |
| Poly Haven "Metal Trash Can" | https://polyhaven.com/a/metal_trash_can | CC0 | yes CC0-MIT | Platform litter bin, back-of-house bin | Weathered metal, removable lid, industrial wear already in the maps. |
| Poly Haven "Fire Hydrant" | https://polyhaven.com/a/fire_hydrant | CC0 | yes CC0-MIT | Street-level entrance dressing | Confirmed present in the earlier sweep, restated here as a concrete slug. |
| Poly Haven models index | https://polyhaven.com/models and /models/props , /models/industrial/props , /models/furniture | CC0 sitewide, no login | yes CC0-MIT | Hand-picked hero props: benches, cabinets, pipes, valves, cable spools | Best CC0 source for a small number of good meshes. Pick by hand. Search "bench", "cabinet", "pipe", "valve", "junction box", "cable". |
| Flap Barrier Turnstile (Maxwell autodoors) | https://sketchfab.com/3d-models/flap-barrier-turnstile-98fcb3e25c4d4eadb3a36b77d584d98b | Per model, check page: Sketchfab mixes CC0 and CC-BY | attribution or yes, check page | Gate line pedestal shape for F1 | Sketchfab download is account-gated. Licence must be read on the model page. Modern flap-barrier geometry, the right shape for a gate line, no operator graphics visible in the thumbnail but confirm. |
| Subway Turnstile (Gunnar Correa) | https://sketchfab.com/3d-models/subway-turnstile-eed7f05ffcff4251a3f1cbef98342093 | Per model, check page | attribution or yes, check page | Tripod turnstile, older gate-line style | Same account gate and per-model licence caveat. Ticket gate, protection grid and directional base modelled. |
| cc0-textures.com models and props | https://cc0-textures.com/ | CC0, stated ("no attribution required"), but it aggregates from ShareTextures, Texture Haven and the old CC0 Textures, so provenance is mixed | yes CC0-MIT, verify per file | Extra surfaces and the odd prop | 3,314 assets. Not the same site as ambientCG. Useful as a second net, but because it re-hosts from other libraries, check the per-asset source line before staging. |

### 1.3 CC0 decals: grime, graffiti, cracks, stains, blood

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| 3DTexel Free PBR Decals | https://3dtexel.com/decals/ | CC0, stated on page ("280+ CC0 decal textures", "ready to project ... in Blender, Unreal Engine or Unity") | yes CC0-MIT | Grime, dirt, cracks, graffiti tags, stains, bullet impacts as deferred decals | 280+ decals with albedo, normal, roughness, opacity. The single best new decal source in this sweep. Page returned 403 to plain fetch, so confirm the download route (site listing vs a bundled zip) with a browser. No login mentioned. |
| karlwirbelwind "CC0 Decal - Graffiti Textures" | https://sketchfab.com/3d-models/cco-decal-graffiti-textures-4af449f78bec4c55807350773ceb5e5b and the sibling https://sketchfab.com/3d-models/cco-decal-graffiti-textures-dcecb765425c4f1e9104b420ad2349de | CC0 1.0 Universal, stated in the model title and description | attribution not required, but Sketchfab download is account-gated | Generic graffiti tags for the derelict dressing | 2048x2048 albedo plus opacity. Generic spray tags, no real-world logos in the previews, but check each tag reads as invented. Account needed to pull the file. |
| ExileGL "Blood Splatter" (OpenGameArt) | https://opengameart.org/content/blood-splatter | CC0 1.0, stated | yes CC0-MIT | Floor and wall blood decal material, and as a Niagara sprite | Single high-res PNG splatter, 2.6 MB. Direct: `https://opengameart.org/sites/default/files/blood_0.png`. No login. |
| AntumDeluge "Blood Splatters" (OpenGameArt) | https://opengameart.org/content/blood-splatters | CC0 1.0 | yes CC0-MIT | Smaller blood hit decals, variety | Several small splatters. Lower resolution, better as hit marks than a pooled decal. |
| "Bloodsplatter and Bloodsplash Animation" (overcrafted, OpenGameArt) | https://opengameart.org/content/bloodsplatter-and-bloodsplash-animation | CC0 1.0, with source files | yes CC0-MIT | Flipbook input for a Niagara blood burst | Animated frames plus source. Feeds a sprite-sheet Niagara emitter, which is the cheap route to the blood-burst gap. |
| ambientCG Decal category | https://ambientcg.com/list?category=Decals | CC0 1.0 | yes CC0-MIT | Surface imperfections, fingerprints, scratches, leaking, smear | Named sub-tags seen: Surface Imperfections, Fingerprints, Scratches, Leaking, Smear. These are wear overlays rather than graphics. Pull the leak and smear decals for the vertical water-streak look without doing it procedurally. Keyless API as above. |

### 1.4 More CC0 PBR surfaces the earlier list did not name

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| cc0-textures.com by category | https://cc0-textures.com/c/brick , /c/metal , /c/asphalt , https://cc0-textures.com/tags | CC0, provenance mixed (see 1.2 note) | yes CC0-MIT, verify per file | Glazed brick, stainless steel, wet asphalt, terrazzo variants | Fills gaps the ambientCG list flagged as unanswered (oxblood tile aside). Check the per-asset source attribution line. |
| ProcTexture | https://proctexture.com/ | CC0, "Free CC0 Procedural PBR Texture Generator Online" | yes CC0-MIT | Custom tiling surfaces generated to spec: painted steel with chips, worn rubber, dirty glass, puddle masks | Generates rather than hosts, so you author exactly the roughness contrast a wet-floor mask needs. Output licence is CC0. Browser tool, no login stated. |
| 3dtextures.me CC0 tag | https://3dtextures.me/tag/cc0/ | CC0, https://3dtextures.me/about/ | yes CC0-MIT | Stylised and sci-fi tile, trim, metal panels | Already noted as a source in the Phase F list but not mined. The CC0 tag page is the fast way in. Per-map buttons, download by hand. |
| Materialize (open source, photo to PBR) | https://boundingboxsoftware.com/materialize/ | MIT | yes CC0-MIT (the tool) | Converting reference photos of London tile, plaster and concrete into tiling PBR | The free stand-in for Substance Sampler that the commissioned guide names. The tool is MIT; output is your own work. macOS build availability needs checking, it is primarily a Windows release. |

---

## 2. Zombies / characters

### 2.1 CC0 / permissive rigged humanoid meshes

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| Quaternius Animated Zombie Pack | https://quaternius.com/packs/animatedzombie.html | CC0, stated on page | yes CC0-MIT | A ready zombie mesh with a bundled animation set for F6 | Atlas-textured zombie, FBX / OBJ / Blend. This is a dedicated zombie, not the Universal Base Characters already staged. Small (2018 vintage, single mesh plus variants). Good as the first horde body while better meshes are sourced. Download button on the Quaternius site, historically routes to a direct zip or itch. |
| Blender Studio Human Base Meshes | https://studio.blender.org/tools/models/human-base-meshes | CC0 1.0, stated by Blender Studio | yes CC0-MIT | Clean base bodies to sculpt or reskin into varied civilians-turned-zombies | 17 meshes: full male and female figures plus hands, feet, heads, jaws, eyeballs. Not game-rigged out of the box, they are modelling bases. Pair with AccuRIG or Blender Rigify, then retarget Quaternius or KayKit animations. Page 403s to plain fetch, download via the Blender Studio site (may want a free Blender ID). |
| OpenGameArt "3d Humanoids under CC0" | https://opengameart.org/content/3d-humanoids-under-cc0 | CC0 1.0 | yes CC0-MIT | Low-poly rigged crowd filler for distant waves | Several low-poly rigged humans in one submission. Fidelity is low, use for far LODs or a dense back rank. Direct download from OpenGameArt, no login. |
| Generic Male Basemesh rigged (Blend Swap 8395) | https://blendswap.com/blend/8395 | CC0, per the blend page (confirm) | yes CC0-MIT if page confirms | A single rigged male base for a hero or boss zombie | Blend Swap account required. Rigged, so closer to usable than the Blender Studio bases. |
| KayKit Character Pack Skeletons 1.0 | https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Skeletons-1.0 | CC0 1.0 Universal, in `LICENSE.txt` (verified) | yes CC0-MIT | A stylised skeleton enemy if the roster ever wants one, or animation donor | GitHub mirror, direct download, about 20 MB, ships the Godot addon form (glTF inside `addons/`). 90+ animations on the rig. Highly stylised, off the reference frame: proportion, prototype, or animation-donor use. The fuller FBX plus GLTF pack is the itch download (name-your-price). |
| KayKit Character Pack Adventurers 1.0 | https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0 | CC0 1.0 Universal, in `LICENSE.txt` | yes CC0-MIT | Stylised civilian stand-ins for a menu diorama or prototype crowd | GitHub mirror, direct download, about 24 MB. Same stylisation caveat. |
| MB-Lab (Blender character generator) | https://mb-lab-community.github.io/MB-Lab.github.io/ | GPL-3.0 (the add-on); generated meshes are yours to licence | yes (the tool) | Generating varied human bodies, ages, builds, with face and finger rigs, for civilian variety | Open-source successor to Manuel Bastioni Lab. Output is anatomically detailed and rigged. Heavier meshes, decimate for a horde. GPL covers the add-on code, not your generated characters. |

### 2.2 More CC0 / permissive animation sets

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| KayKit Character Animations | https://kaylousberg.itch.io/kaykit-character-animations and mirror https://opengameart.org/content/kaykit-character-animations | CC0 1.0, stated | yes CC0-MIT | Generic humanoid walk, idle, run, attack, hit, death for civilians and melee zombies | FBX and glTF, retargets to the UE5 mannequin via IK Retargeter. itch is name-your-price; the OpenGameArt mirror is a direct download with no account. Stylised timing, may want scaling. |
| KayKit Skeletons rig animations (90+) | https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Skeletons-1.0 | CC0 1.0 Universal | yes CC0-MIT | Attack, walk, react, get-up clips as a donor set retargeted onto a realistic zombie | Direct GitHub download. The clips are the value even if the mesh is not used. |
| Rokoko Motion Library (free clips) | https://www.rokoko.com/motion-library | Free clips permit commercial use per Rokoko's terms; the raw file may not be redistributed | ship compiled, do not commit raw | Extra locomotion, stagger, fall, get-up for zombies and civilians | Browsed and exported through the free Rokoko Studio app, so it is account-gated and not a clean direct download. Treat like Mixamo: use the motion, do not re-host the file. Re-audit before any public release. |
| CMU Graphics Lab Motion Capture Database | http://mocap.cs.cmu.edu/ | Free for all uses including commercial; no resale of the raw data | ship compiled, raw data not committed | Bulk locomotion, everyday movement, staggers, falls for retargeting | 2,500+ motions, ASF/AMC and BVH/FBX conversions exist. Needs cleanup and retargeting. Public university release, genuinely free, but the raw database is not something to commit wholesale. |
| Kenney Animated Characters | https://kenney.nl/assets/animated-characters-3 (and -1, -2) | CC0 1.0 | yes CC0-MIT | Rigged low-poly characters with basic clips, for a prototype crowd | CC0, direct download, no account. Very stylised, prototype only. |
| CC0 gore meshes: none found | n/a | n/a | n/a | severed limbs, ragdoll body parts | No genuinely CC0 dismemberment mesh set was found. See Gaps. The nearest is slicing a CC0 base mesh (2.1) into parts yourself. |

---

## 3. Audio / VFX

### 3.1 Free Niagara blood / gore VFX for UE5, the real gap

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| Epic Niagara Examples Pack (UE 5.7) | https://www.fab.com/ , search "Niagara Examples", announced at unrealengine.com/news/discover-over-50-free-niagara-systems-ready-to-use-in-unreal-engine-5-7 | Fab / UE-only licence | UE-only, do not commit raw | Muzzle flash, bullet impact, dust, smoke, spark, debris systems for the shooter | 50+ ready Niagara systems, Epic-authored, free on Fab. No blood system, but it closes the muzzle-flash, impact, dust, smoke and spark needs cleanly. Ship compiled, do not commit the assets. |
| DIY sprite-sheet blood emitter | inputs: overcrafted flipbook https://opengameart.org/content/bloodsplatter-and-bloodsplash-animation , ExileGL splatter https://opengameart.org/content/blood-splatter | CC0 1.0 on the inputs | yes CC0-MIT (the textures) | The blood-burst and blood-pool effect, built in-project | The pragmatic answer to the gap: a small Niagara system spawning CC0 blood sprites on hit, plus a CC0 blood decal on the surface. No third-party blood pack has a free permissive licence. |
| GitHub search result: essentially nothing | searched niagara+blood, ue5+blood+decal, unreal+gore, niagara+vfx+pack | n/a | n/a | n/a | `DuskMoment/UnrealProcGore` and `andres00567/GoreSystem` exist but are zero-star, unlicensed, unverified. Not usable. The GitHub open-source blood-system route is a dead end as of this sweep. |
| CodeLikeMe Niagara Blood VFX Pack | circulated free, origin CodeLikeMe / Patreon | Not a clear permissive licence; "circulated as free" is not a licence | do not commit, treat as unverified | blood burst and decal | Already known and already flagged as quality-variable. Re-listed only to confirm: its licence status is not clean enough to commit, and this sweep found nothing better with a real licence. |

### 3.2 CC0 / permissive SFX beyond Kenney

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| Sonniss GDC Game Audio Bundle 2026 | https://gdc.sonniss.com/ , mirror listing https://gamesounds.xyz/?dir=Sonniss.com | Sonniss GDC bundle licence: royalty free, commercial use, no attribution, worldwide, perpetual. Not CC0. Explicitly forbids AI/ML training use. | ship compiled, treat as attribution-free but bespoke; commit only if the owner accepts a non-CC0 permissive licence | Footsteps, machinery, impacts, UI, creature beds, ambiences | 7.47 GB, 347 WAV files for 2026. The wider archive across nine years is 200 GB+ at https://sonniss.com/gameaudiogdc/ . GameSounds.xyz mirrors older bundles as a plain directory listing with direct file links, no account. The single biggest free audio library available. Flagged for the owner: it is not CC0, but it is safe for a shipped commercial game. |
| Selekt Audio CC0 drone textures | https://selektaudio.com/sound-lab/textures/drone | CC0, "500+ CC0 drone sounds, free for commercial use" | yes CC0-MIT (verify on page) | Station hum, electrical buzz, ventilation drone beds for rounds and menu, all synthetic | 500+ synthetic drone textures. Synthetic by nature, so no field-recording risk. Page returned 429 to fetch, confirm the download route and the no-account claim with a browser. |
| OpenGameArt "Footsteps on different surfaces" | https://opengameart.org/content/footsteps-on-different-surfaces | CC0 1.0 (confirm the block, some OGA posts multi-licence) | yes CC0-MIT | Player and zombie footsteps on concrete, tile, metal | Sourced from Freesound CC0 uploads, collated. Direct download, no account. |
| Breviceps "Zombie gargles" (Freesound) | https://freesound.org/people/Breviceps/sounds/445983/ | CC0 1.0, Breviceps publishes CC0 consistently | yes CC0-MIT | Zombie vocalisations: growl, hiss, moan, snarl | Confirm the licence block on the page. Freesound CC0 download needs a free account. Breviceps is a known reliable CC0 uploader, low provenance risk. Not a real transit recording, it is a voice-and-processing creature sound. |
| OpenGameArt "80 CC0 creature SFX" | https://opengameart.org/content/80-cc0-creature-sfx | CC0 1.0 | yes CC0-MIT | Monster and creature growls, screeches for the screamer and brute | 80 sounds, 7 tagged monster. Synthetic and processed creature sounds, no human transit content. Direct download. |
| OpenGameArt "Gore Blood Gibs Meat Chunks" (Reactorcore) | https://opengameart.org/content/gore-blood-gibs-meat-chunks | CC0 1.0 | yes CC0-MIT | Body-fall and gib impact layer, if a stylised hit is wanted | Pixel-art sprites plus particle sprites, not audio and not 3D. Listed here because it turned up in the gore sweep. Marginal for this project's fidelity. |
| GameSounds.xyz | https://gamesounds.xyz/ | Aggregator: hosts Sonniss bundles (bundle licence), plus sections marked CC0 and public domain. Per-section. | per section | UI clicks, impacts, mechanical, ambience | A directory-listing site with direct file links and no account. Good for grabbing specific one-off sounds fast. Check which sub-directory a file came from for its licence. |

### 3.3 Free CC0 / permissive music

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| MundoSound "Dark Ambient Loop" series (OpenGameArt) | https://opengameart.org/content/dark-ambient-loop-13 and the numbered siblings 1 through 12 | CC-BY 3.0 and OGA-BY 3.0, dual | attribution | Menu bed and low-tension round underscore | 48 kHz 24-bit WAV, about 1:54 each, seamless loops. Direct: `https://opengameart.org/sites/default/files/Dark%20_Atmosphere13_Looped_24bit.wav` and equivalents. Instrumental, no voice. Attribution to Lucas Calvo / mundosound.com required. |
| OpenGameArt "CC0 Music" collection | https://opengameart.org/content/cc0-music-0 | CC0 1.0 (verify each track's block) | yes CC0-MIT | Menu and round music with no attribution string | A collection page; individual tracks vary. Filter for dark or ambient. Some tracks in OGA "CC0" collections carry a second licence block, so confirm per track. |
| OpenGameArt "CC0 Background Ambience" | https://opengameart.org/content/cc0-background-ambience | CC0 1.0 | yes CC0-MIT | Room-tone and ambience beds under the menu and between rounds | Already flagged in the Phase F audio list as a bulk-sweep target, restated with the direct URL. |

### 3.4 Muzzle flash / impact / dust / smoke / sparks

Covered by the Epic Niagara Examples Pack in 3.1 (UE-only, ship compiled). No
CC0 Niagara system set was found. Building these from CC0 sprite textures
(smoke, spark, dust flipbooks on OpenGameArt and ambientCG) plus a hand-built
emitter is the repo-committable route.

---

## 4. Weapons / first-person arms

| Asset / pack | Source URL | Licence (exact) | Repo-committable? | What it feeds | Notes / risks |
|---|---|---|---|---|---|
| drillimpact "PSX First Person Arms" (free) | https://drillimpact.itch.io/psx-first-person-arms-free | CC0, stated ("free for commercial use, no credit required") | yes CC0-MIT | FP arm mesh plus a starter animation set | Rigged arms, 18 animations (relax, push, jab, guard, grab, knife, finger gun and more), bare-hands and black-gloves textures, FBX / GLB / Blend, 512x512, 3.8 MB. No account needed to download. PSX-fidelity, so a cosmetic stopgap, but a genuinely complete CC0 FP-arms package with animations, which is the hard thing to find free. |
| wriks "WRAD ARMS" | https://wriks.itch.io/wrad-arms | CC0 1.0 Universal, stated ("no attribution required, completely free forever") | yes CC0-MIT | FP arm mesh, second option | Rigged IK arms, 2 skin variants, 512x512, 1200 tris, GLB / FBX / OBJ. Half-Life 1 style. No animations bundled, so pair with the drillimpact clips or Quaternius. Name-your-price, itch account to download. |
| OpenGameArt "fps arms (rigged only)" | https://opengameart.org/content/fps-arms-rigged-only | CC0 1.0 (confirm block) | yes CC0-MIT | A third FP-arms base with an IK plus handle-bone rig | .blend and .fbx. Rig set up for quick arm posing. Direct download, no account. No animations. |
| Blend Swap "1st person arms rig" (19157) | https://blendswap.com/blend/19157 | Per-blend, filter CC0, confirm on page | yes CC0-MIT if page confirms | FP-arms rig reference or base | Blend Swap account required to download. |
| KayKit / Quaternius gun meshes | already staged (Quaternius Low Poly Guns) plus https://github.com/KayKit-Game-Assets (no dedicated gun pack) | CC0 | yes CC0-MIT | SMG, pistol, shotgun, rifle cosmetic meshes | Quaternius Ultimate Gun Pack is already in `_incoming_assets/weapons/`. KayKit has no modern-firearm pack. No new CC0 modern-weapon mesh source beyond what is staged was found that beats Quaternius. |
| Sketchfab CC0 weapon meshes | https://sketchfab.com/tags/cc0 crossed with "rifle", "pistol", "smg" | Per model, mixed | attribution or yes, check page | Filling a specific weapon silhouette gap | Account-gated, per-model licence. Only worth it for one specific missing shape. |

---

## Fetch checklist

Everything below is CC0 or clearly permissive AND has a clean direct download
with no login, no captcha, no licence-accept gate. A later run can grab these
fast. Sizes are rough where known. This section downloads nothing.

### Decals and blood textures, to `_incoming_assets/decals/`

- ExileGL blood splatter PNG:
  `https://opengameart.org/sites/default/files/blood_0.png` (2.6 MB, CC0)
- AntumDeluge blood splatters: page
  `https://opengameart.org/content/blood-splatters`, grab the attached zip (CC0,
  small)
- overcrafted bloodsplatter and bloodsplash animation frames plus source: page
  `https://opengameart.org/content/bloodsplatter-and-bloodsplash-animation`
  (CC0)

### Audio, to `_incoming_assets/audio/`

- MundoSound Dark Ambient Loop 13:
  `https://opengameart.org/sites/default/files/Dark%20_Atmosphere13_Looped_24bit.wav`
  (33 MB, CC-BY 3.0 / OGA-BY 3.0, record the credit)
- MundoSound Dark Ambient Loops 1 to 12: from the OpenGameArt user page
  `https://opengameart.org/users/mundosound`, same licence, same credit
- OpenGameArt "Footsteps on different surfaces": attached files on
  `https://opengameart.org/content/footsteps-on-different-surfaces` (CC0,
  confirm the block)
- OpenGameArt "80 CC0 creature SFX": attached zip on
  `https://opengameart.org/content/80-cc0-creature-sfx` (CC0)
- Sonniss GDC bundles via GameSounds.xyz directory listing, e.g.
  `https://gamesounds.xyz/?dir=Sonniss.com%20-%20GDC%202020%20-%20Game%20Audio%20Bundle`
  (large, Sonniss bundle licence not CC0; only if the owner accepts it)

### Characters and animation, to `_incoming_assets/characters/`

- Quaternius Animated Zombie Pack:
  `https://quaternius.com/packs/animatedzombie.html`, follow the download
  button (CC0; verify it lands on a direct zip and not an itch gate)
- KayKit Character Pack Skeletons 1.0 (Godot addon form, glTF plus 90+ anims):
  `https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Skeletons-1.0/archive/refs/heads/main.zip`
  (about 20 MB, CC0)
- KayKit Character Pack Adventurers 1.0:
  `https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0/archive/refs/heads/main.zip`
  (about 24 MB, CC0)
- KayKit City Builder Bits 1.0:
  `https://github.com/KayKit-Game-Assets/KayKit-City-Builder-Bits-1.0/archive/refs/heads/main.zip`
  (about 5 MB, CC0)
- KayKit Prototype Bits 1.0:
  `https://github.com/KayKit-Game-Assets/KayKit-Prototype-Bits-1.0/archive/refs/heads/main.zip`
  (about 5.5 MB, CC0)
- KayKit Character Animations via OpenGameArt mirror:
  `https://opengameart.org/content/kaykit-character-animations`, attached files
  (CC0)
- OpenGameArt "3d Humanoids under CC0": attached files on
  `https://opengameart.org/content/3d-humanoids-under-cc0` (CC0)

### Weapons / FP arms, to `_incoming_assets/weapons/`

- drillimpact PSX First Person Arms (free): from
  `https://drillimpact.itch.io/psx-first-person-arms-free`, the free download
  needs no account (3.8 MB, CC0, ships 18 animations)
- OpenGameArt "fps arms (rigged only)": attached .blend and .fbx on
  `https://opengameart.org/content/fps-arms-rigged-only` (CC0, confirm block)

### Environment, to `_incoming_assets/props/` or `_incoming_assets/surfaces/`

- Poly Haven Security Camera 01, 02 and Metal Trash Can via the keyless API:
  `https://api.polyhaven.com/files/security_camera_01` ,
  `https://api.polyhaven.com/files/security_camera_02` ,
  `https://api.polyhaven.com/files/metal_trash_can` , then pull the glTF or FBX
  plus 2K textures the JSON points to (CC0, no login)
- ambientCG Decal and Atlas assets via the keyless API, e.g.
  `https://ambientcg.com/api/v2/full_json?id=<AssetID>&type=Decal` then the zip
  URL it returns (CC0). Pick leak, smear and surface-imperfection decals.

### Needs a browser or account before it can be fetched (not in the fast list)

- 3DTexel decal library (page 403s to fetch; confirm the download route)
- Selekt Audio CC0 drone textures (page 429; confirm no-account claim)
- Blender Studio Human Base Meshes (page 403; likely wants a Blender ID)
- Blend Swap items 19305, 8395, 19157 (free account required)
- Sketchfab items: karlwirbelwind graffiti decals, the two turnstiles, any CC0
  weapon mesh (account required, per-model licence check)
- wriks WRAD ARMS, CC0 Metro Station Prototype (itch, name-your-price, account)

---

## Skipped / rejected

- **Kaiju Station** (`https://3djon.itch.io/kaiju-station`): ships as a Windows
  `.exe` only, no source assets, no stated licence. Mood and reference only, not
  an asset source. The commissioned guide already over-credits it.
- **PSX Subway Station Asset Pack** (`https://ink-ribbon.itch.io/psx-subway-station-asset-pack`):
  paid, 4.99 USD minimum. Not free. 14 metro asset types including turnstile and
  train, which is a shame, but it is behind a paywall.
- **Monster Vocalization Sound Pack** (`https://morphonaut.itch.io/monster-vocalization-sound-pack`):
  2.00 USD minimum. Paid despite the CC0 tag on the contents. Excluded on the
  no-purchase rule.
- **Vefects Blood VFX for Unreal** (`https://vefects.itch.io/blood-vfx-unreal-engine`):
  57.99 USD. Paid.
- **Hivemind Realistic / Stylised Niagara Blood** (Fab): paid, about 40 USD
  each. The commissioned guide's recommended purchase, not a free asset.
- **Meshy.ai** CC0 zombie and train models: AI-generated, account-gated, and the
  provenance of a generated mesh is not a licence position to ship on. Same
  verdict as the earlier sweeps.
- **CGTrader / TurboSquid "free" rigged zombies** (Aiden Studios, "Zombie Rigged
  4K PBR"): free-to-download but under each site's bespoke royalty-free EULA, not
  CC0 or CC-BY, and each licence needs reading. Not clean enough to commit.
- **Railway Sans, P22 Underground, "London Tube" fonts**: Johnston lookalikes.
  The diamond tittle is the reject test per `rejected-assets.md`. Hard no, same
  as Paddington Plain.
- **BBC Sound Effects / RemArc**: personal, educational and research use only.
  Out for a project that may ship. Also full of real transit recordings.
- **Pixabay audio**: no longer CC0, now the Pixabay Content License. Fails a
  strict CC0 test.
- **Freesound files tagged CC0 as "tube" or "metro" chimes / announcements**:
  the project has already been bitten by mislabelled real operator recordings.
  Any file whose title or tags mention announcement, chime, PA, a station name,
  a line name or an operator is out regardless of the licence label.
- **karlwirbelwind CC0 graffiti decals and 3DTexel graffiti decals**: usable,
  but every individual tag must be eyeballed to confirm it is invented and
  carries no real-world logo, gang mark or operator reference before it goes in.

---

## Gaps that remain

The free world still does not provide these, so a small spend or in-house work
is unavoidable:

- **A believable modular main-line station kit.** Unchanged from every prior
  sweep. No CC0 shell, platform edge, escalator, gate line or tunnel portal
  exists. The blockout aids found here (CC0 Metro Station Prototype, KayKit
  Prototype Bits) are grey-box only. This is in-house modelling or a paid pack
  (Roy Sousa "Abandoned Underground" at about 55 USD is the on-theme option).
- **A free Niagara blood / gore system with a clean permissive licence.** The
  GitHub route is empty, every polished pack is paid or unlicensed, and the
  Epic Niagara Examples Pack has no blood. The realistic answer is a hand-built
  Niagara system driven by the CC0 blood sprite and decal textures in this
  sweep. Dismemberment specifically has no free answer at all.
- **CC0 gore meshes** (severed limbs, ragdoll body parts). None found. Cut them
  from a CC0 base mesh in Blender, or buy.
- **A high-fidelity CC0 rigged zombie or civilian mesh that matches the
  reference frame.** Quaternius Animated Zombie and the KayKit characters are
  stylised and low-fidelity. Blender Studio Human Base Meshes are unrigged
  modelling bases. City Sample Crowds and MetaHuman remain the quality route and
  remain UE-only-licence (ship compiled, cannot commit). A genuinely
  photoreal CC0 rigged human does not exist free.
- **The Elizabeth line hero train (Class 345 Aventra).** No free UE-ready model.
  Unchanged. Kitbash the de-branded S Stock already committed, or model from the
  dimension table, or commission.
- **First-person weapon arms at shipping fidelity.** The two CC0 packs found
  (drillimpact PSX, WRAD ARMS) are retro-fidelity stopgaps. A polished modern FP
  arms plus animation set is a cheap Fab purchase, not a free asset.
- **British-accent platform announcement voice.** Not researched further here
  (the commissioned guide covers MeloTTS and other local TTS). Note the legal
  rule stands: no transcribed real announcements, and synthesised lines must
  avoid the operator's actual wording and house phrases.

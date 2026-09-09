# Free asset sourcing guide, external research

External research commissioned by the project owner and handed to the repository
on 2026-09-09. Reproduced here with the house style applied to dashes and
spelling, and otherwise unedited.

## How this sits against the other two asset documents

Three documents now cover free assets. They do not say the same thing and none
supersedes another.

| Document | What it is | Authority |
|---|---|---|
| `free-assets.md` | The internal acquisition list, licence checked against the repository's own two filters | Authoritative on what may be committed and what must be gitignored |
| `asset-sources-phase-f.md` | The Phase F sweep, per task, with download URL patterns for the fetch script | Authoritative on Phase F scope |
| This file | Outside research, strong on named specific items the other two miss | Leads, not verdicts |

What this one adds that the others do not: the Sketchfab train fleet by name,
the itch.io first-person arms packs, the open source British voice models for
platform announcements, and the current state of Mixamo and Megascans.

## Four cautions before acting on any of it

1. **It targets UE 5.4. This project is on 5.8.** Engine specific claims,
   particularly about the Interchange importers and Nanite behaviour, need
   rechecking.
2. **Its central recommendation, the S Stock train, is already done.** The
   carriage is committed at `SourceArt/ThirdParty/SStock/`, de-branded, and the
   measurements taken from the actual file are in the README beside it. The
   585k triangle figure quoted below is close: the file holds 579,544.
3. **It does not apply this project's trademark filter.** It suggests
   Johnston lookalike fonts and treats operator branding as a licensing
   question. For this project it is not a licensing question, it is a hard no.
   See `tfl-dimensional-reference.md` and `branding-precedent.md`. The one
   font suggestion below that survives the filter is Hammersmith One under the
   SIL Open Font Licence, and even that wants checking against the mark before
   it is adopted.
4. **It does not apply the repository filter either.** Most of what it lists is
   UE-Only Content or Fab Standard, which may be shipped compiled into the game
   but never re hosted here as raw files. `SourceArt/README.md` sets out what
   may and may not be committed.

Everything below this line is the research as received.

---

# Free Asset Shopping List for "London Underground: The Last Train" (UE5.4, macOS)

## TL;DR
- You can greybox the entire game today with genuinely free assets: timblewee's CC-BY London Underground S Stock train (Sketchfab), Dekogon's permanently-free "City Subway Train Modular" station kit (Fab), Mixamo zombies and animations, Epic's own free FPS/Lyra content, Poly Haven and ambientCG CC0 materials and HDRIs, and the Sonniss GDC audio bundles.
- The three genuinely weak spots for free content are: (1) a purpose-built, believable London/metro STATION environment kit, (2) polished first-person WEAPON arms with reload/ADS/inspect animations, and (3) good Niagara BLOOD/gore VFX. A total spend under about £50 on one or two Fab packs closes all three.
- macOS/UE5.4 caveat: everything here imports as FBX/glTF/OBJ or as native Fab/UE content. Watch total triangle budgets on the high-poly Sketchfab trains (585k tris on the S Stock), avoid anything shipping Windows-only plugins, and note that Lumen and Nanite run on Apple Silicon via Metal but cost performance.

## Key Findings
- The user's own reference links are portfolio pieces, not asset packs, but two of them lead to purchasable/downloadable assets and one confirms the key free train model.
- The single best free London-specific asset is timblewee's Sketchfab CC-BY train fleet (S Stock, plus 1959 tube stock and a separate single carriage), which is exactly what Alex Cruz used in his Lambeth North UE5 scene.
- Mixamo is still free with a free Adobe account in 2026 and royalty-free for unlimited use, but it is effectively abandoned: no meaningful updates since the 2015 Adobe acquisition, the Fuse character creator was discontinued in 2020, and per Cinevva (July 2026) the service "broke for everyone for days with server 500 errors" starting 16 June 2025, with a support agent reportedly telling users Mixamo "is not supported anymore." Treat CC0 alternatives (Quaternius, AccuRIG) as your resilient backup.
- Quixel Megascans is no longer blanket-free for Unreal users. Per Epic's Unreal Engine forums, "The Megascans library on Fab.com will no longer be free after December 31, 2024," and per CG Channel (Oct 2024) Epic began "charging for most of the content, including to use it in Unreal Engine projects." Poly Haven and ambientCG are the CC0 workhorses to rely on instead.

## Details

### 0) The four reference links, investigated

**1. "TfL Future Tube" (Behance, UE4).** The Behance page is bot-protected and could not be fetched directly; from search it is a real-time UE4 concept visualisation of a futuristic TfL tube interior. Treat it as visual/mood reference only. No downloadable asset pack or credited free-asset list could be confirmed. Technique takeaway: a real-time UE4 tube interior with clean modern surfaces.

**2. Alex Cruz, "London Underground Station - Unreal Engine 5" (ArtStation, zPNWDQ).** A recreation of Lambeth North station in UE5. Cruz modelled and textured everything himself except the train. His documented workflow: sculpting in ZBrush, retopology in Blender (some sculpts kept raw to exploit Nanite), photographs of plaster/floor/signs/concrete converted to materials in Substance Sampler, texturing finished in Substance Painter, assembled in Unreal with decals; about one week's work. The one credited downloadable asset is the train: **"Bombardier S Stock London Underground" by timblewee, CC-BY 4.0, https://skfb.ly/ossIt**: VERIFIED still live on Sketchfab.
- Full URL: https://sketchfab.com/3d-models/bombardier-s-stock-london-underground-a6718eff2dc843c48fd54376b4c70b06
- Format: downloadable (Sketchfab serves the source plus auto-converted glTF/FBX/OBJ/USDZ). Modelled in Blender.
- Poly count: 585k triangles / 314.6k vertices. Rigged doors included. Creator flags it "NOT PRESENTED AS GAME READY." [sketchfab](https://sketchfab.com/3d-models/bombardier-s-stock-london-underground-a6718eff2dc843c48fd54376b4c70b06)
- UE5.4 import: imports cleanly via FBX or glTF, but 585k tris is heavy for a full train; enable Nanite on it or decimate. No guaranteed game-ready PBR texture set, so expect to re-author materials. Attribution required (CC-BY).

**3. Roy Sousa, "Abandoned Underground station UE5" (ArtStation, Gew1Ga).** A London-Underground-inspired post-apocalyptic environment. Description: "Growing up in London I always wanted to create a small environment of the London underground in a more 'post-apocalyptic' setting... Custom material parameters let users adjust the look and feel of the assets." [artstation](https://www.artstation.com/artwork/Gew1Ga) Directly on-theme for a zombie mode. It is SOLD, not free: available on Fab as "Abandoned Underground" by Roy Sousa (https://www.fab.com/listings/74555c2e-c165-45ad-a784-7d77ae8afab9), priced $54.99 to $85.99 across licence tiers, 65+ meshes, Lumen-ready for UE5.1+, two demo levels, drag-and-drop blueprint actors. [Fab](https://www.fab.com/ja/listings/74555c2e-c165-45ad-a784-7d77ae8afab9)

**4. Alan Aldred, "Sci-Fi Underground Station Environment" (ArtStation, zAGkQ6).** A UE4 marketplace environment. His page links the Unreal Engine Marketplace listing (slug "underground-station-pack"), [ArtStation](https://www.artstation.com/artwork/zAGkQ6) [artstation](https://alanaldred.artstation.com/projects/zAGkQ6) migrated to Fab as "Modular Underground Station Environment Asset Pack" under Ginger Beard Gaming Studios (https://www.fab.com/listings/86efd362-b4d4-433b-a414-0f5be91000b5). Sci-fi styled modular station and corridor pieces, 4K channel-packed textures, tested with the UE4 mannequin. Price not machine-verifiable from Fab's masked HTML.

### A) London Underground / metro specific

**Rolling stock (trains):**
- **timblewee: Bombardier S Stock London Underground** (Sketchfab, CC-BY 4.0). The hero train. 585k tris, rigged doors, Blender-made. https://sketchfab.com/3d-models/bombardier-s-stock-london-underground-a6718eff2dc843c48fd54376b4c70b06 . UE5.4: FBX/glTF import, heavy, use Nanite or decimate.
- **timblewee: Bombardier S Train Carriage** (Sketchfab, CC-BY 4.0). 381.8k tris, rigged doors, [Sketchfab](https://sketchfab.com/3d-models/bombardier-s-train-carriage-london-underground-09c298622ed74c46bd85d6969396943f) a single carriage that is easier to loop into a train. https://sketchfab.com/3d-models/bombardier-s-train-carriage-london-underground-09c298622ed74c46bd85d6969396943f
- **timblewee: 1959 Tube Carriage / 1959 Tube Driver Car** (Sketchfab, CC-BY 4.0). Deep-tube-profile stock (round tunnel silhouette) if you want the classic Underground look rather than the sub-surface S Stock. Roughly 318 to 345k tris each.
- **Samuel Banks: London Underground 2009 Stock** (Sketchfab). Victoria line deep-tube stock built from blueprints with a modular platform. Check the specific licence and downloadable flag on its page before use.
- **Class 345 / Elizabeth line (Aventra):** No clean free UE-ready model was found. The best free geometry is a Cities: Skylines Steam Workshop model ("Class 345 - Elizabeth Line 9-car" and 4-car by the #PurpleTrain modder), which is extremely low-poly (about 3,176 to 4,216 tris per car, 2048x1024 textures) and licensed for "private use" and Cities: Skylines only, extractable via ModTools. Usable as a private hobby stand-in but low fidelity and not a clean FBX. Realistically, for the Elizabeth line hero train you will either kitbash timblewee's S Stock into a purple Aventra livery or model it yourself.

**Modular station environment kits:**
- **Dekogon: City Subway Train Modular** (Fab, FREE, permanent). Originally March 2019 Epic Sponsored Content, so permanently free. 110+ meshes, master materials, 30+ signs/stickers/graffiti decals, LUTs, blueprint light setup. New York styled, but the tiled walls, rubber floors and train read well as generic metro. https://www.fab.com/listings/39abdcd9-baa1-49a0-bb9b-90808d483bca . Rated 4.7 (187 ratings). UE5 compatible.
- **Kaiju Station** (itch.io, 3djon, free download). A full modular subway UE5 environment (Lumen + Nanite, no RTX hardware required) given away as a playable demo/project. A strong greybox base. https://3djon.itch.io/kaiju-station
- **Paid packs worth knowing (for the station gap):**
  - Leartes Studios "Subway Station Environment," 116 modular meshes, artist Jonjo Hemmens, [Gumroad](https://leartesstudios.gumroad.com/l/tjgpp) UE4/UE5. [gumroad](https://leartesstudios.gumroad.com/l/tjgpp) $62.99 on Gumroad (https://leartesstudios.gumroad.com/l/tjgpp), around $69.99 on Fab/ArtStation (the price differs across the seller's own channels). Also Leartes "Abandoned Subway Station in Berlin" (285 meshes, [Fab](https://www.fab.com/listings/487ff0d8-7244-4f1f-9a12-0d61636533ed?lang=en) about $99.99) and "The Blue Metro 2029."
  - "Underground Metro Station Environment" (Fab): complete ready-to-open UE5 project, 272 meshes, 563 lights, Nanite on all meshes, ships as UE project + .blend + GLB. [Fab](https://www.fab.com/listings/e1a16a1d-0bf8-40eb-953b-6adf9a383a7d) It targets UE5.8 and requires DX12/Shader Model 6, so on macOS import the GLB/blend rather than open the DX12 project. https://www.fab.com/listings/e1a16a1d-0bf8-40eb-953b-6adf9a383a7d
  - "Modular Subway Station & Train" (Fab, NY style, 44 meshes, full subway car blueprint, puddle decals, dynamic tile weathering). [Fab](https://www.fab.com/listings/f3172c32-7761-4e55-96e7-ce7a299e3c11) Price not verified.
  - Roy Sousa "Abandoned Underground" (see reference #3) is the most on-theme paid option at $54.99+.
  - No PurePolygons, Nuare Studio or Aesir Interactive subway/station pack exists on Fab (verified). PurePolygons (now Sierra Division) publishes city/nature kits, several free; Nuare and Aesir are game studios, not station-asset sellers.

**Individual station props:** Best sourced from the Dekogon free pack (benches, bins, signage frames, decals) plus Sketchfab CC-BY/CC0 searches for CCTV cameras, ticket barriers, vending machines and help points. Fab's free rotation and Sketchfab's "downloadable + CC" filter are the two places to trawl for the remaining props (dot-matrix displays, tactile paving, fire-equipment cabinets, roundel signage frames).

**TfL typography and roundel:**
- **Hammersmith One** (Google Fonts, OFL, free commercial): Johnston-tradition humanist sans, [Google Fonts](https://fonts.google.com/specimen/Hammersmith%2BOne) the closest free lookalike. https://fonts.google.com/specimen/Hammersmith+One
- **Railway Sans** (Greg Fleming, free): a deliberate Johnston/Underground lookalike used on signage. [Fontswan](https://fontswan.com/london-underground-font/)
- **P22 Underground / "London Tube" fonts** (FontSpace, 1001Fonts, cufonfonts): Johnston-derived. Note the original Johnston lettering entered the public domain on Public Domain Day 2015 (Johnston died 1944), but "New Johnston" and Johnston100 remain TfL property. For a private hobby project the lookalikes are freely usable; check each font's own licence.
- Roundel vectors: the shape is trivial to rebuild from the TfL Basic Elements standard (below); avoid hunting for "official" vector rips.

**Real TfL reference material (open, downloadable PDFs):**
- **TfL Product design standards**: https://content.tfl.gov.uk/tfl-standard-for-tfl-products.pdf (roundels, LED matrix signs described as a "close replica of New Johnston," bench/ceiling/frame construction detail).
- **London Underground Signs manual**: https://foi.tfl.gov.uk/FOI-0092-2526/lu-signs-manual.pdf
- **TfL Basic Elements standard (Issue 8)**: https://content.tfl.gov.uk/tfl-basic-elements-standards-issue-08.pdf (roundel geometry, colour palette).
- **Supplementary signs standard**: https://content.tfl.gov.uk/tfl-supplementary-signs-standard.pdf (CCTV signs with exact mm dimensions, [Transport for London](https://tfl.gov.uk/cdn/static/cms/documents/tfl-supplementary-signs-standard.pdf) platform-barrier signage).
- **Interchange / DLR / London River Services signs standards**: additional roundel and wayfinding geometry.
- These are covered by TfL's transport data terms; [Transport for London](https://foi.tfl.gov.uk/FOI-0092-2526/lu-signs-manual.pdf) fine for building accurate geometry on a private project. Cite as metadata, not gatekeeping.

### B) Zombies and characters
- **Mixamo** (Adobe account, free, royalty-free for unlimited use). Still the fastest route: several zombie-appropriate characters and a full zombie locomotion/attack/reaction set. In 2026 it is effectively abandoned (see Key Findings) but functional. Download FBX, retarget to UE5 Manny/Quinn via IK Rig / IK Retargeter in 5.4.
- **Quaternius Universal Animation Library** (CC0). UAL 1 is 120+ animations (the free "Standard" tier is 45 clips) and UAL 2 is 130+ animations; the zombie locomotion set lives in UAL 2, which "complements the first library, covering... zombie locomotion." Both CC0, so nothing follows the file into your game. The resilient backup to Mixamo. quaternius.com
- **CC0/CC-BY rigged zombies:** Aiden Studios "Zombie (Rigged & Animated)" on Sketchfab (idle/walk/attack plus mouth/eye blendshapes, under 10k verts); CGTrader "Zombie Rigged - 4K PBR UE4 UE5" (free, FBX/OBJ/Maya/Max, about 44k tris, 4K PBR maps). TurboSquid's free "rigged zombie" section has several more.
- **City Sample Crowds** (Fab, free, Epic). MetaHuman-derived rigged crowd characters (6 bodies, 12 heads, hair grooms, clothing). Licensed for use only in Unreal Engine products (fine here). [CG Channel](https://www.cgchannel.com/2022/04/download-epic-games-free-city-sample-assets-for-ue5/) Repurpose as civilians-turned-zombies via material swaps and Mixamo/retargeted zombie animations. Known gotcha: groom/hair "Soft Object Reference" errors when migrating across engine versions; [Unreal Engine](https://forums.unrealengine.com/t/city-sample-crowds-for-ue-5-1/882197) strip grooms if they break.
- **MetaHuman for zombies:** Viable in 5.4, free. Workflow: build a MetaHuman, drive it with retargeted zombie animation, swap in a decayed/gore skin material. The catch is performance: MetaHumans are expensive; use lower LODs and limit on-screen count. Best for hero/boss zombies, not a full horde wave.

**Zombie animation sets:** Mixamo zombie pack (walk, run, shamble, crawl, attack, stagger, hit reactions, deaths); Quaternius UAL 2 zombie clips (CC0); Truebones offers a free Mixamo-derived 2,400-clip download in BVH/FBX (Gumroad, name-your-price); CMU mocap and AMASS are open research libraries for extra locomotion. Retarget all via IK Retargeter in 5.4.

### C) Weapons
- **Low Poly Shooter Pack - Free Sample** (Fab/UE Marketplace, free). Includes first-person arms, multiple weapons each with reload/inspect/fire animations, muzzle flashes, HUD, ready to play. Vault-only (install via the Epic Games Launcher). The single best free "first-person arms plus animations" starting point. https://www.unrealengine.com/marketplace/en-US/product/low-poly-shooter-pack-free-sample
- **Epic "Shooter Game" and "Lyra Starter Game"** (free, Epic). Lyra is the modern reference FPS/TPS sample with full weapon and animation systems; cannibalise its input, weapon and animation framework. Heavier to learn but production-grade.
- **Infima Games "Free FPS Template & Tutorial"** (Fab, free). Basic FPS mechanics, procedural animations, a UE5 Mannequin rig and Blender source files; built for their YouTube series. https://www.fab.com/listings/6a0af880-2b74-480c-a82c-8e597918dffe
- **Kuptchi "PSX-Weapons Assets"** (itch.io, name-your-price, free, no credit required, commercial-suited licence). First-person arms with 80+ animations, 4 fully rigged weapons, FBX + .blend + PNG textures + icons. Low-poly/retro but complete. https://kuptchi.itch.io/f
- **bintoon "Free MW2 MP5 reload/inspect animations"** (itch.io, name-your-price). 111 character + 111 MP5 animations, pre-configured for the UE5 First-Person Mannequin, UE5.4.4+, sockets/camera set up. Licence is educational/non-commercial (modify before any commercial use). https://bintoon.itch.io/free-mw2-mp5-reload-inspect-animations-ue5-mannequin-ready
- **British/UK firearms:** No dedicated free UK-firearm pack surfaced. Generic modern SMGs/pistols/shotguns in the packs above cover the fictional zombie-mode arsenal. For a UK flavour (e.g. L85A2), model it or buy. The first-person ARMS + animations are the hard part to source free, which is why the Low Poly Shooter Pack and Kuptchi pack matter most; loose CC0/CC-BY weapon meshes on Sketchfab fill specific gaps.

### D) Materials and textures
- **Poly Haven** (CC0, no login). Concrete, plaster, floor concrete, tiles, metal; also HDRIs and models. The gold-standard CC0 source. https://polyhaven.com/textures
- **ambientCG** (CC0, no login). 2000+ PBR materials/HDRIs; [alternativeto](https://alternativeto.net/software/ambientcg) [ambientCG](https://ambientcg.com/) larger pure-texture catalogue with channel-packed variants. Search "concrete," "tiles," "metal," "rubber," "surface imperfections." https://ambientcg.com
- **3DTextures.me** (CC0) for stylised/sci-fi tiles; **ShareTextures** (check each material's licence, terms vary); **Textures.com** free tier (watch the resolution cap and per-download limits).
- **Quixel Megascans: 2026 status:** NO LONGER blanket-free for Unreal users. Migrated into Fab; assets are individually priced or in Fab subscription tiers, with the free-for-Unreal era ending 31 December 2024. Anything you downloaded before then, and anything you own in your Fab library, stays usable. Megaplants vegetation is currently free [Quixel](https://quixel.com/news/quixel-on-fab-new-megascans-and-megaplants) but irrelevant to a station. Do not build a pipeline assuming free Megascans.
- **Substance Sampler photo-to-material**: exactly what Alex Cruz used: photograph real London tile/plaster/concrete and convert to tiling PBR. Sampler is paid (Adobe). Free alternative: Materialize (open source) for photo-to-PBR.
- Target surfaces to grab: glazed brick, station tile, wet tile, painted metal, stainless steel, glass, rubber flooring, terrazzo, grime/wear overlays, puddle masks. Poly Haven plus ambientCG cover nearly all.

**HDRIs (Poly Haven, CC0):**
- "Cobblestone Street Night" (up to 24K, warm lamp-lit, high-contrast pools of light, deep shadows) for night exterior. https://polyhaven.com/a/cobblestone_street_night
- Poly Haven Night > Urban, [Poly Haven](https://polyhaven.com/hdris/night/urban) Indoor > Night, and Interiors categories for sodium-lit interior and night exterior lighting. https://polyhaven.com/hdris/night/urban

**Decals:** The Dekogon free subway pack ships 30+ signs/stickers/graffiti/grunge decals. [Fab](https://www.fab.com/listings/39abdcd9-baa1-49a0-bb9b-90808d483bca) Supplement with ambientCG "surface imperfections" and blood decals from a Niagara blood pack (below).

### E) Audio
- **Sonniss GDC Game Audio Bundle** (free, royalty-free, no attribution, commercial-cleared). Per gdc.sonniss.com the GDC 2026 bundle is "7.47GB+ of high-quality sound effects," and the community archive holds "over 200GB of royalty-free sound effects" across nine previous years. Covers ambiences, impacts, machinery, UI, creatures. https://gdc.sonniss.com and https://sonniss.com/gameaudiogdc . The licence explicitly states "Use for AI/ML training is strictly prohibited under our licence terms."
- **Freesound** (CC0, CC-BY, CC-BY-NC mixed: filter by licence). Best source for London-specific field recordings: search "London Underground," "tube train," "platform announcement," "escalator." Check each clip's licence.
- **BBC Sound Effects archive** (RemArc Licence). The RemArc release is 16,000 WAV files at 44.1kHz/16-bit (the wider archive is ~33,000 effects). Per the licence they "may be used for personal, educational or research purposes" only, which is PERFECT for this private hobby project but NOT for any monetised or public release. Excellent for authentic British ambience, trains and crowds. http://bbcsfx.acropolis.org.uk
- **Epic's own free audio** (MetaSounds content, and audio inside Lyra/City Sample). City Sample uses MetaSounds for urban ambience [Epic Games](https://dev.epicgames.com/documentation/en-us/unreal-engine/city-sample-project-unreal-engine-demonstration) you can reuse.
- **99Sounds** (royalty-free, commercial and non-commercial) for extra SFX libraries.
- Footsteps on concrete/tile, gunfire, zombie vocalisations and station hums are all available across Sonniss plus Freesound.

**Free/open British-accent TTS for platform announcements:**
- **MeloTTS** (open source, MyShell.ai): supports a British English dialect, runs on CPU, [BentoML](https://www.bentoml.com/blog/exploring-the-world-of-open-source-text-to-speech-models) free. The best fully-local option for "mind the gap"-style announcements.
- **Coqui XTTS-v2 / StyleTTS 2 / Kokoro**: open-source TTS models with UK-voice capability; run locally, free.
- Browser free tiers (ElevenLabs free tier; EasyVoice ships RP voices bf_emma and bm_daniel on its free tier) generate believable RP announcements quickly; watch free-tier character limits and commercial-rights terms.

### F) Gore, VFX and utility
- **Free Niagara blood/gore:** Several "Niagara Blood VFX Pack" listings exist; the widely mirrored **CodeLikeMe Niagara Blood VFX Pack** (16 materials, 25 textures, made in UE Niagara) has circulated as free. The polished commercial options are Hivemind's Realistic and Stylised Niagara Blood (about $39.99 each, with dismemberment/arterial/decal systems): the blood gap is where a small purchase pays off (see Recommendations). Check Fab's free rotation for blood/impact packs.
- **Muzzle flash / impact / dust / smoke:** The Low Poly Shooter Pack and Lyra ship muzzle flashes and impacts; Epic's free Niagara content and community "Niagara Muzzle Flash / Explosions" packs cover the rest. Fab's free VFX rotation frequently includes impact/smoke kits.
- **Epic free learning content / sample projects to cannibalise:**
  - **Lyra Starter Game** (free): full FPS/TPS framework, weapons, animations, input, UI. The best base for round-based shooter mechanics.
  - **City Sample** (free): buildings, vehicles, and crucially City Sample Crowds (MetaHuman-derived rigged crowds) reusable as zombie fodder; the Mass AI crowd system; [Epic Games](https://dev.epicgames.com/documentation/en-us/unreal-engine/city-sample-project-unreal-engine-demonstration) MetaSounds urban audio. UE5.0+; note the 5.8 version is built in PCG. Licensed for Unreal products only. Access via Fab, then Epic Games Launcher > Unreal Engine > Library > Fab Library.
  - **Shooter Game** (free): the classic UE FPS sample.
  - **Valley of the Ancient, Electric Dreams**: free sample projects; more useful for technique/Nanite/PCG reference than direct cannibalisation for a station shooter.
  - Fab's rotating limited-time free content: as of 9 September 2026 the current drop (through 22 September) is Normandy Village + PCG Plants (Sharur), Industrial Infrastructure (Sierra Division) and RPG Crafting & Environment VFX (VRhinoFX). [X](https://x.com/fab/status/2097354576953536905) None is a station pack, but the rotation is worth checking fortnightly. https://www.fab.com/limited-time-free

## Recommendations

**Fastest path to a playable greybox using only free assets (in order):**
1. Start from **Lyra** (or the Low Poly Shooter Pack free sample if Lyra is too heavy). This gives you first-person shooting, arms, weapon animations and input on day one.
2. Drop in the **Dekogon "City Subway Train Modular"** free pack (and/or **Kaiju Station** from itch.io) as your station greybox: tiled walls, platform, rubber floor, the train, decals.
3. Import **timblewee's S Stock** (or the single carriage) as the hero London train; enable Nanite or decimate; retexture with **Poly Haven/ambientCG** CC0 metal/glass.
4. Bring zombies in from **Mixamo** (character plus the zombie animation set) or repurpose **City Sample Crowds**; retarget via **IK Retargeter** in 5.4.
5. Light it with a **Poly Haven** night/interior HDRI plus placed red emergency lights; grade toward late-evening sodium.
6. Layer audio from the **Sonniss** bundle plus **BBC/Freesound** tube recordings; generate platform announcements with **MeloTTS** (British voice).
7. Add blood with a free Niagara blood pack and muzzle flashes from Lyra/Low Poly Shooter Pack.
This is a complete, playable round-based loop with zero spend.

**The 3-4 gaps where a small paid purchase (under about £50 total) saves the most time:**
1. **Station environment kit (biggest visual win).** Free kits are New-York-styled or sci-fi. A single purpose-built pack transforms the look. Best value: **Roy Sousa "Abandoned Underground"** (about $55, and it is literally a London-tube post-apocalyptic set, on-theme for zombies) OR **Leartes "Subway Station Environment"** (116 meshes, $62.99 Gumroad / ~$70 Fab). Pick one, not both. Most of your budget should go here.
2. **First-person weapon arms and animation polish.** If Lyra/Low Poly Shooter Pack animations feel stiff, the itch.io **bintoon MP5** and **Kuptchi PSX** packs are free stopgaps; a cheap Fab FPS animation pack (often about £10 to £15 in sales) closes it. Only buy if the free arms disappoint.
3. **Niagara blood/gore.** Free blood packs are hit-or-miss. **Hivemind Realistic Niagara Blood** (~$40) or **Stylised** (~$40) [Fab](https://www.fab.com/listings/92c5ff1f-7156-4e4f-94b1-cb37a3a478ea) is the reliable purchase; wait for a Fab sale to get one near £20 to £25.
4. **The Elizabeth line hero train specifically.** There is no good free Class 345. If accurate Aventra geometry matters, budget a commission or model it, or accept an S-Stock kitbash. No cheap ready-made purple Aventra for UE5 was found.
Prioritise gap 1 first. If you buy nothing else, buy the station kit.

**Benchmarks that change the plan:**
- If your target zombie wave count exceeds roughly 15 to 20 on screen, drop MetaHuman/City Sample Crowds for a lightweight custom zombie (CGTrader/Sketchfab low-poly rigged) to protect frame rate on Apple Silicon.
- If you ever intend to release publicly or commercially, the BBC RemArc audio, Mixamo raw-file redistribution, and any "personal/educational-only" itch.io packs must be replaced; re-audit every licence before any release.
- If Nanite/Lumen tank performance on your Mac, bake lighting and decimate the 585k-tri train; the S Stock is the most likely single performance offender.

## Caveats
- **Prices could not all be machine-verified.** Fab masks live prices in page HTML; figures for Alan Aldred's pack, the 44-mesh Modular Subway Station & Train, and Dekogon's paid tunnel/terminal could not be confirmed exactly. Roy Sousa's is a tiered range ($54.99 to $85.99). Leartes' price differs across its own channels ($62.99 Gumroad / ~$69.99 ArtStation / $89.99 in a bundle). Verify inside the Epic Games Launcher/Fab while logged in.
- **"Free" often means account-gated, not public-domain.** Fab/Epic content is licensed for use in Unreal Engine products; Mixamo requires an Adobe account and forbids redistributing raw files; [adobe](https://community.adobe.com/questions-696/mixamo-faq-licensing-royalties-ownership-eula-and-tos-589400) City Sample content is Unreal-only. All fine for this project but not CC0.
- **Licences vary within a single site.** Sketchfab, Freesound and ShareTextures mix CC0, CC-BY and non-commercial. The timblewee trains are CC-BY (attribution required). Always check the individual item.
- **BBC Sound Effects are RemArc-licensed: personal/educational/research only.** Great for a private hobby build, not for any monetised or public release.
- **macOS specifics:** Nanite and Lumen run on Apple Silicon via Metal but cost performance; some Fab packs ship as DX12/Shader-Model-6 UE projects (e.g. the UE5.8 "Underground Metro Station") that you should import as GLB/blend rather than open directly. Verify any pack bundling a code plugin actually builds on macOS before relying on it.
- **The reference portfolio pieces are not asset packs.** Cruz's scene and the Behance TFL piece ship nothing downloadable beyond the credited timblewee train; their value is technique (ZBrush > Blender retopo > Sampler photo-materials > Painter > Unreal + decals).
- **Sketchfab trains are flagged "not game ready"** by their author, meaning you should expect to re-topologise/retexture for real-time use.

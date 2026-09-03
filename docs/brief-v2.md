# LAST TRAIN v2 — high fidelity 3D build brief

Supersedes v1. Same game design, new rendering target.

Round-based zombie survival on a fictionalised Elizabeth line, rendered in real
time 3D with a full post-processing stack. Visual target is stylised realism with
heavy atmospheric lighting, not photorealism.

---

## PART 0 — WHAT CHANGED FROM V1, AND WHY

Everything in Parts 1.1 to 1.12 of v1 (the core loop, train timer, rounds,
zombie roster, weapons, perks, economy, persistence) is **unchanged and still
authoritative**. Do not redesign gameplay.

What changed:

| Area | v1 | v2 |
|---|---|---|
| Renderer | Canvas 2D | Three.js, WebGPU with WebGL2 fallback |
| Camera | Pure top-down | Steep angled third person, ~55 degrees, spring arm |
| Geometry | Tile sprites | Procedural modular architecture generated from the same ASCII grid |
| Lighting | Flat | Real time PBR, dynamic lights, volumetrics, SSR, SSAO |
| Zombies | Circles | Rigged humanoid, GPU crowd via vertex animation textures |
| Post FX | None | Full stack: ACES tonemap, bloom, SSR, SSAO, volumetric fog, grain, CA, motion blur |
| APK | Phase 6 goal | Stretch goal only, behind a low-quality preset |

The ASCII station grids are retained deliberately. They stay the authoring
format and become the input to the geometry generator, so all 41 stations remain
hand-authorable and cheap to produce.

### The honest ceiling

Matrix Awakens was a large-team production dominated by asset creation:
photogrammetry, scanned MetaHumans, mocap libraries, roughly 30GB of data, on
fixed console hardware with Nanite and Lumen. None of that is model-limited and
none of it ports to a browser. Do not target it.

What this brief targets instead is the thing that actually produces the
impression of high production value in a dark, enclosed setting:

1. **Lighting quality over geometry quantity.** Emissive strip lights,
   volumetric shafts through tunnel mouths, a real spotlight torch, wet floors
   with screen space reflections. Darkness is a budget multiplier.
2. **Procedural architecture.** Tube stations are modular and repetitive by
   nature. Extruded tunnel splines, kit-bashed platform and concourse modules,
   instanced props. This is generated code, which is exactly what the model is
   good at, and costs nothing in assets.
3. **Post-processing discipline.** A correct tonemap, restrained bloom, SSAO and
   film grain will do more for perceived quality than doubling the polygon count.

---

## PART 1 — ASSET STRATEGY, READ THIS FIRST

This is the only part of the project a model cannot do for you. Source these
before Phase 4 or the build stalls.

| Asset | Source | Notes |
|---|---|---|
| Rigged humanoid zombie, 1 mesh | Mixamo, Quaternius, or Sketchfab CC0 | One is enough. Vary by scale, colour and animation speed |
| Animations: walk, run, shamble, attack, stagger, death x2 | Mixamo | Free, retargets to any humanoid, export as glTF |
| PBR texture sets: concrete, tile, wet tile, steel, painted brick, rubber, glass | Poly Haven, ambientCG | CC0. 2K is plenty at this camera distance |
| HDRI for indirect light, 2 or 3 | Poly Haven | Sodium-lit interior and night exterior |
| Weapon meshes | Quaternius low poly weapon packs, or procedural boxes | Guns are barely visible at this camera angle. Do not overinvest |
| Fonts | Google Fonts, anything geometric that is NOT Johnston | See legal note |

Everything else is generated: all station geometry, all props by instanced
primitive composition, all VFX, all audio.

**Budget targets.** Total download under 40MB. Draco-compress all glTF. Basis
Universal or KTX2 for every texture. Nothing uncompressed.

**Legal, unchanged from v1.** Station names and geography are factual and fine.
No roundel, no TfL logo, no Johnston or New Johnston typeface, no reproduction of
the official line diagram, no transcribed announcement recordings. Original
palette, original typography, original announcement phrasing.

---

## PART 2 — RENDERING SPEC

### 2.1 Pipeline

- Three.js current release. `WebGPURenderer` primary, WebGL2 fallback with the
  post stack degraded, not disabled.
- ACES Filmic tonemapping, sRGB output, physically correct lights.
- Render graph order: depth prepass, opaque, decals, transparent, volumetrics,
  post chain.
- Three quality presets (Low, Medium, High) switching shadow resolution, SSR
  on/off, volumetric step count and SSAO sample count. Low must run on
  integrated graphics.

### 2.2 Lighting

- One directional light for surface stations only, disabled underground.
- Baked-feel indirect via an HDRI environment map with per-station intensity and
  tint.
- Emissive strip lighting along platform edges and ceilings, driving real point
  lights on a budget of 8 active lights nearest the camera.
- **Torch:** a real spotlight parented to the player with a slight sway, a cookie
  texture for shape, and volumetric scattering. This is the signature effect.
  It must feel like a light, not a fog-of-war circle.
- Muzzle flash as a one-frame high-intensity point light plus bloom. Cheap and
  extremely effective.
- Blackout mechanic kills all emissives and point lights, leaving torch only.

### 2.3 Post-processing chain

In order: SSAO, SSR (wet floors only, via a roughness mask), volumetric fog
raymarch, bloom, motion blur on camera velocity, chromatic aberration at screen
edges, film grain, vignette, sharpen. Every stage individually toggleable and
tuned per station.

Damage feedback: crimson radial vignette that pulses, plus grain intensity ramp
as health drops. No health-bar-only feedback.

### 2.4 Procedural architecture generator

The most important system in v2. Consumes a v1 ASCII grid and emits geometry.

- Walls extrude to height with tiled UVs and a skirting profile.
- Platform edge tiles generate the platform lip, the yellow line, the trackbed
  drop, rails and sleepers.
- Tunnel mouths generate an extruded tube along a spline, receding into fog,
  with ring supports and cable runs.
- Escalators generate stepped geometry with a moving handrail shader.
- Ticket barriers, benches, signage frames, roundel-free wayfinding panels,
  litter bins, ad frames with original placeholder artwork: all instanced props
  placed by grid rule plus seeded random jitter.
- Water tiles generate a reflective plane with a flow-mapped normal shader for
  the flood mechanic, and rise per round.
- Every station gets a seeded deterministic pass, so the same grid always
  produces the same station.

Instancing is mandatory. Target under 400 draw calls on the largest station.

### 2.5 Crowd rendering

46 simultaneously animated humanoids is the hard constraint.

- Bake the six animations to **vertex animation textures** and drive them with a
  single instanced draw call, with per-instance animation index, time offset,
  playback rate, scale and colour tint. Do not use 46 individual SkinnedMeshes.
- LOD: full VAT near, simplified mesh mid, billboard beyond torch range.
- Shadow casting from zombies only within the torch cone.
- Gore: instanced blood decal projection onto surfaces, pooled and capped, plus
  a small GPU particle burst per hit. Ragdoll is out of scope; use a death
  animation with a snap to a settled pose.

### 2.6 Performance budget, non-negotiable

- 60fps at 1080p on a mid-range discrete GPU, High preset, 46 live zombies,
  largest station.
- 60fps at 1080p on integrated graphics, Low preset, 30 zombies.
- Frame budget: under 6ms CPU, under 10ms GPU.
- Ship with an in-build profiler overlay: fps, draw calls, triangles, active
  lights, entity count, VAT instances, per-post-stage GPU time.

### 2.7 Camera

Steep angled third person at roughly 55 degrees, spring-arm follow with damping,
mild lookahead toward the cursor, FOV punch on firing, shake on damage. Player
must never be occluded by geometry: fade or dither any wall between camera and
player.

---

## PART 3 — REVISED CREDIT STRATEGY AND GATES

Scope roughly doubles in v2, so the split shifts and the gates get stricter. The
failure mode to guard against is a beautiful engine with no game in it.

| Phase | Model | Output |
|---|---|---|
| 1 | Fable | Architecture and render graph plan. No code |
| 2 | Fable | Render pipeline, post stack, quality presets, lighting rig, profiler |
| 3 | Fable | Procedural architecture generator plus VAT crowd system |
| 4 | Fable | Engine core: fixed timestep, input, collision, flow field, camera |
| 5 | Fable | Schemas, round manager, train timer, three reference stations |
| 6 | Opus | 38 stations, 12 weapons, attachments, perks, economy |
| 7 | Opus | HUD, menus, gunsmith, stats, save, all audio |
| 8 | Opus | Balance, optimisation, optional low-preset APK |

Fable now carries five phases rather than three, because the three genuinely
hard, expensive-to-redo systems are all rendering-side: the post chain, the
geometry generator and the VAT crowd. Getting any of those wrong is not
recoverable in Opus.

**Cost control.** If credits look tight, cut in this order: SSR first, then
volumetrics, then drop to WebGL2 only. Never cut the geometry generator or the
crowd system, since those are what make it look like a real game.

### Hard gates

- **GATE A** — plan only, no code. You review the render graph and file layout.
- **GATE B** — an empty test station renders with the full lighting and post
  stack, torch working, profiler visible, all three presets switchable. No
  gameplay. This is the go/no-go on whether it looks good enough to continue.
- **GATE C** — the generator turns one ASCII grid into a complete station you can
  fly a debug camera through, and 46 VAT zombies animate at 60fps.
- **GATE D** — engine core runs: player moves, shoots, zombies path and jostle.
- **GATE E** — rounds, breather, train timer and travel across three stations.

Gate B is the important one. If the empty station does not look striking with
nothing in it, stop and retune before spending anything on gameplay.

---

## PART 4 — THE FABLE PROMPT

Paste as a single message.

```
You are building LAST TRAIN: a round-based zombie survival shooter set on a
fictionalised version of London's Elizabeth line, rendered in real time 3D.

VISUAL TARGET
Stylised realism with heavy atmospheric lighting. Dark underground stations, a
volumetric torch, wet reflective floors, emissive strip lighting, restrained
bloom, film grain. Aim for the impression of high production value through
lighting and post-processing discipline, NOT through polygon count or asset
fidelity. Do not attempt photorealism.

STACK, FIXED, DO NOT SUBSTITUTE
TypeScript, Vite, Three.js current release. WebGPURenderer primary with a WebGL2
fallback that degrades the post stack rather than disabling it. No game engine,
no physics library, no React. Fixed 60Hz simulation timestep decoupled from
render, with interpolation.
Modules: engine/, render/, systems/, data/, ui/, audio/. All balance constants in
data/balance.ts. All station grids in data/stations/.

WORKING RULES, IMPORTANT
- Work in five phases. STOP at every GATE and wait for my explicit approval.
  Do not begin the next phase until I reply.
- No prose explanation of code you have written. No summaries. No restating this
  brief back to me. Code and file paths only, plus one line at each gate.
- No TODOs, no placeholder bodies, no "implement later". Everything you write is
  finished and runnable.
- Do NOT write: station data beyond the three named in Phase 5, HUD, menus,
  audio, save system, stats screen, attachment tables, weapon data beyond two
  proof guns. A later pass handles all of that.
- British spelling in comments and user-facing strings. Never use em-dashes.
- Ask a question only if genuinely blocked.

PERFORMANCE BUDGET, NON-NEGOTIABLE
60fps at 1080p, High preset, 46 animated zombies, largest station, mid-range
discrete GPU. 60fps at Low preset on integrated graphics with 30 zombies. Under
400 draw calls. Instancing is mandatory for all repeated geometry.

ASSETS
Assume these exist at /assets and write loaders against them, but do not attempt
to create them: one Draco-compressed rigged humanoid glTF, six glTF animation
clips (walk, run, shamble, attack, stagger, death), KTX2 PBR texture sets for
concrete, tile, wet tile, steel, painted brick, rubber and glass, and two HDRIs.
Everything else must be generated procedurally in code.

LEGAL CONSTRAINT
Station names and geography are factual and fine to use. Do NOT reproduce any TfL
trademark: no roundel, no official logo, no Johnston or New Johnston typeface, no
reproduction of the official line diagram, no transcribed announcement
recordings. Original palette, original typography.
Palette: charcoal base #16161C, violet accent #6C4C9C, sodium amber #E0A030,
crimson #B02030.

=== PHASE 1: PLAN ONLY, NO CODE ===
Produce:
- Full file tree, one line of purpose each.
- The render graph: exact pass order, render targets, formats, and which passes
  each of the three quality presets enables.
- TypeScript interfaces for Player, Zombie, Projectile, Pickup, Interactable,
  StationDef, WeaponDef.
- System execution order per simulation tick, and how it interleaves with render.
- The design of the procedural architecture generator: how an ASCII grid becomes
  geometry, what is instanced, how determinism is seeded.
- The design of the vertex animation texture crowd system: bake format, per
  instance attributes, LOD thresholds.
Then STOP. Print "GATE A" and wait.

=== PHASE 2: RENDER PIPELINE ===
Build only the renderer, on a single hardcoded test box room:
- WebGPU renderer with WebGL2 fallback, ACES Filmic tonemapping, sRGB output,
  physically correct lights.
- Post chain in this order, every stage individually toggleable: SSAO, SSR
  masked by roughness for wet floors, raymarched volumetric fog, bloom, camera
  motion blur, edge chromatic aberration, film grain, vignette, sharpen.
- Lighting rig: HDRI environment with per-station intensity and tint, emissive
  materials driving real point lights capped at the 8 nearest the camera, and a
  player torch as a real spotlight with a cookie texture, sway, and volumetric
  scattering. The torch is the signature effect and must read as a light, not a
  fog-of-war mask.
- Three quality presets switching shadow resolution, SSR, volumetric step count
  and SSAO samples. Low must run on integrated graphics.
- Camera: steep angled third person at 55 degrees, spring arm with damping,
  cursor lookahead, FOV punch hook, shake hook, and dithered fade for any
  geometry between camera and player.
- Profiler overlay: fps, frame time split CPU and GPU, draw calls, triangles,
  active lights, and per-post-stage GPU cost.
Then STOP. Print "GATE B" and wait. I will judge whether it looks good enough
before you write any gameplay.

=== PHASE 3: GEOMETRY GENERATOR AND CROWD ===
Build:
- The procedural architecture generator, consuming this ASCII tile legend:
  # wall, . floor, = platform edge, T tunnel mouth, B boarded breakable spawn,
  D buyable debris door, W wall-buy anchor, P perk machine anchor,
  L lost property office, U upgrade bench, X ticket barrier, E escalator,
  S player spawn, ~ water.
  It must emit: extruded walls with tiled UVs and skirting, platform lips with a
  warning line and a trackbed containing rails and sleepers, tunnel mouths as
  splined extruded tubes receding into fog with ring supports and cable runs,
  stepped escalators with a scrolling handrail shader, and instanced props
  (barriers, benches, bins, signage frames, ad frames with original placeholder
  art, litter) placed by grid rule with seeded jitter. Water emits a reflective
  flow-mapped plane whose height is settable per round.
  Fully deterministic from a seed. Under 400 draw calls on a 60x40 grid.
- The vertex animation texture crowd system: bake the six animation clips to
  textures, render all zombies in a single instanced draw call with per instance
  animation index, time offset, playback rate, scale and tint. LOD from full VAT
  to simplified mesh to billboard beyond torch range. Zombie shadows only inside
  the torch cone.
- Instanced pooled blood decal projection plus a small GPU particle burst per
  hit. No ragdoll: death animation snapping to a settled pose.
- A debug fly camera and a spawner that drops 46 zombies wandering the generated
  station.
Then STOP. Print "GATE C" and wait.

=== PHASE 4: ENGINE CORE ===
Build:
- Fixed 60Hz timestep with accumulator, interpolated render.
- Input: WASD move, mouse aim, left click to fire and hold to auto-fire, Shift
  sprint with stamina, Escape pause.
- Collision: capsule colliders against generated geometry via a navigation grid
  derived from the ASCII source, plus soft push-apart between zombies. Zombies
  crowd and jostle but never overlap and never pass through each other. Must stay
  stable with 60 zombies pressed into a corridor.
- Pathfinding: flow field over the tile grid toward the player, recomputed at a
  fixed 4Hz, NOT per frame. Cost weighting for water and ticket barriers.
- Hitboxes: body plus a head zone at 2.5x damage. Hit markers with a distinct
  headshot tone hook.
- Health: 100 HP, regen begins 4 seconds after last damage at 20 HP per second,
  instant death at zero. Damage feedback is a pulsing crimson radial vignette and
  a grain ramp, not just a bar.
Then STOP. Print "GATE D" and wait.

=== PHASE 5: SCHEMAS, ROUNDS, THREE STATIONS ===
Build:
- Final StationDef and WeaponDef schemas with loaders and validators. StationDef
  carries: display name, line index, adjacency ids, tier 1 to 4, one or two
  mechanic ids from the fixed library, palette accent, HDRI intensity and tint,
  three original announcement text lines, ambient audio parameters, optional boss
  id.
  Mechanic library, fixed, add no new engine code beyond these: flood, blackout,
  narrow, escalator_rush, platform_split, open_concourse, depot, open_air,
  interchange, terminus.
- Zombie types walker, sprinter, brute, crawler, screamer. HP x1.1 per round
  compounding, speed steps at rounds 5, 10 and 20, spawn rate climbing, live cap
  40 plus 6 per heat level.
- Round manager: waves spawn from B and T points, round ends when all spawned
  zombies are dead, then a 10 second breather. Round counter never resets.
  Sprinter horde every 5th round, brute pair every 10th.
- Train timer: a train arrives every TRAIN_INTERVAL_MS (100000) and dwells
  TRAIN_DWELL_MS (25000), both single tuneable constants. Announcement fires 15
  seconds before arrival and on arrival. The train is real geometry arriving in
  the trackbed with headlights, door animation and rumble. Boarding during dwell
  ends the round, banks points, refills ammo and travels to a chosen adjacent
  station. Staying raises station heat by 1, adding 6 to the live cap and 12% to
  spawn rate; heat resets on travel. Travel is bidirectional and free.
- Exactly THREE hand-authored stations at full quality:
  1. Canary Wharf, tier 4, flood plus interchange. Water rises each round,
     shrinking the arena, with SSR heavily featured.
  2. Whitechapel, tier 3, blackout plus platform_split. Two platforms joined only
     by a footbridge; every third round kills all emissives and point lights,
     leaving torch only.
  3. Paddington, tier 4, open_concourse plus interchange. Wide, low cover, high
     spawn count, best wall-buys, surface light shafts through the roof.
  Wire all three as adjacent so travel is testable.
- Implement the WeaponDef system fully but author only TWO guns, a service pistol
  and a pump shotgun, proving spread cone, pellet count and penetration. Muzzle
  flash as a one-frame high intensity point light plus bloom.
- Wall-buy, debris door and perk machine interaction on their anchors, with the
  economy: 10 points per hit, 60 per kill, 130 per headshot kill.
Then STOP. Print "GATE E" and wait.

Do not proceed past GATE E. Begin Phase 1 now.
```

---

## PART 5 — THE OPUS PROMPTS

Run in Claude Code against the repo Fable produced, one session each.

### Phase 6 — content volume

```
This repo is LAST TRAIN, a 3D round-based zombie survival shooter on a
fictionalised Elizabeth line. Engine, renderer, procedural geometry generator,
crowd system, schemas and three reference stations (Canary Wharf, Whitechapel,
Paddington) already exist. Read data/stations/, data/balance.ts and the geometry
generator first, and match existing conventions exactly.

Plan before implementing and show me the plan first.

Build:
1. The remaining 38 stations as StationDef data. Full list and tier guidance in
   docs/brief-v2.md. Each needs a hand-authored ASCII grid of at least 50x35, one
   or two mechanics from the fixed library only, HDRI intensity and tint, palette
   accent, three original announcement lines, correct adjacency. Outer suburban
   are tier 1 to 2, central are tier 3 to 4. Termini get a boss id. Do NOT invent
   new mechanic ids or tile characters, and do NOT modify the generator.
2. The remaining 10 guns as WeaponDef data: machine pistol, SMG, bullpup rifle,
   assault rifle, semi-auto shotgun, LMG, DMR, bolt sniper, crossbow, and one
   original joke weapon. Assign min station tiers and wall prices.
3. Three functional attachment slots per gun (optic, magazine, barrel) with real
   stat effects, plus round-threshold unlock requirements.
4. The four perks: Commuter's Constitution +75 max HP, Quick Hands 40% faster
   reload and swap, Second Wind infinite sprint and faster regen, Double Tap +30%
   fire rate and +15% damage. Max three equipped, lost on death not on travel.
5. Lost property office: random gun, base 950, +10% per use, resets on travel.
   Upgrade bench: 5000 points, double damage, +50% mag, renamed variant, one gun
   at a time.
6. Oyster Credit: 1 per round survived, persistent, spends on attachment unlocks.

Pause after the first five stations so I can review before you do the rest.
British spelling. Never use em-dashes. No TfL trademarks.
```

### Phase 7 — interface, audio, persistence

```
Continuing LAST TRAIN. Plan first, then build:
1. HUD in DOM overlay, not in the 3D scene: health, ammo and reserve, points,
   round counter, station name, heat indicator, perk icons, train countdown,
   minimap generated from the station grid.
2. Menus: title with a slow camera orbit of a generated station, pre-run loadout
   picker, pause, gunsmith (attachment swapping, only during a breather or on the
   train, never mid-wave), death screen, end-of-line screen at termini.
3. Stats screen: rounds survived, kills by type, headshot percentage, accuracy,
   points earned, stations visited, longest single-station stand.
4. Save in localStorage, checkpointing on every arrival at a new station.
   Persist last checkpoint, all-time best round, Oyster Credit, unlocked
   attachments, per-station best. Also persist the chosen quality preset.
5. Audio, procedural where possible, zero recorded assets:
   - Web Audio for gunfire per archetype, reloads, hit markers with a distinct
     headshot tone, zombie vocals, train rumble and door chimes, and per-station
     ambient beds driven by each station's audio parameters.
   - Spatialised via PannerNode against 3D positions.
   - SpeechSynthesis for platform announcements from each station's three lines.
     Original phrasing only. Do not transcribe real recordings or imitate any
     named announcer.
6. A settings screen exposing every post-processing toggle and the quality preset.

British spelling. Never use em-dashes.
```

### Phase 8 — balance, optimisation, packaging

```
Continuing LAST TRAIN. Feature complete. Plan first, then:
1. Balance pass. Target: a competent player reaches round 12 to 15 on a first
   serious run, round 30 is a real achievement. Report every number you change
   and why.
2. Optimisation pass against the stated budget: 60fps at 1080p High with 46
   zombies on the largest station, under 400 draw calls, under 6ms CPU and 10ms
   GPU. Profile first, then fix the top three costs. Do not guess.
3. Fix bugs, especially flow field behaviour at high entity counts, the
   push-apart solver under corridor pressure, and any VAT LOD popping.
4. Verify the WebGL2 fallback path still runs and looks acceptable.
5. STRETCH ONLY, and only if I ask: wrap the build as an Android APK with
   Capacitor, forcing the Low preset, halving the render resolution with
   upscaling, capping zombies at 24, and adding on-screen touch controls behind a
   platform check. Warn me honestly if it does not hold a playable frame rate
   rather than shipping something bad.

British spelling. Never use em-dashes.
```

---

## PART 6 — RISKS

1. **Gate B is the real decision point.** If an empty station with full lighting
   does not look striking, more gameplay will not save it. Retune there or fall
   back to v1.
2. **Assets block Phase 6.** Source the rigged humanoid and animations before you
   get there.
3. **Scope doubled.** The failure mode is a gorgeous empty engine. If credits run
   short after Gate C, skip Phase 5's third station and get the game loop
   finished in Opus instead. A complete game with two stations beats a beautiful
   station with no game.
4. **Mobile is genuinely doubtful.** Treat the APK as a bonus, not a deliverable.

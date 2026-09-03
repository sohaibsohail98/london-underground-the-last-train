# LAST TRAIN, Phase 1: architecture and render graph plan

Plan only. No implementation in this phase. Everything below is the contract
that Phases 2 to 5 build against and Phases 6 to 8 extend without redesign.

---

## 1. File tree

```
last-train/
  index.html                     Single canvas plus DOM overlay root, loads src/main.ts
  package.json                   Vite, TypeScript, three, draco and ktx2 loaders only
  tsconfig.json                  strict, ES2022, bundler resolution, path alias @/ -> src/
  vite.config.ts                 Asset base path, glsl/wgsl raw import plugin, top level await
  assets/                        Sourced binaries only, see assets/README.md
    models/zombie.glb            One rigged humanoid, Draco compressed
    anims/*.glb                  walk run shamble attack stagger death
    textures/<set>/*.ktx2        albedo normal roughness metalness ao per set
    hdri/*.hdr                   sodium_interior, night_exterior
  src/
    main.ts                      Boot: detect backend, load config, create Game, start loop
    engine/
      Game.ts                    Owns World, Renderer, Clock; runs the fixed step accumulator
      Clock.ts                   Fixed 60Hz accumulator, alpha for render interpolation
      World.ts                   Entity arrays (struct of arrays), id allocation, free lists
      Input.ts                   Key and mouse state, edge detection, cursor world ray
      Events.ts                  Typed event bus for hit, kill, headshot, round, train, damage
      Random.ts                  Seeded PCG32, one stream per system, never Math.random
      Nav.ts                     NavGrid from ASCII: walkable, cost, tile lookups
      Collision.ts               Capsule versus nav grid, capsule versus capsule push-apart
      FlowField.ts               Dijkstra integration field to player at 4Hz, vector field cache
      Camera.ts                  Spring arm rig, lookahead, FOV punch, shake, occluder query
    render/
      Renderer.ts                Backend selection, size, presets, render graph execution
      Backend.ts                 WebGPURenderer or WebGL2 fallback, capability flags
      RenderGraph.ts             Ordered pass list, render target pool, per pass GPU timers
      Presets.ts                 Low Medium High tables and the preset switch
      Lighting.ts                HDRI env, emissive light budget (8 nearest), blackout switch
      Torch.ts                   Player spotlight, cookie, sway, volumetric injection
      MuzzleFlash.ts             One frame point light plus bloom boost, pooled
      Materials.ts               PBR material factory, wet mask, tiled UV helpers, KTX2 bind
      Occlusion.ts               Dithered fade for geometry between camera and player
      passes/
        DepthPrepass.ts          Depth plus view normals into GBuffer-lite
        OpaquePass.ts            Main scene colour, HDR RGBA16F
        DecalPass.ts             Instanced blood decals, depth tested, no depth write
        TransparentPass.ts       Water, glass, particles
        SSAOPass.ts              Hemisphere sampling on depth plus normals, blurred
        SSRPass.ts               Roughness masked screen space reflections, wet floors only
        VolumetricPass.ts        Half res raymarch against torch and 8 lights, upsampled
        BloomPass.ts             Threshold, 5 mip downsample, upsample and add
        MotionBlurPass.ts        Camera velocity only, from previous view projection
        ChromaticPass.ts         Radial channel offset at screen edges
        GrainPass.ts             Animated film grain, intensity driven by health
        VignettePass.ts          Base vignette plus crimson damage pulse
        SharpenPass.ts           Small unsharp mask, last before output
      shaders/
        *.wgsl, *.glsl           One file per pass, both backends where semantics differ
      crowd/
        VatBaker.ts              Bakes six clips from zombie.glb into position and normal textures
        VatMaterial.ts           Instanced material sampling the VAT atlas
        Crowd.ts                 Instance buffer management, LOD assignment, shadow gating
        Billboard.ts             Far LOD impostor atlas rendered once per session
      gore/
        Decals.ts                Pooled instanced blood decal projector
        Particles.ts             GPU particle burst, single instanced quad draw
      debug/
        Profiler.ts              Overlay: fps, CPU, GPU, draws, tris, lights, VAT count, pass costs
        FlyCamera.ts             Debug free camera for Gate C
    gen/
      Grid.ts                    ASCII parse, legend, bounds, neighbour queries
      Generator.ts               Orchestrates all emitters from a StationDef grid and seed
      emitters/
        Walls.ts                 Extruded walls, tiled UVs, skirting profile, greedy merge
        Floor.ts                 Floor slabs with wet mask painting
        Platform.ts              Lip, warning line, trackbed, rails, sleepers
        Tunnel.ts                Splined tube, rings, cable runs, fog fade
        Escalator.ts             Stepped geometry, scrolling handrail shader
        Water.ts                 Flow mapped reflective plane, height per round
        Props.ts                 Instanced props by rule plus seeded jitter
        Signage.ts               Wayfinding frames and ad frames with generated art
      Kit.ts                     Primitive prop definitions built from boxes and cylinders
      Placement.ts               Grid rules: which prop where, spacing, jitter bounds
    systems/
      Movement.ts                Player move, sprint, stamina
      Aim.ts                     Cursor ray to floor plane, facing
      Weapons.ts                 Fire, spread, pellets, penetration, reload, swap
      Hitscan.ts                 Ray versus zombie capsules, head zone, wall stop
      Zombies.ts                 Steering along flow field, separation, attack, death
      Spawner.ts                 Wave composition from round and heat, B and T points
      Rounds.ts                  Round state machine, breather, counters
      Train.ts                   Timer, announcements, arrival geometry, boarding, travel
      Health.ts                  Damage, regen, death, feedback signals
      Interact.ts                Wall-buy, debris door, perk machine, prompts
      Economy.ts                 Points, purchases, costs
      Mechanics.ts               Station mechanic hooks: flood, blackout, and the rest
    data/
      balance.ts                 Every tunable constant in the game
      legend.ts                  ASCII tile characters and their meaning
      schemas.ts                 StationDef, WeaponDef and validators
      weapons/
        pistol.ts                Service pistol
        shotgun.ts               Pump shotgun
      stations/
        index.ts                 Registry and adjacency graph
        canary-wharf.ts
        whitechapel.ts
        paddington.ts
      mechanics.ts               Fixed mechanic id library and their parameters
    ui/
      Overlay.ts                 DOM root, damage vignette bridge, later HUD home
      Prompts.ts                 Interaction prompt text
    audio/
      Audio.ts                   Context, master bus, hooks only in Phase 1 to 5
  docs/
    brief-v2.md
    phase-1-plan.md
```

---

## 2. Render graph

### 2.1 Frame order

```
 0  ShadowPass         torch spotlight shadow map, zombies inside cone only
 1  DepthPrepass       depth + octahedral view normals + material id
 2  OpaquePass         scene colour, lit, forward, 8 nearest point lights + torch
 3  DecalPass          blood decals, reads depth, depth test on, write off
 4  TransparentPass    water, glass, particles, sorted back to front
 5  SSAOPass           half res AO from pass 1, 4x4 bilateral blur, applied multiplicatively
 6  SSRPass            full res, roughness mask < 0.35 and wet mask > 0.5 only
 7  VolumetricPass     half res raymarch, torch cookie + point lights, bilateral upsample
 8  BloomPass          threshold 1.0 in HDR, 5 mips
 9  Tonemap            ACES filmic, exposure per station, inside the composite
10  MotionBlurPass     camera velocity reprojection, 8 taps, clamped
11  ChromaticPass      radial, zero in centre 60 percent of frame
12  GrainPass          per frame animated, intensity = base + healthDrivenRamp
13  VignettePass       base + crimson damage pulse
14  SharpenPass        unsharp mask, 0.3 default
15  Present            sRGB out, then DOM overlay draws on top
```

Passes 5 to 8 read the HDR colour and write back to it. Passes 10 to 14 run in
LDR after tonemap. Every pass has an `enabled` flag and a `cost` timer.

### 2.2 Render targets

| Name | Format | Resolution | Producers | Consumers |
|---|---|---|---|---|
| shadowTorch | Depth24 | preset | 0 | 2, 7 |
| depth | Depth32F | full | 1 | 2, 3, 4, 5, 6, 7, 10 |
| normalId | RG16F normals + R8 id | full | 1 | 5, 6 |
| hdrColour | RGBA16F | full | 2, 3, 4 | 5, 6, 7, 8, 9 |
| hdrHistory | RGBA16F | full | 9 (prev frame) | 10 |
| aoHalf | R8 | half | 5 | 5 blur, 2 apply |
| ssrColour | RGBA16F | full | 6 | 9 |
| volHalf | RGBA16F | half | 7 | 9 |
| bloomMips | RGBA16F x5 | half to 1/32 | 8 | 9 |
| ldrA, ldrB | RGBA8 | full | 9 to 14 ping pong | next pass |
| vatPos, vatNrm | RGBA16F | see crowd | baker at load | 2, 0 |

WebGL2 fallback: RGBA16F stays (EXT_color_buffer_float), depth read via a
depth texture, no compute so SSAO and volumetrics run as fragment passes at
lower sample counts.

### 2.3 Presets

| Setting | Low | Medium | High |
|---|---|---|---|
| Torch shadow map | 1024 | 2048 | 4096 |
| Zombie shadows in cone | off | on | on |
| SSAO samples | 8 | 16 | 24 |
| SSR | off | on, half res | on, full res |
| Volumetric steps | 16 | 32 | 64 |
| Volumetric resolution | quarter | half | half |
| Bloom mips | 3 | 5 | 5 |
| Motion blur | off | 4 taps | 8 taps |
| Chromatic aberration | off | on | on |
| Sharpen | off | on | on |
| Point light budget | 4 | 8 | 8 |
| Crowd LOD distances | 8 / 14 | 12 / 20 | 16 / 26 |
| Live zombie cap | 30 | 46 | 46 |

Grain and vignette are always on because they carry damage feedback.

---

## 3. Core interfaces

```ts
type EntityId = number;

interface Vec2 { x: number; z: number }

interface Player {
  id: EntityId;
  pos: Vec2; prevPos: Vec2; vel: Vec2; facing: number;
  radius: number;
  hp: number; maxHp: number; lastDamageTick: number;
  stamina: number; sprinting: boolean;
  points: number;
  weapons: [WeaponInstance, WeaponInstance | null]; active: 0 | 1;
  perks: PerkId[];
  torchOn: boolean;
}

interface WeaponInstance {
  defId: string;
  mag: number; reserve: number;
  reloadEndTick: number; nextFireTick: number;
  upgraded: boolean;
  attachments: { optic?: string; magazine?: string; barrel?: string };
}

type ZombieType = 'walker' | 'sprinter' | 'brute' | 'crawler' | 'screamer';
type ZombieState = 'spawning' | 'chasing' | 'attacking' | 'staggered' | 'dying' | 'dead';

interface Zombie {
  id: EntityId;
  type: ZombieType; state: ZombieState;
  pos: Vec2; prevPos: Vec2; vel: Vec2; facing: number;
  radius: number; height: number; headHeight: number;
  hp: number; maxHp: number; speed: number;
  attackCooldownTick: number; stateEndTick: number;
  scale: number; tint: number;            // per instance render attributes
  animIndex: number; animOffset: number; animRate: number;
  lod: 0 | 1 | 2;
}

interface Projectile {                    // hitscan by default; this is for crossbow style
  id: EntityId; ownerId: EntityId;
  pos: Vec2; vel: Vec2; height: number;
  damage: number; penetration: number;
  bornTick: number; ttlTicks: number;
}

type PickupKind = 'ammo' | 'points' | 'nuke' | 'insta';
interface Pickup {
  id: EntityId; kind: PickupKind; pos: Vec2; expireTick: number;
}

type InteractableKind = 'wallbuy' | 'debris' | 'perk' | 'lostproperty' | 'upgrade' | 'train';
interface Interactable {
  id: EntityId; kind: InteractableKind; pos: Vec2; radius: number;
  cost: number; payload: string;          // weapon id, perk id, door id, station id
  used: boolean; enabled: boolean;
  promptKey: string;
}

type MechanicId =
  | 'flood' | 'blackout' | 'narrow' | 'escalator_rush' | 'platform_split'
  | 'open_concourse' | 'depot' | 'open_air' | 'interchange' | 'terminus';

interface StationDef {
  id: string; displayName: string;
  lineIndex: number; adjacent: string[];
  tier: 1 | 2 | 3 | 4;
  mechanics: [MechanicId] | [MechanicId, MechanicId];
  grid: string[];                         // rows of equal length, legend in data/legend.ts
  seed: number;
  accent: number;                         // hex colour
  hdri: { file: string; intensity: number; tint: number };
  exposure: number;
  fogDensity: number; fogColour: number;
  wetness: number;                        // 0..1 floor wet mask bias
  announcements: [string, string, string];
  ambient: { hum: number; drip: number; wind: number; rumbleDistance: number };
  bossId?: string;
  wallbuys: Record<string, string>;       // anchor label -> weapon id
  perks: Record<string, PerkId>;
  debrisCosts: Record<string, number>;
}

type WeaponClass =
  | 'pistol' | 'machinepistol' | 'smg' | 'bullpup' | 'rifle' | 'shotgun'
  | 'lmg' | 'dmr' | 'sniper' | 'crossbow' | 'joke';

interface WeaponDef {
  id: string; name: string; upgradedName: string; class: WeaponClass;
  damage: number; headMultiplier: number;   // 2.5 default
  rpm: number; auto: boolean;
  magSize: number; reserveMax: number; reloadMs: number; swapMs: number;
  spreadDeg: number; spreadMoveDeg: number;
  pellets: number; pelletSpreadDeg: number;
  penetration: number;                      // zombies passed through before stopping
  range: number; falloffStart: number; falloffEnd: number;
  recoilFovPunch: number; muzzleIntensity: number;
  minTier: 1 | 2 | 3 | 4; wallPrice: number; ammoPrice: number;
  fireSoundKey: string;
}
```

Entities live in `World` as struct of arrays keyed by `EntityId`; the
interfaces above are the logical views used by systems, not heap objects
allocated per frame.

---

## 4. Simulation tick and render interleave

Fixed step `DT = 1/60`. `Clock` accumulates real time; while accumulator
exceeds `DT`, run one tick and subtract. Cap at 5 ticks per frame, then drop
remaining time. Render uses `alpha = accumulator / DT` to interpolate `prevPos`
to `pos` for every visible entity and the camera target.

Per tick, in this order:

```
 1 Input.sample          edge detect keys, cursor ray to floor plane y=0
 2 Rounds.update         state machine, breather timer, round start signals
 3 Train.update          countdown, announce at T-15s, arrive, dwell, depart
 4 Mechanics.update      flood height, blackout toggle, per station hooks
 5 Spawner.update        emits zombies from B and T anchors per rate and cap
 6 Movement.update       player intent to vel, sprint drain and regen
 7 FlowField.update      every 15 ticks (4Hz): rebuild integration + vectors
 8 Zombies.steer         sample flow vector, add separation, clamp speed, set facing
 9 Collision.zombies     push-apart passes x3 with sleep for settled pairs
10 Collision.world       capsule versus nav grid for player and zombies
11 Zombies.attack        range check, cooldown, damage event
12 Weapons.update        fire, reload, swap timers, spawn hitscan queries
13 Hitscan.resolve       ray versus capsules, head zone, wall stop, penetration count
14 Health.update         apply damage events, regen after 4s at 20/s, death
15 Interact.update       prompt selection, purchase on key press
16 Economy.update        points from hit, kill, headshot events
17 Zombies.lifecycle     dying timers, death snap, free entity
18 Camera.tickTarget     lookahead target, shake decay, FOV punch decay
19 Events.flush          deliver queued events to render and audio bridges
20 World.swapPrev        copy pos into prevPos for interpolation
```

Render each frame, after ticks:

```
 a Camera.interpolate     spring arm eval against interpolated player
 b Crowd.sync             write instance buffers: interpolated pos, facing,
                          anim index and time, scale, tint, LOD
 c Lighting.select        8 nearest emissive lights to camera, blackout mask
 d Torch.sync             sway noise, cookie rotation, aim direction
 e Occlusion.query        walls between camera and player -> dither uniform
 f Decals.sync, Particles.tick(gpu)
 g RenderGraph.execute    passes 0 to 15
 h Profiler.draw
```

Render never mutates simulation state. Simulation never touches three.js
objects; it emits events and writes plain arrays that render reads.

---

## 5. Procedural architecture generator

### 5.1 Input

`StationDef.grid` as rows of the fixed legend:

```
#  wall           .  floor          =  platform edge   T  tunnel mouth
B  boarded spawn  D  debris door    W  wall-buy        P  perk machine
L  lost property  U  upgrade bench  X  ticket barrier  E  escalator
S  player spawn   ~  water
```

One tile is `TILE = 1.5m`. Wall height `WALL_H = 3.6m`. Ceiling generated over
every non-wall interior tile at `WALL_H`. Anchors (`W P L U S B D`) are floor
for movement purposes with a prop or trigger placed on them.

### 5.2 Pipeline

```
parse       -> Grid: cells, width, height, bounds, neighbour and run queries
classify    -> per cell: interior, exterior, edge orientation, run membership
rooms       -> flood fill floor regions; platform region = region touching '='
seed        -> Random(stationSeed) split into named streams: walls, props,
               signage, litter, jitter, water. Order of consumption is fixed,
               so output is identical for identical grid + seed.
emit walls  -> greedy merge of collinear wall faces into quads; one merged
               BufferGeometry per texture set; UVs in world metres so tiles
               repeat correctly across merged faces; skirting strip as a
               second extrusion 0.15m high with its own material slot.
emit floor  -> region slabs, vertex colour channel R = wetness from
               StationDef.wetness + distance to water + seeded puddle noise;
               material reads R as roughness and SSR mask.
emit platform -> for each run of '=': lip 0.2m proud of floor, warning line
               as an emissive strip decal 0.1m wide set back 0.6m, trackbed
               dropped 1.1m, two rails as instanced steel boxes per tile,
               sleepers instanced every 0.6m, third rail on the far side.
emit tunnel -> for each 'T' run touching a trackbed: a CatmullRom spline from
               the mouth receding 40m along the track direction with a gentle
               seeded curve; TubeGeometry radius 2.6m, open at the mouth;
               ring supports instanced every 3m; two cable runs as tube
               geometry offset along the spline; fog density boosted inside
               the tube via a per-vertex fog factor so it reads as receding.
emit escalator -> for each 'E' run: stepped stairs at 30 degrees rising
               over the run length, side panels, handrail as a box with a
               material whose UV offset scrolls with time.
emit water  -> one plane covering every '~' cell's bounding region, height
               driven by Mechanics.flood each round; flow-mapped normal
               shader; SSR mask 1.0; also tags nav cost for those cells.
emit props  -> Placement rules, see 5.3, producing InstancedMesh per prop
               kind with seeded jitter in position (up to 0.2 tiles),
               yaw (up to 6 degrees), scale (0.95 to 1.05).
emit lights -> emissive ceiling strips along room long axes every 4 tiles,
               platform edge strips continuous; each strip registers a
               candidate point light for the 8-light budget with its
               position, colour and intensity.
nav         -> NavGrid: walkable = not '#', not lip, not trackbed; cost
               water 3.0, barrier 4.0 (passable but slow), otherwise 1.0.
```

Output is a `GeneratedStation`: groups of merged static meshes, an array of
InstancedMesh, light candidates, water controller, anchor list with world
positions and labels, tunnel spawn points, train stop transform, nav grid.

### 5.3 Prop placement rules

| Prop | Rule |
|---|---|
| Ticket barrier | every 'X' cell, oriented perpendicular to the wider adjacent floor run |
| Bench | wall-adjacent floor cells in rooms larger than 30 cells, every 5 tiles along the wall |
| Litter bin | every 9 tiles along walls, offset from benches |
| Wayfinding panel | wall face at room entrances, 2.2m high |
| Ad frame | wall faces on platforms every 6 tiles at 1.6m, generated art from seeded gradient + shape composition |
| Signage frame | above escalator top and bottom |
| Boarded panel | 'B' cell wall face, planks as instanced boxes, removed by breakable state |
| Debris pile | 'D' cell, seeded cluster of boxes, removed on purchase |
| Machines | 'W' wall-buy plate on nearest wall, 'P' perk machine box with emissive face, 'L' shutter counter, 'U' bench with lamp |
| Litter | seeded scatter of small quads on floor, density scaled by tier |

All props are compositions of box and cylinder primitives from `Kit.ts`,
sharing one PBR material atlas per material class. No external prop meshes.

### 5.4 Draw call budget on a 60x40 grid

| Group | Draws |
|---|---|
| Merged walls per texture set | 4 |
| Floor regions | 1 to 3 |
| Ceiling | 1 |
| Platform lips, trackbed | 2 |
| Rails, sleepers, rings, cables (instanced) | 4 |
| Tunnel tubes | up to 4 |
| Escalators | up to 4 |
| Props, one draw per kind | about 14 |
| Emissive strips (instanced) | 2 |
| Water | 1 |
| Crowd (VAT, mid, billboard) | 3 |
| Decals, particles | 2 |
| Train | 3 |
| Total scene | well under 60 |

Post passes add about 25 draws. Total comfortably under 400; the 400 figure is
the ceiling for Phase 6 content, not the target.

---

## 6. Vertex animation texture crowd

### 6.1 Bake

At load, `VatBaker` loads `zombie.glb` plus six clips and, for each clip,
steps the SkinnedMesh at 30fps, reads skinned vertex positions and normals
into rows of two RGBA16F textures. Layout:

```
width  = vertexCount (target mesh under 4096 verts after decimation, so 4096)
height = sum over clips of frameCount, plus one guard row per clip
clip table (uniform array): startRow, frameCount, fps, loop
```

Six clips at roughly 1s to 2s each, 30fps, gives about 240 rows. Two textures
of 4096 x 256 x RGBA16F is 16MB GPU memory total. Bake once per session and
cache in memory; no disk bake step is required.

Position texture stores offsets from the bind pose so precision holds; normals
stored as unit vectors remapped to 0..1.

### 6.2 Per instance attributes

```
aInstancePos     vec3   interpolated world position
aInstanceFacing  float  yaw
aInstanceScale   float  0.9 to 1.15
aInstanceTint    vec3   skin and clothing tint multiplier
aInstanceAnim    vec4   clipIndex, timeOffset, playbackRate, blendToClip
aInstanceFlags   float  bit 0 hitFlash, bit 1 inTorchCone
```

Shader: frame = (time * playbackRate + timeOffset) * fps mod frameCount;
sample rows floor(frame) and floor(frame)+1, lerp. A second sample from
`blendToClip` allows a 0.15s crossfade when state changes, so transitions
from shamble to attack do not pop.

Death: play death clip once, on last frame hold; entity stays as a static VAT
instance for 4s then is recycled. No ragdoll.

### 6.3 LOD

| LOD | Distance (High) | Rendering |
|---|---|---|
| 0 | 0 to 16m | Full VAT mesh, normals from texture, PBR |
| 1 | 16 to 26m | Decimated mesh (about 800 verts), same VAT rows sampled from a second, smaller position texture baked at load, lambert |
| 2 | beyond 26m or outside torch cone in blackout | Camera facing billboard from an impostor atlas of 8 yaw angles x 4 poses rendered once at load |

Three InstancedMesh, one per LOD, instance counts rewritten each frame.
Distance thresholds come from the preset table. LOD hysteresis 1.5m to avoid
flicker at the boundary.

### 6.4 Shadows and lighting

Zombies cast shadows only into the torch shadow map, and only when
`inTorchCone` is set by a CPU cone test. Point lights from the 8-light budget
light zombies without shadows. Hit flash is a one tick emissive boost via
the flags attribute.

### 6.5 Gore

`Decals.ts`: 256 pooled decal instances, each a box projector; on hit, place
at the impact point on the nearest surface behind the zombie, seeded
rotation and scale, fade over 20s and recycle oldest. `Particles.ts`: one
instanced quad draw of 2048 particles simulated in a ping-pong texture on
WebGPU, or in a CPU array on WebGL2 (cheap at this count), 24 particles per
hit, 0.4s life, gravity, colour crimson.

---

## 7. Determinism and seeding

- `Random` is PCG32. Each station derives named streams from
  `hash(stationSeed, streamName)`.
- The generator consumes streams in fixed order. Adding a new prop rule in
  Phase 6 must append a new stream rather than reordering existing ones.
- Gameplay randomness (spawn choice, weapon spread) uses separate streams
  seeded per run, so the same station always builds identically regardless of
  play.

---

## 8. What Phase 2 will produce

Files: `index.html`, `package.json`, `tsconfig.json`, `vite.config.ts`,
`src/main.ts`, everything under `src/render/` except `crowd/` and `gore/`,
`src/engine/Camera.ts`, `src/engine/Clock.ts`, `src/render/debug/Profiler.ts`,
and a hardcoded test room in `src/render/debug/TestRoom.ts` used only until
the generator exists.

GATE A

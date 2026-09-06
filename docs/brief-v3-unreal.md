# LAST TRAIN v3 - Unreal Engine 5, first person

Supersedes `brief-v2.md`. The game design is largely retained; the engine,
camera and delivery target all change.

Status of the earlier documents:

- `brief-v2.md` - superseded for engine, renderer and camera. Still
  authoritative for the round loop, the train mechanic, station tiering, the
  mechanic library, the economy and the legal constraints.
- `phase-1-plan.md`. Removed. It described a Three.js render graph and the
  `web/` tree is the record of that build.
- The `00` to `08` series in Project Knowledge. These were external Project
  Knowledge uploads and were never committed to this repo. They always assumed
  UE5 and first person, so their gameplay, art and testing guidance still
  applies in spirit, but the surviving authoritative documents are the ones in
  `docs/`. The roadmap they carried is replaced by Part 3 below.
- `docs/reference/`. Holds `reference-frame.png` and `reference-frame-notes.md`
  only. Any other `docs/reference/` design notes cited below were Project
  Knowledge files and are not in the repo.

---

## PART 0 - WHAT CHANGED, AND WHAT IT COSTS

| Area | v2 | v3 |
|---|---|---|
| Engine | Three.js, WebGPU, browser | Unreal Engine 5, Windows first |
| Camera | Third person, 55 degree pitch | First person, wide FOV |
| Lighting | Hand-built post chain | Lumen, plus a tuned post process volume |
| Geometry | Procedural from ASCII grids | Hand-built modular kit, Nanite |
| Crowd | Vertex animation textures | Skeletal meshes, animation budgeting, LODs |
| Scope | 41 stations | One station done properly, then expand |
| Delivery | URL | Packaged Windows build |
| Verification | Typecheck and bundle on every change | Your compile, your editor |

### What carries over

All of it is design, none of it is code:

- The round loop, breather, and difficulty scaling curve.
- The train mechanic: 100 second interval, 25 second dwell, boarding as
  optional escape, station heat when you stay.
- The station tiering, adjacency, and the fixed mechanic library. The 41
  station list is an aspirational expansion list, not a plan: v1 commits to 2
  stations (see Part 5).
- The economy: 10 per hit, 60 per kill, 130 per headshot kill.
- The four perks, the lost property office, the upgrade bench, Oyster Credit.
- Original announcement phrasing and the legal constraints.
- The ASCII station grids, now demoted from an authoring format to a *layout
  sketch* used to block out geometry in the editor.

### What is discarded

The entire `src/` tree: renderer, post chain, procedural generator, crowd
system, collision, aim model. Around 12,000 lines. It is kept on `main` under
the `phase-03` tag rather than deleted, so it remains available if the browser
target is ever revisited.

### The honest ceiling, restated

The reference frame's quality comes from three things. UE5 gives you the first
essentially free, the second with effort, and the third not at all:

1. **Lighting and atmosphere.** Lumen, volumetric fog, emissive materials and
   a well-tuned post process volume will get you there. This is the largest
   share of the impression and it is mostly a tuning problem.
2. **Environment detail.** Achievable with a disciplined modular kit, good
   trim sheets, decals and clutter. This is weeks of work, not days, and it is
   the single biggest time sink in the project.
3. **Characters.** Scanned humans, mocap, and per-zombie variation. Not
   achievable solo at that fidelity. Plan for stylised-but-solid zombies from
   marketplace or MetaHuman bases and accept the gap.

Do not measure early milestones against the frame. Measure them against
"does the first ten minutes feel coherent", which is the success test in
`00_PROJECT_README.md` and a much better one.

---

## PART 1 - TARGET AND CONSTRAINTS

**Engine.** UE5.4 or later. Windows development. C++ for reusable systems,
Blueprints for level assembly and configuration, per
`03_TECHNICAL_ARCHITECTURE.md`. Project prefix `LT`.

**Camera.** First person. Wide default FOV, around 95 to 100 horizontal, so the
platform reads. Weapon occupies the lower portion of frame without dominating.

**Rendering.** Lumen for global illumination and reflections, Nanite on static
geometry, virtual shadow maps. Hardware ray tracing optional and off by
default. Target 60fps at 1080p on a mid-range discrete GPU with 24 zombies.

**Aiming.** Hip fire and aim down sights both required, and the difference must
be mechanical, not cosmetic:

- Hip fire: wide spread cone, full movement speed, no FOV change.
- ADS: tight cone, movement reduced to a walk, FOV narrowed, weapon raised.
- Recoil bloom accumulates per shot and decays, so holding the trigger stops
  being accurate.
- Sprinting forces hip fire.

These numbers exist and are tuned: see `Source/LastTrain/Public/Weapons/`.
The spread model from the discarded web build transfers directly as design.

**Line identity and rolling stock (canonical).** The fictional line is a
fictionalised Crossrail-scale line: a main-line loading-gauge line modelled on
the Elizabeth line, NOT a deep-level tube. The line is renamed in-world;
station names and real geography stay factual.

- **Rolling stock:** the Class 345 "Aventra" silhouette. Full-width curved
  wraparound cab windscreen (the "smiling" front), walk-through articulated
  interior with full open gangways (you see the whole train end to end down the
  centre, NOT distinct cars with black gaps between them), 3 double-leaf sliding
  plug doors per side per car, near-vertical body sides (a box with only slight
  tumblehome), long and low with a flat roof and a deep skirt. Approximate
  dimensions: 23 m driving cars, 22.5 m intermediate cars, 2.77 m body width,
  3.76 m rail to roof, 1.145 m floor above rail, a 9-car unit about 205 m long.
  LED strip lighting, ceiling-mounted passenger-information screens,
  longitudinal (side-facing) seating near the doors and transverse further in.
- **Station profile:** because the train is main-line gauge, the platform
  environment is a large tiled hall or big-bore tunnel: a tall tiled corridor
  receding to a vanishing point, a train filling one whole side as a wall,
  platform screen doors optional. It is NOT a cramped deep-tube bore.
- **Safe livery** (the project's own): a charcoal `#16161C` bodyshell, a sodium
  `#E0A030` cab band, violet `#6C4C9C` door surrounds, an original wayfinding
  typeface (not Johnston), a made-up operator mark. The silhouette says "modern
  London rail"; the dressing says "not TfL".
- **Off limits, do not lift:** the purple ELIZABETH LINE roundel on car sides
  or station totems; the TfL grey/white bodyshell with a single purple sole-bar
  stripe copied as a complete livery; New Johnston on destination blinds, car
  numbering or signage; the operator name or "MTR Elizabeth line" branding; the
  specific TfL purple used as the sole livery colour with nothing else (the
  project may use its own violet `#6C4C9C` as ONE of its four palette colours,
  e.g. door surrounds or a cab band, but a train that is simply purple-and-white
  and unmarked still reads as Elizabeth line and must be avoided).

Full physical detail is in `docs/reference/canary-wharf-research/rolling-stock.md`.

**Legal.** Unchanged and non-negotiable. Station names and geography are
factual and fine. No roundel. No Johnston or New Johnston. No reproduction of
the official line diagram. No operator livery or logo. No transcribed
announcement recordings. No Call of Duty weapon names, perk names, or asset
derivations. All wayfinding, advertising and rolling stock livery is original
work in an original identity. See `docs/art-direction.md`.

**Style.** British spelling throughout, including in code comments and
user-facing strings. Never use em-dashes.

---

## PART 2 - VERTICAL SLICE FIRST

`00_PROJECT_README.md` is right and this plan obeys it: do not build the
station before the loop works.

The first playable milestone is unchanged from that document:

> Player → weapon → one zombie → shooting → zombie death → points → round
> transition.

In a grey box room, with primitive geometry, no materials, and no atmosphere.
Everything in Part 3 phases 3 and later is forbidden until this runs.

The reason this matters more here than it did in the browser build: in UE it is
extremely easy to spend three weeks on a beautiful platform and discover the
combat feels wrong. The web build made that mistake in reverse and produced a
renderer with no game in it. Do not repeat it in the other direction.

---

## PART 3 - PHASES

Each phase has an acceptance test you can actually run. A phase is not done
because it compiles.

### Phase 0 - project and tooling

Create the UE5 project, first person template as a starting point only. Set up
the folder structure from `03_TECHNICAL_ARCHITECTURE.md`. Add the `LastTrain`
C++ module. Configure git with the UE gitignore and Git LFS for binary
content. Confirm the project opens and packages an empty build.

**Accept:** you can launch, walk around a default room, and produce a packaged
Windows build.

### Phase 1 - combat slice

C++: `ALTPlayerCharacter`, `ULTWeaponComponent`, `ULTWeaponData`,
`ALTZombieCharacter`, `ULTPointsComponent`, `ALTRoundManager` in its simplest
form. Hitscan with head and body hitboxes, head at 2.5x. Hip fire and ADS with
the spread model. One zombie that navigates to the player and attacks.

**Accept:** shoot a zombie, kill it, points increase, headshots pay more,
ADS is visibly tighter than hip fire, sprinting forces hip fire.

### Phase 2 - rounds and spawning

`ALTSpawnPoint`, wave composition, round transitions, the breather, increasing
count and health per round, live zombie cap.

**Accept:** rounds 1 to 5 play without touching the editor, difficulty rises
readably, the round ends exactly once.

### Phase 3 - classic economy

Wall buys, purchasable debris doors, the interaction framework via
`ULTInteractableInterface`, purchase validation and feedback, navigation
rebuild on door open.

**Accept:** earn points, buy a weapon off a wall, buy open a door, and the new
area is navigable by zombies.

### Phase 4 - grey box Canary Wharf

Block out the station with primitives only. Platform, train volume, concourse,
secondary platform or service area, escalator bank, traversal loops, spawn
routes. Use the v2 ASCII grid for Canary Wharf as the layout sketch.

**Accept:** the map is enjoyable to train zombies around using nothing but grey
boxes. If it is not fun grey, materials will not save it.

### Phase 5 - the train mechanic

The signature system, and the one thing that makes this not just another
Zombies map. Train arrives on a timer, dwells, doors animate, boarding ends the
round and travels. Departure board and platform announcements drive it
diegetically rather than as a HUD counter. Station heat when you stay.

**Accept:** a train arrives on schedule, you can board it during dwell, and
staying visibly raises pressure.

### Phase 6 - classic systems

Perks, the weapon upgrade bench, the lost property office as the mystery box
equivalent, downed and revive, equipment.

**Accept:** a complete survival session is possible, start to death.

### Phase 7 - art pass

Now, and not before. Modular kit, trim sheets, decals, clutter. Priority order
from `04_DEVELOPMENT_ROADMAP.md`: platform, train, signage, lighting, props,
concourse, secondary areas, dressing. Lumen tuning and the post process volume
happen here, and this is where the reference frame becomes the target.

**Accept:** a screenshot of the platform stands next to the reference frame
without embarrassment. Not equal. Not embarrassing.

### Phase 8 - audio

Ambience, train hum, original announcements, zombie vocals, weapon audio,
interaction, round stingers. Priority order from `01_GAME_VISION.md`. Silence
is a tool.

**Accept:** you can tell what is happening behind you with your eyes closed.

### Phase 9 - HUD

Restrained, per `08_CLASSIC_ZOMBIES_STYLE_GUIDE.md`. Round, points, health,
perks, weapon, magazine, reserve, equipment, minimal prompts. The station
network schematic as the map substitute. Explicitly not the reference frame's
HUD: no permanent minimap, no challenge tracker, no kill feed, no exfil
banner.

**Accept:** you can play well using only the information on screen, and nothing
on screen is decoration.

### Phase 10 - second station and travel

Only now does the line open up. A second station, adjacency, travel on the
train, per-station mechanics from the fixed library.

**Accept:** you can survive at Canary Wharf, board, and continue at a second
station with its round counter intact.

### Phase 11 - balance, polish, packaging

Balance to round 12 to 15 on a first serious run, 30 as an achievement. Profile
before optimising. Spawn fairness. Weapon feel.

---

## PART 4 - MODEL SPLIT

Revised for the loss of automated verification.

**Use Opus for:** C++ systems against an established convention, data assets,
Blueprint setup instructions, balance data, station layout sketches, HUD
widgets, documentation. Anything where being wrong produces a compile error or
an obviously broken behaviour you will notice in ten seconds.

**Use Fable for:** materials and shader graphs, Lumen and post process tuning,
animation blueprint state machines and blend logic, the crowd performance work
once there are 24 zombies and a profile, and any bug that survives two Opus
attempts. These are the cases where the failure is subtle, expensive to
diagnose, and does not announce itself.

**The rule that matters more than either:** one bounded task per request, with
current engine version, current project state, exact files involved, desired
behaviour, constraints, acceptance criteria, and test procedure. That is
`05_AI_PROMPT_LIBRARY.md` and it was written for exactly this situation. Never
ask for a phase in one prompt.

---

## PART 5 - RISKS

1. **No verification loop.** Every line of C++ here is unverified until you
   compile it. Keep changes small, compile often, commit working states.
2. **Asset creation is the real cost.** Phase 7 is longer than phases 0 to 6
   combined. Budget accordingly, and do not start it early.
3. **Trademark drift.** The reference frame is full of marks that cannot be
   used. Every art task needs the constraint restated or it will creep back in.
4. **Scope.** 41 stations was ambitious in a procedural browser build. Hand
   built in UE it is not realistic for one person. Treat the line as an
   expansion path, not a launch requirement. Two good stations beats forty
   grey ones. **v1 commits to exactly 2 stations.** Any "41 stations" reference
   anywhere in the docs is an aspirational expansion list, not a plan.
5. **The discarded build.** If UE proves too heavy, `phase-03` on `main` is a
   working, typechecked, browser-deployable fallback. That is worth remembering
   rather than resenting.

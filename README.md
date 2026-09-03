# Last Train

First person round-based zombie survival on a fictionalised London Underground
line, built in Unreal Engine 5. A station is the arena. A train arrives every
100 seconds and dwells for 25, and boarding it is an optional escape to an
adjacent station. Staying raises the station's heat.

Not affiliated with, endorsed by, or connected to Transport for London. No TfL
trademarks are used: no roundel, no official logo, no Johnston typeface, no
reproduction of the official line diagram, no recorded announcements. Station
names and geography are factual and used as such.

## Layout

```
LastTrain.uproject      Unreal Engine 5 project, the current target
Source/LastTrain/       C++ module: Player Weapons Zombies Rounds Economy Interaction
Content/                Unreal assets, Git LFS, see Content/README.md
Config/                 Engine and project configuration
docs/                   Briefs, plan, art direction, setup
tools/ci/               Static checks run by CI
web/                    Superseded Three.js build, kept as a fallback
```

The Unreal target is current. The web target under `web/` is the earlier
browser build, retained because it runs and is a viable fallback.

## Running the Unreal target

Unreal Engine 5.8. The `LastTrain` C++ module compiles. Follow
`docs/unreal-setup.md` and `docs/tasks/phase-a4-editor-setup.md` for the editor
work that creates the first playable map. `CLAUDE.md` has the build specifics.

## Running the web target

```bash
cd web
npm install
npm run dev
```

Needs a browser with WebGPU for the full pipeline; it falls back to WebGL2
automatically and thins the post-processing rather than disabling it.

```bash
npm run lint          # eslint, type aware
npm run format        # prettier
npm run typecheck     # tsc, strict
npm run build         # typecheck then production bundle
```

## Checks

CI runs on every push and pull request:

| Job | What it does |
|---|---|
| Web target | prettier check, eslint, tsc, vite build |
| Unreal source | clang-format check, then `tools/ci/check_cpp_conventions.py` for naming, `#pragma once`, generated header ordering, `TObjectPtr` in containers, unfinished markers and British spelling |
| Repository hygiene | `tools/ci/check_hygiene.py` for secrets, absolute paths and trademark leakage |

The engine is not available in CI, so the C++ cannot be compiled there. The
convention script catches what does not need a compiler.

Run everything locally:

```bash
cd web && npm run format:check && npm run lint && npm run typecheck && npx vite build
cd .. && python3 tools/ci/check_cpp_conventions.py && python3 tools/ci/check_hygiene.py
```

## Web target controls

| Input | Action |
|---|---|
| WASD | Move |
| Shift | Sprint, forces hip fire |
| Left mouse | Fire |
| Right mouse | Aim, tighter cone and slower movement |
| F | Torch |
| B | Blackout |
| G | Raise flood water one step |
| `[` `]` | Master brightness |
| 1 2 3 | Low, Medium, High preset |
| F2 | Debug fly camera, right drag to look, QE for height |
| F3 | Profiler overlay |
| F4 | Per stage GPU cost probe |
| F6 to F9 | Toggle SSAO, SSR, volumetrics, bloom |

## Current state

The project pivoted to Unreal Engine 5 and first person. See
`docs/brief-v3-unreal.md`.

**Unreal target.** Phase 1 combat slice written and compiling on UE 5.8: player,
weapon component with hip fire and aim down sights, zombie with head hitboxes
and round scaling, round manager, spawn points, points economy, interaction
interface. `Content/` is still empty, so the editor work in
`docs/tasks/phase-a4-editor-setup.md` is what produces the first playable map.
Current plan and status in `docs/tasks/README.md`.

**Web target.** Phases 2 and 3 complete, builds clean, browser playable. No
game loop: no rounds, no train, no HUD, no audio, no save, and the zombies
wander rather than hunting. What exists:

- **Renderer.** WebGPU primary with a WebGL2 fallback from one codebase. Full
  post chain in the planned order: SSAO, SSR, volumetric fog, bloom, ACES
  tonemap, motion blur, chromatic aberration, grain, vignette, sharpen. Three
  quality presets and a profiler with an A/B cost probe.
- **Lighting.** HDRI environment, emissive strip lighting on an eight-light
  budget that follows the camera, a real spotlight torch with a generated
  cookie and volumetric scattering, an ambient readability floor and a player
  fill light, plus a blackout switch and a master brightness setting.
- **Procedural architecture.** An ASCII grid becomes a station: extruded walls
  with skirting, platform lips with warning lines, trackbeds with rails and
  sleepers, splined tunnel mouths, stepped escalators, instanced props,
  signage, flood water, and a nav grid. Seeded and deterministic.
- **Crowd.** Six animations baked to vertex animation textures, three LOD
  buckets each rendered in one instanced draw, crossfaded clip transitions,
  torch-cone-gated shadows.
- **Gore.** Pooled projected blood decals and instanced particle bursts, one
  draw call each.
- **Combat basics.** Hip fire and aimed modes with real spread cones, recoil
  bloom, and a reticle sized to the actual cone.

## Documents

- `CLAUDE.md` — build specifics, module layout, conventions, model split
- `docs/tasks/` — the current plan and one bounded task per file
- `docs/brief-v3-unreal.md` — current brief: engine, camera, phases, model split
- `docs/brief-v2.md` — superseded for engine, still authoritative on game design
- `docs/art-direction.md` — palette, composition, and the trademark substitutions
- `docs/unreal-setup.md` — editor steps the source cannot do for you
- `docs/strategy.md` — phase history and the honest risk position
- `docs/reference/` — the reference frame and notes on what to lift from it
- `Content/ATTRIBUTION.md` — provenance and licence for every imported asset

## Licence

See `LICENSE`.

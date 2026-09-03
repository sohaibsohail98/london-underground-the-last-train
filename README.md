# Last Train

Round-based zombie survival on a fictionalised Elizabeth line, rendered in real
time 3D in the browser. A station is the arena. A train arrives every 100
seconds and dwells for 25, and boarding it is an optional escape to an adjacent
station. Staying raises the station's heat.

Not affiliated with, endorsed by, or connected to Transport for London. No TfL
trademarks are used: no roundel, no official logo, no Johnston typeface, no
reproduction of the official line diagram, no recorded announcements. Station
names and geography are factual and used as such.

## Running it

```bash
npm install
npm run dev
```

Needs a browser with WebGPU for the full pipeline. Without it the build falls
back to WebGL2 automatically and thins the post-processing rather than
disabling it.

```bash
npm run build       # strict typecheck, then production bundle
npm run typecheck   # typecheck only
```

## Controls

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

Phases 2 and 3 of the build plan are complete and merged. There is no game loop
yet: no rounds, no train, no weapons beyond a debug pistol, no HUD, no audio,
no save. The zombies wander rather than hunting.

What does exist:

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

- `docs/brief-v2.md` — the build brief, authoritative for game design
- `docs/phase-1-plan.md` — architecture and render graph plan
- `docs/strategy.md` — remaining phases, asset plan, and model split
- `docs/repo-workflow.md` — token scoping and git conventions
- `assets/README.md` — provenance and licence for every sourced binary

## Licence

See `LICENSE`.

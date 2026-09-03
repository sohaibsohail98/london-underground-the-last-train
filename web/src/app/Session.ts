/**
 * Session assembly. Owns the async boot sequence and hands back everything the
 * frame loop needs, so main is left with wiring and the loop itself.
 */

import { Clock } from '../engine/Clock';
import { Random } from '../engine/Random';
import type { CollisionWorld } from '../engine/Collision';
import { Aim } from '../systems/Aim';
import { GameRenderer } from '../render/Renderer';
import { Crowd } from '../render/crowd/Crowd';
import { bakeCrowd } from '../render/crowd/VatBaker';
import { Decals } from '../render/gore/Decals';
import { Particles } from '../render/gore/Particles';
import { FlyCamera } from '../render/debug/FlyCamera';
import { generateStation, type GeneratedStation } from '../gen/Generator';
import { validateStation, type StationDef } from '../data/schemas';
import type { MaterialSet } from '../render/Materials';
import { Overlay } from '../ui/Overlay';
import { DebugCrowd } from './DebugCrowd';
import { PlayerProxy } from './PlayerProxy';

const MATERIAL_SETS: MaterialSet[] = [
  'concrete',
  'tile',
  'wet_tile',
  'steel',
  'painted_brick',
  'rubber',
  'glass',
];

export interface Session {
  overlay: Overlay;
  renderer: GameRenderer;
  station: GeneratedStation;
  world: CollisionWorld;
  crowd: Crowd;
  debugCrowd: DebugCrowd;
  decals: Decals;
  particles: Particles;
  flyCamera: FlyCamera;
  detachFlyCamera: () => void;
  player: PlayerProxy;
  aim: Aim;
  clock: Clock;
  shotRandom: Random;
}

function requireElement<T extends HTMLElement>(id: string): T {
  const element = document.getElementById(id) as T | null;
  if (!element) throw new Error(`Boot failed: #${id} is missing from index.html`);
  return element;
}

/** Applies the current preset's crowd settings. Also called on preset change. */
export function syncCrowdToPreset(crowd: Crowd, renderer: GameRenderer): void {
  crowd.lodDistances = renderer.preset.crowdLodDistances;
  crowd.castShadows = renderer.preset.crowdShadows;
}

export async function createSession(def: StationDef): Promise<Session> {
  const overlayRoot = requireElement('overlay');
  const overlay = new Overlay(overlayRoot);
  const renderer = new GameRenderer(requireElement('view'), overlayRoot);

  await renderer.init((text) => overlay.setStatus(text));

  for (const issue of validateStation(def)) {
    console.warn(`[last-train] ${issue.station}: ${issue.message}`);
  }

  overlay.setStatus('Loading surfaces');
  await renderer.preloadMaterials(MATERIAL_SETS);

  overlay.setStatus('Lighting the station');
  await renderer.applyEnvironment({ ...def.hdri, surface: false });

  overlay.setStatus('Generating station geometry');
  const station = generateStation(def, renderer.materials, renderer.lighting, renderer.occlusion);
  renderer.scene.add(station.group);
  console.info('[last-train] generated', station.def.id, station.stats);

  const world: CollisionWorld = { grid: station.grid, nav: station.nav };

  overlay.setStatus('Baking crowd animation textures');
  const crowd = new Crowd(renderer.scene, renderer.renderer, await bakeCrowd());
  syncCrowdToPreset(crowd, renderer);

  const flyCamera = new FlyCamera(renderer.rig.camera);

  overlay.setStatus('Spawning');
  const debugCrowd = new DebugCrowd(crowd, station, world, new Random(0xc0ffee));
  debugCrowd.populate(renderer.preset.zombieCap);

  return {
    overlay,
    renderer,
    station,
    world,
    crowd,
    debugCrowd,
    decals: new Decals(renderer.scene),
    particles: new Particles(renderer.scene),
    flyCamera,
    detachFlyCamera: flyCamera.attach(window),
    player: new PlayerProxy(station.playerSpawn),
    aim: new Aim(),
    clock: new Clock(),
    shotRandom: new Random(0x5eed),
  };
}

export function disposeSession(session: Session): void {
  session.detachFlyCamera();
  session.particles.dispose();
  session.decals.dispose();
  session.crowd.dispose();
  session.station.dispose();
}

/**
 * Boot.
 *
 * Phase 3 target: a station generated entirely from an ASCII grid, with a full
 * crowd of animated zombies wandering it, gore on hit, and a debug fly camera
 * to inspect the geometry. There is still no game here: no rounds, no weapons,
 * no pathfinding toward the player. The wander behaviour and the sphere hit
 * test below are debug scaffolding and are replaced wholesale by the real
 * systems in Phase 4.
 */

import { Plane, Raycaster, Vector2, Vector3 } from 'three';
import { Clock, DT } from './engine/Clock';
import { Random } from './engine/Random';
import { floorHeightAt, moveCapsule, type CollisionWorld } from './engine/Collision';
import { Aim } from './systems/Aim';
import { GameRenderer } from './render/Renderer';
import { Crowd, type CrowdSlot } from './render/crowd/Crowd';
import { bakeCrowd } from './render/crowd/VatBaker';
import { Decals } from './render/gore/Decals';
import { Particles } from './render/gore/Particles';
import { FlyCamera } from './render/debug/FlyCamera';
import { generateStation, type GeneratedStation } from './gen/Generator';
import { debugYard } from './data/stations/debug-yard';
import { validateStation } from './data/schemas';
import { Overlay } from './ui/Overlay';
import { PRESET_ORDER, type PresetName } from './render/Presets';
import type { StageName } from './render/RenderGraph';

const canvas = document.getElementById('view') as HTMLCanvasElement | null;
const overlayRoot = document.getElementById('overlay');

if (!canvas || !overlayRoot) {
  throw new Error('Boot failed: the canvas or overlay root is missing from index.html');
}

const overlay = new Overlay(overlayRoot);
const renderer = new GameRenderer(canvas, overlayRoot);

/** Debug wander state, one per crowd slot. Replaced by Zombies.ts in Phase 4. */
interface Wanderer {
  slot: CrowdSlot;
  target: Vector3;
  speed: number;
  hp: number;
  dying: number;
}

const player = {
  position: new Vector3(),
  previous: new Vector3(),
  velocity: new Vector3(),
  interpolated: new Vector3(),
  health: 1,
  speed: 4.2,
  sprintSpeed: 6.8,
};

const keys = new Set<string>();
const pointer = new Vector2(0, 0);
const aim = new Vector3();
const raycaster = new Raycaster();
const floorPlane = new Plane(new Vector3(0, 1, 0), 0);
const clock = new Clock();
const random = new Random(0xc0ffee);
const aimSystem = new Aim();

/** Capsule radii. The player is slightly slimmer than a zombie. */
const PLAYER_RADIUS = 0.32;
const ZOMBIE_RADIUS = 0.36;

let torchOn = true;
let blackout = false;
let firing = false;
let aiming = false;
let fireCooldown = 0;
let floodHeight = 0;

function pickWanderTarget(station: GeneratedStation, out: Vector3): Vector3 {
  const { grid, nav } = station;

  // Rejection sampling rather than building a list of walkable cells: the
  // grids are open enough that this converges in a handful of attempts.
  for (let attempt = 0; attempt < 32; attempt += 1) {
    const x = random.int(0, grid.width - 1);
    const y = random.int(0, grid.height - 1);
    if (nav.walkable[y * grid.width + x] === 1) {
      return grid.worldPosition(x, y, out);
    }
  }

  return out.copy(station.playerSpawn);
}

/** Scale, tint and rate variation, which is most of what stops 46 clones. */
function variation(): Partial<CrowdSlot> {
  return {
    facing: random.range(0, Math.PI * 2),
    scale: random.range(0.9, 1.12),
    tint: new Vector3(
      random.range(0.72, 1.05),
      random.range(0.68, 0.95),
      random.range(0.66, 0.92),
    ),
    playbackRate: random.range(0.82, 1.18),
    timeOffset: random.range(0, 4),
  };
}

function spawnCrowd(crowd: Crowd, station: GeneratedStation, count: number): Wanderer[] {
  const wanderers: Wanderer[] = [];
  const position = new Vector3();

  for (let i = 0; i < count; i += 1) {
    pickWanderTarget(station, position);

    const slot = crowd.spawn(position, variation());
    if (!slot) break;

    crowd.play(slot, random.chance(0.35) ? 'walk' : 'shamble');

    wanderers.push({
      slot,
      target: pickWanderTarget(station, new Vector3()),
      speed: random.range(0.9, 1.9),
      hp: 100,
      dying: 0,
    });
  }

  return wanderers;
}

/** Debug wander: walk to a random walkable cell, pick another on arrival. */
function updateWanderers(
  wanderers: Wanderer[],
  crowd: Crowd,
  station: GeneratedStation,
  world: CollisionWorld,
  dt: number,
): void {
  const toTarget = new Vector3();

  for (const wanderer of wanderers) {
    const slot = wanderer.slot;
    if (!slot.active) continue;

    if (wanderer.dying > 0) {
      wanderer.dying -= dt;

      if (wanderer.dying <= 0) {
        // Recycled rather than left lying about, so the live instance count
        // stays at the figure the budget was tested against.
        crowd.release(slot);
        const fresh = crowd.spawn(pickWanderTarget(station, new Vector3()), variation());
        if (fresh) {
          wanderer.slot = fresh;
          wanderer.hp = 100;
          crowd.play(fresh, 'shamble');
          pickWanderTarget(station, wanderer.target);
        }
      }

      continue;
    }

    toTarget.copy(wanderer.target).sub(slot.position);
    toTarget.y = 0;

    const distance = toTarget.length();
    if (distance < 0.6) {
      pickWanderTarget(station, wanderer.target);
      continue;
    }

    toTarget.divideScalar(distance);

    // Zombies collide with the world too: nothing walks through a wall, and a
    // blocked wanderer repicks rather than grinding into the geometry.
    const step = wanderer.speed * dt;
    const blocked = moveCapsule(
      world,
      slot.position,
      toTarget.x * step,
      toTarget.z * step,
      ZOMBIE_RADIUS,
    );

    slot.position.y = floorHeightAt(world, slot.position.x, slot.position.z);
    slot.facing = Math.atan2(toTarget.x, toTarget.z);

    if (blocked) pickWanderTarget(station, wanderer.target);
  }
}

/** Debug hit test: nearest crowd slot within a thin cylinder about the shot. */
function resolveShot(
  wanderers: Wanderer[],
  crowd: Crowd,
  decals: Decals,
  particles: Particles,
  origin: Vector3,
  direction: Vector3,
): void {
  let best: Wanderer | null = null;
  let bestDistance = Infinity;

  const toZombie = new Vector3();

  for (const wanderer of wanderers) {
    if (!wanderer.slot.active || wanderer.dying > 0) continue;

    toZombie.copy(wanderer.slot.position).sub(origin);
    toZombie.y = 0;

    const along = toZombie.dot(direction);
    if (along <= 0 || along > 40) continue;

    const lateral = Math.hypot(
      toZombie.x - direction.x * along,
      toZombie.z - direction.z * along,
    );
    if (lateral > 0.45) continue;

    if (along < bestDistance) {
      bestDistance = along;
      best = wanderer;
    }
  }

  if (!best) return;

  const impact = best.slot.position.clone();
  impact.y = 1.05;

  crowd.flash(best.slot);
  particles.burst(impact, direction, 24);
  decals.placeOnFloor(impact);

  best.hp -= 45;

  if (best.hp <= 0) {
    crowd.play(best.slot, 'death', { hold: true });
    best.dying = 4;
  } else {
    crowd.play(best.slot, 'stagger', { rate: 1.4 });
  }
}

async function boot(): Promise<void> {
  await renderer.init((text) => overlay.setStatus(text));

  for (const issue of validateStation(debugYard)) {
    console.warn(`[last-train] ${issue.station}: ${issue.message}`);
  }

  overlay.setStatus('Loading surfaces');
  await renderer.preloadMaterials([
    'concrete',
    'tile',
    'wet_tile',
    'steel',
    'painted_brick',
    'rubber',
    'glass',
  ]);

  overlay.setStatus('Lighting the station');
  await renderer.applyEnvironment({
    file: debugYard.hdri.file,
    intensity: debugYard.hdri.intensity,
    tint: debugYard.hdri.tint,
    surface: false,
  });

  overlay.setStatus('Generating station geometry');
  const world: CollisionWorld = { grid: {} as never, nav: {} as never };
  const station = generateStation(
    debugYard,
    renderer.materials,
    renderer.lighting,
    renderer.occlusion,
  );
  renderer.scene.add(station.group);
  world.grid = station.grid;
  world.nav = station.nav;
  console.info('[last-train] generated', station.def.id, station.stats);

  overlay.setStatus('Baking crowd animation textures');
  const bake = await bakeCrowd();
  const crowd = new Crowd(renderer.scene, renderer.renderer, bake);
  crowd.lodDistances = renderer.preset.crowdLodDistances;
  crowd.castShadows = renderer.preset.crowdShadows;

  const decals = new Decals(renderer.scene);
  const particles = new Particles(renderer.scene);
  const flyCamera = new FlyCamera(renderer.rig.camera);
  const detachFly = flyCamera.attach(window);

  overlay.setStatus('Spawning');
  const wanderers = spawnCrowd(crowd, station, renderer.preset.zombieCap);

  player.position.copy(station.playerSpawn);
  player.previous.copy(station.playerSpawn);

  const onKeyDown = (event: KeyboardEvent): void => {
    keys.add(event.code);

    switch (event.code) {
      case 'KeyF':
        torchOn = !torchOn;
        renderer.torch.setEnabled(torchOn);
        break;
      case 'KeyB':
        blackout = !blackout;
        renderer.lighting.setBlackout(blackout);
        break;
      case 'KeyG':
        // Flood test: each press raises the water by roughly a round's worth.
        floodHeight = floodHeight >= 1.2 ? 0 : floodHeight + 0.3;
        station.water?.setHeight(floodHeight);
        break;
      case 'KeyH':
        player.health = Math.max(0, player.health - 0.22);
        overlay.flash(0.7);
        renderer.rig.addShake(0.5);
        break;
      case 'KeyJ':
        player.health = 1;
        break;
      case 'BracketLeft':
        renderer.setBrightness(renderer.brightness - 0.1);
        break;
      case 'BracketRight':
        renderer.setBrightness(renderer.brightness + 0.1);
        break;
      case 'Digit1':
      case 'Digit2':
      case 'Digit3': {
        const index = Number(event.code.slice(-1)) - 1;
        const name = PRESET_ORDER[index] as PresetName | undefined;
        if (name) {
          renderer.setPreset(name);
          crowd.lodDistances = renderer.preset.crowdLodDistances;
          crowd.castShadows = renderer.preset.crowdShadows;
        }
        break;
      }
      case 'F3':
        event.preventDefault();
        renderer.profiler.toggle();
        break;
      case 'F4':
        event.preventDefault();
        renderer.profiler.startProbe();
        break;
      default:
        break;
    }

    const stageKeys: Partial<Record<string, StageName>> = {
      F6: 'ssao',
      F7: 'ssr',
      F8: 'volumetric',
      F9: 'bloom',
    };
    const stage = stageKeys[event.code];
    if (stage) {
      event.preventDefault();
      renderer.setStageEnabled(stage, !renderer.graph.currentToggles[stage]);
    }
  };

  window.addEventListener('keydown', onKeyDown);
  window.addEventListener('keyup', (event) => keys.delete(event.code));
  window.addEventListener('pointermove', (event) => {
    pointer.x = (event.clientX / window.innerWidth) * 2 - 1;
    pointer.y = -(event.clientY / window.innerHeight) * 2 + 1;
  });
  window.addEventListener('pointerdown', (event) => {
    if (flyCamera.active) return;
    if (event.button === 0) firing = true;
    if (event.button === 2) aiming = true;
  });
  window.addEventListener('pointerup', (event) => {
    if (event.button === 0) firing = false;
    if (event.button === 2) aiming = false;
  });
  window.addEventListener('contextmenu', (event) => {
    if (!flyCamera.active) event.preventDefault();
  });
  window.addEventListener('blur', () => {
    keys.clear();
    firing = false;
    aiming = false;
    clock.resync();
  });

  renderer.torch.setEnabled(torchOn);
  overlay.dismissBoot();

  const shotDirection = new Vector3();
  const spreadDirection = new Vector3();
  const muzzle = new Vector3();

  const tick = (): void => {
    player.previous.copy(player.position);

    const forward = (keys.has('KeyW') ? 1 : 0) - (keys.has('KeyS') ? 1 : 0);
    const strafe = (keys.has('KeyD') ? 1 : 0) - (keys.has('KeyA') ? 1 : 0);
    const sprinting = keys.has('ShiftLeft') || keys.has('ShiftRight');

    player.velocity.set(strafe, 0, -forward);
    const movementFraction = player.velocity.lengthSq() > 0 ? (sprinting ? 1 : 0.6) : 0;

    aimSystem.update(DT, aiming, sprinting, movementFraction);

    if (player.velocity.lengthSq() > 0) {
      const speed = (sprinting ? player.sprintSpeed : player.speed) * aimSystem.moveScale;
      player.velocity.normalize().multiplyScalar(speed);
    }

    moveCapsule(
      world,
      player.position,
      player.velocity.x * DT,
      player.velocity.z * DT,
      PLAYER_RADIUS,
    );
    player.position.y = floorHeightAt(world, player.position.x, player.position.z);

    updateWanderers(wanderers, crowd, station, world, DT);

    if (fireCooldown > 0) fireCooldown -= DT;

    if (firing && fireCooldown <= 0) {
      fireCooldown = 0.12;

      muzzle.copy(player.position);
      muzzle.y = 1.3;
      shotDirection.copy(aim).sub(muzzle).setY(0);

      if (shotDirection.lengthSq() > 1e-6) {
        shotDirection.normalize();
        muzzle.addScaledVector(shotDirection, 0.55);

        // The shot goes where the cone says, not where the cursor says. Hip
        // fire is meaningfully less accurate than aiming, and holding the
        // trigger degrades both.
        aimSystem.applySpread(shotDirection, random, spreadDirection);
        aimSystem.registerShot();

        renderer.muzzle.fire(muzzle, 130, 0.05);
        renderer.rig.punch(aimSystem.mode === 'aimed' ? 1.1 : 2.2);
        resolveShot(wanderers, crowd, decals, particles, muzzle, spreadDirection);
      }
    }

    player.health = Math.min(1, player.health + DT * 0.06);
  };

  const frame = (nowMs: number): void => {
    const ticks = clock.advance(nowMs);
    for (let i = 0; i < ticks; i += 1) tick();

    player.interpolated.lerpVectors(player.previous, player.position, clock.alpha);

    raycaster.setFromCamera(pointer, renderer.rig.camera);
    if (!raycaster.ray.intersectPlane(floorPlane, aim)) {
      aim.copy(renderer.rig.camera.position).addScaledVector(raycaster.ray.direction, 40);
      aim.y = 0;
    }

    const dt = Math.max(clock.frameDelta, 1e-4);

    renderer.rig.fovOffset = aimSystem.fovOffset;
    renderer.rig.distanceScale = aimSystem.distanceScale;
    overlay.setReticle(aimSystem.spreadDeg, aimSystem.mode === 'aimed');

    station.update(dt);
    crowd.sync(dt, renderer.rig.camera.position, torchOn ? renderer.torch : null);
    decals.update(dt);
    particles.update(dt, renderer.rig.camera);
    overlay.update(renderer.graph, player.health, dt);

    renderer.profiler.setEntityCounts(
      crowd.counts.active + 1,
      crowd.counts.near + crowd.counts.mid + crowd.counts.far,
    );

    if (flyCamera.active) flyCamera.update(dt);
    renderer.render(clock, player.interpolated, aim, !flyCamera.active);

    window.requestAnimationFrame(frame);
  };

  window.requestAnimationFrame(frame);

  window.addEventListener('beforeunload', () => {
    detachFly();
    particles.dispose();
    decals.dispose();
    crowd.dispose();
    station.dispose();
  });
}

boot().catch((error: unknown) => {
  const message = error instanceof Error ? error.message : String(error);
  overlay.setStatus(`Boot failed: ${message}`);
  console.error('[last-train] boot failed', error);
});

/**
 * Boot.
 *
 * Phase 2 has no game systems, so this file stands in for them: it moves a
 * proxy capsule around the test room on a fixed 60Hz tick and feeds the
 * renderer interpolated positions, which is enough to judge the camera, the
 * torch and the whole post chain at Gate B.
 *
 * Everything here between the clock and the render call is replaced by the
 * real World and system order in Phase 4.
 */

import { Plane, Raycaster, Vector2, Vector3 } from 'three';
import { Clock, DT } from './engine/Clock';
import { GameRenderer } from './render/Renderer';
import { TestRoom, TEST_ROOM_SETS } from './render/debug/TestRoom';
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

/** Proxy player state. Replaced by World in Phase 4. */
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

let torchOn = true;
let blackout = false;
let firing = false;
let fireCooldown = 0;

function onKeyDown(event: KeyboardEvent): void {
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
    case 'KeyH':
      // Stands in for taking a hit until the health system exists.
      player.health = Math.max(0, player.health - 0.22);
      overlay.flash(0.7);
      renderer.rig.addShake(0.5);
      break;
    case 'KeyJ':
      player.health = 1;
      break;
    case 'Digit1':
    case 'Digit2':
    case 'Digit3': {
      const index = Number(event.code.slice(-1)) - 1;
      const name = PRESET_ORDER[index] as PresetName | undefined;
      if (name) renderer.setPreset(name);
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

  // F5 to F9 toggle the expensive stages individually, which is the quickest
  // way to judge what each one is contributing at Gate B.
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
}

function onKeyUp(event: KeyboardEvent): void {
  keys.delete(event.code);
}

function onPointerMove(event: PointerEvent): void {
  pointer.x = (event.clientX / window.innerWidth) * 2 - 1;
  pointer.y = -(event.clientY / window.innerHeight) * 2 + 1;
}

function updateAim(): void {
  raycaster.setFromCamera(pointer, renderer.rig.camera);
  const hit = raycaster.ray.intersectPlane(floorPlane, aim);
  if (!hit) {
    // Cursor above the horizon: aim far along the ray instead of nowhere.
    aim.copy(renderer.rig.camera.position).addScaledVector(raycaster.ray.direction, 40);
    aim.y = 0;
  }
}

/** One fixed simulation tick. Movement only, in Phase 2. */
function tick(room: TestRoom): void {
  player.previous.copy(player.position);

  const forward = (keys.has('KeyW') ? 1 : 0) - (keys.has('KeyS') ? 1 : 0);
  const strafe = (keys.has('KeyD') ? 1 : 0) - (keys.has('KeyA') ? 1 : 0);
  const sprinting = keys.has('ShiftLeft') || keys.has('ShiftRight');

  // Camera relative movement, since the camera never rotates in yaw this is
  // simply world space with z inverted.
  player.velocity.set(strafe, 0, -forward);
  if (player.velocity.lengthSq() > 0) {
    player.velocity.normalize().multiplyScalar(sprinting ? player.sprintSpeed : player.speed);
  }

  player.position.addScaledVector(player.velocity, DT);

  player.position.x = Math.min(Math.max(player.position.x, room.bounds.minX), room.bounds.maxX);
  player.position.z = Math.min(Math.max(player.position.z, room.bounds.minZ), room.bounds.maxZ);

  if (fireCooldown > 0) fireCooldown -= DT;

  if (firing && fireCooldown <= 0) {
    fireCooldown = 0.12;

    const muzzle = player.position.clone();
    muzzle.y = 1.3;
    const direction = aim.clone().sub(muzzle).setY(0);
    if (direction.lengthSq() > 1e-6) muzzle.addScaledVector(direction.normalize(), 0.55);

    renderer.muzzle.fire(muzzle, 130, 0.05);
    renderer.rig.punch(2.2);
  }

  // Slow regen, so the grain and vignette ramp can be watched recovering.
  player.health = Math.min(1, player.health + DT * 0.06);
}

async function boot(): Promise<void> {
  await renderer.init((text) => overlay.setStatus(text));

  overlay.setStatus('Loading surfaces');
  await renderer.preloadMaterials(TEST_ROOM_SETS);

  overlay.setStatus('Lighting the station');
  await renderer.applyEnvironment({
    file: 'sodium_interior.hdr',
    intensity: 0.2,
    tint: 0xe0a030,
    surface: false,
  });

  const room = new TestRoom(renderer.materials, renderer.lighting, renderer.occlusion);
  renderer.scene.add(room.group);

  player.position.copy(room.spawn);
  player.previous.copy(room.spawn);

  window.addEventListener('keydown', onKeyDown);
  window.addEventListener('keyup', onKeyUp);
  window.addEventListener('pointermove', onPointerMove);
  window.addEventListener('pointerdown', () => {
    firing = true;
  });
  window.addEventListener('pointerup', () => {
    firing = false;
  });
  window.addEventListener('blur', () => {
    keys.clear();
    firing = false;
    clock.resync();
  });

  renderer.torch.setEnabled(torchOn);
  renderer.profiler.setEntityCounts(1, 0);

  overlay.dismissBoot();

  const frame = (nowMs: number): void => {
    const ticks = clock.advance(nowMs);
    for (let i = 0; i < ticks; i += 1) tick(room);

    // Interpolate the proxy for rendering, exactly as the crowd sync will.
    player.interpolated.lerpVectors(player.previous, player.position, clock.alpha);
    room.playerProxy.position.set(
      player.interpolated.x,
      room.playerProxy.position.y,
      player.interpolated.z,
    );

    updateAim();
    overlay.update(renderer.graph, player.health, clock.frameDelta);
    renderer.render(clock, player.interpolated, aim);

    window.requestAnimationFrame(frame);
  };

  window.requestAnimationFrame(frame);
}

boot().catch((error: unknown) => {
  const message = error instanceof Error ? error.message : String(error);
  overlay.setStatus(`Boot failed: ${message}`);
  // Surfaced rather than swallowed: a failed backend init is the single most
  // likely thing to go wrong on unfamiliar hardware.
  console.error('[last-train] boot failed', error);
});

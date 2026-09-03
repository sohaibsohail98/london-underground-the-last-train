/**
 * Boot and frame loop. Assembly only: setup lives in app/Session, key bindings
 * in app/DebugKeys, and every system in its own module.
 */

import { Plane, Raycaster, Vector3 } from 'three';
import { DT } from './engine/Clock';
import { debugYard } from './data/stations/debug-yard';
import { bindInput } from './app/DebugInput';
import { createKeyHandler, type ToggleState } from './app/DebugKeys';
import { createSession, disposeSession } from './app/Session';

/** Distance to aim at when the cursor is above the horizon. */
const HORIZON_AIM_DISTANCE = 40;

async function boot(): Promise<void> {
  const session = await createSession(debugYard);
  const { overlay, renderer, station, world, crowd, debugCrowd } = session;
  const { decals, particles, flyCamera, player, aim, clock, shotRandom } = session;

  const toggles: ToggleState = { torchOn: true, blackout: false, floodHeight: 0 };
  const handleKey = createKeyHandler(session, toggles);

  const { state: input, detach: detachInput } = bindInput({
    onKey: handleKey,
    onBlur: () => clock.resync(),
    isSuspended: () => flyCamera.active,
  });

  renderer.torch.setEnabled(toggles.torchOn);
  overlay.dismissBoot();

  const raycaster = new Raycaster();
  const floorPlane = new Plane(new Vector3(0, 1, 0), 0);
  const aimPoint = new Vector3();
  const shotDirection = new Vector3();
  const spreadDirection = new Vector3();

  const tick = (): void => {
    aim.update(DT, input.aiming, input.sprinting, player.movementFraction);
    player.move(input, aim, world);
    debugCrowd.update(DT);

    shotDirection.copy(aimPoint).sub(player.position).setY(0).normalize();

    if (!player.tryFire(input.firing, shotDirection)) return;

    aim.applySpread(shotDirection, shotRandom, spreadDirection);
    aim.registerShot();

    renderer.muzzle.fire(player.muzzle, 130, 0.05);
    renderer.rig.punch(aim.mode === 'aimed' ? 1.1 : 2.2);
    debugCrowd.resolveShot(player.muzzle, spreadDirection, decals, particles);
  };

  const updateAimPoint = (): void => {
    raycaster.setFromCamera(input.pointer, renderer.rig.camera);
    if (raycaster.ray.intersectPlane(floorPlane, aimPoint)) return;

    aimPoint
      .copy(renderer.rig.camera.position)
      .addScaledVector(raycaster.ray.direction, HORIZON_AIM_DISTANCE)
      .setY(0);
  };

  const frame = (nowMs: number): void => {
    const ticks = clock.advance(nowMs);
    for (let i = 0; i < ticks; i += 1) tick();

    const focus = player.interpolate(clock.alpha);
    const dt = Math.max(clock.frameDelta, 1e-4);

    updateAimPoint();

    renderer.rig.fovOffset = aim.fovOffset;
    renderer.rig.distanceScale = aim.distanceScale;
    overlay.setReticle(aim.spreadDeg, aim.mode === 'aimed');

    station.update(dt);
    crowd.sync(dt, renderer.rig.camera.position, toggles.torchOn ? renderer.torch : null);
    decals.update(dt);
    particles.update(dt, renderer.rig.camera);
    overlay.update(renderer.graph, player.health, dt);

    renderer.profiler.setEntityCounts(
      crowd.counts.active + 1,
      crowd.counts.near + crowd.counts.mid + crowd.counts.far,
    );

    if (flyCamera.active) flyCamera.update(dt);
    renderer.render(clock, focus, aimPoint, !flyCamera.active);

    window.requestAnimationFrame(frame);
  };

  window.requestAnimationFrame(frame);

  window.addEventListener('beforeunload', () => {
    detachInput();
    disposeSession(session);
  });
}

boot().catch((error: unknown) => {
  const message = error instanceof Error ? error.message : String(error);
  document.getElementById('boot-status')?.replaceChildren(`Boot failed: ${message}`);
  console.error('[last-train] boot failed', error);
});

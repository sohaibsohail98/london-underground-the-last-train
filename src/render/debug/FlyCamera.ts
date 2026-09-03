/**
 * Debug fly camera.
 *
 * Drives the rig's own PerspectiveCamera rather than creating a second one,
 * because the render graph's scene pass is bound to a specific camera object
 * and swapping it would force a pipeline rebuild. While active, the game skips
 * the spring arm update and this writes the transform instead.
 *
 * Toggle with F2. Right mouse drag to look, WASD to move, Q and E for down and
 * up, Shift to boost.
 */

import { Euler, MathUtils, Vector3, type PerspectiveCamera } from 'three';

const BASE_SPEED = 8;
const BOOST_MULTIPLIER = 4;
const LOOK_SENSITIVITY = 0.0022;

export class FlyCamera {
  active = false;

  private readonly camera: PerspectiveCamera;
  private readonly euler = new Euler(0, 0, 0, 'YXZ');
  private readonly velocity = new Vector3();
  private readonly forward = new Vector3();
  private readonly right = new Vector3();
  private readonly worldUp = new Vector3(0, 1, 0);
  private readonly keys = new Set<string>();

  private looking = false;
  private savedPosition = new Vector3();
  private savedQuaternion = { x: 0, y: 0, z: 0, w: 1 };

  constructor(camera: PerspectiveCamera) {
    this.camera = camera;
  }

  attach(target: EventTarget = window): () => void {
    const onKeyDown = (event: Event): void => {
      const keyboard = event as KeyboardEvent;
      this.keys.add(keyboard.code);
      if (keyboard.code === 'F2') {
        keyboard.preventDefault();
        this.toggle();
      }
    };
    const onKeyUp = (event: Event): void => {
      this.keys.delete((event as KeyboardEvent).code);
    };
    const onPointerDown = (event: Event): void => {
      if (!this.active) return;
      if ((event as PointerEvent).button === 2) this.looking = true;
    };
    const onPointerUp = (): void => {
      this.looking = false;
    };
    const onPointerMove = (event: Event): void => {
      if (!this.active || !this.looking) return;
      const pointer = event as PointerEvent;
      this.euler.y -= pointer.movementX * LOOK_SENSITIVITY;
      this.euler.x = MathUtils.clamp(
        this.euler.x - pointer.movementY * LOOK_SENSITIVITY,
        -Math.PI / 2 + 0.01,
        Math.PI / 2 - 0.01,
      );
    };
    const onContextMenu = (event: Event): void => {
      if (this.active) event.preventDefault();
    };

    target.addEventListener('keydown', onKeyDown);
    target.addEventListener('keyup', onKeyUp);
    target.addEventListener('pointerdown', onPointerDown);
    target.addEventListener('pointerup', onPointerUp);
    target.addEventListener('pointermove', onPointerMove);
    target.addEventListener('contextmenu', onContextMenu);

    return () => {
      target.removeEventListener('keydown', onKeyDown);
      target.removeEventListener('keyup', onKeyUp);
      target.removeEventListener('pointerdown', onPointerDown);
      target.removeEventListener('pointerup', onPointerUp);
      target.removeEventListener('pointermove', onPointerMove);
      target.removeEventListener('contextmenu', onContextMenu);
    };
  }

  toggle(): void {
    this.active = !this.active;

    if (this.active) {
      // Take over from wherever the spring arm had the camera, so activating
      // it is never disorientating.
      this.savedPosition.copy(this.camera.position);
      this.savedQuaternion = { ...this.camera.quaternion };
      this.euler.setFromQuaternion(this.camera.quaternion, 'YXZ');
      this.euler.z = 0;
      return;
    }

    this.camera.position.copy(this.savedPosition);
    this.camera.quaternion.set(
      this.savedQuaternion.x,
      this.savedQuaternion.y,
      this.savedQuaternion.z,
      this.savedQuaternion.w,
    );
    this.looking = false;
  }

  /** Call instead of the rig update while active. */
  update(dt: number): void {
    if (!this.active) return;

    this.camera.quaternion.setFromEuler(this.euler);

    this.forward.set(0, 0, -1).applyQuaternion(this.camera.quaternion);
    this.right.set(1, 0, 0).applyQuaternion(this.camera.quaternion);

    const forwardInput = (this.keys.has('KeyW') ? 1 : 0) - (this.keys.has('KeyS') ? 1 : 0);
    const strafeInput = (this.keys.has('KeyD') ? 1 : 0) - (this.keys.has('KeyA') ? 1 : 0);
    const verticalInput = (this.keys.has('KeyE') ? 1 : 0) - (this.keys.has('KeyQ') ? 1 : 0);
    const boosting = this.keys.has('ShiftLeft') || this.keys.has('ShiftRight');

    this.velocity.set(0, 0, 0);
    this.velocity.addScaledVector(this.forward, forwardInput);
    this.velocity.addScaledVector(this.right, strafeInput);
    this.velocity.addScaledVector(this.worldUp, verticalInput);

    if (this.velocity.lengthSq() > 0) {
      this.velocity
        .normalize()
        .multiplyScalar(BASE_SPEED * (boosting ? BOOST_MULTIPLIER : 1) * dt);
      this.camera.position.add(this.velocity);
    }

    this.camera.updateMatrixWorld();
  }
}

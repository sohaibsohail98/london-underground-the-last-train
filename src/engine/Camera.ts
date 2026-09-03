/**
 * Steep angled third person camera. The rig is a spring arm: a damped follow
 * target plus a fixed pitch offset, so the camera lags the player slightly
 * without ever swinging. Everything here runs on the render side and is fed
 * interpolated positions, never raw simulation state.
 */

import { MathUtils, PerspectiveCamera, Raycaster, Vector2, Vector3, type Object3D } from 'three';

export interface CameraRigOptions {
  /** Downward pitch in degrees. 55 is the project default. */
  pitchDeg?: number;
  /** Spring arm length in metres. */
  distance?: number;
  /** Vertical offset of the look-at point above the player's feet. */
  targetHeight?: number;
  /** Follow damping, higher is snappier. Units are per second. */
  followLambda?: number;
  /** How far the camera leans toward the aim point, in metres. */
  lookaheadMax?: number;
}

const DEFAULTS: Required<CameraRigOptions> = {
  pitchDeg: 55,
  distance: 14,
  targetHeight: 1.1,
  followLambda: 9,
  lookaheadMax: 3.2,
};

/** Frame independent exponential smoothing. */
function damp(current: number, target: number, lambda: number, dt: number): number {
  return MathUtils.lerp(current, target, 1 - Math.exp(-lambda * dt));
}

export class CameraRig {
  readonly camera: PerspectiveCamera;

  /** Base field of view in degrees before any punch is applied. */
  baseFov = 62;

  /** Additive FOV offset, written by the aim system. Negative narrows. */
  fovOffset = 0;

  /** Spring arm length multiplier, written by the aim system. */
  distanceScale = 1;

  /** The point the rig is trying to frame, normally the interpolated player. */
  readonly focus = new Vector3();

  /** Where the player is aiming, used for lookahead. */
  readonly aim = new Vector3();

  private readonly options: Required<CameraRigOptions>;
  private readonly smoothFocus = new Vector3();
  private readonly smoothLookahead = new Vector3();
  private readonly offset = new Vector3();
  private readonly lookAt = new Vector3();
  private readonly shakeOffset = new Vector3();
  private readonly previousPosition = new Vector3();
  private readonly raycaster = new Raycaster();
  private readonly screenCentre = new Vector2(0, 0);

  private fovPunch = 0;
  private shake = 0;
  private shakeSeed = Math.random() * 1000;
  private initialised = false;

  /** Metres travelled by the camera last frame, drives camera motion blur. */
  velocityMagnitude = 0;

  /** 0 means fully visible, 1 means fully dithered away. */
  occlusionAmount = 0;

  constructor(aspect: number, options: CameraRigOptions = {}) {
    this.options = { ...DEFAULTS, ...options };
    this.camera = new PerspectiveCamera(this.baseFov, aspect, 0.1, 220);
    this.camera.position.set(0, this.options.distance, this.options.distance);
  }

  setAspect(aspect: number): void {
    this.camera.aspect = aspect;
    this.camera.updateProjectionMatrix();
  }

  /** Adds a one shot FOV kick, used on firing. Values around 1.5 to 4 read well. */
  punch(degrees: number): void {
    this.fovPunch = Math.min(this.fovPunch + degrees, 12);
  }

  /** Adds camera shake, used on taking damage. 0 to 1. */
  addShake(amount: number): void {
    this.shake = Math.min(this.shake + amount, 1);
  }

  /**
   * Evaluates the rig. Call once per rendered frame with the real frame delta,
   * after writing {@link CameraRig.focus} and {@link CameraRig.aim}.
   */
  update(dt: number): void {
    const o = this.options;

    if (!this.initialised) {
      this.smoothFocus.copy(this.focus);
      this.initialised = true;
    }

    this.smoothFocus.x = damp(this.smoothFocus.x, this.focus.x, o.followLambda, dt);
    this.smoothFocus.y = damp(this.smoothFocus.y, this.focus.y, o.followLambda * 1.6, dt);
    this.smoothFocus.z = damp(this.smoothFocus.z, this.focus.z, o.followLambda, dt);

    // Lookahead: lean a fraction of the way toward the aim point, clamped so a
    // distant cursor cannot drag the player out of frame.
    const leanX = MathUtils.clamp((this.aim.x - this.focus.x) * 0.35, -o.lookaheadMax, o.lookaheadMax);
    const leanZ = MathUtils.clamp((this.aim.z - this.focus.z) * 0.35, -o.lookaheadMax, o.lookaheadMax);
    this.smoothLookahead.x = damp(this.smoothLookahead.x, leanX, 5, dt);
    this.smoothLookahead.z = damp(this.smoothLookahead.z, leanZ, 5, dt);

    const pitch = MathUtils.degToRad(o.pitchDeg);
    const distance = o.distance * this.distanceScale;
    this.offset.set(0, Math.sin(pitch) * distance, Math.cos(pitch) * distance);

    this.lookAt.copy(this.smoothFocus);
    this.lookAt.y += o.targetHeight;
    this.lookAt.x += this.smoothLookahead.x;
    this.lookAt.z += this.smoothLookahead.z;

    // Decay the transient effects.
    this.fovPunch = damp(this.fovPunch, 0, 11, dt);
    this.shake = damp(this.shake, 0, 6, dt);

    const t = this.shakeSeed + performance.now() * 0.001;
    const s = this.shake * this.shake * 0.55;
    this.shakeOffset.set(
      Math.sin(t * 37.1) * s,
      Math.sin(t * 29.7) * s * 0.7,
      Math.sin(t * 41.3) * s,
    );

    this.previousPosition.copy(this.camera.position);

    this.camera.position.copy(this.lookAt).add(this.offset).add(this.shakeOffset);
    this.camera.lookAt(this.lookAt);

    this.velocityMagnitude = this.camera.position.distanceTo(this.previousPosition) / Math.max(dt, 1e-4);

    const targetFov = this.baseFov + this.fovPunch + this.fovOffset;
    if (Math.abs(this.camera.fov - targetFov) > 0.001) {
      this.camera.fov = targetFov;
      this.camera.updateProjectionMatrix();
    }
  }

  /**
   * Tests whether anything sits between the camera and the player. Returns the
   * hit objects so {@link Occlusion} can dither them out. Cheap: one ray from
   * the screen centre, only against the supplied occluder set.
   */
  queryOccluders(occluders: Object3D[], dt: number): Object3D[] {
    if (occluders.length === 0) {
      this.occlusionAmount = damp(this.occlusionAmount, 0, 8, dt);
      return [];
    }

    this.raycaster.setFromCamera(this.screenCentre, this.camera);
    const distanceToFocus = this.camera.position.distanceTo(this.lookAt);
    this.raycaster.far = distanceToFocus - 0.5;

    const hits = this.raycaster.intersectObjects(occluders, false);
    const objects: Object3D[] = [];
    for (const hit of hits) {
      if (!objects.includes(hit.object)) objects.push(hit.object);
    }

    this.occlusionAmount = damp(this.occlusionAmount, objects.length > 0 ? 1 : 0, 8, dt);
    return objects;
  }
}

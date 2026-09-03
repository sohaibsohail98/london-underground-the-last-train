/**
 * Camera occlusion. At a 55 degree pitch, walls and ceilings regularly end up
 * between the camera and the player. Rather than moving the camera, which
 * makes a top down arena feel unstable, the offending geometry is dithered
 * away. Materials created with occludable: true read the shared fade uniform
 * from the material library.
 *
 * Two mechanisms are combined: a global fade for ceilings, which are almost
 * always in the way and are simply faded whenever the camera is inside a
 * roofed region, and a per object fade for walls found by the camera ray.
 */

import { MathUtils, type Object3D } from 'three';
import type { CameraRig } from '../engine/Camera';
import type { MaterialLibrary } from './Materials';

export class Occlusion {
  /** Objects eligible for the ray test. The generator registers walls here. */
  readonly occluders: Object3D[] = [];

  /** How strongly occluders fade at full occlusion, 0..1. */
  strength = 0.86;

  private readonly materials: MaterialLibrary;
  private faded: Object3D[] = [];
  private current = 0;

  constructor(materials: MaterialLibrary) {
    this.materials = materials;
  }

  register(object: Object3D): void {
    if (!this.occluders.includes(object)) this.occluders.push(object);
  }

  clear(): void {
    this.restore();
    this.occluders.length = 0;
  }

  /** Runs the ray query and updates the shared fade uniform. */
  update(rig: CameraRig, dt: number): void {
    const hits = rig.queryOccluders(this.occluders, dt);

    if (hits.length !== this.faded.length || hits.some((object, i) => this.faded[i] !== object)) {
      this.restore();
      this.faded = hits;
      for (const object of hits) object.userData.occluding = true;
    }

    const target = hits.length > 0 ? this.strength : 0;
    this.current = MathUtils.lerp(this.current, target, 1 - Math.exp(-9 * dt));
    this.materials.occlusionFade.value = this.current;
  }

  private restore(): void {
    for (const object of this.faded) object.userData.occluding = false;
    this.faded = [];
  }

  get fade(): number {
    return this.current;
  }
}

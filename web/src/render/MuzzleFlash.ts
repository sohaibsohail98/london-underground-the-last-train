/**
 * Muzzle flash. A single frame of very high intensity point light plus a short
 * bloom boost. Cheap, and it does more for the feel of firing than any amount
 * of particle work, because it lights the actual geometry around the player.
 */

import { Color, PointLight, Scene, Vector3 } from 'three';
import { uniform } from 'three/tsl';

interface Flash {
  light: PointLight;
  /** Remaining life in seconds. */
  life: number;
  duration: number;
  peak: number;
}

const POOL_SIZE = 4;

export class MuzzleFlash {
  /** Additive bloom boost in 0..1, read by the bloom pass. */
  readonly bloomBoost = uniform(0);

  private readonly flashes: Flash[] = [];
  private readonly scene: Scene;

  constructor(scene: Scene) {
    this.scene = scene;

    for (let i = 0; i < POOL_SIZE; i += 1) {
      const light = new PointLight(0xffd9a0, 0, 9, 2);
      light.castShadow = false;
      light.visible = false;
      scene.add(light);
      this.flashes.push({ light, life: 0, duration: 0.06, peak: 0 });
    }
  }

  /**
   * Fires a flash. Intensity around 90 to 160 suits a pistol, higher for a
   * shotgun. Duration is deliberately shorter than one frame at 60fps for
   * small arms, so it reads as a snap rather than a glow.
   */
  fire(position: Vector3, intensity = 120, duration = 0.05, colour = 0xffd9a0): void {
    // Reuse the flash with the least life left rather than skipping the shot.
    let chosen = this.flashes[0];
    for (const flash of this.flashes) {
      if (flash.life < chosen.life) chosen = flash;
    }

    chosen.light.position.copy(position);
    chosen.light.color.set(new Color(colour));
    chosen.light.intensity = intensity;
    chosen.light.visible = true;
    chosen.life = duration;
    chosen.duration = duration;
    chosen.peak = intensity;
  }

  update(dt: number): void {
    let boost = 0;

    for (const flash of this.flashes) {
      if (flash.life <= 0) continue;

      flash.life -= dt;

      if (flash.life <= 0) {
        flash.light.visible = false;
        flash.light.intensity = 0;
        continue;
      }

      const t = flash.life / flash.duration;
      // Sharp attack, fast decay.
      const curve = t * t;
      flash.light.intensity = flash.peak * curve;
      boost = Math.max(boost, curve);
    }

    this.bloomBoost.value = boost;
  }

  dispose(): void {
    for (const flash of this.flashes) {
      this.scene.remove(flash.light);
      flash.light.dispose();
    }
    this.flashes.length = 0;
  }
}

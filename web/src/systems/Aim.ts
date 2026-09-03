/**
 * Aiming.
 *
 * Two modes, and the difference between them is meant to be a real decision
 * rather than a cosmetic zoom:
 *
 *   Hip fire  — wide spread, full movement speed, camera pulled back for
 *               awareness. This is the mode for a horde at close range, where
 *               you need to see what is behind you more than you need
 *               precision.
 *
 *   Aimed     — tight spread, movement cut to a walk, camera pushed in and
 *               narrowed toward the cursor. This is the mode for headshots at
 *               range, and it deliberately costs you mobility.
 *
 * Spread has three contributions: the mode's base cone, a movement penalty,
 * and recoil bloom that accumulates per shot and decays when you stop firing.
 * Holding the trigger on an automatic weapon should stop being accurate.
 */

import { MathUtils, Vector3 } from 'three';
import type { Random } from '../engine/Random';

export type AimMode = 'hip' | 'aimed';

export interface AimTuning {
  /** Base cone half angle in degrees, before movement or bloom. */
  hipSpreadDeg: number;
  aimedSpreadDeg: number;
  /** Extra degrees at full movement speed. */
  moveSpreadDeg: number;
  /** Extra degrees added per shot fired. */
  bloomPerShotDeg: number;
  /** Ceiling on accumulated bloom, in degrees. */
  bloomMaxDeg: number;
  /** Bloom decay in degrees per second once firing stops. */
  bloomDecayDeg: number;
  /** Movement multiplier while aimed. */
  aimedMoveScale: number;
  /** Seconds to transition between modes. */
  transitionSeconds: number;
  /** FOV change while fully aimed, in degrees. Negative narrows. */
  aimedFovOffset: number;
  /** Spring arm length multiplier while fully aimed. */
  aimedDistanceScale: number;
  /** Spring arm length multiplier while hip firing. */
  hipDistanceScale: number;
}

export const DEFAULT_AIM: AimTuning = {
  hipSpreadDeg: 5.4,
  aimedSpreadDeg: 0.9,
  moveSpreadDeg: 2.6,
  bloomPerShotDeg: 0.85,
  bloomMaxDeg: 6,
  bloomDecayDeg: 7,
  aimedMoveScale: 0.52,
  transitionSeconds: 0.16,
  aimedFovOffset: -9,
  aimedDistanceScale: 0.82,
  hipDistanceScale: 1,
};

export class Aim {
  mode: AimMode = 'hip';

  /** 0 fully hip, 1 fully aimed. Smoothed, so the camera does not snap. */
  blend = 0;

  /** Accumulated recoil bloom in degrees. */
  bloom = 0;

  /** Current total spread half angle in degrees, for the reticle. */
  spreadDeg = DEFAULT_AIM.hipSpreadDeg;

  private readonly tuning: AimTuning;

  constructor(tuning: AimTuning = DEFAULT_AIM) {
    this.tuning = tuning;
  }

  /** Movement speed multiplier the movement system should apply. */
  get moveScale(): number {
    return MathUtils.lerp(1, this.tuning.aimedMoveScale, this.blend);
  }

  /** FOV offset in degrees for the camera rig. */
  get fovOffset(): number {
    return this.tuning.aimedFovOffset * this.blend;
  }

  /** Spring arm length multiplier for the camera rig. */
  get distanceScale(): number {
    return MathUtils.lerp(this.tuning.hipDistanceScale, this.tuning.aimedDistanceScale, this.blend);
  }

  /** Sprinting forces hip fire: you cannot aim at a run. */
  update(dt: number, wantsAimed: boolean, sprinting: boolean, movementFraction: number): void {
    this.mode = wantsAimed && !sprinting ? 'aimed' : 'hip';

    const target = this.mode === 'aimed' ? 1 : 0;
    const rate = dt / Math.max(this.tuning.transitionSeconds, 1e-3);
    this.blend = MathUtils.clamp(this.blend + (target - this.blend > 0 ? rate : -rate), 0, 1);

    this.bloom = Math.max(0, this.bloom - this.tuning.bloomDecayDeg * dt);

    const base = MathUtils.lerp(this.tuning.hipSpreadDeg, this.tuning.aimedSpreadDeg, this.blend);
    const movePenalty = this.tuning.moveSpreadDeg * MathUtils.clamp(movementFraction, 0, 1);

    this.spreadDeg = base + movePenalty + this.bloom;
  }

  /** Called once per shot fired. */
  registerShot(): void {
    this.bloom = Math.min(this.tuning.bloomMaxDeg, this.bloom + this.tuning.bloomPerShotDeg);
  }

  /**
   * Jitters a shot direction inside the current cone. The distribution is
   * uniform over the disc rather than gaussian, which is what makes a shotgun
   * pattern read correctly when the same function is used for pellets.
   */
  applySpread(direction: Vector3, random: Random, out = new Vector3()): Vector3 {
    return coneJitter(direction, this.spreadDeg, random, out);
  }
}

/** Shared cone jitter, also used by the shotgun pellet loop in Phase 5. */
export function coneJitter(
  direction: Vector3,
  spreadDeg: number,
  random: Random,
  out = new Vector3(),
): Vector3 {
  out.copy(direction).normalize();

  if (spreadDeg <= 0) return out;

  const maxAngle = MathUtils.degToRad(spreadDeg);
  // Square root of a uniform variable gives a uniform density over the disc.
  const angle = maxAngle * Math.sqrt(random.next());
  const roll = random.range(0, Math.PI * 2);

  // Build an orthonormal basis about the shot direction.
  const helper = Math.abs(out.y) < 0.9 ? new Vector3(0, 1, 0) : new Vector3(1, 0, 0);
  const right = new Vector3().crossVectors(out, helper).normalize();
  const up = new Vector3().crossVectors(right, out).normalize();

  const lateral = Math.tan(angle);
  out
    .addScaledVector(right, Math.cos(roll) * lateral)
    .addScaledVector(up, Math.sin(roll) * lateral)
    .normalize();

  return out;
}

/**
 * Debug player. A capsule with health and a pistol cadence, enough to drive
 * the camera, torch and aim model. Replaced by systems/Movement, Weapons and
 * Health in Phase 4.
 */

import { Vector3 } from 'three';
import { DT } from '../engine/Clock';
import { floorHeightAt, moveCapsule, type CollisionWorld } from '../engine/Collision';
import type { Aim } from '../systems/Aim';

export const PLAYER_RADIUS = 0.32;

const WALK_SPEED = 4.2;
const SPRINT_SPEED = 6.8;
const FIRE_INTERVAL = 0.12;
const MUZZLE_HEIGHT = 1.3;
const MUZZLE_FORWARD = 0.55;
const REGEN_PER_SECOND = 0.06;

export interface MoveIntent {
  forward: number;
  strafe: number;
  sprinting: boolean;
}

export class PlayerProxy {
  readonly position = new Vector3();
  readonly previous = new Vector3();
  readonly interpolated = new Vector3();
  readonly muzzle = new Vector3();

  health = 1;

  private readonly velocity = new Vector3();
  private fireCooldown = 0;

  constructor(spawn: Vector3) {
    this.position.copy(spawn);
    this.previous.copy(spawn);
    this.interpolated.copy(spawn);
  }

  /** 0 stationary, 1 at full speed. Feeds the aim spread penalty. */
  get movementFraction(): number {
    return this.velocity.lengthSq() > 0 ? 1 : 0;
  }

  damage(amount: number): void {
    this.health = Math.max(0, this.health - amount);
  }

  heal(): void {
    this.health = 1;
  }

  /** One fixed tick of movement. */
  move(intent: MoveIntent, aim: Aim, world: CollisionWorld): void {
    this.previous.copy(this.position);
    this.velocity.set(intent.strafe, 0, -intent.forward);

    if (this.velocity.lengthSq() > 0) {
      const speed = (intent.sprinting ? SPRINT_SPEED : WALK_SPEED) * aim.moveScale;
      this.velocity.normalize().multiplyScalar(speed);
    }

    moveCapsule(world, this.position, this.velocity.x * DT, this.velocity.z * DT, PLAYER_RADIUS);
    this.position.y = floorHeightAt(world, this.position.x, this.position.z);

    this.health = Math.min(1, this.health + DT * REGEN_PER_SECOND);
  }

  /**
   * Advances the fire timer and reports whether a shot leaves the barrel this
   * tick, writing the muzzle position as a side effect.
   */
  tryFire(firing: boolean, direction: Vector3): boolean {
    this.fireCooldown = Math.max(0, this.fireCooldown - DT);

    if (!firing || this.fireCooldown > 0 || direction.lengthSq() < 1e-6) return false;

    this.fireCooldown = FIRE_INTERVAL;
    this.muzzle.copy(this.position).setY(MUZZLE_HEIGHT).addScaledVector(direction, MUZZLE_FORWARD);

    return true;
  }

  /** Interpolates for rendering, exactly as the crowd sync does. */
  interpolate(alpha: number): Vector3 {
    return this.interpolated.lerpVectors(this.previous, this.position, alpha);
  }
}

/**
 * Debug crowd behaviour. Scaffolding only: wanders to random walkable cells so
 * the VAT crowd, LOD and collision can be judged. Replaced by systems/Zombies
 * in Phase 4.
 */

import { Vector3 } from 'three';
import { moveCapsule, floorHeightAt, type CollisionWorld } from '../engine/Collision';
import type { Random } from '../engine/Random';
import type { Crowd, CrowdSlot } from '../render/crowd/Crowd';
import type { Decals } from '../render/gore/Decals';
import type { Particles } from '../render/gore/Particles';
import type { GeneratedStation } from '../gen/Generator';

export const ZOMBIE_RADIUS = 0.36;

const HIT_DAMAGE = 45;
const CORPSE_SECONDS = 4;
const ARRIVAL_DISTANCE = 0.6;
const SHOT_RANGE = 40;
const SHOT_LATERAL = 0.45;

interface Wanderer {
  slot: CrowdSlot;
  target: Vector3;
  speed: number;
  hp: number;
  dying: number;
}

const scratch = new Vector3();

export class DebugCrowd {
  private readonly wanderers: Wanderer[] = [];

  constructor(
    private readonly crowd: Crowd,
    private readonly station: GeneratedStation,
    private readonly world: CollisionWorld,
    private readonly random: Random,
  ) {}

  get count(): number {
    return this.wanderers.length;
  }

  /** Fills the crowd to the given size. */
  populate(count: number): void {
    const position = new Vector3();

    for (let i = 0; i < count; i += 1) {
      const slot = this.crowd.spawn(this.pickTarget(position), this.variation());
      if (!slot) break;

      this.crowd.play(slot, this.random.chance(0.35) ? 'walk' : 'shamble');

      this.wanderers.push({
        slot,
        target: this.pickTarget(new Vector3()),
        speed: this.random.range(0.9, 1.9),
        hp: 100,
        dying: 0,
      });
    }
  }

  update(dt: number): void {
    for (const wanderer of this.wanderers) {
      if (!wanderer.slot.active) continue;

      if (wanderer.dying > 0) {
        this.updateDying(wanderer, dt);
        continue;
      }

      this.updateWalking(wanderer, dt);
    }
  }

  /** Nearest slot inside a thin cylinder about the shot. */
  resolveShot(origin: Vector3, direction: Vector3, decals: Decals, particles: Particles): void {
    const target = this.findTarget(origin, direction);
    if (!target) return;

    const impact = target.slot.position.clone().setY(1.05);

    this.crowd.flash(target.slot);
    particles.burst(impact, direction, 24);
    decals.placeOnFloor(impact);

    target.hp -= HIT_DAMAGE;

    if (target.hp <= 0) {
      this.crowd.play(target.slot, 'death', { hold: true });
      target.dying = CORPSE_SECONDS;
    } else {
      this.crowd.play(target.slot, 'stagger', { rate: 1.4 });
    }
  }

  private findTarget(origin: Vector3, direction: Vector3): Wanderer | null {
    let best: Wanderer | null = null;
    let bestDistance = Infinity;

    for (const wanderer of this.wanderers) {
      if (!wanderer.slot.active || wanderer.dying > 0) continue;

      scratch.copy(wanderer.slot.position).sub(origin).setY(0);

      const along = scratch.dot(direction);
      if (along <= 0 || along > SHOT_RANGE || along >= bestDistance) continue;

      const lateral = Math.hypot(scratch.x - direction.x * along, scratch.z - direction.z * along);
      if (lateral > SHOT_LATERAL) continue;

      bestDistance = along;
      best = wanderer;
    }

    return best;
  }

  private updateWalking(wanderer: Wanderer, dt: number): void {
    const slot = wanderer.slot;

    scratch.copy(wanderer.target).sub(slot.position).setY(0);
    const distance = scratch.length();

    if (distance < ARRIVAL_DISTANCE) {
      this.pickTarget(wanderer.target);
      return;
    }

    scratch.divideScalar(distance);

    const step = wanderer.speed * dt;
    const blocked = moveCapsule(
      this.world,
      slot.position,
      scratch.x * step,
      scratch.z * step,
      ZOMBIE_RADIUS,
    );

    slot.position.y = floorHeightAt(this.world, slot.position.x, slot.position.z);
    slot.facing = Math.atan2(scratch.x, scratch.z);

    if (blocked) this.pickTarget(wanderer.target);
  }

  /** Corpses are recycled so the live instance count stays at the tested figure. */
  private updateDying(wanderer: Wanderer, dt: number): void {
    wanderer.dying -= dt;
    if (wanderer.dying > 0) return;

    this.crowd.release(wanderer.slot);

    const fresh = this.crowd.spawn(this.pickTarget(new Vector3()), this.variation());
    if (!fresh) return;

    wanderer.slot = fresh;
    wanderer.hp = 100;
    this.crowd.play(fresh, 'shamble');
    this.pickTarget(wanderer.target);
  }

  /** Rejection sampling: the grids are open enough to converge immediately. */
  private pickTarget(out: Vector3): Vector3 {
    const { grid, nav } = this.station;

    for (let attempt = 0; attempt < 32; attempt += 1) {
      const x = this.random.int(0, grid.width - 1);
      const y = this.random.int(0, grid.height - 1);
      if (nav.walkable[y * grid.width + x] === 1) {
        return grid.worldPosition(x, y, out);
      }
    }

    return out.copy(this.station.playerSpawn);
  }

  /** Scale, tint and rate variation, which is most of what stops 46 clones. */
  private variation(): Partial<CrowdSlot> {
    const r = this.random;

    return {
      facing: r.range(0, Math.PI * 2),
      scale: r.range(0.9, 1.12),
      tint: new Vector3(r.range(0.72, 1.05), r.range(0.68, 0.95), r.range(0.66, 0.92)),
      playbackRate: r.range(0.82, 1.18),
      timeOffset: r.range(0, 4),
    };
  }
}

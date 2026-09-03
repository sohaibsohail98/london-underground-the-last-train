/**
 * Collision.
 *
 * The world is solid: nothing walks through a wall, and nothing walks off a
 * platform edge into the trackbed. Resolution is against the nav grid rather
 * than the generated triangles, because the grid is what the geometry was
 * emitted from, so a wall in the grid and a wall on screen are the same wall
 * by construction.
 *
 * Movement resolves per axis so that sliding along a wall works: if the
 * combined move is blocked, X and Z are retried separately and whichever
 * survives is kept. That is what stops the player sticking on corners, which
 * is the single most noticeable collision fault in a game with this camera.
 */

import { Vector3 } from 'three';
import { LEGEND, TILE, TRACK_DROP } from '../data/legend';
import type { Grid } from '../gen/Grid';
import type { NavGrid } from '../gen/Generator';

/** Rise per escalator step, matching the escalator emitter. */
const STEP_RISE = 0.22;

/** How far a capsule centre is kept from a blocked cell's edge. */
const SKIN = 0.05;

export interface CollisionWorld {
  grid: Grid;
  nav: NavGrid;
}

function walkableAt(world: CollisionWorld, worldX: number, worldZ: number): boolean {
  const cell = world.grid.gridFromWorld(worldX, worldZ);
  return world.nav.walkable[cell.y * world.grid.width + cell.x] === 1;
}

/**
 * True if a capsule of the given radius centred here would overlap a blocked
 * cell. Four corner probes rather than a full circle sweep: at a 1.5m tile and
 * a 0.34m radius the corners are the only samples that can differ.
 */
export function capsuleFits(
  world: CollisionWorld,
  position: Vector3,
  radius: number,
): boolean {
  const r = radius + SKIN;

  return (
    walkableAt(world, position.x, position.z) &&
    walkableAt(world, position.x + r, position.z) &&
    walkableAt(world, position.x - r, position.z) &&
    walkableAt(world, position.x, position.z + r) &&
    walkableAt(world, position.x, position.z - r) &&
    walkableAt(world, position.x + r * 0.7, position.z + r * 0.7) &&
    walkableAt(world, position.x - r * 0.7, position.z + r * 0.7) &&
    walkableAt(world, position.x + r * 0.7, position.z - r * 0.7) &&
    walkableAt(world, position.x - r * 0.7, position.z - r * 0.7)
  );
}

const candidate = new Vector3();

/**
 * Moves a capsule by a delta, resolving against the world. Writes the result
 * back into position and returns true if any part of the move was blocked, so
 * the caller can kill velocity or play a scuff.
 */
export function moveCapsule(
  world: CollisionWorld,
  position: Vector3,
  deltaX: number,
  deltaZ: number,
  radius: number,
): boolean {
  if (deltaX === 0 && deltaZ === 0) return false;

  candidate.set(position.x + deltaX, position.y, position.z + deltaZ);

  if (capsuleFits(world, candidate, radius)) {
    position.x = candidate.x;
    position.z = candidate.z;
    return false;
  }

  let blocked = true;

  // Slide along X.
  candidate.set(position.x + deltaX, position.y, position.z);
  if (deltaX !== 0 && capsuleFits(world, candidate, radius)) {
    position.x = candidate.x;
    blocked = true;
  }

  // Slide along Z.
  candidate.set(position.x, position.y, position.z + deltaZ);
  if (deltaZ !== 0 && capsuleFits(world, candidate, radius)) {
    position.z = candidate.z;
  }

  return blocked;
}

/**
 * Floor height at a world position. Flat everywhere except escalator runs,
 * where it ramps linearly across the run so the player and the crowd walk up
 * the steps rather than through them. Water does not change the floor height;
 * the flood mechanic raises the water surface over it instead.
 */
export function floorHeightAt(world: CollisionWorld, worldX: number, worldZ: number): number {
  const { grid } = world;
  const cell = grid.gridFromWorld(worldX, worldZ);
  const char = grid.charAt(cell.x, cell.y);

  if (char !== LEGEND.ESCALATOR) return 0;

  // Walk the run to find its extent along whichever axis it is longer in.
  let minX = cell.x;
  let maxX = cell.x;
  let minY = cell.y;
  let maxY = cell.y;

  while (grid.charAt(minX - 1, cell.y) === LEGEND.ESCALATOR) minX -= 1;
  while (grid.charAt(maxX + 1, cell.y) === LEGEND.ESCALATOR) maxX += 1;
  while (grid.charAt(cell.x, minY - 1) === LEGEND.ESCALATOR) minY -= 1;
  while (grid.charAt(cell.x, maxY + 1) === LEGEND.ESCALATOR) maxY += 1;

  const spanX = maxX - minX;
  const spanY = maxY - minY;

  const alongZ = spanY >= spanX;
  const steps = (alongZ ? spanY : spanX) + 1;
  if (steps <= 1) return 0;

  const start = new Vector3();
  grid.worldPosition(alongZ ? cell.x : minX, alongZ ? minY : cell.y, start);

  const travelled = alongZ ? worldZ - start.z : worldX - start.x;
  const fraction = Math.min(1, Math.max(0, travelled / (steps * TILE)));

  return fraction * steps * STEP_RISE;
}

/** Depth of the trackbed below the platform, for zombies climbing out. */
export const TRACKBED_DEPTH = TRACK_DROP;

/**
 * Prop placement rules.
 *
 * Every rule is a function of the grid plus a seeded stream, never of anything
 * that changes at runtime, so a station's dressing is identical every time it
 * is built. Jitter is applied within stated bounds: enough to break the grid
 * up, never enough to push a prop through a wall.
 */

import { Matrix4, Quaternion, Vector3 } from 'three';
import { LEGEND, TILE } from '../data/legend';
import type { Random } from '../engine/Random';
import { ORTHOGONAL, type Cell, type Grid } from './Grid';

export const JITTER = {
  position: 0.2 * TILE,
  yawDegrees: 6,
  scale: 0.05,
} as const;

/** Builds a transform with seeded jitter applied. */
export function jitteredMatrix(
  position: Vector3,
  yaw: number,
  random: Random,
  jitterPosition = true,
): Matrix4 {
  const offset = jitterPosition
    ? new Vector3(random.jitter(JITTER.position), 0, random.jitter(JITTER.position))
    : new Vector3();

  const jitterYaw = (random.jitter(JITTER.yawDegrees) * Math.PI) / 180;
  const uniformScale = 1 + random.jitter(JITTER.scale);

  return new Matrix4().compose(
    position.clone().add(offset),
    new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), yaw + jitterYaw),
    new Vector3(uniformScale, uniformScale, uniformScale),
  );
}

/**
 * The direction from an open cell toward its nearest adjacent wall, as a yaw
 * in radians, or null if the cell is not against a wall. Props that mount on
 * walls use this so they always face into the room.
 */
export function wallFacing(grid: Grid, cell: Cell): number | null {
  for (const [dx, dy] of ORTHOGONAL) {
    if (grid.isSolid(cell.x + dx, cell.y + dy)) {
      // Face away from the wall.
      return Math.atan2(-dx, -dy);
    }
  }
  return null;
}

/** The offset from a cell centre to sit flush against its adjacent wall. */
export function wallOffset(grid: Grid, cell: Cell, inset = 0.18): Vector3 {
  for (const [dx, dy] of ORTHOGONAL) {
    if (grid.isSolid(cell.x + dx, cell.y + dy)) {
      return new Vector3(dx, 0, dy).multiplyScalar(TILE / 2 - inset);
    }
  }
  return new Vector3();
}

export function isWallAdjacent(grid: Grid, cell: Cell): boolean {
  return ORTHOGONAL.some(([dx, dy]) => grid.isSolid(cell.x + dx, cell.y + dy));
}

/** Open cells against a wall, in reading order, for a given region. */
export function wallAdjacentCells(grid: Grid, region: Cell[]): Cell[] {
  return region.filter((cell) => cell.char === LEGEND.FLOOR && isWallAdjacent(grid, cell));
}

/**
 * Thins a list to roughly one entry every n tiles of travel, which is how the
 * bench, bin and ad frame rules are expressed in the plan.
 */
export function everyNth<T>(items: T[], n: number, offset = 0): T[] {
  const result: T[] = [];
  for (let i = offset; i < items.length; i += n) result.push(items[i]);
  return result;
}

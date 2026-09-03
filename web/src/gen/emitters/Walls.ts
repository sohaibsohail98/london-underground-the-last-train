/**
 * Wall emission.
 *
 * Only faces that border open space are emitted, so a solid block of wall
 * tiles costs nothing on the inside. Faces are merged into one geometry per
 * material, and UVs are in world metres so the tiling runs continuously across
 * a merged run rather than resetting at every tile boundary.
 *
 * Lower walls are tiled to 2.4m and painted brick above, which is what most of
 * the older stations on the line actually look like.
 */

import { Vector3 } from 'three';
import { SKIRTING_HEIGHT, TILE, WALL_HEIGHT, LEGEND } from '../../data/legend';
import { builderFor, type EmitterContext } from '../Context';
import { ORTHOGONAL } from '../Grid';

const TILED_HEIGHT = 2.4;

interface FaceSpec {
  /** Outward normal of the face. */
  normal: Vector3;
  /** The two horizontal corners of the face, at y = 0. */
  a: Vector3;
  b: Vector3;
}

function faceForDirection(centre: Vector3, dx: number, dy: number): FaceSpec {
  const half = TILE / 2;

  if (dx === 1) {
    return {
      normal: new Vector3(1, 0, 0),
      a: new Vector3(centre.x + half, 0, centre.z + half),
      b: new Vector3(centre.x + half, 0, centre.z - half),
    };
  }
  if (dx === -1) {
    return {
      normal: new Vector3(-1, 0, 0),
      a: new Vector3(centre.x - half, 0, centre.z - half),
      b: new Vector3(centre.x - half, 0, centre.z + half),
    };
  }
  if (dy === 1) {
    return {
      normal: new Vector3(0, 0, 1),
      a: new Vector3(centre.x - half, 0, centre.z + half),
      b: new Vector3(centre.x + half, 0, centre.z + half),
    };
  }
  return {
    normal: new Vector3(0, 0, -1),
    a: new Vector3(centre.x + half, 0, centre.z - half),
    b: new Vector3(centre.x - half, 0, centre.z - half),
  };
}

/** Distance along the face used as the U coordinate, in metres. */
function alongMetres(a: Vector3, b: Vector3): number {
  return a.distanceTo(b);
}

export function emitWalls(context: EmitterContext): void {
  const { grid, station } = context;

  const tiled = builderFor(context, { set: 'tile', tilesPerMetre: 0.55, tint: 0xffffff }, true);
  const brick = builderFor(
    context,
    { set: 'painted_brick', tilesPerMetre: 0.42, tint: station.accent },
    true,
  );
  const skirting = builderFor(context, { set: 'rubber', tilesPerMetre: 1.2 }, true);

  const centre = new Vector3();

  for (const cell of grid.cells) {
    if (cell.char !== LEGEND.WALL) continue;

    grid.worldPosition(cell.x, cell.y, centre);

    for (const [dx, dy] of ORTHOGONAL) {
      // Emit only where this wall meets open space.
      if (!grid.isOpen(cell.x + dx, cell.y + dy)) continue;

      const face = faceForDirection(centre, dx, dy);
      const width = alongMetres(face.a, face.b);

      // Skirting strip at the bottom, then tile, then brick above.
      pushStrip(skirting, face, 0, SKIRTING_HEIGHT, width, 0.02);
      pushStrip(tiled, face, SKIRTING_HEIGHT, TILED_HEIGHT, width, 0.01);
      pushStrip(brick, face, TILED_HEIGHT, WALL_HEIGHT, width, 0);
    }
  }
}

/**
 * Pushes one horizontal band of a wall face. The small outward offset stops
 * coplanar bands from z-fighting where two materials meet.
 */
function pushStrip(
  builder: ReturnType<typeof builderFor>,
  face: FaceSpec,
  y0: number,
  y1: number,
  width: number,
  offset: number,
): void {
  if (y1 <= y0) return;

  const push = face.normal.clone().multiplyScalar(offset);

  const a = face.a.clone().add(push).setY(y0);
  const b = face.b.clone().add(push).setY(y0);
  const c = face.b.clone().add(push).setY(y1);
  const d = face.a.clone().add(push).setY(y1);

  builder.addQuad(a, b, c, d, [0, y0], [width, y0], [width, y1], [0, y1], 0, face.normal);
}

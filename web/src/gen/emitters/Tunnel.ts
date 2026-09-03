/**
 * Tunnel mouths.
 *
 * Each tunnel run produces a splined tube receding away from the station with a
 * gentle seeded curve, so the far end is never visible and the tunnel reads as
 * depth rather than as a hole in the wall. Ring supports and cable runs give
 * the eye something to judge that depth against, which matters more than the
 * tube itself.
 */

import {
  BackSide,
  CatmullRomCurve3,
  Color,
  Matrix4,
  Mesh,
  Quaternion,
  TubeGeometry,
  Vector3,
} from 'three';
import { LEGEND, TILE, TRACK_DROP } from '../../data/legend';
import { addProp, type EmitterContext } from '../Context';
import { ORTHOGONAL } from '../Grid';

const TUNNEL_LENGTH = 40;
const TUNNEL_RADIUS = 2.6;
const scale = new Vector3(1, 1, 1);

/** The outward direction of a mouth: away from whichever side has floor. */
function outwardDirection(context: EmitterContext, cells: { x: number; y: number }[]): Vector3 {
  const tally = new Vector3();

  for (const cell of cells) {
    for (const [dx, dy] of ORTHOGONAL) {
      if (context.grid.isWalkable(cell.x + dx, cell.y + dy)) {
        tally.x -= dx;
        tally.z -= dy;
      }
    }
  }

  if (tally.lengthSq() < 1e-6) {
    // Fall back to pointing at the nearest grid edge.
    const cell = cells[0];
    const toLeft = cell.x;
    const toRight = context.grid.width - 1 - cell.x;
    const toTop = cell.y;
    const toBottom = context.grid.height - 1 - cell.y;
    const smallest = Math.min(toLeft, toRight, toTop, toBottom);

    if (smallest === toLeft) return new Vector3(-1, 0, 0);
    if (smallest === toRight) return new Vector3(1, 0, 0);
    if (smallest === toTop) return new Vector3(0, 0, -1);
    return new Vector3(0, 0, 1);
  }

  return tally.normalize();
}

export function emitTunnels(context: EmitterContext): void {
  const { grid, materials, streams } = context;
  const runs = grid.runs(LEGEND.TUNNEL);
  if (runs.length === 0) return;

  const curveStream = streams.get('tunnel-curves');

  // Cloned rather than shared: we need the inside faces of the tube, and the
  // material library caches by option key, not by side.
  const tunnelMaterial = materials.surface({ set: 'concrete', tilesPerMetre: 0.42 }).clone();
  tunnelMaterial.side = BackSide;
  tunnelMaterial.name = 'tunnel-interior';

  for (const [index, run] of runs.entries()) {
    if (run.cells.length === 0) continue;

    const first = run.cells[0];
    const last = run.cells[run.cells.length - 1];

    const mouth = grid
      .worldPosition(first.x, first.y)
      .add(grid.worldPosition(last.x, last.y))
      .multiplyScalar(0.5)
      .setY(-TRACK_DROP + 1.5);

    const outward = outwardDirection(context, run.cells);
    const lateral = new Vector3(-outward.z, 0, outward.x);

    // Three control points with a seeded lateral drift and a slight rise, so
    // no two tunnels curve the same way.
    const drift = curveStream.range(-0.55, 0.55);
    const rise = curveStream.range(0.1, 0.5);

    const points = [
      mouth.clone(),
      mouth
        .clone()
        .addScaledVector(outward, TUNNEL_LENGTH * 0.3)
        .addScaledVector(lateral, drift * 4)
        .setY(mouth.y + rise * 0.4),
      mouth
        .clone()
        .addScaledVector(outward, TUNNEL_LENGTH * 0.65)
        .addScaledVector(lateral, drift * 11)
        .setY(mouth.y + rise),
      mouth
        .clone()
        .addScaledVector(outward, TUNNEL_LENGTH)
        .addScaledVector(lateral, drift * 20)
        .setY(mouth.y + rise * 1.6),
    ];

    const curve = new CatmullRomCurve3(points);
    const tube = new Mesh(new TubeGeometry(curve, 40, TUNNEL_RADIUS, 16, false), tunnelMaterial);
    tube.name = `tunnel-${index}`;
    tube.receiveShadow = true;
    // No frustum culling: the mouth is often on screen while the bounding
    // sphere centre is well outside it.
    tube.frustumCulled = false;
    context.group.add(tube);

    // Ring supports every three metres, oriented along the spline.
    const ringCount = Math.floor(TUNNEL_LENGTH / 3);
    const up = new Vector3(0, 1, 0);

    for (let i = 0; i < ringCount; i += 1) {
      const t = Math.min(0.98, 0.02 + (i / ringCount) * 0.96);
      const point = curve.getPointAt(t);
      const tangent = curve.getTangentAt(t).normalize();
      const rotation = new Quaternion().setFromUnitVectors(up, tangent);

      addProp(context, 'tunnel_ring', new Matrix4().compose(point, rotation, scale));

      // Cable brackets on the near wall, half as often as the rings.
      if (i % 2 === 0) {
        const bracketPoint = point
          .clone()
          .addScaledVector(lateral, TUNNEL_RADIUS * 0.82)
          .setY(point.y + 0.9);
        addProp(context, 'cable_bracket', new Matrix4().compose(bracketPoint, rotation, scale));
      }
    }

    // A single dim light just inside the mouth. Not blackout sensitive: this
    // is the one thing that stops a blackout making the tunnel unreadable.
    context.lightCandidates.push({
      position: mouth
        .clone()
        .addScaledVector(outward, 2.5)
        .setY(mouth.y + 1.2),
      colour: new Color(0x6c4c9c),
      intensity: 2.4,
      range: 8,
      blackoutSensitive: false,
    });

    // Spawn point, set back inside the mouth so zombies walk out of the dark.
    context.tunnelSpawns.push(mouth.clone().addScaledVector(outward, 3).setY(0));

    context.anchors.push({
      kind: 'tunnel',
      label: `T${index + 1}`,
      position: mouth.clone().setY(0),
      facing: Math.atan2(-outward.x, -outward.z),
      gridX: first.x,
      gridY: first.y,
    });
  }

  void TILE;
}

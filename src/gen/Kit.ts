/**
 * The prop kit.
 *
 * Every prop in the game is a composition of boxes and cylinders, because that
 * is the one class of asset a model can author well and it costs nothing to
 * download. Each kit entry returns a single merged geometry so that a prop is
 * one instanced draw call however many parts it has.
 */

import { BoxGeometry, BufferGeometry, CylinderGeometry, Matrix4, Quaternion, Vector3 } from 'three';
import * as BufferGeometryUtils from 'three/addons/utils/BufferGeometryUtils.js';

export type PropKind =
  | 'barrier'
  | 'bench'
  | 'bin'
  | 'wayfinding'
  | 'ad_frame'
  | 'boarded_panel'
  | 'debris_pile'
  | 'wallbuy_plate'
  | 'perk_machine'
  | 'lost_property'
  | 'upgrade_bench'
  | 'litter'
  | 'rail'
  | 'sleeper'
  | 'tunnel_ring'
  | 'cable_bracket'
  | 'strip_light';

interface Part {
  geometry: BufferGeometry;
  position?: [number, number, number];
  rotation?: [number, number, number];
}

const matrix = new Matrix4();
const quaternion = new Quaternion();
const euler = new Vector3();
const position = new Vector3();
const scale = new Vector3(1, 1, 1);

function merge(parts: Part[]): BufferGeometry {
  const transformed: BufferGeometry[] = [];

  for (const part of parts) {
    const geometry = part.geometry.clone();
    const [px, py, pz] = part.position ?? [0, 0, 0];
    const [rx, ry, rz] = part.rotation ?? [0, 0, 0];

    euler.set(rx, ry, rz);
    quaternion.setFromAxisAngle(new Vector3(1, 0, 0), euler.x);
    const yaw = new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), euler.y);
    const roll = new Quaternion().setFromAxisAngle(new Vector3(0, 0, 1), euler.z);
    quaternion.multiply(yaw).multiply(roll);

    position.set(px, py, pz);
    matrix.compose(position, quaternion, scale);
    geometry.applyMatrix4(matrix);
    transformed.push(geometry);
  }

  const merged = BufferGeometryUtils.mergeGeometries(transformed, false);
  for (const geometry of transformed) geometry.dispose();

  if (!merged) throw new Error('Kit: geometry merge failed');
  merged.computeBoundingSphere();
  return merged;
}

/**
 * Builds every prop geometry once. Sizes are in metres and were chosen to sit
 * correctly against a 1.75m player capsule.
 */
export function buildKit(): Record<PropKind, BufferGeometry> {
  const box = (x: number, y: number, z: number): BufferGeometry => new BoxGeometry(x, y, z);
  const cylinder = (
    top: number,
    bottom: number,
    height: number,
    segments = 12,
  ): BufferGeometry => new CylinderGeometry(top, bottom, height, segments);

  return {
    // Waist high gate with two uprights and a glazed paddle.
    barrier: merge([
      { geometry: box(0.28, 1.02, 1.1), position: [0, 0.51, 0] },
      { geometry: box(0.34, 0.1, 1.2), position: [0, 1.06, 0] },
      { geometry: box(0.06, 0.72, 0.5), position: [0.2, 0.62, 0.4], rotation: [0, 0, 0.06] },
    ]),

    bench: merge([
      { geometry: box(2.2, 0.09, 0.46), position: [0, 0.46, 0] },
      { geometry: box(2.2, 0.36, 0.07), position: [0, 0.66, -0.21] },
      { geometry: box(0.09, 0.46, 0.42), position: [-0.92, 0.23, 0] },
      { geometry: box(0.09, 0.46, 0.42), position: [0.92, 0.23, 0] },
    ]),

    bin: merge([
      { geometry: cylinder(0.29, 0.24, 0.86), position: [0, 0.43, 0] },
      { geometry: cylinder(0.31, 0.31, 0.06, 12), position: [0, 0.88, 0] },
      { geometry: box(0.06, 0.4, 0.06), position: [0, 1.06, 0] },
    ]),

    // Frame only. The panel face carries generated artwork as a separate mesh.
    wayfinding: merge([
      { geometry: box(1.9, 0.7, 0.06), position: [0, 0, 0] },
      { geometry: box(2, 0.06, 0.1), position: [0, 0.38, 0] },
      { geometry: box(2, 0.06, 0.1), position: [0, -0.38, 0] },
    ]),

    ad_frame: merge([
      { geometry: box(1.5, 2.1, 0.07), position: [0, 0, 0] },
      { geometry: box(1.62, 0.08, 0.12), position: [0, 1.09, 0] },
      { geometry: box(1.62, 0.08, 0.12), position: [0, -1.09, 0] },
      { geometry: box(0.08, 2.2, 0.12), position: [-0.79, 0, 0] },
      { geometry: box(0.08, 2.2, 0.12), position: [0.79, 0, 0] },
    ]),

    // Crossed planks over a doorway, removed when the spawn is broken open.
    boarded_panel: merge([
      { geometry: box(1.5, 0.2, 0.06), position: [0, 0.4, 0], rotation: [0, 0, 0.04] },
      { geometry: box(1.5, 0.2, 0.06), position: [0, 0.95, 0], rotation: [0, 0, -0.03] },
      { geometry: box(1.5, 0.2, 0.06), position: [0, 1.5, 0], rotation: [0, 0, 0.05] },
      { geometry: box(1.5, 0.2, 0.06), position: [0, 2.05, 0], rotation: [0, 0, -0.02] },
      { geometry: box(0.18, 2.4, 0.05), position: [-0.5, 1.2, 0.04], rotation: [0, 0, 0.12] },
    ]),

    debris_pile: merge([
      { geometry: box(1.3, 0.7, 1.2), position: [0, 0.35, 0], rotation: [0, 0.3, 0.04] },
      { geometry: box(0.9, 0.5, 0.8), position: [0.4, 0.9, -0.2], rotation: [0.1, 0.9, 0] },
      { geometry: box(1.1, 0.4, 0.5), position: [-0.3, 1.1, 0.2], rotation: [0, 1.6, 0.14] },
      { geometry: box(0.4, 1.6, 0.3), position: [0.5, 0.8, 0.4], rotation: [0.3, 0, 0.5] },
    ]),

    // A weapon outline chalked on a wall plate, the wall-buy convention.
    wallbuy_plate: merge([
      { geometry: box(1.4, 0.9, 0.05), position: [0, 0, 0] },
      { geometry: box(1.1, 0.14, 0.09), position: [0, 0.06, 0], rotation: [0, 0, 0.05] },
      { geometry: box(0.24, 0.3, 0.08), position: [-0.32, -0.14, 0] },
    ]),

    perk_machine: merge([
      { geometry: box(0.9, 1.9, 0.7), position: [0, 0.95, 0] },
      { geometry: box(0.72, 0.9, 0.06), position: [0, 1.25, 0.36] },
      { geometry: box(0.9, 0.12, 0.78), position: [0, 1.92, 0] },
      { geometry: box(0.3, 0.16, 0.16), position: [0, 0.55, 0.4] },
    ]),

    lost_property: merge([
      { geometry: box(2.4, 1.1, 0.5), position: [0, 1.5, 0] },
      { geometry: box(2.6, 0.16, 0.6), position: [0, 2.12, 0] },
      { geometry: box(2.3, 0.08, 0.42), position: [0, 0.95, 0.06] },
      { geometry: box(0.22, 0.22, 0.22), position: [0.7, 1.06, 0.1] },
    ]),

    upgrade_bench: merge([
      { geometry: box(2.2, 0.12, 0.9), position: [0, 0.92, 0] },
      { geometry: box(0.12, 0.92, 0.82), position: [-1, 0.46, 0] },
      { geometry: box(0.12, 0.92, 0.82), position: [1, 0.46, 0] },
      { geometry: cylinder(0.05, 0.05, 0.9, 8), position: [0.8, 1.4, -0.2] },
      { geometry: cylinder(0.16, 0.08, 0.14, 10), position: [0.8, 1.82, -0.02], rotation: [0.7, 0, 0] },
    ]),

    litter: merge([
      { geometry: box(0.22, 0.01, 0.3), position: [0, 0.005, 0], rotation: [0, 0.4, 0] },
      { geometry: box(0.16, 0.01, 0.12), position: [0.2, 0.005, 0.14], rotation: [0, 1.1, 0] },
    ]),

    rail: box(1.5, 0.14, 0.11),
    sleeper: box(0.24, 0.16, 3.2),

    tunnel_ring: cylinder(2.58, 2.58, 0.16, 18),

    cable_bracket: merge([
      { geometry: box(0.08, 0.3, 0.08), position: [0, 0, 0] },
      { geometry: box(0.36, 0.06, 0.08), position: [0.14, 0.14, 0] },
    ]),

    strip_light: box(2.6, 0.09, 0.28),
  };
}

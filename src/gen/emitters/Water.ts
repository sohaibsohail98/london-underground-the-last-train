/**
 * Standing water.
 *
 * One plane over the bounding region of all water tiles, with two layers of
 * scrolling normal map at different rates and directions to fake a flow map,
 * near zero roughness so screen space reflections take hold, and a height that
 * the flood mechanic raises each round.
 */

import { Mesh, PlaneGeometry, Vector3 } from 'three';
import { MeshStandardNodeMaterial } from 'three/webgpu';
import { float, texture as textureNode, time, uniform, uv, vec2, vec3 } from 'three/tsl';
import { LEGEND, TILE } from '../../data/legend';
import type { EmitterContext } from '../Context';

export interface WaterControl {
  mesh: Mesh;
  /** Current surface height in metres above the floor. */
  height: { value: number };
  setHeight(metres: number): void;
  bounds: { minX: number; maxX: number; minZ: number; maxZ: number };
}

export function emitWater(context: EmitterContext): WaterControl | null {
  const { grid, materials, station } = context;
  const cells = grid.find([LEGEND.WATER]);
  if (cells.length === 0) return null;

  let minGx = Number.POSITIVE_INFINITY;
  let maxGx = Number.NEGATIVE_INFINITY;
  let minGy = Number.POSITIVE_INFINITY;
  let maxGy = Number.NEGATIVE_INFINITY;

  for (const cell of cells) {
    minGx = Math.min(minGx, cell.x);
    maxGx = Math.max(maxGx, cell.x);
    minGy = Math.min(minGy, cell.y);
    maxGy = Math.max(maxGy, cell.y);
  }

  const minCorner = grid.worldPosition(minGx, minGy);
  const maxCorner = grid.worldPosition(maxGx, maxGy);

  const bounds = {
    minX: minCorner.x - TILE / 2,
    maxX: maxCorner.x + TILE / 2,
    minZ: minCorner.z - TILE / 2,
    maxZ: maxCorner.z + TILE / 2,
  };

  const width = bounds.maxX - bounds.minX;
  const depth = bounds.maxZ - bounds.minZ;

  const tiles = materials.textures('wet_tile');
  const material = new MeshStandardNodeMaterial();
  material.name = 'water-surface';
  material.transparent = true;
  material.opacity = 0.82;

  const height = uniform(0.05);

  // Two normal layers at different scales and drift directions. Cheap, and at
  // this camera angle it is indistinguishable from a real flow map.
  const uvA = uv().mul(vec2(0.35, 0.35)).add(vec2(time.mul(0.012), time.mul(0.02)));
  const uvB = uv().mul(vec2(0.14, 0.19)).add(vec2(time.mul(-0.016), time.mul(0.009)));

  const normalA = textureNode(tiles.normal, uvA).rgb.mul(2).sub(1);
  const normalB = textureNode(tiles.normal, uvB).rgb.mul(2).sub(1);

  material.normalNode = normalA.add(normalB).mul(vec3(0.35, 0.35, 1)).normalize();
  material.colorNode = vec3(0.045, 0.06, 0.07).mul(float(1).add(height.mul(0.4)));
  material.roughnessNode = float(0.04);
  material.metalnessNode = float(0.02);

  const mesh = new Mesh(new PlaneGeometry(width, depth, 1, 1), material);
  mesh.rotation.x = -Math.PI / 2;
  mesh.position.set((bounds.minX + bounds.maxX) / 2, height.value, (bounds.minZ + bounds.maxZ) / 2);
  mesh.name = 'water';
  mesh.receiveShadow = false;

  // UVs in metres, matching every other surface in the generator.
  const attribute = mesh.geometry.attributes.uv;
  for (let i = 0; i < attribute.count; i += 1) {
    attribute.setXY(i, attribute.getX(i) * width, attribute.getY(i) * depth);
  }
  attribute.needsUpdate = true;

  context.group.add(mesh);

  context.waterBounds = bounds;

  void station;

  return {
    mesh,
    height,
    bounds,
    setHeight: (metres: number): void => {
      height.value = metres;
      mesh.position.y = metres;
    },
  };
}

export function waterCentre(bounds: WaterControl['bounds']): Vector3 {
  return new Vector3((bounds.minX + bounds.maxX) / 2, 0, (bounds.minZ + bounds.maxZ) / 2);
}

/**
 * The procedural architecture generator.
 *
 * Takes a StationDef, returns a finished station: merged static geometry,
 * instanced props, emissive strips, light candidates, a navigation grid and
 * every anchor the gameplay systems need. Fully deterministic from the
 * station's seed, so the same grid always builds the same station.
 *
 * Draw call discipline is the whole point: one draw per material for the
 * static shell, one per prop kind, one per strip kind. A 60x40 station lands
 * in the low tens, well under the 400 ceiling, which leaves the budget for
 * content in Phase 6 rather than spending it on architecture.
 */

import {
  Color,
  Group,
  InstancedMesh,
  Matrix4,
  Mesh,
  Quaternion,
  Vector3,
  type BufferGeometry,
} from 'three';
import { MeshBasicNodeMaterial } from 'three/webgpu';
import { vec3 } from 'three/tsl';
import { StreamSet } from '../engine/Random';
import { TILE, TILE_COST, WALKABLE_TILES, WALL_HEIGHT } from '../data/legend';
import type { StationDef } from '../data/schemas';
import type { Lighting } from '../render/Lighting';
import type { MaterialLibrary } from '../render/Materials';
import type { Occlusion } from '../render/Occlusion';
import { Grid } from './Grid';
import { buildKit, type PropKind } from './Kit';
import { type Anchor, type EmitterContext, type StripKind } from './Context';
import { emitWalls } from './emitters/Walls';
import { emitFloor } from './emitters/Floor';
import { emitPlatform } from './emitters/Platform';
import { emitTunnels } from './emitters/Tunnel';
import { emitEscalators, type HandrailControl } from './emitters/Escalator';
import { emitWater, type WaterControl } from './emitters/Water';
import { emitProps } from './emitters/Props';
import { emitSignage } from './emitters/Signage';

export interface NavGrid {
  width: number;
  height: number;
  /** 1 where a capsule may stand. */
  walkable: Uint8Array;
  /** Movement cost multiplier per cell. */
  cost: Float32Array;
  /** Grid to world and back, shared with the flow field in Phase 4. */
  grid: Grid;
}

export interface GeneratedStation {
  def: StationDef;
  grid: Grid;
  group: Group;
  nav: NavGrid;
  anchors: Anchor[];
  tunnelSpawns: Vector3[];
  boardedSpawns: Vector3[];
  playerSpawn: Vector3;
  trainStop: { position: Vector3; yaw: number } | null;
  water: WaterControl | null;
  handrail: HandrailControl | null;
  stats: { drawCalls: number; triangles: number; instances: number; lightCandidates: number };
  /** Advances any animated materials. Called once per rendered frame. */
  update(dt: number): void;
  dispose(): void;
}

/** Ceiling strips along the long axis of each region, every four tiles. */
function emitCeilingStrips(context: EmitterContext): void {
  const { grid, streams } = context;
  const flicker = streams.get('strip-flicker');

  for (const region of grid.regions()) {
    if (region.length < 6) continue;

    let minX = Number.POSITIVE_INFINITY;
    let maxX = Number.NEGATIVE_INFINITY;
    let minY = Number.POSITIVE_INFINITY;
    let maxY = Number.NEGATIVE_INFINITY;

    for (const cell of region) {
      minX = Math.min(minX, cell.x);
      maxX = Math.max(maxX, cell.x);
      minY = Math.min(minY, cell.y);
      maxY = Math.max(maxY, cell.y);
    }

    const alongX = maxX - minX >= maxY - minY;
    const inRegion = new Set(region.map((cell) => cell.y * grid.width + cell.x));

    // Walk the region's long axis in steps of four tiles on the cross axis.
    const crossStart = alongX ? minY : minX;
    const crossEnd = alongX ? maxY : maxX;

    for (let cross = crossStart + 1; cross <= crossEnd; cross += 4) {
      const longStart = alongX ? minX : minY;
      const longEnd = alongX ? maxX : maxY;

      for (let along = longStart; along <= longEnd; along += 2) {
        const x = alongX ? along : cross;
        const y = alongX ? cross : along;
        if (!inRegion.has(y * grid.width + x)) continue;

        const position = grid.worldPosition(x, y).setY(WALL_HEIGHT - 0.14);

        // One strip in eight is a dead one, which does more for the mood than
        // any amount of extra geometry.
        const dead = flicker.chance(0.12);
        const intensity = dead ? 0.05 : 3.2;

        context.strips.push({
          kind: 'ceiling',
          matrix: new Matrix4().compose(
            position,
            new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), alongX ? 0 : Math.PI / 2),
            new Vector3((TILE * 2) / 2.6, 1, 1),
          ),
          colour: new Color(dead ? 0x60606a : 0xffe6bc),
          intensity,
        });

        if (!dead) {
          context.lightCandidates.push({
            position: position.clone().setY(WALL_HEIGHT - 0.35),
            colour: new Color(0xffe6bc),
            intensity: 11,
            range: 11,
            blackoutSensitive: true,
          });
        }
      }
    }
  }
}

function buildNav(grid: Grid): NavGrid {
  const walkable = new Uint8Array(grid.width * grid.height);
  const cost = new Float32Array(grid.width * grid.height);

  const walkableSet = new Set<string>(WALKABLE_TILES);

  for (const cell of grid.cells) {
    const index = cell.y * grid.width + cell.x;
    const passable = walkableSet.has(cell.char);
    walkable[index] = passable ? 1 : 0;
    cost[index] = passable ? (TILE_COST[cell.char] ?? 1) : Number.POSITIVE_INFINITY;
  }

  return { width: grid.width, height: grid.height, walkable, cost, grid };
}

/** One instanced mesh per emissive strip kind, sharing a basic material. */
function buildStrips(
  context: EmitterContext,
  lighting: Lighting,
): { meshes: InstancedMesh[]; instances: number } {
  const byKind = new Map<StripKind, typeof context.strips>();

  for (const strip of context.strips) {
    const bucket = byKind.get(strip.kind);
    if (bucket) bucket.push(strip);
    else byKind.set(strip.kind, [strip]);
  }

  const meshes: InstancedMesh[] = [];
  let instances = 0;

  for (const [kind, strips] of byKind) {
    if (strips.length === 0) continue;

    const material = new MeshBasicNodeMaterial();
    material.name = `strip-${kind}`;
    // Emissive scale is shared, so a blackout takes every strip to black
    // without touching a single mesh.
    material.colorNode = vec3(1, 1, 1).mul(lighting.emissiveScale);

    const mesh = new InstancedMesh(context.kit.strip_light, material, strips.length);
    mesh.name = `strips-${kind}`;
    mesh.frustumCulled = false;

    for (const [index, strip] of strips.entries()) {
      mesh.setMatrixAt(index, strip.matrix);
      // Instance colour carries both tint and intensity, so one material and
      // one draw call covers dead strips, warm strips and accent strips.
      mesh.setColorAt(index, strip.colour.clone().multiplyScalar(strip.intensity));
    }

    mesh.instanceMatrix.needsUpdate = true;
    if (mesh.instanceColor) mesh.instanceColor.needsUpdate = true;

    meshes.push(mesh);
    instances += strips.length;
  }

  return { meshes, instances };
}

export function generateStation(
  def: StationDef,
  materials: MaterialLibrary,
  lighting: Lighting,
  occlusion: Occlusion,
): GeneratedStation {
  const grid = new Grid(def.grid);
  const group = new Group();
  group.name = `station-${def.id}`;

  const kit = buildKit();

  const context: EmitterContext = {
    grid,
    station: def,
    streams: new StreamSet(def.seed),
    materials,
    lighting,
    occlusion,
    kit,
    group,
    builders: new Map(),
    props: new Map(),
    strips: [],
    anchors: [],
    lightCandidates: [],
    tunnelSpawns: [],
    playerSpawn: new Vector3(),
    trainStop: null,
    waterBounds: null,
  };

  // Emitter order is fixed. Streams are named, so this order does not affect
  // determinism, but changing it would change which cells later emitters see
  // as already dressed, so it stays as authored.
  emitWalls(context);
  emitFloor(context);
  emitPlatform(context);
  emitTunnels(context);
  const handrail = emitEscalators(context);
  const water = emitWater(context);
  emitProps(context);
  emitSignage(context);
  emitCeilingStrips(context);

  let triangles = 0;
  let drawCalls = 0;

  // Merged static shell: one mesh per material.
  for (const [key, entry] of context.builders) {
    if (entry.builder.isEmpty) continue;

    const geometry = entry.builder.build();
    const material = materials.surface(entry.options);
    const mesh = new Mesh(geometry, material);
    mesh.name = `shell-${key}`;
    mesh.castShadow = true;
    mesh.receiveShadow = true;

    if (entry.occludable) occlusion.register(mesh);

    group.add(mesh);
    triangles += entry.builder.triangleCount;
    drawCalls += 1;
  }

  // Instanced props: one draw per kind.
  const propMaterial = materials.surface({ set: 'steel', tilesPerMetre: 1.2 });
  const rubberMaterial = materials.surface({ set: 'rubber', tilesPerMetre: 1 });
  const concreteMaterial = materials.surface({ set: 'concrete', tilesPerMetre: 0.8 });

  const propMaterials: Partial<Record<PropKind, typeof propMaterial>> = {
    sleeper: rubberMaterial,
    bin: rubberMaterial,
    litter: rubberMaterial,
    debris_pile: concreteMaterial,
    tunnel_ring: concreteMaterial,
    boarded_panel: rubberMaterial,
  };

  let instances = 0;

  for (const [kind, matrices] of context.props) {
    if (matrices.length === 0) continue;

    const geometry: BufferGeometry = kit[kind];
    const mesh = new InstancedMesh(geometry, propMaterials[kind] ?? propMaterial, matrices.length);
    mesh.name = `props-${kind}`;
    mesh.castShadow = kind !== 'litter';
    mesh.receiveShadow = true;
    mesh.frustumCulled = false;

    for (const [index, matrix] of matrices.entries()) mesh.setMatrixAt(index, matrix);
    mesh.instanceMatrix.needsUpdate = true;

    group.add(mesh);
    drawCalls += 1;
    instances += matrices.length;
  }

  const strips = buildStrips(context, lighting);
  for (const mesh of strips.meshes) {
    group.add(mesh);
    drawCalls += 1;
  }
  instances += strips.instances;

  // Register the light candidates. The rig activates only the nearest few, so
  // a station may register hundreds without cost.
  for (const candidate of context.lightCandidates) lighting.register(candidate);

  const boardedSpawns = context.anchors
    .filter((anchor) => anchor.kind === 'boarded')
    .map((anchor) => anchor.position.clone());

  const nav = buildNav(grid);

  if (context.playerSpawn.lengthSq() === 0) {
    // No S tile parsed: fall back to the centre of the largest region rather
    // than dropping the player into a wall.
    const regions = grid.regions().sort((a, b) => b.length - a.length);
    const cell = regions[0]?.[Math.floor((regions[0]?.length ?? 1) / 2)];
    if (cell) grid.worldPosition(cell.x, cell.y, context.playerSpawn);
  }

  return {
    def,
    grid,
    group,
    nav,
    anchors: context.anchors,
    tunnelSpawns: context.tunnelSpawns,
    boardedSpawns,
    playerSpawn: context.playerSpawn,
    trainStop: context.trainStop,
    water,
    handrail,
    stats: {
      drawCalls,
      triangles,
      instances,
      lightCandidates: context.lightCandidates.length,
    },
    update(dt: number): void {
      if (handrail) handrail.scroll.value = (handrail.scroll.value + dt * handrail.speed) % 1;
    },
    dispose(): void {
      group.traverse((object) => {
        if (object instanceof InstancedMesh || object instanceof Mesh) object.geometry.dispose();
      });
      for (const geometry of Object.values(kit)) geometry.dispose();
      group.clear();
    },
  };
}

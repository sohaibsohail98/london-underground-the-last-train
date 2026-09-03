/**
 * Shared state passed to every emitter.
 *
 * Emitters never create meshes themselves. They push geometry into a builder
 * keyed by material, or an instance matrix into a prop bucket, and the
 * generator turns those into the smallest possible number of draw calls at the
 * end. That is the whole reason a station of any size stays under budget.
 */

import { Group, Matrix4, Vector3, type BufferGeometry } from 'three';
import type { Color } from 'three';
import type { LightCandidate, Lighting } from '../render/Lighting';
import type { MaterialLibrary, SurfaceOptions } from '../render/Materials';
import type { Occlusion } from '../render/Occlusion';
import type { StreamSet } from '../engine/Random';
import type { StationDef } from '../data/schemas';
import type { Grid } from './Grid';
import { MeshBuilder } from './MeshBuilder';
import type { PropKind } from './Kit';

/** Emissive geometry kinds. Each becomes one instanced draw call. */
export type StripKind = 'ceiling' | 'edge' | 'warning' | 'sign';

export type AnchorKind =
  | 'wallbuy'
  | 'perk'
  | 'debris'
  | 'lost_property'
  | 'upgrade'
  | 'spawn'
  | 'boarded'
  | 'tunnel'
  | 'barrier';

export interface Anchor {
  kind: AnchorKind;
  /** Stable label, assigned in grid reading order, eg "W1", "P2". */
  label: string;
  position: Vector3;
  /** Yaw the prop faces, in radians. */
  facing: number;
  gridX: number;
  gridY: number;
}

export interface EmitterContext {
  grid: Grid;
  station: StationDef;
  streams: StreamSet;
  materials: MaterialLibrary;
  lighting: Lighting;
  occlusion: Occlusion;
  kit: Record<PropKind, BufferGeometry>;
  group: Group;

  /** Merged static geometry, one builder per distinct material. */
  builders: Map<string, { options: SurfaceOptions; builder: MeshBuilder; occludable: boolean }>;
  /** Instanced props, one draw call per kind. */
  props: Map<PropKind, Matrix4[]>;
  /** Emissive strips, one instanced batch per kind. */
  strips: { kind: StripKind; matrix: Matrix4; colour: Color; intensity: number }[];

  anchors: Anchor[];
  lightCandidates: LightCandidate[];
  tunnelSpawns: Vector3[];
  playerSpawn: Vector3;
  /** Set by the platform emitter: where the train comes to rest. */
  trainStop: { position: Vector3; yaw: number } | null;
  /** Set by the water emitter, if the station has any. */
  waterBounds: { minX: number; maxX: number; minZ: number; maxZ: number } | null;
}

/** Fetches or creates the builder for a material, keyed by its options. */
export function builderFor(
  context: EmitterContext,
  options: SurfaceOptions,
  occludable = false,
): MeshBuilder {
  const key = `${options.set}|${options.tilesPerMetre ?? 0.5}|${options.wetness ?? 0}|${
    options.tint ?? 0xffffff
  }|${occludable}|${options.roughnessBias ?? 0}`;

  const existing = context.builders.get(key);
  if (existing) return existing.builder;

  const created = { options: { ...options, occludable }, builder: new MeshBuilder(), occludable };
  context.builders.set(key, created);
  return created.builder;
}

/** Queues an instance of a prop. */
export function addProp(context: EmitterContext, kind: PropKind, matrix: Matrix4): void {
  const existing = context.props.get(kind);
  if (existing) {
    existing.push(matrix);
    return;
  }
  context.props.set(kind, [matrix]);
}

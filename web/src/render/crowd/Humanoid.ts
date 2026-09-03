/**
 * Procedural humanoid.
 *
 * The brief assumes a rigged glTF at /assets, and the baker prefers one when
 * it is there. This is the fallback: a jointed figure built from boxes with a
 * bone hierarchy and hand authored poses, so the crowd system is testable and
 * Gate C is reachable with an empty assets folder.
 *
 * It is not a substitute for a real mesh. It is a stand-in of the correct
 * shape, vertex count and animation set, which is what the vertex animation
 * texture pipeline actually cares about.
 */

import { BoxGeometry, BufferAttribute, Euler, Matrix4, Quaternion } from 'three';
import * as BufferGeometryUtils from 'three/addons/utils/BufferGeometryUtils.js';
import type { BufferGeometry } from 'three';

export type BoneName =
  | 'pelvis'
  | 'torso'
  | 'head'
  | 'armUpperL'
  | 'armLowerL'
  | 'armUpperR'
  | 'armLowerR'
  | 'legUpperL'
  | 'legLowerL'
  | 'legUpperR'
  | 'legLowerR';

interface BoneSpec {
  name: BoneName;
  parent: BoneName | null;
  /** Offset from the parent's origin, in metres. */
  offset: [number, number, number];
  /** Box size of the limb. */
  size: [number, number, number];
  /** Where the box sits relative to the joint, normally half its length down. */
  centre: [number, number, number];
}

/** Total height comes out at about 1.78m, matching the player capsule. */
const SKELETON: BoneSpec[] = [
  {
    name: 'pelvis',
    parent: null,
    offset: [0, 0.94, 0],
    size: [0.32, 0.24, 0.2],
    centre: [0, 0, 0],
  },
  {
    name: 'torso',
    parent: 'pelvis',
    offset: [0, 0.12, 0],
    size: [0.42, 0.56, 0.24],
    centre: [0, 0.28, 0],
  },
  {
    name: 'head',
    parent: 'torso',
    offset: [0, 0.56, 0],
    size: [0.22, 0.26, 0.24],
    centre: [0, 0.15, 0],
  },

  {
    name: 'armUpperL',
    parent: 'torso',
    offset: [0.26, 0.48, 0],
    size: [0.12, 0.3, 0.12],
    centre: [0, -0.15, 0],
  },
  {
    name: 'armLowerL',
    parent: 'armUpperL',
    offset: [0, -0.3, 0],
    size: [0.1, 0.3, 0.1],
    centre: [0, -0.15, 0],
  },
  {
    name: 'armUpperR',
    parent: 'torso',
    offset: [-0.26, 0.48, 0],
    size: [0.12, 0.3, 0.12],
    centre: [0, -0.15, 0],
  },
  {
    name: 'armLowerR',
    parent: 'armUpperR',
    offset: [0, -0.3, 0],
    size: [0.1, 0.3, 0.1],
    centre: [0, -0.15, 0],
  },

  {
    name: 'legUpperL',
    parent: 'pelvis',
    offset: [0.11, -0.12, 0],
    size: [0.15, 0.42, 0.16],
    centre: [0, -0.21, 0],
  },
  {
    name: 'legLowerL',
    parent: 'legUpperL',
    offset: [0, -0.42, 0],
    size: [0.13, 0.42, 0.15],
    centre: [0, -0.21, 0],
  },
  {
    name: 'legUpperR',
    parent: 'pelvis',
    offset: [-0.11, -0.12, 0],
    size: [0.15, 0.42, 0.16],
    centre: [0, -0.21, 0],
  },
  {
    name: 'legLowerR',
    parent: 'legUpperR',
    offset: [0, -0.42, 0],
    size: [0.13, 0.42, 0.15],
    centre: [0, -0.21, 0],
  },
];

export const BONE_ORDER: BoneName[] = SKELETON.map((bone) => bone.name);

export type Pose = Partial<Record<BoneName, [number, number, number]>>;

/** Clip names, in the fixed order the crowd system indexes them by. */
export const CLIP_NAMES = ['walk', 'run', 'shamble', 'attack', 'stagger', 'death'] as const;
export type ClipName = (typeof CLIP_NAMES)[number];

export interface ClipSpec {
  name: ClipName;
  /** Duration in seconds. */
  duration: number;
  loop: boolean;
  /** Evaluates the pose at a normalised time in 0..1. */
  pose(t: number): Pose;
}

const TAU = Math.PI * 2;

/** A shambling gait: unequal stride, dragged trailing leg, dropped shoulders. */
function shamblePose(t: number, intensity: number, speed: number): Pose {
  const phase = t * TAU * speed;
  const swing = Math.sin(phase) * intensity;
  const counter = Math.sin(phase + Math.PI) * intensity;
  const bob = Math.abs(Math.sin(phase)) * 0.06 * intensity;

  return {
    pelvis: [0.06 * intensity, Math.sin(phase * 0.5) * 0.06, bob * 0.3],
    torso: [0.18 * intensity, Math.sin(phase) * 0.07, Math.sin(phase * 0.5) * 0.09 * intensity],
    head: [0.12 * intensity, Math.sin(phase * 0.7) * 0.12, -0.08 * intensity],

    // Arms hang forward and low, swinging out of time with the legs.
    armUpperL: [-0.9 - swing * 0.25, 0, 0.28],
    armLowerL: [-0.55 - counter * 0.2, 0, 0],
    armUpperR: [-0.78 - counter * 0.3, 0, -0.34],
    armLowerR: [-0.68 - swing * 0.15, 0, 0],

    legUpperL: [swing * 0.62, 0, 0.04],
    legLowerL: [Math.max(0, -swing) * 0.7, 0, 0],
    legUpperR: [counter * 0.52, 0, -0.06],
    legLowerR: [Math.max(0, -counter) * 0.85, 0, 0],
  };
}

export const CLIPS: ClipSpec[] = [
  {
    name: 'walk',
    duration: 1.1,
    loop: true,
    pose: (t) => shamblePose(t, 0.85, 1),
  },
  {
    name: 'run',
    duration: 0.62,
    loop: true,
    pose: (t) => {
      const base = shamblePose(t, 1.25, 1);
      // Leaning further forward, arms pulled back: a sprinter, not a shuffler.
      base.torso = [0.34, (base.torso?.[1] ?? 0) * 1.2, base.torso?.[2] ?? 0];
      base.armUpperL = [-1.5, 0, 0.2];
      base.armUpperR = [-1.42, 0, -0.22];
      return base;
    },
  },
  {
    name: 'shamble',
    duration: 1.9,
    loop: true,
    pose: (t) => shamblePose(t, 0.55, 1),
  },
  {
    name: 'attack',
    duration: 0.72,
    loop: false,
    pose: (t) => {
      // Wind up over the first third, strike, then recover.
      const strike = t < 0.34 ? t / 0.34 : 1 - (t - 0.34) / 0.66;
      const reach = Math.sin(strike * Math.PI * 0.5);

      return {
        pelvis: [0.08, 0, 0],
        torso: [0.1 + reach * 0.22, reach * 0.16, 0],
        head: [-0.18 - reach * 0.1, 0, 0],
        armUpperL: [-2.2 * reach - 0.4, 0.3 * reach, 0.4],
        armLowerL: [-0.3 * (1 - reach), 0, 0],
        armUpperR: [-2.05 * reach - 0.35, -0.26 * reach, -0.42],
        armLowerR: [-0.34 * (1 - reach), 0, 0],
        legUpperL: [0.22 * reach, 0, 0.05],
        legUpperR: [-0.18 * reach, 0, -0.05],
        legLowerR: [0.3 * reach, 0, 0],
      };
    },
  },
  {
    name: 'stagger',
    duration: 0.44,
    loop: false,
    pose: (t) => {
      const jolt = Math.sin(t * Math.PI) * (1 - t * 0.4);

      return {
        pelvis: [-0.14 * jolt, 0.1 * jolt, 0.12 * jolt],
        torso: [-0.4 * jolt, -0.2 * jolt, 0.18 * jolt],
        head: [-0.5 * jolt, 0.24 * jolt, 0],
        armUpperL: [-0.4 - jolt * 0.9, 0, 0.7 * jolt + 0.3],
        armUpperR: [-0.35 - jolt * 0.8, 0, -0.66 * jolt - 0.3],
        legUpperL: [-0.3 * jolt, 0, 0.1],
        legUpperR: [0.24 * jolt, 0, -0.1],
        legLowerL: [0.4 * jolt, 0, 0],
      };
    },
  },
  {
    name: 'death',
    duration: 1.25,
    loop: false,
    pose: (t) => {
      // Collapse forward, knees first, settling flat. The last frame is held
      // by the crowd system, so the settled pose is what matters most.
      const fall = Math.min(1, t * 1.35);
      const ease = fall * fall * (3 - 2 * fall);

      return {
        pelvis: [ease * 0.4, 0, -ease * 0.9],
        torso: [ease * 1.05, ease * 0.2, ease * 0.2],
        head: [ease * 0.5, -ease * 0.3, 0],
        armUpperL: [-ease * 1.9 - 0.3, ease * 0.4, 0.5],
        armLowerL: [-ease * 0.7, 0, 0],
        armUpperR: [-ease * 1.6 - 0.3, -ease * 0.5, -0.5],
        armLowerR: [-ease * 0.5, 0, 0],
        legUpperL: [ease * 1.5, 0, 0.1],
        legLowerL: [-ease * 1.7, 0, 0],
        legUpperR: [ease * 1.35, 0, -0.1],
        legLowerR: [-ease * 1.55, 0, 0],
      };
    },
  },
];

export interface HumanoidSource {
  /** Merged geometry with a boneIndex attribute per vertex. */
  geometry: BufferGeometry;
  vertexCount: number;
  /** Rest pose local matrices, indexed as BONE_ORDER. */
  restLocal: Matrix4[];
  parents: number[];
  /** Applies a pose and writes world matrices for every bone. */
  evaluate(pose: Pose, out: Matrix4[]): void;
}

/**
 * Builds the fallback humanoid. Vertex count lands near 260, which is well
 * under the 4096 the VAT layout allows, so the texture stays small.
 */
export function buildHumanoid(): HumanoidSource {
  const geometries: BufferGeometry[] = [];
  const restLocal: Matrix4[] = [];
  const parents: number[] = [];

  const indexOf = new Map<BoneName, number>();
  SKELETON.forEach((bone, index) => indexOf.set(bone.name, index));

  for (const bone of SKELETON) {
    const geometry = new BoxGeometry(bone.size[0], bone.size[1], bone.size[2]);

    // Move the box so the joint sits at the bone's origin.
    geometry.translate(bone.centre[0], bone.centre[1], bone.centre[2]);

    const boneIndex = indexOf.get(bone.name) ?? 0;
    const count = geometry.attributes.position.count;
    const indices = new Float32Array(count);
    indices.fill(boneIndex);
    geometry.setAttribute('boneIndex', new BufferAttribute(indices, 1));

    geometries.push(geometry);
    restLocal.push(new Matrix4().makeTranslation(bone.offset[0], bone.offset[1], bone.offset[2]));
    parents.push(bone.parent === null ? -1 : (indexOf.get(bone.parent) ?? -1));
  }

  const merged = BufferGeometryUtils.mergeGeometries(geometries, false);
  for (const geometry of geometries) geometry.dispose();
  if (!merged) throw new Error('Humanoid: geometry merge failed');

  const scratchQuaternion = new Quaternion();
  const scratchEuler = new Euler();
  const scratchLocal = new Matrix4();

  return {
    geometry: merged,
    vertexCount: merged.attributes.position.count,
    restLocal,
    parents,
    evaluate(pose: Pose, out: Matrix4[]): void {
      for (let i = 0; i < SKELETON.length; i += 1) {
        const bone = SKELETON[i];
        const rotation = pose[bone.name] ?? [0, 0, 0];

        scratchEuler.set(rotation[0], rotation[1], rotation[2], 'XYZ');
        scratchQuaternion.setFromEuler(scratchEuler);
        scratchLocal.makeRotationFromQuaternion(scratchQuaternion);
        scratchLocal.setPosition(bone.offset[0], bone.offset[1], bone.offset[2]);

        const parent = parents[i];
        if (!out[i]) out[i] = new Matrix4();

        if (parent === -1) out[i].copy(scratchLocal);
        else out[i].multiplyMatrices(out[parent], scratchLocal);
      }
    },
  };
}

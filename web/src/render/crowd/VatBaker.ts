/**
 * Vertex animation texture baker.
 *
 * Bakes the six clips into two textures at load: one of vertex positions, one
 * of normals. Rows are frames, columns are vertices. Every zombie then reads
 * its pose from the texture in the vertex shader, which is what allows 46 of
 * them in a single instanced draw call instead of 46 SkinnedMeshes.
 *
 * Two sources are supported. A rigged glTF at assets/models/zombie.glb with
 * clips in assets/anims is preferred and baked by stepping the skeleton. When
 * that is absent the procedural humanoid stands in, baked analytically from
 * its bone hierarchy. The output format is identical either way, so nothing
 * downstream needs to know which was used.
 */

import {
  AnimationMixer,
  DataTexture,
  HalfFloatType,
  LoopRepeat,
  Matrix4,
  NearestFilter,
  Object3D,
  RGBAFormat,
  SkinnedMesh,
  Vector3,
  type AnimationClip,
  type BufferGeometry,
} from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { DRACOLoader } from 'three/addons/loaders/DRACOLoader.js';
import { BONE_ORDER, CLIPS, CLIP_NAMES, buildHumanoid, type ClipName } from './Humanoid';

/** Baked at 30fps; the shader interpolates between rows. */
export const VAT_FPS = 30;

export interface ClipTableEntry {
  name: ClipName;
  startRow: number;
  frameCount: number;
  loop: boolean;
  duration: number;
}

export interface VatBake {
  geometry: BufferGeometry;
  positionTexture: DataTexture;
  normalTexture: DataTexture;
  vertexCount: number;
  rowCount: number;
  clips: ClipTableEntry[];
  /** True when a real rigged mesh was found rather than the fallback. */
  fromAsset: boolean;
  /** Height of the figure in metres, used to size the billboard LOD. */
  height: number;
  dispose(): void;
}

function makeTexture(width: number, height: number): DataTexture {
  // Half float: 2 bytes per channel is ample for positions at human scale and
  // halves the memory against full float.
  const data = new Uint16Array(width * height * 4);
  const texture = new DataTexture(data, width, height, RGBAFormat, HalfFloatType);
  texture.minFilter = NearestFilter;
  texture.magFilter = NearestFilter;
  texture.generateMipmaps = false;
  texture.needsUpdate = true;
  return texture;
}

/** IEEE 754 half precision encode. */
function toHalf(value: number): number {
  floatView[0] = value;
  const bits = intView[0];

  const sign = (bits >>> 16) & 0x8000;
  let exponent = (bits >>> 23) & 0xff;
  let mantissa = bits & 0x7fffff;

  if (exponent === 255) return sign | 0x7c00 | (mantissa ? 0x200 : 0);
  exponent -= 127 - 15;

  if (exponent >= 31) return sign | 0x7c00;
  if (exponent <= 0) {
    if (exponent < -10) return sign;
    mantissa |= 0x800000;
    const shift = 14 - exponent;
    return sign | (mantissa >> shift);
  }

  return sign | (exponent << 10) | (mantissa >> 13);
}

const floatView = new Float32Array(1);
const intView = new Int32Array(floatView.buffer);

function writeVector(data: Uint16Array, offset: number, x: number, y: number, z: number): void {
  data[offset] = toHalf(x);
  data[offset + 1] = toHalf(y);
  data[offset + 2] = toHalf(z);
  data[offset + 3] = toHalf(1);
}

/** Attempts the real asset path. Resolves null when anything is missing. */
async function loadRigged(): Promise<{
  mesh: SkinnedMesh;
  clips: Map<ClipName, AnimationClip>;
} | null> {
  try {
    const draco = new DRACOLoader().setDecoderPath('assets/draco/');
    const loader = new GLTFLoader().setDRACOLoader(draco);

    const gltf = await loader.loadAsync('assets/models/zombie.glb');

    let mesh: SkinnedMesh | null = null;
    gltf.scene.traverse((object: Object3D) => {
      if (!mesh && object instanceof SkinnedMesh) mesh = object;
    });

    if (!mesh) {
      draco.dispose();
      return null;
    }

    const clips = new Map<ClipName, AnimationClip>();

    // Clips may ship inside the model or as separate files, so try both.
    for (const name of CLIP_NAMES) {
      const embedded = gltf.animations.find((clip) => clip.name.toLowerCase().includes(name));
      if (embedded) {
        clips.set(name, embedded);
        continue;
      }

      try {
        const file = await loader.loadAsync(`assets/anims/${name}.glb`);
        if (file.animations[0]) clips.set(name, file.animations[0]);
      } catch {
        // Missing clip: handled by the completeness check below.
      }
    }

    draco.dispose();

    if (clips.size !== CLIP_NAMES.length) return null;
    return { mesh, clips };
  } catch {
    return null;
  }
}

/** Bakes from a rigged SkinnedMesh by stepping the skeleton frame by frame. */
function bakeRigged(mesh: SkinnedMesh, clips: Map<ClipName, AnimationClip>): VatBake {
  const geometry = mesh.geometry.clone();
  const vertexCount = geometry.attributes.position.count;

  const table: ClipTableEntry[] = [];
  let rowCount = 0;

  for (const name of CLIP_NAMES) {
    const clip = clips.get(name);
    if (!clip) continue;
    const frameCount = Math.max(2, Math.ceil(clip.duration * VAT_FPS));
    table.push({
      name,
      startRow: rowCount,
      frameCount,
      loop: name !== 'attack' && name !== 'stagger' && name !== 'death',
      duration: clip.duration,
    });
    // One guard row per clip, so bilinear bleed between clips is impossible.
    rowCount += frameCount + 1;
  }

  const positionTexture = makeTexture(vertexCount, rowCount);
  const normalTexture = makeTexture(vertexCount, rowCount);
  const positionData = positionTexture.image.data as Uint16Array;
  const normalData = normalTexture.image.data as Uint16Array;

  const mixer = new AnimationMixer(mesh);
  const restPosition = geometry.attributes.position;
  const restNormal = geometry.attributes.normal;

  const vertex = new Vector3();
  const normal = new Vector3();
  let height = 0;

  for (const entry of table) {
    const clip = clips.get(entry.name);
    if (!clip) continue;

    const action = mixer.clipAction(clip);
    mixer.stopAllAction();
    action.reset();
    action.setLoop(LoopRepeat, Infinity);
    action.play();

    for (let frame = 0; frame < entry.frameCount; frame += 1) {
      const time = (frame / entry.frameCount) * clip.duration;
      mixer.setTime(time);
      mesh.skeleton.update();

      const row = (entry.startRow + frame) * vertexCount * 4;

      for (let i = 0; i < vertexCount; i += 1) {
        vertex.fromBufferAttribute(restPosition, i);
        mesh.applyBoneTransform(i, vertex);
        writeVector(positionData, row + i * 4, vertex.x, vertex.y, vertex.z);
        height = Math.max(height, vertex.y);

        // Normals are transformed by the same bones; good enough at this
        // camera distance and far cheaper than recomputing per frame.
        normal.fromBufferAttribute(restNormal, i);
        mesh.applyBoneTransform(i, normal);
        normal.normalize();
        writeVector(normalData, row + i * 4, normal.x, normal.y, normal.z);
      }
    }

    action.stop();
  }

  positionTexture.needsUpdate = true;
  normalTexture.needsUpdate = true;

  return {
    geometry,
    positionTexture,
    normalTexture,
    vertexCount,
    rowCount,
    clips: table,
    fromAsset: true,
    height: height || 1.78,
    dispose(): void {
      geometry.dispose();
      positionTexture.dispose();
      normalTexture.dispose();
    },
  };
}

/** Bakes the procedural humanoid analytically from its bone hierarchy. */
function bakeProcedural(): VatBake {
  const source = buildHumanoid();
  const vertexCount = source.vertexCount;

  const table: ClipTableEntry[] = [];
  let rowCount = 0;

  for (const clip of CLIPS) {
    const frameCount = Math.max(2, Math.ceil(clip.duration * VAT_FPS));
    table.push({
      name: clip.name,
      startRow: rowCount,
      frameCount,
      loop: clip.loop,
      duration: clip.duration,
    });
    rowCount += frameCount + 1;
  }

  const positionTexture = makeTexture(vertexCount, rowCount);
  const normalTexture = makeTexture(vertexCount, rowCount);
  const positionData = positionTexture.image.data as Uint16Array;
  const normalData = normalTexture.image.data as Uint16Array;

  const positions = source.geometry.attributes.position;
  const normals = source.geometry.attributes.normal;
  const boneIndices = source.geometry.attributes.boneIndex;

  // Bind pose world matrices, so vertices can be moved into bone local space
  // once and then transformed per frame.
  const bind: Matrix4[] = [];
  source.evaluate({}, bind);
  const bindInverse = bind.map((matrix) => matrix.clone().invert());

  const local = new Vector3();
  const localNormal = new Vector3();
  const vertex = new Vector3();
  const normal = new Vector3();
  const pose: Matrix4[] = [];
  const normalMatrix = new Matrix4();

  let height = 0;

  for (const [clipIndex, clip] of CLIPS.entries()) {
    const entry = table[clipIndex];

    for (let frame = 0; frame < entry.frameCount; frame += 1) {
      const t = frame / entry.frameCount;
      source.evaluate(clip.pose(t), pose);

      const row = (entry.startRow + frame) * vertexCount * 4;

      for (let i = 0; i < vertexCount; i += 1) {
        const bone = boneIndices.getX(i);

        local.fromBufferAttribute(positions, i).applyMatrix4(bindInverse[bone]);
        vertex.copy(local).applyMatrix4(pose[bone]);
        writeVector(positionData, row + i * 4, vertex.x, vertex.y, vertex.z);
        height = Math.max(height, vertex.y);

        normalMatrix.copy(pose[bone]);
        normalMatrix.setPosition(0, 0, 0);
        localNormal.fromBufferAttribute(normals, i).applyMatrix4(bindInverse[bone]);
        localNormal.normalize();
        normal.copy(localNormal).applyMatrix4(normalMatrix).normalize();
        writeVector(normalData, row + i * 4, normal.x, normal.y, normal.z);
      }
    }
  }

  positionTexture.needsUpdate = true;
  normalTexture.needsUpdate = true;

  void BONE_ORDER;

  return {
    geometry: source.geometry,
    positionTexture,
    normalTexture,
    vertexCount,
    rowCount,
    clips: table,
    fromAsset: false,
    height: height || 1.78,
    dispose(): void {
      source.geometry.dispose();
      positionTexture.dispose();
      normalTexture.dispose();
    },
  };
}

/**
 * Bakes the crowd, preferring the real asset. Runs once per session and takes
 * a few hundred milliseconds; call it during the loading screen.
 */
export async function bakeCrowd(): Promise<VatBake> {
  const rigged = await loadRigged();
  if (rigged) return bakeRigged(rigged.mesh, rigged.clips);
  return bakeProcedural();
}

/** Index of a clip in the baked table, by name. */
export function clipIndex(bake: VatBake, name: ClipName): number {
  const index = bake.clips.findIndex((clip) => clip.name === name);
  return index === -1 ? 0 : index;
}

/**
 * The crowd.
 *
 * 46 animated humanoids in three draw calls: full vertex animation texture up
 * close, a cheaper single sample material at mid range, and impostor quads
 * beyond torch range. Instance buffers are rewritten every frame from plain
 * arrays that the simulation owns, so nothing here ever touches game state.
 *
 * The simulation writes position, facing and clip; this module decides LOD,
 * manages the crossfade weights and packs the attributes.
 */

import {
  Color,
  DynamicDrawUsage,
  InstancedBufferAttribute,
  InstancedMesh,
  Matrix4,
  Quaternion,
  Vector3,
  type Scene,
} from 'three';
import type { WebGPURenderer } from 'three/webgpu';
import { bakeBillboards, billboardGeometry, BILLBOARD_POSES, BILLBOARD_YAWS } from './Billboard';
import { createVatMaterials, CROWD_ATTRIBUTES, type VatMaterialSet } from './VatMaterial';
import { clipIndex, type VatBake } from './VatBaker';
import type { ClipName } from './Humanoid';
import type { Torch } from '../Torch';

export const CROWD_CAPACITY = 64;

/** Crossfade duration when a zombie changes animation. */
const BLEND_SECONDS = 0.15;

/** Hysteresis on LOD boundaries, in metres, to stop flicker at the edge. */
const LOD_HYSTERESIS = 1.5;

export interface CrowdSlot {
  active: boolean;
  position: Vector3;
  facing: number;
  scale: number;
  /** Multiplied into albedo. Varying this is most of what stops clones. */
  tint: Vector3;
  clip: number;
  previousClip: number;
  /** Seconds since the current clip started. */
  clipTime: number;
  /** Remaining crossfade, in seconds. */
  blend: number;
  playbackRate: number;
  /** Per instance phase offset, so identical clips do not march in step. */
  timeOffset: number;
  hitFlash: number;
  holdLastFrame: boolean;
  lod: 0 | 1 | 2;
}

interface Bucket {
  mesh: InstancedMesh;
  anim: InstancedBufferAttribute;
  animPrevious: InstancedBufferAttribute;
  flags: InstancedBufferAttribute;
  tile: InstancedBufferAttribute | null;
  count: number;
}

const matrix = new Matrix4();
const quaternion = new Quaternion();
const scaleVector = new Vector3();
const yAxis = new Vector3(0, 1, 0);

function makeBucket(mesh: InstancedMesh, capacity: number, withTile: boolean): Bucket {
  const anim = new InstancedBufferAttribute(new Float32Array(capacity * 4), 4);
  const animPrevious = new InstancedBufferAttribute(new Float32Array(capacity * 4), 4);
  const flags = new InstancedBufferAttribute(new Float32Array(capacity), 1);

  anim.setUsage(DynamicDrawUsage);
  animPrevious.setUsage(DynamicDrawUsage);
  flags.setUsage(DynamicDrawUsage);

  mesh.geometry.setAttribute(CROWD_ATTRIBUTES.anim, anim);
  mesh.geometry.setAttribute(CROWD_ATTRIBUTES.animPrevious, animPrevious);
  mesh.geometry.setAttribute(CROWD_ATTRIBUTES.flags, flags);

  let tile: InstancedBufferAttribute | null = null;
  if (withTile) {
    tile = new InstancedBufferAttribute(new Float32Array(capacity * 2), 2);
    tile.setUsage(DynamicDrawUsage);
    mesh.geometry.setAttribute('aTile', tile);
  }

  mesh.instanceMatrix.setUsage(DynamicDrawUsage);
  mesh.frustumCulled = false;
  mesh.count = 0;

  return { mesh, anim, animPrevious, flags, tile, count: 0 };
}

export class Crowd {
  readonly slots: CrowdSlot[] = [];

  /** Written by the preset; LOD 0 to 1 and 1 to 2 boundaries in metres. */
  lodDistances: [number, number] = [16, 26];

  /** Whether the near LOD casts into the torch shadow map. */
  castShadows = true;

  private readonly bake: VatBake;
  private readonly materials: VatMaterialSet;
  private readonly billboards: ReturnType<typeof bakeBillboards>;
  private readonly near: Bucket;
  private readonly mid: Bucket;
  private readonly far: Bucket;
  private readonly scene: Scene;
  private clock = 0;

  /** Live counts per LOD, for the profiler. */
  readonly counts = { near: 0, mid: 0, far: 0, active: 0 };

  constructor(scene: Scene, renderer: WebGPURenderer, bake: VatBake) {
    this.scene = scene;
    this.bake = bake;
    this.materials = createVatMaterials(bake);
    this.billboards = bakeBillboards(renderer, bake);

    // Separate geometry clones per LOD, because each carries its own instanced
    // attributes. The vertex data itself is small enough that this is cheaper
    // than any sharing scheme would be.
    const nearMesh = new InstancedMesh(bake.geometry.clone(), this.materials.near, CROWD_CAPACITY);
    nearMesh.name = 'crowd-near';
    nearMesh.castShadow = true;
    nearMesh.receiveShadow = true;

    const midMesh = new InstancedMesh(bake.geometry.clone(), this.materials.mid, CROWD_CAPACITY);
    midMesh.name = 'crowd-mid';
    midMesh.castShadow = false;
    midMesh.receiveShadow = false;

    const farMesh = new InstancedMesh(
      billboardGeometry(bake.height),
      this.billboards.material,
      CROWD_CAPACITY,
    );
    farMesh.name = 'crowd-far';
    farMesh.castShadow = false;
    farMesh.receiveShadow = false;

    this.near = makeBucket(nearMesh, CROWD_CAPACITY, false);
    this.mid = makeBucket(midMesh, CROWD_CAPACITY, false);
    this.far = makeBucket(farMesh, CROWD_CAPACITY, true);

    scene.add(nearMesh, midMesh, farMesh);

    for (let i = 0; i < CROWD_CAPACITY; i += 1) {
      this.slots.push({
        active: false,
        position: new Vector3(),
        facing: 0,
        scale: 1,
        tint: new Vector3(1, 1, 1),
        clip: 0,
        previousClip: 0,
        clipTime: 0,
        blend: 0,
        playbackRate: 1,
        timeOffset: 0,
        hitFlash: 0,
        holdLastFrame: false,
        lod: 0,
      });
    }
  }

  /** Claims a free slot, or null when at capacity. */
  spawn(position: Vector3, options: Partial<CrowdSlot> = {}): CrowdSlot | null {
    const slot = this.slots.find((candidate) => !candidate.active);
    if (!slot) return null;

    slot.active = true;
    slot.position.copy(position);
    slot.facing = options.facing ?? 0;
    slot.scale = options.scale ?? 1;
    slot.tint.copy(options.tint ?? new Vector3(1, 1, 1));
    slot.clip = options.clip ?? 0;
    slot.previousClip = slot.clip;
    slot.clipTime = 0;
    slot.blend = 0;
    slot.playbackRate = options.playbackRate ?? 1;
    slot.timeOffset = options.timeOffset ?? Math.random() * 4;
    slot.hitFlash = 0;
    slot.holdLastFrame = false;
    slot.lod = 0;

    return slot;
  }

  release(slot: CrowdSlot): void {
    slot.active = false;
  }

  /** Switches clip with a crossfade. Named for the simulation's convenience. */
  play(slot: CrowdSlot, name: ClipName, options: { hold?: boolean; rate?: number } = {}): void {
    const index = clipIndex(this.bake, name);
    if (slot.clip === index) return;

    slot.previousClip = slot.clip;
    slot.clip = index;
    slot.blend = BLEND_SECONDS;
    slot.clipTime = 0;
    slot.holdLastFrame = options.hold ?? false;
    if (options.rate !== undefined) slot.playbackRate = options.rate;
  }

  /** One tick of hit flash, cleared automatically on the next frame. */
  flash(slot: CrowdSlot): void {
    slot.hitFlash = 1;
  }

  /**
   * Rewrites every instance buffer. Called once per rendered frame with the
   * camera position, after the simulation has written slot positions.
   */
  sync(dt: number, cameraPosition: Vector3, torch: Torch | null): void {
    this.clock += dt;
    this.materials.clock.value = this.clock;

    this.near.count = 0;
    this.mid.count = 0;
    this.far.count = 0;

    let active = 0;

    for (const slot of this.slots) {
      if (!slot.active) continue;
      active += 1;

      slot.clipTime += dt * slot.playbackRate;
      if (slot.blend > 0) slot.blend = Math.max(0, slot.blend - dt);

      const distance = slot.position.distanceTo(cameraPosition);
      slot.lod = this.selectLod(slot, distance, torch);

      const bucket = slot.lod === 0 ? this.near : slot.lod === 1 ? this.mid : this.far;
      const index = bucket.count;

      // Billboards face the camera; the mesh LODs face their travel direction.
      let yaw = slot.facing;
      if (slot.lod === 2) {
        yaw = Math.atan2(cameraPosition.x - slot.position.x, cameraPosition.z - slot.position.z);
      }

      quaternion.setFromAxisAngle(yAxis, yaw);
      scaleVector.setScalar(slot.scale);
      matrix.compose(slot.position, quaternion, scaleVector);
      bucket.mesh.setMatrixAt(index, matrix);
      bucket.mesh.setColorAt(index, colourFrom(slot.tint));

      const clip = this.bake.clips[slot.clip] ?? this.bake.clips[0];
      const previous = this.bake.clips[slot.previousClip] ?? clip;

      bucket.anim.setXYZW(
        index,
        clip.startRow,
        clip.frameCount,
        slot.playbackRate,
        slot.timeOffset,
      );

      bucket.animPrevious.setXYZW(
        index,
        previous.startRow,
        previous.frameCount,
        slot.blend / BLEND_SECONDS,
        slot.timeOffset,
      );

      const inCone = torch?.isInCone(slot.position) ?? false;
      const flags = (slot.hitFlash > 0 ? 1 : 0) + (inCone ? 2 : 0) + (slot.holdLastFrame ? 4 : 0);
      bucket.flags.setX(index, flags);

      if (bucket.tile) {
        // Pick an atlas tile from the facing relative to the camera and a pose
        // stepped from the animation clock.
        const relative = yaw - slot.facing;
        const yawTile = Math.round(
          ((relative / (Math.PI * 2)) * BILLBOARD_YAWS + BILLBOARD_YAWS) % BILLBOARD_YAWS,
        );
        const poseTile = Math.floor((slot.clipTime * 6) % BILLBOARD_POSES);
        bucket.tile.setXY(index, yawTile % BILLBOARD_YAWS, poseTile);
      }

      bucket.count += 1;

      // Hit flash lasts exactly one frame, matching the one tick emissive
      // punch the material expects.
      slot.hitFlash = 0;
    }

    this.commit(this.near);
    this.commit(this.mid);
    this.commit(this.far);

    this.near.mesh.castShadow = this.castShadows;

    this.counts.near = this.near.count;
    this.counts.mid = this.mid.count;
    this.counts.far = this.far.count;
    this.counts.active = active;
  }

  private selectLod(slot: CrowdSlot, distance: number, torch: Torch | null): 0 | 1 | 2 {
    const [nearEdge, midEdge] = this.lodDistances;

    // Outside the torch cone in a blackout there is nothing to see, so drop
    // straight to an impostor whatever the distance.
    if (torch && !torch.isInCone(slot.position, 1.2) && distance > nearEdge * 0.5) {
      return slot.lod === 2 ? 2 : distance > nearEdge ? 2 : slot.lod;
    }

    const hysteresis = LOD_HYSTERESIS;

    if (slot.lod === 0) return distance > nearEdge + hysteresis ? 1 : 0;
    if (slot.lod === 1) {
      if (distance < nearEdge - hysteresis) return 0;
      return distance > midEdge + hysteresis ? 2 : 1;
    }
    return distance < midEdge - hysteresis ? 1 : 2;
  }

  private commit(bucket: Bucket): void {
    bucket.mesh.count = bucket.count;
    bucket.mesh.instanceMatrix.needsUpdate = true;
    if (bucket.mesh.instanceColor) bucket.mesh.instanceColor.needsUpdate = true;
    bucket.anim.needsUpdate = true;
    bucket.animPrevious.needsUpdate = true;
    bucket.flags.needsUpdate = true;
    if (bucket.tile) bucket.tile.needsUpdate = true;
  }

  dispose(): void {
    for (const bucket of [this.near, this.mid, this.far]) {
      this.scene.remove(bucket.mesh);
      bucket.mesh.geometry.dispose();
      bucket.mesh.dispose();
    }
    this.materials.dispose();
    this.billboards.dispose();
  }
}

const scratchColour = new Color();

function colourFrom(tint: Vector3): Color {
  return scratchColour.setRGB(tint.x, tint.y, tint.z);
}

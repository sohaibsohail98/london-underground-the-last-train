/**
 * Vertex animation texture materials.
 *
 * The pose is read from the baked texture in the vertex shader. Each instance
 * carries its own clip, time offset and playback rate in an instanced
 * attribute, so a crowd of 46 in one draw call still has 46 different
 * animation states rather than marching in step.
 *
 * No clip lookup table exists in the shader on purpose. The CPU writes the
 * clip's start row and frame count straight into the instance attribute, which
 * removes the only piece of dynamic indexing the shader would otherwise need
 * and keeps both backends on the same code path.
 */

import { Color } from 'three';
import { MeshLambertNodeMaterial, MeshStandardNodeMaterial } from 'three/webgpu';
import {
  attribute,
  float,
  mix,
  texture as textureNode,
  uniform,
  vec2,
  vec3,
  vec4,
  vertexIndex,
} from 'three/tsl';
import type { Node } from 'three/webgpu';
import { VAT_FPS, type VatBake } from './VatBaker';

/**
 * aAnim  = clip start row, clip frame count, playback rate, time offset
 * aAnimB = previous clip start row, previous frame count, blend weight, unused
 * aFlags = bit 0 hit flash, bit 1 inside the torch cone, bit 2 hold last frame
 */
export const CROWD_ATTRIBUTES = {
  anim: 'aAnim',
  animPrevious: 'aAnimB',
  flags: 'aFlags',
} as const;

export interface VatMaterialSet {
  near: MeshStandardNodeMaterial;
  mid: MeshLambertNodeMaterial;
  /** Global animation clock, advanced by the crowd each frame. */
  clock: { value: number };
  dispose(): void;
}

export function createVatMaterials(bake: VatBake): VatMaterialSet {
  const clock = uniform(0);
  const fps = float(VAT_FPS);
  const rowCount = float(bake.rowCount);
  const vertexColumns = float(bake.vertexCount);

  // attribute() is typed loosely, so the swizzles are asserted here once
  // rather than at every use.
  const anim = attribute(CROWD_ATTRIBUTES.anim, 'vec4') as unknown as Node<'vec4'>;
  const animPrevious = attribute(CROWD_ATTRIBUTES.animPrevious, 'vec4') as unknown as Node<'vec4'>;
  const flags = attribute(CROWD_ATTRIBUTES.flags, 'float') as unknown as Node<'float'>;

  // Column for this vertex, at texel centre so nearest sampling lands exactly
  // on the intended texel rather than on a boundary.
  const column = float(vertexIndex).add(0.5).div(vertexColumns);

  /** Samples a texture at a given clip row, interpolating between frames. */
  const sampleClip = (
    map: typeof bake.positionTexture,
    startRow: Node<'float'>,
    frameCount: Node<'float'>,
    rate: Node<'float'>,
    offset: Node<'float'>,
    hold: Node<'float'>,
  ) => {
    const raw = clock.mul(rate).add(offset).mul(fps);

    // Looping clips wrap; one shot clips clamp to the last frame, which is how
    // the death pose is held before the entity is recycled.
    const looped = raw.mod(frameCount);
    const clamped = raw.min(frameCount.sub(1.001));
    const frame = mix(looped, clamped, hold);

    const frameA = frame.floor();
    const frameB = frameA.add(1).min(frameCount.sub(1));
    const blend = frame.sub(frameA);

    const rowA = startRow.add(frameA).add(0.5).div(rowCount);
    const rowB = startRow.add(frameB).add(0.5).div(rowCount);

    const a = textureNode(map, vec2(column, rowA));
    const b = textureNode(map, vec2(column, rowB));

    return mix(a.xyz, b.xyz, blend);
  };

  const hold = flags.div(4).floor().mod(2);

  const currentPosition = sampleClip(bake.positionTexture, anim.x, anim.y, anim.z, anim.w, hold);
  const currentNormal = sampleClip(bake.normalTexture, anim.x, anim.y, anim.z, anim.w, hold);

  // Crossfade out of the previous clip, so shamble to attack does not pop. The
  // weight is decayed on the CPU over 0.15s.
  const previousPosition = sampleClip(
    bake.positionTexture,
    animPrevious.x,
    animPrevious.y,
    anim.z,
    animPrevious.w,
    float(0),
  );

  const blendedPosition = mix(currentPosition, previousPosition, animPrevious.z);

  const hitFlash = flags.mod(2);
  const tint = vec3(1, 1, 1);

  const near = new MeshStandardNodeMaterial();
  near.name = 'crowd-vat-near';
  near.positionNode = blendedPosition;
  near.normalNode = currentNormal.normalize();
  near.colorNode = tint;
  near.roughnessNode = float(0.78);
  near.metalnessNode = float(0.02);
  // A one tick emissive punch on being hit, which reads as a flinch even at
  // distances where the stagger animation is not legible.
  near.emissiveNode = vec3(1, 0.42, 0.38).mul(hitFlash).mul(1.6);

  const mid = new MeshLambertNodeMaterial();
  mid.name = 'crowd-vat-mid';
  // Mid LOD skips the crossfade and the normal texture: one sample instead of
  // four, which is the whole point of the LOD.
  mid.positionNode = currentPosition;
  // Lambert has no emissive slot, so the hit flash is folded into base colour
  // at this LOD. Visually identical at mid distance.
  mid.colorNode = vec4(tint.mul(0.9).add(vec3(1, 0.42, 0.38).mul(hitFlash)), 1);

  void Color;

  return {
    near,
    mid,
    clock,
    dispose(): void {
      near.dispose();
      mid.dispose();
    },
  };
}

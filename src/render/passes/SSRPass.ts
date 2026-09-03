/**
 * Screen space reflections, restricted to wet floors.
 *
 * SSR is the single dearest stage in the chain, so it is masked twice: the
 * node itself skips rough surfaces, and the result is additionally gated by a
 * roughness threshold so only genuinely wet ground reflects. Dry concrete
 * reflecting the ceiling looks wrong and costs the same as looking right.
 */

import { ssr } from 'three/addons/tsl/display/SSRNode.js';
import { smoothstep, uniform } from 'three/tsl';
import type { Camera } from 'three';
import type { Node } from 'three/webgpu';
import type { QualityPreset } from '../Presets';

export interface SsrInputs {
  colourNode: Node<'vec4'>;
  depthNode: Node<'float'> | Node<'vec4'>;
  normalNode: Node<'vec3'>;
  metalnessNode: Node<'float'>;
  roughnessNode: Node<'float'>;
}

export function createSsr(inputs: SsrInputs, camera: Camera, preset: QualityPreset) {
  const node = ssr(inputs.colourNode, inputs.depthNode, inputs.normalNode, {
    metalnessNode: inputs.metalnessNode,
    roughnessNode: inputs.roughnessNode,
    reflectNonMetals: true,
    binaryRefine: true,
    camera,
  });

  node.resolutionScale = preset.ssrResolutionScale;
  node.quality.value = preset.ssrQuality;
  node.maxDistance.value = 18;
  node.thickness.value = 0.12;
  node.intensity.value = 1;
  node.screenEdgeFade.value = 0.25;

  const strength = uniform(0.85);

  // Only surfaces below the roughness threshold contribute, which in practice
  // means the wet mask painted by the floor emitter.
  const wetMask = smoothstep(0.34, 0.1, inputs.roughnessNode);
  const contribution = node.rgb.mul(wetMask).mul(strength);

  return {
    name: 'ssr',
    contribution,
    strength,
    quality: node.quality,
    maxDistance: node.maxDistance,
    setSize: (width: number, height: number) => node.setSize(width, height),
  };
}

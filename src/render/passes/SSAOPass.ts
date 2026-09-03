/**
 * Screen space ambient occlusion, via three's ground truth AO node. AO is
 * applied multiplicatively to the lit HDR colour rather than to ambient only,
 * which is not strictly correct but reads far better in a space this dark.
 */

import { ao } from 'three/addons/tsl/display/GTAONode.js';
import { float, mix, uniform } from 'three/tsl';
import type { Camera } from 'three';
import type { Node } from 'three/webgpu';
import type { QualityPreset } from '../Presets';

export function createSsao(depthNode: Node, normalNode: Node, camera: Camera, preset: QualityPreset) {
  const node = ao(depthNode, normalNode, camera);

  node.resolutionScale = preset.ssaoResolutionScale;
  node.samples.value = preset.ssaoSamples;
  node.radius.value = 0.4;
  node.thickness.value = 1;
  node.distanceExponent.value = 1.6;
  node.distanceFallOff.value = 1;
  node.scale.value = 1;

  const intensity = uniform(0.95);

  // Lerping from white toward the AO term keeps the strength tunable at
  // runtime without rebuilding the pipeline.
  const factor = mix(float(1), node.clamp(0, 1), intensity);

  return {
    name: 'ssao',
    factor,
    intensity,
    radius: node.radius,
    samples: node.samples,
    setSize: (width: number, height: number) => node.setSize(width, height),
  };
}

/**
 * Bloom. Restrained by default: the threshold sits just above the brightest
 * unlit surface so only emissive strips, the torch hotspot and muzzle flashes
 * bloom. Anything more and a dark station turns to soup.
 *
 * The preset's mip count is expressed as a resolution scale rather than a mip
 * count, because three's bloom node has a fixed mip chain. The visual result
 * is the same: fewer effective mips at Low.
 */

import { bloom } from 'three/addons/tsl/display/BloomNode.js';
import { uniform } from 'three/tsl';
import type { Node } from 'three/webgpu';
import type { QualityPreset } from '../Presets';

export function createBloom(colourNode: Node<'vec4'>, preset: QualityPreset) {
  const strength = uniform(0.42);
  const radius = uniform(0.62);
  const threshold = uniform(1.05);

  const node = bloom(colourNode, strength, radius, threshold);
  node.setResolutionScale(preset.bloomMips >= 5 ? 1 : 0.5);

  return {
    name: 'bloom',
    node,
    strength,
    radius,
    threshold,
    setSize: (width: number, height: number) => node.setSize(width, height),
  };
}

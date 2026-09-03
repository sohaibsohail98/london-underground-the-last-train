/**
 * A small unsharp mask, last before present. Post chains this heavy soften the
 * image; this puts the edges back without introducing ringing.
 */

import { sharpen } from 'three/addons/tsl/display/SharpenNode.js';
import { uniform } from 'three/tsl';
import type { TextureNode } from 'three/webgpu';

export function createSharpen(inputNode: TextureNode) {
  const sharpness = uniform(0.3);
  const node = sharpen(inputNode, sharpness, false);

  return {
    name: 'sharpen',
    node,
    sharpness,
    setSize: (width: number, height: number) => node.setSize(width, height),
  };
}

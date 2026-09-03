/**
 * Chromatic aberration, zero in the middle of the frame and ramping toward the
 * edges, so it reads as lens character rather than a colour fault.
 */

import { Vector2 } from 'three';
import { chromaticAberration } from 'three/addons/tsl/display/ChromaticAberrationNode.js';
import { uniform } from 'three/tsl';
import type { TextureNode } from 'three/webgpu';

export function createChromatic(inputNode: TextureNode) {
  const strength = uniform(0.9);
  const scale = uniform(1.25);
  const centre = uniform(new Vector2(0.5, 0.5));

  const node = chromaticAberration(inputNode, strength, centre, scale);

  return { name: 'chromatic', node, strength, scale, centre };
}

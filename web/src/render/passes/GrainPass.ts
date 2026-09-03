/**
 * Film grain. Always present, because its intensity is one of the two damage
 * feedback channels: as health drops the grain ramps, which reads as the
 * player's vision degrading without needing a single HUD element.
 */

import { film } from 'three/addons/tsl/display/FilmNode.js';
import { float, uniform } from 'three/tsl';
import type { Node } from 'three/webgpu';

export function createGrain(inputNode: Node<'vec4'>) {
  /** Baseline grain with the player at full health. */
  const baseIntensity = uniform(0.09);
  /** Extra grain at zero health. */
  const damageIntensity = uniform(0.34);
  /** 0 at full health, 1 at death. Written by the health system. */
  const damageAmount = uniform(0);

  const intensity = baseIntensity.add(damageIntensity.mul(damageAmount.pow(float(1.6))));
  const node = film(inputNode, intensity);

  return { name: 'grain', node, baseIntensity, damageIntensity, damageAmount };
}

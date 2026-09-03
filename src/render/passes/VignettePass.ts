/**
 * Vignette, plus the crimson damage pulse.
 *
 * The pulse is the primary damage signal in the game: a red radial wash that
 * beats faster as health falls. Health bars are secondary. Written by hand
 * rather than taken from an addon because the pulse shape matters.
 */

import { Color } from 'three';
import { clamp, float, mix, oscSine, screenUV, smoothstep, uniform, vec2, vec4 } from 'three/tsl';
import type { Node } from 'three/webgpu';

export function createVignette(inputNode: Node<'vec4'>) {
  /** Base vignette darkness at the corners, 0..1. */
  const baseStrength = uniform(0.42);
  /** Where the base vignette starts, in normalised radius. */
  const baseRadius = uniform(0.58);
  /** 0 at full health, 1 at death. */
  const damageAmount = uniform(0);
  /** One shot flash, decayed on the CPU per hit taken. */
  const hitFlash = uniform(0);
  const damageColour = uniform(new Color(0xb02030));

  const centred = vec2(screenUV.x.sub(0.5).mul(2), screenUV.y.sub(0.5).mul(2));
  const radius = centred.length().mul(0.72);

  const base = smoothstep(baseRadius, float(1.25), radius).mul(baseStrength);

  // The pulse speeds up as health drops: roughly 1Hz at half health, 2.4Hz on
  // the edge of death.
  const pulseRate = float(1).add(damageAmount.mul(1.4));
  const pulse = oscSine(pulseRate).mul(0.5).add(0.5).mul(0.55).add(0.45);

  const damageBand = smoothstep(float(0.18), float(1.05), radius);
  const damageMask = clamp(damageAmount.pow(float(1.4)).mul(pulse).add(hitFlash), 0, 1);

  const darkened = inputNode.rgb.mul(float(1).sub(base));
  const withDamage = mix(darkened, damageColour, damageBand.mul(damageMask).mul(0.85));

  return {
    name: 'vignette',
    node: vec4(withDamage, inputNode.a),
    baseStrength,
    baseRadius,
    damageAmount,
    hitFlash,
    damageColour,
  };
}

/**
 * Volumetric fog and torch scattering.
 *
 * A single raymarch from the camera to the depth buffer, accumulating
 * in-scattered light from the torch cone plus a uniform ambient fog term. This
 * is what makes the torch read as a beam in the air rather than a projected
 * texture on the floor, so it is worth its cost.
 *
 * Step count is a compile time constant taken from the quality preset, which
 * means changing preset rebuilds the pipeline. That is deliberate: a dynamic
 * loop bound costs measurably more on both backends, and presets change once
 * per session rather than once per frame.
 *
 * Known limitation, accepted for now: the march does not sample the torch
 * shadow map, so the beam is not occluded by geometry along its own length,
 * only by scene depth along the view ray. In an enclosed station with the
 * torch pointing away from the camera this is very rarely visible. Sampling
 * the spotlight shadow map from a post pass is a Phase 8 optimisation item.
 */

import { Vector3 } from 'three';
import {
  Loop,
  cameraPosition,
  clamp,
  exp,
  float,
  hash,
  max,
  screenCoordinate,
  smoothstep,
  time,
  uniform,
  vec3,
  vec4,
} from 'three/tsl';
import type { Node } from 'three/webgpu';
import type { Torch } from '../Torch';
import { worldPositionFromViewZ } from './Reconstruct';

export interface VolumetricOptions {
  steps: number;
  /** Furthest the march will travel, in metres. */
  maxDistance?: number;
}

export function createVolumetric(
  viewZNode: Node<'float'>,
  torch: Torch,
  options: VolumetricOptions,
) {
  const steps = Math.max(4, Math.round(options.steps));
  const maxDistance = options.maxDistance ?? 42;

  /** Fog density per metre at floor level. */
  const density = uniform(0.042);
  /** How quickly density falls off with height, per metre. */
  const heightFalloff = uniform(0.11);
  /** Height at which density is at its stated value. */
  const floorHeight = uniform(0);
  /** Ambient in-scatter, the dull glow of a lit tunnel with no torch. */
  const ambientColour = uniform(new Vector3(0.055, 0.05, 0.062));
  /** Henyey Greenstein anisotropy. Positive values scatter forward. */
  const anisotropy = uniform(0.62);
  /** Overall multiplier on the torch contribution. */
  const torchScatter = uniform(1.35);

  const node = (() => {
    const world = worldPositionFromViewZ(viewZNode);
    const origin = cameraPosition;

    const rayVector = world.sub(origin);
    const distance = clamp(rayVector.length(), 0.05, maxDistance);
    const direction = rayVector.normalize();
    const stepSize = distance.div(float(steps));

    // Per pixel, per frame jitter of the first sample. Without this the march
    // bands badly at low step counts; with it the banding becomes noise, which
    // the film grain then hides.
    const jitter = hash(
      screenCoordinate.x
        .mul(1973)
        .add(screenCoordinate.y.mul(9277))
        .add(time.mul(60).floor().mul(26699)),
    );

    const scattered = vec3(0).toVar('volScattered');
    const transmittance = float(1).toVar('volTransmittance');

    const g = anisotropy;
    const gSquared = g.mul(g);

    Loop({ start: 0, end: steps, type: 'int', condition: '<' }, ({ i }) => {
      const t = stepSize.mul(float(i).add(jitter));
      const samplePoint = origin.add(direction.mul(t));

      // Height fogged density: thickest at floor level, thinning upward.
      const heightTerm = exp(samplePoint.y.sub(floorHeight).mul(heightFalloff).negate());
      const localDensity = density.mul(clamp(heightTerm, 0, 1.6));
      const sigma = localDensity.mul(stepSize);

      // Torch cone contribution.
      const toTorch = torch.uniforms.position.sub(samplePoint);
      const torchDistance = max(toTorch.length(), 0.05);
      const toTorchDir = toTorch.div(torchDistance);

      const coneCos = toTorchDir.negate().dot(torch.uniforms.direction);
      const cone = smoothstep(torch.uniforms.cosOuter, torch.uniforms.cosInner, coneCos);

      const falloff = clamp(float(1).sub(torchDistance.div(torch.uniforms.range)), 0, 1);
      const attenuation = falloff
        .mul(falloff)
        .div(torchDistance.mul(torchDistance).mul(0.35).add(1));

      // Henyey Greenstein phase, so looking down the beam is brighter than
      // looking across it.
      const cosTheta = direction.dot(toTorchDir.negate());
      const phaseDenominator = float(1).add(gSquared).sub(g.mul(2).mul(cosTheta));
      const phase = float(1)
        .sub(gSquared)
        .div(phaseDenominator.mul(phaseDenominator.sqrt()).mul(12.566).max(0.001));

      const torchInscatter = torch.uniforms.colour
        .mul(torch.uniforms.intensity.mul(0.02))
        .mul(cone)
        .mul(attenuation)
        .mul(phase)
        .mul(torchScatter)
        .mul(torch.uniforms.enabled);

      const inscatter = torchInscatter.add(ambientColour);

      scattered.addAssign(inscatter.mul(sigma).mul(transmittance));
      transmittance.mulAssign(exp(sigma.negate()));
    });

    // rgb is in-scattered light, alpha is the transmittance of the scene
    // behind it, so the composite is colour * a + rgb.
    return vec4(scattered, transmittance);
  })();

  return {
    name: 'volumetric',
    steps,
    node,
    density,
    heightFalloff,
    floorHeight,
    ambientColour,
    anisotropy,
    torchScatter,
  };
}

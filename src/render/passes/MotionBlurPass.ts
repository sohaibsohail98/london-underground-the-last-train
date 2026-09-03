/**
 * Camera motion blur.
 *
 * Only camera motion is blurred, never object motion: with 46 zombies on
 * screen, per object velocity would need a velocity buffer for the crowd and
 * the payoff at this camera distance is negligible. Reprojecting last frame's
 * view projection against reconstructed world position gives a per pixel
 * velocity that covers camera pans, shake and the FOV punch.
 */

import { Matrix4 } from 'three';
import { motionBlur } from 'three/addons/tsl/display/MotionBlur.js';
import { screenUV, uniform, vec2, vec4 } from 'three/tsl';
import type { Camera } from 'three';
import type { Node } from 'three/webgpu';
import { worldPositionFromViewZ } from './Reconstruct';

export function createMotionBlur(
  inputNode: Node<'vec4'>,
  viewZNode: Node<'float'>,
  camera: Camera,
  samples: number,
) {
  const previousViewProjection = uniform(new Matrix4());
  const strength = uniform(0.65);

  const world = worldPositionFromViewZ(viewZNode);
  const previousClip = previousViewProjection.mul(vec4(world.x, world.y, world.z, 1));
  const previousUv = previousClip.xy.div(previousClip.w).mul(0.5).add(0.5);

  const velocity = vec2(screenUV.x.sub(previousUv.x), screenUV.y.sub(previousUv.y))
    .mul(strength)
    .clamp(-0.06, 0.06);

  const node = motionBlur(inputNode, velocity, uniform(Math.max(2, samples)));

  const scratch = new Matrix4();

  return {
    name: 'motionBlur',
    node,
    strength,
    /** Call at the very end of every rendered frame, after the camera moves. */
    capture: () => {
      scratch.multiplyMatrices(camera.projectionMatrix, camera.matrixWorldInverse);
      previousViewProjection.value.copy(scratch);
    },
  };
}

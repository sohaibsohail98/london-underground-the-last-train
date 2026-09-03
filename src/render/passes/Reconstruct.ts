/**
 * Shared reconstruction helpers.
 *
 * Depth conventions differ between the WebGPU and WebGL2 backends, so nothing
 * here touches raw depth buffer values. Everything is derived from the pass
 * node's view Z, which three normalises for us, which keeps both backends on
 * one code path.
 */

import { cameraProjectionMatrixInverse, cameraWorldMatrix, screenUV, vec2, vec4 } from 'three/tsl';
import type { Node } from 'three/webgpu';

/**
 * A point on the view ray through the current fragment, in view space. The
 * result is not normalised; it is scaled later against view Z.
 */
export function viewRay() {
  const ndc = vec2(screenUV.x.mul(2).sub(1), screenUV.y.mul(2).sub(1));
  const unprojected = cameraProjectionMatrixInverse.mul(vec4(ndc.x, ndc.y, 0.5, 1));
  return unprojected.xyz.div(unprojected.w);
}

/** View space position of the fragment, given the pass node's view Z. */
export function viewPositionFromViewZ(viewZ: Node<'float'>) {
  const ray = viewRay();
  // Scale the ray so that its z component equals the sampled view Z. Both are
  // negative in front of the camera, so the ratio is positive.
  return ray.mul(viewZ.div(ray.z));
}

/** World space position of the fragment, given the pass node's view Z. */
export function worldPositionFromViewZ(viewZ: Node<'float'>) {
  const view = viewPositionFromViewZ(viewZ);
  return cameraWorldMatrix.mul(vec4(view.x, view.y, view.z, 1)).xyz;
}

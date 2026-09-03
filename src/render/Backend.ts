/**
 * Backend selection. WebGPU is the primary target; WebGL2 is a genuine
 * fallback that degrades the post chain rather than disabling it, so every
 * pass still runs, just at lower sample counts.
 */

import { ACESFilmicToneMapping, PCFSoftShadowMap } from 'three';
import { WebGPURenderer } from 'three/webgpu';

export type BackendKind = 'webgpu' | 'webgl2';

export interface BackendCapabilities {
  kind: BackendKind;
  /** GPU timestamp queries, used by the profiler for per pass costs. */
  timestamps: boolean;
  /** Compute shaders, used by the GPU particle simulation from Phase 3 on. */
  compute: boolean;
  /** Half float render targets. Required; we refuse to boot without them. */
  halfFloatTargets: boolean;
  /** Human readable adapter string where the browser exposes one. */
  adapter: string;
}

export interface BackendResult {
  renderer: WebGPURenderer;
  capabilities: BackendCapabilities;
}

async function webgpuAvailable(): Promise<boolean> {
  const gpu = (navigator as Navigator & { gpu?: { requestAdapter(): Promise<unknown> } }).gpu;
  if (!gpu) return false;

  try {
    const adapter = await gpu.requestAdapter();
    return adapter !== null && adapter !== undefined;
  } catch {
    return false;
  }
}

function describeAdapter(canvas: HTMLCanvasElement, kind: BackendKind): string {
  if (kind === 'webgpu') return 'WebGPU adapter';

  try {
    const gl = canvas.getContext('webgl2');
    if (!gl) return 'unknown';
    const info = gl.getExtension('WEBGL_debug_renderer_info');
    if (!info) return 'WebGL2';
    return String(gl.getParameter(info.UNMASKED_RENDERER_WEBGL));
  } catch {
    return 'WebGL2';
  }
}

/**
 * Creates and initialises the renderer. Resolves only once the device is
 * ready, so callers never have to deal with the async init path again.
 */
export async function createBackend(canvas: HTMLCanvasElement): Promise<BackendResult> {
  const wantsWebGPU = await webgpuAvailable();

  const renderer = new WebGPURenderer({
    canvas,
    antialias: false, // we resolve aliasing in the post chain, not with MSAA
    alpha: false,
    forceWebGL: !wantsWebGPU,
    trackTimestamp: true,
    powerPreference: 'high-performance',
  });

  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  renderer.setSize(window.innerWidth, window.innerHeight, false);
  renderer.toneMapping = ACESFilmicToneMapping;
  renderer.toneMappingExposure = 1;
  renderer.shadowMap.enabled = true;
  renderer.shadowMap.type = PCFSoftShadowMap;

  await renderer.init();

  const backend = renderer.backend as unknown as { isWebGPUBackend?: boolean };
  const kind: BackendKind = backend.isWebGPUBackend === true ? 'webgpu' : 'webgl2';

  const halfFloatTargets =
    kind === 'webgpu' ? true : await renderer.hasFeatureAsync('EXT_color_buffer_float').catch(() => false);

  return {
    renderer,
    capabilities: {
      kind,
      timestamps: kind === 'webgpu',
      compute: kind === 'webgpu',
      halfFloatTargets: halfFloatTargets || kind === 'webgpu',
      adapter: describeAdapter(canvas, kind),
    },
  };
}

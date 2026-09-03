/**
 * Quality presets. These are the only place render cost is dialled; individual
 * post stages remain separately toggleable from the settings screen, but the
 * preset sets their defaults. Numbers match the Phase 1 preset table.
 */

import type { BackendCapabilities } from './Backend';

export type PresetName = 'low' | 'medium' | 'high';

export interface QualityPreset {
  name: PresetName;
  label: string;

  /** Torch shadow map resolution in pixels, square. */
  shadowMapSize: number;
  /** Whether crowd entities cast into the torch shadow map at all. */
  crowdShadows: boolean;

  ssaoEnabled: boolean;
  ssaoSamples: number;
  ssaoResolutionScale: number;

  ssrEnabled: boolean;
  ssrResolutionScale: number;
  ssrQuality: number;

  volumetricEnabled: boolean;
  volumetricSteps: number;
  volumetricResolutionScale: number;

  bloomEnabled: boolean;
  bloomMips: number;

  motionBlurEnabled: boolean;
  motionBlurSamples: number;

  chromaticEnabled: boolean;
  sharpenEnabled: boolean;

  /** Grain and vignette are always on because they carry damage feedback. */
  pointLightBudget: number;
  crowdLodDistances: [number, number];
  zombieCap: number;

  /** Render scale applied to the whole frame before upscaling to the canvas. */
  renderScale: number;
}

export const PRESETS: Record<PresetName, QualityPreset> = {
  low: {
    name: 'low',
    label: 'Low',
    shadowMapSize: 1024,
    crowdShadows: false,
    ssaoEnabled: true,
    ssaoSamples: 8,
    ssaoResolutionScale: 0.5,
    ssrEnabled: false,
    ssrResolutionScale: 0.5,
    ssrQuality: 0.35,
    volumetricEnabled: true,
    volumetricSteps: 16,
    volumetricResolutionScale: 0.25,
    bloomEnabled: true,
    bloomMips: 3,
    motionBlurEnabled: false,
    motionBlurSamples: 4,
    chromaticEnabled: false,
    sharpenEnabled: false,
    pointLightBudget: 4,
    crowdLodDistances: [8, 14],
    zombieCap: 30,
    renderScale: 0.85,
  },
  medium: {
    name: 'medium',
    label: 'Medium',
    shadowMapSize: 2048,
    crowdShadows: true,
    ssaoEnabled: true,
    ssaoSamples: 16,
    ssaoResolutionScale: 0.5,
    ssrEnabled: true,
    ssrResolutionScale: 0.5,
    ssrQuality: 0.5,
    volumetricEnabled: true,
    volumetricSteps: 32,
    volumetricResolutionScale: 0.5,
    bloomEnabled: true,
    bloomMips: 5,
    motionBlurEnabled: true,
    motionBlurSamples: 4,
    chromaticEnabled: true,
    sharpenEnabled: true,
    pointLightBudget: 8,
    crowdLodDistances: [12, 20],
    zombieCap: 46,
    renderScale: 1,
  },
  high: {
    name: 'high',
    label: 'High',
    shadowMapSize: 4096,
    crowdShadows: true,
    ssaoEnabled: true,
    ssaoSamples: 24,
    ssaoResolutionScale: 1,
    ssrEnabled: true,
    ssrResolutionScale: 1,
    ssrQuality: 0.7,
    volumetricEnabled: true,
    volumetricSteps: 64,
    volumetricResolutionScale: 0.5,
    bloomEnabled: true,
    bloomMips: 5,
    motionBlurEnabled: true,
    motionBlurSamples: 8,
    chromaticEnabled: true,
    sharpenEnabled: true,
    pointLightBudget: 8,
    crowdLodDistances: [16, 26],
    zombieCap: 46,
    renderScale: 1,
  },
};

export const PRESET_ORDER: PresetName[] = ['low', 'medium', 'high'];

/**
 * Picks a starting preset. WebGL2 never starts above Medium, because the
 * fragment shader fallbacks for AO and volumetrics are considerably dearer
 * than the WebGPU compute paths.
 */
export function defaultPreset(capabilities: BackendCapabilities): PresetName {
  if (capabilities.kind === 'webgl2') return 'low';

  const cores = navigator.hardwareConcurrency ?? 4;
  return cores >= 8 ? 'high' : 'medium';
}

/**
 * WebGL2 cannot afford the same numbers. Rather than disabling stages we thin
 * them, which keeps the look consistent between backends.
 */
export function adaptForBackend(
  preset: QualityPreset,
  capabilities: BackendCapabilities,
): QualityPreset {
  if (capabilities.kind === 'webgpu') return preset;

  return {
    ...preset,
    ssaoSamples: Math.max(6, Math.round(preset.ssaoSamples * 0.5)),
    ssaoResolutionScale: Math.min(preset.ssaoResolutionScale, 0.5),
    volumetricSteps: Math.max(12, Math.round(preset.volumetricSteps * 0.5)),
    volumetricResolutionScale: Math.min(preset.volumetricResolutionScale, 0.5),
    ssrResolutionScale: Math.min(preset.ssrResolutionScale, 0.5),
    ssrQuality: Math.min(preset.ssrQuality, 0.45),
    shadowMapSize: Math.min(preset.shadowMapSize, 2048),
    motionBlurSamples: Math.min(preset.motionBlurSamples, 4),
  };
}

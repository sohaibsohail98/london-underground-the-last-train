/**
 * The render graph.
 *
 * One scene pass with a multi render target for depth, view normals, metalness
 * and roughness, then the post chain in the order fixed by the Phase 1 plan:
 *
 *   SSAO, SSR, volumetrics, bloom, tonemap, motion blur, chromatic aberration,
 *   grain, vignette, sharpen, present.
 *
 * Two classes of toggle exist, deliberately:
 *
 *   Cheap stages (motion blur, chromatic aberration, grain, vignette, sharpen)
 *   are toggled by uniform, so they switch instantly from the settings screen.
 *
 *   Expensive stages (SSAO, SSR, volumetrics, bloom) are toggled by rebuilding
 *   the chain, because the point of turning them off is to stop paying for
 *   them. A rebuild costs a shader recompile, which is why preset changes are
 *   a menu action rather than something the game does mid wave.
 */

import { ACESFilmicToneMapping, SRGBColorSpace, type Camera, type Scene } from 'three';
import { RenderPipeline, type Node, type TextureNode, type WebGPURenderer } from 'three/webgpu';
import {
  convertToTexture,
  metalness,
  mix,
  mrt,
  output,
  pass,
  renderOutput,
  roughness,
  transformedNormalView,
  uniform,
  vec4,
} from 'three/tsl';
import type { QualityPreset } from './Presets';
import type { Torch } from './Torch';
import { createSsao } from './passes/SSAOPass';
import { createSsr } from './passes/SSRPass';
import { createVolumetric } from './passes/VolumetricPass';
import { createBloom } from './passes/BloomPass';
import { createMotionBlur } from './passes/MotionBlurPass';
import { createChromatic } from './passes/ChromaticPass';
import { createGrain } from './passes/GrainPass';
import { createVignette } from './passes/VignettePass';
import { createSharpen } from './passes/SharpenPass';

export type StageName =
  | 'ssao'
  | 'ssr'
  | 'volumetric'
  | 'bloom'
  | 'motionBlur'
  | 'chromatic'
  | 'grain'
  | 'vignette'
  | 'sharpen';

/** Stages whose cost is only removed by rebuilding the chain. */
export const REBUILD_STAGES: StageName[] = ['ssao', 'ssr', 'volumetric', 'bloom'];

export type StageToggles = Record<StageName, boolean>;

export function togglesFromPreset(preset: QualityPreset): StageToggles {
  return {
    ssao: preset.ssaoEnabled,
    ssr: preset.ssrEnabled,
    volumetric: preset.volumetricEnabled,
    bloom: preset.bloomEnabled,
    motionBlur: preset.motionBlurEnabled,
    chromatic: preset.chromaticEnabled,
    grain: true,
    vignette: true,
    sharpen: preset.sharpenEnabled,
  };
}

export interface RenderGraphContext {
  renderer: WebGPURenderer;
  scene: Scene;
  camera: Camera;
  torch: Torch;
}

type Stages = {
  ssao?: ReturnType<typeof createSsao>;
  ssr?: ReturnType<typeof createSsr>;
  volumetric?: ReturnType<typeof createVolumetric>;
  bloom?: ReturnType<typeof createBloom>;
  motionBlur?: ReturnType<typeof createMotionBlur>;
  chromatic?: ReturnType<typeof createChromatic>;
  grain?: ReturnType<typeof createGrain>;
  vignette?: ReturnType<typeof createVignette>;
  sharpen?: ReturnType<typeof createSharpen>;
};

export class RenderGraph {
  /** Live handles on every built stage, for the settings screen. */
  stages: Stages = {};

  /** Runtime mix amounts for the cheap stages, 1 is fully applied. */
  readonly amounts = {
    motionBlur: uniform(1),
    chromatic: uniform(1),
    grain: uniform(1),
    vignette: uniform(1),
    sharpen: uniform(1),
  };

  /** Exposure, set per station. */
  readonly exposure = uniform(1);

  private readonly context: RenderGraphContext;
  private pipeline: RenderPipeline | null = null;
  private scenePass: ReturnType<typeof pass> | null = null;
  private preset: QualityPreset;
  private toggles: StageToggles;
  private width = 1;
  private height = 1;

  /** How many RTT materialisations the current chain needs, for the profiler. */
  materialisations = 0;

  constructor(context: RenderGraphContext, preset: QualityPreset) {
    this.context = context;
    this.preset = preset;
    this.toggles = togglesFromPreset(preset);
    this.build();
  }

  get activeStages(): StageName[] {
    return (Object.keys(this.stages) as StageName[]).filter((name) => this.stages[name] !== undefined);
  }

  get currentToggles(): StageToggles {
    return { ...this.toggles };
  }

  /** Rebuilds the whole chain. Costs a shader compile; not a per frame call. */
  build(): void {
    const { renderer, scene, camera, torch } = this.context;

    this.pipeline?.dispose();
    this.stages = {};
    this.materialisations = 0;

    const scenePass = pass(scene, camera);
    scenePass.setMRT(
      mrt({
        output,
        normal: transformedNormalView,
        metalness,
        roughness,
      }),
    );
    this.scenePass = scenePass;

    const sceneColour = scenePass.getTextureNode('output');
    const sceneDepth = scenePass.getTextureNode('depth');
    const sceneNormal = scenePass.getTextureNode('normal');
    const sceneMetalness = scenePass.getTextureNode('metalness');
    const sceneRoughness = scenePass.getTextureNode('roughness');
    const viewZ = scenePass.getViewZNode();

    let hdr: Node<'vec4'> = sceneColour;

    // 5 SSAO, multiplicative.
    if (this.toggles.ssao) {
      const stage = createSsao(sceneDepth, sceneNormal, camera, this.preset);
      this.stages.ssao = stage;
      hdr = vec4(hdr.rgb.mul(stage.factor), hdr.a);
    }

    // 6 SSR, additive and masked to wet floors.
    if (this.toggles.ssr) {
      const stage = createSsr(
        {
          colourNode: hdr,
          depthNode: sceneDepth,
          normalNode: sceneNormal.rgb,
          metalnessNode: sceneMetalness.r,
          roughnessNode: sceneRoughness.r,
        },
        camera,
        this.preset,
      );
      this.stages.ssr = stage;
      hdr = vec4(hdr.rgb.add(stage.contribution), hdr.a);
    }

    // 7 Volumetrics. rgb is in-scatter, a is transmittance of the scene behind.
    if (this.toggles.volumetric) {
      const stage = createVolumetric(viewZ, torch, {
        steps: Math.round(this.preset.volumetricSteps * this.preset.volumetricResolutionScale * 2),
      });
      this.stages.volumetric = stage;
      hdr = vec4(hdr.rgb.mul(stage.node.a).add(stage.node.rgb), hdr.a);
    }

    // 8 Bloom, additive in HDR before the tonemap.
    if (this.toggles.bloom) {
      const stage = createBloom(hdr, this.preset);
      this.stages.bloom = stage;
      hdr = vec4(hdr.rgb.add(stage.node.rgb), hdr.a);
    }

    // 9 Tonemap and transfer. Everything below this line is LDR.
    let ldr: Node<'vec4'> = renderOutput(
      hdr.mul(this.exposure),
      ACESFilmicToneMapping,
      SRGBColorSpace,
    ) as unknown as Node<'vec4'>;

    // 10 Motion blur. Samples with an offset, so it needs a real texture.
    if (this.toggles.motionBlur) {
      const source = convertToTexture(ldr) as unknown as TextureNode;
      this.materialisations += 1;
      const stage = createMotionBlur(source, viewZ, camera, this.preset.motionBlurSamples);
      this.stages.motionBlur = stage;
      ldr = mix(source, stage.node, this.amounts.motionBlur) as unknown as Node<'vec4'>;
    }

    // 11 Chromatic aberration. Also samples with an offset.
    if (this.toggles.chromatic) {
      const source = convertToTexture(ldr) as unknown as TextureNode;
      this.materialisations += 1;
      const stage = createChromatic(source);
      this.stages.chromatic = stage;
      ldr = mix(source, stage.node as unknown as Node<'vec4'>, this.amounts.chromatic) as unknown as Node<'vec4'>;
    }

    // 12 Grain, health driven.
    if (this.toggles.grain) {
      const stage = createGrain(ldr);
      this.stages.grain = stage;
      ldr = mix(ldr, stage.node as unknown as Node<'vec4'>, this.amounts.grain) as unknown as Node<'vec4'>;
    }

    // 13 Vignette plus the crimson damage pulse.
    if (this.toggles.vignette) {
      const stage = createVignette(ldr);
      this.stages.vignette = stage;
      ldr = mix(ldr, stage.node, this.amounts.vignette) as unknown as Node<'vec4'>;
    }

    // 14 Sharpen, last before present.
    if (this.toggles.sharpen) {
      const source = convertToTexture(ldr) as unknown as TextureNode;
      this.materialisations += 1;
      const stage = createSharpen(source);
      this.stages.sharpen = stage;
      ldr = mix(source, stage.node, this.amounts.sharpen) as unknown as Node<'vec4'>;
    }

    const pipeline = new RenderPipeline(renderer, ldr);
    // The tonemap and transfer are applied explicitly above so that the LDR
    // stages run in display space, as the plan requires.
    pipeline.outputColorTransform = false;
    this.pipeline = pipeline;

    this.setSize(this.width, this.height);
  }

  /** Applies a new preset, rebuilding only if an expensive stage changed. */
  setPreset(preset: QualityPreset): void {
    const previous = this.toggles;
    this.preset = preset;
    this.toggles = togglesFromPreset(preset);
    void previous;
    this.build();
  }

  /**
   * Toggles a stage. Cheap stages switch instantly; expensive ones trigger a
   * rebuild so their cost actually goes away.
   */
  setStageEnabled(name: StageName, enabled: boolean): void {
    if (this.toggles[name] === enabled) return;
    this.toggles[name] = enabled;

    if (REBUILD_STAGES.includes(name)) {
      this.build();
      return;
    }

    const amount = this.amounts[name as keyof typeof this.amounts];
    if (amount) amount.value = enabled ? 1 : 0;
  }

  setSize(width: number, height: number): void {
    this.width = Math.max(1, width);
    this.height = Math.max(1, height);

    this.scenePass?.setSize(this.width, this.height);
    this.stages.ssao?.setSize(this.width, this.height);
    this.stages.ssr?.setSize(this.width, this.height);
    this.stages.bloom?.setSize(this.width, this.height);
    this.stages.sharpen?.setSize(this.width, this.height);
  }

  /** Renders one frame through the chain. */
  render(): void {
    this.pipeline?.render();
    this.stages.motionBlur?.capture();
  }

  dispose(): void {
    this.pipeline?.dispose();
    this.pipeline = null;
    this.scenePass = null;
    this.stages = {};
  }
}

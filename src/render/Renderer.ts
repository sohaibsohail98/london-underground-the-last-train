/**
 * Renderer facade.
 *
 * Owns everything on the render side and exposes one render() call. The
 * simulation never touches anything in here directly: it writes interpolated
 * positions and emits events, and this module reads them. That separation is
 * what keeps the fixed timestep honest once the game systems land.
 */

import { Scene, Vector3 } from 'three';
import type { WebGPURenderer } from 'three/webgpu';
import { CameraRig } from '../engine/Camera';
import type { Clock } from '../engine/Clock';
import { createBackend, type BackendCapabilities } from './Backend';
import { Lighting, DEFAULT_ENVIRONMENT, type EnvironmentSettings } from './Lighting';
import { MaterialLibrary, type MaterialSet } from './Materials';
import { MuzzleFlash } from './MuzzleFlash';
import { Occlusion } from './Occlusion';
import { adaptForBackend, defaultPreset, PRESETS, type PresetName, type QualityPreset } from './Presets';
import { RenderGraph, type StageName } from './RenderGraph';
import { Torch } from './Torch';
import { Profiler } from './debug/Profiler';

export interface RendererHandles {
  renderer: WebGPURenderer;
  capabilities: BackendCapabilities;
  scene: Scene;
  rig: CameraRig;
  materials: MaterialLibrary;
  lighting: Lighting;
  torch: Torch;
  muzzle: MuzzleFlash;
  occlusion: Occlusion;
  graph: RenderGraph;
  profiler: Profiler;
}

export class GameRenderer {
  readonly scene = new Scene();

  renderer!: WebGPURenderer;
  capabilities!: BackendCapabilities;
  rig!: CameraRig;
  materials!: MaterialLibrary;
  lighting!: Lighting;
  torch!: Torch;
  muzzle!: MuzzleFlash;
  occlusion!: Occlusion;
  graph!: RenderGraph;
  profiler!: Profiler;

  presetName: PresetName = 'medium';
  preset!: QualityPreset;

  private readonly canvas: HTMLCanvasElement;
  private readonly overlayRoot: HTMLElement;
  private resizeQueued = false;

  constructor(canvas: HTMLCanvasElement, overlayRoot: HTMLElement) {
    this.canvas = canvas;
    this.overlayRoot = overlayRoot;
  }

  /** Boots the backend and builds the whole render side. */
  async init(onStatus: (text: string) => void = () => {}): Promise<void> {
    onStatus('Selecting a graphics backend');
    const { renderer, capabilities } = await createBackend(this.canvas);
    this.renderer = renderer;
    this.capabilities = capabilities;

    this.presetName = defaultPreset(capabilities);
    this.preset = adaptForBackend(PRESETS[this.presetName], capabilities);

    onStatus('Generating surface materials');
    this.materials = new MaterialLibrary(renderer);

    this.lighting = new Lighting(renderer, this.scene, this.preset.pointLightBudget);
    this.torch = new Torch(this.scene, { shadowMapSize: this.preset.shadowMapSize });
    this.muzzle = new MuzzleFlash(this.scene);
    this.occlusion = new Occlusion(this.materials);

    this.rig = new CameraRig(window.innerWidth / Math.max(1, window.innerHeight));

    onStatus('Compiling the post processing chain');
    this.graph = new RenderGraph(
      { renderer, scene: this.scene, camera: this.rig.camera, torch: this.torch },
      this.preset,
    );

    this.profiler = new Profiler(
      this.overlayRoot,
      renderer,
      capabilities,
      this.graph,
      this.lighting,
    );

    this.applySize();
    window.addEventListener('resize', this.onResize);
  }

  /** Loads a texture set group up front, so nothing pops in mid frame. */
  async preloadMaterials(sets: MaterialSet[]): Promise<void> {
    await this.materials.preload(sets);
  }

  async applyEnvironment(settings: EnvironmentSettings = DEFAULT_ENVIRONMENT): Promise<void> {
    await this.lighting.applyEnvironment(settings);
    this.graph.exposure.value = 1;
  }

  setPreset(name: PresetName): void {
    if (name === this.presetName) return;

    this.presetName = name;
    this.preset = adaptForBackend(PRESETS[name], this.capabilities);

    this.lighting.setBudget(this.preset.pointLightBudget);
    this.torch.setShadowMapSize(this.preset.shadowMapSize);
    this.graph.setPreset(this.preset);
    this.applySize();
  }

  /** Master brightness, wired to a settings slider in Phase 7. */
  setBrightness(value: number): void {
    this.lighting.setBrightness(value);
  }

  get brightness(): number {
    return this.lighting.currentBrightness;
  }

  setStageEnabled(name: StageName, enabled: boolean): void {
    this.graph.setStageEnabled(name, enabled);
  }

  /**
   * Renders one frame. Order matches the plan's render section: camera, then
   * lighting selection, then the torch, then occlusion, then the graph.
   */
  render(clock: Clock, focus: Vector3, aim: Vector3, driveCamera = true): void {
    const dt = Math.max(clock.frameDelta, 1e-4);

    this.profiler.beginFrame();

    this.rig.focus.copy(focus);
    this.rig.aim.copy(aim);

    // The debug fly camera writes the same camera transform directly, so the
    // spring arm is skipped rather than fighting it.
    if (driveCamera) this.rig.update(dt);

    this.lighting.update(this.rig.camera);
    this.lighting.syncPlayerFill(focus);
    this.torch.update(focus, aim, dt, clock.elapsed);
    this.muzzle.update(dt);
    this.occlusion.update(this.rig, dt);

    this.graph.render();

    this.profiler.endFrame(clock);
  }

  private onResize = (): void => {
    if (this.resizeQueued) return;
    this.resizeQueued = true;
    window.requestAnimationFrame(() => {
      this.resizeQueued = false;
      this.applySize();
    });
  };

  private applySize(): void {
    const width = Math.max(1, window.innerWidth);
    const height = Math.max(1, window.innerHeight);
    const scale = this.preset.renderScale;

    this.renderer.setSize(width, height, false);
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2) * scale);
    this.rig.setAspect(width / height);
    this.graph.setSize(Math.round(width * scale), Math.round(height * scale));
  }

  dispose(): void {
    window.removeEventListener('resize', this.onResize);
    this.profiler.dispose();
    this.graph.dispose();
    this.occlusion.clear();
    this.muzzle.dispose();
    this.torch.dispose();
    this.lighting.dispose();
    this.materials.dispose();
    this.renderer.dispose();
  }
}

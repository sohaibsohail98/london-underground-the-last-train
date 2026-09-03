/**
 * Lighting rig.
 *
 * Underground stations get no directional light at all. Everything comes from
 * an HDRI environment for cheap indirect, emissive strip lighting for the
 * shape of the space, and a small pool of real point lights that follows the
 * camera. The point light budget is the hard cost lever: the scene registers
 * as many light candidates as it likes and the rig activates only the nearest
 * few, so a large station costs the same as a small one.
 */

import {
  AmbientLight,
  Color,
  DataTexture,
  DirectionalLight,
  EquirectangularReflectionMapping,
  FloatType,
  PMREMGenerator,
  PointLight,
  RGBAFormat,
  Scene,
  Vector3,
  type Camera,
  type Texture,
} from 'three';
import { RGBELoader } from 'three/addons/loaders/RGBELoader.js';
import { uniform } from 'three/tsl';
import type { WebGPURenderer } from 'three/webgpu';

export interface LightCandidate {
  position: Vector3;
  colour: Color;
  /** Intensity in candela before the environment multiplier. */
  intensity: number;
  /** Metres. Lights beyond this from the camera are never considered. */
  range: number;
  /** Killed by the blackout mechanic. Safety lighting is not. */
  blackoutSensitive: boolean;
}

export interface EnvironmentSettings {
  /** File under assets/hdri, without a path. */
  file: string;
  /** Multiplier on environment lighting. Underground values sit near 0.15. */
  intensity: number;
  /** Tint applied to the environment, as a hex colour. */
  tint: number;
  /** Whether this station gets a sun. Surface and open air only. */
  surface: boolean;
}

export const DEFAULT_ENVIRONMENT: EnvironmentSettings = {
  file: 'sodium_interior.hdr',
  intensity: 0.18,
  tint: 0xe0a030,
  surface: false,
};

/** Builds a small equirectangular gradient used when no HDRI is present. */
function proceduralEnvironment(tint: number): DataTexture {
  const width = 64;
  const height = 32;
  const data = new Float32Array(width * height * 4);
  const sky = new Color(tint).multiplyScalar(0.35);
  const ground = new Color(0x0a0a0e);

  for (let y = 0; y < height; y += 1) {
    // Bias the horizon downward so most of the energy arrives from above,
    // which is how a tunnel lit by ceiling strips actually behaves.
    const t = Math.pow(1 - y / (height - 1), 1.6);
    const r = ground.r + (sky.r - ground.r) * t;
    const g = ground.g + (sky.g - ground.g) * t;
    const b = ground.b + (sky.b - ground.b) * t;

    for (let x = 0; x < width; x += 1) {
      const i = (y * width + x) * 4;
      data[i] = r;
      data[i + 1] = g;
      data[i + 2] = b;
      data[i + 3] = 1;
    }
  }

  const texture = new DataTexture(data, width, height, RGBAFormat, FloatType);
  texture.mapping = EquirectangularReflectionMapping;
  texture.needsUpdate = true;
  return texture;
}

export class Lighting {
  /** Multiplies every emissive strip material. Blackout drives this to zero. */
  readonly emissiveScale = uniform(1);

  /** True while the blackout mechanic is active. */
  blackout = false;

  private readonly renderer: WebGPURenderer;
  private readonly scene: Scene;
  private readonly pmrem: PMREMGenerator;
  private readonly pool: PointLight[] = [];
  private readonly candidates: LightCandidate[] = [];
  private readonly sun: DirectionalLight;
  private readonly cameraPosition = new Vector3();

  /**
   * Readability floor. A pitch black station is atmospheric for about thirty
   * seconds and then it is simply unplayable: you cannot read a horde you
   * cannot see. This ambient term is deliberately not physically motivated.
   * It exists so that geometry, zombie silhouettes and the platform edge are
   * always legible, with the torch providing contrast on top rather than
   * being the only source of information.
   */
  private readonly ambient: AmbientLight;

  /**
   * A dim light carried with the player, independent of the torch. Even with
   * the torch off or during a blackout, the player's immediate surroundings
   * stay readable, which is what stops a blackout round becoming a guessing
   * game.
   */
  private readonly playerFill: PointLight;

  /** Master brightness in 0..1, exposed as a setting. */
  private brightness = 1;

  private environmentTexture: Texture | null = null;
  private settings: EnvironmentSettings = DEFAULT_ENVIRONMENT;
  private budget = 8;
  private ranked: { candidate: LightCandidate; distance: number }[] = [];

  /** How many pool lights were actually lit last frame, for the profiler. */
  activeLights = 0;

  constructor(renderer: WebGPURenderer, scene: Scene, budget = 8) {
    this.renderer = renderer;
    this.scene = scene;
    // PMREMGenerator is typed against WebGLRenderer but works with any renderer
    // exposing the same render surface, which WebGPURenderer does.
    this.pmrem = new PMREMGenerator(
      renderer as unknown as ConstructorParameters<typeof PMREMGenerator>[0],
    );

    this.ambient = new AmbientLight(0xb8c2d8, 0.55);
    scene.add(this.ambient);

    this.playerFill = new PointLight(0xffe3bc, 5.5, 9, 2);
    this.playerFill.castShadow = false;
    scene.add(this.playerFill);

    this.sun = new DirectionalLight(0xbfd0e4, 0);
    this.sun.position.set(-24, 40, 18);
    this.sun.castShadow = false;
    this.sun.visible = false;
    scene.add(this.sun);

    this.setBudget(budget);
  }

  /** Resizes the point light pool. Called on preset change. */
  setBudget(budget: number): void {
    this.budget = budget;

    while (this.pool.length < budget) {
      const light = new PointLight(0xffffff, 0, 18, 2);
      light.castShadow = false;
      light.visible = false;
      this.scene.add(light);
      this.pool.push(light);
    }

    while (this.pool.length > budget) {
      const light = this.pool.pop();
      if (light) {
        this.scene.remove(light);
        light.dispose();
      }
    }
  }

  /**
   * Applies a station's environment. Falls back to a generated gradient if the
   * HDRI is missing, so a station is always lit even with an empty assets
   * folder.
   */
  async applyEnvironment(settings: EnvironmentSettings): Promise<void> {
    this.settings = settings;

    this.environmentTexture?.dispose();
    this.environmentTexture = null;

    let source: Texture;

    try {
      source = await new RGBELoader().loadAsync(`assets/hdri/${settings.file}`);
      source.mapping = EquirectangularReflectionMapping;
    } catch {
      source = proceduralEnvironment(settings.tint);
    }

    const target = this.pmrem.fromEquirectangular(source);
    source.dispose();

    this.environmentTexture = target.texture;
    this.scene.environment = target.texture;
    this.scene.environmentRotation.set(0, 0, 0);
    this.applyLevels();

    this.sun.visible = settings.surface;
    this.sun.intensity = settings.surface ? 1.6 : 0;

    this.scene.backgroundIntensity = settings.surface ? 0.4 : 0;
    this.scene.background = settings.surface ? target.texture : null;
  }

  /** Registers a light candidate. The geometry generator calls this per strip. */
  register(candidate: LightCandidate): void {
    this.candidates.push(candidate);
  }

  /** Clears all candidates, called when tearing a station down. */
  clearCandidates(): void {
    this.candidates.length = 0;
    for (const light of this.pool) light.visible = false;
    this.activeLights = 0;
  }

  get candidateCount(): number {
    return this.candidates.length;
  }

  /**
   * Master brightness, 0.4 to 1.6. Exposed in settings because the acceptable
   * level for a dark game varies enormously between panels, and asking the
   * player to play a horror game they cannot see is not a design decision.
   */
  setBrightness(value: number): void {
    this.brightness = Math.min(1.6, Math.max(0.4, value));
    this.applyLevels();
  }

  get currentBrightness(): number {
    return this.brightness;
  }

  /**
   * Kills or restores emissives and blackout sensitive lights. Note that the
   * ambient floor and the player fill survive a blackout at reduced strength:
   * a blackout should feel like losing the station lighting, not like losing
   * the ability to see.
   */
  setBlackout(active: boolean): void {
    this.blackout = active;
    this.applyLevels();
  }

  private applyLevels(): void {
    const blackoutScale = this.blackout ? 0.22 : 1;

    this.emissiveScale.value = this.blackout ? 0 : this.brightness;
    this.ambient.intensity = 0.55 * this.brightness * blackoutScale;
    this.playerFill.intensity = 5.5 * this.brightness * (this.blackout ? 0.75 : 1);
    this.scene.environmentIntensity =
      this.settings.intensity * this.brightness * (this.blackout ? 0.12 : 1);
  }

  /** Follows the player. Called once per rendered frame. */
  syncPlayerFill(position: Vector3): void {
    this.playerFill.position.set(position.x, position.y + 1.5, position.z);
  }

  /**
   * Selects the nearest candidates to the camera and binds them to the pool.
   * Runs once per rendered frame; the sort is over candidate count, which for
   * the largest station is a few hundred entries.
   */
  update(camera: Camera): void {
    this.cameraPosition.setFromMatrixPosition(camera.matrixWorld);

    this.ranked.length = 0;

    for (const candidate of this.candidates) {
      if (this.blackout && candidate.blackoutSensitive) continue;

      const distance = candidate.position.distanceTo(this.cameraPosition);
      if (distance > candidate.range + 24) continue;

      this.ranked.push({ candidate, distance });
    }

    this.ranked.sort((a, b) => a.distance - b.distance);

    const count = Math.min(this.budget, this.ranked.length);

    for (let i = 0; i < this.pool.length; i += 1) {
      const light = this.pool[i];

      if (i >= count) {
        light.visible = false;
        continue;
      }

      const entry = this.ranked[i];
      light.position.copy(entry.candidate.position);
      light.color.copy(entry.candidate.colour);
      light.distance = entry.candidate.range;

      // Fade in at the edge of the budget so a light does not pop on as the
      // player walks along a platform.
      const edge = 1 - Math.min(1, Math.max(0, (entry.distance - entry.candidate.range) / 12));
      light.intensity = entry.candidate.intensity * edge;
      light.visible = light.intensity > 0.01;
    }

    this.activeLights = count;
  }

  dispose(): void {
    this.clearCandidates();
    for (const light of this.pool) {
      this.scene.remove(light);
      light.dispose();
    }
    this.pool.length = 0;
    this.scene.remove(this.sun);
    this.sun.dispose();
    this.scene.remove(this.ambient);
    this.scene.remove(this.playerFill);
    this.playerFill.dispose();
    this.environmentTexture?.dispose();
    this.pmrem.dispose();
    void this.renderer;
  }
}

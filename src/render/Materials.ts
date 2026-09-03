/**
 * Material factory. Every surface in the game is one of a small set of PBR
 * material classes, so they share texture sets and can be merged aggressively
 * by the geometry generator in Phase 3.
 *
 * Texture sets are loaded from /assets as KTX2. Until those binaries are in
 * place the factory falls back to procedurally generated maps of the same
 * shape, so the render pipeline is testable with an empty assets folder.
 */

import {
  CanvasTexture,
  Color,
  LinearSRGBColorSpace,
  RepeatWrapping,
  SRGBColorSpace,
  type Texture,
} from 'three';
import { MeshStandardNodeMaterial } from 'three/webgpu';
import { KTX2Loader } from 'three/addons/loaders/KTX2Loader.js';
import {
  clamp,
  float,
  fract,
  mix,
  screenCoordinate,
  texture as textureNode,
  uniform,
  uv,
  vec2,
  vec3,
} from 'three/tsl';
import type { WebGPURenderer } from 'three/webgpu';

export type MaterialSet =
  | 'concrete'
  | 'tile'
  | 'wet_tile'
  | 'steel'
  | 'painted_brick'
  | 'rubber'
  | 'glass';

export interface SurfaceOptions {
  /** Which texture set to bind. */
  set: MaterialSet;
  /** UV repeats per metre. The generator writes UVs in metres. */
  tilesPerMetre?: number;
  /** Baseline wetness 0..1, raised locally by the floor emitter's vertex colour. */
  wetness?: number;
  /** Tint multiplied into albedo, for per station palette accents. */
  tint?: number;
  /** Whether this surface participates in the dither fade for camera occlusion. */
  occludable?: boolean;
  /** Force a roughness floor, useful for glass and rails. */
  roughnessBias?: number;
}

interface TextureSet {
  albedo: Texture;
  normal: Texture;
  roughness: Texture;
}

const SET_TINTS: Record<MaterialSet, number> = {
  concrete: 0x4a4a52,
  tile: 0xb9b3a8,
  wet_tile: 0x8f8d88,
  steel: 0x6e737a,
  painted_brick: 0x5c4b46,
  rubber: 0x24242a,
  glass: 0x9fb2bd,
};

const SET_ROUGHNESS: Record<MaterialSet, number> = {
  concrete: 0.86,
  tile: 0.42,
  wet_tile: 0.18,
  steel: 0.35,
  painted_brick: 0.72,
  rubber: 0.9,
  glass: 0.05,
};

const SET_METALNESS: Record<MaterialSet, number> = {
  concrete: 0,
  tile: 0,
  wet_tile: 0,
  steel: 0.9,
  painted_brick: 0,
  rubber: 0,
  glass: 0.1,
};

function hash2(x: number, y: number, seed: number): number {
  let h = x * 374761393 + y * 668265263 + seed * 2147483647;
  h = (h ^ (h >>> 13)) * 1274126177;
  return ((h ^ (h >>> 16)) >>> 0) / 4294967295;
}

/** Value noise on a grid, good enough for surface break-up at this camera distance. */
function valueNoise(x: number, y: number, seed: number): number {
  const xi = Math.floor(x);
  const yi = Math.floor(y);
  const xf = x - xi;
  const yf = y - yi;
  const sx = xf * xf * (3 - 2 * xf);
  const sy = yf * yf * (3 - 2 * yf);

  const a = hash2(xi, yi, seed);
  const b = hash2(xi + 1, yi, seed);
  const c = hash2(xi, yi + 1, seed);
  const d = hash2(xi + 1, yi + 1, seed);

  return (a * (1 - sx) + b * sx) * (1 - sy) + (c * (1 - sx) + d * sx) * sy;
}

function fbm(x: number, y: number, seed: number, octaves: number): number {
  let sum = 0;
  let amplitude = 0.5;
  let frequency = 1;
  for (let i = 0; i < octaves; i += 1) {
    sum += valueNoise(x * frequency, y * frequency, seed + i * 17) * amplitude;
    amplitude *= 0.5;
    frequency *= 2;
  }
  return sum;
}

/** Draws a grouted tile lattice, used by the tile and brick sets. */
function drawLattice(
  data: Uint8ClampedArray,
  size: number,
  cols: number,
  rows: number,
  offsetPerRow: number,
  groutWidth: number,
  base: Color,
  grout: Color,
  seed: number,
): void {
  const cellW = size / cols;
  const cellH = size / rows;

  for (let y = 0; y < size; y += 1) {
    const row = Math.floor(y / cellH);
    const shift = (row * offsetPerRow * cellW) % size;

    for (let x = 0; x < size; x += 1) {
      const sx = (x + shift) % size;
      const inGroutX = sx % cellW < groutWidth || cellW - (sx % cellW) < groutWidth;
      const inGroutY = y % cellH < groutWidth || cellH - (y % cellH) < groutWidth;
      const isGrout = inGroutX || inGroutY;

      const col = Math.floor(sx / cellW);
      const perTile = hash2(col, row, seed) * 0.14 - 0.07;
      const grime = fbm(x / 24, y / 24, seed + 5, 3) * 0.22 - 0.11;

      const source = isGrout ? grout : base;
      const shade = 1 + perTile + grime;

      const i = (y * size + x) * 4;
      data[i] = source.r * 255 * shade;
      data[i + 1] = source.g * 255 * shade;
      data[i + 2] = source.b * 255 * shade;
      data[i + 3] = 255;
    }
  }
}

function makeAlbedo(set: MaterialSet, size: number): CanvasTexture {
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('2D context unavailable for procedural texture generation');

  const image = context.createImageData(size, size);
  const data = image.data;
  const base = new Color(SET_TINTS[set]);
  const seed = set.length * 31;

  if (set === 'tile' || set === 'wet_tile') {
    drawLattice(data, size, 16, 16, 0, 2, base, new Color(0x3b3a37), seed);
  } else if (set === 'painted_brick') {
    drawLattice(data, size, 8, 20, 0.5, 2, base, new Color(0x2e2724), seed);
  } else {
    for (let y = 0; y < size; y += 1) {
      for (let x = 0; x < size; x += 1) {
        const n = fbm(x / 40, y / 40, seed, 4);
        const grit = hash2(x, y, seed + 3) * 0.08 - 0.04;
        const shade = 0.82 + n * 0.36 + grit;
        const i = (y * size + x) * 4;
        data[i] = base.r * 255 * shade;
        data[i + 1] = base.g * 255 * shade;
        data[i + 2] = base.b * 255 * shade;
        data[i + 3] = 255;
      }
    }
  }

  context.putImageData(image, 0, 0);

  const texture = new CanvasTexture(canvas);
  texture.colorSpace = SRGBColorSpace;
  texture.wrapS = RepeatWrapping;
  texture.wrapT = RepeatWrapping;
  texture.generateMipmaps = true;
  texture.anisotropy = 4;
  return texture;
}

function makeNormal(set: MaterialSet, size: number): CanvasTexture {
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('2D context unavailable for procedural texture generation');

  const image = context.createImageData(size, size);
  const data = image.data;
  const seed = set.length * 71;
  const strength = set === 'painted_brick' ? 2.6 : set === 'concrete' ? 1.6 : 1;

  const height = (x: number, y: number): number => fbm(x / 18, y / 18, seed, 3);

  for (let y = 0; y < size; y += 1) {
    for (let x = 0; x < size; x += 1) {
      const dx = (height(x + 1, y) - height(x - 1, y)) * strength;
      const dy = (height(x, y + 1) - height(x, y - 1)) * strength;
      const len = Math.hypot(dx, dy, 1);
      const i = (y * size + x) * 4;
      data[i] = ((-dx / len) * 0.5 + 0.5) * 255;
      data[i + 1] = ((-dy / len) * 0.5 + 0.5) * 255;
      data[i + 2] = (1 / len) * 0.5 * 255 + 127;
      data[i + 3] = 255;
    }
  }

  context.putImageData(image, 0, 0);

  const texture = new CanvasTexture(canvas);
  texture.colorSpace = LinearSRGBColorSpace;
  texture.wrapS = RepeatWrapping;
  texture.wrapT = RepeatWrapping;
  return texture;
}

function makeRoughness(set: MaterialSet, size: number): CanvasTexture {
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('2D context unavailable for procedural texture generation');

  const image = context.createImageData(size, size);
  const data = image.data;
  const seed = set.length * 113;
  const base = SET_ROUGHNESS[set];

  for (let y = 0; y < size; y += 1) {
    for (let x = 0; x < size; x += 1) {
      const n = fbm(x / 30, y / 30, seed, 3);
      const value = Math.max(0, Math.min(1, base + (n - 0.5) * 0.3)) * 255;
      const i = (y * size + x) * 4;
      data[i] = value;
      data[i + 1] = value;
      data[i + 2] = value;
      data[i + 3] = 255;
    }
  }

  context.putImageData(image, 0, 0);

  const texture = new CanvasTexture(canvas);
  texture.colorSpace = LinearSRGBColorSpace;
  texture.wrapS = RepeatWrapping;
  texture.wrapT = RepeatWrapping;
  return texture;
}

export class MaterialLibrary {
  /** Global 0..1 dither fade applied to surfaces marked occludable. */
  readonly occlusionFade = uniform(0);

  /** Global wetness multiplier, raised by the flood mechanic. */
  readonly wetnessScale = uniform(1);

  private readonly sets = new Map<MaterialSet, TextureSet>();
  private readonly cache = new Map<string, MeshStandardNodeMaterial>();
  private readonly ktx2: KTX2Loader;
  private readonly loadedFromDisk = new Set<MaterialSet>();

  constructor(renderer: WebGPURenderer) {
    this.ktx2 = new KTX2Loader().setTranscoderPath('assets/basis/').detectSupport(renderer);
  }

  /** True where a real KTX2 set was found rather than the procedural fallback. */
  usingDiskTextures(set: MaterialSet): boolean {
    return this.loadedFromDisk.has(set);
  }

  /**
   * Loads a texture set, preferring KTX2 from /assets and falling back to
   * generated maps. Safe to call repeatedly; results are cached.
   */
  async loadSet(set: MaterialSet): Promise<TextureSet> {
    const existing = this.sets.get(set);
    if (existing) return existing;

    const paths = {
      albedo: `assets/textures/${set}/albedo.ktx2`,
      normal: `assets/textures/${set}/normal.ktx2`,
      roughness: `assets/textures/${set}/roughness.ktx2`,
    };

    let result: TextureSet;

    try {
      const [albedo, normal, roughness] = await Promise.all([
        this.ktx2.loadAsync(paths.albedo),
        this.ktx2.loadAsync(paths.normal),
        this.ktx2.loadAsync(paths.roughness),
      ]);

      albedo.colorSpace = SRGBColorSpace;
      for (const texture of [albedo, normal, roughness]) {
        texture.wrapS = RepeatWrapping;
        texture.wrapT = RepeatWrapping;
      }

      result = { albedo, normal, roughness };
      this.loadedFromDisk.add(set);
    } catch {
      const size = 512;
      result = {
        albedo: makeAlbedo(set, size),
        normal: makeNormal(set, size),
        roughness: makeRoughness(set, size),
      };
    }

    this.sets.set(set, result);
    return result;
  }

  /** Preloads every set used by the current station. */
  async preload(sets: MaterialSet[]): Promise<void> {
    await Promise.all(sets.map((set) => this.loadSet(set)));
  }

  /**
   * Builds a surface material. Identical option objects share one material so
   * the generator's merged geometry stays inside the draw call budget.
   */
  surface(options: SurfaceOptions): MeshStandardNodeMaterial {
    const {
      set,
      tilesPerMetre = 0.5,
      wetness = 0,
      tint = 0xffffff,
      occludable = false,
      roughnessBias = 0,
    } = options;

    const key = `${set}|${tilesPerMetre}|${wetness}|${tint}|${occludable}|${roughnessBias}`;
    const cached = this.cache.get(key);
    if (cached) return cached;

    const textures = this.sets.get(set);
    if (!textures) {
      throw new Error(`Material set "${set}" was requested before it was loaded`);
    }

    const material = new MeshStandardNodeMaterial();
    material.name = `surface:${set}`;

    const repeat = vec2(tilesPerMetre, tilesPerMetre);
    const scaledUv = uv().mul(repeat);

    const albedoSample = textureNode(textures.albedo, scaledUv);
    const normalSample = textureNode(textures.normal, scaledUv);
    const roughnessSample = textureNode(textures.roughness, scaledUv);

    // Vertex colour red channel carries locally painted wetness from the floor
    // emitter, so puddles can be authored per tile without a second texture.
    const localWet = clamp(float(wetness).mul(this.wetnessScale), float(0), float(1));

    material.colorNode = albedoSample.rgb
      .mul(vec3(new Color(tint).r, new Color(tint).g, new Color(tint).b))
      // Wet surfaces darken as well as smooth out.
      .mul(mix(float(1), float(0.62), localWet));

    material.normalNode = normalSample.rgb.mul(2).sub(1).normalize();

    material.roughnessNode = clamp(
      mix(roughnessSample.r.add(roughnessBias), float(0.06), localWet),
      0.02,
      1,
    );

    material.metalnessNode = float(SET_METALNESS[set]);

    if (occludable) {
      // Dithered fade rather than alpha blending: fragments are discarded in a
      // screen space pattern, so the surface still writes depth and costs
      // nothing extra. Interleaved gradient noise reads better than an ordered
      // 4x4 matrix at partial fade and needs no lookup table.
      material.maskNode = this.ditherThreshold().greaterThanEqual(this.occlusionFade);
    }

    if (set === 'glass') {
      material.transparent = true;
      material.opacity = 0.32;
    }

    this.cache.set(key, material);
    return material;
  }

  /** Screen space interleaved gradient noise threshold in 0..1. */
  private ditherThreshold() {
    const coord = screenCoordinate.xy;
    const gradient = coord.x.mul(0.06711056).add(coord.y.mul(0.00583715));
    return fract(fract(gradient).mul(52.9829189));
  }

  dispose(): void {
    for (const set of this.sets.values()) {
      set.albedo.dispose();
      set.normal.dispose();
      set.roughness.dispose();
    }
    for (const material of this.cache.values()) material.dispose();
    this.sets.clear();
    this.cache.clear();
    this.ktx2.dispose();
  }
}

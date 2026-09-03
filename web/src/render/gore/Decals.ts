/**
 * Blood decals.
 *
 * A fixed pool of instanced quads, projected onto whatever surface the hit
 * ray stopped against. Nothing is ever allocated after construction: the pool
 * recycles the oldest decal, so a long round cannot leak memory or draw calls.
 * The whole system is one draw call regardless of how many decals are live.
 */

import {
  CanvasTexture,
  Color,
  DynamicDrawUsage,
  InstancedBufferAttribute,
  InstancedMesh,
  Matrix4,
  PlaneGeometry,
  Quaternion,
  SRGBColorSpace,
  Scene,
  Vector3,
} from 'three';
import { MeshBasicNodeMaterial } from 'three/webgpu';
import { attribute, texture as textureNode, uv, vec4 } from 'three/tsl';

/** Pool size. 256 is generous: at 46 zombies this is several rounds of hits. */
export const DECAL_POOL = 256;

/** Seconds a decal takes to fade out entirely. */
const DECAL_LIFETIME = 20;

interface DecalSlot {
  active: boolean;
  age: number;
  lifetime: number;
}

/**
 * Draws a splatter: one main blob, a ring of satellites and a few streaks.
 * Alpha carries the shape so the same texture works on any surface colour.
 */
function makeSplatter(size = 128, seed = 1): CanvasTexture {
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('2D context unavailable for the decal texture');

  context.clearRect(0, 0, size, size);

  let state = seed * 2654435761;
  const random = (): number => {
    state = (state ^ (state << 13)) >>> 0;
    state = (state ^ (state >>> 17)) >>> 0;
    state = (state ^ (state << 5)) >>> 0;
    return state / 4294967295;
  };

  const centre = size / 2;
  const draw = (x: number, y: number, radius: number, alpha: number): void => {
    const gradient = context.createRadialGradient(x, y, 0, x, y, radius);
    gradient.addColorStop(0, `rgba(150,16,22,${alpha})`);
    gradient.addColorStop(0.62, `rgba(110,10,16,${alpha * 0.82})`);
    gradient.addColorStop(1, 'rgba(70,6,10,0)');
    context.fillStyle = gradient;
    context.beginPath();
    context.arc(x, y, radius, 0, Math.PI * 2);
    context.fill();
  };

  draw(centre, centre, size * 0.3, 0.95);

  for (let i = 0; i < 14; i += 1) {
    const angle = random() * Math.PI * 2;
    const distance = size * (0.16 + random() * 0.26);
    draw(
      centre + Math.cos(angle) * distance,
      centre + Math.sin(angle) * distance,
      size * (0.02 + random() * 0.07),
      0.5 + random() * 0.4,
    );
  }

  // Streaks, which is what stops every decal reading as a circle.
  for (let i = 0; i < 5; i += 1) {
    const angle = random() * Math.PI * 2;
    const length = size * (0.14 + random() * 0.24);
    context.strokeStyle = `rgba(120,12,18,${0.35 + random() * 0.3})`;
    context.lineWidth = size * (0.008 + random() * 0.018);
    context.beginPath();
    context.moveTo(centre, centre);
    context.lineTo(centre + Math.cos(angle) * length, centre + Math.sin(angle) * length);
    context.stroke();
  }

  const texture = new CanvasTexture(canvas);
  texture.colorSpace = SRGBColorSpace;
  return texture;
}

const matrix = new Matrix4();
const quaternion = new Quaternion();
const scaleVector = new Vector3();
const up = new Vector3(0, 0, 1);

export class Decals {
  private readonly mesh: InstancedMesh;
  private readonly slots: DecalSlot[] = [];
  private readonly fade: InstancedBufferAttribute;
  private readonly texture: CanvasTexture;
  private readonly scene: Scene;
  private cursor = 0;

  /** Live decal count, for the profiler. */
  live = 0;

  constructor(scene: Scene) {
    this.scene = scene;
    this.texture = makeSplatter(128, 7);

    const material = new MeshBasicNodeMaterial();
    const sample = textureNode(this.texture, uv());
    const decalFade = attribute<'float'>('aFade', 'float');

    material.colorNode = vec4(sample.rgb, sample.a.mul(decalFade));
    material.transparent = true;
    material.depthWrite = false;
    // Decals sit fractionally proud of the surface they are projected onto;
    // the offset is applied on placement rather than with polygon offset so
    // both backends behave identically.
    material.side = 2;

    const geometry = new PlaneGeometry(1, 1);
    this.fade = new InstancedBufferAttribute(new Float32Array(DECAL_POOL), 1);
    this.fade.setUsage(DynamicDrawUsage);
    geometry.setAttribute('aFade', this.fade);

    this.mesh = new InstancedMesh(geometry, material, DECAL_POOL);
    this.mesh.instanceMatrix.setUsage(DynamicDrawUsage);
    this.mesh.frustumCulled = false;
    this.mesh.count = 0;
    this.mesh.renderOrder = 2;
    this.mesh.name = 'blood-decals';
    scene.add(this.mesh);

    for (let i = 0; i < DECAL_POOL; i += 1) {
      this.slots.push({ active: false, age: 0, lifetime: DECAL_LIFETIME });
      this.fade.setX(i, 0);
    }
  }

  /**
   * Places a decal. The normal is the surface normal at the impact point; pass
   * the floor normal when a hit stops in mid air, which is what happens when a
   * zombie dies without a wall behind it.
   */
  place(point: Vector3, normal: Vector3, size = 0.55, seedRotation = Math.random()): void {
    // Round robin rather than a free list: the oldest decal is always the one
    // reused, which is what you want visually.
    const index = this.cursor;
    this.cursor = (this.cursor + 1) % DECAL_POOL;

    const slot = this.slots[index];
    slot.active = true;
    slot.age = 0;
    slot.lifetime = DECAL_LIFETIME * (0.7 + Math.random() * 0.6);

    quaternion.setFromUnitVectors(up, normal.clone().normalize());
    // Random roll about the surface normal, so repeated hits on one wall do not
    // stamp the same texture orientation.
    const roll = new Quaternion().setFromAxisAngle(
      normal.clone().normalize(),
      seedRotation * Math.PI * 2,
    );
    quaternion.premultiply(roll);

    const scale = size * (0.75 + Math.random() * 0.7);
    scaleVector.set(scale, scale, scale);

    const offsetPoint = point.clone().addScaledVector(normal, 0.012);
    matrix.compose(offsetPoint, quaternion, scaleVector);
    this.mesh.setMatrixAt(index, matrix);
    this.fade.setX(index, 1);

    this.mesh.count = DECAL_POOL;
    this.mesh.instanceMatrix.needsUpdate = true;
    this.fade.needsUpdate = true;
  }

  /** Convenience for a hit with no surface information: splatter downward. */
  placeOnFloor(point: Vector3, floorY = 0): void {
    const at = point.clone();
    at.y = floorY;
    this.place(at, new Vector3(0, 1, 0), 0.7);
  }

  update(dt: number): void {
    let live = 0;
    let dirty = false;

    for (let i = 0; i < DECAL_POOL; i += 1) {
      const slot = this.slots[i];
      if (!slot.active) continue;

      slot.age += dt;

      if (slot.age >= slot.lifetime) {
        slot.active = false;
        this.fade.setX(i, 0);
        dirty = true;
        continue;
      }

      // Hold full opacity for the first 60 per cent of life, then fade.
      const t = slot.age / slot.lifetime;
      const value = t < 0.6 ? 1 : 1 - (t - 0.6) / 0.4;
      this.fade.setX(i, value);
      dirty = true;
      live += 1;
    }

    this.live = live;
    if (dirty) this.fade.needsUpdate = true;
  }

  /** Wipes every decal, used on travelling to a new station. */
  clear(): void {
    for (let i = 0; i < DECAL_POOL; i += 1) {
      this.slots[i].active = false;
      this.fade.setX(i, 0);
    }
    this.fade.needsUpdate = true;
    this.mesh.count = 0;
    this.live = 0;
  }

  dispose(): void {
    this.scene.remove(this.mesh);
    this.mesh.geometry.dispose();
    (this.mesh.material as MeshBasicNodeMaterial).dispose();
    this.texture.dispose();
  }

  /** Kept so the tint can be palette shifted per station if wanted. */
  static readonly bloodColour = new Color(0xb02030);
}

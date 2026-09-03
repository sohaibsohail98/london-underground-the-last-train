/**
 * The torch. This is the signature effect of the whole game, so it is a real
 * spotlight with a shadow map, a cookie texture and volumetric scattering, not
 * a fog of war circle. It must read as a light: the cone should be visible in
 * the air, it should throw hard shadows from geometry, and it should sway as
 * the player moves.
 */

import {
  CanvasTexture,
  Color,
  LinearFilter,
  MathUtils,
  Object3D,
  SRGBColorSpace,
  Scene,
  SpotLight,
  Vector3,
} from 'three';
import { uniform } from 'three/tsl';

export interface TorchOptions {
  shadowMapSize?: number;
  /** Cone half angle in degrees. */
  angleDeg?: number;
  /** Effective range in metres. */
  range?: number;
  intensity?: number;
  colour?: number;
}

const DEFAULTS: Required<TorchOptions> = {
  shadowMapSize: 2048,
  angleDeg: 26,
  range: 26,
  intensity: 46,
  colour: 0xfff0d2,
};

/**
 * Draws the cookie: a slightly irregular hot centre with a soft edge and two
 * faint diffraction rings, which is what stops the cone looking like a clean
 * mathematical shape.
 */
function makeCookie(size = 256): CanvasTexture {
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('2D context unavailable for the torch cookie');

  context.fillStyle = '#000000';
  context.fillRect(0, 0, size, size);

  const centre = size / 2;

  const core = context.createRadialGradient(centre, centre, 0, centre, centre, centre);
  core.addColorStop(0, 'rgba(255,255,255,1)');
  core.addColorStop(0.42, 'rgba(255,255,255,0.92)');
  core.addColorStop(0.72, 'rgba(255,255,255,0.38)');
  core.addColorStop(0.92, 'rgba(255,255,255,0.06)');
  core.addColorStop(1, 'rgba(255,255,255,0)');
  context.fillStyle = core;
  context.fillRect(0, 0, size, size);

  context.globalCompositeOperation = 'lighter';
  for (const [radius, alpha] of [
    [0.55, 0.05],
    [0.78, 0.035],
  ] as const) {
    context.beginPath();
    context.arc(centre, centre, centre * radius, 0, Math.PI * 2);
    context.lineWidth = size * 0.012;
    context.strokeStyle = `rgba(255,240,210,${alpha})`;
    context.stroke();
  }

  // A shallow off-centre lobe, as if the reflector is not quite true.
  const lobe = context.createRadialGradient(
    centre * 1.12,
    centre * 0.92,
    0,
    centre * 1.12,
    centre * 0.92,
    centre * 0.5,
  );
  lobe.addColorStop(0, 'rgba(255,238,205,0.16)');
  lobe.addColorStop(1, 'rgba(255,238,205,0)');
  context.fillStyle = lobe;
  context.fillRect(0, 0, size, size);
  context.globalCompositeOperation = 'source-over';

  const texture = new CanvasTexture(canvas);
  texture.colorSpace = SRGBColorSpace;
  texture.minFilter = LinearFilter;
  texture.magFilter = LinearFilter;
  texture.generateMipmaps = false;
  return texture;
}

export class Torch {
  readonly light: SpotLight;
  readonly target: Object3D;

  /** Uniforms consumed by the volumetric pass. */
  readonly uniforms = {
    position: uniform(new Vector3()),
    direction: uniform(new Vector3(0, 0, -1)),
    colour: uniform(new Color(DEFAULTS.colour)),
    intensity: uniform(DEFAULTS.intensity),
    range: uniform(DEFAULTS.range),
    cosOuter: uniform(Math.cos(MathUtils.degToRad(DEFAULTS.angleDeg))),
    cosInner: uniform(Math.cos(MathUtils.degToRad(DEFAULTS.angleDeg * 0.55))),
    enabled: uniform(1),
  };

  private readonly options: Required<TorchOptions>;
  private readonly cookie: CanvasTexture;
  private readonly desiredDirection = new Vector3(0, 0, -1);
  private readonly swayedDirection = new Vector3(0, 0, -1);
  private readonly holdPoint = new Vector3();
  private readonly scratch = new Vector3();
  private swayPhase = Math.random() * 100;

  /** Height above the player's feet the torch is held at. */
  holdHeight = 1.35;

  constructor(scene: Scene, options: TorchOptions = {}) {
    this.options = { ...DEFAULTS, ...options };
    this.cookie = makeCookie();

    this.light = new SpotLight(
      this.options.colour,
      this.options.intensity,
      this.options.range,
      MathUtils.degToRad(this.options.angleDeg),
      0.45,
      1.4,
    );
    this.light.map = this.cookie;
    this.light.castShadow = true;
    this.light.shadow.mapSize.setScalar(this.options.shadowMapSize);
    this.light.shadow.camera.near = 0.4;
    this.light.shadow.camera.far = this.options.range;
    this.light.shadow.bias = -0.0006;
    this.light.shadow.normalBias = 0.02;
    this.light.shadow.focus = 1;

    this.target = new Object3D();

    scene.add(this.light);
    scene.add(this.target);
    this.light.target = this.target;
  }

  setShadowMapSize(size: number): void {
    if (this.light.shadow.mapSize.x === size) return;
    this.light.shadow.mapSize.setScalar(size);
    this.light.shadow.map?.dispose();
    this.light.shadow.map = null;
  }

  setEnabled(enabled: boolean): void {
    this.light.visible = enabled;
    this.uniforms.enabled.value = enabled ? 1 : 0;
  }

  /**
   * Points the torch. Called once per rendered frame with the interpolated
   * player position and the aim point.
   */
  update(playerPosition: Vector3, aimPoint: Vector3, dt: number, elapsed: number): void {
    this.holdPoint.copy(playerPosition);
    this.holdPoint.y += this.holdHeight;

    this.desiredDirection.copy(aimPoint).sub(this.holdPoint);
    // Aim slightly above the floor plane so the cone lands ahead of the player
    // rather than pooling at their feet.
    this.desiredDirection.y += 0.25;
    if (this.desiredDirection.lengthSq() < 1e-6) this.desiredDirection.set(0, 0, -1);
    this.desiredDirection.normalize();

    // Sway: two out of phase sines plus a slower drift, scaled small enough to
    // read as a hand holding a torch rather than a wobble.
    this.swayPhase += dt;
    const t = elapsed + this.swayPhase;
    const swayX = Math.sin(t * 1.7) * 0.016 + Math.sin(t * 0.41) * 0.01;
    const swayY = Math.sin(t * 2.3 + 1.1) * 0.013 + Math.sin(t * 0.53) * 0.008;

    this.swayedDirection.lerp(this.desiredDirection, 1 - Math.exp(-14 * dt));
    this.scratch.set(swayX, swayY, 0);
    this.swayedDirection.add(this.scratch).normalize();

    this.light.position.copy(this.holdPoint);
    this.target.position.copy(this.holdPoint).addScaledVector(this.swayedDirection, 6);

    this.light.updateMatrixWorld();
    this.target.updateMatrixWorld();

    this.uniforms.position.value.copy(this.holdPoint);
    this.uniforms.direction.value.copy(this.swayedDirection);
  }

  /** CPU cone test, used to gate crowd shadow casting from Phase 3 onward. */
  isInCone(point: Vector3, padding = 0.9): boolean {
    if (this.uniforms.enabled.value === 0) return false;

    this.scratch.copy(point).sub(this.holdPoint);
    const distance = this.scratch.length();
    if (distance > this.options.range) return false;
    if (distance < 0.001) return true;

    this.scratch.divideScalar(distance);
    return this.scratch.dot(this.swayedDirection) >= this.uniforms.cosOuter.value * padding;
  }

  dispose(): void {
    this.light.shadow.map?.dispose();
    this.light.dispose();
    this.cookie.dispose();
  }
}

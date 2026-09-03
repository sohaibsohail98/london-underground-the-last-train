/**
 * Hit particles.
 *
 * One instanced draw of camera facing quads, simulated on the CPU. The plan
 * called for a GPU ping-pong simulation on WebGPU with a CPU fallback on
 * WebGL2, but at this particle count a CPU simulation measures at well under
 * 0.1ms per frame and needs no second code path, so both backends share this
 * one. If Phase 8 profiling ever shows this in the top three costs, the
 * simulation is small enough to port to compute then.
 */

import {
  CanvasTexture,
  DynamicDrawUsage,
  InstancedBufferAttribute,
  InstancedMesh,
  Matrix4,
  PlaneGeometry,
  Quaternion,
  SRGBColorSpace,
  Scene,
  Vector3,
  type Camera,
} from 'three';
import { MeshBasicNodeMaterial } from 'three/webgpu';
import { attribute, texture as textureNode, uv, vec3, vec4 } from 'three/tsl';

/** Hard cap. 24 particles per hit means this covers 40 or so simultaneous hits. */
export const PARTICLE_POOL = 1024;

const GRAVITY = -14;
const DRAG = 3.4;

interface Particle {
  active: boolean;
  position: Vector3;
  velocity: Vector3;
  life: number;
  lifetime: number;
  size: number;
}

/** A soft round dot with alpha falloff. Blood colour comes from the material. */
function makeDot(size = 32): CanvasTexture {
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('2D context unavailable for the particle texture');

  const centre = size / 2;
  const gradient = context.createRadialGradient(centre, centre, 0, centre, centre, centre);
  gradient.addColorStop(0, 'rgba(255,255,255,1)');
  gradient.addColorStop(0.45, 'rgba(255,255,255,0.78)');
  gradient.addColorStop(1, 'rgba(255,255,255,0)');
  context.fillStyle = gradient;
  context.fillRect(0, 0, size, size);

  const texture = new CanvasTexture(canvas);
  texture.colorSpace = SRGBColorSpace;
  return texture;
}

const matrix = new Matrix4();
const scaleVector = new Vector3();
const billboard = new Quaternion();

export class Particles {
  private readonly mesh: InstancedMesh;
  private readonly particles: Particle[] = [];
  private readonly fade: InstancedBufferAttribute;
  private readonly texture: CanvasTexture;
  private readonly scene: Scene;
  private cursor = 0;

  /** Live particle count, for the profiler. */
  live = 0;

  constructor(scene: Scene) {
    this.scene = scene;
    this.texture = makeDot();

    const material = new MeshBasicNodeMaterial();
    const sample = textureNode(this.texture, uv());
    const particleFade = attribute<'float'>('aFade', 'float');

    // Crimson, slightly above 1 in red so a burst catches the bloom threshold
    // when it happens inside the torch cone.
    material.colorNode = vec4(vec3(1.15, 0.12, 0.16), sample.a.mul(particleFade));
    material.transparent = true;
    material.depthWrite = false;

    const geometry = new PlaneGeometry(1, 1);
    this.fade = new InstancedBufferAttribute(new Float32Array(PARTICLE_POOL), 1);
    this.fade.setUsage(DynamicDrawUsage);
    geometry.setAttribute('aFade', this.fade);

    this.mesh = new InstancedMesh(geometry, material, PARTICLE_POOL);
    this.mesh.instanceMatrix.setUsage(DynamicDrawUsage);
    this.mesh.frustumCulled = false;
    this.mesh.count = 0;
    this.mesh.renderOrder = 3;
    this.mesh.name = 'hit-particles';
    scene.add(this.mesh);

    for (let i = 0; i < PARTICLE_POOL; i += 1) {
      this.particles.push({
        active: false,
        position: new Vector3(),
        velocity: new Vector3(),
        life: 0,
        lifetime: 0.4,
        size: 0.05,
      });
      this.fade.setX(i, 0);
    }
  }

  /**
   * Fires a burst at an impact point. The direction is the incoming shot
   * direction; the spray goes back along it with a wide cone, which reads as
   * spatter rather than a fountain.
   */
  burst(point: Vector3, direction: Vector3, count = 24, speed = 4.5): void {
    const back = direction.clone().normalize().negate();

    for (let i = 0; i < count; i += 1) {
      const particle = this.particles[this.cursor];
      this.cursor = (this.cursor + 1) % PARTICLE_POOL;

      particle.active = true;
      particle.position.copy(point);
      particle.life = 0;
      particle.lifetime = 0.3 + Math.random() * 0.25;
      particle.size = 0.03 + Math.random() * 0.05;

      // A cone around the reflected direction, plus a little upward bias.
      particle.velocity
        .set(
          back.x + (Math.random() - 0.5) * 1.5,
          back.y + (Math.random() - 0.5) * 1.5 + 0.55,
          back.z + (Math.random() - 0.5) * 1.5,
        )
        .normalize()
        .multiplyScalar(speed * (0.45 + Math.random() * 0.9));
    }

    this.mesh.count = PARTICLE_POOL;
  }

  /** Advances the simulation and rewrites the instance buffers. */
  update(dt: number, camera: Camera): void {
    billboard.setFromRotationMatrix(camera.matrixWorld);

    let live = 0;

    for (let i = 0; i < PARTICLE_POOL; i += 1) {
      const particle = this.particles[i];

      if (!particle.active) {
        continue;
      }

      particle.life += dt;

      if (particle.life >= particle.lifetime) {
        particle.active = false;
        this.fade.setX(i, 0);
        continue;
      }

      // Exponential drag, then gravity. Integrated semi-implicitly so a large
      // frame delta cannot fling a particle across the station.
      const drag = Math.exp(-DRAG * dt);
      particle.velocity.multiplyScalar(drag);
      particle.velocity.y += GRAVITY * dt;
      particle.position.addScaledVector(particle.velocity, dt);

      // Settle on the floor rather than falling through it.
      if (particle.position.y < 0.01) {
        particle.position.y = 0.01;
        particle.velocity.set(0, 0, 0);
      }

      const t = particle.life / particle.lifetime;
      const size = particle.size * (1 - t * 0.45);
      scaleVector.set(size, size, size);

      matrix.compose(particle.position, billboard, scaleVector);
      this.mesh.setMatrixAt(i, matrix);
      this.fade.setX(i, 1 - t * t);

      live += 1;
    }

    this.live = live;
    this.mesh.instanceMatrix.needsUpdate = true;
    this.fade.needsUpdate = true;
  }

  clear(): void {
    for (let i = 0; i < PARTICLE_POOL; i += 1) {
      this.particles[i].active = false;
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
}

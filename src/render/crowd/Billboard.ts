/**
 * Far LOD impostors.
 *
 * Beyond torch range a zombie is a handful of pixels, so it is drawn as a
 * camera facing quad sampled from an atlas rendered once at load: eight yaw
 * angles by four poses. Rendering the atlas from the real VAT mesh rather than
 * drawing a silhouette by hand means the impostor matches the near LOD in
 * proportion and shading, which is what stops the transition being obvious.
 */

import {
  HalfFloatType,
  InstancedBufferAttribute,
  InstancedMesh,
  NearestFilter,
  OrthographicCamera,
  PlaneGeometry,
  RenderTarget,
  Scene,
  Vector3,
} from 'three';
import { MeshBasicNodeMaterial, type WebGPURenderer } from 'three/webgpu';
import { attribute, float, texture as textureNode, uv, vec2, vec4 } from 'three/tsl';
import type { Node } from 'three/webgpu';
import { createVatMaterials, CROWD_ATTRIBUTES } from './VatMaterial';
import type { VatBake } from './VatBaker';

export const BILLBOARD_YAWS = 8;
export const BILLBOARD_POSES = 4;

const TILE_SIZE = 128;

export interface BillboardAtlas {
  target: RenderTarget;
  material: MeshBasicNodeMaterial;
  width: number;
  height: number;
  dispose(): void;
}

/**
 * Renders the atlas. Called once during loading; costs 32 small draws.
 */
export function bakeBillboards(renderer: WebGPURenderer, bake: VatBake): BillboardAtlas {
  const width = TILE_SIZE * BILLBOARD_YAWS;
  const height = TILE_SIZE * BILLBOARD_POSES;

  const target = new RenderTarget(width, height, {
    type: HalfFloatType,
    depthBuffer: true,
  });
  target.texture.minFilter = NearestFilter;
  target.texture.magFilter = NearestFilter;
  target.texture.generateMipmaps = false;

  const scene = new Scene();
  scene.background = null;

  // Orthographic, framed on a 1.9m tall figure with a little headroom.
  const camera = new OrthographicCamera(-1.1, 1.1, 2.1, -0.1, 0.1, 10);

  const materials = createVatMaterials(bake);
  const single = new InstancedMesh(bake.geometry, materials.mid, 1);
  applySingleInstanceAttributes(single, bake);
  scene.add(single);

  const previousTarget = renderer.getRenderTarget();

  for (let pose = 0; pose < BILLBOARD_POSES; pose += 1) {
    // Four evenly spaced frames of the walk cycle, which covers the readable
    // range of leg positions at this size.
    materials.clock.value = (pose / BILLBOARD_POSES) * bake.clips[0].duration;

    for (let yaw = 0; yaw < BILLBOARD_YAWS; yaw += 1) {
      const angle = (yaw / BILLBOARD_YAWS) * Math.PI * 2;
      camera.position.set(Math.sin(angle) * 4, 1, Math.cos(angle) * 4);
      camera.lookAt(new Vector3(0, 0.95, 0));

      renderer.setRenderTarget(target);
      renderer.setViewport(yaw * TILE_SIZE, pose * TILE_SIZE, TILE_SIZE, TILE_SIZE);
      renderer.setScissor(yaw * TILE_SIZE, pose * TILE_SIZE, TILE_SIZE, TILE_SIZE);
      renderer.setScissorTest(true);
      renderer.render(scene, camera);
    }
  }

  renderer.setScissorTest(false);
  renderer.setRenderTarget(previousTarget);
  renderer.setViewport(0, 0, renderer.domElement.width, renderer.domElement.height);

  scene.remove(single);
  single.dispose();
  materials.dispose();

  const material = billboardMaterial(target);

  return {
    target,
    material,
    width,
    height,
    dispose(): void {
      target.dispose();
      material.dispose();
    },
  };
}

/** Writes the one instance's animation attributes for the atlas bake. */
function applySingleInstanceAttributes(mesh: InstancedMesh, bake: VatBake): void {
  const walk = bake.clips[0];
  const geometry = mesh.geometry;

  setInstancedAttribute(geometry, CROWD_ATTRIBUTES.anim, 4, [
    walk.startRow,
    walk.frameCount,
    1,
    0,
  ]);
  setInstancedAttribute(geometry, CROWD_ATTRIBUTES.animPrevious, 4, [
    walk.startRow,
    walk.frameCount,
    0,
    0,
  ]);
  setInstancedAttribute(geometry, CROWD_ATTRIBUTES.flags, 1, [0]);

  mesh.count = 1;
}

function setInstancedAttribute(
  geometry: InstancedMesh['geometry'],
  name: string,
  itemSize: number,
  values: number[],
): void {
  const array = new Float32Array(itemSize);
  for (let i = 0; i < itemSize; i += 1) array[i] = values[i] ?? 0;
  geometry.setAttribute(name, new InstancedBufferAttribute(array, itemSize));
}

/**
 * The billboard material. Per instance yaw and pose select an atlas tile; the
 * quad itself is oriented by the crowd's instance matrix, which always faces
 * the camera.
 */
function billboardMaterial(target: RenderTarget): MeshBasicNodeMaterial {
  const material = new MeshBasicNodeMaterial();
  material.name = 'crowd-billboard';
  material.transparent = true;
  material.depthWrite = true;

  const tile = attribute('aTile', 'vec2') as unknown as Node<'vec2'>;

  // Map the quad's own UVs into the selected atlas tile.
  const tileUv = uv()
    .mul(vec2(1 / BILLBOARD_YAWS, 1 / BILLBOARD_POSES))
    .add(vec2(tile.x.div(BILLBOARD_YAWS), tile.y.div(BILLBOARD_POSES)));

  const sample = textureNode(target.texture, tileUv);

  material.colorNode = vec4(sample.rgb, sample.a);
  // Anything the atlas left empty is discarded rather than blended, so
  // impostors do not sort against each other.
  material.maskNode = sample.a.greaterThan(float(0.35));

  return material;
}

export function billboardGeometry(height: number): PlaneGeometry {
  const geometry = new PlaneGeometry(height * 0.62, height);
  geometry.translate(0, height / 2, 0);
  return geometry;
}

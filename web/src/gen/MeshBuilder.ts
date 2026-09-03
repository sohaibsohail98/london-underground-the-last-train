/**
 * Geometry accumulator.
 *
 * Emitters push quads and the builder produces one merged BufferGeometry per
 * material set. Merging at authoring time rather than relying on the renderer
 * is what keeps the draw call count in single figures for a whole station.
 *
 * UVs are written in world metres, so a material's tilesPerMetre setting means
 * exactly what it says regardless of how large the merged surface is.
 */

import { BufferAttribute, BufferGeometry, Vector3 } from 'three';

const edgeA = new Vector3();
const edgeB = new Vector3();
const faceNormal = new Vector3();

export class MeshBuilder {
  private readonly positions: number[] = [];
  private readonly normals: number[] = [];
  private readonly uvs: number[] = [];
  private readonly colours: number[] = [];
  private readonly indices: number[] = [];
  private vertexCount = 0;

  get triangleCount(): number {
    return this.indices.length / 3;
  }

  get isEmpty(): boolean {
    return this.indices.length === 0;
  }

  /**
   * Adds a quad. Vertices must wind anticlockwise when viewed from the front,
   * which for a floor means starting at the minus x, minus z corner. UVs are
   * per vertex in metres, and the wetness value lands in the vertex colour red
   * channel where the floor material reads it.
   */
  addQuad(
    a: Vector3,
    b: Vector3,
    c: Vector3,
    d: Vector3,
    uvA: [number, number],
    uvB: [number, number],
    uvC: [number, number],
    uvD: [number, number],
    wetness = 0,
    normal?: Vector3,
  ): void {
    let n = normal;
    if (!n) {
      edgeA.subVectors(b, a);
      edgeB.subVectors(c, a);
      faceNormal.crossVectors(edgeA, edgeB).normalize();
      n = faceNormal;
    }

    const base = this.vertexCount;

    for (const [vertex, uv] of [
      [a, uvA],
      [b, uvB],
      [c, uvC],
      [d, uvD],
    ] as const) {
      this.positions.push(vertex.x, vertex.y, vertex.z);
      this.normals.push(n.x, n.y, n.z);
      this.uvs.push(uv[0], uv[1]);
      this.colours.push(wetness, 0, 0);
      this.vertexCount += 1;
    }

    this.indices.push(base, base + 1, base + 2, base, base + 2, base + 3);
  }

  /** Axis aligned box, used for lips, skirtings, slabs and steps. */
  addBox(centre: Vector3, size: Vector3, wetness = 0, uvScale = 1, skipBottom = true): void {
    const hx = size.x / 2;
    const hy = size.y / 2;
    const hz = size.z / 2;

    const x0 = centre.x - hx;
    const x1 = centre.x + hx;
    const y0 = centre.y - hy;
    const y1 = centre.y + hy;
    const z0 = centre.z - hz;
    const z1 = centre.z + hz;

    const v = (x: number, y: number, z: number): Vector3 => new Vector3(x, y, z);
    const s = uvScale;

    // Top
    this.addQuad(
      v(x0, y1, z1),
      v(x1, y1, z1),
      v(x1, y1, z0),
      v(x0, y1, z0),
      [x0 * s, z1 * s],
      [x1 * s, z1 * s],
      [x1 * s, z0 * s],
      [x0 * s, z0 * s],
      wetness,
      new Vector3(0, 1, 0),
    );

    if (!skipBottom) {
      this.addQuad(
        v(x0, y0, z0),
        v(x1, y0, z0),
        v(x1, y0, z1),
        v(x0, y0, z1),
        [x0 * s, z0 * s],
        [x1 * s, z0 * s],
        [x1 * s, z1 * s],
        [x0 * s, z1 * s],
        0,
        new Vector3(0, -1, 0),
      );
    }

    // Four sides
    this.addQuad(
      v(x0, y0, z1),
      v(x1, y0, z1),
      v(x1, y1, z1),
      v(x0, y1, z1),
      [x0 * s, y0 * s],
      [x1 * s, y0 * s],
      [x1 * s, y1 * s],
      [x0 * s, y1 * s],
      0,
      new Vector3(0, 0, 1),
    );
    this.addQuad(
      v(x1, y0, z0),
      v(x0, y0, z0),
      v(x0, y1, z0),
      v(x1, y1, z0),
      [x1 * s, y0 * s],
      [x0 * s, y0 * s],
      [x0 * s, y1 * s],
      [x1 * s, y1 * s],
      0,
      new Vector3(0, 0, -1),
    );
    this.addQuad(
      v(x1, y0, z1),
      v(x1, y0, z0),
      v(x1, y1, z0),
      v(x1, y1, z1),
      [z1 * s, y0 * s],
      [z0 * s, y0 * s],
      [z0 * s, y1 * s],
      [z1 * s, y1 * s],
      0,
      new Vector3(1, 0, 0),
    );
    this.addQuad(
      v(x0, y0, z0),
      v(x0, y0, z1),
      v(x0, y1, z1),
      v(x0, y1, z0),
      [z0 * s, y0 * s],
      [z1 * s, y0 * s],
      [z1 * s, y1 * s],
      [z0 * s, y1 * s],
      0,
      new Vector3(-1, 0, 0),
    );
  }

  build(): BufferGeometry {
    const geometry = new BufferGeometry();
    geometry.setAttribute('position', new BufferAttribute(new Float32Array(this.positions), 3));
    geometry.setAttribute('normal', new BufferAttribute(new Float32Array(this.normals), 3));
    geometry.setAttribute('uv', new BufferAttribute(new Float32Array(this.uvs), 2));
    geometry.setAttribute('color', new BufferAttribute(new Float32Array(this.colours), 3));
    geometry.setIndex(this.indices);
    geometry.computeBoundingSphere();
    geometry.computeBoundingBox();
    return geometry;
  }
}

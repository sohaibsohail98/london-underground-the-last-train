/**
 * Hardcoded test room.
 *
 * Exists only to judge the renderer at Gate B and is deleted once the
 * procedural generator lands in Phase 3. It deliberately contains one of
 * everything the generator will emit: tiled walls, a wet floor, a platform lip
 * with a warning line, a trackbed with rails and sleepers, a tunnel mouth
 * receding into fog, emissive ceiling and platform strips, glass, steel and a
 * handful of instanced props. If the lighting does not sell this room, it will
 * not sell a generated one either.
 */

import {
  BackSide,
  BoxGeometry,
  CapsuleGeometry,
  Color,
  CylinderGeometry,
  Group,
  InstancedMesh,
  Matrix4,
  Mesh,
  PlaneGeometry,
  Quaternion,
  TubeGeometry,
  CatmullRomCurve3,
  Vector3,
} from 'three';
import { MeshBasicNodeMaterial } from 'three/webgpu';
import { vec3 } from 'three/tsl';
import type { Lighting } from '../Lighting';
import type { MaterialLibrary, MaterialSet } from '../Materials';
import type { Occlusion } from '../Occlusion';

export const TEST_ROOM_SETS: MaterialSet[] = [
  'concrete',
  'tile',
  'wet_tile',
  'steel',
  'painted_brick',
  'rubber',
  'glass',
];

/** Room dimensions in metres. Roughly one platform bay of a real station. */
const ROOM = { width: 34, depth: 20, height: 4.2 };
const PLATFORM_Z = 4.5;
const TRACK_DROP = 1.1;

interface StripSpec {
  position: Vector3;
  length: number;
  axis: 'x' | 'z';
  colour: number;
  intensity: number;
  emissive: number;
}

export class TestRoom {
  readonly group = new Group();
  readonly playerProxy: Mesh;

  /** Where the player starts, and the extent they may walk in. */
  readonly spawn = new Vector3(0, 0, ROOM.depth * 0.5 - 5);
  readonly bounds = {
    minX: -ROOM.width / 2 + 1,
    maxX: ROOM.width / 2 - 1,
    minZ: -PLATFORM_Z + 1.2,
    maxZ: ROOM.depth / 2 - 1,
  };

  private readonly materials: MaterialLibrary;
  private readonly lighting: Lighting;
  private readonly occlusion: Occlusion;

  constructor(materials: MaterialLibrary, lighting: Lighting, occlusion: Occlusion) {
    this.materials = materials;
    this.lighting = lighting;
    this.occlusion = occlusion;

    this.group.name = 'test-room';

    this.buildFloor();
    this.buildWalls();
    this.buildCeiling();
    this.buildPlatformAndTrack();
    this.buildTunnel();
    this.buildProps();
    this.buildStrips();

    this.playerProxy = this.buildPlayerProxy();
    this.group.add(this.playerProxy);
  }

  private buildFloor(): void {
    // Dry concourse behind the platform, wet tile at the platform edge where
    // water tracks in off the trains. The wet band is what SSR reflects.
    const dry = new Mesh(
      new PlaneGeometry(ROOM.width, ROOM.depth - 9),
      this.materials.surface({ set: 'concrete', tilesPerMetre: 0.34, wetness: 0.05 }),
    );
    dry.rotation.x = -Math.PI / 2;
    dry.position.set(0, 0, 4.5);
    dry.receiveShadow = true;
    this.applyMetreUvs(dry.geometry, ROOM.width, ROOM.depth - 9);
    this.group.add(dry);

    const wet = new Mesh(
      new PlaneGeometry(ROOM.width, 9),
      this.materials.surface({ set: 'wet_tile', tilesPerMetre: 0.66, wetness: 0.92 }),
    );
    wet.rotation.x = -Math.PI / 2;
    wet.position.set(0, 0.001, -2.5);
    wet.receiveShadow = true;
    this.applyMetreUvs(wet.geometry, ROOM.width, 9);
    this.group.add(wet);
  }

  private buildWalls(): void {
    const brick = this.materials.surface({
      set: 'painted_brick',
      tilesPerMetre: 0.42,
      occludable: true,
    });
    const tile = this.materials.surface({
      set: 'tile',
      tilesPerMetre: 0.55,
      occludable: true,
    });

    // Back wall, tiled to head height then painted brick above, which is what
    // most of the older stations on the line actually look like.
    const backLower = new Mesh(new BoxGeometry(ROOM.width, 2.4, 0.4), tile);
    backLower.position.set(0, 1.2, ROOM.depth / 2);
    backLower.castShadow = true;
    backLower.receiveShadow = true;
    this.occlusion.register(backLower);
    this.group.add(backLower);

    const backUpper = new Mesh(new BoxGeometry(ROOM.width, ROOM.height - 2.4, 0.4), brick);
    backUpper.position.set(0, 2.4 + (ROOM.height - 2.4) / 2, ROOM.depth / 2);
    backUpper.castShadow = true;
    backUpper.receiveShadow = true;
    this.occlusion.register(backUpper);
    this.group.add(backUpper);

    for (const sign of [-1, 1]) {
      const side = new Mesh(new BoxGeometry(0.4, ROOM.height, ROOM.depth), tile);
      side.position.set((sign * ROOM.width) / 2, ROOM.height / 2, 0);
      side.castShadow = true;
      side.receiveShadow = true;
      this.occlusion.register(side);
      this.group.add(side);
    }

    // Two pillars, which is what really sells the volumetric torch: hard
    // shadow edges moving across the floor as the player turns.
    const steel = this.materials.surface({ set: 'steel', tilesPerMetre: 1 });
    for (const x of [-8, 8]) {
      const pillar = new Mesh(new CylinderGeometry(0.34, 0.38, ROOM.height, 16), steel);
      pillar.position.set(x, ROOM.height / 2, 2.6);
      pillar.castShadow = true;
      pillar.receiveShadow = true;
      this.group.add(pillar);
    }

    // A glazed screen at one end, for a transparent surface in the chain.
    const glass = new Mesh(
      new BoxGeometry(6, 2.6, 0.08),
      this.materials.surface({ set: 'glass', tilesPerMetre: 0.3 }),
    );
    glass.position.set(11, 1.5, ROOM.depth / 2 - 0.4);
    this.group.add(glass);
  }

  private buildCeiling(): void {
    const ceiling = new Mesh(
      new PlaneGeometry(ROOM.width, ROOM.depth),
      this.materials.surface({ set: 'concrete', tilesPerMetre: 0.3, occludable: true }),
    );
    ceiling.rotation.x = Math.PI / 2;
    ceiling.position.set(0, ROOM.height, 0);
    ceiling.receiveShadow = true;
    this.applyMetreUvs(ceiling.geometry, ROOM.width, ROOM.depth);
    this.occlusion.register(ceiling);
    this.group.add(ceiling);
  }

  private buildPlatformAndTrack(): void {
    const tile = this.materials.surface({ set: 'tile', tilesPerMetre: 0.6, wetness: 0.6 });
    const steel = this.materials.surface({ set: 'steel', tilesPerMetre: 1.6 });
    const rubber = this.materials.surface({ set: 'rubber', tilesPerMetre: 0.8 });

    // Platform lip, standing 0.2m proud of the floor.
    const lip = new Mesh(new BoxGeometry(ROOM.width, 0.2, 0.5), tile);
    lip.position.set(0, 0.1, -PLATFORM_Z);
    lip.castShadow = true;
    lip.receiveShadow = true;
    this.group.add(lip);

    // Warning line, set back from the edge. Emissive so it catches the torch
    // and glows faintly in the dark, which is exactly what the real ones do
    // under sodium light.
    const warning = new Mesh(new PlaneGeometry(ROOM.width, 0.42), this.emissiveMaterial(0xe0a030, 1.6));
    warning.rotation.x = -Math.PI / 2;
    warning.position.set(0, 0.202, -PLATFORM_Z + 0.62);
    this.group.add(warning);

    // Trackbed, dropped below the platform.
    const bed = new Mesh(new BoxGeometry(ROOM.width, 0.4, 6), rubber);
    bed.position.set(0, -TRACK_DROP - 0.2, -PLATFORM_Z - 3.2);
    bed.receiveShadow = true;
    this.group.add(bed);

    const backdrop = new Mesh(
      new BoxGeometry(ROOM.width, ROOM.height + TRACK_DROP, 0.4),
      this.materials.surface({ set: 'painted_brick', tilesPerMetre: 0.42 }),
    );
    backdrop.position.set(0, (ROOM.height - TRACK_DROP) / 2, -PLATFORM_Z - 6.2);
    backdrop.castShadow = true;
    backdrop.receiveShadow = true;
    this.group.add(backdrop);

    // Rails: two instanced steel boxes, plus a third rail on the far side.
    const railGeometry = new BoxGeometry(1.5, 0.14, 0.12);
    const rails = new InstancedMesh(railGeometry, steel, 3 * 22);
    rails.castShadow = false;
    rails.receiveShadow = true;

    const matrix = new Matrix4();
    const quaternion = new Quaternion();
    const scale = new Vector3(1, 1, 1);
    let index = 0;

    for (const offset of [-1.7, -3.15, -4.6]) {
      for (let i = 0; i < 22; i += 1) {
        const x = -ROOM.width / 2 + 0.75 + i * 1.5;
        matrix.compose(
          new Vector3(x, -TRACK_DROP + 0.07, -PLATFORM_Z + offset - 0.5),
          quaternion,
          scale,
        );
        rails.setMatrixAt(index, matrix);
        index += 1;
      }
    }
    rails.count = index;
    rails.instanceMatrix.needsUpdate = true;
    this.group.add(rails);

    // Sleepers.
    const sleeperGeometry = new BoxGeometry(0.24, 0.16, 3.4);
    const sleepers = new InstancedMesh(sleeperGeometry, rubber, 56);
    let sleeperIndex = 0;
    for (let i = 0; i < 56; i += 1) {
      const x = -ROOM.width / 2 + 0.3 + i * 0.6;
      if (x > ROOM.width / 2) break;
      matrix.compose(new Vector3(x, -TRACK_DROP - 0.02, -PLATFORM_Z - 3.15), quaternion, scale);
      sleepers.setMatrixAt(sleeperIndex, matrix);
      sleeperIndex += 1;
    }
    sleepers.count = sleeperIndex;
    sleepers.instanceMatrix.needsUpdate = true;
    sleepers.receiveShadow = true;
    this.group.add(sleepers);
  }

  private buildTunnel(): void {
    // A splined tube receding from the trackbed. The far end is unlit, so it
    // reads as depth rather than as a hole in the geometry.
    const curve = new CatmullRomCurve3([
      new Vector3(-ROOM.width / 2, -TRACK_DROP + 1.4, -PLATFORM_Z - 3.2),
      new Vector3(-ROOM.width / 2 - 12, -TRACK_DROP + 1.5, -PLATFORM_Z - 4.4),
      new Vector3(-ROOM.width / 2 - 26, -TRACK_DROP + 1.7, -PLATFORM_Z - 8.6),
      new Vector3(-ROOM.width / 2 - 40, -TRACK_DROP + 1.9, -PLATFORM_Z - 15.2),
    ]);

    // Cloned rather than shared, because we need the inside faces and the
    // library caches by option key, not by side.
    const tunnelMaterial = this.materials.surface({ set: 'concrete', tilesPerMetre: 0.5 }).clone();
    tunnelMaterial.side = BackSide;

    const tube = new Mesh(new TubeGeometry(curve, 32, 2.6, 16, false), tunnelMaterial);
    tube.receiveShadow = true;
    this.group.add(tube);

    // Ring supports every few metres along the tube.
    const steel = this.materials.surface({ set: 'steel', tilesPerMetre: 2 });
    const ringGeometry = new CylinderGeometry(2.58, 2.58, 0.16, 16, 1, true);
    const rings = new InstancedMesh(ringGeometry, steel, 12);
    const matrix = new Matrix4();
    const quaternion = new Quaternion();
    const up = new Vector3(0, 1, 0);

    for (let i = 0; i < 12; i += 1) {
      const t = 0.04 + (i / 12) * 0.92;
      const point = curve.getPointAt(t);
      const tangent = curve.getTangentAt(t);
      quaternion.setFromUnitVectors(up, tangent.normalize());
      matrix.compose(point, quaternion, new Vector3(1, 1, 1));
      rings.setMatrixAt(i, matrix);
    }
    rings.instanceMatrix.needsUpdate = true;
    this.group.add(rings);
  }

  private buildProps(): void {
    const steel = this.materials.surface({ set: 'steel', tilesPerMetre: 1.4 });
    const rubber = this.materials.surface({ set: 'rubber', tilesPerMetre: 1 });
    const matrix = new Matrix4();
    const quaternion = new Quaternion();
    const scale = new Vector3(1, 1, 1);

    // Benches against the back wall.
    const benchGeometry = new BoxGeometry(2.2, 0.12, 0.5);
    const benches = new InstancedMesh(benchGeometry, steel, 4);
    for (let i = 0; i < 4; i += 1) {
      matrix.compose(new Vector3(-12 + i * 8, 0.46, ROOM.depth / 2 - 1.1), quaternion, scale);
      benches.setMatrixAt(i, matrix);
    }
    benches.instanceMatrix.needsUpdate = true;
    benches.castShadow = true;
    this.group.add(benches);

    // Bench legs, and litter bins, sharing one instanced draw each.
    const legGeometry = new BoxGeometry(0.1, 0.46, 0.42);
    const legs = new InstancedMesh(legGeometry, steel, 8);
    let legIndex = 0;
    for (let i = 0; i < 4; i += 1) {
      for (const dx of [-0.9, 0.9]) {
        matrix.compose(
          new Vector3(-12 + i * 8 + dx, 0.23, ROOM.depth / 2 - 1.1),
          quaternion,
          scale,
        );
        legs.setMatrixAt(legIndex, matrix);
        legIndex += 1;
      }
    }
    legs.count = legIndex;
    legs.instanceMatrix.needsUpdate = true;
    legs.castShadow = true;
    this.group.add(legs);

    const binGeometry = new CylinderGeometry(0.28, 0.24, 0.9, 12);
    const bins = new InstancedMesh(binGeometry, rubber, 3);
    for (let i = 0; i < 3; i += 1) {
      matrix.compose(new Vector3(-6 + i * 7, 0.45, ROOM.depth / 2 - 2.4), quaternion, scale);
      bins.setMatrixAt(i, matrix);
    }
    bins.instanceMatrix.needsUpdate = true;
    bins.castShadow = true;
    this.group.add(bins);
  }

  private buildStrips(): void {
    const specs: StripSpec[] = [
      // Ceiling strips down the length of the room.
      {
        position: new Vector3(-9, ROOM.height - 0.14, 1),
        length: 9,
        axis: 'x',
        colour: 0xffe6bc,
        intensity: 14,
        emissive: 3.4,
      },
      {
        position: new Vector3(4, ROOM.height - 0.14, 1),
        length: 9,
        axis: 'x',
        colour: 0xffe6bc,
        intensity: 14,
        emissive: 3.4,
      },
      // Platform edge strip, continuous and cooler in tone.
      {
        position: new Vector3(0, 0.26, -PLATFORM_Z - 0.05),
        length: ROOM.width - 2,
        axis: 'x',
        colour: 0xbcd0ff,
        intensity: 6,
        emissive: 1.9,
      },
      // A dying strip near the tunnel, deliberately dimmer and violet, so the
      // palette accent appears somewhere in the room.
      {
        position: new Vector3(-13.5, ROOM.height - 0.14, -2),
        length: 4,
        axis: 'x',
        colour: 0x9a78d8,
        intensity: 5,
        emissive: 1.5,
      },
    ];

    for (const spec of specs) {
      const geometry =
        spec.axis === 'x'
          ? new BoxGeometry(spec.length, 0.09, 0.3)
          : new BoxGeometry(0.3, 0.09, spec.length);

      const strip = new Mesh(geometry, this.emissiveMaterial(spec.colour, spec.emissive));
      strip.position.copy(spec.position);
      this.group.add(strip);

      // Register point light candidates along the strip, spaced so the falloff
      // overlaps rather than beading.
      const count = Math.max(1, Math.round(spec.length / 3));
      for (let i = 0; i < count; i += 1) {
        const t = count === 1 ? 0.5 : i / (count - 1);
        const offset = (t - 0.5) * spec.length;
        const position = spec.position.clone();
        if (spec.axis === 'x') position.x += offset;
        else position.z += offset;
        position.y -= 0.2;

        this.lighting.register({
          position,
          colour: new Color(spec.colour),
          intensity: spec.intensity / count,
          range: 11,
          blackoutSensitive: true,
        });
      }
    }
  }

  private buildPlayerProxy(): Mesh {
    // A stand-in for the rigged player, purely so the camera has something to
    // frame and the torch has an origin. Replaced in Phase 4.
    const proxy = new Mesh(
      new CapsuleGeometry(0.34, 1.05, 8, 16),
      this.materials.surface({ set: 'rubber', tilesPerMetre: 1.2 }),
    );
    proxy.position.copy(this.spawn);
    proxy.position.y = 0.87;
    proxy.castShadow = true;
    return proxy;
  }

  /** An unlit material bright enough to bloom, dimmed by the blackout switch. */
  private emissiveMaterial(colour: number, intensity: number): MeshBasicNodeMaterial {
    const material = new MeshBasicNodeMaterial();
    const base = new Color(colour);

    // Above the bloom threshold of 1.05, so strips are the only thing in the
    // room that blooms, and driven by the shared emissive scale so a blackout
    // takes them to black without touching any geometry.
    material.colorNode = vec3(base.r, base.g, base.b)
      .mul(intensity)
      .mul(this.lighting.emissiveScale);

    return material;
  }

  /**
   * Rewrites plane UVs in metres so that tilesPerMetre in the material factory
   * means what it says. The generator does this natively in Phase 3.
   */
  private applyMetreUvs(geometry: PlaneGeometry, width: number, depth: number): void {
    const uv = geometry.attributes.uv;
    for (let i = 0; i < uv.count; i += 1) {
      uv.setXY(i, uv.getX(i) * width, uv.getY(i) * depth);
    }
    uv.needsUpdate = true;
  }

  dispose(): void {
    this.group.traverse((object) => {
      if (object instanceof Mesh || object instanceof InstancedMesh) {
        object.geometry.dispose();
      }
    });
  }
}

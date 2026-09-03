/**
 * Escalators.
 *
 * Stepped geometry rising over the length of the run, with side panels and a
 * handrail whose material scrolls. The scroll is the detail that sells it:
 * a static escalator reads as stairs, a moving handrail reads as a machine.
 */

import { Color, Vector3 } from 'three';
import { MeshStandardNodeMaterial } from 'three/webgpu';
import { texture as textureNode, uniform, uv, vec2 } from 'three/tsl';
import { Mesh, BoxGeometry } from 'three';
import { LEGEND, TILE, WALL_HEIGHT } from '../../data/legend';
import { builderFor, type EmitterContext } from '../Context';

const STEP_RISE = 0.22;

export interface HandrailControl {
  /** Advanced once per rendered frame by the generated station. */
  scroll: { value: number };
  speed: number;
}

export function emitEscalators(context: EmitterContext): HandrailControl | null {
  const { grid, materials } = context;
  const runs = grid.runs(LEGEND.ESCALATOR);
  if (runs.length === 0) return null;

  const steps = builderFor(context, { set: 'steel', tilesPerMetre: 1.2 });
  const panels = builderFor(context, { set: 'painted_brick', tilesPerMetre: 0.5 }, true);

  const scroll = uniform(0);
  const control: HandrailControl = { scroll, speed: 0.35 };

  // The handrail needs a scrolling UV, which the shared material factory does
  // not offer, so it is built directly against the loaded texture set.
  const rubber = materials.textures('rubber');
  const handrailMaterial = new MeshStandardNodeMaterial();
  handrailMaterial.name = 'escalator-handrail';
  const scrolledUv = uv().mul(vec2(4, 1)).add(vec2(scroll, 0));
  handrailMaterial.colorNode = textureNode(rubber.albedo, scrolledUv).rgb.mul(0.7);
  handrailMaterial.normalNode = textureNode(rubber.normal, scrolledUv)
    .rgb.mul(2)
    .sub(1)
    .normalize();
  handrailMaterial.roughnessNode = textureNode(rubber.roughness, scrolledUv).r;

  for (const run of runs) {
    if (run.cells.length < 2) continue;

    const alongX = run.axis === 'x';
    const start = grid.worldPosition(run.cells[0].x, run.cells[0].y);
    const length = run.cells.length * TILE;
    const stepCount = Math.max(2, Math.round(WALL_HEIGHT / STEP_RISE));
    const stepDepth = length / stepCount;

    const direction = alongX ? new Vector3(1, 0, 0) : new Vector3(0, 0, 1);
    const lateral = alongX ? new Vector3(0, 0, 1) : new Vector3(1, 0, 0);

    const base = start.clone().addScaledVector(direction, -TILE / 2);

    for (let i = 0; i < stepCount; i += 1) {
      const centre = base
        .clone()
        .addScaledVector(direction, stepDepth * (i + 0.5))
        .setY(STEP_RISE * (i + 0.5));

      const size = alongX
        ? new Vector3(stepDepth, STEP_RISE, TILE * 1.4)
        : new Vector3(TILE * 1.4, STEP_RISE, stepDepth);

      steps.addBox(centre, size, 0.1, 1.2);
    }

    // Side panels, following the incline as one sloped slab each side.
    for (const side of [-1, 1]) {
      const panelCentre = base
        .clone()
        .addScaledVector(direction, length / 2)
        .addScaledVector(lateral, side * TILE * 0.78)
        .setY((STEP_RISE * stepCount) / 2);

      const panelSize = alongX
        ? new Vector3(length, STEP_RISE * stepCount, 0.14)
        : new Vector3(0.14, STEP_RISE * stepCount, length);

      panels.addBox(panelCentre, panelSize, 0, 0.5);

      // Handrail: a separate mesh so it can carry the scrolling material.
      const railGeometry = alongX
        ? new BoxGeometry(length, 0.1, 0.16)
        : new BoxGeometry(0.16, 0.1, length);

      const rail = new Mesh(railGeometry, handrailMaterial);
      rail.position
        .copy(base)
        .addScaledVector(direction, length / 2)
        .addScaledVector(lateral, side * TILE * 0.78);
      rail.position.y = STEP_RISE * stepCount + 0.62;

      // Tilt to match the incline.
      const pitch = Math.atan2(STEP_RISE * stepCount, length);
      if (alongX) rail.rotation.z = -pitch;
      else rail.rotation.x = pitch;

      rail.castShadow = true;
      context.group.add(rail);
    }

    // Lighting inside the escalator well, which is always brighter than the
    // concourse in a real station.
    const lightCount = Math.max(2, Math.round(length / 5));
    for (let i = 0; i < lightCount; i += 1) {
      const t = (i + 0.5) / lightCount;
      context.lightCandidates.push({
        position: base
          .clone()
          .addScaledVector(direction, length * t)
          .setY(STEP_RISE * stepCount + 1.9),
        colour: new Color(0xffe6bc),
        intensity: 6,
        range: 9,
        blackoutSensitive: true,
      });
    }
  }

  return control;
}

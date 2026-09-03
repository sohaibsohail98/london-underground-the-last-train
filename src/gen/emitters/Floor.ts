/**
 * Floor and ceiling emission.
 *
 * Wetness is painted per tile into the vertex colour red channel, from three
 * sources: the station's baseline, proximity to water, and seeded puddle
 * noise. The floor material reads that channel as a roughness and SSR mask, so
 * reflections appear exactly where the wet patches are and nowhere else.
 */

import { Vector3 } from 'three';
import { LEGEND, TILE, WALL_HEIGHT } from '../../data/legend';
import { builderFor, type EmitterContext } from '../Context';

/** Manhattan search radius, in tiles, over which water wets the floor. */
const WATER_INFLUENCE = 4;

function distanceToWater(context: EmitterContext, x: number, y: number): number {
  const { grid } = context;
  let best = Number.POSITIVE_INFINITY;

  for (let dy = -WATER_INFLUENCE; dy <= WATER_INFLUENCE; dy += 1) {
    for (let dx = -WATER_INFLUENCE; dx <= WATER_INFLUENCE; dx += 1) {
      if (grid.charAt(x + dx, y + dy) !== LEGEND.WATER) continue;
      best = Math.min(best, Math.abs(dx) + Math.abs(dy));
    }
  }

  return best;
}

export function emitFloor(context: EmitterContext): void {
  const { grid, station, streams } = context;
  const puddles = streams.get('floor-puddles');

  const dry = builderFor(context, {
    set: 'concrete',
    tilesPerMetre: 0.34,
    wetness: station.wetness,
  });
  const platformFloor = builderFor(context, {
    set: 'tile',
    tilesPerMetre: 0.6,
    wetness: Math.max(station.wetness, 0.35),
  });

  const ceiling = builderFor(
    context,
    { set: 'concrete', tilesPerMetre: 0.3, tint: 0xbfbfc6 },
    true,
  );

  const half = TILE / 2;
  const centre = new Vector3();

  for (const cell of grid.cells) {
    if (cell.char === LEGEND.WALL) continue;

    grid.worldPosition(cell.x, cell.y, centre);

    const x0 = centre.x - half;
    const x1 = centre.x + half;
    const z0 = centre.z - half;
    const z1 = centre.z + half;

    // Water cells get a floor slab underneath so the water plane has a bed to
    // sit on rather than showing through to nothing.
    const submerged = cell.char === LEGEND.WATER;
    const y = submerged ? -0.32 : 0;

    const waterDistance = distanceToWater(context, cell.x, cell.y);
    const proximityWet =
      waterDistance === Number.POSITIVE_INFINITY
        ? 0
        : Math.max(0, 1 - waterDistance / WATER_INFLUENCE) * 0.85;

    // Seeded puddle noise, sampled per tile so it is stable across rebuilds.
    const puddle = puddles.next() < 0.12 ? puddles.range(0.35, 0.8) : 0;

    const wetness = Math.min(1, Math.max(station.wetness, proximityWet, puddle, submerged ? 1 : 0));

    const nearPlatform =
      cell.char === LEGEND.PLATFORM_EDGE ||
      grid.charAt(cell.x, cell.y - 1) === LEGEND.PLATFORM_EDGE ||
      grid.charAt(cell.x, cell.y + 1) === LEGEND.PLATFORM_EDGE ||
      grid.charAt(cell.x - 1, cell.y) === LEGEND.PLATFORM_EDGE ||
      grid.charAt(cell.x + 1, cell.y) === LEGEND.PLATFORM_EDGE;

    const builder = nearPlatform ? platformFloor : dry;

    builder.addQuad(
      new Vector3(x0, y, z1),
      new Vector3(x1, y, z1),
      new Vector3(x1, y, z0),
      new Vector3(x0, y, z0),
      [x0, z1],
      [x1, z1],
      [x1, z0],
      [x0, z0],
      wetness,
      new Vector3(0, 1, 0),
    );

    // Ceiling over every open tile. Faces down, and is registered as an
    // occluder so the camera can dither it away.
    ceiling.addQuad(
      new Vector3(x0, WALL_HEIGHT, z0),
      new Vector3(x1, WALL_HEIGHT, z0),
      new Vector3(x1, WALL_HEIGHT, z1),
      new Vector3(x0, WALL_HEIGHT, z1),
      [x0, z0],
      [x1, z0],
      [x1, z1],
      [x0, z1],
      0,
      new Vector3(0, -1, 0),
    );
  }
}

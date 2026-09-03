/**
 * Platform edges, trackbed and the train stop.
 *
 * A platform edge run is authored as a line of '=' tiles. Which side is the
 * track is inferred rather than authored: whichever side has less walkable
 * floor behind it is the trackbed. That keeps the grids simple and means a
 * platform can face either way without a second character in the legend.
 */

import { Color, Matrix4, Quaternion, Vector3 } from 'three';
import { LEGEND, PLATFORM_LIP, TILE, TRACK_DROP } from '../../data/legend';
import { addProp, builderFor, type EmitterContext } from '../Context';

const WARNING_SETBACK = 0.62;
const TRACKBED_WIDTH = 4.2;
const scale = new Vector3(1, 1, 1);
const identity = new Quaternion();

/** Counts walkable cells within three tiles on one side of a run. */
function openness(
  context: EmitterContext,
  cells: { x: number; y: number }[],
  dx: number,
  dy: number,
): number {
  let count = 0;
  for (const cell of cells) {
    for (let step = 1; step <= 3; step += 1) {
      if (context.grid.isWalkable(cell.x + dx * step, cell.y + dy * step)) count += 1;
    }
  }
  return count;
}

export function emitPlatform(context: EmitterContext): void {
  const { grid, station } = context;
  const runs = grid.runs(LEGEND.PLATFORM_EDGE);
  if (runs.length === 0) return;

  const lip = builderFor(context, { set: 'tile', tilesPerMetre: 0.7, wetness: 0.55 });
  const bed = builderFor(context, { set: 'rubber', tilesPerMetre: 0.8 });
  const wall = builderFor(context, { set: 'painted_brick', tilesPerMetre: 0.42 });

  let longest = { length: 0, position: new Vector3(), yaw: 0 };

  for (const run of runs) {
    if (run.cells.length === 0) continue;

    // Perpendicular candidates for this run's axis.
    const candidates: [number, number][] =
      run.axis === 'x'
        ? [
            [0, 1],
            [0, -1],
          ]
        : [
            [1, 0],
            [-1, 0],
          ];

    const [first, second] = candidates;
    const trackDirection =
      openness(context, run.cells, ...first) <= openness(context, run.cells, ...second)
        ? first
        : second;

    const start = grid.worldPosition(run.cells[0].x, run.cells[0].y);
    const end = grid.worldPosition(
      run.cells[run.cells.length - 1].x,
      run.cells[run.cells.length - 1].y,
    );

    const centre = start.clone().add(end).multiplyScalar(0.5);
    const length = run.cells.length * TILE;

    const alongX = run.axis === 'x';
    const runSize = alongX
      ? new Vector3(length, PLATFORM_LIP, TILE * 0.34)
      : new Vector3(TILE * 0.34, PLATFORM_LIP, length);

    const edgeOffset = new Vector3(trackDirection[0], 0, trackDirection[1]).multiplyScalar(
      TILE * 0.5 - TILE * 0.17,
    );

    // The lip: 0.2m proud of the floor at the very edge.
    lip.addBox(
      centre
        .clone()
        .add(edgeOffset)
        .setY(PLATFORM_LIP / 2),
      runSize,
      0.6,
      0.7,
    );

    // Warning line, set back from the edge, emissive so it catches the torch.
    const warningOffset = new Vector3(trackDirection[0], 0, trackDirection[1]).multiplyScalar(
      TILE * 0.5 - WARNING_SETBACK,
    );
    const warningMatrix = new Matrix4().compose(
      centre
        .clone()
        .add(warningOffset)
        .setY(PLATFORM_LIP + 0.012),
      identity,
      alongX
        ? new Vector3(length / 2.6, 0.06, 0.42 / 0.28)
        : new Vector3(0.42 / 2.6, 0.06, length / 0.28),
    );
    context.strips.push({
      kind: 'warning',
      matrix: warningMatrix,
      colour: new Color(0xe0a030),
      intensity: 1.7,
    });

    // Trackbed: a dropped slab beyond the edge, with a back wall so the player
    // cannot see out of the world.
    const bedCentre = centre
      .clone()
      .add(
        new Vector3(trackDirection[0], 0, trackDirection[1]).multiplyScalar(
          TILE * 0.5 + TRACKBED_WIDTH / 2,
        ),
      )
      .setY(-TRACK_DROP - 0.2);

    bed.addBox(
      bedCentre,
      alongX ? new Vector3(length, 0.4, TRACKBED_WIDTH) : new Vector3(TRACKBED_WIDTH, 0.4, length),
      0.75,
      0.6,
    );

    const backCentre = centre
      .clone()
      .add(
        new Vector3(trackDirection[0], 0, trackDirection[1]).multiplyScalar(
          TILE * 0.5 + TRACKBED_WIDTH,
        ),
      )
      .setY((3.6 - TRACK_DROP) / 2);

    wall.addBox(
      backCentre,
      alongX
        ? new Vector3(length, 3.6 + TRACK_DROP, 0.4)
        : new Vector3(0.4, 3.6 + TRACK_DROP, length),
      0,
      0.42,
    );

    // Rails and sleepers, instanced along the run.
    emitTrack(context, run.cells.length, centre, trackDirection, alongX);

    if (length > longest.length) {
      longest = {
        length,
        position: centre
          .clone()
          .add(
            new Vector3(trackDirection[0], 0, trackDirection[1]).multiplyScalar(
              TILE * 0.5 + TRACKBED_WIDTH / 2,
            ),
          )
          .setY(-TRACK_DROP),
        yaw: alongX ? 0 : Math.PI / 2,
      };
    }

    // A cool strip along the platform edge, which is what gives the floor its
    // reflection to work with.
    const stripMatrix = new Matrix4().compose(
      centre
        .clone()
        .add(edgeOffset)
        .setY(PLATFORM_LIP + 0.08),
      identity,
      alongX ? new Vector3(length / 2.6, 1, 1) : new Vector3(1, 1, length / 2.6),
    );
    context.strips.push({
      kind: 'edge',
      matrix: stripMatrix,
      colour: new Color(station.accent),
      intensity: 1.5,
    });

    const lightCount = Math.max(1, Math.round(length / 4));
    for (let i = 0; i < lightCount; i += 1) {
      const t = lightCount === 1 ? 0.5 : i / (lightCount - 1);
      const offset = (t - 0.5) * length;
      const position = centre.clone();
      if (alongX) position.x += offset;
      else position.z += offset;
      position.y = 0.5;

      context.lightCandidates.push({
        position,
        colour: new Color(station.accent),
        intensity: 4.5,
        range: 9,
        blackoutSensitive: true,
      });
    }
  }

  context.trainStop = { position: longest.position, yaw: longest.yaw };
}

function emitTrack(
  context: EmitterContext,
  tiles: number,
  centre: Vector3,
  trackDirection: [number, number],
  alongX: boolean,
): void {
  const perpendicular = new Vector3(trackDirection[0], 0, trackDirection[1]);
  const along = alongX ? new Vector3(1, 0, 0) : new Vector3(0, 0, 1);
  const length = tiles * TILE;

  const railYaw = alongX ? 0 : Math.PI / 2;
  const rotation = new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), railYaw);

  // Two running rails plus a conductor rail on the far side.
  for (const [offset, isConductor] of [
    [1.5, false],
    [2.94, false],
    [3.9, true],
  ] as const) {
    const railCount = Math.ceil(length / TILE);

    for (let i = 0; i < railCount; i += 1) {
      const t = (i + 0.5) / railCount - 0.5;
      const position = centre
        .clone()
        .addScaledVector(along, t * length)
        .addScaledVector(perpendicular, TILE * 0.5 + offset)
        .setY(-TRACK_DROP + (isConductor ? 0.14 : 0.07));

      addProp(context, 'rail', new Matrix4().compose(position, rotation, scale));
    }
  }

  const sleeperCount = Math.floor(length / 0.6);
  for (let i = 0; i < sleeperCount; i += 1) {
    const t = (i + 0.5) / sleeperCount - 0.5;
    const position = centre
      .clone()
      .addScaledVector(along, t * length)
      .addScaledVector(perpendicular, TILE * 0.5 + 2.2)
      .setY(-TRACK_DROP - 0.02);

    addProp(context, 'sleeper', new Matrix4().compose(position, rotation, scale));
  }
}

/**
 * Prop placement.
 *
 * Anchor tiles get their machine, and the rest is dressing placed by rule:
 * benches and bins along walls, litter scattered by tier, barriers on their
 * tiles. Every prop is one instanced draw call regardless of how many
 * instances exist, so dressing a station densely is nearly free.
 */

import { Color, Matrix4, Quaternion, Vector3 } from 'three';
import { LEGEND, TILE } from '../../data/legend';
import { addProp, type AnchorKind, type EmitterContext } from '../Context';
import { everyNth, jitteredMatrix, wallAdjacentCells, wallFacing, wallOffset } from '../Placement';
import type { PropKind } from '../Kit';

const ANCHOR_PROPS: Record<string, { kind: PropKind; anchor: AnchorKind; prefix: string }> = {
  [LEGEND.WALL_BUY]: { kind: 'wallbuy_plate', anchor: 'wallbuy', prefix: 'W' },
  [LEGEND.PERK]: { kind: 'perk_machine', anchor: 'perk', prefix: 'P' },
  [LEGEND.LOST_PROPERTY]: { kind: 'lost_property', anchor: 'lost_property', prefix: 'L' },
  [LEGEND.UPGRADE]: { kind: 'upgrade_bench', anchor: 'upgrade', prefix: 'U' },
  [LEGEND.DEBRIS]: { kind: 'debris_pile', anchor: 'debris', prefix: 'D' },
  [LEGEND.BOARDED]: { kind: 'boarded_panel', anchor: 'boarded', prefix: 'B' },
};

export function emitProps(context: EmitterContext): void {
  const { grid, station, streams } = context;
  const dressing = streams.get('props-dressing');
  const litterStream = streams.get('props-litter');

  const counters = new Map<string, number>();
  const nextLabel = (prefix: string): string => {
    const count = (counters.get(prefix) ?? 0) + 1;
    counters.set(prefix, count);
    return `${prefix}${count}`;
  };

  // Anchors first, in grid reading order, so labels are stable and the station
  // data's wallbuys and perks maps line up with what is generated.
  for (const cell of grid.cells) {
    const spec = ANCHOR_PROPS[cell.char];
    if (!spec) continue;

    const position = grid.worldPosition(cell.x, cell.y);
    const facing = wallFacing(grid, cell);
    const yaw = facing ?? 0;

    // Wall mounted anchors sit flush; freestanding ones stay centred.
    const wallMounted = spec.kind === 'wallbuy_plate' || spec.kind === 'boarded_panel';
    if (facing !== null && wallMounted) position.add(wallOffset(grid, cell, 0.1));

    // The plate and the boarded panel hang at eye height, not on the floor.
    if (spec.kind === 'wallbuy_plate') position.y = 1.55;

    addProp(
      context,
      spec.kind,
      new Matrix4().compose(
        position,
        new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), yaw),
        new Vector3(1, 1, 1),
      ),
    );

    context.anchors.push({
      kind: spec.anchor,
      label: nextLabel(spec.prefix),
      position: grid.worldPosition(cell.x, cell.y),
      facing: yaw,
      gridX: cell.x,
      gridY: cell.y,
    });

    // Perk machines glow, which is how a player finds them in the dark.
    if (spec.kind === 'perk_machine') {
      context.lightCandidates.push({
        position: grid.worldPosition(cell.x, cell.y).setY(1.3),
        colour: new Color(station.accent),
        intensity: 5.5,
        range: 6,
        blackoutSensitive: false,
      });
    }
  }

  // Ticket barriers, oriented across the wider adjacent floor run.
  for (const cell of grid.find([LEGEND.BARRIER])) {
    const horizontal =
      (grid.isWalkable(cell.x - 1, cell.y) ? 1 : 0) + (grid.isWalkable(cell.x + 1, cell.y) ? 1 : 0);
    const vertical =
      (grid.isWalkable(cell.x, cell.y - 1) ? 1 : 0) + (grid.isWalkable(cell.x, cell.y + 1) ? 1 : 0);

    const yaw = horizontal >= vertical ? 0 : Math.PI / 2;

    addProp(
      context,
      'barrier',
      new Matrix4().compose(
        grid.worldPosition(cell.x, cell.y),
        new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), yaw),
        new Vector3(1, 1, 1),
      ),
    );

    context.anchors.push({
      kind: 'barrier',
      label: nextLabel('X'),
      position: grid.worldPosition(cell.x, cell.y),
      facing: yaw,
      gridX: cell.x,
      gridY: cell.y,
    });
  }

  // Player spawn.
  const spawnCell = grid.find([LEGEND.SPAWN])[0];
  if (spawnCell) {
    context.playerSpawn.copy(grid.worldPosition(spawnCell.x, spawnCell.y));
    context.anchors.push({
      kind: 'spawn',
      label: 'S1',
      position: context.playerSpawn.clone(),
      facing: 0,
      gridX: spawnCell.x,
      gridY: spawnCell.y,
    });
  }

  // Dressing: benches every five tiles along a wall in rooms of any size, bins
  // every nine and offset so the two do not collide.
  for (const region of grid.regions()) {
    if (region.length < 24) continue;

    const wallCells = wallAdjacentCells(grid, region);

    for (const cell of everyNth(wallCells, 5)) {
      const facing = wallFacing(grid, cell);
      if (facing === null) continue;
      const position = grid.worldPosition(cell.x, cell.y).add(wallOffset(grid, cell, 0.34));
      addProp(context, 'bench', jitteredMatrix(position, facing, dressing));
    }

    for (const cell of everyNth(wallCells, 9, 3)) {
      const facing = wallFacing(grid, cell);
      if (facing === null) continue;
      const position = grid.worldPosition(cell.x, cell.y).add(wallOffset(grid, cell, 0.3));
      addProp(context, 'bin', jitteredMatrix(position, facing, dressing));
    }
  }

  // Litter, density scaled by tier: the central stations are filthier.
  const litterChance = 0.04 + station.tier * 0.035;
  for (const cell of grid.cells) {
    if (cell.char !== LEGEND.FLOOR) continue;
    if (!litterStream.chance(litterChance)) continue;

    const position = grid.worldPosition(cell.x, cell.y);
    position.y = 0.005;
    addProp(context, 'litter', jitteredMatrix(position, litterStream.range(0, Math.PI * 2), litterStream));
  }

  void TILE;
}

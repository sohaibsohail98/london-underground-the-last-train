/**
 * The ASCII tile legend. This is the authoring format for all 41 stations and
 * the sole input to the geometry generator, so it is deliberately small and
 * closed: Phase 6 adds stations, never new characters.
 */

export const TILE = 1.5;
export const WALL_HEIGHT = 3.6;
export const PLATFORM_LIP = 0.2;
export const TRACK_DROP = 1.1;
export const SKIRTING_HEIGHT = 0.15;

export const LEGEND = {
  WALL: '#',
  FLOOR: '.',
  PLATFORM_EDGE: '=',
  TUNNEL: 'T',
  BOARDED: 'B',
  DEBRIS: 'D',
  WALL_BUY: 'W',
  PERK: 'P',
  LOST_PROPERTY: 'L',
  UPGRADE: 'U',
  BARRIER: 'X',
  ESCALATOR: 'E',
  SPAWN: 'S',
  WATER: '~',
} as const;

export type TileChar = (typeof LEGEND)[keyof typeof LEGEND];

export const ALL_TILES: TileChar[] = Object.values(LEGEND);

/** Anchor tiles carry a prop or trigger but are walkable floor underneath. */
export const ANCHOR_TILES: TileChar[] = [
  LEGEND.BOARDED,
  LEGEND.DEBRIS,
  LEGEND.WALL_BUY,
  LEGEND.PERK,
  LEGEND.LOST_PROPERTY,
  LEGEND.UPGRADE,
  LEGEND.SPAWN,
];

/** Tiles a capsule may stand on. Trackbed and lips are excluded elsewhere. */
export const WALKABLE_TILES: TileChar[] = [
  LEGEND.FLOOR,
  LEGEND.PLATFORM_EDGE,
  LEGEND.BARRIER,
  LEGEND.ESCALATOR,
  LEGEND.WATER,
  ...ANCHOR_TILES,
];

/** Movement cost multipliers used by the flow field. */
export const TILE_COST: Partial<Record<TileChar, number>> = {
  [LEGEND.WATER]: 3,
  [LEGEND.BARRIER]: 4,
  [LEGEND.ESCALATOR]: 1.4,
};

export function isTileChar(char: string): char is TileChar {
  return (ALL_TILES as string[]).includes(char);
}

export function isWalkable(char: string): boolean {
  return (WALKABLE_TILES as string[]).includes(char);
}

export function isAnchor(char: string): boolean {
  return (ANCHOR_TILES as string[]).includes(char);
}

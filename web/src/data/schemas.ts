/**
 * Data schemas.
 *
 * StationDef is finalised here because the geometry generator consumes it in
 * Phase 3. WeaponDef arrives in Phase 5 alongside the weapon system, and the
 * 38 remaining stations in Phase 6 are pure data against this same shape, with
 * no engine changes permitted.
 */

export type MechanicId =
  | 'flood'
  | 'blackout'
  | 'narrow'
  | 'escalator_rush'
  | 'platform_split'
  | 'open_concourse'
  | 'depot'
  | 'open_air'
  | 'interchange'
  | 'terminus';

export const MECHANIC_IDS: MechanicId[] = [
  'flood',
  'blackout',
  'narrow',
  'escalator_rush',
  'platform_split',
  'open_concourse',
  'depot',
  'open_air',
  'interchange',
  'terminus',
];

export type PerkId = 'constitution' | 'quick_hands' | 'second_wind' | 'double_tap';

export type StationTier = 1 | 2 | 3 | 4;

export interface StationDef {
  id: string;
  displayName: string;
  /** Position along the line, used for adjacency sanity checks and tiering. */
  lineIndex: number;
  adjacent: string[];
  tier: StationTier;
  mechanics: [MechanicId] | [MechanicId, MechanicId];

  /** Rows of equal length using the characters in data/legend.ts. */
  grid: string[];
  /** Any integer. The same grid and seed always build the same station. */
  seed: number;

  /** Palette accent as a hex colour, used by signage and strip lighting. */
  accent: number;
  hdri: { file: string; intensity: number; tint: number };
  /** Post chain exposure multiplier for this station. */
  exposure: number;

  fogDensity: number;
  fogColour: number;
  /** Baseline floor wetness in 0..1, raised locally near water. */
  wetness: number;

  /** Three original lines. Never transcribe a real recording. */
  announcements: [string, string, string];
  ambient: { hum: number; drip: number; wind: number; rumbleDistance: number };

  bossId?: string;

  /** Anchor label to weapon id. Labels are assigned in grid reading order. */
  wallbuys: Record<string, string>;
  perks: Record<string, PerkId>;
  debrisCosts: Record<string, number>;
}

export interface ValidationIssue {
  station: string;
  message: string;
}

/**
 * Validates a station definition. Called on every station at boot in a dev
 * build, because a malformed grid is far cheaper to catch here than as a
 * geometry artefact forty stations later.
 */
export function validateStation(station: StationDef): ValidationIssue[] {
  const issues: ValidationIssue[] = [];
  const fail = (message: string): void => {
    issues.push({ station: station.id, message });
  };

  if (station.grid.length < 8) fail('grid has fewer than 8 rows');

  const width = station.grid[0]?.length ?? 0;
  if (width < 8) fail('grid is narrower than 8 columns');

  for (let i = 0; i < station.grid.length; i += 1) {
    if (station.grid[i].length !== width) {
      fail(`grid row ${i} is ${station.grid[i].length} wide, expected ${width}`);
    }
  }

  const flat = station.grid.join('');
  if (!flat.includes('S')) fail('grid has no player spawn (S)');
  if (!flat.includes('B') && !flat.includes('T')) fail('grid has no spawn points (B or T)');

  for (const mechanic of station.mechanics) {
    if (!MECHANIC_IDS.includes(mechanic)) fail(`unknown mechanic "${mechanic}"`);
  }

  if (station.mechanics.includes('flood') && !flat.includes('~')) {
    fail('flood mechanic but no water tiles (~)');
  }

  if (station.announcements.length !== 3) fail('expected exactly three announcement lines');
  if (station.adjacent.length === 0) fail('station has no adjacent stations');
  if (station.tier < 1 || station.tier > 4) fail(`tier ${station.tier} is out of range`);

  return issues;
}

export function validateRegistry(stations: StationDef[]): ValidationIssue[] {
  const issues = stations.flatMap(validateStation);
  const byId = new Map(stations.map((station) => [station.id, station]));

  for (const station of stations) {
    for (const neighbourId of station.adjacent) {
      const neighbour = byId.get(neighbourId);
      if (!neighbour) {
        issues.push({
          station: station.id,
          message: `adjacency to unknown station "${neighbourId}"`,
        });
        continue;
      }
      if (!neighbour.adjacent.includes(station.id)) {
        issues.push({
          station: station.id,
          message: `adjacency to "${neighbourId}" is not reciprocated`,
        });
      }
    }
  }

  return issues;
}

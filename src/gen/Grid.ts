/**
 * ASCII grid parsing and queries.
 *
 * The grid is the authoring format, so parsing is strict: rows must be equal
 * length and every character must be in the legend. A station that does not
 * parse is a bug in the data, not something to paper over at runtime.
 *
 * Coordinates: grid x runs east, grid y runs south. World x maps from grid x,
 * world z maps from grid y, and the grid is centred on the world origin so
 * that camera and physics maths stays symmetrical.
 */

import { Vector3 } from 'three';
import { isTileChar, isWalkable, LEGEND, TILE, type TileChar } from '../data/legend';

export interface Cell {
  x: number;
  y: number;
  char: TileChar;
  /** True where every orthogonal neighbour is inside the grid. */
  interior: boolean;
}

export interface Run {
  /** Cells in order along the run. */
  cells: Cell[];
  axis: 'x' | 'y';
}

export class Grid {
  readonly width: number;
  readonly height: number;
  readonly cells: Cell[];

  /** World size in metres. */
  readonly worldWidth: number;
  readonly worldDepth: number;

  constructor(rows: string[]) {
    if (rows.length === 0) throw new Error('Grid: no rows');

    this.height = rows.length;
    this.width = rows[0].length;
    this.worldWidth = this.width * TILE;
    this.worldDepth = this.height * TILE;

    this.cells = new Array<Cell>(this.width * this.height);

    for (let y = 0; y < this.height; y += 1) {
      const row = rows[y];
      if (row.length !== this.width) {
        throw new Error(`Grid: row ${y} is ${row.length} wide, expected ${this.width}`);
      }

      for (let x = 0; x < this.width; x += 1) {
        const char = row[x];
        if (!isTileChar(char)) {
          throw new Error(`Grid: unknown tile "${char}" at ${x},${y}`);
        }

        this.cells[y * this.width + x] = {
          x,
          y,
          char,
          interior: x > 0 && y > 0 && x < this.width - 1 && y < this.height - 1,
        };
      }
    }
  }

  inside(x: number, y: number): boolean {
    return x >= 0 && y >= 0 && x < this.width && y < this.height;
  }

  at(x: number, y: number): Cell | null {
    if (!this.inside(x, y)) return null;
    return this.cells[y * this.width + x];
  }

  charAt(x: number, y: number): TileChar | null {
    return this.at(x, y)?.char ?? null;
  }

  is(x: number, y: number, char: TileChar): boolean {
    return this.charAt(x, y) === char;
  }

  /** Cells matching any of the given characters, in row major order. */
  find(chars: TileChar[]): Cell[] {
    const set = new Set<string>(chars);
    return this.cells.filter((cell) => set.has(cell.char));
  }

  /** The four orthogonal neighbours, nulls omitted. */
  neighbours(cell: Cell): Cell[] {
    const result: Cell[] = [];
    for (const [dx, dy] of ORTHOGONAL) {
      const neighbour = this.at(cell.x + dx, cell.y + dy);
      if (neighbour) result.push(neighbour);
    }
    return result;
  }

  /** Whether a cell is solid for the purposes of wall emission. */
  isSolid(x: number, y: number): boolean {
    const char = this.charAt(x, y);
    if (char === null) return true; // outside the grid counts as solid
    return char === LEGEND.WALL;
  }

  isOpen(x: number, y: number): boolean {
    const char = this.charAt(x, y);
    return char !== null && char !== LEGEND.WALL;
  }

  isWalkable(x: number, y: number): boolean {
    const char = this.charAt(x, y);
    return char !== null && isWalkable(char);
  }

  /**
   * Groups matching cells into maximal straight runs. Used for platform edges,
   * tunnel mouths and escalators, all of which are authored as lines.
   */
  runs(char: TileChar): Run[] {
    const seen = new Set<number>();
    const result: Run[] = [];

    for (const cell of this.cells) {
      if (cell.char !== char) continue;
      const key = cell.y * this.width + cell.x;
      if (seen.has(key)) continue;

      // Prefer the axis with more matching neighbours, so a two cell wide run
      // still resolves to a sensible direction.
      const horizontalLength = this.runLength(cell, 1, 0, char);
      const verticalLength = this.runLength(cell, 0, 1, char);
      const axis: 'x' | 'y' = horizontalLength >= verticalLength ? 'x' : 'y';
      const dx = axis === 'x' ? 1 : 0;
      const dy = axis === 'x' ? 0 : 1;

      const cells: Cell[] = [];
      let cursor: Cell | null = cell;
      while (cursor && cursor.char === char) {
        const cursorKey = cursor.y * this.width + cursor.x;
        if (seen.has(cursorKey)) break;
        seen.add(cursorKey);
        cells.push(cursor);
        cursor = this.at(cursor.x + dx, cursor.y + dy);
      }

      result.push({ cells, axis });
    }

    return result;
  }

  private runLength(start: Cell, dx: number, dy: number, char: TileChar): number {
    let length = 0;
    let cursor: Cell | null = start;
    while (cursor && cursor.char === char) {
      length += 1;
      cursor = this.at(cursor.x + dx, cursor.y + dy);
    }
    return length;
  }

  /**
   * Flood fills open regions, returning one array of cells per region. The
   * platform region is later identified as whichever region touches a
   * platform edge.
   */
  regions(): Cell[][] {
    const visited = new Uint8Array(this.width * this.height);
    const result: Cell[][] = [];

    for (const cell of this.cells) {
      const key = cell.y * this.width + cell.x;
      if (visited[key] === 1) continue;
      if (!this.isOpen(cell.x, cell.y)) {
        visited[key] = 1;
        continue;
      }

      const region: Cell[] = [];
      const stack: Cell[] = [cell];
      visited[key] = 1;

      while (stack.length > 0) {
        const current = stack.pop() as Cell;
        region.push(current);

        for (const [dx, dy] of ORTHOGONAL) {
          const next = this.at(current.x + dx, current.y + dy);
          if (!next) continue;
          const nextKey = next.y * this.width + next.x;
          if (visited[nextKey] === 1) continue;
          if (!this.isOpen(next.x, next.y)) continue;
          visited[nextKey] = 1;
          stack.push(next);
        }
      }

      result.push(region);
    }

    return result;
  }

  /** Centre of a cell in world space, at floor level. */
  worldPosition(x: number, y: number, out = new Vector3()): Vector3 {
    return out.set(
      (x - (this.width - 1) / 2) * TILE,
      0,
      (y - (this.height - 1) / 2) * TILE,
    );
  }

  /** World space corner of a cell, minus x and minus z. */
  worldCorner(x: number, y: number, out = new Vector3()): Vector3 {
    return out.set(
      (x - this.width / 2) * TILE,
      0,
      (y - this.height / 2) * TILE,
    );
  }

  /** Inverse of worldPosition, clamped to the grid. */
  gridFromWorld(worldX: number, worldZ: number): { x: number; y: number } {
    const x = Math.round(worldX / TILE + (this.width - 1) / 2);
    const y = Math.round(worldZ / TILE + (this.height - 1) / 2);
    return {
      x: Math.min(Math.max(x, 0), this.width - 1),
      y: Math.min(Math.max(y, 0), this.height - 1),
    };
  }
}

export const ORTHOGONAL: ReadonlyArray<readonly [number, number]> = [
  [1, 0],
  [-1, 0],
  [0, 1],
  [0, -1],
];

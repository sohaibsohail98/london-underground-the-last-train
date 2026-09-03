/**
 * Seeded random. PCG32, chosen because it is small, fast, has good statistical
 * quality and, crucially, is reproducible across machines, which Math.random
 * is not.
 *
 * Every system that needs randomness takes its own named stream. Streams are
 * derived by hashing the stream name into the seed, so adding a new stream in a
 * later phase cannot shift the output of an existing one. That is what makes
 * station generation deterministic: the same grid and seed always build the
 * same station, whatever else the game does.
 */

const MULTIPLIER = 6364136223846793005n;
const MASK_64 = (1n << 64n) - 1n;
const MASK_32 = 0xffffffff;

/** FNV-1a over a string, used to derive stream seeds from names. */
export function hashString(text: string, seed = 2166136261): number {
  let hash = seed >>> 0;
  for (let i = 0; i < text.length; i += 1) {
    hash ^= text.charCodeAt(i);
    hash = Math.imul(hash, 16777619) >>> 0;
  }
  return hash >>> 0;
}

/** Mixes two integers into one, for combining a station seed with a stream. */
export function mixSeeds(a: number, b: number): number {
  let h = (a ^ Math.imul(b, 0x9e3779b1)) >>> 0;
  h ^= h >>> 16;
  h = Math.imul(h, 0x85ebca6b) >>> 0;
  h ^= h >>> 13;
  h = Math.imul(h, 0xc2b2ae35) >>> 0;
  return (h ^ (h >>> 16)) >>> 0;
}

export class Random {
  private state: bigint;
  private readonly increment: bigint;

  constructor(seed: number, sequence = 0) {
    this.increment = ((BigInt(sequence >>> 0) << 1n) | 1n) & MASK_64;
    this.state = 0n;
    this.nextUint32();
    this.state = (this.state + BigInt(seed >>> 0)) & MASK_64;
    this.nextUint32();
  }

  /** A fresh generator for a named sub stream of this seed. */
  static stream(seed: number, name: string): Random {
    return new Random(mixSeeds(seed, hashString(name)), hashString(name) & 0x7fffffff);
  }

  nextUint32(): number {
    const previous = this.state;
    this.state = (previous * MULTIPLIER + this.increment) & MASK_64;

    const xorshifted = Number(((previous >> 18n) ^ previous) >> 27n) & MASK_32;
    const rotation = Number(previous >> 59n) & 31;

    return ((xorshifted >>> rotation) | (xorshifted << ((-rotation >>> 0) & 31))) >>> 0;
  }

  /** Uniform in [0, 1). */
  next(): number {
    return this.nextUint32() / 4294967296;
  }

  /** Uniform in [min, max). */
  range(min: number, max: number): number {
    return min + this.next() * (max - min);
  }

  /** Uniform integer in [min, max]. */
  int(min: number, max: number): number {
    return min + Math.floor(this.next() * (max - min + 1));
  }

  /** True with the given probability. */
  chance(probability: number): boolean {
    return this.next() < probability;
  }

  pick<T>(items: readonly T[]): T {
    if (items.length === 0) throw new Error('Random.pick called with an empty list');
    return items[this.int(0, items.length - 1)];
  }

  /** Fisher Yates, in place. */
  shuffle<T>(items: T[]): T[] {
    for (let i = items.length - 1; i > 0; i -= 1) {
      const j = this.int(0, i);
      const swap = items[i];
      items[i] = items[j];
      items[j] = swap;
    }
    return items;
  }

  /** Signed jitter in [-amount, amount]. */
  jitter(amount: number): number {
    return this.range(-amount, amount);
  }
}

/**
 * A named set of streams for one station build. The generator asks for streams
 * by name and never shares one between emitters, so emitter order changes do
 * not alter output.
 */
export class StreamSet {
  private readonly seed: number;
  private readonly streams = new Map<string, Random>();

  constructor(seed: number) {
    this.seed = seed;
  }

  get(name: string): Random {
    const existing = this.streams.get(name);
    if (existing) return existing;

    const created = Random.stream(this.seed, name);
    this.streams.set(name, created);
    return created;
  }
}

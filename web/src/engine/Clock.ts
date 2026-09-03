/**
 * Fixed timestep clock. The simulation always advances in whole 60Hz ticks so
 * that behaviour is deterministic regardless of display refresh rate; the
 * renderer interpolates between the last two states using {@link Clock.alpha}.
 */

export const TICK_HZ = 60;
export const DT = 1 / TICK_HZ;
export const DT_MS = 1000 / TICK_HZ;

/** Above this many ticks in one frame we drop the remainder rather than spiral. */
const MAX_TICKS_PER_FRAME = 5;

export class Clock {
  /** Total ticks since boot. Monotonic, never reset. */
  tick = 0;

  /** Interpolation factor in 0..1 between the previous and current tick. */
  alpha = 0;

  /** Wall clock seconds since boot, used by render side effects only. */
  elapsed = 0;

  /** Real frame delta in seconds, unsmoothed. */
  frameDelta = 0;

  /** Smoothed frame delta in milliseconds, for the profiler. */
  smoothedFrameMs = DT_MS;

  /** True on any frame where we hit the tick cap and discarded time. */
  stalled = false;

  private accumulator = 0;
  private lastTimeMs: number | null = null;

  /**
   * Advances the clock and returns how many simulation ticks are owed. The
   * caller runs exactly that many ticks, then reads {@link Clock.alpha}.
   */
  advance(nowMs: number): number {
    if (this.lastTimeMs === null) {
      this.lastTimeMs = nowMs;
      return 0;
    }

    let deltaMs = nowMs - this.lastTimeMs;
    this.lastTimeMs = nowMs;

    // A tab that has been backgrounded returns an enormous delta. Clamp it so
    // we do not run hundreds of ticks on the first frame back.
    if (deltaMs > 250) deltaMs = 250;
    if (deltaMs < 0) deltaMs = 0;

    this.frameDelta = deltaMs / 1000;
    this.elapsed += this.frameDelta;
    this.smoothedFrameMs += (deltaMs - this.smoothedFrameMs) * 0.1;

    this.accumulator += this.frameDelta;

    let ticks = 0;
    this.stalled = false;

    while (this.accumulator >= DT) {
      this.accumulator -= DT;
      ticks += 1;

      if (ticks >= MAX_TICKS_PER_FRAME) {
        if (this.accumulator >= DT) this.stalled = true;
        this.accumulator = 0;
        break;
      }
    }

    this.tick += ticks;
    this.alpha = this.accumulator / DT;

    return ticks;
  }

  /** Frames per second derived from the smoothed frame time. */
  get fps(): number {
    return this.smoothedFrameMs > 0 ? 1000 / this.smoothedFrameMs : 0;
  }

  /** Resets timing without touching the tick counter. */
  resync(): void {
    this.lastTimeMs = null;
    this.accumulator = 0;
    this.alpha = 0;
  }
}

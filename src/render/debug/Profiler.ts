/**
 * Profiler overlay.
 *
 * Honest note on per stage GPU cost: three exposes one timestamp pair for the
 * whole frame, not one per pass, so a true per stage breakdown is not
 * available from the public API. Instead the profiler can run a probe: it
 * disables one expensive stage for a short window, measures the difference in
 * mean frame GPU time, and reports that delta as the stage's cost. It is an
 * A/B measurement rather than a query, which is slower to converge but is
 * measuring the thing that actually matters.
 *
 * Press F3 to show or hide, F4 to start a probe sweep.
 */

import { TimestampQuery } from 'three';
import type { WebGPURenderer } from 'three/webgpu';
import type { Clock } from '../../engine/Clock';
import type { BackendCapabilities } from '../Backend';
import type { Lighting } from '../Lighting';
import { REBUILD_STAGES, type RenderGraph, type StageName } from '../RenderGraph';

interface RollingMean {
  samples: number[];
  index: number;
  filled: boolean;
}

function makeRolling(size: number): RollingMean {
  return { samples: new Array<number>(size).fill(0), index: 0, filled: false };
}

function push(rolling: RollingMean, value: number): void {
  rolling.samples[rolling.index] = value;
  rolling.index = (rolling.index + 1) % rolling.samples.length;
  if (rolling.index === 0) rolling.filled = true;
}

function mean(rolling: RollingMean): number {
  const count = rolling.filled ? rolling.samples.length : rolling.index;
  if (count === 0) return 0;
  let total = 0;
  for (let i = 0; i < count; i += 1) total += rolling.samples[i];
  return total / count;
}

type ProbePhase = 'idle' | 'baseline' | 'measuring' | 'settling';

const PROBE_FRAMES = 90;

export class Profiler {
  visible = true;

  /** Measured cost per stage in milliseconds of GPU time, from the probe. */
  readonly stageCost = new Map<StageName, number>();

  private readonly element: HTMLDivElement;
  private readonly renderer: WebGPURenderer;
  private readonly capabilities: BackendCapabilities;
  private readonly graph: RenderGraph;
  private readonly lighting: Lighting;

  private readonly cpu = makeRolling(60);
  private readonly gpu = makeRolling(60);
  private readonly probeGpu = makeRolling(PROBE_FRAMES);

  private cpuStart = 0;
  private lastGpuMs = 0;
  private pendingTimestamp = false;

  private probePhase: ProbePhase = 'idle';
  private probeQueue: StageName[] = [];
  private probeStage: StageName | null = null;
  private probeBaseline = 0;
  private probeFrames = 0;

  private entityCount = 0;
  private vatInstances = 0;

  constructor(
    parent: HTMLElement,
    renderer: WebGPURenderer,
    capabilities: BackendCapabilities,
    graph: RenderGraph,
    lighting: Lighting,
  ) {
    this.renderer = renderer;
    this.capabilities = capabilities;
    this.graph = graph;
    this.lighting = lighting;

    this.element = document.createElement('div');
    this.element.style.cssText = [
      'position:absolute',
      'top:12px',
      'left:12px',
      'padding:10px 12px',
      'font:11px/1.55 ui-monospace,SFMono-Regular,Menlo,monospace',
      'color:#cfd2da',
      'background:rgba(10,10,14,0.72)',
      'border:1px solid rgba(224,160,48,0.28)',
      'border-radius:4px',
      'white-space:pre',
      'pointer-events:none',
      'min-width:232px',
    ].join(';');
    parent.appendChild(this.element);
  }

  /** Counts reported by the game side, shown for context. */
  setEntityCounts(entities: number, vatInstances: number): void {
    this.entityCount = entities;
    this.vatInstances = vatInstances;
  }

  toggle(): void {
    this.visible = !this.visible;
    this.element.style.display = this.visible ? 'block' : 'none';
  }

  /** Starts an A/B sweep over the expensive stages. */
  startProbe(): void {
    if (this.probePhase !== 'idle') return;
    if (!this.capabilities.timestamps) return;

    this.probeQueue = REBUILD_STAGES.filter((name) => this.graph.currentToggles[name]);
    if (this.probeQueue.length === 0) return;

    this.probePhase = 'baseline';
    this.probeFrames = 0;
    this.probeGpu.index = 0;
    this.probeGpu.filled = false;
  }

  beginFrame(): void {
    this.cpuStart = performance.now();
  }

  /** Call after the render graph has executed. */
  endFrame(clock: Clock): void {
    push(this.cpu, performance.now() - this.cpuStart);

    if (this.capabilities.timestamps && !this.pendingTimestamp) {
      this.pendingTimestamp = true;
      this.renderer
        .resolveTimestampsAsync(TimestampQuery.RENDER)
        .then((value) => {
          if (typeof value === 'number') {
            this.lastGpuMs = value;
            push(this.gpu, value);
            if (this.probePhase !== 'idle') push(this.probeGpu, value);
          }
        })
        .catch(() => {
          // Timestamp queries can be unavailable mid session; not fatal.
        })
        .finally(() => {
          this.pendingTimestamp = false;
        });
    }

    this.stepProbe();

    if (this.visible) this.draw(clock);
  }

  private stepProbe(): void {
    if (this.probePhase === 'idle') return;

    this.probeFrames += 1;
    if (this.probeFrames < PROBE_FRAMES) return;

    this.probeFrames = 0;

    if (this.probePhase === 'baseline') {
      this.probeBaseline = mean(this.probeGpu);
      this.probeStage = this.probeQueue.shift() ?? null;

      if (!this.probeStage) {
        this.probePhase = 'idle';
        return;
      }

      this.graph.setStageEnabled(this.probeStage, false);
      this.probePhase = 'settling';
      return;
    }

    if (this.probePhase === 'settling') {
      // One window discarded after a rebuild, so shader compilation and cache
      // warming do not land in the measurement.
      this.probeGpu.index = 0;
      this.probeGpu.filled = false;
      this.probePhase = 'measuring';
      return;
    }

    if (this.probePhase === 'measuring' && this.probeStage) {
      const withoutStage = mean(this.probeGpu);
      this.stageCost.set(this.probeStage, Math.max(0, this.probeBaseline - withoutStage));

      this.graph.setStageEnabled(this.probeStage, true);
      this.probeStage = this.probeQueue.shift() ?? null;

      if (!this.probeStage) {
        this.probePhase = 'idle';
        return;
      }

      this.graph.setStageEnabled(this.probeStage, false);
      this.probePhase = 'settling';
    }
  }

  private draw(clock: Clock): void {
    const info = this.renderer.info.render;
    const cpuMs = mean(this.cpu);
    const gpuMs = mean(this.gpu);

    const lines: string[] = [
      `LAST TRAIN  ${this.capabilities.kind.toUpperCase()}`,
      `${this.capabilities.adapter.slice(0, 28)}`,
      '',
      `fps        ${clock.fps.toFixed(1).padStart(6)}${clock.stalled ? '  stall' : ''}`,
      `frame      ${clock.smoothedFrameMs.toFixed(2).padStart(6)} ms`,
      `cpu        ${cpuMs.toFixed(2).padStart(6)} ms`,
      this.capabilities.timestamps
        ? `gpu        ${gpuMs.toFixed(2).padStart(6)} ms`
        : 'gpu           n/a (webgl2)',
      '',
      `draws      ${String(info.drawCalls).padStart(6)}`,
      `tris       ${String(info.triangles).padStart(6)}`,
      `lights     ${String(this.lighting.activeLights).padStart(6)} / ${this.lighting.candidateCount}`,
      `entities   ${String(this.entityCount).padStart(6)}`,
      `vat        ${String(this.vatInstances).padStart(6)}`,
      `blits      ${String(this.graph.materialisations).padStart(6)}`,
      '',
      'stages',
    ];

    const toggles = this.graph.currentToggles;
    for (const name of Object.keys(toggles) as StageName[]) {
      const on = toggles[name];
      const cost = this.stageCost.get(name);
      const costText = cost !== undefined ? `${cost.toFixed(2)} ms` : '';
      lines.push(`  ${on ? 'x' : ' '} ${name.padEnd(11)}${costText}`);
    }

    if (this.probePhase !== 'idle') {
      lines.push('', `probing ${this.probeStage ?? ''} (${this.probePhase})`);
    }

    lines.push('', 'F3 hud  F4 probe  1/2/3 preset');

    this.element.textContent = lines.join('\n');
  }

  get gpuMs(): number {
    return this.lastGpuMs;
  }

  dispose(): void {
    this.element.remove();
  }
}

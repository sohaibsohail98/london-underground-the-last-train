/**
 * DOM overlay root.
 *
 * The HUD lives in the DOM rather than in the 3D scene, so text stays crisp
 * and costs nothing on the GPU. Phase 7 fills this in properly; for now it
 * owns the boot panel, the control hints and the bridge that carries health
 * into the grain and vignette uniforms.
 */

import type { RenderGraph } from '../render/RenderGraph';

export class Overlay {
  readonly root: HTMLElement;

  private readonly hint: HTMLDivElement;
  private readonly boot: HTMLElement | null;
  private readonly status: HTMLElement | null;
  private hitFlash = 0;

  constructor(root: HTMLElement) {
    this.root = root;
    this.boot = document.getElementById('boot');
    this.status = document.getElementById('boot-status');

    this.hint = document.createElement('div');
    this.hint.style.cssText = [
      'position:absolute',
      'bottom:14px',
      'left:50%',
      'transform:translateX(-50%)',
      'font:11px/1.6 ui-monospace,SFMono-Regular,Menlo,monospace',
      'letter-spacing:0.06em',
      'color:#7b7b88',
      'text-align:center',
      'pointer-events:none',
    ].join(';');
    this.hint.textContent =
      'WASD move   click fire   F torch   B blackout   G flood   H damage   F2 fly   F3 profiler   F4 probe   1/2/3 preset';
    root.appendChild(this.hint);
  }

  setStatus(text: string): void {
    if (this.status) this.status.textContent = text;
  }

  dismissBoot(): void {
    if (!this.boot) return;
    this.boot.classList.add('gone');
    window.setTimeout(() => this.boot?.remove(), 500);
  }

  /** Fires the one shot crimson flash used when the player takes a hit. */
  flash(amount = 0.6): void {
    this.hitFlash = Math.min(1, this.hitFlash + amount);
  }

  /**
   * Pushes health into the post chain. Health is 0..1; the grain ramp and the
   * vignette pulse both read from it, which is the whole damage feedback
   * channel until the HUD exists.
   */
  update(graph: RenderGraph, health: number, dt: number): void {
    const damage = Math.min(1, Math.max(0, 1 - health));

    this.hitFlash = Math.max(0, this.hitFlash - dt * 2.2);

    if (graph.stages.grain) graph.stages.grain.damageAmount.value = damage;
    if (graph.stages.vignette) {
      graph.stages.vignette.damageAmount.value = damage;
      graph.stages.vignette.hitFlash.value = this.hitFlash;
    }
  }

  dispose(): void {
    this.hint.remove();
  }
}

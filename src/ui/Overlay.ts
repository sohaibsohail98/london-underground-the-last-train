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
  private readonly reticle: HTMLDivElement;
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
    this.reticle = document.createElement('div');
    this.reticle.style.cssText = [
      'position:absolute',
      'left:50%',
      'top:50%',
      'border:1px solid rgba(224,224,232,0.72)',
      'border-radius:50%',
      'transform:translate(-50%,-50%)',
      'pointer-events:none',
      'transition:border-color 0.12s ease',
    ].join(';');
    root.appendChild(this.reticle);

    this.hint.textContent =
      'WASD move   left click fire   right click aim   F torch   B blackout   G flood   [ ] brightness   F2 fly   F3 profiler   1/2/3 preset';
    root.appendChild(this.hint);
  }

  /**
   * Sizes the reticle to the current spread cone. This is the only honest way
   * to communicate hip fire versus aimed: the circle is the actual cone the
   * shot can land in, not a decorative crosshair.
   */
  setReticle(spreadDeg: number, aiming: boolean, screenHeight = window.innerHeight): void {
    // Convert the cone half angle to a pixel radius at the screen centre,
    // assuming a 62 degree vertical field of view.
    const pixels = (spreadDeg / 31) * (screenHeight / 2);
    const diameter = Math.max(8, Math.min(240, pixels * 2));

    this.reticle.style.width = `${diameter.toFixed(1)}px`;
    this.reticle.style.height = `${diameter.toFixed(1)}px`;
    this.reticle.style.borderColor = aiming
      ? 'rgba(224,160,48,0.9)'
      : 'rgba(224,224,232,0.6)';
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
    this.reticle.remove();
  }
}

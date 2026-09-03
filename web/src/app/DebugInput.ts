/**
 * Debug input. Owns every listener and exposes the resulting state, so main
 * stays an assembly file. Replaced by engine/Input in Phase 4.
 */

import { Vector2 } from 'three';

export interface InputState {
  readonly keys: ReadonlySet<string>;
  firing: boolean;
  aiming: boolean;
  readonly pointer: Vector2;
  forward: number;
  strafe: number;
  sprinting: boolean;
}

export interface InputHandlers {
  /** Fires once per keydown, after the state has been updated. */
  onKey(code: string): void;
  /** Called when focus is lost, so held inputs do not stick. */
  onBlur(): void;
  /** Suppresses fire and aim while the debug camera has control. */
  isSuspended(): boolean;
}

const AIM_BUTTON = 2;
const FIRE_BUTTON = 0;

export function bindInput(handlers: InputHandlers): {
  state: InputState;
  detach: () => void;
} {
  const keys = new Set<string>();
  const pointer = new Vector2(0, 0);

  const state: InputState = {
    keys,
    firing: false,
    aiming: false,
    pointer,
    forward: 0,
    strafe: 0,
    sprinting: false,
  };

  const refreshAxes = (): void => {
    state.forward = (keys.has('KeyW') ? 1 : 0) - (keys.has('KeyS') ? 1 : 0);
    state.strafe = (keys.has('KeyD') ? 1 : 0) - (keys.has('KeyA') ? 1 : 0);
    state.sprinting = keys.has('ShiftLeft') || keys.has('ShiftRight');
  };

  const onKeyDown = (event: KeyboardEvent): void => {
    keys.add(event.code);
    refreshAxes();
    handlers.onKey(event.code);
  };

  const onKeyUp = (event: KeyboardEvent): void => {
    keys.delete(event.code);
    refreshAxes();
  };

  const onPointerMove = (event: PointerEvent): void => {
    pointer.set(
      (event.clientX / window.innerWidth) * 2 - 1,
      -(event.clientY / window.innerHeight) * 2 + 1,
    );
  };

  const onPointerDown = (event: PointerEvent): void => {
    if (handlers.isSuspended()) return;
    if (event.button === FIRE_BUTTON) state.firing = true;
    if (event.button === AIM_BUTTON) state.aiming = true;
  };

  const onPointerUp = (event: PointerEvent): void => {
    if (event.button === FIRE_BUTTON) state.firing = false;
    if (event.button === AIM_BUTTON) state.aiming = false;
  };

  const onContextMenu = (event: MouseEvent): void => {
    if (!handlers.isSuspended()) event.preventDefault();
  };

  const onBlur = (): void => {
    keys.clear();
    refreshAxes();
    state.firing = false;
    state.aiming = false;
    handlers.onBlur();
  };

  window.addEventListener('keydown', onKeyDown);
  window.addEventListener('keyup', onKeyUp);
  window.addEventListener('pointermove', onPointerMove);
  window.addEventListener('pointerdown', onPointerDown);
  window.addEventListener('pointerup', onPointerUp);
  window.addEventListener('contextmenu', onContextMenu);
  window.addEventListener('blur', onBlur);

  return {
    state,
    detach: () => {
      window.removeEventListener('keydown', onKeyDown);
      window.removeEventListener('keyup', onKeyUp);
      window.removeEventListener('pointermove', onPointerMove);
      window.removeEventListener('pointerdown', onPointerDown);
      window.removeEventListener('pointerup', onPointerUp);
      window.removeEventListener('contextmenu', onContextMenu);
      window.removeEventListener('blur', onBlur);
    },
  };
}

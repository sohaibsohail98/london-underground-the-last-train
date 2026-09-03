/**
 * Debug key bindings. One place to look for what a key does, and no switch
 * statement buried in the boot sequence.
 */

import { PRESET_ORDER, type PresetName } from '../render/Presets';
import type { StageName } from '../render/RenderGraph';
import { syncCrowdToPreset, type Session } from './Session';

/** Expensive stages, bound to function keys for judging their contribution. */
const STAGE_KEYS: Record<string, StageName> = {
  F6: 'ssao',
  F7: 'ssr',
  F8: 'volumetric',
  F9: 'bloom',
};

const FLOOD_STEP = 0.3;
const FLOOD_MAX = 1.2;
const HIT_DAMAGE = 0.22;
const BRIGHTNESS_STEP = 0.1;

export interface ToggleState {
  torchOn: boolean;
  blackout: boolean;
  floodHeight: number;
}

export function createKeyHandler(session: Session, toggles: ToggleState): (code: string) => void {
  const { renderer, station, crowd, player, overlay } = session;

  const actions: Record<string, () => void> = {
    KeyF: () => {
      toggles.torchOn = !toggles.torchOn;
      renderer.torch.setEnabled(toggles.torchOn);
    },
    KeyB: () => {
      toggles.blackout = !toggles.blackout;
      renderer.lighting.setBlackout(toggles.blackout);
    },
    KeyG: () => {
      toggles.floodHeight = toggles.floodHeight >= FLOOD_MAX ? 0 : toggles.floodHeight + FLOOD_STEP;
      station.water?.setHeight(toggles.floodHeight);
    },
    KeyH: () => {
      player.damage(HIT_DAMAGE);
      overlay.flash(0.7);
      renderer.rig.addShake(0.5);
    },
    KeyJ: () => player.heal(),
    BracketLeft: () => renderer.setBrightness(renderer.brightness - BRIGHTNESS_STEP),
    BracketRight: () => renderer.setBrightness(renderer.brightness + BRIGHTNESS_STEP),
    F3: () => renderer.profiler.toggle(),
    F4: () => renderer.profiler.startProbe(),
  };

  return (code: string): void => {
    const action = actions[code];
    if (action) {
      action();
      return;
    }

    const stage = STAGE_KEYS[code];
    if (stage) {
      renderer.setStageEnabled(stage, !renderer.graph.currentToggles[stage]);
      return;
    }

    if (code.startsWith('Digit')) {
      const preset: PresetName | undefined = PRESET_ORDER[Number(code.slice(-1)) - 1];
      if (preset) {
        renderer.setPreset(preset);
        syncCrowdToPreset(crowd, renderer);
      }
    }
  };
}

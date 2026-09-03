/**
 * Debug station.
 *
 * Not part of the line. This exists so Phase 3 has a grid to generate and a
 * space to test the crowd in, and it deliberately contains every character in
 * the legend at least once, so a missing emitter shows up immediately rather
 * than forty stations later. The three real reference stations arrive in
 * Phase 5 and this one stays for regression testing.
 */

import type { StationDef } from '../schemas';

export const debugYard: StationDef = {
  id: 'debug-yard',
  displayName: 'Engineering Yard',
  lineIndex: -1,
  adjacent: ['debug-yard'],
  tier: 2,
  mechanics: ['flood', 'interchange'],

  grid: [
    '################################################',
    '################################################',
    '#T============================================T#',
    '#.....W.............W....................W.....#',
    '#..............................................#',
    '#.......................S......................#',
    '#..............................................#',
    '#########D....####################....D#########',
    '#..P....................B...................P..#',
    '#B............................................B#',
    '#...X..X..X..X..X.............X..X..X..X..X....#',
    '#..............................................#',
    '#...................EEEEEEEE...................#',
    '#...................EEEEEEEE...................#',
    '#B..................EEEEEEEE..................B#',
    '#...................EEEEEEEE...................#',
    '#.L..........................................U.#',
    '#..............................................#',
    '#.........W.........................W..........#',
    '#.....~~~~~~~~~~~~~............................#',
    '#.....~~~~~~~~~~~~~...........~~~~~~~~~~~~.....#',
    '#B....~~~~~~~~~~~~~...........~~~~~~~~~~~~....B#',
    '#.....~~~~~~~~~~~~~...........~~~~~~~~~~~~.....#',
    '#.....~~~~~~~~~~~~~...........~~~~~~~~~~~~.....#',
    '#.......................P......................#',
    '############B###########B###########B###########',
  ],
  seed: 20260903,

  accent: 0x6c4c9c,
  hdri: { file: 'sodium_interior.hdr', intensity: 0.55, tint: 0xe0a030 },
  exposure: 1,

  fogDensity: 0.03,
  fogColour: 0x16161c,
  wetness: 0.25,

  announcements: [
    'This is a non-passenger platform. Please do not board.',
    'Engineering works are in progress. Mind your step.',
    'The next service does not stop here.',
  ],
  ambient: { hum: 0.4, drip: 0.7, wind: 0.15, rumbleDistance: 30 },

  wallbuys: {},
  perks: {},
  debrisCosts: {},
};

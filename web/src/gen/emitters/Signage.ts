/**
 * Signage and advertising.
 *
 * Wayfinding panels and ad frames are placed on wall faces, and their artwork
 * is generated from seeded gradients and shape composition. Nothing here
 * reproduces any real mark: no roundel, no official logo, no line diagram, and
 * the typeface is whatever geometric sans the page is using, never Johnston.
 * The wording is invented.
 */

import {
  CanvasTexture,
  Color,
  Mesh,
  Matrix4,
  PlaneGeometry,
  Quaternion,
  SRGBColorSpace,
  Vector3,
} from 'three';
import { MeshBasicNodeMaterial } from 'three/webgpu';
import { texture as textureNode, uv, vec4 } from 'three/tsl';
import { LEGEND } from '../../data/legend';
import { addProp, type EmitterContext } from '../Context';
import { everyNth, wallFacing, wallOffset, isWallAdjacent } from '../Placement';
import type { Random } from '../../engine/Random';

/** Invented destinations, so no real service pattern is reproduced. */
const WAYFINDING_WORDS = [
  'WAY OUT',
  'EASTBOUND',
  'WESTBOUND',
  'LIFTS',
  'NO EXIT',
  'STAFF ONLY',
  'TO THE STREET',
];

const AD_WORDS = [
  'DRINK MORE TEA',
  'THE FUTURE IS FINE',
  'MIND THE OTHERS',
  'STILL HIRING',
  'SLEEP WELL',
  'ASK US ANYTHING',
];

function drawPanel(
  text: string,
  accent: number,
  random: Random,
  width: number,
  height: number,
): CanvasTexture {
  const canvas = document.createElement('canvas');
  canvas.width = width;
  canvas.height = height;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('Signage: 2D context unavailable');

  const base = new Color(accent);
  const shift = random.range(-0.14, 0.14);

  const gradient = context.createLinearGradient(0, 0, width, height);
  gradient.addColorStop(0, `#${base.clone().offsetHSL(shift, 0.1, -0.22).getHexString()}`);
  gradient.addColorStop(
    1,
    `#${base
      .clone()
      .offsetHSL(shift * -1, 0, -0.42)
      .getHexString()}`,
  );
  context.fillStyle = gradient;
  context.fillRect(0, 0, width, height);

  // Seeded shape composition: a few soft blocks, so no two panels match.
  context.globalAlpha = 0.16;
  for (let i = 0; i < 5; i += 1) {
    context.fillStyle = i % 2 === 0 ? '#ffffff' : '#000000';
    const w = random.range(width * 0.15, width * 0.7);
    const h = random.range(height * 0.1, height * 0.5);
    context.fillRect(random.range(0, width - w), random.range(0, height - h), w, h);
  }
  context.globalAlpha = 1;

  const fontSize = Math.round(height * 0.16);
  context.font = `600 ${fontSize}px "Helvetica Neue", Arial, sans-serif`;
  context.fillStyle = '#f2efe8';
  context.textAlign = 'center';
  context.textBaseline = 'middle';
  context.fillText(text, width / 2, height / 2, width * 0.88);

  const texture = new CanvasTexture(canvas);
  texture.colorSpace = SRGBColorSpace;
  return texture;
}

function panelMaterial(texture: CanvasTexture, brightness: number): MeshBasicNodeMaterial {
  const material = new MeshBasicNodeMaterial();
  const sample = textureNode(texture, uv());
  // Slightly emissive so signage is legible by torchlight and catches a touch
  // of bloom, which is exactly how a backlit panel behaves.
  material.colorNode = vec4(sample.rgb.mul(brightness), 1);
  return material;
}

export function emitSignage(context: EmitterContext): void {
  const { grid, station, streams } = context;
  const signStream = streams.get('signage');

  const adCells: typeof grid.cells = [];
  const wayCells: typeof grid.cells = [];

  for (const cell of grid.cells) {
    if (cell.char !== LEGEND.FLOOR && cell.char !== LEGEND.PLATFORM_EDGE) continue;
    if (!isWallAdjacent(grid, cell)) continue;

    if (cell.char === LEGEND.PLATFORM_EDGE) adCells.push(cell);
    else wayCells.push(cell);
  }

  // Ad frames on platform walls every six tiles, at 1.6m.
  for (const cell of everyNth(adCells, 6, 2)) {
    const facing = wallFacing(grid, cell);
    if (facing === null) continue;

    const position = grid.worldPosition(cell.x, cell.y).add(wallOffset(grid, cell, 0.14));
    position.y = 1.6;

    const rotation = new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), facing);
    addProp(context, 'ad_frame', new Matrix4().compose(position, rotation, new Vector3(1, 1, 1)));

    const texture = drawPanel(signStream.pick(AD_WORDS), station.accent, signStream, 256, 360);
    const face = new Mesh(new PlaneGeometry(1.42, 2.02), panelMaterial(texture, 1.15));
    face.position
      .copy(position)
      .addScaledVector(new Vector3(Math.sin(facing), 0, Math.cos(facing)), 0.05);
    face.quaternion.copy(rotation);
    context.group.add(face);
  }

  // Wayfinding panels at room entrances, at 2.2m. Entrances are approximated
  // as wall adjacent cells with three or more open neighbours, which in
  // practice is a doorway or the mouth of a corridor.
  const entrances = wayCells.filter((cell) => {
    let open = 0;
    for (const [dx, dy] of [
      [1, 0],
      [-1, 0],
      [0, 1],
      [0, -1],
    ] as const) {
      if (grid.isOpen(cell.x + dx, cell.y + dy)) open += 1;
    }
    return open >= 3;
  });

  for (const cell of everyNth(entrances, 7)) {
    const facing = wallFacing(grid, cell);
    if (facing === null) continue;

    const position = grid.worldPosition(cell.x, cell.y).add(wallOffset(grid, cell, 0.14));
    position.y = 2.2;

    const rotation = new Quaternion().setFromAxisAngle(new Vector3(0, 1, 0), facing);
    addProp(context, 'wayfinding', new Matrix4().compose(position, rotation, new Vector3(1, 1, 1)));

    const texture = drawPanel(signStream.pick(WAYFINDING_WORDS), 0x1c2b46, signStream, 384, 140);
    const face = new Mesh(new PlaneGeometry(1.84, 0.66), panelMaterial(texture, 1.4));
    face.position
      .copy(position)
      .addScaledVector(new Vector3(Math.sin(facing), 0, Math.cos(facing)), 0.05);
    face.quaternion.copy(rotation);
    context.group.add(face);

    // A sign this bright reads as backlit, so it gets a light candidate.
    context.lightCandidates.push({
      position: position
        .clone()
        .addScaledVector(new Vector3(Math.sin(facing), 0, Math.cos(facing)), 0.5),
      colour: new Color(0x9fc0ff),
      intensity: 2.2,
      range: 5,
      blackoutSensitive: true,
    });
  }
}

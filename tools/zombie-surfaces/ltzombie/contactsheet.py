"""The human review sheet.

The author of this tool cannot see its output, so this file is the only thing
standing between a plausible looking build and a wrong one. It shows four
things per variant that no single mask view shows on its own:

  1. each channel on its own, so a dead or saturated channel is obvious
  2. a composite that approximates what `M_Zombie_Tintable` will do with the
     masks, so the sheet is predictive of the engine rather than decorative
  3. a two by two repeat, so a tiling seam is visible as a cross through the
     middle of the block rather than having to be measured
  4. for wounds, the channels read through a plausible wound ramp. Sending
     depth straight into the red channel reads as bubblegum and tells the
     reviewer nothing about whether the mask is right

Not a shipped asset, so it is excluded from the determinism assertion and may
use whatever font Pillow bundles.
"""

from __future__ import annotations

import numpy as np
from PIL import Image, ImageDraw, ImageFont

# Palette, from docs/art-direction.md.
CHARCOAL = (0x16, 0x16, 0x1C)
VIOLET = (0x6C, 0x4C, 0x9C)
SODIUM = (0xE0, 0xA0, 0x30)
CRIMSON = (0xB0, 0x20, 0x30)
OFF_WHITE = (0xEE, 0xEB, 0xEB)

# Approximate sRGB of the S9 DeadFleshColour, for the composite preview only.
DEAD_FLESH = np.array([116.0, 119.0, 106.0], dtype=np.float32)
SOOT = np.array([46.0, 44.0, 42.0], dtype=np.float32)

# Wound ramp: open tissue, surrounding flesh, dried crust.
WOUND_DEEP = np.array([58.0, 12.0, 18.0], dtype=np.float32)
WOUND_FLESH = np.array([138.0, 70.0, 66.0], dtype=np.float32)
WOUND_CRUST = np.array([74.0, 40.0, 30.0], dtype=np.float32)

THUMB = 220
PAD = 12
LABEL = 22


def _font(size: int):
	try:
		return ImageFont.load_default(size=size)
	except TypeError:
		return ImageFont.load_default()


def composite(blood: np.ndarray, grime: np.ndarray, lividity: np.ndarray) -> np.ndarray:
	"""Approximate the shipped material: mottle, soil, then bloody.

	Mirrors the operations `M_Zombie_Tintable` performs, in the same order, so
	that judging this image is close to judging the zombie. It is not a render:
	there is no lighting here, so read it for distribution and contrast rather
	than for final tone.
	"""
	base = np.tile(DEAD_FLESH, (blood.shape[0], blood.shape[1], 1))
	base = base * (np.float32(0.82) + lividity[..., None] * np.float32(0.36))
	base = base * (1.0 - grime[..., None] * 0.65) + SOOT * (grime[..., None] * 0.65)
	weight = np.clip(blood, 0.0, 1.0)[..., None]
	crimson = np.array(CRIMSON, dtype=np.float32)
	base = base * (1.0 - weight) + crimson * weight
	return np.clip(base, 0, 255).astype(np.uint8)


def wound_preview(rgba: np.ndarray) -> np.ndarray:
	"""Read a wound's four channels through the wound ramp, over charcoal."""
	depth = rgba[..., 0].astype(np.float32) / 255.0
	rim = rgba[..., 1].astype(np.float32) / 255.0
	wet = rgba[..., 2].astype(np.float32) / 255.0
	shape = rgba[..., 3].astype(np.float32) / 255.0

	colour = WOUND_FLESH * (1.0 - depth[..., None]) + WOUND_DEEP * depth[..., None]
	colour = colour * (1.0 - rim[..., None]) + WOUND_CRUST * rim[..., None]
	colour = colour + np.array(CRIMSON, dtype=np.float32) * (wet[..., None] * 0.30)

	ground = np.array(CHARCOAL, dtype=np.float32)
	blended = colour * shape[..., None] + ground * (1.0 - shape[..., None])
	return np.clip(blended, 0, 255).astype(np.uint8)


def _thumb(array: np.ndarray) -> Image.Image:
	return Image.fromarray(array, mode="L").resize((THUMB, THUMB), Image.LANCZOS)


def _tile_proof(field: np.ndarray) -> Image.Image:
	"""Two by two repeat. A seam shows as a cross through the centre."""
	half = THUMB // 2
	small = Image.fromarray(field, mode="L").resize((half, half), Image.LANCZOS)
	sheet = Image.new("L", (THUMB, THUMB))
	for x in (0, half):
		for y in (0, half):
			sheet.paste(small, (x, y))
	return sheet


def build(body_fields: list, wound_fields: list, size: int) -> Image.Image:
	"""`body_fields` is a list of (record, blood, grime, lividity) as uint8.

	`wound_fields` is a list of (record, rgba) as uint8.
	"""
	columns = ["composite", "R blood", "G grime", "B lividity", "2x2 tile proof"]
	width = PAD + len(columns) * (THUMB + PAD)
	rows = len(body_fields)
	height = (
		92
		+ rows * (THUMB + PAD + LABEL)
		+ ((LABEL + 8 + THUMB + PAD + LABEL) if wound_fields else 0)
	)

	sheet = Image.new("RGB", (width, height), CHARCOAL)
	draw = ImageDraw.Draw(sheet)
	title = _font(26)
	label = _font(15)
	small = _font(13)

	draw.rectangle([0, 0, width, 8], fill=VIOLET)
	draw.text((PAD, 22), "ZOMBIE SURFACE VARIATION", font=title, fill=SODIUM)
	draw.text(
		(PAD, 56),
		f"{size} px per mask. Composite approximates M_Zombie_Tintable, it is not a render.",
		font=small,
		fill=OFF_WHITE,
	)

	for index, name in enumerate(columns):
		draw.text((PAD + index * (THUMB + PAD), 76), name, font=label, fill=VIOLET)

	y = 92
	for record, blood, grime, lividity in body_fields:
		cells = [
			Image.fromarray(
				composite(
					blood.astype(np.float32) / 255.0,
					grime.astype(np.float32) / 255.0,
					lividity.astype(np.float32) / 255.0,
				),
				mode="RGB",
			).resize((THUMB, THUMB), Image.LANCZOS),
			_thumb(blood),
			_thumb(grime),
			_thumb(lividity),
			_tile_proof(blood),
		]
		for index, cell in enumerate(cells):
			sheet.paste(cell.convert("RGB"), (PAD + index * (THUMB + PAD), y))
		types = ", ".join(record["types"]) or "unassigned"
		draw.text(
			(PAD, y + THUMB + 4),
			f"{record['id']}   seed {record['seed']}   {types}   {record['note']}",
			font=label,
			fill=OFF_WHITE,
		)
		y += THUMB + PAD + LABEL

	if wound_fields:
		draw.text(
			(PAD, y),
			"WOUND DECALS, read through the wound ramp   R depth  G rim  B wetness  A shape",
			font=label,
			fill=VIOLET,
		)
		y += LABEL + 8
		for index, (record, rgba) in enumerate(wound_fields):
			cell = Image.fromarray(wound_preview(rgba), mode="RGB")
			sheet.paste(cell.resize((THUMB, THUMB), Image.LANCZOS), (PAD + index * (THUMB + PAD), y))
			draw.text(
				(PAD + index * (THUMB + PAD), y + THUMB + 4),
				f"{record['id']}  {record['note']}"[:36],
				font=label,
				fill=OFF_WHITE,
			)

	return sheet

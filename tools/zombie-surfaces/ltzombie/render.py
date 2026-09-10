"""Turning float fields into files.

Two output shapes, and the reason for each is a rendering cost:

	body   one RGB PNG carrying three separate masks, one per channel, so the
	       material samples once for blood, grime and lividity instead of three
	       times. At 24 to 40 zombies alive that is the difference between one
	       texture fetch per pixel and three.
	wound  one RGBA PNG per wound, because a deferred decal needs its own
	       opacity and must not wrap.

Quantisation is a plain round to 8 bit with no dithering. Dithering would add
noise that the mask already has plenty of, and it would make the byte level
determinism assertion depend on a dither pattern for no visual gain.
"""

from __future__ import annotations

import numpy as np
from PIL import Image


def to_bytes(field: np.ndarray) -> np.ndarray:
	"""Clamp to 0 to 1 and quantise to uint8."""
	return np.clip(np.rint(np.clip(field, 0.0, 1.0) * 255.0), 0, 255).astype(np.uint8)


def pack_body(blood: np.ndarray, grime: np.ndarray, lividity: np.ndarray) -> Image.Image:
	"""Pack the three tiling masks into the R, G and B channels of one image."""
	shapes = {blood.shape, grime.shape, lividity.shape}
	if len(shapes) != 1:
		raise ValueError(f"channels must share a shape, got {shapes}")
	stacked = np.dstack([to_bytes(blood), to_bytes(grime), to_bytes(lividity)])
	return Image.fromarray(stacked, mode="RGB")


def pack_wound(wound: np.ndarray) -> Image.Image:
	"""Pack a wound field of shape (size, size, 4) into an RGBA image."""
	if wound.ndim != 3 or wound.shape[2] != 4:
		raise ValueError(f"wound field must be (size, size, 4), got {wound.shape}")
	return Image.fromarray(to_bytes(wound), mode="RGBA")


def save_png(image: Image.Image, path) -> int:
	"""Write a PNG and return its size in bytes.

	`optimize=True` is deterministic in Pillow: the same pixels produce the same
	file, which the test suite checks by building twice rather than trusting it.
	"""
	path.parent.mkdir(parents=True, exist_ok=True)
	image.save(path, format="PNG", optimize=True)
	return path.stat().st_size

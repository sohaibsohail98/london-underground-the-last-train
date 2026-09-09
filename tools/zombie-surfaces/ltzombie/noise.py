"""Deterministic, exactly tileable noise primitives.

Every field here is periodic on the lattice, so a texture built from it wraps
without a seam, and every value derives from an integer hash rather than from
numpy's random generator. That second point is deliberate: numpy's PCG64 stream
is stable in practice but it is not a documented file format, and this pipeline
promises byte identical output across machines and across numpy releases. An
integer hash is arithmetic, so it cannot drift.

All generators take a period in lattice cells. The period must divide the
texture size for the wrap to be exact, which `assert_wraps` in the test suite
checks rather than assumes.
"""

from __future__ import annotations

import numpy as np

_UINT32 = np.uint32(0xFFFFFFFF)

# Odd 32 bit constants with good avalanche behaviour. Any odd multiplier works;
# these are the widely used xxhash and Wang mixing constants.
_PRIME_X = np.uint32(374761393)
_PRIME_Y = np.uint32(668265263)
_PRIME_S = np.uint32(2246822519)
_MIX_A = np.uint32(1274126177)
_MIX_B = np.uint32(2654435761)


def hash_lattice(ix: np.ndarray, iy: np.ndarray, seed: int) -> np.ndarray:
	"""Hash integer lattice coordinates to uint32.

	Pure arithmetic on uint32, so the result is identical on any platform and
	under any numpy version. Overflow is intended and numpy wraps silently for
	unsigned integers, so the warning suppression below is about noise in the
	output rather than about correctness.
	"""
	with np.errstate(over="ignore"):
		h = (
			ix.astype(np.uint32) * _PRIME_X
			+ iy.astype(np.uint32) * _PRIME_Y
			+ np.uint32(seed & 0xFFFFFFFF) * _PRIME_S
		)
		h ^= h >> np.uint32(13)
		h *= _MIX_A
		h ^= h >> np.uint32(16)
		h *= _MIX_B
		h ^= h >> np.uint32(15)
	return h


def hash_unit(ix: np.ndarray, iy: np.ndarray, seed: int) -> np.ndarray:
	"""Hash integer lattice coordinates to float32 in the half open range 0 to 1."""
	return (hash_lattice(ix, iy, seed) >> np.uint32(8)).astype(np.float32) / np.float32(
		1 << 24
	)


def _smoothstep(t: np.ndarray) -> np.ndarray:
	"""Quintic fade, clamped to 0 to 1.

	Continuous in the first and second derivative, so the interpolated field
	has no visible lattice creasing the way a cubic fade does.

	The clamp is not decoration. Evaluated in float32 the polynomial overshoots
	just below t = 1: `_smoothstep(0.99999994)` returns 1.0000007, which
	propagated into masks brighter than full white and was caught by the range
	assertion. It is mathematically bounded on 0 to 1, so clamping restores the
	property the arithmetic loses.
	"""
	shaped = t * t * t * (t * (t * np.float32(6.0) - np.float32(15.0)) + np.float32(10.0))
	return np.clip(shaped, 0.0, 1.0)


def divisors(size: int) -> list[int]:
	"""Every period that tiles exactly at this texture size.

	Exported so the config loader can name the legal values in an error rather
	than leaving the author of a data file to guess why a period was refused.
	"""
	return [n for n in range(1, size + 1) if size % n == 0]


def value_noise(size: int, period: int, seed: int) -> np.ndarray:
	"""Bilinear value noise on a wrapped lattice, float32 in 0 to 1.

	`period` is the number of lattice cells across the texture. Lattice indices
	are taken modulo `period`, so cell `period` is cell 0 and the field is
	exactly periodic.
	"""
	if period < 1:
		raise ValueError(f"period must be at least 1, got {period}")
	if size % period:
		raise ValueError(
			f"value noise period {period} does not divide size {size}, so it would "
			f"not tile; legal periods at this size are {divisors(size)}"
		)

	scale = period / size
	coords = (np.arange(size, dtype=np.float32) + np.float32(0.5)) * np.float32(scale)
	cell = np.floor(coords).astype(np.int64)
	frac = _smoothstep(coords - cell.astype(np.float32))

	x0 = (cell % period)[None, :]
	y0 = (cell % period)[:, None]
	x1 = ((cell + 1) % period)[None, :]
	y1 = ((cell + 1) % period)[:, None]

	fx = frac[None, :]
	fy = frac[:, None]

	c00 = hash_unit(x0, y0, seed)
	c10 = hash_unit(x1, y0, seed)
	c01 = hash_unit(x0, y1, seed)
	c11 = hash_unit(x1, y1, seed)

	top = c00 + (c10 - c00) * fx
	bottom = c01 + (c11 - c01) * fx
	return top + (bottom - top) * fy


def fbm(size: int, period: int, octaves: int, seed: int, gain: float = 0.5) -> np.ndarray:
	"""Fractional Brownian motion: octaves of value noise at doubling period.

	Each octave doubles the period, so every octave stays periodic on the
	texture and the sum wraps exactly. Octaves stop early rather than exceed
	the texture size, because a period larger than `size` would alias.
	"""
	total = np.zeros((size, size), dtype=np.float32)
	amplitude = np.float32(1.0)
	norm = np.float32(0.0)
	current = period
	for octave in range(octaves):
		if current > size:
			break
		total += value_noise(size, current, seed + octave * 7919) * amplitude
		norm += amplitude
		amplitude *= np.float32(gain)
		current *= 2
	if norm == 0:
		raise ValueError(f"period {period} exceeds size {size}, no octave fitted")
	return total / norm


def worley(size: int, period: int, seed: int) -> np.ndarray:
	"""Cellular noise: normalised distance to the nearest feature point.

	One jittered feature point per lattice cell, searched over the nine
	neighbouring cells with wrapped indices, so the field is periodic. Returned
	as 0 at a feature point rising to roughly 1 between them, which reads as
	crust and flaking rather than as cloud.
	"""
	if size % period:
		raise ValueError(
			f"worley period {period} does not divide size {size}, so it would not "
			f"tile; legal periods at this size are {divisors(size)}"
		)

	cell_size = size / period
	coords = (np.arange(size, dtype=np.float32) + np.float32(0.5)) / np.float32(cell_size)
	gx = coords[None, :]
	gy = coords[:, None]
	base_x = np.floor(gx).astype(np.int64)
	base_y = np.floor(gy).astype(np.int64)

	best = np.full((size, size), np.float32(1.0e9), dtype=np.float32)
	for oy in (-1, 0, 1):
		for ox in (-1, 0, 1):
			cx = base_x + ox
			cy = base_y + oy
			wx = cx % period
			wy = cy % period
			jitter_x = hash_unit(wx, wy, seed)
			jitter_y = hash_unit(wx, wy, seed ^ 0x5F37)
			px = cx.astype(np.float32) + jitter_x
			py = cy.astype(np.float32) + jitter_y
			dist = np.square(px - gx) + np.square(py - gy)
			np.minimum(best, dist, out=best)

	return np.clip(np.sqrt(best) * np.float32(1.4), 0.0, 1.0)


def contrast(field: np.ndarray, pivot: float, strength: float) -> np.ndarray:
	"""S curve about `pivot`. `strength` 1 is identity, higher is harder."""
	if strength <= 0:
		raise ValueError(f"strength must be positive, got {strength}")
	shifted = np.clip((field - np.float32(pivot)) * np.float32(strength) + np.float32(0.5), 0.0, 1.0)
	return _smoothstep(shifted)


def remap(field: np.ndarray, low: float, high: float) -> np.ndarray:
	"""Rescale so `low` maps to 0 and `high` maps to 1, clamped."""
	if high <= low:
		raise ValueError(f"high {high} must exceed low {low}")
	return np.clip((field - np.float32(low)) / np.float32(high - low), 0.0, 1.0)


def blur_wrapped(field: np.ndarray, radius: int) -> np.ndarray:
	"""Separable box blur with wrapped edges, so blurring preserves tileability.

	Two passes approximate a Gaussian well enough for a mask and cost far less
	than a real kernel at these radii.
	"""
	if radius <= 0:
		return field.copy()
	out = field.astype(np.float32)
	width = 2 * radius + 1
	for _ in range(2):
		padded = np.concatenate([out[:, -radius:], out, out[:, :radius]], axis=1)
		cumulative = np.cumsum(padded, axis=1, dtype=np.float64)
		cumulative = np.concatenate(
			[np.zeros((out.shape[0], 1), dtype=np.float64), cumulative], axis=1
		)
		out = ((cumulative[:, width:] - cumulative[:, :-width]) / width).astype(np.float32)
		out = out.T.copy()
	return out


def normalise(field: np.ndarray) -> np.ndarray:
	"""Stretch to the full 0 to 1 range. Flat input is returned as zeros."""
	low = float(field.min())
	high = float(field.max())
	if high - low < 1.0e-6:
		return np.zeros_like(field)
	# Clipped defensively: the span is rounded to float32 separately from the
	# subtraction, so the ratio at the maximum is not guaranteed to land on
	# exactly 1.0.
	return np.clip((field - np.float32(low)) / np.float32(high - low), 0.0, 1.0)

"""The four mask families.

Three of them are tiling body detail, packed into one RGB texture so the
material costs a single sampler for all three:

	R  blood      fresh and dried fluid, gravity driven
	G  grime      soot, dust and tunnel fluff accumulation
	B  lividity   mottled dead flesh discolouration, low frequency

The fourth, wound, is a sparse non tiling RGBA decal:

	R  depth      how deep the wound reads
	G  rim        dried and crusted edge
	B  wetness    where it still runs
	A  shape      the decal's own opacity

Every family is built only from the primitives in `noise`, so every family
inherits exact tileability where it claims it and byte determinism everywhere.
Blood is the only family with a direction in it, because blood is the only one
gravity acts on.
"""

from __future__ import annotations

import numpy as np

from . import noise


def _hashed_points(count: int, seed: int, margin: float = 0.0) -> tuple:
	"""`count` hashed positions in 0 to 1, with a weight each.

	Positions come from the same integer hash as the noise, so a point set is
	reproducible without touching a random generator.
	"""
	if count < 1:
		raise ValueError(f"count must be at least 1, got {count}")
	index = np.arange(count, dtype=np.int64)
	zeros = np.zeros_like(index)
	span = np.float32(1.0 - 2.0 * margin)
	xs = noise.hash_unit(index, zeros, seed) * span + np.float32(margin)
	ys = noise.hash_unit(index, zeros + 1, seed ^ 0x2B1D) * span + np.float32(margin)
	weight = np.float32(0.45) + noise.hash_unit(index, zeros + 2, seed) * np.float32(0.55)
	return xs, ys, weight


def _clustered_points(
	count: int, clusters: int, spread: float, seed: int
) -> tuple:
	"""`count` positions gathered into `clusters` groups of radius `spread`.

	Uniformly hashed positions look scattered rather than spilt: real fluid
	comes from a few places and pools around them. Clustering is what stops the
	blood mask reading as an evenly spaced printed pattern, which is what the
	first version of it did.
	"""
	if clusters < 1:
		raise ValueError(f"clusters must be at least 1, got {clusters}")
	index = np.arange(count, dtype=np.int64)
	zeros = np.zeros_like(index)
	group = index % clusters

	cx = noise.hash_unit(group, zeros, seed ^ 0x4D2B)
	cy = noise.hash_unit(group, zeros + 1, seed ^ 0x7E51)
	# Polar offset inside the cluster, square rooted so points fill the disc
	# evenly instead of bunching at the centre.
	angle = noise.hash_unit(index, zeros + 2, seed) * np.float32(2.0 * np.pi)
	radius = np.sqrt(noise.hash_unit(index, zeros + 3, seed)) * np.float32(spread)
	xs = (cx + np.cos(angle) * radius) % np.float32(1.0)
	ys = (cy + np.sin(angle) * radius) % np.float32(1.0)
	weight = np.float32(0.40) + noise.hash_unit(index, zeros + 4, seed) * np.float32(0.60)
	return xs, ys, weight


def _stamp_at(
	size: int,
	xs: np.ndarray,
	ys: np.ndarray,
	weights: np.ndarray,
	radii: np.ndarray,
	softness: float,
) -> np.ndarray:
	"""Stamp soft discs at explicit positions, wrapped at the edges."""
	field = np.zeros((size, size), dtype=np.float32)
	for i in range(len(xs)):
		radius = float(radii[i])
		extent = max(1, int(np.ceil(radius)))
		local = np.arange(-extent, extent + 1, dtype=np.float32)
		dist = np.sqrt(local[None, :] ** 2 + local[:, None] ** 2) / max(radius, 1.0e-6)
		disc = np.clip((np.float32(1.0) - dist) / max(softness, 1.0e-6), 0.0, 1.0)
		disc = disc * disc * (np.float32(3.0) - np.float32(2.0) * disc) * np.float32(weights[i])
		rows = (np.arange(-extent, extent + 1, dtype=np.int64) + int(float(ys[i]) * size)) % size
		cols = (np.arange(-extent, extent + 1, dtype=np.int64) + int(float(xs[i]) * size)) % size
		patch = field[np.ix_(rows, cols)]
		field[np.ix_(rows, cols)] = np.maximum(patch, disc)
	return field


def _stamp_discs(
	size: int,
	seed: int,
	count: int,
	radius_min: float,
	radius_max: float,
	softness: float = 0.5,
) -> np.ndarray:
	"""Stamp `count` soft discs at hashed positions, wrapped at the edges.

	Discs rather than single pixels, because a one pixel impulse carries almost
	no mass: blurring fourteen of them across a 2048 texture and rescaling
	leaves a field that is black nearly everywhere, which is exactly the defect
	the first version of this function had. A disc has an area, so coverage
	becomes a property the parameters control and the test suite can assert.

	Radii are a fraction of the texture size, so a variant looks the same at
	1024 as at 2048 and the resolution is a build option rather than a look.
	"""
	xs, ys, weights = _hashed_points(count, seed)
	index = np.arange(count, dtype=np.int64)
	spread = noise.hash_unit(index, index + 3, seed ^ 0x11C3)
	radii = (np.float32(radius_min) + spread * np.float32(radius_max - radius_min)) * size

	field = np.zeros((size, size), dtype=np.float32)
	for i in range(count):
		radius = float(radii[i])
		extent = max(1, int(np.ceil(radius)))
		local = np.arange(-extent, extent + 1, dtype=np.float32)
		dist = np.sqrt(local[None, :] ** 2 + local[:, None] ** 2) / max(radius, 1.0e-6)
		disc = np.clip((np.float32(1.0) - dist) / max(softness, 1.0e-6), 0.0, 1.0)
		disc = disc * disc * (np.float32(3.0) - np.float32(2.0) * disc) * np.float32(weights[i])

		cy = int(float(ys[i]) * size)
		cx = int(float(xs[i]) * size)
		rows = (np.arange(-extent, extent + 1, dtype=np.int64) + cy) % size
		cols = (np.arange(-extent, extent + 1, dtype=np.int64) + cx) % size
		patch = field[np.ix_(rows, cols)]
		field[np.ix_(rows, cols)] = np.maximum(patch, disc)
	return field


def _gravity_smear(field: np.ndarray, length: int, decay: float) -> np.ndarray:
	"""Smear downward with exponential falloff, wrapped, in log passes.

	The naive form is one shifted maximum per pixel of run length. Because the
	falloff is exponential it composes, so doubling the shift each pass reaches
	the same result in log2(length) passes instead of `length` of them. At a
	2048 texture with a 400 pixel run that is 9 passes rather than 400.
	"""
	if length < 1:
		return field.copy()
	out = field.astype(np.float32)
	step = 1
	while step < length:
		out = np.maximum(out, np.roll(out, step, axis=0) * np.float32(decay**step))
		step *= 2
	return out


def _warp_rows(field: np.ndarray, offsets: np.ndarray) -> np.ndarray:
	"""Roll each row by its own integer offset, wrapped.

	This is what bends the drips. A straight smear gives perfectly vertical
	runs, which reads as a printed stripe rather than as fluid; displacing each
	row by a low frequency offset makes each run wander independently while
	keeping the wrap exact, because a roll is periodic by construction.
	"""
	size = field.shape[1]
	columns = (np.arange(size, dtype=np.int64)[None, :] - offsets[:, None]) % size
	rows = np.arange(field.shape[0], dtype=np.int64)[:, None]
	return field[rows, columns]


def blood(size: int, seed: int, params: dict) -> np.ndarray:
	"""Fresh and dried fluid.

	Five components, and each exists because the mask looked wrong without it:

	  pools     wide soft masses at the sources
	  rivulets  thin runs falling from the sources. Narrower than the pools,
	            because a drip is a rivulet leaving a wet patch, not the whole
	            patch sliding downward. Smearing the pool itself is what made
	            the first version read as evenly spaced tadpoles
	  sheets    irregular low frequency masses near the sources, so heavy
	            variants have continuous wet area rather than only streaks
	  spatter   many small marks, denser near the sources
	  rim       a dried crust on the boundary of all of it

	Run length varies per source rather than globally: the sources are split
	into three groups and each group is smeared a different distance, so some
	runs barely leave the pool and others cross the texture.
	"""
	count = int(params["origins"])
	xs, ys, weights = _clustered_points(
		count, int(params["clusters"]), float(params["cluster_spread"]), seed
	)
	index = np.arange(count, dtype=np.int64)
	spread = noise.hash_unit(index, index + 9, seed ^ 0x11C3)

	pool_radii = (
		np.float32(params["pool_radius_min"])
		+ spread * np.float32(params["pool_radius_max"] - params["pool_radius_min"])
	) * size
	pools = _stamp_at(size, xs, ys, weights, pool_radii, softness=0.85)
	# Ragged the pool edges. A perfect disc reads as a printed dot, and the
	# pools are the largest single feature in the mask, so they are the most
	# damaging thing to leave circular.
	pool_edge = noise.fbm(size, int(params["pool_edge_period"]), 4, seed ^ 0x3B71)
	pools = pools * (
		np.float32(1.0 - params["pool_ragged"])
		+ noise.remap(pool_edge, 0.22, 0.86) * np.float32(params["pool_ragged"])
	)

	# Rivulets: a much smaller source, smeared far. Three length groups.
	thin_radii = np.maximum(pool_radii * np.float32(params["rivulet_width"]), 1.2)
	runs = np.zeros((size, size), dtype=np.float32)
	group = index % 3
	for g, scale in enumerate((0.28, 0.62, 1.0)):
		mask = group == g
		if not mask.any():
			continue
		source = _stamp_at(
			size, xs[mask], ys[mask], weights[mask], thin_radii[mask], softness=1.0
		)
		length = max(1, int(size * float(params["run_length"]) * scale))
		runs = np.maximum(runs, _gravity_smear(source, length, float(params["run_decay"])))

	wander = noise.fbm(size, int(params["wander_period"]), 3, seed ^ 0x77A1)
	amplitude = float(params["run_wander"]) * size
	offsets = ((wander[:, 0] - 0.5) * 2.0 * amplitude).astype(np.int64)
	runs = _warp_rows(runs, offsets)
	runs = noise.blur_wrapped(runs, max(1, int(size * 0.002)))

	breakup = noise.remap(
		noise.fbm(size, int(params["breakup_period"]), 4, seed ^ 0x1357), 0.14, 0.90
	)
	runs = runs * (
		np.float32(1.0 - params["breakup"]) + breakup * np.float32(params["breakup"])
	)

	# Sheets: irregular continuous wet area, gated to near the sources so it
	# does not become an all over wash.
	sheet_shape = noise.remap(
		noise.fbm(size, int(params["sheet_period"]), 4, seed ^ 0x2F8C),
		float(params["sheet_threshold"]),
		0.92,
	)
	# Normalised, not remapped to a fixed band. The blurred value of a sparse
	# disc field is a couple of per cent at most, so remapping it into 0 to
	# 0.22 collapsed the gate to zero and the sheet component never appeared at
	# all. This is the same mistake as the original empty-mask defect, in a
	# second place, and it is why the gate is normalised here.
	near = noise.normalise(
		noise.blur_wrapped(pools, max(1, int(size * float(params["sheet_reach"]))))
	)
	near = np.clip(near * np.float32(params["sheet_gain"]), 0.0, 1.0)
	sheets = sheet_shape * near * np.float32(params["sheet_weight"])

	spatter_radii = (
		np.float32(params["spatter_radius_min"])
		+ noise.hash_unit(
			np.arange(int(params["spatter"]), dtype=np.int64),
			np.arange(int(params["spatter"]), dtype=np.int64) + 5,
			seed ^ 0x51A7,
		)
		* np.float32(params["spatter_radius_max"] - params["spatter_radius_min"])
	) * size
	sxs, sys, sweights = _clustered_points(
		int(params["spatter"]),
		max(1, int(params["clusters"])),
		float(params["cluster_spread"]) * 1.9,
		seed ^ 0x51A7,
	)
	spatter = _stamp_at(size, sxs, sys, sweights, np.maximum(spatter_radii, 1.0), softness=1.0)
	spatter = spatter * (np.float32(0.30) + near * np.float32(0.70))

	field = np.clip(
		pools * np.float32(params["pool_weight"])
		+ runs * np.float32(params["run_weight"])
		+ sheets
		+ spatter * np.float32(params["spatter_weight"]),
		0.0,
		1.0,
	)

	rim = np.clip(field - noise.blur_wrapped(field, max(1, int(size * 0.005))), 0.0, 1.0)
	field = np.clip(field + rim * np.float32(params["rim_weight"]), 0.0, 1.0)

	return noise.contrast(field, float(params["pivot"]), float(params["sharpness"]))


def grime(size: int, seed: int, params: dict) -> np.ndarray:
	"""Soot, dust and tunnel fluff.

	Cloudy, because that is how dirt accumulates: heavy in some areas, thin in
	others, with no discrete shapes. The crust component is the *boundary
	network* of the cellular noise, which reads as fine cracking, rather than
	the filled cells, which read as bubble wrap. That distinction is the whole
	difference between this looking like grime and looking like foam, and the
	first version got it wrong.
	"""
	broad = noise.fbm(size, int(params["period"]), int(params["octaves"]), seed, float(params["gain"]))
	detail = noise.fbm(size, int(params["detail_period"]), 3, seed ^ 0x6C1B, 0.6)
	field = broad * np.float32(1.0 - params["detail_weight"]) + detail * np.float32(params["detail_weight"])

	# Cell boundaries: far from every feature point is the skeleton between
	# cells, so remapping the top of the distance field isolates thin ridges.
	# A narrow band at the top of the distance field is the thin skeleton
	# between cells. A wide band selects the cell interiors instead, which is
	# what made this read as bubble wrap.
	cracks = noise.remap(
		noise.worley(size, int(params["crust_period"]), seed ^ 0x3C19),
		float(params["crust_band"]),
		1.0,
	)
	field = np.clip(field + cracks * np.float32(params["crust_weight"]), 0.0, 1.0)

	field = noise.normalise(field)
	return noise.contrast(field, float(params["pivot"]), float(params["sharpness"]))


def lividity(size: int, seed: int, params: dict) -> np.ndarray:
	"""Mottled dead flesh discolouration.

	Deliberately the softest and lowest contrast of the four. This is the one
	that must not read as a pattern: it is meant to make the skin uneven, not
	to draw a shape on it, so it is low frequency, low contrast and blurred.
	"""
	field = noise.fbm(size, int(params["period"]), int(params["octaves"]), seed, float(params["gain"]))
	field = noise.blur_wrapped(field, max(1, int(size * float(params["softness"]))))
	field = noise.normalise(field)
	return noise.contrast(field, float(params["pivot"]), float(params["sharpness"]))


def wound(size: int, seed: int, params: dict) -> np.ndarray:
	"""A single wound as an RGBA decal, shape `(size, size, 4)`.

	Not tiling: it is stamped once through a deferred decal, so it falls to zero
	alpha at its own border rather than wrapping.

	Three things stop it reading as a rounded blob, which is what the first
	version was: the outline is modulated by higher frequency noise so it tears
	rather than lobes, the disc is stretched and rotated per seed so no two
	share an axis, and the interior is deepest off centre.
	"""
	axis = (np.arange(size, dtype=np.float32) + np.float32(0.5)) / np.float32(size) * np.float32(2.0) - np.float32(1.0)
	gx = np.broadcast_to(axis[None, :], (size, size))
	gy = np.broadcast_to(axis[:, None], (size, size))

	# Anisotropy: stretch on one axis and rotate, both hashed from the seed.
	one = np.array([0], dtype=np.int64)
	stretch = np.float32(1.0) + noise.hash_unit(one, one, seed ^ 0x1A7B)[0] * np.float32(
		params["anisotropy"]
	)
	angle = noise.hash_unit(one, one + 1, seed ^ 0x51D9)[0] * np.float32(np.pi)
	cos_a = np.float32(np.cos(float(angle)))
	sin_a = np.float32(np.sin(float(angle)))
	rx = (gx * cos_a - gy * sin_a) * stretch
	ry = (gx * sin_a + gy * cos_a) / stretch
	radius = np.sqrt(rx * rx + ry * ry)

	tear = noise.fbm(size, int(params["tear_period"]), int(params["tear_octaves"]), seed)
	edge = np.float32(params["extent"]) * (
		np.float32(1.0) + (tear - np.float32(0.5)) * np.float32(2.0 * params["tear_amount"])
	)
	shape = noise.remap(edge - radius, 0.0, float(params["falloff"]))
	shape = np.clip(shape, 0.0, 1.0)

	# Depth: deepest off centre, and hard, so the middle reads as open rather
	# than as a soft gradient.
	centre_bias = noise.fbm(size, max(2, int(params["tear_period"]) // 2), 3, seed ^ 0x6A2F)
	depth = np.clip(shape * (np.float32(0.35) + centre_bias * np.float32(1.05)), 0.0, 1.0)
	depth = noise.contrast(depth, float(params["depth_pivot"]), float(params["depth_sharpness"]))
	depth = depth * shape

	# Rim: the crusted boundary. Widened deliberately, because the dried edge is
	# most of what makes a wound read as a wound rather than as a stain.
	inner = noise.blur_wrapped(shape, max(2, int(size * float(params["rim_width"]))))
	rim = np.clip(shape - inner, 0.0, 1.0)
	rim = noise.normalise(rim) * shape * np.float32(params["rim_weight"])

	wet_field = _gravity_smear(
		np.clip(depth - np.float32(0.5), 0.0, 1.0), max(1, int(size * float(params["wet_run"]))), 0.995
	)
	wetness = np.clip(noise.normalise(wet_field) * shape, 0.0, 1.0)

	out = np.zeros((size, size, 4), dtype=np.float32)
	out[..., 0] = depth
	out[..., 1] = np.clip(rim, 0.0, 1.0)
	out[..., 2] = wetness
	out[..., 3] = shape
	return out


TILING_FAMILIES = {"blood": blood, "grime": grime, "lividity": lividity}

"""Shared measurement helpers for the assertion suite.

The seam test is the one worth reading. Three versions of it were tried:

  1. `max seam step <= max interior step`. Correct but weak: a near binary mask
     has a step of 1.0 somewhere in the interior anyway, so almost nothing
     fails it.
  2. `mean seam step <= k * mean interior step`. Rejected. On a sparse mask the
     interior mean is diluted by the large empty areas, so the denominator
     collapses and a perfectly tiling blood mask scores 3.1 while a genuinely
     non tiling worley crop scores 2.9. It cannot tell them apart.
  3. `mean seam step <= k * mean of the two immediately adjacent steps`. Used.
     The comparison is against the same region of the same image, so content
     density cancels. Measured: every real field scores at most 2.0, while
     crops taken out of a larger field, which genuinely do not tile, score
     3.0, 68.7 and 95.6.

Both 1 and 3 are asserted, because they fail in different ways.
"""

from __future__ import annotations

import numpy as np

# Above the worst real field (2.0) and below the mildest genuine break (3.0).
SEAM_TOLERANCE = 3.0


def max_step_ratio(field: np.ndarray) -> float:
	"""Largest wrap step over largest interior step, per axis, worst of the two.

	At or below 1.0 means the wrap introduces no discontinuity bigger than one
	the texture already contains.
	"""
	worst = 0.0
	for axis in (0, 1):
		rolled = np.abs(np.take(field, -1, axis=axis) - np.take(field, 0, axis=axis)).max()
		interior = np.abs(np.diff(field, axis=axis)).max()
		worst = max(worst, float(rolled / interior) if interior > 1e-9 else 0.0)
	return worst


def adjacent_seam_ratio(field: np.ndarray) -> float:
	"""Mean wrap step over the mean of the two steps either side of the wrap."""
	worst = 0.0
	for axis in (0, 1):
		seam = np.abs(np.take(field, -1, axis=axis) - np.take(field, 0, axis=axis)).mean()
		before = np.abs(np.take(field, -2, axis=axis) - np.take(field, -1, axis=axis)).mean()
		after = np.abs(np.take(field, 0, axis=axis) - np.take(field, 1, axis=axis)).mean()
		local = 0.5 * (before + after)
		if local > 1e-9:
			worst = max(worst, float(seam / local))
		elif seam > 1e-9:
			worst = max(worst, 99.0)
	return worst


def mean_absolute_difference(a: np.ndarray, b: np.ndarray) -> float:
	return float(np.abs(a.astype(np.float64) - b.astype(np.float64)).mean())

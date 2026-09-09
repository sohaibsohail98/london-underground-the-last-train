"""Assertions on the mask fields themselves.

Bands are the measured range of the committed data with headroom, not round
numbers picked to pass. They are measured at `SIZE`, so changing `SIZE` without
re-measuring will make them meaningless.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

import numpy as np

TOOL_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_DIR))
sys.path.insert(0, str(TOOL_DIR / "tests"))

from assertions import (  # noqa: E402
	SEAM_TOLERANCE,
	adjacent_seam_ratio,
	max_step_ratio,
	mean_absolute_difference,
)
from ltzombie import config, families  # noqa: E402

SIZE = 512

# Measured at SIZE across the committed variants, with headroom either side.
MEAN_BANDS = {
	"blood": (0.005, 0.35),
	"grime": (0.08, 0.75),
	"lividity": (0.20, 0.85),
}
MIN_STANDARD_DEVIATION = 0.02
MIN_DISTINCTNESS = 0.02
WOUND_ALPHA_BAND = (0.05, 0.55)


class MaskFieldTests(unittest.TestCase):
	@classmethod
	def setUpClass(cls):
		cls.resolved = config.load(TOOL_DIR / "data", SIZE)
		cls.tiling = {}
		for record in cls.resolved["body"]:
			for family, builder in families.TILING_FAMILIES.items():
				cls.tiling[(record["id"], family)] = builder(
					SIZE, record["seed"], record["params"][family]
				)
		cls.wounds = {
			record["id"]: families.wound(SIZE, record["seed"], record["params"]["wound"])
			for record in cls.resolved["wound"]
		}

	def test_every_variant_built(self):
		self.assertEqual(len(self.resolved["body"]), 8)
		self.assertEqual(len(self.resolved["wound"]), 4)
		self.assertEqual(len(self.tiling), 8 * 3)

	def test_fields_are_in_range_and_not_degenerate(self):
		for key, field in self.tiling.items():
			with self.subTest(key=key):
				self.assertGreaterEqual(float(field.min()), 0.0)
				self.assertLessEqual(float(field.max()), 1.0)
				self.assertGreater(
					float(field.std()),
					MIN_STANDARD_DEVIATION,
					"field is nearly flat, so it carries no information",
				)

	def test_coverage_is_inside_the_measured_band(self):
		for (variant, family), field in self.tiling.items():
			low, high = MEAN_BANDS[family]
			with self.subTest(variant=variant, family=family):
				self.assertGreaterEqual(float(field.mean()), low)
				self.assertLessEqual(float(field.mean()), high)

	def test_wrap_introduces_no_new_discontinuity(self):
		for key, field in self.tiling.items():
			with self.subTest(key=key):
				self.assertLessEqual(max_step_ratio(field), 1.0 + 1e-6)

	def test_wrap_is_statistically_seamless(self):
		for key, field in self.tiling.items():
			with self.subTest(key=key):
				self.assertLess(adjacent_seam_ratio(field), SEAM_TOLERANCE)

	def test_a_non_tiling_field_is_actually_caught(self):
		"""The seam assertions have teeth.

		Without this, a bug that made every field trivially pass the seam tests
		would look like success. A crop out of a larger field genuinely does not
		tile, so it must fail.
		"""
		from ltzombie import noise

		big = noise.fbm(SIZE * 2, 4, 6, 7)
		crop = big[200 : 200 + SIZE, 300 : 300 + SIZE].copy()
		self.assertGreater(adjacent_seam_ratio(crop), SEAM_TOLERANCE)

	def test_variants_are_distinct(self):
		for family in families.TILING_FAMILIES:
			fields = [f for (_, fam), f in self.tiling.items() if fam == family]
			for i in range(len(fields)):
				for j in range(i + 1, len(fields)):
					with self.subTest(family=family, pair=(i, j)):
						self.assertGreater(
							mean_absolute_difference(fields[i], fields[j]),
							MIN_DISTINCTNESS,
							"two variants are near identical, so one is wasted",
						)

	def test_seed_changes_the_result(self):
		record = self.resolved["body"][0]
		params = record["params"]["blood"]
		a = families.blood(SIZE, record["seed"], params)
		b = families.blood(SIZE, record["seed"] + 1, params)
		self.assertGreater(mean_absolute_difference(a, b), MIN_DISTINCTNESS)

	def test_fields_are_deterministic(self):
		for (variant, family), field in self.tiling.items():
			record = next(r for r in self.resolved["body"] if r["id"] == variant)
			again = families.TILING_FAMILIES[family](
				SIZE, record["seed"], record["params"][family]
			)
			with self.subTest(variant=variant, family=family):
				self.assertTrue(np.array_equal(field, again))

	def test_wounds_have_four_channels_and_a_clear_border(self):
		for identifier, field in self.wounds.items():
			with self.subTest(wound=identifier):
				self.assertEqual(field.shape, (SIZE, SIZE, 4))
				alpha = field[..., 3]
				border = max(
					float(alpha[0, :].max()),
					float(alpha[-1, :].max()),
					float(alpha[:, 0].max()),
					float(alpha[:, -1].max()),
				)
				self.assertLess(
					border,
					1.0 / 255.0,
					"a decal that touches its own border will show a cut edge in engine",
				)

	def test_wound_alpha_coverage(self):
		for identifier, field in self.wounds.items():
			low, high = WOUND_ALPHA_BAND
			with self.subTest(wound=identifier):
				coverage = float(field[..., 3].mean())
				self.assertGreaterEqual(coverage, low)
				self.assertLessEqual(coverage, high)

	def test_wound_channels_are_masked_by_shape(self):
		"""Nothing may be non zero where the decal is fully transparent."""
		for identifier, field in self.wounds.items():
			outside = field[..., 3] <= 0.0
			with self.subTest(wound=identifier):
				for channel in range(3):
					self.assertLessEqual(
						float(field[..., channel][outside].max(initial=0.0)),
						1.0 / 255.0,
					)


if __name__ == "__main__":
	unittest.main()

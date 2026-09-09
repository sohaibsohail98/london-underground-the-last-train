"""Assertions on the built output: determinism, the manifest contract, the CLI.

The determinism test builds the whole set twice into two directories and
compares every byte, including the manifest. That is the only form of the test
worth having: asserting that a hash function is stable proves nothing about
whether the PNG encoder, the parameter resolution or the manifest serialisation
are.
"""

from __future__ import annotations

import contextlib
import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

TOOL_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_DIR))

import generate  # noqa: E402
from ltzombie import config, manifest  # noqa: E402

# Small, because this suite builds the full set several times and the assertions
# here are about bytes and structure rather than about how a mask looks.
SIZE = 256


def build(out: Path, extra=None) -> int:
	"""Build quietly. The CLI reports progress on stdout, which is useful when a
	person runs it and pure noise interleaved with test results."""
	argv = ["--out", str(out), "--size", str(SIZE), "--no-contact-sheet"]
	if extra:
		argv += extra
	with contextlib.redirect_stdout(io.StringIO()):
		return generate.main(argv)


class DeterminismTests(unittest.TestCase):
	def test_two_builds_are_byte_identical(self):
		with tempfile.TemporaryDirectory() as tmp:
			first = Path(tmp) / "a"
			second = Path(tmp) / "b"
			self.assertEqual(build(first), 0)
			self.assertEqual(build(second), 0)

			names = sorted(p.name for p in first.iterdir())
			self.assertEqual(names, sorted(p.name for p in second.iterdir()))
			self.assertIn("manifest.json", names)

			for name in names:
				with self.subTest(file=name):
					self.assertEqual(
						(first / name).read_bytes(),
						(second / name).read_bytes(),
						f"{name} is not reproducible",
					)

	def test_seed_offset_changes_the_output(self):
		with tempfile.TemporaryDirectory() as tmp:
			plain = Path(tmp) / "plain"
			shifted = Path(tmp) / "shifted"
			self.assertEqual(build(plain, ["--only", "worn_01"]), 0)
			self.assertEqual(build(shifted, ["--only", "worn_01", "--seed", "97"]), 0)
			name = "T_ZombieSurface_worn_01.png"
			self.assertNotEqual(
				(plain / name).read_bytes(),
				(shifted / name).read_bytes(),
				"--seed had no effect, so the whole set cannot be reshuffled",
			)

	def test_manifest_carries_no_timestamp_by_default(self):
		with tempfile.TemporaryDirectory() as tmp:
			out = Path(tmp) / "o"
			self.assertEqual(build(out, ["--only", "worn_01"]), 0)
			document = json.loads((out / "manifest.json").read_text())
			self.assertNotIn("provenance", document)


class ManifestTests(unittest.TestCase):
	@classmethod
	def setUpClass(cls):
		cls.tmp = tempfile.TemporaryDirectory()
		cls.out = Path(cls.tmp.name) / "out"
		assert build(cls.out) == 0
		cls.document = json.loads((cls.out / "manifest.json").read_text())

	@classmethod
	def tearDownClass(cls):
		cls.tmp.cleanup()

	def test_every_entry_points_at_a_real_file_with_a_matching_digest(self):
		for entry in self.document["entries"]:
			with self.subTest(entry=entry["id"]):
				path = self.out / entry["path"]
				self.assertTrue(path.is_file(), f"{entry['path']} is missing")
				self.assertEqual(entry["sha256"], manifest.sha256_file(path))
				self.assertEqual(entry["bytes"], path.stat().st_size)

	def test_every_output_file_appears_in_the_manifest(self):
		listed = {entry["path"] for entry in self.document["entries"]}
		on_disk = {
			p.name for p in self.out.iterdir()
			if p.suffix == ".png" and p.name != "contact-sheet.png"
		}
		self.assertEqual(on_disk, listed)

	def test_entry_count_matches_the_data(self):
		resolved = config.load(TOOL_DIR / "data", SIZE)
		self.assertEqual(
			len(self.document["entries"]),
			len(resolved["body"]) + len(resolved["wound"]),
		)

	def test_unreal_block_is_correct_for_masks(self):
		for kind in ("body", "wound"):
			block = self.document["unreal"][kind]
			with self.subTest(kind=kind):
				self.assertFalse(
					block["srgb"],
					"a mask read as sRGB is the classic quiet bug: every value is wrong",
				)
				self.assertEqual(block["compression_settings"], "TC_Masks")
				self.assertEqual(block["sampler_type"], "SAMPLERTYPE_MASKS")
				self.assertEqual(block["texture_group"], "TEXTUREGROUP_Character")
				self.assertEqual(len(block["channels"]), 3 if kind == "body" else 4)

	def test_body_masks_wrap_and_wound_decals_clamp(self):
		body = self.document["unreal"]["body"]
		wound = self.document["unreal"]["wound"]
		self.assertEqual((body["address_x"], body["address_y"]), ("TA_Wrap", "TA_Wrap"))
		self.assertEqual((wound["address_x"], wound["address_y"]), ("TA_Clamp", "TA_Clamp"))

	def test_entries_record_the_parameters_that_produced_them(self):
		for entry in self.document["entries"]:
			with self.subTest(entry=entry["id"]):
				self.assertIn("params", entry)
				self.assertIn("seed", entry)
				expected = ("wound",) if entry["kind"] == "wound" else ("blood", "grime", "lividity")
				self.assertEqual(tuple(sorted(entry["params"])), tuple(sorted(expected)))

	def test_asset_names_are_prefixed_for_unreal(self):
		for entry in self.document["entries"]:
			prefix = manifest.BODY_UNREAL if entry["kind"] == "body" else manifest.WOUND_UNREAL
			with self.subTest(entry=entry["id"]):
				self.assertTrue(entry["asset_name"].startswith(prefix["asset_prefix"]))
				self.assertTrue(entry["asset_name"].startswith("T_"))


class PackingTests(unittest.TestCase):
	def test_body_png_is_rgb_and_wound_png_is_rgba(self):
		from PIL import Image

		with tempfile.TemporaryDirectory() as tmp:
			out = Path(tmp) / "o"
			self.assertEqual(build(out, ["--only", "worn_01", "--only", "tear_01"]), 0)
			body = Image.open(out / "T_ZombieSurface_worn_01.png")
			wound = Image.open(out / "T_ZombieWound_tear_01.png")
			self.assertEqual(body.mode, "RGB")
			self.assertEqual(body.size, (SIZE, SIZE))
			self.assertEqual(wound.mode, "RGBA")
			self.assertEqual(wound.size, (SIZE, SIZE))

	def test_body_channels_are_not_duplicates_of_each_other(self):
		"""A packing bug that wrote the same field into all three channels would
		otherwise pass every other assertion in this suite."""
		import numpy as np
		from PIL import Image

		with tempfile.TemporaryDirectory() as tmp:
			out = Path(tmp) / "o"
			self.assertEqual(build(out, ["--only", "drenched_01"]), 0)
			pixels = np.asarray(Image.open(out / "T_ZombieSurface_drenched_01.png"))
			for a, b in ((0, 1), (0, 2), (1, 2)):
				with self.subTest(pair=(a, b)):
					self.assertFalse(np.array_equal(pixels[..., a], pixels[..., b]))


class CommandLineTests(unittest.TestCase):
	def test_dry_run_writes_nothing(self):
		with tempfile.TemporaryDirectory() as tmp:
			out = Path(tmp) / "nothing-here"
			with contextlib.redirect_stdout(io.StringIO()):
				self.assertEqual(generate.main(["--out", str(out), "--dry-run"]), 0)
			self.assertFalse(out.exists())

	def test_unknown_variant_is_rejected(self):
		with tempfile.TemporaryDirectory() as tmp:
			with contextlib.redirect_stdout(io.StringIO()):
				self.assertEqual(
					generate.main(["--out", str(Path(tmp) / "o"), "--only", "not_a_variant"]), 2
				)

	def test_kind_filter_selects_only_that_kind(self):
		with tempfile.TemporaryDirectory() as tmp:
			out = Path(tmp) / "o"
			self.assertEqual(build(out, ["--kind", "wound"]), 0)
			names = sorted(p.name for p in out.iterdir() if p.suffix == ".png")
			self.assertTrue(all(n.startswith("T_ZombieWound_") for n in names), names)

	def test_module_is_runnable_as_a_script(self):
		with tempfile.TemporaryDirectory() as tmp:
			result = subprocess.run(
				[sys.executable, str(TOOL_DIR / "generate.py"), "--dry-run"],
				capture_output=True,
				text=True,
			)
			self.assertEqual(result.returncode, 0, result.stderr)
			self.assertIn("T_ZombieSurface_", result.stdout)


class ConfigErrorTests(unittest.TestCase):
	def _with_data(self, mutate):
		"""Write a mutated copy of the real data and load it."""
		families_doc = json.loads((TOOL_DIR / "data" / "families.json").read_text())
		variants_doc = json.loads((TOOL_DIR / "data" / "variants.json").read_text())
		mutate(families_doc, variants_doc)
		with tempfile.TemporaryDirectory() as tmp:
			directory = Path(tmp)
			(directory / "families.json").write_text(json.dumps(families_doc))
			(directory / "variants.json").write_text(json.dumps(variants_doc))
			return config.load(directory, SIZE)

	def test_unknown_parameter_is_named(self):
		def mutate(_, variants):
			variants["body"][0]["overrides"]["blood"]["nonsense"] = 1

		with self.assertRaises(config.ConfigError) as caught:
			self._with_data(mutate)
		self.assertIn("nonsense", str(caught.exception))

	def test_unknown_family_is_named(self):
		def mutate(_, variants):
			variants["body"][0]["overrides"]["ectoplasm"] = {}

		with self.assertRaises(config.ConfigError) as caught:
			self._with_data(mutate)
		self.assertIn("ectoplasm", str(caught.exception))

	def test_duplicate_variant_id_is_rejected(self):
		def mutate(_, variants):
			variants["body"].append(dict(variants["body"][0]))

		with self.assertRaises(config.ConfigError) as caught:
			self._with_data(mutate)
		self.assertIn("duplicate", str(caught.exception))

	def test_unknown_zombie_type_is_rejected(self):
		def mutate(_, variants):
			variants["body"][0]["types"] = ["mimic"]

		with self.assertRaises(config.ConfigError) as caught:
			self._with_data(mutate)
		self.assertIn("mimic", str(caught.exception))

	def test_a_period_that_would_not_tile_is_rejected_with_the_legal_values(self):
		def mutate(_, variants):
			variants["body"][0]["overrides"].setdefault("grime", {})["crust_period"] = 37

		with self.assertRaises(config.ConfigError) as caught:
			self._with_data(mutate)
		message = str(caught.exception)
		self.assertIn("37", message)
		self.assertIn("would not tile", message)
		self.assertIn("Legal values", message)


if __name__ == "__main__":
	unittest.main()

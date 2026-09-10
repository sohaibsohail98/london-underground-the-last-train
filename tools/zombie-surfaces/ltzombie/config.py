"""Loading and validating the data files.

The tool is a renderer over `data/`. Nothing here knows what a mask looks like;
it knows what a well formed variant record is, and it refuses a malformed one
with a message that names the fix. That matters more than it sounds: the whole
point of the data driven split is that somebody adds content by editing JSON,
and a bad error message there costs more than a bad error message in the
renderer.
"""

from __future__ import annotations

import json
from pathlib import Path

from . import noise

FAMILIES = ("blood", "grime", "lividity", "wound")
TILING = ("blood", "grime", "lividity")

# Parameters that index a noise lattice and therefore have to divide the
# texture size for the result to tile. Checked up front against the build size
# rather than deep inside a generator.
PERIOD_KEYS = {
	"blood": ("breakup_period", "sheet_period", "wander_period", "pool_edge_period"),
	"grime": ("period", "detail_period", "crust_period"),
	"lividity": ("period",),
	"wound": ("tear_period",),
}

ZOMBIE_TYPES = ("walker", "sprinter", "brute", "crawler", "screamer")


class ConfigError(ValueError):
	"""A data file problem, phrased for whoever is editing the data file."""


def _merge(defaults: dict, overrides: dict, where: str) -> dict:
	merged = dict(defaults)
	for key, value in overrides.items():
		if key not in defaults:
			raise ConfigError(
				f"{where}: unknown parameter {key!r}. Valid parameters for this "
				f"family are {sorted(defaults)}"
			)
		merged[key] = value
	return merged


def _check_periods(family: str, params: dict, size: int, where: str) -> None:
	for key in PERIOD_KEYS.get(family, ()):
		period = int(params[key])
		if size % period:
			raise ConfigError(
				f"{where}: {family}.{key} is {period}, which does not divide the "
				f"build size {size}, so the texture would not tile. Legal values "
				f"at this size are {noise.divisors(size)}"
			)


def load(data_dir: Path, size: int) -> dict:
	"""Read `families.json` and `variants.json` and return resolved records.

	Returns a dict with `body` and `wound` lists. Each record carries its id,
	its seed, its note, its advisory types, and a fully resolved `params` map
	per family with the variant's overrides already merged in, so nothing
	downstream has to think about defaults again.
	"""
	families_path = data_dir / "families.json"
	variants_path = data_dir / "variants.json"
	for path in (families_path, variants_path):
		if not path.is_file():
			raise ConfigError(f"missing data file: {path}")

	defaults = json.loads(families_path.read_text(encoding="utf-8"))
	variants = json.loads(variants_path.read_text(encoding="utf-8"))

	for family in FAMILIES:
		if family not in defaults:
			raise ConfigError(f"{families_path}: no defaults for family {family!r}")

	resolved: dict = {"body": [], "wound": []}
	seen: set[str] = set()

	for kind, wanted in (("body", TILING), ("wound", ("wound",))):
		records = variants.get(kind)
		if not isinstance(records, list) or not records:
			raise ConfigError(f"{variants_path}: {kind!r} must be a non empty array")

		for record in records:
			for key in ("id", "seed"):
				if key not in record:
					raise ConfigError(f"{variants_path}: a {kind} record is missing {key!r}")
			identifier = str(record["id"])
			if identifier in seen:
				raise ConfigError(f"{variants_path}: duplicate variant id {identifier!r}")
			seen.add(identifier)

			for type_name in record.get("types", []):
				if type_name not in ZOMBIE_TYPES:
					raise ConfigError(
						f"{variants_path}: variant {identifier!r} names type "
						f"{type_name!r}, which is not one of {list(ZOMBIE_TYPES)}"
					)

			overrides = record.get("overrides", {})
			for family in overrides:
				if family not in FAMILIES:
					raise ConfigError(
						f"{variants_path}: variant {identifier!r} overrides unknown "
						f"family {family!r}. Valid families are {list(FAMILIES)}"
					)

			params = {}
			for family in wanted:
				where = f"{variants_path}: variant {identifier!r}"
				params[family] = _merge(defaults[family], overrides.get(family, {}), where)
				_check_periods(family, params[family], size, where)

			resolved[kind].append(
				{
					"id": identifier,
					"seed": int(record["seed"]),
					"note": str(record.get("note", "")),
					"types": list(record.get("types", [])),
					"params": params,
				}
			)

	return resolved

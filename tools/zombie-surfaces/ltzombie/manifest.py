"""The manifest: the contract with the Unreal side.

Deliberately carries no timestamp. A manifest with a build time in it is not
byte identical between two builds, which would either break the determinism
promise or force the manifest to be excluded from it, and the manifest is
exactly the file most worth checking. Provenance is available behind `--stamp`
for a human reading a one off build, and is off by default so committed output
stays stable.

The `unreal` block per entry is the part that earns its keep. Without it,
importing a dozen textures means making a dozen decisions about sRGB and
compression, and getting sRGB wrong on a mask is the classic quiet bug: it
looks nearly right and every value is wrong.
"""

from __future__ import annotations

import hashlib
import json

SCHEMA_VERSION = 1

# Applies to every packed body mask. Kept as one dict rather than repeated per
# entry so a change is a change in one place.
BODY_UNREAL = {
	"content_root": "/Game/LastTrain/Zombies/Surfaces",
	"asset_prefix": "T_ZombieSurface_",
	"compression_settings": "TC_Masks",
	"srgb": False,
	"texture_group": "TEXTUREGROUP_Character",
	"address_x": "TA_Wrap",
	"address_y": "TA_Wrap",
	"mip_gen_settings": "TMGS_FromTextureGroup",
	"sampler_type": "SAMPLERTYPE_MASKS",
	"channels": {
		"R": "blood, fresh and dried fluid",
		"G": "grime, soot and dust accumulation",
		"B": "lividity, dead flesh mottling",
	},
}

WOUND_UNREAL = {
	"content_root": "/Game/LastTrain/Zombies/Surfaces",
	"asset_prefix": "T_ZombieWound_",
	"compression_settings": "TC_Masks",
	"srgb": False,
	"texture_group": "TEXTUREGROUP_Character",
	"address_x": "TA_Clamp",
	"address_y": "TA_Clamp",
	"mip_gen_settings": "TMGS_FromTextureGroup",
	"sampler_type": "SAMPLERTYPE_MASKS",
	"compression_no_alpha": False,
	"channels": {
		"R": "depth, how deep the wound reads",
		"G": "rim, dried and crusted edge",
		"B": "wetness, where it still runs",
		"A": "shape, the decal's own opacity",
	},
}


def sha256_file(path) -> str:
	digest = hashlib.sha256()
	with open(path, "rb") as handle:
		for block in iter(lambda: handle.read(1 << 20), b""):
			digest.update(block)
	return digest.hexdigest()


def entry(kind: str, record: dict, path, relative: str, size: int) -> dict:
	"""One manifest entry. `record` is a resolved variant from `config.load`."""
	return {
		"id": record["id"],
		"kind": kind,
		"path": relative,
		"bytes": path.stat().st_size,
		"sha256": sha256_file(path),
		"size_px": size,
		"seed": record["seed"],
		"note": record["note"],
		"types": record["types"],
		"asset_name": (BODY_UNREAL if kind == "body" else WOUND_UNREAL)["asset_prefix"]
		+ record["id"],
		"params": record["params"],
	}


def build(entries: list, size: int, stamp: dict | None = None) -> dict:
	document = {
		"schema_version": SCHEMA_VERSION,
		"tool": "zombie-surfaces",
		"size_px": size,
		"texel_density_note": (
			"Tiling detail masks, so world texel density is set by the material's "
			"tiling parameter and not by this file. The wound decals are Class A, "
			"512 px per metre, which at this size is a decal about "
			f"{size / 512.0:.2f} m across."
		),
		"unreal": {"body": BODY_UNREAL, "wound": WOUND_UNREAL},
		"entries": entries,
	}
	if stamp:
		document["provenance"] = stamp
	return document


def write(document: dict, path) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	# sort_keys and a fixed separator so the bytes depend only on the content.
	path.write_text(
		json.dumps(document, indent=2, sort_keys=True, separators=(",", ": ")) + "\n",
		encoding="utf-8",
	)

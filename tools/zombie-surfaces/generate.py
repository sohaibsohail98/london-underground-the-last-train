#!/usr/bin/env python3
"""Generate the zombie surface variation set.

Reads `data/families.json` and `data/variants.json`, renders every variant, and
writes packed PNGs, a manifest and a contact sheet.

	python3 tools/zombie-surfaces/generate.py --out build/zombie-surfaces
	python3 tools/zombie-surfaces/generate.py --only drenched_01 --size 512
	python3 tools/zombie-surfaces/generate.py --dry-run

Determinism: the same data, size and seed offset produce byte identical PNGs
and a byte identical manifest. `--stamp` is the one exception and is off by
default, because provenance carries a build time and a build time is not
reproducible.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

TOOL_DIR = Path(__file__).resolve().parent
if str(TOOL_DIR) not in sys.path:
	sys.path.insert(0, str(TOOL_DIR))

from ltzombie import config, contactsheet, families, manifest, render  # noqa: E402
from ltzombie import __version__  # noqa: E402

VALID_SIZES = (256, 512, 1024, 2048, 4096)


def parse_args(argv=None) -> argparse.Namespace:
	parser = argparse.ArgumentParser(
		prog="zombie-surfaces",
		description="Deterministic zombie surface masks for LAST TRAIN.",
	)
	parser.add_argument(
		"--out",
		type=Path,
		default=Path("build/zombie-surfaces"),
		help="output directory (default: build/zombie-surfaces)",
	)
	parser.add_argument(
		"--data",
		type=Path,
		default=TOOL_DIR / "data",
		help="data directory holding families.json and variants.json",
	)
	parser.add_argument(
		"--size",
		type=int,
		default=2048,
		choices=VALID_SIZES,
		help="square texture size in pixels (default: 2048)",
	)
	parser.add_argument(
		"--seed",
		type=int,
		default=0,
		help="offset added to every variant seed, so the whole set can be reshuffled "
		"without editing the data file (default: 0)",
	)
	parser.add_argument(
		"--only",
		action="append",
		metavar="ID",
		help="render only this variant id; repeatable",
	)
	parser.add_argument(
		"--kind",
		choices=("body", "wound", "all"),
		default="all",
		help="render only body masks, only wound decals, or both (default: all)",
	)
	parser.add_argument(
		"--dry-run",
		action="store_true",
		help="resolve and validate the data, list what would be written, write nothing",
	)
	parser.add_argument(
		"--no-contact-sheet",
		action="store_true",
		help="skip the review sheet",
	)
	parser.add_argument(
		"--stamp",
		action="store_true",
		help="add build time and git commit to the manifest. Breaks byte determinism, "
		"so leave it off for anything committed",
	)
	return parser.parse_args(argv)


def _git_commit() -> str:
	try:
		out = subprocess.run(
			["git", "rev-parse", "HEAD"],
			capture_output=True,
			text=True,
			check=True,
			cwd=TOOL_DIR,
		)
		return out.stdout.strip()
	except (subprocess.CalledProcessError, OSError):
		return "unknown"


def _selected(records: list, only) -> list:
	if not only:
		return records
	wanted = set(only)
	chosen = [r for r in records if r["id"] in wanted]
	return chosen


def main(argv=None) -> int:
	args = parse_args(argv)

	try:
		resolved = config.load(args.data, args.size)
	except config.ConfigError as exc:
		print(f"data error: {exc}", file=sys.stderr)
		return 2

	body = _selected(resolved["body"], args.only) if args.kind in ("body", "all") else []
	wound = _selected(resolved["wound"], args.only) if args.kind in ("wound", "all") else []

	if args.only:
		known = {r["id"] for r in resolved["body"]} | {r["id"] for r in resolved["wound"]}
		unknown = sorted(set(args.only) - known)
		if unknown:
			print(
				f"unknown variant id(s) {unknown}; known ids are {sorted(known)}",
				file=sys.stderr,
			)
			return 2

	if not body and not wound:
		print("nothing selected to render", file=sys.stderr)
		return 2

	if args.dry_run:
		print(f"zombie-surfaces {__version__}, size {args.size}, seed offset {args.seed}")
		print(f"would write into {args.out}")
		for record in body:
			print(f"  body   {manifest.BODY_UNREAL['asset_prefix']}{record['id']}.png")
		for record in wound:
			print(f"  wound  {manifest.WOUND_UNREAL['asset_prefix']}{record['id']}.png")
		if not args.no_contact_sheet:
			print("  contact-sheet.png")
		print("  manifest.json")
		return 0

	entries = []
	sheet_body = []
	sheet_wound = []

	for record in body:
		seed = record["seed"] + args.seed
		params = record["params"]
		fields = {
			name: families.TILING_FAMILIES[name](args.size, seed, params[name])
			for name in ("blood", "grime", "lividity")
		}
		image = render.pack_body(fields["blood"], fields["grime"], fields["lividity"])
		name = f"{manifest.BODY_UNREAL['asset_prefix']}{record['id']}.png"
		path = args.out / name
		render.save_png(image, path)
		entries.append(manifest.entry("body", record, path, name, args.size))
		sheet_body.append(
			(
				record,
				render.to_bytes(fields["blood"]),
				render.to_bytes(fields["grime"]),
				render.to_bytes(fields["lividity"]),
			)
		)
		print(f"body   {name}  {path.stat().st_size // 1024} KiB")

	for record in wound:
		seed = record["seed"] + args.seed
		field = families.wound(args.size, seed, record["params"]["wound"])
		image = render.pack_wound(field)
		name = f"{manifest.WOUND_UNREAL['asset_prefix']}{record['id']}.png"
		path = args.out / name
		render.save_png(image, path)
		entries.append(manifest.entry("wound", record, path, name, args.size))
		sheet_wound.append((record, render.to_bytes(field)))
		print(f"wound  {name}  {path.stat().st_size // 1024} KiB")

	if not args.no_contact_sheet:
		sheet = contactsheet.build(sheet_body, sheet_wound, args.size)
		sheet_path = args.out / "contact-sheet.png"
		render.save_png(sheet, sheet_path)
		print(f"sheet  contact-sheet.png  {sheet_path.stat().st_size // 1024} KiB")

	stamp = None
	if args.stamp:
		stamp = {
			"generated": datetime.now(timezone.utc).isoformat(timespec="seconds"),
			"commit": _git_commit(),
			"tool_version": __version__,
		}
	document = manifest.build(entries, args.size, stamp)
	manifest.write(document, args.out / "manifest.json")
	print(f"manifest  {len(entries)} entries  {args.out / 'manifest.json'}")
	return 0


if __name__ == "__main__":
	sys.exit(main())

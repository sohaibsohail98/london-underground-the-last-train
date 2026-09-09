#!/usr/bin/env bash
#
# Stages the Phase F asset shopping list into _incoming_assets/.
#
# Why this exists: the remote Claude session that researched these sources has
# no network route to any asset host, and _incoming_assets/ is gitignored and
# lives only on this machine. So the research is committed as
# docs/reference/asset-sources-phase-f.md and this script turns it back into a
# folder of files here, where the hosts are reachable.
#
# What it does:
#   - creates _incoming_assets/<category>/ and writes a SOURCES.txt per category
#   - downloads the ambientCG and Poly Haven sets, which have public keyless APIs
#   - prints a manual checklist for everything that needs a human click
#
# What it deliberately does NOT do:
#   - import anything into the project (the editor session triages)
#   - run git, or add anything to it (assets are gitignored, keep it that way)
#   - touch any account-gated, paid or licence-click source
#
# Nothing here is verified. Every licence in the manifest rests on search
# evidence, not on a page anyone read. Confirm the licence on the page before
# you use a file, and work the "Inspect before staging" section of the manifest.
#
# Overridable by environment:
#   LASTTRAIN_ASSET_RES     default 2K   (1K, 2K, 4K, 8K)
#   LASTTRAIN_ASSET_FORMAT  default JPG  (JPG or PNG, ambientCG only)
#   LASTTRAIN_ASSET_DEST    default <repo>/_incoming_assets
#   LASTTRAIN_DRY_RUN       set to 1 to list what would be fetched and stop

set -euo pipefail

RES="${LASTTRAIN_ASSET_RES:-2K}"
FMT="${LASTTRAIN_ASSET_FORMAT:-JPG}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DEST="${LASTTRAIN_ASSET_DEST:-$REPO_ROOT/_incoming_assets}"
DRY="${LASTTRAIN_DRY_RUN:-0}"

MANIFEST="docs/reference/asset-sources-phase-f.md"

# ---------------------------------------------------------------------------
# The list. Keep this in step with the manifest: one line per asset, as
# "<AssetID>|<category>|<one line on what it is for>".
# ---------------------------------------------------------------------------

AMBIENTCG=(
	"Tiles036|surfaces/tile|Hero pair, clean half. Canonical clean white subway tile, procedural."
	"Tiles133B|surfaces/tile|Hero pair, dirty half. Same white family, grimed and aged. Top priority."
	"Tiles010|surfaces/tile|Second white subway tile, different bond and grout width."
	"Tiles032|surfaces/tile|Dark green glazed accent band, reflective out of the box."
	"Tiles033|surfaces/tile|Yellow, same procedural series. Pull toward cream in the material."
	"Tiles141|surfaces/tile|Tagged Urban, Dirty and Stained. Worst-maintained stretch of wall."
	"Tiles093|surfaces/tile|Black, dark, old, random. Dark utility floor."
	"PaintedPlaster017|surfaces/concrete|First pick above the tile line. Photogrammetry, real roller texture."
	"PaintedPlaster003|surfaces/concrete|Variation between bays so the ceiling is not one plane."
	"Concrete020|surfaces/concrete|Bare structural concrete under the paint, and the soffit."
	"Concrete012|surfaces/concrete|Bare concrete variant. Keep whichever albedo is flattest."
	"Concrete040|surfaces/concrete|Bare concrete variant. Keep whichever albedo is flattest."
	"Concrete047A|surfaces/floor|Possible polished concrete. Verify visually, evidence is weak."
	"Metal009|surfaces/metal|The brushed steel pick. Handrails, bench frames, door surrounds, bins."
	"Metal032|surfaces/metal|Clean smooth metal for anything recently replaced."
	"Metal036|surfaces/metal|Sibling of Metal032 at a different roughness."
	"PaintedMetal001|surfaces/metal|Scratched yellow. Retints to sodium E0A030 or crimson B02030."
	"PaintedMetal012|surfaces/metal|White with rust blooming through. Bins, trunking, board housing."
	"PaintedMetal006|surfaces/metal|Green with rust. Retints to violet 6C4C9C cleanly."
	"MetalPlates003|surfaces/metal|Panelised metal for a service door or cable riser."
	"TactilePaving001|surfaces/tactile|The platform-edge strip, first pick. Purpose-built blister spacing."
	"TactilePaving003|surfaces/tactile|Blister variant. Pick the closest stud pitch, keep one or two."
	"TactilePaving004|surfaces/tactile|Blister variant. Pick the closest stud pitch, keep one or two."
	"TactilePaving005|surfaces/tactile|Blister variant. Pick the closest stud pitch, keep one or two."
	"Rubber003|surfaces/tactile|Handrail grip, door seals, stair nosings."
	"Rubber004|surfaces/tactile|Handrail grip, door seals, stair nosings."
	"Terrazzo004|surfaces/floor|The polished floor answer. Period correct, real specular sheen."
	"Terrazzo001|surfaces/floor|Second aggregate size, for the concourse or platform split."
	"Terrazzo005|surfaces/floor|Third aggregate size."
	"Rust001|surfaces/decal-source|Tiling rust, NOT an alpha decal. Mask it or streak procedurally."
	"Rust004|surfaces/decal-source|Tiling rust, NOT an alpha decal."
	"Gravel023|surfaces/trackbed|Closest to ballast. Photoscanned pebbles: strengthen normal, desaturate."
	"Gravel042|surfaces/trackbed|Ballast candidate."
	"Asphalt002|surfaces/trackbed|Trackbed and access ways."
	"Planks010|surfaces/trackbed|Sleeper shape. Also drives a board-marked concrete height map."
	"Wood049|surfaces/trackbed|Weathered timber, darken to creosote."
)

POLYHAVEN=(
	"painted_concrete|surfaces/concrete|Painted concrete already failing. Damaged half of a blend."
	"concrete_wall_008|surfaces/concrete|Chipped plaster, hairline cracks and bolt holes. The bolt holes sell it."
	"precast_concrete_wall|surfaces/concrete|Closest CC0 answer to coffering. Stamped rectangular pattern."
	"concrete_panels|surfaces/concrete|Corrugated grooves, nearest CC0 thing to board marking."
	"preconcrete_wall_001|surfaces/concrete|Cracked, chipped, stained weathered plaster."
	"preconcrete_wall_001_long|surfaces/concrete|Non-square tile, for a long platform wall run."
	"peeling_painted_wall|surfaces/concrete|Peeling paint, first pick."
	"cracked_concrete_wall|surfaces/concrete|Bare plaster, weathered, cracked, damaged."
	"concrete_wall_003|surfaces/concrete|Worn, discoloured, dirty painted plaster."
	"concrete_wall_006|surfaces/concrete|Rough, chipped, dirty plaster."
	"smooth_concrete_floor|surfaces/floor|First pick for the platform floor. Smooth base for a wet puddle mask."
	"concrete_floor_02|surfaces/floor|Dirty variant to blend at the platform edge. Desaturate the moss."
	"hangar_concrete_floor|surfaces/floor|Large-scale industrial floor, does not repeat across a wide platform."
	"worn_concrete_floor|surfaces/floor|Wear detail for the boarding zone."
	"concrete_floor_worn_001|surfaces/floor|Wear detail, indoor tagged."
	"dirty_tiles|surfaces/tile|Best single answer for dirty grout. Reddish brown doubles as oxblood."
	"interior_tiles|surfaces/tile|Beige ceramic, dark grout, soft bevels, low sheen."
	"worn_tile_floor|surfaces/tile|Ceramic glazed, weathered."
	"tiled_floor_001|surfaces/tile|Works on floor and wall."
	"rubber_tiles|surfaces/tactile|Plain rubber matting, first pick. Matte and dark, holds the charcoal end."
	"metal_plate_02|surfaces/metal|Photogrammetry plate. Floor hatch, cabinet door, plated ramp section."
	"asphalt_01|surfaces/trackbed|Trackbed and access ways."
)

# ---------------------------------------------------------------------------

log() { printf '%s\n' "$*"; }
have() { command -v "$1" >/dev/null 2>&1; }

for tool in curl python3; do
	have "$tool" || { log "ERROR: $tool is required and is not on PATH."; exit 1; }
done

log "LAST TRAIN, Phase F asset staging"
log "  manifest:   $MANIFEST"
log "  dest:       $DEST"
log "  resolution: $RES $FMT"
log ""
log "Nothing here is verified. Confirm each licence on the page, and work the"
log "'Inspect before staging' section of the manifest before you use a file."
log ""

if [ "$DRY" = "1" ]; then
	log "DRY RUN. Would fetch ${#AMBIENTCG[@]} ambientCG and ${#POLYHAVEN[@]} Poly Haven sets:"
	for entry in "${AMBIENTCG[@]}"; do log "  ambientCG  ${entry%%|*}"; done
	for entry in "${POLYHAVEN[@]}"; do log "  polyhaven  ${entry%%|*}"; done
	exit 0
fi

started_at="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
# Plain string rather than an array: bash 3.2, which is what macOS ships,
# errors on an empty array expansion under set -u.
FAILED=""
FAILED_COUNT=0
note_failure() {
	FAILED="${FAILED}  $1\n"
	FAILED_COUNT=$((FAILED_COUNT + 1))
}

# Writes one SOURCES.txt line for a file that landed.
record() {
	local category="$1" source="$2" asset="$3" licence="$4" url="$5" note="$6"
	local file="$DEST/$category/SOURCES.txt"
	if [ ! -f "$file" ]; then
		{
			printf 'Phase F asset staging, category: %s\n' "$category"
			printf 'Staged %s by tools/asset-fetch/fetch-phase-f.sh\n' "$started_at"
			printf 'Research: %s\n' "$MANIFEST"
			printf '\n'
			printf 'Licences below are as recorded in the manifest and rest on search\n'
			printf 'evidence, not on a page anyone read. Confirm on the source page.\n'
			printf '\n'
		} > "$file"
	fi
	{
		printf 'asset:   %s\n' "$asset"
		printf 'source:  %s\n' "$source"
		printf 'licence: %s\n' "$licence"
		printf 'url:     %s\n' "$url"
		printf 'for:     %s\n' "$note"
		printf '\n'
	} >> "$file"
}

fetch_ambientcg() {
	local id="$1" category="$2" note="$3"
	local dir="$DEST/$category"
	local zip="$dir/${id}_${RES}-${FMT}.zip"
	mkdir -p "$dir"

	if [ -f "$zip" ]; then
		log "  skip     $id (already staged)"
		return 0
	fi

	local url="https://ambientcg.com/get?file=${id}_${RES}-${FMT}.zip"
	if ! curl -fsSL --retry 3 --retry-delay 2 -o "$zip" "$url"; then
		# Constructed URL failed. Fall back to the documented v2 API, which
		# returns exact file URLs as CSV.
		log "  retry    $id via the v2 API"
		local csv
		csv="$(curl -fsSL --retry 3 "https://ambientcg.com/api/v2/downloads_csv?id=${id}" || true)"
		# Do not depend on the CSV column names: just find a URL in the
		# response whose filename carries the resolution and format we want.
		local api_url
		api_url="$(printf '%s' "$csv" | python3 -c '
import re, sys
want = (sys.argv[1] + "-" + sys.argv[2]).lower()
text = sys.stdin.read()
for url in re.findall(r"https?://[^\s,\"]+", text):
    if want in url.lower() and url.lower().endswith(".zip"):
        print(url)
        break
' "$RES" "$FMT" 2>/dev/null || true)"
		if [ -n "$api_url" ] && curl -fsSL --retry 3 -o "$zip" "$api_url"; then
			url="$api_url"
		else
			rm -f "$zip"
			log "  FAILED   $id"
			note_failure "ambientCG $id"
			return 0
		fi
	fi

	log "  ok       $id"
	record "$category" "ambientCG" "$id" "CC0 1.0 Universal (https://docs.ambientcg.com/license/)" \
		"https://ambientcg.com/view?id=${id}" "$note"
}

fetch_polyhaven() {
	local slug="$1" category="$2" note="$3"
	local dir="$DEST/$category/$slug"
	mkdir -p "$dir"

	if [ -n "$(ls -A "$dir" 2>/dev/null)" ]; then
		log "  skip     $slug (already staged)"
		return 0
	fi

	# The keyless public API is authoritative for which maps exist and where.
	local files_json
	files_json="$(curl -fsSL --retry 3 "https://api.polyhaven.com/files/${slug}" || true)"
	if [ -z "$files_json" ]; then
		log "  FAILED   $slug (API returned nothing)"
		note_failure "Poly Haven $slug"
		return 0
	fi

	local urls
	urls="$(printf '%s' "$files_json" | python3 -c '
import json, sys
res = sys.argv[1].lower()
fmt = "jpg"
data = json.load(sys.stdin)
out = []
for map_name, by_res in data.items():
    if not isinstance(by_res, dict):
        continue
    entry = by_res.get(res)
    if not isinstance(entry, dict):
        continue
    node = entry.get(fmt) or entry.get("png")
    if isinstance(node, dict) and node.get("url"):
        out.append(node["url"])
for u in out:
    print(u)
' "$RES" 2>/dev/null || true)"

	if [ -z "$urls" ]; then
		rmdir "$dir" 2>/dev/null || true
		log "  FAILED   $slug (no $RES maps in the API response)"
		note_failure "Poly Haven $slug"
		return 0
	fi

	local count=0
	while IFS= read -r u; do
		[ -n "$u" ] || continue
		if curl -fsSL --retry 3 --retry-delay 2 -o "$dir/$(basename "$u")" "$u"; then
			count=$((count + 1))
		fi
	done <<< "$urls"

	if [ "$count" -eq 0 ]; then
		rmdir "$dir" 2>/dev/null || true
		log "  FAILED   $slug (nothing downloaded)"
		note_failure "Poly Haven $slug"
		return 0
	fi

	log "  ok       $slug ($count maps)"
	record "$category" "Poly Haven" "$slug" "CC0 (https://polyhaven.com/license), no attribution required" \
		"https://polyhaven.com/a/${slug}" "$note"
}

log "ambientCG (${#AMBIENTCG[@]} set(s))"
for entry in "${AMBIENTCG[@]}"; do
	IFS='|' read -r id category note <<< "$entry"
	fetch_ambientcg "$id" "$category" "$note"
done

log ""
log "Poly Haven (${#POLYHAVEN[@]} set(s))"
for entry in "${POLYHAVEN[@]}"; do
	IFS='|' read -r slug category note <<< "$entry"
	fetch_polyhaven "$slug" "$category" "$note"
done

log ""
if [ "$FAILED_COUNT" -gt 0 ]; then
	log "$FAILED_COUNT set(s) failed. Fetch these by hand:"
	printf '%b' "$FAILED"
	log ""
fi

cat <<'MANUAL'
Manual checklist: everything below needs a human click, an account decision or
a legal inspection. Section numbers refer to docs/reference/asset-sources-phase-f.md.

SURFACES, per-map download buttons, no URL pattern (section 1)
  cgbookcase Subway Tiles 01   https://www.cgbookcase.com/textures/subway-tiles-01
      Confirm the CC0 line on the page: the manifest has it at medium confidence.
  TextureCan tactile pavement  https://www.texturecan.com/details/47/
  3dtextures.me Subway Tiles   https://3dtextures.me/2018/02/17/subway-tiles-001/

MESHES, itch.io and per-site downloads (section 2)
  Kenney, no account, direct zips:
      https://kenney.nl/assets/city-kit-commercial
      https://kenney.nl/assets/furniture-kit
      https://kenney.nl/assets/conveyor-kit
      https://kenney.nl/assets/retro-urban-kit
      https://kenney.nl/assets/train-kit          (scale sanity check only)
  Quaternius, no account:
      https://quaternius.com/packs/modularscifimegakit.html   (proportion reference only)
      https://quaternius.com/packs/modulartrain.html          (menu silhouette only)
  3DModelsCC0 on itch.io, CC0 stated, formats unconfirmed:
      https://3dmodelscc0.itch.io/city-environment-pack
      https://3dmodelscc0.itch.io/free-cc0-city-environment-pack-2
      https://3dmodelscc0.itch.io/free-cc0-industrial-3d-models
      https://3dmodelscc0.itch.io/free-cc0-3d-industrial-props-pack-2
  Poly Haven props, pick a handful by hand:
      https://polyhaven.com/models/props  and  /models/industrial/props

  DECISION, not a download:
      https://loafbrr.itch.io/modular-underground-metro is CC-BY 1.0, not CC0.
      It is the closest thing to a station kit that exists free, and using any
      of it means carrying a credit line. Decide before staging.

REFERENCE PHOTOGRAPHY, licence is per file, never per site (section 3)
      Wikimedia Commons and Geograph. Start with the Canary Wharf and Elizabeth
      line categories and the Featured pictures page. Record source URL, file
      name, author, exact licence and capture date for every file you keep.
      Highest value for a modeller: Library of Congress HABS/HAER measured
      drawings, which give dimensions rather than impressions.

TRAIN (section 4)
      Model from Class 720 or Class 701 reference plus the Class 345 numbers.
      Do not build reference habits out of the purple fleet.
      The FOI drawings on whatdotheyknow.com are the best single find; read the
      re-use notice attached to the response before relying on them.
      No CC0 kitbash mesh is worth using: box-model from the dimensions instead.

MENU AUDIO (section 5)
      https://opengameart.org/content/sci-fi-drone-loop   (base layer)
      https://signaturesounds.org/store/p/room-tones      (hall or basement tone)
      Read the Signature Sounds licence page before relying on it: the CC0 claim
      rests on one unread source.
      Check https://freesound.org/people/Kinoton/sounds/353159/ first, since if
      it is genuinely CC0 it is a one-file solution.
      Listen to every candidate end to end at raised gain. Reject anything with
      an announcement, a chime or a jingle, whatever licence it claims.

BEFORE ANY OF IT IS IMPORTED
      Work section 6 of the manifest, "Inspect before staging". Four items carry
      an unresolved legal question.
      Then append what actually landed to _incoming_assets/ASSET-RESEARCH.md.
      Do not git add anything under _incoming_assets/. It is gitignored, and it
      stays that way.
MANUAL

log ""
log "Done. Staged under $DEST"

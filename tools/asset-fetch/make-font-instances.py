#!/usr/bin/env python3
"""Cut static weight instances out of the Overpass variable fonts.

Unreal's font importer takes a TTF and gives you one instance of it. Handed a
variable font it takes the default instance and offers no way to pick another,
which is a problem in both directions here:

  Overpass       default is Regular 400. Fine, but there is no second weight,
                 and the typographic policy in tfl-dimensional-reference.md
                 emphasises with weight rather than italics or capitals.
  Overpass Mono  default is Light 300. Too thin to be read at a glance off a
                 HUD readout, which is the one job it has.

So the weights are cut here instead, and the editor imports plain static TTFs.

The Overpass copyright carries no Reserved Font Name, unlike many OFL families,
so the SIL Open Font License permits these modified versions to keep the family
name. The copyright and licence strings ride along in each instance's name
table; OFL.txt sits beside them. Both conditions the licence imposes are met.

This is a fix to reach for, not a step in the normal path. The fonts are already
imported at Content/LastTrain/UI/Fonts/. Run this only if Font_UI_OverpassMono
turns out to be Light, which happens when the variable file was imported
directly. See docs/art-direction.md section 7.

Usage:
    python3 tools/asset-fetch/make-font-instances.py <source-dir> <output-dir>

<source-dir> holds the variable fonts. They are not committed, being redundant
with the imported assets. Fetch them from google/fonts, which is public:

    base=https://raw.githubusercontent.com/google/fonts/main/ofl
    curl -o 'Overpass[wght].ttf'     "$base/overpass/Overpass%5Bwght%5D.ttf"
    curl -o 'OverpassMono[wght].ttf' "$base/overpassmono/OverpassMono%5Bwght%5D.ttf"

The OFL.txt files are already in the repository beside the imported fonts, so
there is no need to fetch those again.
"""

from __future__ import annotations

import sys
from pathlib import Path

try:
    from fontTools import varLib
    from fontTools.ttLib import TTFont
    from fontTools.varLib import instancer
except ImportError:
    print("error: fonttools is not installed. pip install fonttools")
    sys.exit(1)

# Source file, weight axis value, output name, style name for the name table.
INSTANCES = [
    ("Overpass[wght].ttf", 400, "Overpass-Regular.ttf", "Regular"),
    ("Overpass[wght].ttf", 700, "Overpass-Bold.ttf", "Bold"),
    ("OverpassMono[wght].ttf", 400, "OverpassMono-Regular.ttf", "Regular"),
    ("OverpassMono[wght].ttf", 700, "OverpassMono-Bold.ttf", "Bold"),
]

# Name table records that must survive instancing for the licence to be met.
REQUIRED_NAME_IDS = {0: "copyright", 13: "licence", 14: "licence URL"}


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__.strip())
        return 2

    source = Path(sys.argv[1])
    output = Path(sys.argv[2])
    output.mkdir(parents=True, exist_ok=True)

    for filename, weight, out_name, style in INSTANCES:
        src = source / filename
        if not src.is_file():
            print(f"error: {src} does not exist")
            return 1

        font = TTFont(src)
        if "fvar" not in font:
            print(f"error: {src} is not a variable font")
            return 1

        axis = {a.axisTag: a for a in font["fvar"].axes}["wght"]
        if not axis.minValue <= weight <= axis.maxValue:
            print(f"error: {filename} weight axis is {axis.minValue} to "
                  f"{axis.maxValue}, cannot cut {weight}")
            return 1

        instance = instancer.instantiateVariableFont(font, {"wght": weight})

        # instantiateVariableFont leaves the family and style names describing
        # the variable default, so state the weight explicitly. Anything else
        # and the editor shows four fonts all called Regular.
        name = instance["name"]
        family = name.getDebugName(16) or name.getDebugName(1)
        name.setName(family, 1, 3, 1, 0x409)
        name.setName(style, 2, 3, 1, 0x409)
        name.setName(f"{family} {style}", 4, 3, 1, 0x409)
        name.setName(f"{family.replace(' ', '')}-{style}", 6, 3, 1, 0x409)
        instance["OS/2"].usWeightClass = weight

        missing = [
            label for nid, label in REQUIRED_NAME_IDS.items()
            if not name.getDebugName(nid)
        ]
        if missing:
            print(f"error: {out_name} lost its {', '.join(missing)} record; "
                  f"instancing it would breach the licence")
            return 1

        instance.save(output / out_name)
        size = (output / out_name).stat().st_size
        print(f"  {out_name:<28} weight {weight:<4} {size:>7,} bytes")

    print()
    print(f"wrote {len(INSTANCES)} static instances to {output}")
    print("OFL.txt must sit beside them. Do not ship one without the other.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

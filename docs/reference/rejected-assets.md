# Assets inspected and rejected

One row per asset that was looked at properly and then kept out of the tree, with
the reason. The point is that nobody re-adds them in six months, and that a
future licence audit can see the filter was actually applied rather than assumed.

Opened and rejected on 2026-09-09, from the batch handed to the remote session.

## 1. Paddington Plain, TrueType font

**Rejected on the trademark filter. Do not add.**

| Field | Value |
|---|---|
| File | `paddington.ttf`, 22,648 bytes, 1000 units per em |
| Name table | Family `Paddington Plain`, subfamily `Roman`, PostScript `PaddingtonPlain` |
| Copyright string | `Stephen Moye, Providence, RI, Version 1.1, February 1997` |
| Licence shipped | None. The archive contained the font and nothing else. |

It is a Johnston revival, and this was verified in the outlines rather than
guessed from the name. The tittle on lowercase `i` is a single six point
contour, every point on curve, at (192, 578), (120, 650), (112, 650),
(40, 578), (112, 506), (120, 506). That is a diamond: a square rotated through
45 degrees and stretched 8 units on the horizontal. Lowercase `j` carries the
same diamond at (203, 575) through (131, 503).

The diamond tittle is Edward Johnston's signature move and the single most
recognisable feature of the Underground alphabet. A face carrying it is a
Johnston lookalike whatever it is called.

Two independent reasons it stays out:

1. `CLAUDE.md` forbids the Johnston and New Johnston typefaces outright. That
   rule does not have a lookalike exemption, and a lookalike is exactly what a
   revival is. The whole point of the substitution strategy in
   `branding-precedent.md` is that the game's lettering should not read as the
   operator's lettering.
2. No licence accompanied the file. Even had the first reason not applied, an
   unlicensed 1997 shareware era font is not something to build a shipping
   wayfinding system on.

**Use instead: Overpass, which the project already chose and already has.**
`Content/LastTrain/UI/Fonts/` holds Overpass, Overpass Mono, Barlow, Barlow
Condensed and Public Sans as imported `Font_` assets, all SIL Open Font Licence,
all with their `OFL.txt` beside them, all listed in `Content/ATTRIBUTION.md`.
`../art-direction.md` section 7 settles Overpass as the functional face on US
highway heritage rather than anything in the Johnston line. There is no font gap
to fill and no need to fetch anything.

Overpass was put through the same outline test as Paddington and passes on the
letterforms, not on its description. Its tittle is a 13 point curved contour in
a 218 by 220 unit box, which is a round dot. Overpass Mono matches at 184 by 188.
Neither has a straight side anywhere in the mark.

For completeness, since it was the original suggestion here: Hammersmith One also
passes the diamond test, with an 8 point curved tittle. It is still the worse
choice. Its copyright carries a Reserved Font Name, so the licence forbids a
modified version keeping the name, and it ships a single weight, which cannot
satisfy the "emphasise with a different weight" rule in
`tfl-dimensional-reference.md` section 6. Overpass has no Reserved Font Name and
a 100 to 900 weight axis. Nothing needs to change.

## 2. Three textures inside the S Stock carriage

**Rejected on the trademark filter. Stripped before the mesh was committed.**

| File | Reason |
|---|---|
| `Underground_Logo_baseColor.png`, 1024 square | The operator's registered circle and bar station mark, with the mode name set in the corporate typeface across the bar |
| `Underground_Logo_metallicRoughness.png`, 1024 square | Companion map for the above |
| `Material.001_baseColor.png`, 256 square | Door warning panel, wording set in the corporate typeface |

The rest of the carriage was fine and is committed at
`SourceArt/ThirdParty/SStock/`. `tools/asset-fetch/clean-sstock.py` performs the
strip and is deterministic, so the result can be re-derived from a fresh
Sketchfab download. `SourceArt/ThirdParty/SStock/LICENCE.txt` records the change
as CC-BY-4.0 requires.

`Electric_Warning_baseColor.png` was **kept**. It is the generic electrical
hazard triangle of BS EN ISO 7010 W012: yellow field, black border, black
lightning bolt, no lettering, no mark, no operator. It is a public safety
symbol, not anybody's property.

## 3. Three operator design standards, as PDFs

**Not rejected on content. Not committed as files.**

| Document | Size |
|---|---|
| Basic elements standard, issue 8 | 548 KB |
| Supplementary signs standard, issue 3 | 7.1 MB |
| Standard for TfL products | 2.8 MB |

These are published documents, freely downloadable, and reading them is exactly
the right thing to do. Re hosting 10 MB of somebody else's copyrighted PDFs in a
public repository is a different act from reading them, and it is not one this
project needs to perform.

So the measurements were extracted and the documents were not committed.
`tfl-dimensional-reference.md` holds panel sizes, cap heights, corner radii,
border widths, mounting heights, materials, finishes and the legibility rule,
with canonical URLs for anyone who wants the drawings. Dimensions and
construction methods are functional facts and are free to use. The identity
those dimensions carry is not, and none of it was extracted.

## The filter, restated

Anything arriving from outside gets three questions before it is committed:

1. **Does the licence permit redistribution of the raw file?** CC0, CC-BY and
   MIT do. UE-Only Content and Fab Standard do not. Personal, educational or
   research only does not.
2. **Does it carry an operator mark anywhere, including inside a texture, a
   glyph or a mesh nobody looks at?** Open the textures. Do not assume.
3. **Can the answer to both be reproduced later?** If the asset needed
   modifying to pass, the modification is a script in `tools/asset-fetch/`, not
   a manual edit somebody remembers doing.

## The outline test, for fonts specifically

A font's name tells you nothing. Run this instead, which is what settled both
questions above:

```python
from fontTools.ttLib import TTFont
from fontTools.pens.recordingPen import RecordingPen

font = TTFont(path)
pen = RecordingPen()
font.getGlyphSet()["i"].draw(pen)
# Rebuild the contours, take the one sitting highest: that is the tittle.
# Straight sided and at most six points means a diamond, which means Johnston.
# Curved, in a roughly square box, means a round dot, which does not.
```

Results so far, all on the lowercase `i` tittle:

| Font | Points | Curved | Box | Verdict |
|---|---|---|---|---|
| Overpass | 13 | yes | 218 by 220 | Round. Passes. |
| Overpass Mono | 13 | yes | 184 by 188 | Round. Passes. |
| Hammersmith One | 8 | yes | 382 by 382 | Round. Passes, but see above. |
| Paddington Plain | 6 | no | 152 by 144 | Diamond. Rejected. |

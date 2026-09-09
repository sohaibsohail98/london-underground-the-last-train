# Art direction

Keyed to the reference frame in `docs/reference/reference-frame-notes.md`.

The `06_ASSET_AND_LEVEL_GUIDE.md` and `08_CLASSIC_ZOMBIES_STYLE_GUIDE.md`
documents cited below were external Project Knowledge uploads. They were never
committed to this repository and are not present here. Their guidance on
restraint is summarised in `docs/reference/reference-frame-notes.md` and the HUD
section of `docs/brief-v3-unreal.md`.

---

## 1. What to copy from the reference

**Composition.** The train fills one entire side of frame as a wall, the
platform recedes to a vanishing point, and the horde arrives down that
corridor. The player is boxed in on one side and open on the other. This is a
better platform composition than anything symmetrical and it should be the
default framing for every platform in the game.

**Palette.** Sodium amber and warm white for functional lighting. Violet as the
line accent, used sparingly and mostly on the rolling stock and signage
furniture: on the train it lives in the door surrounds and a cab band, NOT as
an all-over purple bodyshell (a purple-and-white train reads as the Elizabeth
line, see section 3). Crimson for emergency lighting only, receding into the
tunnel so it reads as depth. Everything else desaturated: concrete, steel,
dirty tile. Saturation is a resource, spend it on the three accents.

**Rolling stock silhouette.** The train is a fictionalised Crossrail-scale
main-line unit modelled on the Class 345 "Aventra", NOT a deep-level tube
train. Long and low, near-vertical box sides with only slight tumblehome, a
flat roof, a deep skirt, a full-width curved wraparound cab windscreen, and a
walk-through articulated interior with full open gangways (the whole train
reads end to end down the centre, not distinct cars with black gaps). Three
double-leaf sliding plug doors per side per car. A 9-car unit is about 205 m
long and fills one whole side of the platform as a wall. Safe livery: charcoal
`#16161C` bodyshell, sodium `#E0A030` cab band, violet `#6C4C9C` door
surrounds, an original wayfinding typeface, a made-up operator mark. Full
physical detail: `docs/reference/canary-wharf-research/rolling-stock.md`.

Working palette, unchanged from v2:

| Role | Hex |
|---|---|
| Charcoal base | `#16161C` |
| Violet accent | `#6C4C9C` |
| Sodium amber | `#E0A030` |
| Crimson | `#B02030` |

**Materials.** Wet platform floor with real reflections, which Lumen gives you
almost free and which does more for the look than any other single decision.
Dirty tile at head height, painted concrete above, brushed steel furniture,
rubber tactile paving. Wear concentrated where people touch things, not sprayed
uniformly.

**Dressing.** Litter drifts in corners rather than scattered evenly. A bin. A
bench run. Abandoned personal items as environmental storytelling. Fluid
staining on the floor near the horde, not everywhere.

**Emergency lighting used as depth cue.** The red units are not general
illumination, they are a gradient down the tunnel telling you where the space
continues. Copy this precisely.

---

## 2. The best idea in the reference, and it is a mechanic

The departure board. In the frame it shows two services with minutes and a
clock. That is the train timer made diegetic.

Do not put the train countdown in the HUD. Put it on the board, on the platform
displays, and in the announcements. The player learns to read the station
rather than a widget. The T-minus-15-seconds announcement then does real work,
and glancing at a board under pressure is a better moment than watching a bar
deplete.

This is the single most valuable thing to lift from the image and it belongs in
Phase 5.

---

## 3. What must not be copied

The reference frame contains protected material. Every item below has a
substitution. If an art task does not restate these, they creep back in.

For why this is the normal path rather than an unusually cautious one, including
the Modern Warfare 3 and Watch Dogs: Legion precedents and why assets from other
London games are not reusable, see `docs/reference/branding-precedent.md`.

| In the reference | Status | Substitute |
|---|---|---|
| Roundel on the platform wall | TfL trademark | Original station mark: a violet horizontal bar over a charcoal field, station name in the project typeface. Distinct silhouette, no circle-and-bar |
| Johnston wayfinding type | Licensed typeface | An original or freely licensed geometric sans. Not a Johnston clone. Something with a taller x-height reads as deliberately different |
| "Way out" panel in house style | Trademark dress | Same information, original panel geometry and colour split |
| Official line diagram | Copyright | Original network schematic, which doubles as the HUD map substitute |
| Elizabeth line identity on rolling stock | Operator identity | Do not lift: the purple ELIZABETH LINE roundel on car sides or station totems; the TfL grey/white bodyshell with a single purple sole-bar stripe as a copied complete livery; New Johnston on destination blinds, car numbering or signage; the operator name or "MTR Elizabeth line" branding; the specific TfL purple used as the sole livery colour with nothing else. Substitute: charcoal `#16161C` bodyshell, sodium `#E0A030` cab band, violet `#6C4C9C` door surrounds, original typeface, made-up operator mark. Violet `#6C4C9C` as one of the four palette colours is fine; a train that is simply purple-and-white and unmarked still reads as Elizabeth line and must be avoided |
| Line name on signage | Trademark | Rename the line in-world. Station names stay factual |
| `SAUG 9MM` | Treyarch weapon name | Original weapon names throughout |
| Modern CoD HUD furniture | Style, and banned by your own guide | Restrained classic HUD per Phase 9 |

**Fair game.** Panel shapes and mounting heights. Sign proportions. Tactile
paving patterns. Escalator geometry and pitch. Tunnel ring spacing. Platform
edge treatment. Barrier forms and gate mechanisms. Bench and bin design
language. Station architecture generally. None of this is protected and copying
it closely is what will make the place feel real.

---

## 4. Advertising

All original, and it is one of the better lore channels available. Categories
from `06_ASSET_AND_LEVEL_GUIDE.md`: technology, banking, streaming, travel,
education, fashion, London events, fictional corporate campaigns.

The reference frame does this well: a film poster and a technology campaign,
both fictional, both with a faint wrongness that suggests something has
happened. Copy the approach, not the artwork.

Guidance:

- Two or three fictional companies recurring across the whole game does more
  than twenty one-offs.
- One in four adverts should carry lore. The rest should be mundane, otherwise
  the mundane ones stop reading as real.
- Period-anchor a few of them so the timeline is legible without exposition.
- No real company names, logos, or recognisable campaign artwork.

---

## 5. Lighting rules

Three layers, carried over from the web build where this was worked out
properly:

1. **Readability floor.** A low ambient contribution plus a subtle light
   carried with the player. Not physically motivated, and that is fine. A horde
   you cannot see is not frightening, it is unfair. Zombie silhouettes and the
   platform edge must always be legible.
2. **Station lighting.** Emissive strips, ceiling units, train interior spill.
   This gives the space its shape and carries the palette. This is what a
   blackout removes.
3. **Torch and muzzle flash.** Contrast and directed attention on top of the
   other two, never the only source of information.

A blackout should drop layer 2 entirely and layer 1 to roughly a fifth. It
should feel like losing the station, not like losing your eyes.

Expose master brightness as a setting. Acceptable levels vary enormously
between panels and asking a player to fight what they cannot see is not a
design decision.

Do not make the whole map red. Red means danger, an emergency system, or
narrative progression.

---

## 6. Gore

Restraint, per the style guide. High-value moments beat uniform coverage. Blood
concentrated in the immediate combat area and on the zombies themselves rather
than sprayed across every surface. Impact decals pooled and capped, oldest
recycled.

---

## 7. UI typeface

Overpass is the choice for `WBP_HUD` and all downstream UI: OFL licensed, US
highway heritage, reads as transit signage without being Johnston or a clone of
it. Its lowercase tittle is round, not the straight sided diamond that is
Johnston's signature, so it passes the test in `reference/rejected-assets.md`
on the letterforms rather than on the description.

**It is imported.** `Content/LastTrain/UI/Fonts/` holds Overpass, Overpass Mono,
Barlow, Barlow Condensed and Public Sans as `Font_` assets, each with its
`OFL.txt` beside it, all recorded in `Content/ATTRIBUTION.md`. Earlier revisions
of this section said Overpass was absent. That has not been true for some time.

`WBP_HUD` has not caught up. It still references
`/Engine/EngineFonts/DroidSansMono.DroidSansMono` for numeric readouts and
`/Engine/EngineFonts/Roboto.Roboto` for body text. Swapping those two
`FontObject` references to `Font_UI_OverpassMono` and `Font_UI_Overpass` is the
whole job and it belongs to `tasks/phase-g2-hud.md`, which already specs it.
Nothing else about the HUD layout depends on the typeface.

**Check before trusting the mono face.** Overpass Mono ships from Google Fonts
as a variable font whose default instance is Light 300, not Regular 400, and
Unreal's importer takes the default instance with no way to choose another. So
if `Font_UI_OverpassMono` was imported from `OverpassMono[wght].ttf` it is Light,
which is too thin to read at a glance off a HUD readout. Open the asset and look
at the stroke weight. If it is Light, cut proper static weights with
`tools/asset-fetch/make-font-instances.py` and re-import. Overpass itself
defaults to Regular 400 and is unaffected.

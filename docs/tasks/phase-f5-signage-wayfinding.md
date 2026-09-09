# F5: signage and wayfinding (finish and fix S11)

Lane: Opus in the editor. No `Source/` change. Depends on: F1.

Read `docs/reference/reference-frame-notes.md` section 3 (the "must not be
copied" table) and `docs/art-direction.md`.

## Context: S11 is half-done and broken

The S11 pass (commit `f635ece`) placed station signage on both maps, but the S12
Opus review (2026-09-09, in `handover.md` under "## S12") found:

- **MAJOR:** all six station name signs render their text edge-on (Pitch=90 text
  on Yaw=-90 panels) and 2 cm behind their backing panel. The station name is
  not readable in either map.
- **MAJOR:** the text colour is `#EEEBEB` off-white, not the sodium `#E0A030`
  the S11 commit message claimed.
- **MINOR:** the 26 pictogram textures are on the editor-only `TC_EditorIcon`
  texture group (~27 MB, will not cook right).
- **MINOR:** the greybox map has 6 pictogram actors, not the 9 its handover row
  documents.

## Goal

Signage that is readable in-game, correctly coloured, cooks properly, and is
entirely original (no roundel, no Johnston, no official line diagram).

## Scope

1. **Station-name boards.** Fix the text orientation so it faces the platform
   and sits in front of its panel. Colour: sodium `#E0A030` on charcoal
   `#16161C`, per the reference. Original panel geometry: a violet `#6C4C9C`
   horizontal bar over a charcoal field, name in the project's freely-licensed
   geometric sans (`Font_UI_Overpass` or `Font_UI_Barlow`, taller x-height, not
   a Johnston clone). No circle-and-bar roundel form. Text = the map's
   `StationDisplayNames` value ("Canary Wharf" / "Greybox Test"). One over the
   platform mid-span, two or three down the platform wall.
2. **Wayfinding signs.** "Way out", "To trains", platform-number panels: same
   information as the real thing, original panel geometry and colour split. Use
   the ISO 7010 pictogram textures already imported (`Content/LastTrain/UI/
   Pictograms/`), moved off `TC_EditorIcon` to a cookable group
   (`TEXTUREGROUP_World` or UI as appropriate). Unlit or lightly emissive so
   they read in the sodium gloom.
3. **The hanging departure board.** A ceiling-mounted board mid-platform, its
   face driven by the existing `ALTDepartureBoard` hooks (`OnCountdownChanged`
   etc: confirm names against `Source/LastTrain/Public/Train/LTDepartureBoard.h`).
   Two service rows with minutes and a clock, matching the reference. This is
   the train timer made diegetic: it must actually show the live countdown, not
   static text.
4. **Line identity.** Rename the line in-world on any sign that needs a line
   name. Station names stay factual. An original network schematic can appear as
   a wall map (it doubles as the Phase G HUD map substitute) but is not required
   for F5.

## Rules

- No gameplay actor moves (the departure board actor keeps its transform; only
  its face mesh/material changes).
- `MAP CHECK` console exec banned (see `editor-crash-endplaymap.md`). Use
  `MAP CHECKDEP NOCLEARLOG`.
- Never mutate anything while PIE is running.
- Update the S11 / S12 rows in `handover.md` to reflect the fix.

## Accept

- Play `L_CanaryWharf_Greybox`: the station name is legible from the platform,
  sodium on charcoal, original geometry.
- The hanging departure board shows the live train countdown ticking down, and
  the numbers match the `ALTTrain` state.
- Wayfinding pictograms are legible and on a cookable texture group.
- Nothing on any sign is a roundel, Johnston, or the official line diagram.
- Map Check 0/0. Gameplay actors unmoved. Committed via LFS.

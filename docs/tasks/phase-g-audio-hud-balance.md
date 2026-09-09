# Phase G: audio, restrained HUD, balance

Written 2026-09-09. Runs after Phase F (the art pass). Read
`docs/brief-v3-unreal.md` Part 3 phases 8 and 9, and
`docs/reference/reference-frame-notes.md` section 3 (HUD table).

Split: most of Phase G design and the C++ is editor-free and can be done by a
remote session; the widget assembly and audio placement need the editor.

## G1: audio (`phase-g1-audio.md` when written)

Ambience, train hum, original announcements, zombie vocals, weapon audio,
interaction stingers, round stingers. "Silence is a tool." Priority order from
the brief. Kenney CC0 audio is staged in `_incoming_assets/audio/`. Original
announcement phrasing only, no transcribed operator recordings, no operator
chimes.

**Accept:** you can tell what is happening behind you with your eyes closed.

## G2: the restrained HUD (`phase-g2-hud.md` when written)

Per the style guide summarised in `reference-frame-notes.md`: round, points,
health, perks, weapon, magazine, reserve, equipment, minimal prompts. The
station network schematic as the map substitute.

**Explicitly NOT the reference frame's HUD furniture:** no permanent minimap, no
challenge tracker, no kill feed, no exfil banner. The reference is an art target,
not a HUD target.

The current `WBP_HUD` has 5 minor findings from the S7 review still open (2
debug nodes to remove, stock fonts to swap to the imported OFL families, widgets
Visible at rest). Fold those into this task.

The train countdown stays diegetic (on the departure board, F5), NOT in the HUD.

**Accept:** you can play well using only what is on screen, and nothing on
screen is decoration.

## G3: balance (`phase-g3-balance.md` when written)

Balance to a first serious run reaching round 12 to 15, with 30 as an
achievement. Profile before optimising. Spawn fairness. Weapon feel. Tune across
both stations.

**Accept:** a competent first run dies around 12 to 15, not 5 and not 40.

## Not in Phase G

A third station (out of v1 scope: v1 ships 2 stations). Perks, the upgrade
bench and the lost property office are Phase E items that were deferred: if they
are still wanted for v1 they get their own specs, otherwise v1 ships without
them and the downed/auto-revive state (already in `ALTPlayerCharacter`) is the
only survival-extension mechanic.

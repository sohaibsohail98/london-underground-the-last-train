# Phase G: audio, restrained HUD, balance

Written 2026-09-09. Runs after Phase F (the art pass). Read
`docs/brief-v3-unreal.md` Part 3 phases 8 and 9, and
`docs/reference/reference-frame-notes.md` section 3 (HUD table).

Split: most of Phase G design and the C++ is editor-free and can be done by a
remote session; the widget assembly and audio placement need the editor.

## G1: audio (spec written: `phase-g1-audio.md`)

Ambience, train hum, original announcements, zombie vocals, weapon audio,
interaction stingers, round stingers. "Silence is a tool." Priority order from
the brief. Kenney CC0 audio is staged in `_incoming_assets/audio/`. Original
announcement phrasing only, no transcribed operator recordings, no operator
chimes.

The `Source/` half is **done**, unlike G2 which needs none: 17 optional
`USoundBase` properties on the weapon data asset, the zombie character and its
type asset, the train, the round manager, station heat and the interaction
component, all null by default and null checked at the call site, so the game
still runs silently with nothing assigned. Written with no engine available and
**not compiled**. `phase-g1-audio.md` carries the rest, which is all editor
work: which staged pack goes on which hook, the import and concurrency
settings a 40 strong horde needs, the assignment order, and the attribution
that has to travel with the two CC-BY assets.

**Accept:** you can tell what is happening behind you with your eyes closed.

## G2: the restrained HUD (spec written: `phase-g2-hud.md`)

Per the style guide summarised in `reference-frame-notes.md`: round, points,
health, perks, weapon, magazine, reserve, equipment, minimal prompts. The
station network schematic as the map substitute.

**Explicitly NOT the reference frame's HUD furniture:** no permanent minimap, no
challenge tracker, no kill feed, no exfil banner. The reference is an art target,
not a HUD target.

The current `WBP_HUD` has 5 minor findings from the S7 review still open (2
debug nodes to remove, stock fonts to swap to the imported OFL families, widgets
Visible at rest). Fold those into this task.

`phase-g2-hud.md` now carries the full spec: the binding table (every value the
HUD needs already exists and is Blueprint-visible, so G2 needs no `Source/`
change), the layout, per-element behaviour, the two new states (downed overlay,
run-over card), all 5 S7 findings, and the cut list. Two owner decisions are
flagged in it: the typeface split between the menu and the HUD, and whether the
station schematic belongs in the HUD at all (the spec recommends moving it to
F5 as diegetic platform signage, since v1's two stations give nothing to
navigate).

The train countdown stays diegetic (on the departure board, F5), NOT in the HUD.

**Accept:** you can play well using only what is on screen, and nothing on
screen is decoration.

## G3: balance (`phase-g3-balance.md` when written)

Balance to a first serious run reaching round 12 to 15, with 30 as an
achievement. Profile before optimising. Spawn fairness. Weapon feel. Tune across
both stations.

**Accept:** a competent first run dies around 12 to 15, not 5 and not 40.

## G4: the pause menu (spec written: `phase-g4-pause-menu.md`)

Added 2026-09-11. Escape pauses the run and opens a menu with resume, settings
and quit to the main menu. This closes the S13 follow-up that there is no route
back to `L_MainMenu` from a live run; G2's run-over card closes the other half
of it.

The `Source/` hook is already written: `PauseAction`, `TogglePause()`,
`RequestPause()`, `RequestResume()`, `CanPause()` and an `OnPauseStateChanged`
delegate on `ALTPlayerCharacter`. It is unverified and uncompiled, so compile
before the editor work. Everything else is `WBP_PauseMenu` and the input asset.
One open question is flagged in the spec: whether pause should be reachable
while the player is `Downed`.

**Accept:** Escape stops the run and gets you out of it, and does nothing at
all once the run is over.

## Not in Phase G

A third station (out of v1 scope: v1 ships 2 stations). Perks, the upgrade
bench and the lost property office are Phase E items that were deferred: if they
are still wanted for v1 they get their own specs, otherwise v1 ships without
them and the downed/auto-revive state (already in `ALTPlayerCharacter`) is the
only survival-extension mechanic.

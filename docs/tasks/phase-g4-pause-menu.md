# G4: the pause menu

Lane: CC-in-Unreal (editor only). The `Source/` half is already written on
branch `claude/pause-menu-prompt`: a `PauseAction` input action property, a
pause toggle and an `OnPauseStateChanged` delegate on `ALTPlayerCharacter`. It
was written by the remote session, which has no engine, so it is **unverified
and uncompiled**: run `./tools/ci/compile.sh` before starting the editor work.

Depends on: that compile passing. Soft depends on G2 (`phase-g2-hud.md`) for the
HUD visual language. G4 does not block G2, and if G2 has not landed yet, match
`WBP_MainMenu` instead and revisit the styling when G2 does.

Read the S13 row in `handover.md` (the main menu, and the follow-up that there
is no route back to it from a death or a pause), `phase-g2-hud.md` for the HUD
styling, and `docs/reference/reference-frame-notes.md` section 3 for what must
not be copied.

## Goal

Escape opens a pause menu that stops the run, shows the cursor, and offers
resume, settings and quit to the main menu. Nothing else. This closes S13's
follow-up (2) from the pause side; the death side is G2's run-over card.

## What `Source/` already gives you

All of it is on `ALTPlayerCharacter` and Blueprint-visible. Bind to these, do
not poll on Tick.

| What | Where it comes from | Kind |
|---|---|---|
| The pause input action slot | `PauseAction` (`EditDefaultsOnly`, null by default) | property |
| Pause or resume, whichever applies | `TogglePause()` | callable |
| Pause only | `RequestPause()` | callable |
| Resume only | `RequestResume()` | callable |
| Is a pause allowed right now | `CanPause()` | pure |
| Paused or resumed | `OnPauseStateChanged(bool bPaused)` | delegate |

`PauseAction` is bound on `ETriggerEvent::Started` to `TogglePause()` in
`SetupPlayerInputComponent`, exactly like `InteractAction` and the rest. It is
null until a Blueprint assigns it, which is deliberate and matches every other
action in this project: no `Source/` default to change.

What the C++ does on a pause: it checks `CanPause()`, calls
`UGameplayStatics::SetGamePaused`, sets `bShowMouseCursor` and then the input
mode (UI only when paused, game only when resumed, the same order
`BP_MenuGameMode` uses on the main menu), and broadcasts the delegate. It names
no widget and never will: the widget is yours.

`CanPause()` is false while `ELTRunState` is `Dead` or `Boarded`. Both of those
already own the whole screen, so the key is a deliberate no-op there rather than
a pause overlay on top of a run-over card or a departing train. `PreGame`,
`Active` and `Downed` all pause. See the open question below about `Downed`.

## The editor work

### 1. The input asset

- New `IA_Pause` `UInputAction` next to the existing ones in
  `Content/LastTrain/Input/`, value type Digital (bool).
- **Tick `Trigger When Paused` on the asset.** Without it the key pauses the
  game and then cannot unpause it, because a paused world does not evaluate the
  action. This is the one setting that will waste an afternoon if it is missed.
- Map it to Escape in `IMC_Default`, the existing mapping context. Escape is
  also the editor's stop-PIE key, so test it in a standalone or a new editor
  window as well as in PIE, and consider mapping Gamepad Special Right
  alongside it.
- Assign it to the player Blueprint's `PauseAction`.

### 2. `WBP_PauseMenu`

Same visual language as `WBP_MainMenu` and whatever G2 has settled: charcoal
`#16161C` ground, violet `#6C4C9C` on the player's own controls, sodium
`#E0A030` for attention, crimson `#B02030` for loss only. No roundel, no
Johnston, nothing from the reference frame's HUD furniture.

- A charcoal wash over the frozen game at roughly 70 per cent, not an opaque
  screen. Seeing the room you paused in is the point.
- `PAUSED` as a heading, and the station name from
  `ALTGameState::GetStationName()` under it, small and muted.
- Three stacked buttons in the `WBP_MainMenu` style (idle dark, hover violet
  fill, pressed brighter violet, sodium outline on hover and press):
  `ResumeButton` "RESUME", `SettingsButton` "SETTINGS", `QuitButton`
  "QUIT TO MAIN MENU".
- `IsFocusable` on all three, `DesiredFocus = ResumeButton`, so a pad or the
  keyboard can drive it. The C++ sets UI only input mode but names no focus
  widget, so the widget must claim focus itself.
- Add it to the viewport with a Z order above `WBP_HUD`.

### 3. Wiring

- The player Blueprint binds `OnPauseStateChanged`. On true: create the widget
  (or reuse a stored reference), add to viewport, claim focus. On false: remove
  it from the parent. Do not create it on `BeginPlay` and leave it collapsed;
  a menu that exists for the whole run is a menu that can be left behind by a
  level travel.
- `ResumeButton` calls `RequestResume()` on the player character. Do not call
  `Set Game Paused` directly from the widget: the cursor, the input mode and
  the delegate all go through the C++, and a direct unpause would leave the
  cursor up and the input mode on UI only.
- `SettingsButton`: there is no settings screen in this project yet, not in
  `WBP_MainMenu` and not in `phase-f8-menu-polish.md`. **Recommend shipping the
  button disabled** with a muted label until a settings screen exists, or
  cutting it and leaving the slot in the layout. A button that does nothing is
  worse than a button that is not there. Owner decision, flagged.
- `QuitButton`: call `RequestResume()` **first**, then `Open Level (by Name)`
  `L_MainMenu`. Resuming first costs nothing and does not depend on the new
  world clearing the pauser and the old controller's input mode for you. This
  is `WBP_MainMenu`'s own START RUN button in reverse, per S13.

### 4. Not in G4

- No save, no restart-run button (restart is quit to menu then START RUN, and
  a second route to the same place is clutter while v1 has two stations).
- No settings screen itself. If one is wanted it gets its own spec and serves
  both menus.
- No `Source/` change beyond the hook already written. If the widget wants
  something the table above does not give it, say so rather than adding a C++
  field the remote session cannot compile.

## Open question: should pause be reachable while `Downed`?

Left open deliberately. The C++ currently allows it, because refusing would
have been a silent design decision made in a `Source/`-only branch.

**The tension.** The bleed-out clock is 30 seconds and the solo auto-revive
lands at 8. `BleedOutRemaining` is decremented in `ALTPlayerCharacter::Tick`,
so a world pause stops that clock along with everything else. Argue it either
way:

- **Against allowing it.** A bleed-out is the tensest moment in the game and
  the one place where the player wanting time is exactly the thing the design
  is denying them. Pausing an active death clock to think, or to walk away and
  come back, is a stall that removes a stake the round loop is built on.
- **For allowing it.** The pause freezes the horde too, so nothing is gained
  except time away from the keyboard. This is a single player game with no
  leaderboard, so the only person the stall costs is the person doing it.
  Refusing the key mid-bleed-out is also the worst moment to tell a player
  their menu is unavailable, and the alarming version of that bug (a pause menu
  that opens while the clock keeps running) is not possible here.

**Recommendation: allow it, which is what the code does now.** The exploit
argument only bites when someone else is waiting, and nobody is in v1. Revisit
if co-op ever lands, because a co-op pause cannot freeze the world anyway and
the whole question changes shape.

If the owner or CC-in-Unreal wants it refused, the change is one line: add
`ELTRunState::Downed` to the states `ALTPlayerCharacter::CanPause()` rejects.
Do not filter it in the widget: a key that visibly opens nothing reads as a
bug.

## Accept

- Escape during a live round: the world stops, the cursor appears, the menu
  reads clearly over the frozen station.
- Escape again, or RESUME: the world runs, the cursor is gone, mouse look
  works immediately and the weapon is not stuck firing from a held trigger.
- QUIT TO MAIN MENU: `L_MainMenu` loads, unpaused, cursor up, START RUN works.
- Escape after a death, on the run-over card: nothing happens, no overlay, no
  frozen screen.
- Escape during the boarding travel delay: nothing happens, the train leaves.
- Escape while downed: behaves as the open question above is settled. Record
  which way it went in this file and in `handover.md`.
- Palette check against the four hex values. No engine stock font left in the
  widget.
- Changed `.uasset` files committed via LFS (verify each is a pointer:
  `git show HEAD:<path> | head -c 45` gives `version https://git-lfs...`).

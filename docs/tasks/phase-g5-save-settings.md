# G5: the settings panel and the best round line

Lane: CC-in-Unreal. Small, bounded, no `Source/` change needed unless you take
one of the optional items at the bottom. Depends on: the C++ that landed with
this file, and on `L_MainMenu` from the S13 row in `handover.md`.

## What already exists, so you do not rebuild it

Two `USaveGame` classes and the game instance wiring are in C++ and need no
editor work at all.

- `ULTSaveGame` (`Core/LTSaveGame.h`), slot `LastTrainProgress`. A
  `TMap<FName, FLTStationRecord>` keyed by station map asset name, exactly as
  `ULTGameInstance::VisitedStations` and `ALTGameMode::StationRoutes` are keyed.
  Each record holds `BestRound`, `RunsStarted` and `RunsBoarded`.
- `ULTSettingsSaveGame` (`Core/LTSettingsSaveGame.h`), slot
  `LastTrainSettings`. Holds one `FLTGameSettings`: `MasterVolume` (0 to 1),
  `FieldOfView` (clamped 70 to 120, default 95 to match the coded
  `BaseFieldOfView`), and `bSubtitlesEnabled`.
- `ULTGameInstance` loads both on `Init`, applies the settings on every map
  load, and writes progression at the end of every run. The game mode calls
  `RecordRunStarted` on a cold start and `RecordRunEnded` on a death and on a
  boarding.

`bSubtitlesEnabled` is a seam, not a feature: the subtitle system is being
built on its own branch and this is the single bool it expects to find in the
settings block, so the two meet without a schema change on either side. Nothing
reads it yet. Leave the checkbox out of the panel until that lands, or ship it
disabled, whichever the owner prefers.

### The functions a widget calls

All on `ULTGameInstance`, all `BlueprintCallable` or `BlueprintPure`, so a
widget gets at them with Get Game Instance, Cast To `LTGameInstance`.

| Function | Use |
|---|---|
| `GetGameSettings()` | Fill the panel's controls when it opens. |
| `ApplyAndSaveSettings(NewSettings)` | The one call the APPLY button makes. Clamps, applies, writes. |
| `ApplySettingsNow()` | Apply without writing, for a live preview while a slider drags. |
| `SaveKeyBindings()` | Only if you build a remapping row. See the Enhanced Input note. |
| `GetBestRoundAnywhere()` | The single number for a main menu line. |
| `GetBestRoundForStation(MapName)` | Per station, if a station picker ever wants it. |
| `GetProgress()` | The whole save, for a stats panel later. Read only in practice. |
| `OnSettingsApplied` | Blueprint assignable. Bind it to push master volume at a real sound mix once G1 audio has one. |

## The work

1. **`WBP_Settings`.** A panel in the S13 visual language: charcoal ground,
   violet accents, sodium for values, the same three UI faces `WBP_MainMenu`
   uses. Three controls, matching the three fields and no more:
   - master volume slider, 0 to 1, showing a percentage;
   - field of view slider, 70 to 120, showing degrees;
   - subtitles checkbox, per the note above.

   On Construct, read `GetGameSettings()` and set the controls from it. On
   APPLY, Make `FLTGameSettings` from the controls and call
   `ApplyAndSaveSettings`. On BACK, close without applying. A live preview on
   slider release through `ApplySettingsNow()` is a nice touch and costs one
   node; it is optional.

2. **Reach it from the main menu.** A SETTINGS button on `WBP_MainMenu`
   between START RUN and QUIT, same button style, same focus treatment.

3. **Reach it from the pause menu, when there is one.** Checked on
   2026-09-11: `docs/tasks/phase-g4-pause-menu.md` does not exist and no pause
   menu is built, so this step waits. When the pause menu lands, it opens the
   same `WBP_Settings` rather than a second copy of it. Field of view applies
   live mid-run, because `ApplyAndSaveSettings` pushes it straight at the pawn
   through `ALTPlayerCharacter::SetBaseFieldOfView`.

4. **Show the best round somewhere.** A suggestion, not a mandate: a quiet line
   under the LAST TRAIN subtitle on `WBP_MainMenu` reading the number from
   `GetBestRoundAnywhere()`, hidden when it is 0 so a first run is not greeted
   by "BEST ROUND 0". The main menu is the obvious spot because it is the only
   screen that exists outside a run; the run over card in `phase-g2-hud.md`
   is the other candidate and would read better there if that card gets built
   first.

## The Enhanced Input note, read before building a remapping row

Key rebinding is **not** in `ULTSettingsSaveGame` and deliberately so.
`UEnhancedInputUserSettings` is itself a `USaveGame` with its own slot, so
storing a key map in ours as well would give the player two files that can
disagree. `ULTGameInstance::SaveKeyBindings()` is a thin wrapper over the
engine's own save.

It returns false and logs why until two editor steps are done, neither of which
C++ can do for itself:

1. Tick **Enable User Settings** in Project Settings, Engine, Enhanced Input.
   Until then `GetUserSettings()` returns null and the wrapper is inert. The
   rest of the settings panel is unaffected.
2. Give each mapping in `IMC_Default` a **Player Mappable Key Settings** entry
   with a name and a display name, for each of the eight actions. A mapping
   without one cannot be rebound and will not appear in a mapping row.

Only then is a remapping row worth building, and it should use the engine's own
`UEnhancedInputUserSettings` mapping calls, not a hand rolled key list.

## Accept

- Launch the game, open SETTINGS from the main menu, drag both sliders, APPLY,
  quit the game entirely, relaunch: the panel opens on the values you set.
- Start a run with the field of view changed, and the camera is at the chosen
  angle from the first frame, with aiming still narrowing to the weapon's
  `AimedFieldOfView`.
- Play a run, die on round 3, quit, relaunch: the best round line reads 3. Play
  again, die on round 2, and it still reads 3.
- Board a train at Canary Wharf and `RunsBoarded` for that map is 1. Travel
  back and forth within the same run, and it is still 1.

## Not in this task

- A profile picker, a second save slot, or a delete save button. One profile,
  one slot each, and deleting anything stays a manual step per `CLAUDE.md`.
- Look sensitivity. `ALTPlayerCharacter::Look` passes the input axis straight to
  `AddControllerYawInput` with no multiplier, so there is no property to
  persist. Adding one is a small `Source/` change and a fourth slider, and it
  should be its own task rather than smuggled in here.
- A graphics quality preset. `brief-v2.md` phase 7 asks for one; nothing in the
  project exposes a quality setting yet, and `UGameUserSettings` is a separate
  decision.
- Oyster Credit and any other meta progression. Still unscoped in
  `../design/gameplay-canon.md` section 8, and this save deliberately does not
  invent it.

## Status of the C++ this depends on

Written without an engine, so **unverified: it has never been compiled**. See
`../known-issues.md` 1.3 and 2.14. Compile with `./tools/ci/compile.sh` before
building any of the above on top of it.

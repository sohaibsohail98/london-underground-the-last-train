# F8: menu scene polish

Lane: Opus in the editor. No `Source/` change. Depends on: F1 (kit pieces), F2
(lighting look). Small task, do it after F2.

Read the S13 row in `handover.md`, `docs/reference/reference-frame-notes.md`, and
the menu-audio note in `docs/reference/asset-sources-phase-f.md`.

## Context

S13 shipped a working main menu (`L_MainMenu`, `BP_MenuGameMode`, `WBP_MainMenu`)
over a bare dark box: a charcoal cube, one violet spot, one sodium fill, one
camera. The UI is fine. The scene behind it is a placeholder. This task makes the
menu read as a London Underground game at a glance.

## Scope

1. **Dress the menu scene with F1 kit pieces.** A shallow diorama in front of
   `MM_MenuCamera`: a tiled wall section, a platform-edge run, a pillar, a bench,
   the nose or a slice of the F3 train if it is ready, a hanging sign silhouette.
   Not a playable space, just what the camera sees. Keep it cheap: this is a
   menu, it should load instantly.
2. **Light it to the F2 look.** Sodium key, deep shadow, one violet accent on the
   name-bar wall or a piece of furniture, a hint of the crimson tunnel gradient
   if a tunnel mouth is in frame. Reuse the F2 post process volume settings.
3. **Optional, if quick:** a very slow camera drift or a parallax so the menu is
   not a frozen still. Keep it under a few degrees, slow, looping. Respect a
   reduced-motion sensibility, keep it subtle.
4. **Menu ambience.** A synthesised CC0 drone plus a room tone, per the audio
   note in `asset-sources-phase-f.md` (a synth drone provably cannot contain a
   hidden operator announcement, a field recording cannot make that claim). Low,
   looping, quiet. Add it as a Sound actor in `L_MainMenu` or trigger it from
   `BP_MenuGameMode` BeginPlay.
5. **Wire the SELECT STATION sub-panel** that S13 skipped: a small panel with
   "Canary Wharf" and "Greybox Test", each an `OpenLevel` to the matching map.
   Only if it does not balloon the task.

## Rules

- The menu still has no nav, no round manager, no spawns, no `ALTGameMode`.
- `MAP CHECK` console exec is banned (see `editor-crash-endplaymap.md`). Use
  `MAP CHECKDEP NOCLEARLOG`.
- Never mutate anything while PIE is running.

## Accept

- Launch the game: the menu reads as an underground station, not a black void.
  The UI is still legible over it.
- START RUN still opens `L_CanaryWharf_Greybox` with the run going Active.
- Ambience loops quietly and is demonstrably synthetic, not a recording.
- Map Check 0/0 (bar the stock needs-lighting-rebuilt line).
- Committed via LFS, `handover.md` S13 row updated.

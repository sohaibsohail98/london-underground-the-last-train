# Asset research and gathering for Phase F

Lane: remote Claude Code session (no editor needed), spinning up research
subagents. Output goes to `_incoming_assets/` (gitignored staging), triaged into
`Content/LastTrain/` later by the editor session.

This repeats the model of the earlier haul: see `_incoming_assets/ASSET-RESEARCH.md`
for what is already downloaded (21 PBR surface sets, 5 OFL font families, 26 ISO
7010 pictograms, 2 HDRIs, Quaternius citykit props, CC0 weapon meshes, Kenney
CC0 audio, Quaternius characters).

## Hard constraints (same as the last run, non negotiable)

- **No project modification. No git. No `Source/`.** Downloads land in
  `_incoming_assets/` only.
- **Licences:** CC0, OFL, public domain, or Unreal-compatible only. Record the
  licence and source URL for every file in a `SOURCES.txt` beside it. No
  login-gated, account-gated, or licence-click downloads. No captcha solving. No
  auth.
- **Legal, from `CLAUDE.md`:** nothing carrying the roundel, Johnston / New
  Johnston, the official line diagram, operator livery or logo, or transcribed
  operator announcement recordings. The Freesound "CC0" tube-chime files already
  flagged in the last haul are real operator recordings: do not use, do not
  re-download.
- Keep downloads modest: 2K textures over 8K, a few reference meshes not whole
  bundles.

## What Phase F actually needs

### For the main menu (S13) - small, may not need new downloads

The menu should read as a London Underground game at a glance. Check
`_incoming_assets/` and `/Content/SubwayTrain/` first; only research if there is
a real gap.

- A background: either a still (a moody platform / train-nose / tiled-wall
  render or photo, CC0) or a few meshes to build a shallow menu diorama (tiled
  wall panel, a train nose, a bench, a hanging sign silhouette).
- An ambient loop for the menu: low station hum / distant train. Kenney CC0
  audio is already staged; a CC0 room-tone or industrial-hum loop from
  freesound CC0 or similar is fine if it is clearly not an operator recording.
- The title fonts are already imported (Overpass, Barlow). No new fonts.

### For F1 (modular kit)

- Reference photos (not assets, just reference): main-line station hall
  interiors, tiled platform corridors, tunnel-mouth portals, coffered concrete
  ceilings, platform-edge treatments. For the modeller to work from.
- CC0 trim-sheet textures: dirty subway tile, painted concrete, brushed steel,
  rubber tactile paving. ambientCG and Poly Haven are the known-good CC0
  sources; some may already be in `_incoming_assets/surfaces/`.
- Optional: a CC0 modular subway/station kit if a good one exists (Quaternius,
  Kenney, or similar). Not a paid marketplace pack.

### For F3 (train exterior)

- Reference photos of a Class 345 "Aventra" or similar modern EMU: the smiling
  cab front, the flush plug doors, the deep skirt, the flat roof with AC pods,
  the continuous window band. Shape reference only.
- Optional: a CC0 modern-EMU or generic-passenger-train mesh with a
  rounded-rectangle box profile, to kitbash from. Must be CC0. A tube-train
  shape is wrong (too round, too small); a suburban/main-line EMU shape is
  right.

### For F6 (zombie bodies)

- `/Content/CitySample/` is the intended route and is already available locally.
  No research needed unless that route is abandoned for the MetaHuman fallback,
  which also needs no downloads.

## Output

- Everything under `_incoming_assets/<category>/` with a `SOURCES.txt`.
- Append a section to `_incoming_assets/ASSET-RESEARCH.md` listing what was added,
  the licence of each, and which Phase F task it feeds.
- Do not import anything into the project. The editor session does the triage.

## Done, with one change of shape (2026-09-09)

The research ran. The **downloading could not**, and would not have helped if it
had: the remote container's egress proxy refuses every asset host, and
`_incoming_assets/` is gitignored and dies with the container, so a file staged
there has no route to this machine.

So the output landed as the half that survives, which is the research itself:

- **`docs/reference/asset-sources-phase-f.md`** is the shopping list. Sources,
  asset IDs, licences, what each feeds, what to inspect before staging, and what
  the research could not settle. Every entry rests on search evidence only: no
  page was opened and no file was looked at.
- **`tools/asset-fetch/fetch-phase-f.sh`** turns it back into
  `_incoming_assets/<category>/` with `SOURCES.txt` per category, on a machine
  that can reach the hosts. It fetches the ambientCG and Poly Haven sets through
  their keyless APIs and prints a manual checklist for everything needing a
  human click. It imports nothing and runs no git.

**The finding that changes Phase F planning: there is no CC0 modular station
kit.** The shell, platform edge, escalator, gate line and tunnel portal are
in-house modelling. What the CC0 world gives free is surfacing, small props and
proportion reference. F1's budget should assume that.

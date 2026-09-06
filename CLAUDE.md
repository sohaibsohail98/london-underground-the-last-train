# LAST TRAIN

First person round based zombie survival on a fictionalised London Underground
line, built in Unreal Engine 5. A station is the arena. A train arrives on a
timer and dwells; boarding it is an optional escape. Staying raises the station's
heat.

The visual target is `docs/reference/reference-frame.png`. Keep it in mind while
building. It is a Phase F art target, not something to measure the grey box
against. Notes on what to lift and what is off limits are in
`docs/reference/reference-frame-notes.md`.

## Engine and build

- **Unreal Engine 5.8.** `LastTrain.uproject` `EngineAssociation` is `5.8`.
  `DefaultBuildSettings` is `V7` and `IncludeOrderVersion` is `Unreal5_8` in both
  target files.
- **Xcode lives on an external drive:** `/Volumes/DriveSohaib/Applications/Xcode.app`.
  It must be mounted to compile C++ or shaders. `xcode-select -p` should point
  inside it. The Metal toolchain is installed
  (`xcodebuild -downloadComponent MetalToolchain`).
- Build the editor target from the engine's batch file:
  `"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"`
- **There is no CI compile.** The engine is not available in CI. Compile locally
  after every C++ change and keep changes small.

## Module layout

`Source/LastTrain/` is the one C++ module. `LT` prefix on every type.

| Area | Files | State |
|---|---|---|
| Player | `Player/LTPlayerCharacter` | FP pawn, camera, health with 4s delay regen, ADS FOV lerp, sprint cancels aim. Enhanced Input actions are `EditDefaultsOnly` and null until a Blueprint assigns them. |
| Weapons | `Weapons/LTWeaponComponent`, `Weapons/LTWeaponData` | Hitscan, hip and ADS spread, movement and recoil bloom, pellets, penetration, falloff, reload, refill. Traces on `ECC_GameTraceChannel1`. |
| Zombies | `Zombies/LTZombieCharacter` | Health, `ApplyRoundScaling`, head bone hitbox, attack via `ApplyDamage`, death broadcast. Navigation is `AIController::MoveToActor` on a jittered repath cadence (B1), no behaviour tree, plus a stall-recovery nudge for queued attackers. |
| Rounds | `Rounds/LTRoundManager`, `Rounds/LTSpawnPoint`, `Rounds/LTStationHeat` | Wave counts, `MaximumAlive` cap, breather, decaying spawn interval, weighted spawn point choice. Station heat widens the live cap and quickens spawns; the round manager reads it if present. |
| Economy | `Economy/LTPointsComponent` | 500 start, 10 hit, 60 kill, 130 headshot kill. `TrySpend`, `CanAfford`. |
| Interaction | `Interaction/LTInteractableInterface`, `Interaction/LTInteractionComponent`, `Interaction/LTWallBuy` | Interface, a tracing component that holds the current interactable and drives the prompt delegate, and `ALTWallBuy` as the first implementer (weapon then ammunition). |
| Core | `Core/LTGameMode`, `Core/LTGameState` | Run lifecycle for one station arena. `ELTRunState` (PreGame, Active, Downed, Dead, Boarded); the game mode flips it and starts the round manager. Travel and boarding are Phase C. |
| Module | `LastTrain.h/.cpp` in `Private/`, NOT the module root. `LT_LOG` macro lives here. |

`Content/LastTrain/` now holds the Phase A/B editor assets: the grey box maps
(`L_GreyboxTest`, `L_CanaryWharf_Greybox`), the `BP_` Blueprints, the Enhanced
Input assets, `DA_Weapon_SMG` and `WBP_HUD`. The C++ is otherwise still a chassis:
no train, no zombie types, no perks, no travel. `web/` is the discarded Three.js
build, tagged `phase-03`, not part of this work.

## Conventions, enforced by `tools/ci/`

- **British spelling** everywhere, including comments, user facing strings and
  the docs. The checker rejects organiz*, color, behavior, customiz*.
- **Never use em or en dashes** in source or in `docs/`, `CLAUDE.md`,
  `README.md`. Plain punctuation only. Use a spaced hyphen, a comma or a colon.
- Headers begin with `#pragma once`. `X.generated.h` is the last include.
- `TObjectPtr` in containers, never raw `UObject*`.
- `LT_LOG(Verbosity, TEXT("..."))`, never `UE_LOG(LogTemp, ...)`.
- No `TODO`, `FIXME`, `HACK`, `XXX` markers. Finish it or open an issue.
- clang-format 20 (`.clang-format`), tab indent for `.h/.cpp/.cs`.
- `.uasset` and `.umap` are Git LFS. Never commit one as raw binary.
- CI (`.github/workflows/ci.yml`) runs four gates on every push and PR:
  `check_hygiene.py` (secrets, absolute paths, trademark leakage),
  `check_cpp_conventions.py` plus clang-format (source style),
  `check_docs.py` (British spelling, no dashes, JSON validity, dead links),
  `check_content.py` (LFS pointer integrity). The engine is not in CI, so
  C++ is compiled locally after every change.

## Legal, non negotiable

Station names and geography are factual and fine. No roundel. No Johnston or New
Johnston typeface. No reproduction of the official line diagram. No operator
livery or logo. No transcribed announcement recordings. No Call of Duty weapon,
perk or asset names. All wayfinding, advertising and rolling stock livery is
original work. Palette: `#16161C` charcoal, `#6C4C9C` violet, `#E0A030` sodium,
`#B02030` crimson.

## Design documents

- `docs/brief-v3-unreal.md` - current brief: engine, camera, phases, model split.
- `docs/brief-v2.md` - superseded for engine, still authoritative for the round
  loop, train timing (100s interval, 25s dwell), station heat, the five zombie
  types, the mechanic library and the economy numbers.
- `docs/design/gameplay-canon.md` - the settled design distilled tight: round
  loop, run lifecycle, train, heat, zombie roster, economy, HUD, with coded
  values marked authoritative and provisional values marked as such. Replaced
  the 167-item open-questions sweep, which is in git history at commit `0b7e35f`.
- `docs/art-direction.md` - palette, composition, trademark substitutions.
- `docs/unreal-setup.md` - the editor steps the C++ cannot do for itself.
- `docs/tasks/` - one bounded task spec per file. Point a fresh session at the
  relevant one rather than re typing the spec. `docs/tasks/NEXT.md` is the
  resume point.

## Working rules for models

- One bounded task per request. State: engine version (5.8), exact files,
  desired behaviour, constraints, acceptance test. Never ask for a whole phase.
- Match the surrounding code's conventions exactly.
- **Opus** for C++ against the existing convention, data assets, Blueprint setup
  instructions, layout sketches, balance numbers, docs. **Fable** for Lumen and
  post process tuning, material graphs, animation blueprint blend logic, crowd
  performance once there is a profile, and any bug surviving two Opus attempts.
  **Sonnet** for bulk data entry against a fixed schema.
- Compile after every C++ change. A small verified change beats a large
  unverified one.

## Phase plan

See `docs/tasks/README.md` for the current plan and where each task stands.
Phase A is foundation and the first playable grey box. Nothing past a gate
starts until that gate passes.

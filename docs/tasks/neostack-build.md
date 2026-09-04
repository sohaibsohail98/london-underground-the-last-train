# NeoStack build brief for the Unreal editor assets

This file is for a NeoStack agent driving the Unreal editor through
`execute_script`. It is the project specific brief: which assets to create, from
which C++ classes, with what values, and the constraints. The generic Lua API is
already in the NeoStack skills (`neostack-blueprint`, `neostack-level-design`,
`neostack-umg-widget`, `neostack-widget`, `neostack-umg-design`) which auto load,
so this file does not repeat them.

The human readable per phase specs stay the source of truth:
`phase-a4-editor-setup.md`, `phase-a5-acceptance.md`, `phase-b2-interaction.md`,
`phase-b3-feedback-widgets.md`. Where this file and those differ, those win and
you should stop and flag it.

## Ground rules, every task

1. **Engine is Unreal 5.8.** External Xcode on `/Volumes/DriveSohaib`.
2. **Never edit anything under `Source/`.** The C++ is complete and compiles. If
   a task seems to need a C++ change, stop and report it. Do not touch the
   `.Build.cs`, the target files, or any `.h`/`.cpp`.
3. **Never run git.** No commits, no branches, no staging. A human reviews and
   commits.
4. **British spelling in every user facing string**, including widget text and
   asset display names. The CI checker rejects organiz*, color, behavior,
   customiz*. "Ammunition", never "Ammo".
5. **No em or en dashes** anywhere you author text.
6. **Legal, non negotiable.** No roundel. No Johnston or New Johnston typeface or
   anything that reads as a clone. No reproduction of the official line diagram.
   No operator livery or logo. No Call of Duty weapon, perk or asset names.
   Original names only. Palette for any UI or material: `#16161C` charcoal,
   `#6C4C9C` violet, `#E0A030` sodium, `#B02030` crimson.
7. **All game content lives under `/Game/LastTrain/`** in the sub folders each
   task names. Not loose in `/Game/`.
8. **Verify in a fresh script.** A non nil mutation result is not proof. Re open
   the asset or level in a new `execute_script` and read the state back.
9. **When a task finishes:** save all, run Play In Editor yourself, and report
   back every asset created (path and type), anything you did differently from
   the spec and why, and the PIE result. Then stop. Do not roll straight into
   the next phase.

## What NeoStack cannot do, hand these to a human

- **Custom trace and object channels.** Project Settings, Engine, Collision is
  special cased native UI, not reflected properties. `write_config` reports
  success and changes nothing. The `Weapon` trace channel was created by hand and
  is already in slot 1 (`ECC_GameTraceChannel1`, response Ignore). If a future
  task needs another channel, stop and ask.
- **Other Project Settings pages that are bespoke UI** rather than a plain
  settings object. If a config write silently no ops, do not hand edit the
  `.ini` and do not touch C++ constants. Flag it.
- **Editor restarts.** If you were reconnected after an editor restart and
  `execute_script` is missing from your tool list, a human has to start a fresh
  NeoStack chat in this workspace. Tools attach at chat creation.

## C++ classes you will parent Blueprints to

All in the `LastTrain` module, `LT` prefix. Parent a Blueprint by the class
name, for example `ParentClass = "LTPlayerCharacter"`.

| Blueprint | C++ parent | Key properties you set |
|---|---|---|
| `BP_PlayerCharacter` | `LTPlayerCharacter` | Input category: `InputMapping`, `MoveAction`, `LookAction`, `JumpAction`, `SprintAction`, `FireAction`, `AimAction`, `ReloadAction`, `InteractAction`. Weapon component: `WeaponData`. |
| `BP_Zombie` | `LTZombieCharacter` | Mesh component: skeletal mesh + anim class. Do not edit `HeadBoneNames`, `RepathIntervalSeconds`, `RepathJitterFraction` or any Combat value. |
| `BP_RoundManager` | `LTRoundManager` | `ZombieClass` only. Everything else default. |
| `BP_GameMode` | `GameModeBase` | `DefaultPawnClass = BP_PlayerCharacter`. |

Existing components on `LTPlayerCharacter`, already created in C++, nothing to
add: `Camera`, `ViewModel` (leave mesh empty), `Weapon` (`ULTWeaponComponent`),
`Points` (`ULTPointsComponent`), `Interaction` (`ULTInteractionComponent`).

Data asset class: `LTWeaponData` (a `UPrimaryDataAsset`). Interactable actor
class already in C++: `ALTWallBuy` (implements `ILTInteractableInterface`), with
`Plate` static mesh component and properties `Weapon`, `WeaponCost` (500),
`AmmunitionCost` (250).

Spawn point actor class: `LTSpawnPoint`, properties `AreaTag`, `Weight`,
`FirstRound`, `CooldownSeconds`, `bEnabled`.

## Delegates that already exist and already fire

The B3 HUD binds to these. If any plan proposes new C++ for feedback, it has not
read the headers.

| Signal | Delegate | Owner |
|---|---|---|
| Hit and headshot | `OnHitConfirmed(bool bHeadshot)` | `ULTWeaponComponent` |
| Ammunition | `OnAmmoChanged(int32 Magazine, int32 Reserve)` | `ULTWeaponComponent` |
| Current spread, aim blend | `GetCurrentSpreadDegrees()`, `GetAimAlpha()` | `ULTWeaponComponent` |
| Points | `OnPointsChanged(int32 NewTotal, int32 Delta)` | `ULTPointsComponent` |
| Health | `OnHealthChanged(float HealthFraction)` | `ALTPlayerCharacter` |
| Damage taken | `OnDamageTaken(float Fraction)` BlueprintImplementableEvent | `ALTPlayerCharacter` |
| Interaction prompt | `OnInteractableChanged(const FText& Prompt, bool bAvailable)` | `ULTInteractionComponent` |
| Round start and end | `OnRoundStarted(int32 Round)`, `OnRoundEnded(int32 Round)` | `ALTRoundManager` |
| Zombie died | `OnZombieDied(ALTZombieCharacter*, bool bHeadshot)` | `ALTZombieCharacter` |

---

# Phase A4, the grey box map

Full spec: `phase-a4-editor-setup.md`. Step 1, the `Weapon` trace channel, is
done by hand. Build steps 2 to 5 end to end.

## 1. Enhanced Input, under `/Game/LastTrain/Input/`

Input Actions:

| Asset | Value type |
|---|---|
| `IA_Move` | Axis2D (Vector2D) |
| `IA_Look` | Axis2D (Vector2D) |
| `IA_Jump` | Digital (bool) |
| `IA_Sprint` | Digital (bool) |
| `IA_Fire` | Digital (bool) |
| `IA_Aim` | Digital (bool) |
| `IA_Reload` | Digital (bool) |
| `IA_Interact` | Digital (bool) |

`IMC_Default` Input Mapping Context, mappings:

| Action | Key | Modifiers |
|---|---|---|
| `IA_Move` | W, A, S, D | Standard WASD 2D axis setup: Swizzle on the vertical keys, Negate on A and S. Use the built in 2D axis preset if the API exposes one. |
| `IA_Look` | Mouse XY 2D Axis | Negate on the Y axis only |
| `IA_Jump` | Space Bar | none |
| `IA_Sprint` | Left Shift | none |
| `IA_Fire` | Left Mouse Button | none |
| `IA_Aim` | Right Mouse Button | none |
| `IA_Reload` | R | none |
| `IA_Interact` | E | none |

## 2. `DA_Weapon_SMG`, under `/Game/LastTrain/Data/`

An `LTWeaponData` data asset. Leave every numeric field at its class default,
they are the intended balance. Set only:

- `DisplayName`: an original, in world name that fits a fictionalised London
  Underground setting. Not a real firearm model. Not a weapon name from another
  game. British spelling.
- `UpgradedName`: optional, an original upgraded variant name, or leave blank.

Leave `Mesh`, `FireSound`, `MuzzleFlash` empty.

## 3. Blueprints, under `/Game/LastTrain/Blueprints/`

- **`BP_PlayerCharacter`**, parent `LTPlayerCharacter`. Assign the nine Input
  slots from the table above. On the `Weapon` component set `WeaponData` to
  `DA_Weapon_SMG`. Leave `ViewModel` mesh empty. Compile, save.
- **`BP_Zombie`**, parent `LTZombieCharacter`. On the Mesh component set the
  skeletal mesh to the fifth generation mannequin (`SKM_Manny` or `SKM_Quinn`)
  and the anim class to its third person locomotion Animation Blueprint
  (`ABP_Manny` or `ABP_Quinn`). If the mannequin content is not in the project,
  add it. Mesh transform: location Z about -90, rotation Z -90. Do not edit any
  Combat property. Verify, do not set, that the Mesh blocks the `Weapon` channel
  and the Capsule ignores it, this is set in the C++ constructor.
- **`BP_RoundManager`**, parent `LTRoundManager`. Set `ZombieClass` to
  `BP_Zombie`. Nothing else.
- **`BP_GameMode`**, parent `GameModeBase`. `DefaultPawnClass = BP_PlayerCharacter`.

## 4. `L_GreyboxTest`, under `/Game/LastTrain/Maps/`

- Floor and four walls from Cube static meshes, roughly 30 m by 15 m, one closed
  short side so the platform composition is suggested. Static mobility. A default
  cube is 1 m, so a floor is scale 30 x 15 x 0.2, walls about 4 m tall.
- Lighting enough to see: a Directional Light, Sky Atmosphere, Sky Light,
  Exponential Height Fog.
- A `NavMeshBoundsVolume` covering the floor and about 2 m up. Confirm the nav
  mesh builds, the floor should show navigable.
- 4 to 6 `LTSpawnPoint` actors in a line along the far short wall, on the floor,
  a little inside the room. Varied `Weight`, for example 1, 1, 2, 1.5. Leave
  `AreaTag` empty, `bEnabled` true.
- One `BP_RoundManager`. Position irrelevant.
- A `PlayerStart` near the closed end, facing down the long axis toward the
  spawn points, about 1 m off the floor.
- One `ALTWallBuy` on a side wall near the `PlayerStart`, about chest height.
  Set the `Plate` component static mesh to the engine Cube, scaled flat against
  the wall, roughly 0.1 x 1 x 1.5. On the actor set `Weapon` to `DA_Weapon_SMG`.
  Leave `WeaponCost` 500 and `AmmunitionCost` 250. This is the
  `phase-b2-interaction.md` acceptance step 2 placement.
- World Settings: `GameMode Override = BP_GameMode`.
- Level Blueprint: on `Event BeginPlay`, get the `BP_RoundManager` actor
  (Get All Actors Of Class, then Get index 0) and call `BeginRounds` on it.

## 5. Acceptance for A4

Save all, run PIE. Confirm: WASD and mouse and jump and sprint work, sprint is
faster; Left Mouse fires and the magazine count drops and it reloads at empty
and R reloads early; a zombie spawns and walks toward the player; shooting its
torso lowers its health and points rise by 10. Then run the full ten point test
in `phase-a5-acceptance.md` and report each result.

---

# Phase B3, the HUD widget

Full spec: `phase-b3-feedback-widgets.md`. **No C++.** Every signal is in the
delegate table above and already fires.

One Widget Blueprint, `WBP_HUD`, under `/Game/LastTrain/UI/`, added to the
viewport by `BP_PlayerCharacter` on `BeginPlay`. Read `neostack-umg-widget` and
`neostack-umg-design` first.

## The feel

Barebones, clean, classic. A calm diegetic HUD, the four corners and a centre
reticle, nothing else. **Not** a modern military shooter HUD: no kill feed, no
floating damage numbers, no minimap, no powerup banners, no perk or equipment
rows, no challenge tracker, no special weapon meter, no exfil prompt, no latency
readout. If a layout reference is used for corner placement only, strip it of
all of that. An empty reserved slot on screen is worse than no slot, so do not
lay out space for systems that do not exist yet.

Thin strokes, generous margins from the screen edge (about 4 percent), one
weight of type, tabular figures for anything numeric. Let the charcoal of the
world read through, keep fills to low opacity panels or none at all.

## Palette, as LinearColor

- charcoal `#16161C` = `(R=0.086,G=0.086,B=0.110,A=1)`
- violet `#6C4C9C` = `(R=0.424,G=0.298,B=0.612,A=1)`
- sodium `#E0A030` = `(R=0.878,G=0.627,B=0.188,A=1)`
- crimson `#B02030` = `(R=0.690,G=0.125,B=0.188,A=1)`
- plus a near white and a muted grey for body text, derived from charcoal, not
  new hues.

## Layout and elements

Corner anchored. Six elements plus the vignette.

1. **Round.** Top left. From `OnRoundStarted(int32 Round)`. The number, large,
   with a small "ROUND" label above it in the muted grey. No banner, no
   round change animation beyond a quiet fade of the number.
2. **Player name.** Bottom left, above the points. Plain text from the player
   state display name, muted grey, small. No portrait, no level, no icons.
3. **Points.** Bottom left, the primary readout of that corner. From
   `OnPointsChanged(int32 NewTotal, int32 Delta)`. Show the total in near white,
   tabular. On a change, briefly show the delta beside it, sodium for a gain,
   crimson for a spend, then let it fade and settle to the new total.
4. **Health.** Bottom left, a thin horizontal bar under the points. From
   `OnHealthChanged(float HealthFraction)`. Violet fill on a dark track, no
   numbers. It simply drains and refills. The regeneration is felt through the
   bar, not labelled.
5. **Weapon block.** Bottom right. From `OnAmmoChanged(int32 Magazine, int32
   Reserve)`: magazine large, reserve smaller beside or below it, a thin divider
   between. Weapon name above, small, from the held `LTWeaponData` `DisplayName`.
   No weapon icon needed, text is enough for the classic feel. If an icon is
   trivial, keep it a flat silhouette in muted grey.
6. **Crosshair and hit marker.** Screen centre.
   - Crosshair: a small dot, or four short lines around a gap. Drive the gap or
     line length from `GetCurrentSpreadDegrees()` so it opens on movement and
     recoil bloom and tightens when still. At full `GetAimAlpha()` it collapses
     to a single dot or hides entirely.
   - Hit marker: four short diagonal strokes just outside the crosshair, hidden
     by default. Bind `OnHitConfirmed(bool bHeadshot)`. Body hit: near white,
     about 0.12 s. Headshot: sodium, slightly longer strokes, about 0.16 s.
     Drive with a short widget animation, not a tick and a timer.
7. **Interaction prompt.** Bottom centre, clear of the weapon block. Bind
   `OnInteractableChanged(const FText& Prompt, bool bAvailable)`: set the text
   and fade in when `bAvailable` is true, fade out when false. The key glyph
   ("E") then the prompt text, one line, centred. No flicker when standing at
   the edge of interaction range, the fade timing absorbs a single frame drop.
8. **Damage vignette.** Not a widget element. A post process material on the
   player camera, driven from the existing `OnDamageTaken(float Fraction)`
   Blueprint event. Darkens and slightly desaturates the screen edges on a hit,
   recovers as health regenerates. Do not fake it with a red image in UMG.

## Typeface

Pick the project UI typeface here. A clean humanist or grotesque sans that reads
as classic transit signage in spirit without being Johnston or New Johnston or a
clone of them. One family, two weights at most. Record the choice and the reason
in `docs/art-direction.md`, since every downstream UI matches it.

## Acceptance for B3

Save, `screenshot({mode="asset", asset="/Game/LastTrain/UI/WBP_HUD"})` and check
it renders with the four corners populated and the centre reticle visible. Then
in PIE:

- Round number shows and updates once per round with no banner.
- Points total is correct, a gain flashes sodium and a spend flashes crimson,
  then settles.
- The health bar drains on a hit and refills on regeneration, no numbers.
- Magazine and reserve are correct and update the same frame as a shot or a
  reload. The weapon name matches `DA_Weapon_SMG` `DisplayName`.
- A body shot shows a white marker, a head shot a sodium one, both readable at
  1080p without pulling the eye off the target.
- Aiming collapses the crosshair, sprinting opens it, both settle smoothly.
- The wall buy prompt appears on look and clears on look away with no flicker.
- Taking damage darkens the screen edges and it recovers with health.
- Nothing on screen is information the player cannot act on, and nothing is a
  placeholder for a system that does not exist.

---

# Phase C, editor assets, SPECS PENDING THE C++

The Phase C C++ does not exist yet: no `LTTrain`, no per type zombie classes, no
departure board actor, no station heat component. Do not attempt Phase C editor
work from this file. When that C++ lands, a session fills in the sections below
against the real class names and properties. Listed here so the shape is known:

- **Five zombie type Blueprints.** Parented to whatever variant classes or a
  single data driven `LTZombieCharacter` the C++ ends up using. Roster and
  behaviour numbers come from `docs/brief-v2.md`.
- **`BP_Train`** parented to the Phase C train actor. Arrival on a 100 s
  interval, 25 s dwell, boarding volume, doors. Timings from `docs/brief-v2.md`.
- **Departure board widget or actor.** The countdown and next arrival, in the
  restrained style, palette only, no official line diagram.
- **Station heat readout.** Only once the heat system exists behind it.
- **`BP_RoundManager` roster wiring.** Point it at the five types with per round
  weights once the types exist.

Until then, Phase C is a C++ task, not a NeoStack task.

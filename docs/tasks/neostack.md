# NeoStack, the editor work brief

Everything LAST TRAIN needs built in the Unreal editor, and nothing else. All of
it is asset and Blueprint work: no `Source/` change is required by anything in
this file.

Written for a NeoStack agent driving the editor through `execute_script`, but it
is equally the checklist for a human doing it by hand in the editor UI. The
NeoStack trial expired around 2026-09-07, so assume by hand unless a session
proves otherwise. The generic Lua API lives in the NeoStack skills
(`neostack-blueprint`, `neostack-level-design`, `neostack-umg-widget`,
`neostack-umg-design`); this file is the project specific part.

`docs/tasks/handover.md` is the resume point for the whole project, including the
C++ side. Read it first: **every C++ system after C1 is written but has never
been compiled**, and compiling is the gate on all of the work below. An editor
running the old binary will not even show the new properties this file asks you
to set.

## Ground rules, every task

1. **Engine is Unreal 5.8.** External Xcode on `/Volumes/DriveSohaib`.
2. **Never edit anything under `Source/`.** If a task seems to need a C++
   change, stop and report it. Do not touch the `.Build.cs`, the target files, or
   any `.h`/`.cpp`.
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
10. **Never mutate actors, assets or levels while PIE is running.** No
    `set_actor_property`, no spawning, no `open_level`, no asset save, no widget
    or Blueprint edit during a live Play In Editor session. Doing so puts a PIE
    world object into the editor's transaction (undo) buffer, and on PIE end
    `UEditorEngine::EndPlayMap` asserts on the still referenced `GameInstance`
    and the editor crashes with a `SIGSEGV`. This has crashed the editor at
    least three times. Order of work is always: stop PIE cleanly, then mutate,
    then start PIE again to observe. During PIE, read only: `playtest` status,
    screenshots, log reads, `DisplayAll` console reads, pausing to inspect the
    Details panel. Full write up in `editor-crash-endplaymap.md`.
11. **Stop PIE cleanly.** Use `playtest.stop` or press Escape in the PIE
    viewport. Never end a session by closing the PIE window directly, that is
    the messy teardown path that trips the crash above.

## What cannot be scripted, hand these to a human

- **Custom trace and object channels.** Project Settings, Engine, Collision is
  special cased native UI, not reflected properties. `write_config` reports
  success and changes nothing. The `Weapon` trace channel was created by hand and
  is already in slot 1 (`ECC_GameTraceChannel1`, response Ignore). If a future
  task needs another channel, stop and ask.
- **Other Project Settings pages that are bespoke UI** rather than a plain
  settings object. If a config write silently no ops, do not hand edit the
  `.ini` and do not touch C++ constants. Flag it. **Exception, the packaging map
  list:** `Project Settings > Packaging > List of maps to include in a packaged
  build` IS a plain `ProjectPackagingSettings` object and its `MapsToCook` array
  can be set by writing `Config/DefaultGame.ini` directly. S8 did this on
  2026-09-08: `[/Script/UnrealEd.ProjectPackagingSettings]` with a `+MapsToCook`
  line per map. Committed in `f635ece`.
- **The level Blueprint EventGraph.** Not reachable through the bridge.
- **Editor restarts.** If you were reconnected after an editor restart and
  `execute_script` is missing from your tool list, a human has to start a fresh
  NeoStack chat in this workspace. Tools attach at chat creation.

Two things previously listed here are no longer true, both proved by the
2026-09-07 run: writing `OpeningRoundCounts` on a placed round manager instance
**works** (`bb0ec42` made it `EditAnywhere`), and spawn points **can** now be
moved, because `871f062` gave `ALTSpawnPoint` a root component.

## C++ classes you will parent Blueprints to

All in the `LastTrain` module, `LT` prefix. Parent a Blueprint by the class name,
for example `ParentClass = "LTPlayerCharacter"`.

| Blueprint | C++ parent | Key properties you set |
|---|---|---|
| `BP_PlayerCharacter` | `LTPlayerCharacter` | Input category: `InputMapping`, `MoveAction`, `LookAction`, `JumpAction`, `SprintAction`, `FireAction`, `AimAction`, `ReloadAction`, `InteractAction`. Weapon component: `WeaponData`. Downed category: `BleedOutSeconds`, `ReviveHealthFraction`, `bSoloAutoRevive`, `SoloReviveDelaySeconds`. |
| `BP_Zombie` | `LTZombieCharacter` | Mesh component: skeletal mesh + anim class. Do not edit `HeadBoneNames`, `RepathIntervalSeconds`, `RepathJitterFraction` or any Combat value: the type data assets override what needs overriding. |
| `BP_RoundManager` | `LTRoundManager` | `ZombieClass`, and `Roster`. The four special-round properties are correct by default. |
| `BP_GameMode` | `LTGameMode`, see the note below | `DefaultPawnClass = BP_PlayerCharacter`, `StationRoutes`, `TravelDelaySeconds`. |
| `BP_Train` | `LTTrain` | Timing properties, `BoardingVolume` extent, the nine presentation hooks. |
| `BP_DepartureBoard` | `LTDepartureBoard` | `TrainOverride` (leave empty on a one-train station), the two hooks. |

`BP_GameMode` is currently parented to plain `GameModeBase` and **must be
reparented to `LTGameMode`**. Until it is, no run lifecycle, no boarding and no
travel runs at all, whatever else is built.

Existing components on `LTPlayerCharacter`, already created in C++, nothing to
add: `Camera`, `ViewModel` (leave mesh empty), `Weapon` (`ULTWeaponComponent`),
`Points` (`ULTPointsComponent`), `Interaction` (`ULTInteractionComponent`).

Data asset classes, both `UPrimaryDataAsset`: `LTWeaponData` and
`LTZombieTypeData`. Interactable actors already in C++: `ALTWallBuy` (`Plate`
static mesh, `Weapon`, `WeaponCost` 500, `AmmunitionCost` 250) and `ALTTrain`
(boarding). Spawn point actor: `LTSpawnPoint`, properties `AreaTag`, `Weight`,
`FirstRound`, `CooldownSeconds`, `bEnabled`.

## Delegates and reads that already exist

If a plan proposes new C++ for feedback, it has not read the headers.

| Signal | Delegate or getter | Owner |
|---|---|---|
| Hit and headshot | `OnHitConfirmed(bool bHeadshot)` | `ULTWeaponComponent` |
| Ammunition | `OnAmmoChanged(int32 Magazine, int32 Reserve)` | `ULTWeaponComponent` |
| Current spread, aim blend | `GetCurrentSpreadDegrees()`, `GetAimAlpha()` | `ULTWeaponComponent` |
| Points | `OnPointsChanged(int32 NewTotal, int32 Delta)` | `ULTPointsComponent` |
| Health | `OnHealthChanged(float HealthFraction)` | `ALTPlayerCharacter` |
| Damage taken | `OnDamageTaken(float Fraction)` | `ALTPlayerCharacter` |
| Downed state | `OnDowned()`, `OnRevived()`, `OnBleedOutExpired()`, `IsDowned()`, `GetBleedOutRemaining()` | `ALTPlayerCharacter` |
| Interaction prompt | `OnInteractableChanged(const FText& Prompt, bool bAvailable)` | `ULTInteractionComponent` |
| Round start and end | `OnRoundStarted(int32 Round)`, `OnRoundEnded(int32 Round)` | `ALTRoundManager` |
| Special round | `IsSpecialRound()`, `GetSpecialRoundTag()` | `ALTRoundManager` |
| Station heat | `OnHeatChanged(int32 NewHeat)`, `GetHeat()` | `ULTStationHeat` |
| Zombie died | `OnZombieDied(ALTZombieCharacter*, bool bHeadshot)` | `ALTZombieCharacter` |
| Zombie type reads | `GetZombieType()`, `GetAnimPlayRate()`, `GetArmourRemaining()` | `ALTZombieCharacter` |
| Zombie hooks | `OnHitReaction`, `OnDeathPresentation`, `OnAttackWindUp`, `OnScream` | `ALTZombieCharacter` |
| Train phase | `OnTrainPhaseChanged(NewPhase, OldPhase)`, `GetPhase()`, `AreDoorsOpen()` | `ALTTrain` |
| Train countdown | `GetSecondsUntilArrival()`, `GetSecondsUntilDeparture()` | `ALTTrain` |
| Train presentation | nine `BlueprintImplementableEvent` hooks, listed in `phase-c1-train.md` | `ALTTrain` |
| Board countdown | `OnCountdownChanged(WholeSeconds, Phase)`, `OnPhaseChanged` | `ALTDepartureBoard` |

---

# Phase C, editor assets

This is the bulk of what is left in the project. None of it exists yet, and
until at least the type assets and the roster exist, the editor plays the Phase B
grey box no matter what the C++ says.

The Phase C C++ has landed: `ALTTrain`, `ALTDepartureBoard`, `ULTStationHeat`,
`ULTZombieTypeData` with `ALTZombieCharacter::ApplyTypeData`, and the roster plus
special-round layer on `ALTRoundManager`. None of it has been compiled yet (it was
written with no engine to hand), so **build the editor target first and fix
anything the compiler finds before starting any of this**.

There are no per-type Blueprints and no per-type classes. One mesh, one
`BP_Zombie`, five data assets. The type is scale, tint, play rate and behaviour.

### 1. Five `ULTZombieTypeData` assets

Under `Content/LastTrain/Zombies/`, named `DA_Zombie_Walker`,
`DA_Zombie_Sprinter`, `DA_Zombie_Brute`, `DA_Zombie_Crawler`,
`DA_Zombie_Screamer`. Every value below is **provisional**: it settles in the
Phase G balance pass. Put "provisional, Phase G to confirm" in each asset's
description. Numbers are from `docs/design/gameplay-canon.md` section 6 and
`docs/tasks/phase-c-zombie-types.md`.

Speed and health are multipliers on the character's `BaseWalkSpeed` 130 and
`BaseHealth` 150. Every field named `...Override` falls through to the
character's own value when left at 0, so a walker asset of zeroes and ones is
exactly today's coded default.

| Field | Walker | Sprinter | Brute | Crawler | Screamer |
|---|---|---|---|---|---|
| `Type` | Walker | Sprinter | Brute | Crawler | Screamer |
| `DisplayName` | Walker | Sprinter | Brute | Crawler | Screamer |
| `Behaviour` | None | Sprint | ArmourPlate | LowProfile | Scream |
| `HealthMultiplier` | 1.0 | 0.55 | 5.0 | 0.35 | 0.8 |
| `WalkSpeedMultiplier` | 1.0 | 3.85 | 0.73 | 1.15 | 0.85 |
| `AttackDamageOverride` | 0 | 18 | 45 | 20 | 10 |
| `AttackCooldownOverride` | 0 | 1.0 | 2.2 | 1.1 | 1.5 |
| `AttackRangeOverride` | 0 | 0 | 0 | 0 | 0 |
| `MeshScale` | 1.0 | 0.95 | 1.5 | 0.5 | 1.0 |
| `ColourTint` | white | sodium `#E0A030` | crimson `#B02030` | violet `#6C4C9C` | white |
| `AnimPlayRate` | 1.0 | 1.35 | 0.8 | 1.0 | 1.0 |
| `RepathIntervalOverride` | 0 | 0.2 | 0 | 0.4 | 0 |
| `CapsuleHalfHeightOverride` | 0 | 0 | 130 | 45 | 0 |
| `CapsuleRadiusOverride` | 0 | 0 | 0 | 0 | 0 |
| `AvoidanceConsiderationRadiusOverride` | 0 | 0 | 70 | 0 | 0 |
| `ContactRangeOverride` | 0 | 25 | 0 | 0 | 0 |
| `SpawnWeightNormalRound` | 100 | 0 | 0 | 12 | 6 |
| `FirstRoundAvailable` | 1 | 5 | 10 | 8 | 12 |
| `HighHeatWeightMultiplier` | 1.0 | 2.0 | 1.0 | 2.0 | 2.0 |
| `MaxAliveOfThisType` | 0 | 0 | 0 | 0 | 1 |
| `SprintLungeImpulse` | 0 | 200 | 0 | 0 | 0 |
| `ArmourBodyDamageToBreak` | 0 | 0 | 200 | 0 | 0 |
| `ScreamLineOfSightSeconds` | - | - | - | - | 2.0 |
| `ScreamSummonCount` | - | - | - | - | 4 |
| `ScreamCancelWindowSeconds` | - | - | - | - | 0.5 |
| `bRagdollOnDeath` | false | false | false | false | false |
| `CorpseLifetimeOverride` | 0 | 0 | 10 | 0 | 0 |
| `DeathScreenShakeRadius` | 0 | 0 | 600 | 0 | 0 |

Two notes. `AnimPlayRate` for the sprinter and the brute are eye-tune values, not
canon: set them so the feet do not skate at the new speed. Weight 0 means
special-round only, which is why the sprinter and the brute never appear in a
normal round's mix.

### 2. The tintable zombie material

`ColourTint` is deliberately **not** applied by C++. A 40 strong mixed crowd
cannot afford a dynamic material instance per spawn, so:

- One shared master material for the zombie body with a vector parameter named
  `TintColour`.
- Five `UMaterialInstance` assets off it, one per type, each with `TintColour` set
  to the row above.
- `BP_Zombie` picks the instance for its type on spawn, reading `GetZombieType()`,
  and calls `SetMaterial` with it. No `CreateDynamicMaterialInstance`.

### 3. `BP_Zombie` graph additions

- `OnDeathPresentation`: read `bRagdollOnDeath` and `DeathScreenShakeRadius` off
  the character (they are `BlueprintReadOnly`) rather than expecting parameters.
  The event signature is unchanged.
- `OnAttackWindUp`: the per-type attack tell. The sprinter's lunge impulse is
  already applied in C++; this is animation and audio only.
- `OnScream`: the screamer's animation and audio. The summon itself is C++.
- `OnHitReaction`: branch on `GetArmourRemaining() > 0` for the brute's blocked
  response versus a wounded one.
- The anim Blueprint reads `GetAnimPlayRate()` and drives the locomotion play
  rate from it.

### 4. `BP_RoundManager` roster wiring

Fill `Roster` with the five data assets, in `L_GreyboxTest` and
`L_CanaryWharf_Greybox`. An empty roster keeps the old single-walker behaviour, so
this is the switch that turns the whole system on. `SprinterRoundInterval` 5,
`SprinterRoundCountFraction` 0.75, `BruteRoundInterval` 10,
`BruteRoundBruteCount` 2 are the class defaults and need no change. Add a
`ULTStationHeat` component to the placed round manager if it does not have one:
the train, the round cap and the high-heat roster shift all read it.

### 5. `BP_Train` and `BP_DepartureBoard`

- **`BP_Train`** from `ALTTrain`: a box mesh child in the trackbed, placed at the
  platform edge, `BoardingVolume` sized over the door aperture. The nine
  presentation hooks are listed in `docs/tasks/phase-c1-train.md`, whose 12-point
  acceptance list is the test.
- **`BP_DepartureBoard`** from `ALTDepartureBoard`: a text render child driven by
  `OnCountdownChanged(WholeSeconds, Phase)`, wording switched off
  `OnPhaseChanged`. `HasTrain()` false is the blank or no-service state. Leave
  `TrainOverride` empty on a one-train station. Palette only, no roundel, no
  Johnston, no official line diagram.

### 6. `BP_GameMode`, for travel

Travel needs two things this asset does not have yet:

- **Reparent `BP_GameMode` to `ALTGameMode`.** It is currently on plain
  `GameModeBase`, so none of the run lifecycle, boarding or travel runs at all.
- **Fill `StationRoutes`**, the map of this map's asset name to the destination's:
  `L_GreyboxTest` to `L_CanaryWharf_Greybox`, and `L_CanaryWharf_Greybox` back to
  `L_GreyboxTest`. One shared game mode Blueprint serves both stations, and
  `StationRoutes` is what lets it: `NextStationMap` alone is a single class
  default and would send both stations to the same place.
- Confirm World Settings on both maps uses `BP_GameMode`.

`TravelDelaySeconds` defaults to 1.5, which is the window `OnPlayerBoarded` has
for a door chime and a fade before the level loads. A Blueprint fade on
`ULTGameInstance::OnTravelStarted` additionally needs a `BP_GameInstance` and the
`GameInstanceClass` line in `Config/DefaultEngine.ini` repointed at it; nothing
needs that today.

---

# Canary Wharf spawn points

`L_CanaryWharf_Greybox` has ten spawn points, `CW_SpawnPoint_1` to `_10`, and
**all ten sit at world origin** while the platform is centred around
`(5550,3150,-10)`. Every zombie therefore spawns 5,500 to 6,900 units off the
level and free falls. The 2026-09-07 run diagnosed it: `ALTSpawnPoint` had no
root component, so it could not hold a transform and the moves in `f76e0a4`
silently did not persist.

The C++ half is fixed (`871f062` gives the actor a `USceneComponent` root), so
the points can now be moved. Place them along the north end of the platform,
around `y=5600`, `x` roughly 900 to 10200, `z=100`, near the two tunnel mouths at
`x=4050` and `x=7050`. Points 9 and 10 are deliberately `FirstRound=4`; leave
that alone. Then re-run the horde smoke test: rounds already start, so what you
are checking is that zombies land on the floor and path to the player.

Nothing else about those points is wrong. Every one has `bEnabled=True`, an
empty `AreaTag` (which `IsAvailable` does not even read), `CooldownSeconds` 0.9
and a sane weight, and `CW_RoundManager` has the right `ZombieClass`.

---

# Phase B leftovers

Small, and all of them editor or PIE work rather than asset building.

1. **Measure the crowd frame gate.** Editor reading DONE 2026-09-09 (S6): PIE
   `L_GreyboxTest`, `OpeningRoundCounts[0]` 6 -> 30 (reverted after), **30
   BP_Zombie alive**, whole frame **~40 fps / ~24 ms held flat over 6x120-frame
   batches**. `profile("frame_stats")` shows `delta_time_ms` == `gpu_frame_time_ms`
   with `idle_time_ms` 0, so it is **GPU-bound, not game-thread-bound** - the
   game+draw threads had ~8 ms headroom on the frame where the GPU fit inside
   the budget. `stat unit` overlay is NOT captured by `playtest_observe`; use
   `profile("frame_stats")` for the Frame/Game/GPU split. **Still open:** the
   same reading in a **packaged Development build** at a fixed resolution, which
   is the honest number for the B3 60 fps gate. The editor number is
   render-limited by the S9 zombie material + Lumen on this Mac's GPU, not by
   the crowd logic.
2. **Wall buy PIE acceptance.** `phase-b2-interaction.md` steps 3 to 7. The buy
   itself PASSED via property read-back (`WeaponCost` 500 spent, `AmmunitionCost`
   250 on the second interact). Still needs a human at the keyboard for the
   walk-up: the bridge cannot aim a first-person camera at the plate. Walk to
   `GreyboxTest_WallBuy_SMG`, check the prompt fades in, `E` under 500 points
   no-ops, `E` with 500 or more buys and swaps the weapon and flashes the points
   crimson, and `E` again offers ammunition at 250.
3. **HUD polish review.** DONE 2026-09-09 (S7). `WBP_HUD` matches
   `phase-b3-feedback-widgets.md` element-for-element and the graph wiring is
   healthy (5 delegates bound, crosshair spread on `GetCurrentSpreadDegrees` +
   `GetAimAlpha`, 8 `PlayAnimation` calls at the spec durations). **Follow-up
   fixes, none blocking:** delete the 2 leftover `PrintString` debug nodes in
   the EventGraph; migrate the fonts from engine stock `DroidSansMono` / `Roboto`
   to the OFL project faces the S9 haul imported (Overpass / Barlow) and record
   the choice in `docs/art-direction.md` as B3 asks; default `PointsDelta`, the
   4 `HitMark*` and `DamageVignette` to Collapsed/Hidden and let their
   animations reveal them, rather than sitting `Visible` at rest.

---

# Records

The original A4 grey box asset spec and the full B3 HUD layout spec are kept in
`phase-a4-editor-setup.md` and `phase-b3-feedback-widgets.md`. Both are built;
those files are the record of how, and B3's acceptance list is still live.

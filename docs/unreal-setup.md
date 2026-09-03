# Unreal setup, Phase 0 and 1

Everything the C++ in `Source/` needs from the editor before it will run. None
of this can be done from source files, so it is written out step by step.

## 1. Project creation

1. New project, Games, First Person, C++, no starter content, name `LastTrain`.
2. Close the editor. Replace the generated `Source/` with the one in this repo,
   and replace the generated `LastTrain.uproject` with this repo's copy.
3. Right click `LastTrain.uproject`, Generate Visual Studio project files.
4. Build in Visual Studio, Development Editor, Win64. Fix compile errors before
   opening the editor. This code has never been compiled, so expect some.

## 2. Custom trace channel, required

The weapon component traces on `ECC_GameTraceChannel1`. Without this the
zombie mesh will not be hit.

Project Settings, Engine, Collision, Trace Channels, New Trace Channel:

- Name: `Weapon`
- Default response: Ignore

Confirm it lands in slot 1. If it does not, change the channel constant in
`LTWeaponComponent.cpp` and `LTZombieCharacter.cpp` to match.

## 3. Enhanced Input assets

Create under `Content/LastTrain/Input/`:

| Asset | Type | Value type |
|---|---|---|
| `IA_Move` | Input Action | Axis2D |
| `IA_Look` | Input Action | Axis2D |
| `IA_Jump` | Input Action | Digital |
| `IA_Sprint` | Input Action | Digital |
| `IA_Fire` | Input Action | Digital |
| `IA_Aim` | Input Action | Digital |
| `IA_Reload` | Input Action | Digital |
| `IMC_Default` | Input Mapping Context | |

In `IMC_Default`, map: WASD to `IA_Move` with the standard 2D axis modifiers,
mouse XY to `IA_Look` with Negate on Y, Space to `IA_Jump`, Left Shift to
`IA_Sprint`, Left Mouse to `IA_Fire`, Right Mouse to `IA_Aim`, R to
`IA_Reload`.

## 4. Blueprints

**`BP_PlayerCharacter`**, parent `LTPlayerCharacter`. Assign all seven input
actions and `IMC_Default` in the Input category. Assign a view model skeletal
mesh, or leave empty for now.

**`BP_Zombie`**, parent `LTZombieCharacter`. Assign a skeletal mesh and
animation blueprint. Set `HeadBoneNames` to match the actual skeleton, since
head shots do nothing if these names are wrong. Confirm the mesh blocks the
`Weapon` channel and the capsule ignores it.

**`BP_RoundManager`**, parent `LTRoundManager`. Set `ZombieClass` to
`BP_Zombie`. Place one in the level and call `BeginRounds` from the level
Blueprint or the game mode.

**`BP_GameMode`**, set Default Pawn Class to `BP_PlayerCharacter`. Set it in
World Settings.

## 5. Weapon data asset

Create a `LTWeaponData` asset, `DA_Weapon_SMG`. Defaults in the class are tuned
for a compact SMG and are a reasonable starting point. Assign it to
`BP_PlayerCharacter`'s Weapon component in the Weapon Data slot.

Original names only. Do not reuse a weapon name from another game.

## 6. Navigation

Place a Nav Mesh Bounds Volume covering the playable area. Press P to visualise
it. Zombies use `MoveToActor` through their AI controller, so without a nav
mesh they will stand still.

Set `AutoPossessAI` is already `PlacedInWorldOrSpawned` in C++, so no AI
controller assignment is needed for basic movement.

## 7. Spawn points

Place several `LTSpawnPoint` actors around the grey box room. Leave `AreaTag`
empty for now. Vary `Weight` so some routes see more traffic.

## 8. Phase 1 acceptance test

Run these before calling Phase 1 done:

1. Walk, look, jump, sprint.
2. Fire. Rounds decrease, the magazine reloads when empty.
3. Hit a zombie in the body. It takes damage and points increase by 10.
4. Kill with a body shot. Points increase by 60.
5. Kill with a head shot. Points increase by 130.
6. Aim. Field of view narrows, movement slows, the spread cone tightens.
7. Sprint while aiming. Aim drops.
8. Hold the trigger. Accuracy degrades, then recovers after release.
9. A zombie navigates to you and attacks. Health drops, then regenerates four
   seconds after the last hit.
10. Kill every zombie. The round ends exactly once, and the next begins after
    ten seconds with more zombies.

If any of these fail, that is a Phase 1 bug, not a Phase 2 task.

# Phase A4 — editor setup for the first playable grey box

**Engine:** Unreal Engine 5.8, macOS, external Xcode on `/Volumes/DriveSohaib`
mounted.

**Prerequisite:** the `LastTrain` C++ module compiles. Confirm with the batch
build in `CLAUDE.md` before opening the editor.

**Goal:** the shortest path to shooting a zombie in a grey box room, so the
`unreal-setup.md` section 8 checklist can be run. No art, no atmosphere, no
station geometry.

This is editor work. A model cannot create `.uasset` or `.umap` files. Follow
`docs/unreal-setup.md` sections 2 to 8; the deltas below are the only changes
from that document.

## Deltas from `unreal-setup.md`

- **Section 1 is done.** The project exists, the module compiles, targets are on
  `V7` and `Unreal5_8`. Skip it.
- **Skeleton for the zombie:** use the Unreal fifth generation mannequin
  (`SKM_Manny` or `SKM_Quinn`) as a stand in. `HeadBoneNames` in
  `LTZombieCharacter.h` already contains `head`, which the mannequin has, so head
  shots work without editing the array.
- **View model:** leave `BP_PlayerCharacter`'s `ViewModel` mesh empty for now.
- **Zombie animation:** assign the mannequin's default idle or the third person
  locomotion animation blueprint. Movement matters, animation quality does not.

## Checklist

1. **Trace channel.** Project Settings, Engine, Collision, Trace Channels, new
   channel `Weapon`, default response Ignore. Confirm it lands in slot 1
   (`ECC_GameTraceChannel1`). If not, change the constant in
   `LTWeaponComponent.cpp` and `LTZombieCharacter.cpp` to match.
2. **Enhanced Input** under `Content/LastTrain/Input/`: `IA_Move` and `IA_Look`
   as Axis2D, `IA_Jump` `IA_Sprint` `IA_Fire` `IA_Aim` `IA_Reload` as Digital,
   `IMC_Default`. In `IMC_Default` map WASD to `IA_Move` with the standard 2D
   axis modifiers, mouse XY to `IA_Look` with Negate on Y, Space to `IA_Jump`,
   Left Shift to `IA_Sprint`, Left Mouse to `IA_Fire`, Right Mouse to `IA_Aim`,
   R to `IA_Reload`.
3. **`DA_Weapon_SMG`**, a `LTWeaponData` asset under `Content/LastTrain/Data/`.
   The class defaults are already tuned for a compact automatic weapon. Give it
   an original `DisplayName`. Do not reuse a weapon name from another game.
4. **`BP_PlayerCharacter`**, parent `LTPlayerCharacter`. Assign the seven input
   actions and `IMC_Default` in the Input category. Assign `DA_Weapon_SMG` to the
   Weapon component's Weapon Data slot.
5. **`BP_Zombie`**, parent `LTZombieCharacter`. Assign the mannequin skeletal
   mesh and an animation blueprint. Confirm the mesh blocks the `Weapon` channel
   and the capsule ignores it (the C++ constructor already sets this, so just
   verify).
6. **`BP_RoundManager`**, parent `LTRoundManager`. Set `ZombieClass` to
   `BP_Zombie`.
7. **`BP_GameMode`**, Default Pawn Class `BP_PlayerCharacter`. Set it in World
   Settings of the test map.
8. **Grey box test map** under `Content/LastTrain/Maps/`, `L_GreyboxTest`:
   - A floor and four walls from primitives or geometry brushes, roughly 30 by
     15 metres, so the platform composition from the reference frame is at least
     suggested: long axis, one closed side.
   - A `NavMeshBoundsVolume` covering the floor. Press P to confirm the nav mesh
     builds.
   - Four to six `LTSpawnPoint` actors along the far short wall, varied `Weight`.
   - One `BP_RoundManager`.
   - A `PlayerStart`.
   - Call `BeginRounds` on the round manager from the level Blueprint's
     `BeginPlay`, or from `BP_GameMode`.
9. Save all. Commit the `Content/` additions with Git LFS (`.uasset` and `.umap`
   are already LFS tracked in `.gitattributes`).

## Accept

Play in editor. You can walk, look, jump, sprint, fire, and a zombie spawns and
walks toward you. Then run `docs/tasks/phase-a5-acceptance.md`.

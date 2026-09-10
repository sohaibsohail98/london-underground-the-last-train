# H1 supporting spec: starting pistol and wall buy progression

Lane: remote session for the design draft (numbers, unlock order, no editor
access needed); CC-in-Unreal to create the actual `ULTWeaponData` assets and
place/configure `ALTWallBuy` actors once the draft is reviewed.

Depends on: nothing to start the design draft. The editor half depends on
that draft being reviewed, and on Phase H's FP arms/weapon mesh existing if
the new weapons are to be visually distinct, though placeholder meshes are
fine to start with.

## Problem

The player currently starts with whatever `BP_PlayerCharacter` is configured
with in the editor (today, effectively `DA_Weapon_SMG` if anything). There is
no designed starting weapon, no pistol, and `ALTWallBuy` (weapon cost 500,
ammunition cost 250 by default) has never been placed against a real
progression: what is buyable, where, in what order, at what price, is
undesigned.

## What exists to build on

- `ULTWeaponData`: `BaseDamage`, `HeadshotMultiplier`, `Penetration`,
  `MaxRange`, `FalloffStart/End`, `RoundsPerMinute`, `bAutomatic`,
  `PelletsPerShot`, `MagazineSize`, `MaxReserve`, `ReloadSeconds`, plus spread
  fields. One data asset per weapon, no code change needed to add a new one.
- `ALTWallBuy`: `Weapon`, `WeaponCost` (default 500), `AmmunitionCost`
  (default 250). Implements the interaction interface already, sells the
  weapon first, then ammunition once the player holds it.
- `ULTPointsComponent`: 500 start, 10 per hit, 60 per kill, 130 per headshot
  kill. This is the economy the wall buy prices have to work against: a 500
  point starting weapon costs the player's entire starting bank in one
  purchase, which may or may not be the intended pacing, flag this explicitly
  in the draft rather than assuming either way.

## Scope of the design draft

1. **The starting pistol.** A `ULTWeaponData` numbers draft: lower magazine,
   lower `RoundsPerMinute` or non-automatic, modest damage, cheap or free to
   start with (the player should not need to buy their first weapon). Name
   it something original, not a real firearm model name.
2. **2 to 4 wall buy weapons**, each a numbers draft plus a suggested price
   in `ULTPointsComponent` terms: how many kills/headshots should it
   realistically take to afford each one, given the 10/60/130 economy. State
   this as "roughly N kills at round M" rather than just a bare price, so the
   pacing intent is checkable later.
3. **Suggested wall buy placement order** relative to the existing station
   layout (nearest spawn versus furthest from spawn, train-adjacent or not),
   as a placement note for CC-in-Unreal to action, not a literal transform.
4. **Ammunition cost sanity check** against the drafted weapons' magazine and
   reserve sizes: the existing 250 default should be revisited per weapon,
   not left uniform, if a weapon's `MaxReserve` is dramatically bigger or
   smaller than `DA_Weapon_SMG`'s.

## Rules

- Original weapon names only, no real firearm model names, no Call of Duty
  naming conventions, per `CLAUDE.md`.
- This spec is the design draft only. Creating the actual `ULTWeaponData`
  assets, placing `ALTWallBuy` actors, and configuring
  `BP_PlayerCharacter`'s starting weapon is editor work for CC-in-Unreal
  after the draft is reviewed, not part of the remote draft itself.

## Accept, for the draft

- A short table: weapon name, role (starter / sidegrade / upgrade), key
  `ULTWeaponData` numbers, suggested cost, suggested unlock point in the
  station.
- The starting pistol is free or trivially affordable, consistent with
  "the player always has something to fight with".
- Explicitly states whether the 500/10/60/130 point economy needs to change
  alongside the new prices, or whether it holds as is.

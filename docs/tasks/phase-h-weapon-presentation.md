# Phase H: first person weapon presentation

Lane: Opus in the editor (CC-in-Unreal), for the skeletal mesh, anim
Blueprint, montages and Blueprint wiring. No `Source/` change is expected;
`ULTWeaponComponent` already exposes every hook an animation layer needs (see
below). If a hook is genuinely missing, stop and write it up as a spec rather
than improvising in Blueprint.

Depends on: an FP arms skeletal mesh and at least one weapon mesh existing in
`Content/`. This is the actual blocker today, not the animation work itself:
raised 2026-09-10 against the free asset research already under way (see
`docs/reference/asset-sources-phase-f.md` for the pattern to follow, same
licence discipline applies here).

## Why this is a phase, not a task

Playtesting on 2026-09-10 found the combat loop has no weapon presentation at
all: no visible gun, no fire animation, no reload animation, hits register as
a trace with no visual or audio confirmation beyond the hit marker. The
underlying hitscan system (`ULTWeaponComponent`) is complete and already
animation ready; what does not exist is anything to animate.

## What already exists to build on

`Source/LastTrain/Public/Weapons/LTWeaponComponent.h`:

- `GetAimAlpha()`: 0 fully hip fired, 1 fully aimed, already smoothed. Drive
  an aim offset or a full ADS pose blend from this directly.
- `IsAiming()`, `IsReloading()`: state booleans an anim Blueprint state
  machine reads directly.
- `OnAmmoChanged` (magazine, reserve): fires on every shot and every reload
  completion. The reload montage's end should line up with this firing, not
  the other way round: `StartReload()` already runs a real timer
  (`ReloadSeconds` on `ULTWeaponData`) and calls `FinishReload()` on
  expiry, so a montage that does not match `ReloadSeconds` will visibly
  desync from when the magazine actually refills.
- `OnHitConfirmed` (bool headshot): fires per pellet hit, already there for
  a hit marker; reuse for a muzzle flash colour cue or a headshot specific
  sound if wanted, not required.
- `StartFiring()` / `StopFiring()` call `FireOnce()` internally on the
  `RoundsPerMinute` cadence from `ULTWeaponData`. The fire montage should be
  a single shot loop triggered per `FireOnce()`, not a held animation, so it
  stays in sync with automatic weapons' RPM.

`Source/LastTrain/Public/Player/LTPlayerCharacter.h` already has the camera
FOV lerp for ADS and sprint cancelling aim; this phase adds the arms/weapon
visual to sit under that camera, it does not change the camera behaviour.

## Scope

1. **FP arms and first weapon mesh.** A generic FP arms skeletal mesh (no
   name, no operator likeness, original or a properly licensed asset per the
   project's asset sourcing discipline) and one weapon mesh: the starting
   pistol (see `phase-h1-starting-loadout.md` for what that weapon's data
   asset should look like; if that spec has not landed yet, use a plain
   placeholder pistol silhouette and do not block on the balance numbers).
2. **Anim Blueprint.** States for idle, fire, reload, aim in and aim out, at
   minimum. Aim in/out blends on `GetAimAlpha()`, not a binary switch, so the
   ADS transition reads smooth against the camera's own FOV lerp.
3. **Fire montage.** Triggered once per `FireOnce()` (a Blueprint event on
   the weapon component would need adding if there is no existing hook to
   bind to for "a shot just fired", check `OnAmmoChanged`'s magazine delta
   first before assuming a new delegate is needed).
4. **Reload montage.** Length matched to `ReloadSeconds` on the active
   `ULTWeaponData`, not hardcoded, so a future weapon with a different reload
   time does not desync. If the montage cannot be dynamically retimed per
   weapon, note that as a limitation rather than silently shipping a fixed
   length.
5. **Muzzle flash and a minimal shell eject.** Nice to have, not blocking
   accept.
6. **Weapon swap on `SetWeapon()`.** Swapping the held mesh when the
   component's `WeaponData` changes (wall buy purchase), so buying a new gun
   visibly changes what is in the player's hands, not just the stats.

## Rules

- No `Source/` change expected. `ULTWeaponComponent`'s public interface above
  is deliberately already sufficient; if the anim Blueprint genuinely cannot
  do something with what is exposed, stop and write the missing hook up as
  its own small spec rather than reaching into `Source/` from the editor
  session (CC-in-Unreal never edits `Source/`, per `who-does-what.md`).
- `MAP CHECK` as a console exec is banned, use `MAP CHECKDEP NOCLEARLOG`.
- Never mutate an actor or asset while PIE is running: stop PIE cleanly
  first.
- No Call of Duty weapon or attachment naming, per `CLAUDE.md`. Original
  names only, matching `DA_Weapon_SMG`'s existing convention.

## Accept

- Standing still, firing the starting weapon shows a visible gun, a fire
  animation per shot (or per RPM tick on automatic), and a muzzle flash.
- Reloading plays a reload animation whose length reads as matched to the
  weapon's actual `ReloadSeconds`, ending at or near the moment
  `OnAmmoChanged` reports the refilled magazine.
- Aiming down sights blends the arms/weapon pose smoothly with the camera's
  existing FOV lerp, driven by `GetAimAlpha()`, not a binary snap.
- Buying a new weapon at a wall buy visibly swaps the held mesh.
- Map Check 0 errors, 0 warnings if the session's tooling can obtain a
  reading; note plainly if it cannot, same as prior Phase F sessions.
- Commit new meshes, animations and Blueprints via LFS.

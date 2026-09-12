# H3: melee presentation

Lane: CC-in-Unreal, for the input asset entry, the swing animation and the
impact sound. The C++ is done and no `Source/` change is expected; if a hook
is genuinely missing, stop and write it up as its own small spec rather than
reaching into `Source/` from the editor session, per `who-does-what.md`.

Depends on: nothing for the key binding, which can be done today. The swing
animation depends on whichever FP arms skeletal mesh
`phase-h-weapon-presentation.md` lands on, so it is blocked behind the same
asset that blocks H. The impact sound depends on the G1 audio hooks.

## What already exists

`ALTPlayerCharacter` carries the whole mechanic as of this branch. See
`../design/gameplay-canon.md` section 9 for the settled design and why melee
is a contextual bash rather than a weapon slot.

| Member | What it is |
|---|---|
| `MeleeAction` | `UInputAction`, `EditDefaultsOnly`, null until a Blueprint assigns it. Exactly the pattern `FireAction` and `InteractAction` follow. Bound in `SetupPlayerInputComponent` on `ETriggerEvent::Started`. |
| `PerformMelee()` | `BlueprintCallable`. Traces `MeleeRange` from the view point, applies `MeleeDamage` through `ALTZombieCharacter::ReceiveShot`, starts the cooldown. |
| `OnMeleeSwing()` | `BlueprintImplementableEvent`. Fires on every strike the cooldown and the run state allowed, hit or miss. This is the animation and swing audio hook. |
| `OnMeleeHitConfirmed` | `BlueprintAssignable`, one `bool bHeadshot` parameter. Fires only when a zombie was struck. Same shape as `ULTWeaponComponent`'s `OnHitConfirmed`, so the existing hit marker binding works off it unchanged. |
| `GetMeleeCooldownRemaining()` | `BlueprintPure`. Seconds until the next strike, 0 when ready. |
| `MeleeRange`, `MeleeDamage`, `MeleeCooldownSeconds` | `EditDefaultsOnly` on the character. 150, 50 and 0.8 by default, all provisional pending the Phase G balance pass. |

## Scope

1. **Bind `MeleeAction` in the Enhanced Input asset.** Create `IA_Melee`
   alongside the existing input actions, add it to the mapping context, and
   assign it on `BP_PlayerCharacter`. Recommended key: `V` on keyboard and
   mouse, the genre convention, and right thumbstick click on a gamepad.
   Do not set a default in `Source/`: the property is intentionally null
   until a Blueprint assigns it, the same as every other input action in
   this project.

2. **The bash animation.** Drive it from `OnMeleeSwing`, not from
   `OnMeleeHitConfirmed`, so a whiff still swings. Length should read as
   under `MeleeCooldownSeconds` (0.8s) so the arms have settled before the
   next strike is allowed; if the animation has to be longer, raise
   `MeleeCooldownSeconds` on `BP_PlayerCharacter` to match rather than
   letting the two desync, exactly as the reload montage has to match
   `ReloadSeconds`.

   **Sourcing note.** `../reference/asset-sources-phase-h.md` scoped idle,
   fire, reload and aim in/out only. A melee swing was not in that sweep, so
   it needs either its own quick sourcing check or hand authoring. The
   recommendation is the same one that document reached in section 3.3 for
   fire and reload: hand author the montage in Sequencer against whichever
   FP arms mesh section 1 lands on, using the Quaternius Universal Animation
   Library (CC0) purely as motion reference. A one second shove is a shorter
   and easier hand key than either the fire or the reload montage, so this
   should not become a blocker on its own. If CC-in-Unreal is opening the
   Fab "Ultimate FPS Animations Pack" page anyway for fire and reload, check
   whether it carries a melee or bash clip on the same skeleton while it is
   open; that would be a legitimate time saver, but do not block on it.

3. **The impact sound.** There is no audio hook for melee in `Source/` yet.
   G1 is the branch adding `USoundBase` hooks across the weapon, the zombies
   and the train (see `phase-g-audio-hud-balance.md` section G1; the detailed
   spec file `phase-g1-audio.md` does not exist at the time of writing and
   lands with that branch). Two sounds are wanted: the swing itself, off
   `OnMeleeSwing`, and the impact, off `OnMeleeHitConfirmed` with the
   headshot flag choosing a wetter variant. Until G1 lands, both can be
   played straight from the Blueprint hooks with `Play Sound At Location`,
   the same way `ULTWeaponData::FireSound` is played today. If G1 lands
   first, add the melee hooks to whatever pattern it established rather than
   inventing a second one.

4. **HUD, optional.** The centre hit marker already fires from
   `OnHitConfirmed`; binding `OnMeleeHitConfirmed` to the same widget event
   gives melee the same confirmation for free. Nothing else on the HUD needs
   to change: melee has no ammunition counter and, per `phase-g2-hud.md`'s
   restraint rule, does not earn a cooldown meter of its own.
   `GetMeleeCooldownRemaining()` is there if playtesting says otherwise.

## Rules

- No `Source/` change expected.
- `MAP CHECK` as a console exec is banned, use `MAP CHECKDEP NOCLEARLOG`.
- Never mutate an actor or asset while PIE is running: stop PIE cleanly
  first.
- Commit new animations and Blueprints via LFS.

## Accept

- Pressing the melee key plays a swing animation whether or not it connects.
- A strike on a zombie at contact range damages it, shows the hit marker,
  and spawns the same restrained blood spatter a gunshot does.
- Three strikes kill a round one walker, and the cooldown visibly prevents
  spamming the key faster than 0.8s.
- Melee still works with the magazine and the reserve both at zero.
- Map Check 0 errors, 0 warnings if the session's tooling can obtain a
  reading; note plainly if it cannot.

## Gore, and why this branch spawns none of its own

`ALTPlayerCharacter::PerformMelee` deliberately does not call
`ULTGoreDecalSubsystem::SpawnBloodDecalForWorld`. It does not need to. The
gore branch calls it from inside `ALTZombieCharacter::ReceiveShot`, which
the melee path routes its damage through, so a bash already produces the
same restrained spatter a gunshot does, from the same shared subsystem and
the same single entry point.

Calling it a second time from the melee side was tried and removed. It drew
two spatters for one strike, and because the melee side had no sight of the
brute's armour plate it also drew blood on a blow the plate had stopped.
`ReceiveShot`'s call sits after that early return and is correct for both
the hitscan and the melee path. Do not add a melee specific gore call back
in: if melee ever grows a damage route that bypasses `ReceiveShot`, spawn
the decal there instead.

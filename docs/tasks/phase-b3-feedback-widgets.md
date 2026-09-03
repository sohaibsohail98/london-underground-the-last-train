# Phase B3 — hit marker and interaction prompt widgets

**Engine:** Unreal Engine 5.8. Editor work. No C++ change, so nothing to
compile.

**Model:** none needed for the build itself. Hand a session this file only if
you want the widget tree written out step by step.

**Prerequisite:** B2 landed and its acceptance passed.

## Why there is no C++ here

Both delegates already exist and already fire. Nothing needs adding.

| Signal | Delegate | Broadcast from |
|---|---|---|
| Hit and headshot | `ULTWeaponComponent::OnHitConfirmed(bool bHeadshot)` | `LTWeaponComponent.cpp`, inside `TracePellet` on a confirmed zombie hit |
| Ammunition | `ULTWeaponComponent::OnAmmoChanged(int32 Magazine, int32 Reserve)` | `SetWeapon`, `FireOnce`, `FinishReload`, `RefillAmmunition` |
| Points | `ULTPointsComponent::OnPointsChanged(int32 NewTotal, int32 Delta)` | `AwardHit`, `AwardKill`, `AddPoints`, `TrySpend` |
| Health | `ALTPlayerCharacter::OnHealthChanged(float HealthFraction)` | `TakeDamage` and the regeneration tick |
| Interaction prompt | `ULTInteractionComponent::OnInteractableChanged(FText, bool)` | the B2 tick, on change only |

If a session proposes new C++ for any of the above, it has not read the headers.
Stop it and point at this table.

## What to build

One widget, `WBP_HUD`, added to the viewport by the player Blueprint on
`BeginPlay`. Keep it restrained. `docs/art-direction.md` and the classic
restraint rules mean no kill feed, no floating damage numbers, no permanent
minimap.

### Elements

1. **Hit marker.** Four short strokes around screen centre, hidden by default.
   Bind `OnHitConfirmed`. On a body hit, show in near white for about 0.12s. On
   a headshot, show in sodium `#E0A030`, slightly larger, for about 0.16s.
   Drive both with a short animation rather than a tick and a timer.
2. **Crosshair.** A small static dot or four-line reticle. Drive its spread from
   `GetCurrentSpreadDegrees()` so it opens on movement and recoil bloom and
   tightens when aiming. It should visibly disappear or shrink to a dot at full
   `GetAimAlpha()`.
3. **Interaction prompt.** Bottom centre, above the weapon block. Bind
   `OnInteractableChanged`: set the text and fade in when `bAvailable`, fade out
   when not. Show the key glyph and the prompt text on one line.
4. **Weapon block.** Bottom right. Magazine and reserve from `OnAmmoChanged`,
   weapon name from the held `ULTWeaponData`.
5. **Points.** Bottom left. Bind `OnPointsChanged`. Flash the delta briefly in
   sodium on a gain and crimson on a spend, then settle back to the total.
6. **Round.** Top left, from the round manager. Number only, no banner.
7. **Damage vignette.** Not a widget element. It is a post process material
   driven from the existing `OnDamageTaken` Blueprint event on the player. Keep
   it there rather than faking it with a red image in UMG.

### Do not add yet

Perk icons, equipment slots, the station schematic, the train countdown and the
heat indicator all belong to Phase C or later, once the systems behind them
exist. An empty slot on screen is worse than no slot.

## Constraints

- Palette only: `#16161C`, `#6C4C9C`, `#E0A030`, `#B02030`.
- No Johnston or New Johnston, and nothing that reads as a clone of it. Pick the
  project typeface here and record the choice in `docs/art-direction.md`, since
  everything downstream will match it.
- British spelling in every user facing string. "Ammunition", not "Ammo".

## Acceptance

1. Shooting a zombie in the body shows a white marker, in the head a sodium one,
   and they read clearly at 1080p without being distracting.
2. Aiming tightens the crosshair; sprinting widens it; both settle smoothly.
3. Walking up to the wall buy shows the prompt, walking away removes it, with no
   flicker when standing at the edge of range.
4. Buying deducts points, the total flashes crimson, and the weapon block
   updates in the same frame.
5. Taking damage darkens the edges of the screen and it recovers with health.
6. Nothing on screen is information the player cannot act on.

## On pass

Phase B is done. Run the Phase B gate: 24 to 40 zombies on the grey box platform
holding 60fps with `stat unit` open. Record the result in
`docs/tasks/NEXT.md` and mark Phase B complete in `docs/tasks/README.md`, then
Phase C begins with the round loop and the train.

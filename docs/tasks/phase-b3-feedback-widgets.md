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
`BeginPlay`. Barebones, clean, classic: the four screen corners and a centre
reticle, nothing else. Thin strokes, generous edge margins, one weight of type,
tabular figures. Let the world read through. `docs/art-direction.md` and the
classic restraint rules mean no kill feed, no floating damage numbers, no
minimap, no powerup banners.

### Elements

1. **Round.** Top left. From `OnRoundStarted`. The number, large, with a small
   "ROUND" label above it in muted grey. No banner, no round change animation
   beyond a quiet fade.
2. **Player name.** Bottom left, above the points. Plain text from the player
   state display name, muted grey, small. No portrait, no level, no icons.
3. **Points.** Bottom left, the primary readout of that corner. Bind
   `OnPointsChanged`. Total in near white, tabular. On a change, briefly show
   the delta beside it, sodium for a gain, crimson for a spend, then fade and
   settle to the new total.
4. **Health.** Bottom left, a thin horizontal bar under the points. Bind
   `OnHealthChanged`. Violet fill on a dark track, no numbers. It drains and
   refills; the regeneration is felt through the bar, not labelled.
5. **Weapon block.** Bottom right. Magazine and reserve from `OnAmmoChanged`,
   magazine large and reserve smaller with a thin divider. Weapon name above,
   small, from the held `ULTWeaponData` `DisplayName`. Text is enough; any icon
   is a flat muted silhouette.
6. **Crosshair.** A small static dot or four-line reticle. Drive its spread from
   `GetCurrentSpreadDegrees()` so it opens on movement and recoil bloom and
   tightens when aiming. It should visibly disappear or shrink to a dot at full
   `GetAimAlpha()`.
7. **Hit marker.** Four short strokes just outside the crosshair, hidden by
   default. Bind `OnHitConfirmed`. On a body hit, show in near white for about
   0.12s. On a headshot, show in sodium `#E0A030`, slightly larger, for about
   0.16s. Drive both with a short animation rather than a tick and a timer.
8. **Interaction prompt.** Bottom centre, clear of the weapon block. Bind
   `OnInteractableChanged`: set the text and fade in when `bAvailable`, fade out
   when not. Key glyph then prompt text, one line, centred. No flicker at the
   edge of range.
9. **Damage vignette.** Not a widget element. It is a post process material
   driven from the existing `OnDamageTaken` Blueprint event on the player. Keep
   it there rather than faking it with a red image in UMG.

### Do not add yet

Perk icons, equipment slots, the challenge tracker, the special weapon meter,
the station schematic, the train countdown, the exfil prompt and the heat
indicator all belong to Phase C or later, once the systems behind them exist.
An empty slot on screen is worse than no slot, so do not reserve space for them.

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

# G2: the restrained HUD

Lane: the widget assembly is CC-in-Unreal (editor only, `WBP_HUD` plus two new
widgets). The binding table and the layout below are settled here so the editor
session does not have to re-derive them. No `Source/` change is required: see
"C++ this does not need" at the end.

Depends on: nothing hard. F5 (signage) and the departure board own the train
countdown, so G2 must not duplicate it. Unblocks: G3 balance, which needs a
readable HUD to balance against.

Read `docs/reference/reference-frame-notes.md` section 3 (the HUD row of the
"must not be copied" table), `docs/design/gameplay-canon.md` section 10, and
the `WBP_HUD` review in `docs/tasks/handover.md` (S7) first.

## Goal

Finish the Phase B3 HUD into the shipping one: fix the five open S7 findings,
move it onto the project typefaces now that they are imported, and add the
three states B3 never had (downed, dead, boarded). Nothing else gets added.

The bar for every element: **you can play well using only what is on screen,
and nothing on screen is decoration.** If an element does not change a decision
the player makes, it does not ship.

## Principles, in priority order

1. **The station carries the information, not the widget.** The train countdown
   is on the departure board (Phase C, F5). Heat is in the light and the spawn
   rate. Round is a number because there is nowhere diegetic to put it.
2. **Four corners and a centre reticle.** No new screen regions.
3. **Nothing at rest.** An element that is not currently telling you something
   is `Collapsed`, not present-but-empty. An empty reserved slot is worse than
   no slot.
4. **Saturation is a resource.** Sodium `#E0A030` for gain and attention,
   crimson `#B02030` for loss and danger, violet `#6C4C9C` for the player's own
   state, off-white for everything else. If everything is coloured, nothing is.
5. **Explicitly not the reference frame's furniture.** No permanent minimap, no
   challenge tracker, no kill feed, no exfil banner, no floating damage
   numbers, no perk or equipment row, no train countdown widget.

## The binding surface

Every value the HUD needs already exists and is Blueprint-visible. This table is
the authority: bind to these exactly, do not add polling on Tick for anything
that has a delegate.

| What | Where it comes from | Kind |
|---|---|---|
| Round number | `ULTRoundManager::OnRoundStarted(int32 Round)` | delegate |
| Round ended | `ULTRoundManager::OnRoundEnded(int32 Round)` | delegate |
| Zombies remaining | `ULTRoundManager::GetZombiesRemaining()` | pure |
| Special round | `ULTRoundManager::IsSpecialRound()`, `GetSpecialRoundTag()` | pure |
| Points total and delta | `ULTPointsComponent::OnPointsChanged(int32 NewTotal, int32 Delta)` | delegate |
| Points total | `ULTPointsComponent::GetPoints()` | pure |
| Affordability | `ULTPointsComponent::CanAfford(int32 Cost)` | pure |
| Health | `ALTPlayerCharacter::OnHealthChanged(float HealthFraction)` | delegate |
| Health | `ALTPlayerCharacter::GetHealthFraction()` | pure |
| Downed | `ALTPlayerCharacter::IsDowned()` | pure |
| Bleed-out clock | `ALTPlayerCharacter::GetBleedOutRemaining()` | pure |
| Magazine and reserve | `ULTWeaponComponent::OnAmmoChanged(int32 Magazine, int32 Reserve)` | delegate |
| Magazine and reserve | `GetMagazine()`, `GetReserve()` | pure |
| Reloading | `ULTWeaponComponent::IsReloading()` | pure |
| Aim alpha | `ULTWeaponComponent::GetAimAlpha()` | pure |
| Crosshair spread | `ULTWeaponComponent::GetCurrentSpreadDegrees()` | pure |
| Hit confirmed | `ULTWeaponComponent::OnHitConfirmed(bool bHeadshot)` | delegate |
| Weapon name | the held `ULTWeaponData::DisplayName` (or `UpgradedName`) | data |
| Interaction prompt | `ULTInteractionComponent::OnInteractableChanged(const FText& Prompt, bool bAvailable)` | delegate |
| Current interactable | `ULTInteractionComponent::GetCurrentInteractable()` | pure |
| Run state | `ALTGameState::OnRunStateChanged(ELTRunState New, ELTRunState Old)` | delegate |
| Run state | `ALTGameState::GetRunState()`, `IsRunOver()` | pure |
| Station name | `ALTGameState::GetStationName()` | pure |

**The one indirection to know about.** `OnDamageTaken`, `OnDowned`, `OnRevived`,
`OnBleedOutExpired` and `OnDied` are `protected BlueprintImplementableEvent` on
`ALTPlayerCharacter`, not assignable delegates. The HUD cannot bind them. The
**player Blueprint** implements each one and forwards to the HUD widget through
a custom event. Wire that forwarding rather than polling `IsDowned()` on Tick.

`ELTRunState` is `PreGame, Active, Downed, Dead, Boarded`.

## Layout

16:9 reference, anchored so it survives other aspect ratios. Everything inside a
5 per cent safe margin. Anchors, never absolute screen coordinates.

| Element | Anchor | Contents |
|---|---|---|
| Round block | top left | `ROUND` label over the number |
| Station name | top left, under the round block | `GetStationName()`, small, muted |
| Player state block | bottom left | health bar, points total, points delta |
| Weapon block | bottom right | name, magazine, divider, reserve |
| Reticle | centre | dot plus four lines, spread driven |
| Hit marker | centre | four short marks |
| Interaction prompt | bottom centre | key glyph plus text |
| Damage vignette | screen edges | post process material, not a widget |
| Downed overlay | full screen | new, see below |
| Run-over card | full screen | new, see below |

The player name in the bottom left is **cut**. S7 finding (4) is that it reads
the machine ID in PIE; the real answer is that a single player already knows
their own name. Delete the widget rather than fixing it.

## Typography

The OFL faces are now imported under `Content/LastTrain/UI/Fonts/`. This closes
S7 finding (2) and makes `docs/art-direction.md` section 7 stale: that section
says Overpass "is not present in the project", which is no longer true. Update
it as part of this task.

- **All tabular numerics** (round number, points, magazine, reserve, bleed-out
  clock): `Font_UI_OverpassMono`. A mono face keeps numbers from jittering as
  they change width, which is the whole reason for it.
- **All labels and body text** (the `ROUND` label, station name, weapon name,
  interaction prompt): `Font_UI_Overpass`.
- Replace every `/Engine/EngineFonts/DroidSansMono` and
  `/Engine/EngineFonts/Roboto` reference in `WBP_HUD`. There should be no
  engine font reference left when this is done.

`WBP_MainMenu` currently uses Barlow Bold for the title and Public Sans for the
buttons. **Decision for the owner:** either the menu moves to the Overpass pair
too, or Barlow Condensed is declared the display face used for titles only and
Overpass is the functional face everywhere else. Either is defensible; the
current mix of three families across two screens is not. Recommend the second:
one display face, one functional pair. Record whichever is chosen in
`docs/art-direction.md` section 7.

## Element behaviour

### Round block

`ROUND` label in muted off-white, number below it large in Overpass Mono.
Bound to `OnRoundStarted`. On a new round the number does a single 0.4s scale
and sodium colour pulse settling back to off-white. On a special round
(`IsSpecialRound()`) the pulse is crimson instead and the tag from
`GetSpecialRoundTag()` appears under the number for 3s, then collapses. That is
the only warning the player gets that round 5 is sprinters, and it is worth one
line of text.

No zombies-remaining counter. `GetZombiesRemaining()` is listed above because
G3 will want it for tuning, not because it goes on screen: knowing the exact
count left turns the last two zombies into a chore rather than a hunt.

### Health

Violet `#6C4C9C` fill on a charcoal `#16161C` track, no numbers. Unchanged from
B3 and it works. Keep the existing damage-flash logic, which fires only when
`HealthFraction` falls, so regeneration ticks do not pulse it.

Add one thing: below about 30 per cent the fill goes crimson `#B02030` and
breathes slowly. The player needs to know they are one hit from downed without
reading a number.

### Points

Total in Overpass Mono, off-white. The delta flashes above it for 0.9s: sodium
for a gain, crimson for a spend. Existing behaviour, keep it.

### Weapon block

Name in Overpass, magazine and reserve in Overpass Mono either side of a
divider. Magazine goes sodium at or below 25 per cent of the magazine size and
crimson at zero. When `IsReloading()` is true the magazine number is replaced by
a row of dots or a thin bar that fills over the reload; drive it from a widget
animation started by the reload, not from a Tick poll, because the reload
duration is on the weapon data and the animation can simply match it.

The whole block collapses when the player holds no weapon.

### Reticle and hit marker

Unchanged from B3. Spread from `GetCurrentSpreadDegrees()`, collapses to the dot
at full `GetAimAlpha()`. Hit marker white for a body hit, sodium for a headshot,
0.12s and 0.16s. This is the most-used feedback in the game and the review found
it correct: do not touch it.

### Interaction prompt

Key glyph plus text from `OnInteractableChanged`. One addition: when the
interactable is a wall buy the player cannot afford, the prompt renders crimson
and the key glyph dims. Get the cost by casting `GetCurrentInteractable()` to
`ALTWallBuy` and testing `ULTPointsComponent::CanAfford`. Blueprint can do this
with no C++ change.

### Downed overlay, new

Fires from the player Blueprint's `OnDowned`, clears on `OnRevived`. Bleed-out
is 30s and the solo auto-revive lands at 8s, so this is on screen for a short
time and must read instantly.

- The main HUD dims to about 40 per cent opacity. The player is not shooting
  well from here; the numbers matter less than the room does.
- A crimson arc or bar at the bottom centre draining over `BleedOutSeconds`,
  driven off `GetBleedOutRemaining()`. No digits: a shrinking bar reads faster
  than a countdown, and this is the one place a numeric clock would pull the
  eye away from the horde.
- The word `DOWN` in crimson above it. Nothing else. No revive prompt while
  `bSoloAutoRevive` is set, because the player cannot do anything to hurry it.
  When a self-revive item or a second player exists, the prompt for it goes
  here and the arc stays.
- Heavy desaturation and a crimson edge vignette through the post process, not
  through widget colour. The screen effect is the message.

### Run-over card, new

Bound to `OnRunStateChanged`.

- On `Dead`: the whole HUD collapses. A centred card on a charcoal wash: the
  station name, `ROUND N` reached, final points. Three lines, Overpass Mono for
  the numbers. Below it a `RETURN TO STATION` button that opens `L_MainMenu`.
  This closes the S13 follow-up that there is currently no route back to the
  menu from a death.
- On `Boarded`: the HUD collapses and the card does **not** show. Boarding is
  not a run ending, it is a level travel, and `ALTGameInstance` carries points
  and weapon across. A brief charcoal fade is all this state needs.
- On `PreGame`: the HUD is collapsed. It reveals on the transition to `Active`.

### Damage vignette

Post process material, already wired through the player Blueprint's
`OnDamageTaken`. Unchanged.

## The five open S7 findings, all folded in

1. **Two leftover `PrintString` debug nodes in the `WBP_HUD` EventGraph.**
   Delete both. Nothing ships with a debug print.
2. **Engine stock fonts.** Swapped to the imported OFL faces per "Typography"
   above, and `docs/art-direction.md` section 7 updated to match reality.
3. **`PointsDelta`, `HitMarkTL..BR` and `DamageVignette` sit at
   `Visibility=Visible` at rest**, relying on empty text and zero render
   opacity. Default all three to `Collapsed` and let the animations set them
   visible on the first frame. This is principle 3 and it is also cheaper.
4. **`PlayerName` reads the machine ID in PIE.** The widget is cut entirely, see
   "Layout".
5. **Palette.** Already compliant where it counts. Confirm after the changes:
   health fill `#6C4C9C`, health track `#16161C`, points delta `#E0A030`,
   low-health and downed crimson `#B02030`. Grey and near-white body text stays
   as the spec-internal allowance.

## Not in G2, and why

- **The station network schematic.** The Phase G plan lists it as the map
  substitute. **Recommend cutting it from the HUD entirely.** v1 ships two
  stations, so there is nothing to navigate and a two-node schematic is
  decoration, which fails the bar at the top of this file. The schematic is
  still worth building as **diegetic platform signage in F5**, where it does the
  legal job of replacing the official line diagram with an original one and
  sells the world. If a later version ships enough stations for route choice to
  be a real decision, it gets its own spec then. Owner decision, flagged.
- **Perk and equipment rows.** No perks exist. Whether v1 ships them is the open
  Phase G decision in `phase-g-audio-hud-balance.md`. If the answer is no, this
  never gets built; if yes, it is a bounded addition to the bottom-left block.
- **A heat indicator.** Heat is meant to be felt through spawn pressure and
  light, not read. Revisit only if G3 playtesting shows players cannot tell heat
  has risen.
- **The train countdown.** Diegetic, on the departure board. Putting it in the
  HUD would undo the best idea in the reference frame.
- **Zombies remaining, kill feed, damage numbers, minimap, challenge tracker,
  exfil banner.** Banned by the style guide.

## C++ this does not need

Every binding above already exists and is Blueprint-visible, so G2 is an editor
task with no `Source/` change and no compile.

One optional addition would improve the reload indicator: a
`BlueprintPure float GetReloadProgress() const` on `ULTWeaponComponent`
returning 0 to 1 through the reload. Without it the widget animation has to be
authored to match `ULTWeaponData`'s reload duration by hand, which drifts if the
G3 balance pass retunes that number. It is a genuinely small change, but this
repo's rule is that C++ is compiled before it is trusted and the remote session
cannot compile, so it is proposed here rather than written. Take it or leave it
when G3 touches the weapon numbers.

## Accept

- Play a full run in `L_CanaryWharf_Greybox` reading only the HUD: you can tell
  your round, your points, your health, your magazine and reserve, and what you
  are standing in front of, without guessing.
- Nothing is on screen at rest that is not telling you something. Screenshot the
  idle HUD and every visible element is load-bearing.
- No engine font reference remains in `WBP_HUD`.
- No `PrintString` node remains in `WBP_HUD`.
- Go down on purpose: the overlay reads instantly, the bleed-out arc drains, the
  auto-revive at 8s clears it cleanly and the HUD comes back to full opacity.
- Die on purpose: the HUD collapses, the card shows the right station, round and
  points, and the button returns to `L_MainMenu`.
- Board the train: the HUD collapses, no death card, the next station loads with
  points and weapon carried.
- Palette check against the four hex values.
- `docs/art-direction.md` section 7 no longer claims Overpass is missing.
- Changed `.uasset` files committed via LFS (verify each is a pointer:
  `git show HEAD:<path> | head -c 45` gives `version https://git-lfs...`).

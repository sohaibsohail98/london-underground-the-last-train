# Phase H1 draft: starting loadout and wall buy progression

Draft written 2026-09-10, per `docs/tasks/phase-h1-starting-loadout.md`. This
is numbers and placement notes only. No `ULTWeaponData` asset is created and
no `ALTWallBuy` actor is placed; that is CC-in-Unreal's editor work once this
draft is reviewed.

## 1. The weapon table

All names are original, no real firearm model names, no Call of Duty style
naming. Field names match `ULTWeaponData` exactly
(`Source/LastTrain/Public/Weapons/LTWeaponData.h`). `DA_Weapon_SMG` ("Stag
Compact") is included as the existing reference point: 34 `BaseDamage`, 2.5x
`HeadshotMultiplier`, 1 `Penetration`, 600 `RoundsPerMinute`, automatic, 1
`PelletsPerShot`, 30 `MagazineSize`, 240 `MaxReserve`, 2.1s `ReloadSeconds`,
per `docs/design/gameplay-canon.md` section 9.

| Weapon | Role | BaseDamage | HeadshotMultiplier | Penetration | RoundsPerMinute | bAutomatic | PelletsPerShot | MagazineSize | MaxReserve | ReloadSeconds | Weapon cost | Ammunition cost | Placement |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **Halyard Sidearm** | Starter | 28 | 2.2x | 1 | 260 | No | 1 | 10 | 60 | 1.6s | **0 (issued, not bought)** | 150 | Carried from the start, no wall placement |
| **Stag Compact** *(existing `DA_Weapon_SMG`)* | Sidegrade | 34 | 2.5x | 1 | 600 | Yes | 1 | 30 | 240 | 2.1s | 750 | 300 | Near spawn |
| **Marlow Pump** | Sidegrade | 22 per pellet | 2.0x | 1 | 75 (non-auto, cycled) | No | 6 | 6 | 24 | 3.4s | 1000 | 200 | Mid platform, train-adjacent |
| **Coldharbour Rifle** | Upgrade | 46 | 2.8x | 2 | 450 | Yes | 1 | 25 | 150 | 2.6s | 2500 | 500 | Far from spawn |

Notes on the numbers:

- **Halyard Sidearm** deliberately undercuts `DA_Weapon_SMG` on every firing
  stat (lower damage, far lower rate of fire, a 10 round magazine) so it
  reads as a genuine starter rather than a stealth-free upgrade. Its
  `HeadshotMultiplier` is kept close to the SMG's (2.2x against 2.5x) so a
  precise player is never punished for using the free weapon well; the
  differentiator from the SMG is sustained output, not accuracy reward.
- **Marlow Pump** is the project's shotgun archetype (`PelletsPerShot = 6`,
  `BaseDamage` per pellet 22, so a full six-pellet hit at close range is 132,
  well above the SMG's single-shot 34) with a small magazine and a semi-auto,
  non-automatic cycle to keep it a close-range specialist rather than a
  straight upgrade. `Penetration = 1` because a shotgun blast dumping its
  energy into the first target it hits reads correctly, unlike a rifle round.
- **Coldharbour Rifle** is the round tier upgrade: the highest `BaseDamage`
  and the only weapon with `Penetration = 2`, so it is the pick for a player
  choosing to punch through a line of zombies rather than deal with them one
  at a time. Its 450 `RoundsPerMinute` sits below the SMG's 600 deliberately:
  the upgrade axis here is damage and penetration, not rate of fire, so the
  SMG stays relevant as the higher-DPS, lower-per-shot option rather than
  being made obsolete outright.

## 2. The starting pistol: free, not trivially cheap

`phase-h1-starting-loadout.md` allows "cheap or free to start with"; this
draft picks **free, issued at spawn**, not a nominal wall price. Reasoning:

- The design intent already on record (`gameplay-canon.md` section 9, and the
  accept criterion itself) is "the player should not need to buy their first
  weapon" and "always has something to fight with". A wall-priced starter,
  even a cheap one, means a round can begin with the player unarmed if they
  spend down to a low balance during a purchase phase (points are never lost,
  but the player character's held weapon slot has to start populated by
  something, and the only mechanism that guarantees that today is
  `BP_PlayerCharacter`'s configured starting weapon, not a wall buy at all).
- Making the Halyard Sidearm the actual `BP_PlayerCharacter` starting
  `ULTWeaponData`, the same way the character is "today, effectively
  `DA_Weapon_SMG` if anything" per the problem statement, removes the
  ambiguity entirely rather than trusting the player to always have 150
  points spare. This is an editor configuration step for CC-in-Unreal, not
  something this draft can set, but the number in the table (cost 0) reflects
  that it is issued, not sold.
- The Halyard Sidearm is **still worth listing at a wall buy for ammunition
  only** if the player runs the reserve dry and wants a refill without
  switching away from it; `ALTWallBuy` already supports "buying a weapon you
  already hold instead refills its reserve at the ammunition cost" per
  `gameplay-canon.md` section 8, so no new mechanic is needed, only a wall
  buy placement carrying it, priced in section 4 below.

## 3. Does the 500/10/60/130 economy need to change

**Holds as designed. No change recommended.** Reasoning against each
drafted price:

- **500 starting points** was flagged in the spec as a possible concern
  because a 500 point starter weapon costs the entire starting bank in one
  purchase. This draft avoids that collision by making the starter free
  (section 2), so the concern the spec raised does not apply to this draft's
  numbers: the player's first 500 points are free to spend on the Stag
  Compact (750, needs one round's kills on top of the start) or held back for
  ammunition and the Marlow Pump.
- **10 points per hit, 60 per kill, 130 per headshot kill** are unchanged,
  and this draft's wall prices are sized against them directly (see section
  5's kill counts). None of the four weapons needs a price above roughly two
  rounds' worth of kills to stay reachable, which keeps the existing pacing
  intent (buy your first upgrade inside round 1 to 2, the top upgrade by
  round 3 to 4) without touching the per-event point values.
- Raising per-kill points to make expensive weapons reachable faster would
  also inflate the ammunition economy and any future perk/upgrade bench
  pricing (`gameplay-canon.md` notes the upgrade bench at 5000, lost property
  at 950 plus 10 percent per use) since those are all denominated against the
  same per-kill numbers. Changing the base economy to fit four new weapon
  prices is solving the wrong problem; sizing the four prices against the
  fixed economy is the correct order of operations, which is what this draft
  does.

## 4. Ammunition cost per weapon

The existing `ALTWallBuy` default (`AmmunitionCost = 250`) was tuned against
nothing in particular; `ULTWeaponData` itself separately carries `WallPrice`
(500 default) and `AmmoPrice` (250 default) as per-weapon fields already, so
the "uniform 250" the spec flags is the class default on an unpopulated data
asset, not a designed number. This draft prices `AmmoPrice` per weapon
against `MagazineSize` and `MaxReserve`:

| Weapon | MagazineSize | MaxReserve | Reserve as multiple of magazine | Ammunition cost | Reasoning |
|---|---|---|---|---|---|
| Halyard Sidearm | 10 | 60 | 6x | 150 | Cheapest reserve pool in absolute rounds (60), priced low so the free starter never becomes a trap the player cannot afford to keep feeding. |
| Stag Compact | 30 | 240 | 8x | 300 | Baseline. Set above the `ALTWallBuy` class default of 250 because 240 rounds at 600 rpm is a large reserve (8 full magazines); 300 keeps a full refill costing roughly 5 kills, in line with its 750 weapon price sitting at roughly 12 to 13 kills (see section 5).
| Marlow Pump | 6 | 24 | 4x | 200 | Smallest reserve in absolute rounds (24, only 4 magazines) but each pellet volley is disproportionately powerful, so pricing it below the SMG's 300 keeps a shotgun run affordable despite the small magazine forcing more frequent reloads and, if the wall buy is used mid-fight, more frequent ammunition purchases. |
| Coldharbour Rifle | 25 | 150 | 6x | 500 | The upgrade tier weapon should cost more to feed as well as more to buy: 500 for a 150 round reserve is roughly double the SMG's cost for a smaller absolute reserve, which is intentional, this is the weapon a player commits to once they can already afford to spend heavily. |

The reserve-to-magazine ratio (4x to 8x across the four weapons) is
intentionally not flattened to one number: a shotgun with a small magazine
and few reserve shells should feel scarce by design, while the SMG's high
rate of fire needs a genuinely large bank of reserve rounds to avoid feeling
like it starves itself artificially. Ammunition cost tracks the reserve size
and the weapon's price tier together, not magazine size alone.

## 5. Pacing check against the 10/60/130 economy

Stated as "roughly N kills at round M" per the spec's request, assuming a mix
of body and headshot kills roughly matching how the round manager's roster
plays out (a player who lands some headshots but not every kill), so figures
below use an approximate blended 75 points per kill rather than assuming
either pure 60 or pure 130:

- **Stag Compact, 750 points.** Starting from the free 500 point bank spent
  on nothing else, this needs roughly 250 more points, about 3 to 4 kills.
  Comfortably inside round 1. Its 300 point ammunition refill after that is
  roughly 4 more kills, so a player can buy the SMG and refill it once before
  round 2 breathing room typically ends.
- **Marlow Pump, 1000 points.** From a zero saved balance (having spent the
  starting 500 on the SMG already, for example) this is roughly 13 kills, so
  realistically a round 2 to round 3 purchase for a player upgrading past
  their first buy, or a round 1 to 2 purchase for a player who saves the
  starting 500 rather than spending it on the SMG. Its 200 point ammunition
  cost is under 3 kills, cheap to sustain once bought.
- **Coldharbour Rifle, 2500 points.** Roughly 33 kills from zero, which reads
  as a round 3 to 4 purchase given the round manager's escalating zombie
  counts, matching the design intent that the top upgrade should not be
  reachable in the first couple of rounds. Its 500 point ammunition cost is
  under 7 kills, proportionate to its higher price tier.
- **Halyard Sidearm ammunition refill, 150 points.** Under 2 kills, by design
  the cheapest single purchase in the game, since it is the safety-net
  weapon a player falls back to if they cannot afford anything else.

## 6. Placement relative to the existing station layout

Placement notes only, per `docs/reference/canary-wharf-grid.md`'s tile
legend, not literal transforms. The grid already marks four `W` wall buy
anchor tiles: two on the platform level (row 11, left and right, near the
`X` barrier/gate line and roughly equidistant from the two `S` spawn
clusters at the top of the platform) and two on the lower concourse level
(row 68 area, left near `L` lost property and right near `U` upgrade bench,
past the escalator bank).

| Weapon | Suggested anchor | Placement note |
|---|---|---|
| Stag Compact | One of the two platform-level `W` anchors (row 11) | **Near spawn.** The platform level is where the horde funnels in from the `T` tunnel mouths at the top of the grid; putting the first purchasable upgrade here means a player who spends their starting points immediately does so without leaving the main fighting space, consistent with "buy your first upgrade inside round 1". |
| Marlow Pump | The other platform-level `W` anchor (row 11), or the platform-level anchor nearest the train volume rows 8 to 10 | **Train-adjacent.** A close-range, high-burst weapon suits the tight quarters near the train wall and the boarding zone, where fights are more likely to be at point-blank range than down the long sightline toward the tunnel mouths. |
| Coldharbour Rifle | One of the two concourse-level `W` anchors (row 68 area) | **Far from spawn.** Reaching the concourse means crossing the escalator bank and the flooded zone, so this is already the farthest-travelled purchase in the station, matching its round 3 to 4 price tier: a player has to commit to leaving the platform's relative safety to afford and reach it. |
| Halyard Sidearm (ammunition only) | The remaining concourse-level `W` anchor | **Far from spawn, secondary.** Placed opposite the rifle rather than duplicating a platform-level anchor, so a player who has pushed to the concourse for the rifle can also top up their starter's reserve on the same trip, rather than needing to backtrack to the platform. |

This uses all four existing `W` anchors on the grid with no new anchor
needed. If a fifth wall buy is ever wanted (for example, to duplicate the SMG
or the pump on both sides of the platform for a two-player or larger-arena
future), that would need a new anchor tile added to the grid, out of scope
here.

# Gameplay canon

The settled design of LAST TRAIN, stated tightly. This is the single reference
for the round loop, the train, station heat, the zombie roster, the economy, the
player mechanics and the HUD. It replaces the 167 item `open-questions.md` sweep:
every ambiguity in that sweep was resolved to its suggested default, and the
resolved values are folded in here.

Rule of the road for numbers:

- **Coded** means the value is in `Source/LastTrain/` today and is authoritative.
- **Provisional** means the value is agreed as a working default but expected to
  move in the Phase G balance pass or once a frame profile exists. Provisional
  numbers are marked inline. Do not treat a provisional number as locked.

Design lineage: the round loop, train timing, station heat, the five zombie
types, the mechanic library and the economy come from `docs/brief-v2.md`
(design-authoritative, engine-dead). Engine, camera, phases and the model split
come from `docs/brief-v3-unreal.md`. Line identity is settled in
`docs/brief-v3-unreal.md` Part 1.

---

## 1. What the game is

First person, round based zombie survival on a fictionalised Crossrail-scale
London rail line. A single station is the arena. A train arrives on a timer and
dwells; boarding it is an optional escape to an adjacent station. Staying raises
the station's heat.

- **Camera:** first person, base FOV 95 (coded, `BaseFieldOfView`), narrowing to
  the weapon's `AimedFieldOfView` (65 on the current weapon) while aiming.
- **Line identity:** a fictionalised main-line loading-gauge line modelled on the
  Elizabeth line, NOT a deep-level tube. Rolling stock is the Class 345 "Aventra"
  silhouette: walk-through, roughly 205 m for a 9-car unit, near-vertical box
  sides, curved wraparound cab front, three double-leaf plug doors per side per
  car. The line is renamed in-world; station names and real geography stay
  factual. Full detail in `docs/brief-v3-unreal.md` Part 1 and
  `docs/reference/canary-wharf-research/rolling-stock.md`.
- **v1 scope:** exactly **2 stations**. Any "41 stations" reference anywhere in
  the docs is an aspirational expansion list, not a plan. Station 1 is Canary
  Wharf (tier 4, flood + interchange). Station 2 and its mechanic pair are not
  yet named; picking them is an open decision (see section 11).
- **Legal:** no roundel, no Johnston or New Johnston typeface, no reproduction of
  the official line diagram, no operator livery or logo, no transcribed
  announcement recordings, no Call of Duty weapon or perk names. All wayfinding,
  advertising and rolling stock livery is original work. Palette is exactly
  `#16161C` charcoal, `#6C4C9C` violet, `#E0A030` sodium, `#B02030` crimson,
  plus neutrals.

---

## 2. The round loop

Coded in `ALTRoundManager`.

- Rounds are numbered from 1 and **never reset** for the run.
- Round N spawns a fixed count of zombies. Opening counts are `{6, 8, 10, 12,
  14}` (coded `OpeningRoundCounts`); past that a formula adds `CountGrowthPerRound
  = 2.4` per round (coded) on top of the last opening value.
- A round ends when every spawned zombie of that round is dead. Then a
  **breather** of `BreatherSeconds = 10` (coded), then the next round starts.
  The breather never drops below 8s so the upgrade bench and gunsmith stay
  usable within it (provisional floor).
- Spawn cadence decays per round: `effective_interval = BASE_INTERVAL *
  ROUND_DECAY^(round-1)`, with `BASE_INTERVAL = 1.6`, `ROUND_DECAY = 0.96`,
  `MIN_INTERVAL = 0.35` (all coded). Heat multiplies this further, section 4.
- Spawn points are hand-placed `ALTSpawnPoint` actors read from the level, so the
  round manager is station-agnostic. Each carries a `Weight` (relative traffic),
  a `FirstRound` (earliest active round), a `CooldownSeconds` gate and an
  `AreaTag` for door gating.
- Concurrent live cap: `concurrent_cap(heat) = BASE_CAP + HEAT_CAP_ADD * heat`.
  `BASE_CAP = 24` (coded `MaximumAlive`), **provisional**, to be derived from a
  real frame profile. `HEAT_CAP_ADD` is discussed in section 4.
- Round-end has genuine stall detection rather than a hard time ceiling: a zombie
  that loses the player entirely paths to its last known location, then walks to
  the nearest spawn point and de-spawns silently, so `LiveZombies.Num() == 0`
  stays reachable. **Not yet built**; required alongside the Phase C roster.

No game-start cinematic. The first round begins on level `BeginPlay` via the
game mode (`bAutoStart = true` on `ALTGameMode`, coded). A short pre-round
establishing beat is allowed but not required, and is absorbed into the first
train's `FirstTrainStopSeconds`, not added to it.

---

## 3. The run lifecycle

Coded in `ALTGameState` (`ELTRunState`) and driven by `ALTGameMode`.

| State | Meaning |
|---|---|
| `PreGame` | Level loaded, rounds not started. |
| `Active` | Rounds running. The normal play state. |
| `Downed` | Player is downed. Rounds continue. A revive returns to `Active`. |
| `Dead` | Player is dead. The run is over. |
| `Boarded` | Player boarded the train. The arena is finished; travel to the next station is a separate transition. |

- The game mode owns the transitions: `StartRun`, `NotifyPlayerDied`,
  `NotifyPlayerDowned`, `NotifyPlayerRevived` (all coded and `BlueprintCallable`).
  `NotifyPlayerBoarded` is a Phase C addition (section 5).
- `Downed` vs `Dead` is currently a single `bDead` flag on the player. Splitting
  it into a proper `bDowned` -> bleed-out -> `bDead` path with a
  disengage-based bleed-out (not "kill N while crawling") is agreed design,
  **not yet built** (Phase E, downed and revive).
- Travel between stations, the loading state, a `ULTGameInstance` travel payload
  and rehydrating a fresh arena are all Phase C "travel" work.
  `NotifyPlayerBoarded` is the seam it hooks into.

---

## 4. Station heat

Coded in `ULTStationHeat` (component on the round manager, or found on any actor
in the level; optional, absent means base cap and rate).

- Heat is an int, starts at 0.
- **Accrual:** +1 each time a train departs with the player still in the station.
  So it ticks roughly once per train interval (about 100s), which drifts from the
  round number as rounds lengthen. Resets to 0 on travel. Persists through a
  down and revive.
- **Live cap bonus:** `GetLiveCapBonus() = heat * LiveCapPerHeat`, with
  `LiveCapPerHeat = 6` (coded), clamped by `MaximumHeat = 10` (coded).
- **Spawn rate:** `GetSpawnRateMultiplier() = 1 + heat * SpawnRateFractionPerHeat`,
  with `SpawnRateFractionPerHeat = 0.12` (coded). The round manager divides the
  round's spawn interval by this, so a higher rate is a shorter interval.
- **Roster pressure:** from heat 3+, the special-type spawn weights (sprinter,
  screamer, crawler) roughly double. Agreed design, lands with the Phase C
  roster.
- Heat's only job is pressure. It adds no new rules.

**Provisional retune, pending a frame profile:** `open-questions.md` 5.1 proposes
replacing the coded numbers with a profile-derived model: `HEAT_CAP_ADD = 4`,
`HEAT_MAX = 5` (worst case `24 + 20 = 44` concurrent), and a single
`HEAT_RATE_MULT = 0.90` per heat level applied once to the final rate instead of
the `+12%` fraction. That retune is its own task once the section-18 frame budget
exists. Until then the coded `6 / 10 / 0.12` values stand and Phase C does not
touch them.

---

## 5. The train

The signature system. `ALTTrain` (Phase C1, spec `docs/tasks/phase-c1-train.md`)
is **not yet built**; the design below is settled.

- **Timing:** arrives every `TrainInterval = 100s`, dwells `DwellDuration = 25s`
  (from brief-v2, measured stop to start). Phases: `Away` -> `Approaching` ->
  `Dwelling` -> `Departing` -> `Away`.
- **First train:** the first arrival cannot count "departure to arrival". The
  train stops at `level load + FirstTrainStopSeconds` (default 30), then
  `TrainInterval` governs every cycle after.
- **Doors:** open `DoorOpenDelaySeconds` (default 1s) after the stop, close
  `DoorCloseLeadSeconds` (default 3s) before departure. The boarding window is
  the open-door gap, about 21s on the defaults.
- **Announcements:** an inbound announcement fires `InboundAnnouncementLeadSeconds`
  (default 15s, from brief-v2) before the train stops; an on-arrival line on the
  stop; a "stand clear of the doors" line with door close. Exact wording and the
  voice source are a later audio task. No announcement text lives in C++, and
  none may transcribe or imitate a real TfL recording.
- **The countdown is diegetic only.** It lives on a departure board actor
  (`ALTDepartureBoard`, a later Phase C task), on platform displays and in the
  announcements. It is never a HUD element. The train exposes
  `GetSecondsUntilArrival()` and `GetSecondsUntilDeparture()` for the board to
  read.
- **Boarding** is an interact: `ALTTrain` implements `ILTInteractableInterface`,
  so the existing `ULTInteractionComponent` drives the "Board train" prompt.
  `CanInteract` is true only while `Dwelling`, doors open, run not over. A single
  interact commits; there is no hold-to-confirm and no destination picker yet
  (both deferred to the travel task, which will add the picker and a target-
  station argument to `NotifyPlayerBoarded`).
- **What boarding does in-arena** (`ALTGameMode::NotifyPlayerBoarded`): stop the
  rounds, refill the weapon reserve to full (magazine untouched), reset station
  heat to 0, flip run state to `Boarded`. Live zombies are left in the world
  (harmless; the arena is about to be torn down on travel). Travel itself is not
  wired yet.
- **"Banks points"** from brief-v2 resolves to a **no-op**: there is no
  points-at-risk mechanic, points are never lost, and `NotifyPlayerBoarded` does
  nothing to `ULTPointsComponent`. A points-to-Oyster-Credit conversion on board
  is a separate meta-economy proposal that needs its own sign-off.
- **Not boarding** raises heat by exactly 1 on the `Dwelling -> Departing`
  transition, once per departure.
- **Out of scope for C1:** the trackbed as a kill volume, "stand clear" klaxon,
  platform screen doors as level geometry, the train as a nav obstacle,
  track-side spawn gating while the train is present, degrading the arrival
  spectacle over a long run. All noted, none built.

---

## 6. The five zombie types

brief-v2 names walker, sprinter, brute, crawler, screamer. There is **one rigged
humanoid mesh**; the types are distinguished by scale, colour tint, animation
play rate and one behaviour each, not by five bespoke characters.

Mechanism (agreed): a `ULTZombieTypeData` `UPrimaryDataAsset`, one per type,
read on spawn by `ALTZombieCharacter::ApplyTypeData`. Spec
`docs/tasks/phase-c-zombie-types.md`. **Not yet built.**

**All stat numbers below are provisional** and settle in the Phase G balance
pass. The walker column IS the current C++ default.

| Type | HP mult (round-1 HP) | Base speed | Dmg | Cooldown | Mesh scale | First round | Normal-round weight | Signature trait |
|---|---|---|---|---|---|---|---|---|
| Walker | 1.0 (150) | 130 | 24 | 1.3s | 1.0 | 1 | 100 | none |
| Sprinter | 0.55 (83) | 500 | 18 | 1.0s | 0.95 | 5 | 0 (100 on sprinter rounds) | genuinely fast: between the player's `WalkSpeed 420` and `SprintSpeed 640`, so it catches a walking or reloading player and is only lost by a clean sprint. 0.35s wind-up telegraph. |
| Brute | 5.0 (750) | 95 | 45 | 2.2s | 1.5 | 10 | 0 (pair on x10 rounds) | front armour plate absorbs body shots until ~200 accumulated body damage, then normal damage. Headshots and rear hits always land. No knockback, no blanket stagger immunity. |
| Crawler | 0.35 (52) | 150 (crawl) | 20 | 1.1s | 0.5 (prone) | 8 | 12 from round 8 | low profile: head reachable only by a crouched player, body reachable standing. No death gas in the base design. |
| Screamer | 0.8 (120) | 110 | 10 | 1.5s | 1.0 | 12 | 6 from round 12, max 1 alive | on clear line of sight to the player camera for 2 continuous seconds, screams: summons one extra wave of 4 walkers. The scream is loud and directional and masks other audio while it lasts. No post-death deafen. Kill it fast. |

Effective HP compounds: `ApplyRoundScaling` sets `Health = BaseHealth *
1.1^(round-1)` (coded), and the type multiplier stacks on that. A round-10 brute
is roughly 1770 HP, about 52 body hits from the coded weapon, so the armour weak
point is essential counterplay.

**Special rounds:**

- Every 5th round (5, 15, 25, ...): the whole roster is sprinters, count reduced
  to 75% of the normal curve. Crawlers and screamers never appear on special
  rounds.
- Every 10th round (10, 20, ...): a normal walker round plus 2 brutes spawned at
  roughly 30% and 70% through the count. Rounds 20, 30, ... are both a sprinter
  round and a brute pair.

**Deferred alongside the roster** (agreed, not built): per-type locomotion sets
(a scale-1.5 brute, a prone crawler and a 500-speed sprinter cannot share one
walk cycle), per-instance gait jitter, the leash/de-spawn behaviour in section 2,
per-type navmesh and capsule overrides, the crawler death gas (currently **out**;
resurrecting it is new design), zombie vocalisation.

Screamer line-of-sight test: a trace from the screamer head bone to the player
camera on `ECC_Visibility`, blocked by geometry and by the flood water plane
above 120 cm, NOT blocked by other zombies, held clear for 2 continuous seconds.
Darkness is not occlusion.

---

## 7. Zombie AI

Coded in `ALTZombieCharacter`. Navigation is UE5 navmesh via
`AIController::MoveToActor`, re-issued on a jittered cadence
(`RepathIntervalSeconds = 0.35`, `RepathJitterFraction = 0.4`, coded), not every
tick and **not a flow field** (that was the discarded web build).

- Local separation is RVO avoidance (`AvoidanceConsiderationRadius = 45`, coded).
- A direct-push fallback drives the zombie straight at the target when the
  navmesh cannot path there or before an AI controller is possessed.
- A stall-recovery pass (`UpdateStallRecovery` and friends, coded) detects the
  path-following-succeeds-but-pinned-by-a-pawn case and applies a lateral nudge
  so a corridor queue does not freeze behind the front attacker. Note: the
  2026-09-06 NeoStack run found an edge case where the nudge does not fire for a
  single zombie a short distance outside `AttackRange`; a follow-up C++ pass is
  open.
- The target is always the local player pawn. Anti-kite behaviour (a fraction of
  the horde pathing to a predicted intercept, spawn-point bias ahead of the
  player's velocity, the sprinter as the anti-kite tool, a sprinter dropping to
  walker speed on lost line of sight) is agreed design direction, **not built**;
  it is Phase C/G work.
- Attack: root-to-root distance `<= AttackRange` (130, coded), `AttackCooldown`
  at 0, valid target, then `UGameplayStatics::ApplyDamage`.

---

## 8. Economy

Coded in `ULTPointsComponent`.

| Event | Points | Source |
|---|---|---|
| Start of run | 500 | `StartingPoints` |
| Hit | 10 | `PointsPerHit` |
| Kill (body) | 60 | `PointsPerKill` |
| Kill (headshot) | 130 | `PointsPerHeadshotKill` |

- `TrySpend(cost)` refuses when unaffordable and changes nothing. `CanAfford`
  does not spend. Callers never test affordability separately.
- No points cap.
- **Wall buy** (`ALTWallBuy`, coded): `WeaponCost = 500`, `AmmunitionCost = 250`.
  Buying a weapon you already hold instead refills its reserve at the ammunition
  cost. Prompt text uses "Ammunition", never "Ammo".
- **Perk, mystery box, upgrade bench, door prices:** provisional, from brief-v2
  where stated (upgrade bench 5000, lost property base 950 +10% per use resetting
  on travel). Perk prices specifically are a proposal needing sign-off. These
  land in Phase E.
- **Oyster Credit** (1 per round survived, persistent, spends on attachment
  unlocks) is meta-progression, Phase E or later, scope not finalised.

---

## 9. Player and weapons

Player coded in `ALTPlayerCharacter`.

- Health 100 (`MaxHealth`), regen begins `RegenerationDelaySeconds = 4` after the
  last damage at `RegenerationPerSecond = 20`, instant death at zero. All coded.
- Movement: `WalkSpeed = 420`, `SprintSpeed = 640` (coded). Sprinting cancels
  aim rather than blocking it. There is **no sprint stamina** and no fall damage;
  adding either is a proposal, not a confirmed ambiguity, and the sprinter type
  plus arena design are the intended answer to lazy kiting instead.
- Enhanced Input actions (`Move`, `Look`, `Jump`, `Sprint`, `Fire`, `Aim`,
  `Reload`, `Interact`, `Melee`) are `EditDefaultsOnly` and null until a
  Blueprint assigns them.

Weapons coded in `ULTWeaponComponent` + `ULTWeaponData`.

- Hitscan on `ECC_GameTraceChannel1` (the `Weapon` trace channel, created by
  hand in the editor). Hip and ADS spread, movement and recoil bloom, pellets,
  penetration, distance falloff, reload, reserve refill.
- The one authored weapon (`DA_Weapon_SMG`, display name "Stag Compact") uses the
  class defaults: 34 base damage, 2.5x headshot, 600 rpm, 30 magazine, 240
  reserve, 2.1s reload. All `EditDefaultsOnly` and **provisional** pending the
  Phase G balance pass.
- The full weapon roster (count, archetypes, tiers, which need new code such as a
  projectile system for a crossbow) is a proposal, shape only. The ~130 stat
  numbers are not to be data-entered until Phase G.
- **Melee is settled and coded** on `ALTPlayerCharacter`, not on the weapon: a
  contextual bash with whatever is already held, not a weapon slot, not a knife
  with its own ammunition or equip state. `PerformMelee()` traces `MeleeRange`
  150 from the view point, applies a flat `MeleeDamage` 50 through the same
  `ALTZombieCharacter::ReceiveShot` call the hitscan path uses, and is gated
  only by `MeleeCooldownSeconds` 0.8 and the run state, never by ammunition,
  because its whole purpose is the moment the magazine and the reserve are both
  empty. It fires `OnMeleeHitConfirmed` (bool headshot, the same shape as
  `OnHitConfirmed`) on a connection and the `OnMeleeSwing` Blueprint hook on
  every allowed strike. All three numbers are `EditDefaultsOnly` and
  **provisional** pending the Phase G balance pass; 50 damage is three strikes
  to a round one walker at 150 `BaseHealth`. The key binding, the swing
  animation and the impact sound are editor work: see
  `../tasks/phase-h3-melee-presentation.md`.
- Two-weapon carry is an unimplemented feature scoped for Phase C/E, not a bug.

---

## 10. HUD

Built as `WBP_HUD` (Phase B3), added to the viewport by the player Blueprint on
`BeginPlay`. Restrained and diegetic in spirit: four screen corners plus a centre
reticle, nothing else.

| Element | Corner | Source |
|---|---|---|
| Round number ("ROUND N") | Top left | `OnRoundStarted` |
| Player name | Bottom left | player state display name |
| Points total (with a brief delta flash: sodium gain, crimson spend) | Bottom left | `OnPointsChanged` |
| Health bar (violet fill, no numbers) | Bottom left | `OnHealthChanged` |
| Weapon block (magazine, reserve, name) | Bottom right | `OnAmmoChanged`, held `ULTWeaponData` |
| Crosshair (spread driven by `GetCurrentSpreadDegrees`, collapses at full `GetAimAlpha`) | Centre | `ULTWeaponComponent` |
| Hit marker (white body, sodium headshot, ~0.12-0.16s) | Centre | `OnHitConfirmed` |
| Interaction prompt (key glyph + text, fades on availability) | Bottom centre | `OnInteractableChanged` |
| Damage vignette (post-process material, not a widget) | screen edges | `OnDamageTaken` |

Explicitly **not** the reference frame's HUD furniture: no permanent minimap, no
challenge tracker, no kill feed, no exfil banner, no floating damage numbers, no
perk or equipment rows, no train countdown widget. Deferred elements (perk icons,
heat indicator, station schematic, train countdown board) wait until the systems
behind them exist. An empty reserved slot is worse than no slot.

UI typeface: Overpass and Overpass Mono are the intended choice (OFL, transit
heritage, not a Johnston clone). Not yet in the project; `WBP_HUD` currently uses
engine `DroidSansMono` for numerics and `Roboto` for body text as a stand-in.
See `docs/art-direction.md` section 7.

---

## 11. The mechanic library and stations

brief-v2 fixes ten station mechanic ids: `flood`, `blackout`, `narrow`,
`escalator_rush`, `platform_split`, `open_concourse`, `depot`, `open_air`,
`interchange`, `terminus`. Only the four that v1 could use have any agreed
mechanical definition work outstanding: `flood`, `interchange`, `blackout`,
`platform_split`. The other six are expansion documentation, not v1 scope.

Agreed adjustments for the v1 mechanics: flood rises slower and no longer blocks
firing; the interchange escalator is de-exploited so it is not a safe kill
funnel.

**Open decisions that still need the owner:**

- Name station 2 and its mechanic pair. A tonally contrasting station (bright,
  run-and-gun, teaching range) is worth considering over a second dark one.
- Name the in-world line. This blocks all announcement text and the schematic UI
  and is a CI/legal check.
- The section 18 frame budget and the crowd-system decision (per-agent
  AIController + RVO as now, DetourCrowd, or Mass; skeletal vs vertex-animation
  meshes). `BASE_CAP`, `HEAT_CAP_ADD` and the per-station cap overrides all wait
  on this.
- Downed-and-revive as a coherent solo mechanic (Phase E).
- The music question: whether there is any, and where it plays.

---

## Provenance

Distilled from the 167-item open-questions sweep (2026-09-06), all resolved to
the suggested default. Git history holds the full sweep, the adversarial review
and the parsed JSON at commit `0b7e35f` (`docs/design/open-questions.md`,
`open-questions-review.md`, `open-questions.json`), the last commit before they
were removed.

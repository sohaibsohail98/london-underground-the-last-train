# Fresh-eyes review of open-questions.md - 2026-09-06

Adversarial review. Findings are things the audit missed, got wrong, or under-specified.

## Verdict in one paragraph

The audit is a strong, genuinely useful sweep of the *systems* it chose to look
at, and roughly two thirds of its 98 items are solid enough to lock with only
minor edits. But it is not yet fit to generate a locked spec, for four systemic
reasons. First, it under-serves the non-systems half of the game: audio is one
five-item section for an entire discipline that brief-v3 makes a whole phase,
art is seven items that are mostly "someone should design this later" rather than
resolved questions, and there is no section at all for performance budget, the
crowd design constraint, onboarding, the anti-training AI problem that every
round-based survival game must solve, or telemetry and balance methodology.
Second, its "Suggested defaults" quietly invent a large amount of new game
design (a stamina system, melee, self-revive budgets, signal flares, a fintech
advertising brand, an "Inspector" boss, a full 12-weapon roster with numbers)
and present it in the same register as "confirm 500/10/60/130", so a reader
cannot tell a recorded decision from a fresh proposal that needs its own review.
Third, several defaults are mutually inconsistent or ungrounded (heat maths in
5.1 vs 2.7 vs 4.x, `MaximumAlive` picked three different ways, the sprinter speed
of 340 with no reference, the 4-minute round ceiling). Fourth, a few "currently
in code" claims are imprecise in ways that matter (the wall-buy price wiring, the
`OnAttackWindUp` hook that is claimed absent when the real gap is different, the
per-pellet award which the audit gets right but frames as a smaller problem than
it is). The priority list is directionally right but buries at least two Phase C
blockers (the train's platform-screen-door question, the Elizabeth-vs-Jubilee
rolling-stock contradiction) and over-rates one item (10.2 stamina). Recommended:
one more sweep pass adding the missing sections and reconciling the heat and cap
numbers, then lock.

## Section-by-section

### Section 1: Game start and round loop presentation

**Missed:**
1. No item on **what "the run" is scoped to**. The audit assumes endless-until-death
   (confirmed only in 13.5) but section 1 never states whether a session is one
   station or the whole line, whether there is a target round, or what "win" means.
   This is the framing every other item in section 1 hangs off and it is implicit.
2. No item on the **first-ever-launch vs subsequent-launch** difference. 11.6
   touches onboarding but section 1 owns "level load to round 1" and never asks
   whether a first run gets a different opening beat (a longer establishing hold,
   a one-time line of text) from run 50.
3. **Pre-round beat and the train timer interact and the audit half-notices.**
   1.1's default starts the train timer at level start so the first train is
   "~100s away", but 4.2's default measures the interval departure-to-arrival, and
   there has been no prior departure. Which zero does the first train count from,
   and is the 5s pre-round hold added to it or absorbed? Unresolved across 1.1
   and 4.2.
4. No item on **pause during the pre-round beat or the breather** (11.3 disables
   pause only during train windows and the boarding hold). Can you pause during
   the 5s establishing hold? During the breather? Matters for the "no save, quit =
   over" arcade framing.
5. No item on **round-number display cap / rollover**. 7.4 caps points; nothing
   caps or defines the round readout at round 100+, which a farming player reaches.
6. **Downed presentation (1.7) does not cover the camera.** 10.5 later adds
   "camera lowered to ~30cm" but 1.7, the primary item, says nothing about view
   height, whether the camera can still free-look, or what happens to the ADS/FOV
   state on entering downed.
7. No item on **the very first zombie**. brief-v3 Phase 1's accept is "a zombie
   reaches you and damages you"; the audit never asks whether round 1 zombie 1
   should be a guaranteed gentle single-file approach (a teach) or just the normal
   spawn logic.

**Weak defaults:**
- **1.3** breather drops to 7s at round 10 and 5s at round 20. Ungrounded and
  probably bad: the breather is the *only* window for wall buys, reload,
  repositioning and (Phase E) the gunsmith, and 13.6's default explicitly makes
  the gunsmith usable only in the breather without pausing it. Shrinking it to 5s
  makes the gunsmith almost unusable off the train and punishes the exact
  build-craft the economy is selling. Better: hold 10s through the mid game,
  consider a *longer* breather (12 to 15s) on the rounds after a special round so
  the player can recover and re-kit, and never go below 8s.
- **1.4** "hard round-length ceiling of 4 minutes after the last spawn". Pulled
  from nowhere and dangerous: a legitimate high-round fight against a full
  `MaximumAlive` cap with a weak weapon can exceed 4 minutes, and force-killing
  the remaining horde hands the player free points and breaks the difficulty
  contract. Better: no time ceiling; rely on the per-zombie unreachable check
  (which is the right mechanism) plus a much longer safety net (e.g. 90s since
  the last kill AND last damage dealt or taken, i.e. genuine stall detection, not
  a fight-length timer).
- **1.5** opening counts `{6, 8, 12, 16, 22}` with `CountGrowthPerRound = 3`. The
  jump from the coded `{6,8,10,12,14}` to `{...,12,16,22}` is asserted as "a
  gentle teach then a real spike" but no reasoning ties it to the round 12 to 15
  target in brief-v3 Phase 11. With `MaximumAlive` also unresolved (2.7), the two
  curves cannot be tuned independently in prose. Better: leave the opening curve
  as the coded values, mark them explicitly provisional, and defer the shape to
  Phase 11 with `MaximumAlive` as a joint tuning target. Do not invent a second
  set of placeholder numbers to replace the first.
- **1.2** round-start sub-bass "~40Hz rising to ~80Hz" and the crimson tunnel
  pulse. The 40Hz figure is fine as a starting point, but reusing the *same*
  swell for breather-end and round-start (so the player "learns it as here they
  come") collides with 1.8's default, which pitches the same swell up a fifth for
  special rounds. Three meanings on one motif is muddy. Better: distinct short
  motifs for round-start and breather-end, keep the pitched-up variant for
  specials.

**Wrong/miscategorised:**
- **1.1** "currently in code ... `L_GreyboxTest` level Blueprint calls it on
  `BeginPlay`". Per `docs/tasks/NEXT.md` the `L_GreyboxTest` Level Blueprint
  wiring for the *round manager* is fine, but the `L_CanaryWharf_Greybox` Level
  Blueprint `BeginPlay` wiring (item 3.14 in the NeoStack run) is *not done* and
  is flagged as a NeoStack API gap needing a human. The audit treats "the level
  calls BeginRounds" as a settled fact; in the second grey box it is an open task.
- **1.7** "the run has no ending" is correct, but it is stated as if `OnDied()`
  being an empty hook is the whole gap. The deeper gap the audit skips: `bDead`
  is a single bool that already permanently blocks `Move`, `StartFire`, `Reload`,
  `Interact` (verified in `LTPlayerCharacter.cpp`). Implementing the proposed
  `bDowned` crawl state means untangling every one of those `if (bDead)` guards,
  which is a non-trivial refactor the audit should call out as scope, not a hook
  to fill.

### Section 2: Zombie AI and locomotion

**Missed:**
1. No item on the **navmesh itself**: who builds it, is it static or dynamic,
   what happens on debris-door open (brief-v3 Phase 3 accept says "navigation
   rebuild on door open" and `LTSpawnPoint` has `AreaTag` for door gating, but
   the audit never asks whether the rebuild is a full `RebuildNavigation`, a
   dirty-region rebuild, or runtime nav-link toggling, or what the hitch cost is).
2. No item on **AI controller lifecycle and budget**. Every zombie runs
   `MoveToActor` off its own tick with an `AAIController`. At `MaximumAlive` of
   24 to 44 that is 24 to 44 controllers plus 24 to 44 ticking characters. The
   audit's 2.8 touches corpse tick but never the live-crowd tick budget, path-
   query throttling under the pathfinder's own concurrency limit, or whether
   `crowdmanager`/`DetourCrowd` should replace per-agent RVO.
3. No item on **zombies clumping at a shared goal**. `MoveToActor(target,
   AttackRange*0.5)` gives every zombie the *same* acceptance sphere around the
   player, so the whole horde funnels to one ring and the back rows shove the
   front rows into the player. 2.1's lateral-lane-bias default helps en route but
   not at the destination. Needs an explicit "attack slot" / encirclement rule.
4. No item on **zombies and the flood water plane** as a moving obstacle (6.2
   defines flood's effect on zombies as a static tile cost; nothing says what
   happens to a zombie mid-path when the water rises a step and its lane becomes
   `>60cm` deep, the drown threshold from 6.2).
5. No item on **spawn-point starvation**. `TrySpawnOne` bails silently if no
   point `IsAvailable` (cooldown, `FirstRound`, `bEnabled`). On a small station
   with doors shut, the round can stall not because zombies are stuck but because
   nothing can spawn. The audit's 1.4 covers stuck *live* zombies, not a spawn
   drought.
6. No item on **hit-reaction vs pathing**. `OnHitReaction` is a Blueprint hook;
   if it plays a montage that uses root motion or disables movement, it fights
   the `MoveToActor` the audit is tuning. 2.6 covers stagger but not the general
   "montage vs navmovement" ownership question.
7. **Zombie audio (2.9) misses the listener-relative mix entirely.** It specs
   per-zombie beds and attenuation but not concurrency (how many of 44 vocal
   loops are audible at once), voice stealing, or the priority order when the
   train rumble, gunfire and 40 groans all compete. This is the actual hard part
   of "tell what is behind you with your eyes closed" (brief-v3 Phase 8 accept).

**Weak defaults:**
- **2.1** sinusoidal weave "amplitude ~60cm, period ~2s" applied to the goal
  location. Applying a lateral offset to the *goal* (the player) rather than to
  the agent's steering means all zombies weave toward the same moving phantom
  point; it will not read as individual wander and may cause the whole horde to
  list to one side. Better: offset per-agent along the path (a path-follow
  lateral bias in the movement component) or accept that real crowd texture
  needs `DetourCrowd` / a StateTree and is a Phase C+ system, not a one-line
  goal-offset hack.
- **2.2** sprinter speed band `0.95 to 1.08`. Fine in isolation, but it is
  multiplied onto the 3.1 sprinter base speed of **340**, which is itself
  ungrounded (see section 3). Compounding a made-up number by a made-up band.
- **2.6** stagger "budget" that decays at 60/s, threshold 40, `Damage * 0.5` per
  shot. With the coded SMG at `BaseDamage 34`, one hit adds 17 to the meter and
  two hits in ~0.2s (600 RPM) cross 40, so the SMG staggers on every second
  round anyway, i.e. the "budget" barely changes the current every-shot
  behaviour it is meant to fix. Better: raise the threshold to a multiple of a
  typical weapon's per-shot damage (e.g. 3 to 4 SMG rounds), or gate stagger to
  heavy single hits (shotgun, DMR, sniper) plus a small headshot chance, and
  leave sustained SMG fire as non-staggering so the horde stays a threat.
- **2.7** `MaximumAlive = 28` "a compromise". This is the third different way the
  audit picks this number: brief-v2 says 40+6/heat, code says 24, brief-v3's perf
  note says 24, the audit says 28 base +4/heat to 44. None is grounded in a
  profile because there is no profile yet (no Phase C, no 24-zombie build with
  final-ish assets). Better: state explicitly that `MaximumAlive` is
  UNRESOLVED pending the Phase 11 profile, keep 24 as the safe build-time
  default so nothing is tuned against a fantasy, and make it a per-station
  `EditDefaultsOnly` property so a profile can raise it later without a design
  change.
- **2.4** spawn-in "emerge" state walks forward 2m at half speed. Fine, but the
  `bInPlayerView` skip-this-point check "needs the player camera" and the audit
  does not flag that `LTSpawnPoint::IsAvailable` is a `BlueprintPure` const with
  no world/camera access today, so this is a signature change plus a per-tick
  frustum test on every spawn point. Scope, not a free default.

**Wrong/miscategorised:**
- **2.5** "there is no `OnAttack` hook at all". Correct that there is no attack
  hook, but the framing implies the fix is just adding `OnAttackWindUp()`. The
  real state (verified in `LTZombieCharacter.cpp`): `TryAttack` applies damage
  the same tick the range check passes, with `AttackCooldown` set *before* the
  damage call, and there is a live `// DIAGNOSTIC` `LT_LOG` block in there plus
  an open PIE finding (`NEXT.md`: "zombie attack lands nothing"). So section 2.5
  is proposing a wind-up redesign on top of an attack path that is currently
  under active debugging and may change. The audit should note the interaction
  with the open bug.
- **2.1** "the flow field was a web-build concept, discarded" - correct, but the
  audit does not catch that `docs/reference/reference-frame-notes.md` section 4
  still lists "Flow field plus spawn routes down the tunnel mouths" as the Phase
  B/D mechanism for the horde. That is a live doc contradiction that belongs in
  section 16 and is missed there too.

### Section 3: The five zombie types

**Missed:**
1. No item on **which anims each type needs and where they come from**.
   `free-assets.md` leans on Game Animation Sample + Mixamo for locomotion and
   "the specifically undead clips GASP does not have". A brute at scale 1.5, a
   crawler at scale 0.5 prone, and a sprinter all need different locomotion sets
   and the crawler needs a genuinely different rig pose. This is a Phase C/F
   blocker the type table (3.1) does not mention.
2. No item on **the single-mesh-varied-by-scale-and-colour premise itself**.
   brief-v2 Part 1 and free-assets both assume "one mesh, vary by scale, colour,
   animation speed", but `free-assets.md` section B actually proposes City Sample
   Crowds (6 bodies, 12 heads) for "5 to 8 distinct undead Londoner types". So
   the plan has quietly shifted from one mesh to a crowd-variation system and the
   audit never asks which it is. This decides whether "five types" reads visually
   at all (the brief-v3 honest-ceiling section flags exactly this: "per-zombie
   variation ... not achievable solo at that fidelity").
3. No item on **crawler reachability and the crosshair**. 3.1 says "only body
   hitbox reachable standing" but the weapon trace is a single line from the
   camera; a prone crawler at scale 0.5 against `MaximumAlive` of 40 in a flooded
   arena is a hitreg nightmare and a melee-only threat, which fights 8.3's
   "instant-kill melee to round 5" (a crawler on round 8+ is past that).
4. No item on **screamer line-of-sight** to the player in a dark station with
   corners: 3.1's "on line-of-sight to player for 2s" needs a defined LoS test
   (to the camera? through the readability-floor darkness? blocked by the flood
   plane? by other zombies?) and 3.5 only covers the audio effect, not the
   trigger.
5. No item on **first-appearance rounds vs the 2-station v1**. 3.1 puts the
   screamer at round 12 and the brute at round 10; if a typical run is "round 12
   to 15 on a first serious run" (brief-v3 Phase 11), a first-time player may
   never see three of the five types. Is that intended? The audit does not ask.
6. No item on **de-spawn / leash**. If a sprinter loses the player (mezzanine,
   escalator), does it keep pathing forever, give up, or de-spawn? Affects the
   `LiveZombies.Num() == 0` round-end condition.

**Weak defaults:**
- **3.1 sprinter speed 340.** Ungrounded and probably too fast. Player
  `WalkSpeed` is 420 and `SprintSpeed` 640 (verified). A 340 sprinter is slower
  than a walking player, so it never catches a mobile player and only threatens
  someone cornered, which makes it a worse walker rather than a distinct threat.
  A classic-zombies sprinter is *faster than player sprint* in a straight line
  and the counterplay is training a tight loop. Better: sprinter base ~480 to
  520 (between walk and sprint, catches a walking or reloading player, loses a
  clean sprint), tune in Phase 11. Whatever the number, ground it in the player
  speeds, which the audit had and did not use.
- **3.1 brute HP mult 6.0 (900).** Against the coded SMG (34 dmg, 600 RPM) that
  is ~26 body rounds, most of a mag, at round 10 when the player likely has one
  wall weapon. Fine as a bullet sponge intent but the "knocks player back on
  hit" plus 55 damage plus stagger immunity under 120 is three punishing traits
  at once with no counterplay stated (headshots? legs? a weak point?). Better:
  pick one signature trait (armour that must be broken, or the knockback), not
  three.
- **3.1 screamer "deafened muffle for 4s".** In a game whose Phase 8 accept
  criterion is literally "tell what is happening behind you with your eyes
  closed", a mechanic that removes the player's hearing is removing the primary
  sense the whole audio design exists to serve, and 3.5 already hedges that the
  audio system might not support it. Better: the screamer's threat is the summon
  plus a loud directional scream that *masks* other audio while it is screaming
  (a natural consequence of a loud sound, no submix trickery), and killing it
  fast is the counterplay. Drop the post-death deafen.
- **3.3 crawler death gas** "3m radius, 4 HP/s for 4s". The audit itself flags
  this as its own invention with nothing in the docs. It is a reasonable idea
  but it is *new design*, not a resolved ambiguity, and it interacts with the
  flood mechanic (gas over waist-deep water?) and the downed state. Flag as a
  proposal needing its own sign-off, not a default.
- **3.2 "every 5th round the entire roster is sprinters at 60% count".** With the
  sprinter speed problem above, a whole round of 340-speed sprinters at reduced
  count could be *easier* than a normal walker round, inverting the intended
  spike. Resolve sprinter speed first, then this.

**Wrong/miscategorised:**
- **3.1** "`BaseHealth = 150`" is quoted correctly from the header, but the audit
  says "the current C++ defaults ARE the walker" and builds the table's HP
  column off 150. Note that `ApplyRoundScaling` sets `Health = BaseHealth *
  1.1^(round-1)` so by round 10 the "walker" already has ~354 HP before any type
  multiplier; the table's "HP mult" column multiplies on top of a compounding
  base and the audit never shows a worked round-10/round-15 effective-HP figure,
  which is what Phase 11 actually needs.

### Section 4: The train

**Missed:**
1. **The single biggest missed item in the whole audit: platform screen doors.**
   `docs/reference/canary-wharf-research/rolling-stock.md` states Canary Wharf
   Jubilee has "Platform screen doors ... full height, so from the platform you
   see the train through glass screen doors when stopped, and a blank screen wall
   when it has gone." This changes everything in section 4: the "train as a wall"
   is a *screen door wall* even when no train is present, boarding is through two
   sets of doors (PSD + car door) that must animate in sync, a zombie cannot
   fall in the trackbed (4.3) if the platform is screened, and the arrival
   headlight beat (4.1) is seen through glass. The audit specs an open-platform
   train (LU sub-surface style) and never asks which Canary Wharf this is.
2. **Elizabeth line vs Jubilee line rolling stock contradiction.** brief-v2 and
   art-direction call the line "a fictionalised Elizabeth line". The Elizabeth
   line at Canary Wharf is Class 345 Aventra: main-line gauge, walk-through, 2.8m
   wide floor near platform level, *no* platform screen doors at the Elizabeth
   station. The Jubilee is 1996 Stock: deep tube, small curved body, PSDs. These
   are completely different trains, arenas and boarding geometries. The audit's
   4.1 default just picks "1996 Stock deep-tube profile" without noting it
   contradicts the stated line. This is a Phase C blocker and belongs in section
   16 as a doc contradiction too.
3. No item on **the train blocking fire / line of sight**. If the train fills one
   whole side as a solid wall, it is also cover the player can put between
   themselves and half the spawn routes. Is that intended (a tactical wall) or a
   problem (the horde can never flank from the track side, trivialising that
   flank)? Nothing addresses it.
4. No item on **multiple trains over a long run**. At 100s interval + 25s dwell,
   a round-20 run sees ~15 to 20 train cycles. Does every arrival get the full
   4s slide, headlights, brake squeal, doors, or does it degrade to a quicker
   cycle after the first few so it stops being a 30-second spectacle every 100
   seconds? Pacing question the audit misses.
5. No item on **what the train does to spawns and live zombies during dwell**.
   Do spawns pause while the train is in? Do zombies path *around* the newly
   arrived train (dynamic nav obstacle)? Does the round timer/spawn interval keep
   running? 4.3 covers a zombie *in the trackbed*; nothing covers the 25s the
   train is a new wall in the arena.
6. No item on **boarding while downed** (1.7). Can a crawling downed player board
   to escape a lost run? Strong interaction, unaddressed.
7. No item on **the departure board as a physical asset**: text-render component
   vs a material with a render target vs a widget-component; how it reads in the
   dark (it is a light source); how many exist and where (11.2 says "repeated"
   but section 4 owns the board and never specs it).
8. No item on **audio of the board / announcements vs the diegetic-only timer**.
   If the player is on the mezzanine and cannot see any board (11.2's problem),
   the announcements are the only signal, so their timing (4.4) is load-bearing,
   yet 4.4 leaves the T-15 line as "arrives shortly" with no countdown numbers,
   meaning a player who mishears has no recovery.

**Weak defaults:**
- **4.1** "4.0s slide from off-platform to aligned, ease-out". For a 126m,
  7-car train (rolling-stock.md) to cover its own length plus approach in 4s is
  ~30 m/s, ~110 km/h, decelerating to zero at the platform. That reads as a
  train that does not stop so much as teleport. Real tube approach into a
  platform is more like 10 to 15s of visible deceleration. If the grey box uses
  a 2 to 3 car stub (4.1 says it does), 4s is closer to plausible, but the
  default should state the slide time as a function of visible train length, not
  a flat 4s that will look wrong once the full train exists in Phase 7.
- **4.2** "boarding window ~21s" (doors open 1s after stop, close 3s before
  departure, within a 25s stop-to-start dwell). 21s is generous and probably
  fine, but it is asserted with no reference to how long a kiting player needs to
  break contact, cross the platform and reach a door under round-15 pressure.
  That is the number that matters and it is not derived.
- **4.7** "travel is a hard cut: 0.5s fade, level load, 1.0s fade in". A full
  UE5 level load with Lumen is not 1.5s of fades on a mid spec machine; it is a
  multi-second hitch or a streaming level. The audit picks "separate `.umap` per
  station" (a real architecture decision) and hides the load behind fades that
  are too short. Either commit to level streaming / World Partition and say so,
  or accept a loading screen and design it. Do not pretend the cut is seamless.
- **4.8** Oyster Credit conversion "1 Credit per 500 points spent-or-held-at-
  board, capped at 5 per board". Invented rate with no grounding, and it
  interacts with 7.6's "1 Credit per round survived" and 13.4's "meta-progression
  is deliberately thin". Adding a second Credit earn path with its own cap is
  more meta-economy design than a round-loop audit should be locking. Flag as a
  proposal.
- **4.9** "refill reserve, not magazine". Reasonable, but combined with 8.6's
  "Max Ammunition pickup every 5th round" and wall-buy ammo, the audit is
  spreading ammo-economy decisions across 4.9, 8.6 and 7.2 without a single
  "here is the ammo economy" statement. See structural note.

**Wrong/miscategorised:**
- **4.5** "`ILTInteractableInterface` exists and is the obvious mechanism". True,
  but the audit does not note that the interface has *no C++ implementers except
  `ALTWallBuy`* and no hold-to-interact support anywhere (`ULTInteractionComponent`
  is press-once, `TryInteract`). The proposed "1.0s hold-to-board bar" is a new
  capability on the interaction component, not a config of the existing one.
- **4.4** "StationDef schema has `announcements: [string, string, string]`". This
  is a `web/src/data/schemas.ts` TypeScript field, and the audit's own 6.4 and
  16.5 say that schema is unported and the grid legend references are stale. So
  citing it here as if it constrains the UE build is inconsistent with the
  audit's own position two sections later.

### Section 5: Station heat

**Missed:**
1. No item on **heat and the round counter interaction**. Heat ticks "+1 per
   train that departs without the player" (5.1), i.e. ~once per 100s, which is
   *not* once per round (rounds get longer). So heat and round drift apart: by
   round 20 a slow player might be at heat 10+, a fast player at heat 3. The
   audit caps heat at 6 (5.1) but never reconciles the tick rate with round
   length or says what "staying" costs a player who clears rounds faster than
   trains arrive.
2. No item on **heat persisting or not through a down/self-revive** (1.7). You
   survived the station but got downed; does heat keep climbing?
3. No item on **heat and spawn-point activation**. Does high heat open the
   later-round pincer spawn points (the grid's bottom `S` clusters,
   `FirstRound ~4`) early, or just multiply rate through the existing points?
   The "wall of zombies" read depends on this.
4. No item on **heat's effect on the economy loop**. 5.3 asserts "points-per-
   minute is higher at high heat" as staying's upside but never checks it against
   the per-pellet award (7.1) or the `MaximumAlive` cap: if the cap is the real
   limiter, more heat past the cap adds pressure without adding kill throughput,
   so points-per-minute could *plateau* while danger keeps rising, which makes
   staying strictly worse, the opposite of the intended tension.
5. No **audio signature for heat** beyond "a rising drone" (5.2). The brief calls
   audio a whole phase; heat is the mechanic most in need of a non-visual tell
   because 5.2 itself says it should not be a HUD number.

**Weak defaults:**
- **5.1** heat effects: `MaximumAlive +4` per level AND spawn interval `*0.90`
  compounding per level, ceiling 6. This is a *third* set of heat numbers
  (brief-v2: +6 cap / +12%; the audit's own 2.7: +4 to a ceiling of 44; this:
  +4 with `0.9^6` interval). 2.7 and 5.1 at least agree on +4, but 5.1's
  interval multiplier is new and stacks multiplicatively with the per-round
  `SpawnIntervalDecay = 0.96` already in code, so at round 20 heat 6 the
  effective interval is `1.6 * 0.96^19 * 0.9^6 ≈ 0.38s`, basically the 0.35 floor.
  The audit never shows this compounded figure. Better: pick ONE heat model,
  express it as a single multiplier on the final spawn rate and a single flat
  add to the cap, and show the round-20-heat-6 worked number so it can be
  sanity-checked.
- **5.1** "heat ceiling 6". Arbitrary. If a farming player's whole strategy is
  "stay and bank at max heat" (5.3), the ceiling is the single most important
  number for late-game balance and it is picked with no reasoning. Tie it to
  something (e.g. the cap add should not exceed the perf headroom above
  `MaximumAlive`).
- **5.2** "the departure board's service list shortens/glitches" as a heat tell.
  Cute, but it fights 4.x and 11.2, where the board is the *trustworthy* diegetic
  train timer the player must rely on. Glitching the one instrument the player
  needs to make the stay/go decision is self-defeating. Better: heat reads on the
  *station* (creeping crimson, the drone, emissive flicker), leave the board's
  train data clean.

**Wrong/miscategorised:**
- **5.1** "`+1 per train that departs without the player aboard`" is presented as
  a clarification of brief-v2, but brief-v2 says "Staying raises station heat by
  1" with no per-what, and brief-v3 says "station heat when you stay". Neither
  says "per train departure". This is a new definition, not a disambiguation, and
  it is a reasonable one but should be labelled as a proposal.

### Section 6: Stations and the mechanic library

**Missed:**
1. No item on **the modular kit and the mechanic library colliding**. brief-v3
   Phase 7 builds a hand-made modular kit for Canary Wharf. `open_air`,
   `depot`, `surface` mechanics (6.2) need a *different* kit (daylight,
   street stairs, weather). The audit defines all 10 mechanics but never flags
   that half of them are un-buildable with the one kit v1 will have, so they are
   defined-but-dead. For a 2-station v1 only `flood`, `interchange`, `blackout`,
   `platform_split` matter; the audit should say the other 6 definitions are
   documentation for the expansion, not v1 scope.
2. No item on **the grey box vs the mechanic**. `L_CanaryWharf_Greybox` is built
   (`NEXT.md`) with the flood zone "marked, no rising mechanic" and escalators as
   static slabs. So flood-rise and the interchange conveyor are *not yet in the
   grey box the audit assumes exists*. 6.2's flood/interchange defaults are new
   systems, and the audit's "currently in code: nothing" is right but it does not
   note the grey box geometry is already committed and may constrain the
   mechanic (e.g. the `~` block is rows 17 to 22, a fixed rectangle, so
   "shrinking arena" is really "two lanes get wetter").
3. No item on **navmesh cost weighting actually working**. 6.2 and 2.1 both lean
   on `UNavArea` cost classes for water/barrier tiles, but `canary-wharf-grid.md`
   ("What Phase D builds") says the flood zone is navigable "at higher cost if
   the API allows setting that, otherwise just navigable". So the grey box may
   already be built without the cost weighting the audit's crowd-splitting
   defaults depend on. Unverified assumption.
4. No item on **the second station's grid**. 6.1 picks Whitechapel as station 2
   but there is no Whitechapel grid, no research folder (only Canary Wharf has
   `canary-wharf-research/`), and `blackout + platform_split` needs a two-
   platform-plus-footbridge layout that does not exist even as a sketch. That is
   a Phase 10 blocker the audit creates and does not size.
5. No item on **`ALTStationInfo` / `ALTStationData` and the round manager**. The
   round manager is "station agnostic" (finds spawn points by world iteration).
   6.4 proposes `ULTStationData` but never says how the round manager reads it
   (does it get a ref on `BeginPlay`? does the GameMode inject it?). Wiring gap.
6. No item on **boss music / boss arena** even as "deferred" beyond 6.8's one
   paragraph; 13.x lists an "end-of-line screen" implying termini matter.

**Weak defaults:**
- **6.1** "commit to 2 stations for v1 ... second station Whitechapel". Picking
  the second station is a real decision the audit is right to force, but
  `blackout` as the second mechanic doubles down on darkness in a game whose
  named risk is "fight what you cannot see" (art section 5, brief-v3). A second
  station that contrasts *tonally* (the audit's own 6.2 has `open_concourse`:
  wide, bright, high cap, run-and-gun) would teach the player the game has range.
  Better: Whitechapel with `platform_split` + `open_concourse`, or Paddington
  (`open_concourse + interchange`, already has brief-v2 backing) as station 2,
  and hold `blackout` for station 3.
- **6.2 flood** "rises 12cm per round to 150cm". 12cm/round means waist-deep by
  round ~10, which with ">120cm blocks fire" means the flood mechanic *ends the
  ability to shoot on the platform* by round 10 on the one tier-4 station. That
  is a hard wall, not a shrinking arena. Better: much slower rise (3 to 5cm/
  round), a lower ceiling (~90cm), and the "blocks fire" state removed or
  replaced with a heavy accuracy/handling penalty, so flood pressures movement
  without disabling the core verb.
- **6.2 interchange** escalators "one-way-fast, standing on an up escalator
  moves you up at 2x". A 2x conveyor the player cannot fight against is a
  one-way valve that a kiting player will exploit (ride up, horde bottlenecks at
  the escalator foot, shoot down the stairs forever). That is the classic
  training exploit the audit never addresses (see structural). Better: escalators
  move you at a modest boost, zombies use them at near-parity, and the design
  goal is a *vertical loop* the player runs, not a safe perch.
- **6.7** Canary Wharf content map: "the 4 wall buys are the starting SMG refill
  spot plus 3 guns". But 8.2's default *removes* the SMG from the start and makes
  it a 500-point platform wall buy. So one of the 4 `W` anchors is "buy the SMG"
  and the audit should say that explicitly rather than "starting SMG refill
  spot", which implies you already have it.
- **6.7** perk split "2 perks at Canary Wharf, 2 at the second station" as a
  travel incentive. With only 2 stations and a 3-perk cap (9.2), this means you
  can get 2 perks at station 1, must travel for a 3rd, and can never get the 4th.
  That is a reasonable design but it hard-couples the perk economy to the
  2-station count; if station 3 ever ships, the split breaks. Flag the coupling.

**Wrong/miscategorised:**
- **6.6** grid normalisation "flagged as a task, out of scope for this audit
  doc". Correct call, but the audit lists it as item 6.6 and counts it in the 98,
  inflating the count with a non-question. It is a cleanup ticket, not an open
  design question.
- **6.3** "debug-yard.ts uses `['flood', 'interchange']`" - true, but debug-yard
  also has `adjacent: ['debug-yard']` (a self-loop, the audit's own 16.6), so
  citing it as evidence of a mechanic pairing decision is citing a test hack.

### Section 7: Economy and pricing

**Missed:**
1. No item on **the wall-buy price double-source being live right now**.
   `ALTWallBuy` has `WeaponCost = 500` / `AmmunitionCost = 250` as instance
   fields and reads *those* (verified in `LTWallBuy.cpp`), never touching
   `ULTWeaponData::WallPrice` / `AmmoPrice`. The audit's 7.2 notes "the data
   asset's price fields are currently dead" but files it as a future "wiring bug
   in waiting" rather than a live inconsistency that will bite the moment someone
   sets a price on a data asset expecting it to matter.
2. No item on **points feedback**. There is no floating "+60" / "+130" number,
   no kill-confirm sound spec, no economy readout behaviour on a big multi-kill.
   `OnPointsChanged` broadcasts total and delta; nothing consumes the delta for
   feedback. In a game with no kill feed (banned), the points tick is the *only*
   confirmation a kill paid out.
3. No item on **negative economy events**. Is there ever a points penalty (dying,
   a mechanic, a mistake)? `AddPoints` allows negatives and `TrySpend` uses them.
   Classic zombies has none; the audit should confirm LAST TRAIN has none either,
   because 4.8 ("banks points" implies points at risk) raised the question and
   7.x never closes it.
4. No item on **first-purchase pacing**. With 500 start and the pistol-only start
   (8.2), round 1 economy is: pistol kills at 60 each, ~2 kills to afford the 500
   SMG wall buy. Is that the intended round-1 arc? Nobody checks the actual
   round-1 points curve against the round-1 zombie count (1.5).
5. No item on **shared vs per-player economy** (moot if solo-only, but 10.4 only
   confirms solo late; section 7 assumes it silently).

**Weak defaults:**
- **7.1** "multi-pellet hits award `PointsPerHit` once per shot, not per pellet".
  This is the right fix, but it is a *behaviour change to shipped C++*
  (`TracePellet` currently calls `Points->AwardHit()` per connecting pellet,
  verified). The audit lists it as a "suggested default" alongside "confirm
  500/10/60/130 as intended", conflating a code change with a documentation
  confirmation. Also, awarding once per shot *regardless of pellet count* nerfs
  the shotgun's point economy hard (a 8-pellet point-blank hit that currently
  pays 80 would pay 10); the audit should model the shotgun's intended
  points-per-shot, not just remove the exploit.
- **7.3** perk prices (2500 / 2000 / 3000 / 2000). Invented, and internally
  reasoned only by "Double Tap cheap because pure offence, Second Wind dear
  because infinite sprint is strong". But 10.2's default *adds a stamina system*
  specifically so Second Wind has a purpose, and 9.1 makes Second Wind also do
  regen + extra self-revive. A perk that does three strong things for 3000 in an
  economy where a wall LMG is 2500 is underpriced if anything. These numbers
  cannot be set before the stamina/revive decisions and the audit orders them
  backwards.
- **7.4** points soft cap 250,000. Harmless but pointless: a round-15 run does
  not approach it and a farming run that does would rather the cap not exist.
  Either cap for a real reason (anti-cheese, HUD layout) with a real number or
  drop the item.
- **7.6** attachment unlock costs "optic 3, mag 5, barrel 8 Credit per weapon ...
  ~15 to 20 runs for everything". Entirely invented progression pacing for a
  meta-system (13.4) the audit elsewhere says should be "deliberately thin".
  Setting a 15-to-20-run grind is a retention-design decision that needs the
  user, not a default.

**Wrong/miscategorised:**
- **7.1** "`AwardHit` on every pellet hit that does not kill". Verified accurate
  (`if (Points && !Zombie->IsDead())` per pellet in `LTWeaponComponent.cpp`).
  So the audit is *correct* here, contrary to a first read that it might be
  describing an unwired system. Noting this because several other "currently in
  code" claims in the audit are looser than this one.

### Section 8: Weapons and the gunsmith

**Missed:**
1. No item on **the weapon has no world model / view model pipeline**.
   `ULTWeaponData` has `Mesh` (a `USkeletalMesh`) and `ALTPlayerCharacter` has a
   `ViewModel` `USkeletalMeshComponent` that is *never assigned a mesh or
   animated* (verified). A 12-weapon roster (8.1) needs 12 view models, draw/
   reload/fire anims, and an anim blueprint. free-assets points at Lyra's weapon
   system as the reference. This is the single largest un-scoped art/anim task in
   section 8 and it is not an item.
2. No item on **recoil as camera kick**. The spread model is bloom-only
   (`GetCurrentSpreadDegrees`); there is no vertical camera punch, no recoil
   pattern, no recovery-to-origin. brief-v3 Part 1 says "Recoil bloom accumulates
   ... so holding the trigger stops being accurate" and calls that the whole
   model, but a shooter with zero camera movement on fire feels dead. Is
   bloom-only deliberate? The audit never asks.
3. No item on **hit feedback from the weapon side**: tracer, impact decal,
   impact particle, surface-typed impact sound. `MuzzleFlash` is a
   `UParticleSystem` (legacy Cascade, not Niagara) field that is unset; the audit
   should flag the Cascade-vs-Niagara mismatch with free-assets' Niagara pack.
4. No item on **penetration and the horde**. `Penetration` passes a shot through
   N zombies. Against a `MaximumAlive` wall of 40, a `Penetration 3` rifle line
   through a packed corridor is a very different weapon from the same rifle in
   the open, and the audit's per-weapon `Pen` column (8.1) is set with no
   reference to typical horde density.
5. No item on **ADS and the FP camera / view model**. 8.x never touches how ADS
   looks (the audit's 16.2 notes FP but not that ADS needs a weapon-raise pose
   and a sight alignment the view model must provide).
6. No item on **the joke weapon's tone**. "Oyster Reader ... fires ticket stubs"
   is exactly the CoD-furniture whimsy the brief says to avoid ("deliberately NOT
   Call of Duty furniture", "restrained tone"). The audit proposes it in 8.1 and
   9.4 without checking it against the tone brief.

**Weak defaults:**
- **8.1** the entire 12-weapon table. Every number is invented and the audit
  admits it ("starting numbers ... tuned in Phase 11"). That is fine as a
  *starting roster shape*, but presenting 130+ specific numbers as a "suggested
  default" invites a Sonnet bulk-entry pass to enshrine them as data assets
  before any of them is playtested. Better: lock only the *roster shape* (how
  many, which archetypes, tier spread, which need code) and explicitly defer all
  numbers to a Phase 11 tuning doc so nobody data-enters fantasy stats.
- **8.1** "Terminus" bolt sniper at 260 damage, `Pen 5`. Against a round-10
  walker's ~354 effective HP that is not a one-shot body kill, but against a
  round-1 walker (150) it 1-shots through 5. A `Pen 5` 260-damage line is a
  horde-deleter in a corridor and probably breaks the mid-game economy. No
  reference to the HP curve.
- **8.2** "v1 starts with the Warden pistol only, SMG becomes a 500 wall buy".
  This is a real, defensible design pivot (classic-zombies opening), but it is a
  *change to `BP_PlayerCharacter`* which currently starts with `DA_Weapon_SMG`
  (verified via README and audit). It also invalidates 6.7's wall-buy plan and
  the existing grey box wall-buy placement. Reasonable, but flag the ripple.
- **8.3** melee "instant-kill to round 5, then flat 150". Adding melee is a
  sound call (the audit is right that round-based survival assumes it), but "flat
  150 past round 5" against a compounding HP curve means melee is useless by
  round 8, so it is really "a round 1 to 5 tool then dead weight". Classic
  zombies keeps the knife relevant with a perk. Either commit melee as an
  early-game-only mechanic explicitly, or scale it.
- **8.5** upgrade bench "double damage, +50% mag, persists through travel not
  through death". Fine, but "one gun at a time" is left genuinely ambiguous even
  after the audit's clarification ("only one of your carried weapons may be
  upgraded") because 8.7 gives you 2 weapon slots and the interaction with
  swapping an upgraded gun into the box (9.4) or the bench again is unresolved.

**Wrong/miscategorised:**
- **8.7** "buying a shotgun off a wall currently DELETES your SMG with no
  warning. That is a serious economy bug." Verified: `SetWeapon` replaces
  `WeaponData` wholesale, no second slot. But this is framed as a bug in section
  8 and *also* implicitly in the priority list (#6). It is not a bug, it is an
  unimplemented feature (2-weapon carry) - the current code is a Phase 1 combat
  slice that only ever had one weapon. Calling it a bug overstates the regression
  risk; it is scoped Phase C/E work.
- **8.1** "Crossbow and joke weapon likely need small code additions
  (projectile, knockback)". Understated: there is *no projectile system at all*
  (the weapon is pure hitscan, verified). A projectile weapon is a new movement/
  collision/impact subsystem, not a "small addition".

### Section 9: Perks, bench, lost property, revive

**Missed:**
1. No item on **perk acquisition presentation**: is it a machine you interact
   with (like a wall buy), does it have a per-station model, an animation, a
   drink/use gesture, a HUD icon appear? 11.1 adds "a perk row ... one flat
   sodium glyph per perk" but section 9 never specs the acquisition moment.
2. No item on **perk stacking / re-buy after a down**. 9.2 says perks are kept
   through a self-revive, but if "lost on death" ever means a down (the audit
   hedges), re-buying at a per-station machine you may have travelled away from
   is impossible. Unresolved.
3. No item on **Second Wind vs the no-stamina reality**. 9.1 and 10.2 both
   circle this: 10.2 proposes adding stamina *so that* Second Wind means
   something. So Second Wind's spec is contingent on a system that does not exist
   and that the audit is only proposing. Section 9 should say "Second Wind is
   undefined until the stamina question (10.2) is resolved" rather than speccing
   it as if stamina is a given.
4. No item on **equipment beyond one flare**. 9.5 invents a signal flare; brief-v3
   Phase 6 says "equipment" (plural-ish). Is one piece the whole category for v1?
   The audit asserts yes without asking.
5. No item on **lost property / mystery box presentation**: the classic box has a
   long-established open animation, a light beam, a teddy-bear equivalent (the
   "box moves" fail state, which 9.4 explicitly drops). Dropping the move is a
   real design choice that removes a known pressure valve; the audit makes it in
   passing.

**Weak defaults:**
- **9.1** Commuter's Constitution "MaxHealth 100 to 175". brief-v2 says "+75 max
  HP"; the audit reads that as a new max of 175 (100 + 75). Fine. But it also
  says "Regen unchanged" at `RegenerationPerSecond = 20`, so healing from 0 to
  175 takes ~8.75s vs ~5s for 100. A bigger pool that heals *slower in relative
  terms* is a subtle nerf the audit does not flag; classic Juggernog-style perks
  usually also speed regen.
- **9.1** Double Tap "`RoundsPerMinute *1.3`, `BaseDamage *1.15`". brief-v2 says
  "+30% fire rate and +15% damage", so this is a faithful port, but multiplying
  RPM by 1.3 on the coded SMG (600 to 780) with bloom-per-shot unchanged means
  the gun blooms to max faster and is *less* accurate in sustained fire, possibly
  making Double Tap a downgrade for a spray weapon. Interaction with the spread
  model unexamined.
- **9.3 / 1.7** self-revive: "kill 4 zombies while downed OR survive to the next
  breather, once per round free". With the downed player on 30% move speed and
  pistol-only, "kill 4" while a horde closes is very hard, and "survive to the
  next breather" could be 2+ minutes away on a high round. So the free self-
  revive is often unachievable, making every down effectively run-ending, which
  makes the whole downed state theatre. Better: a fixed bleed-out timer you can
  beat by *disengaging* (get X metres from all zombies for Y seconds), which a
  crawl speed can actually accomplish.
- **9.4** lost property pool "all weapons except your two starting-tier ones,
  plus the joke weapon at low weight". With no tier gating "that is the gamble",
  a round-4 player can pull the `Pen 5` 260-damage sniper (8.1) and trivialise
  rounds 4 to 15. Classic zombies accepts this; LAST TRAIN's "round 12 to 15 is
  a serious achievement" target does not survive it. Consider soft tier weighting
  by current round.

**Wrong/miscategorised:**
- **9.2** "with only 4 perks and a cap of 3, you can nearly get all; is that
  intended?" The audit raises it and then 6.7 answers it elsewhere (2+2 split
  means max 3 requires travel, and the 4th is unreachable in a 2-station v1).
  The answer is split across two sections; a reader of section 9 alone gets a
  question with no answer.

### Section 10: Player mechanics

**Missed:**
1. No item on **movement acceleration / friction**. `CharacterMovementComponent`
   defaults (high accel, near-instant stop) give a twitchy arcade feel. brief-v3
   wants "the platform reads" and a restrained tone. Is default UE character
   accel the intended feel, or is there a ground-friction / braking-deceleration
   spec? This is core moment-to-moment feel and it is nowhere.
2. No item on **crouch**. There is no crouch input (verified: Move, Look, Jump,
   Sprint, Fire, Aim, Reload, Interact only). No crouch means no crouch-behind-
   cover, and the crawler's "only body hitbox reachable standing" (3.1) implies
   crouching should let you hit its head. Missing verb.
3. No item on **mantle / vault / traversal**. The interchange mezzanine, the
   trackbed ramps (4.3), debris - a FP survival game usually has at least a
   mantle. `JumpZVelocity = 420` is a plain jump. free-assets points at GASP's
   traversal system. Unaddressed.
4. No item on **the view model and hands**. `ViewModel` component exists,
   unassigned. No first-person arms, no idle sway, no bob. This is most of what
   "weapon handling feel" is and section 10 does not mention it.
5. No item on **look sensitivity, acceleration, controller aim assist**. 11.4
   lists a sensitivity slider in settings but section 10 (player mechanics)
   never specs the aim model: raw mouse, any smoothing, any controller curve,
   any aim assist against the horde (a FP console shooter needs it).
6. No item on **fall-through / getting stuck on the grey box**. `NEXT.md` notes
   the horde smoke test only proved "composition and no fall through"; player
   collision against the escalator slabs and debris cubes is untested and not an
   audit item.
7. No item on **interact vs fire priority**. Both are on separate inputs so no
   direct conflict, but looking at a wall buy while a zombie is on you: does the
   prompt distract, does `E` ever mis-fire? Minor, unlisted.

**Weak defaults:**
- **10.2** the entire stamina proposal. This is the audit *adding a core
  mechanic the game does not have and neither brief-v3 nor the code wants*
  (brief-v3 Part 1 lists sprint with no stamina; the only pro-stamina source is
  a stale brief-v2 Fable prompt). The reasoning ("unlimited sprint trivialises
  the horde") is real, but the fix is not necessarily stamina: it could be a
  faster horde (fix sprinter speed, section 3), tighter arenas, or sprint that
  cancels aim/fire (already true). Adding a stamina meter, a regen delay, a
  desaturation vignette and a perk interaction is a large new system smuggled in
  as a "suggested default". If stamina is wanted, it deserves its own top-level
  decision, not item 10.2.
- **10.1** fall damage "8 HP/metre past 4m, capped 60". Invented, and it
  interacts with the mezzanine (6.2) and the downed state (a fall while downed?).
  60 damage is 60% of base health from one drop, which in a kiting game where
  dropping off the mezzanine is an escape route makes that route a coin-flip.
  Either commit to "the mezzanine drop is a deliberate escape, no fall damage"
  or make it a small tax (10 to 20). 60 is punishing enough to matter and random
  enough to feel unfair.
- **10.6** "keep 100 / 4s / 20/s ... 4 to 5 walker hits kill". The audit does the
  right thing (derives the combat triangle) but then does not carry that maths
  into section 3's damage numbers: sprinter 18, brute 55, screamer 10, crawler
  20 (3.1) are set without showing hits-to-kill for each against 100 HP with 20/s
  regen. Brute at 55 is a 2-hit kill (100 -> 45 -> dead), which with a brute's
  knockback-on-hit (3.1) is a stunlock death. Cross-check missing.

**Wrong/miscategorised:**
- **10.3** "interact range 250 ... you can interact with a wall buy from further
  than a zombie can hit you". Correct, and the audit's "good enough, no change"
  is a reasonable call, but it is listed as an open question (item 10.3, counted
  in the 98) when the audit itself resolves it to "document it, no change". Like
  6.6, it pads the count.
- **10.4** co-op: "v1 is single-player only. State it explicitly." Correct
  conclusion, but this is not a "player mechanics" question, it is a *scope /
  structural* question that should gate several other sections (HUD, revive,
  targeting, travel). Burying "is this game solo?" as item 10.4 under-weights it.

### Section 11: HUD and UI

**Missed:**
1. No item on **the damage direction indicator**. `OnDamageTaken(Fraction)` is a
   Blueprint hook with only a "vignette and audio" comment. With a horde that
   surrounds you and a banned kill feed, *which direction a hit came from* is
   critical survival info and there is no spec (a directional bloodsplat? a
   vignette that biases toward the hit? nothing?).
2. No item on **the reload / low-ammo indicator behaviour**. `OnAmmoChanged`
   fires; the audit's 11.1 lists "weapon block mag/reserve/name" but not what it
   does at 0 mag, mid-reload, or on empty-reserve (the moment the player must
   decide to wall-buy or board).
3. No item on **hit marker states**. brief-v3 Phase 1 and phase-b3 mention "a
   distinct headshot tone" and `OnHitConfirmed(bHeadshot)`. The audit's 11.x
   never specs the hit marker visually (shape, colour against the red/violet
   palette, headshot variant, kill variant).
4. No item on **subtitles / caption UI** despite 11.4 and 12.2 both saying
   "subtitles on by default given the diegetic-audio dependence". Where do they
   render, how long, speaker label or not, do they show the announcement text or
   also zombie/train cues? For a game that puts the timer in audio, this is a
   first-class UI surface with no home.
5. No item on **colourblind accessibility**, which the review brief explicitly
   calls out: the palette is charcoal + violet + sodium + crimson, and
   crimson-vs-violet is exactly a deutan/protan confusion. Heat reads crimson
   (5.2), emergency lighting is crimson, damage vignette is crimson, the "special
   round" tell is crimson (1.2). A colourblind player loses several distinct
   signals. No item anywhere.
6. No item on **the departure board readability as UI** (the diegetic timer is a
   UI element even though it is in-world): font size at distance, glare in the
   dark, what it shows exactly (two services? one? a clock? minutes only?), how
   the "T-15" state looks different from the "T-60" state.
7. No item on **controller support** at all beyond 11.4 listing "invert Y". Is
   v1 KBM-only? brief-v3 says "packaged Windows build" with no platform
   statement. Button prompts (`E` vs a face button) ripple through every prompt
   string (`GetInteractionPrompt` hardcodes "Buy {0}: {1}" with no key glyph).
8. No item on **HUD scaling / safe area / resolution** (11.5 mentions "scaled by
   resolution" for the crosshair only).

**Weak defaults:**
- **11.1** "a perk row ... only appears once you own 1+ perk". Consistent with
  the "no reserved empty slots" rule, but combined with 11.1 also adding the heat
  bar in Phase C and equipment count in Phase E, the HUD's bottom-left and top-
  left both grow items mid-run, which means the layout must reflow live. The
  audit never says whether elements slide in, and reflowing a diegetic-minimal
  HUD under combat is jarring. Better: fixed anchor positions per element,
  elements fade in place, nothing reflows.
- **11.2** train countdown "diegetic only, a subtle sodium glow spills from the
  platform into adjacent areas" when doors are open. A light-spill cue tells you
  the train is *already here*, not that it is *arriving in 15s*, which is the
  decision moment. The audit acknowledges the deep-concourse player problem and
  then does not really solve it (the announcements are the only backstop and
  4.4's T-15 line has no countdown number). Better: accept one minimal non-
  diegetic affordance (a single sodium pip near the round number that appears
  only in the final 15s), or put a repeater board at every escalator top *and*
  the concourse (6.7 geometry permitting) and make that a hard level-design rule.
- **11.6** onboarding "the first time the player looks at a wall buy, the prompt
  is slightly longer for 5s". Cute but fragile: a player who never looks at a
  wall buy in their first run never learns wall buys exist. brief-v3's success
  test is "the first ten minutes feel coherent". Better: a proper (still
  restrained) first-run-only onboarding beat, or a "how to play" screen that is
  actually surfaced (not just "on the main menu" where a Play-button masher skips
  it).
- **11.3** pause "disabled during the train arrival/departure kill windows and
  the boarding hold, to prevent cheese". Disabling pause is a legitimacy /
  accessibility problem (a player who needs to stop *cannot*), and "cheese" via
  pause in a single-player game with no save and no leaderboard (13.x) is a
  non-issue. Better: pause freely; if run integrity ever matters (leaderboards),
  handle it then.

**Wrong/miscategorised:**
- **11.1** "`WBP_HUD` has the phase-b3 set. `HealthBar.Percent` binding was noted
  as missing/being fixed in NEXT.md." Understated: per `NEXT.md` the health bar
  has *zero bindings, confirmed structurally*, and this is one of two open PIE
  findings blocking Phase B sign-off ("zombie attack lands nothing" being tangled
  with it). The audit treats the HUD as substantially built; it is built but
  partly non-functional and under active debugging.
- **11.5** crosshair spread formula `gap_px = 6 + 14 * sqrt(spread_deg / 5)`.
  Presented as a "suggested default" but it is a fresh invention with a made-up
  constant, not a disambiguation of anything in the docs. Fine to propose, but
  it is design, not audit.

### Section 12: Audio and music

**Missed:**
1. No item on **the mix bus / submix architecture**, which is the thing that has
   to exist before any other audio item can be built: master, SFX, music, voice,
   UI submixes; ducking rules (does gunfire duck the ambient bed? does an
   announcement duck everything? does low health duck the world?); a limiter on
   master. 3.5 needs a "player-affecting submix" and 12.5 needs occlusion traces;
   neither works without the bus layout defined first.
2. No item on **weapon audio tails and interior acoustics** beyond one line in
   12.4. A gunshot in a tiled tunnel is 80% reverb tail; the reverb model (a
   reverb submix per station volume? convolution? UE's built-in?) is the
   difference between "toy gun" and "this is a real place". Named as "tuned per
   station" and left there.
3. No item on **the train's full audio arrival**, which the review brief
   explicitly asks about: distant rail rumble that grows, brake squeal, the
   specific "doors" chime (must be original, not the TfL three-tone), the
   departure whine, the Doppler as it passes, the sub-bass floor-shake. 4.1
   mentions "rumble bed" and "brake-squeal one-shot" in passing; there is no
   audio item that owns the train.
4. No item on **low-health audio**: heartbeat, tinnitus, muffled world, breathing.
   brief-v2 wants "grain intensity ramp as health drops"; the audio equivalent
   is unaddressed.
5. No item on **UI / interaction sound**: the wall-buy purchase sting
   (`OnPurchased` hook), the "can't afford" negative, the perk-acquire sound, the
   interact-available blip, menu navigation. `OnPurchased` is a Blueprint hook
   with a "flash and audio" comment and no spec.
6. No item on **round-start / round-end / special-round stings** owned by section
   12. They are scattered into 1.2, 1.8, 12.1 without a single "here is the sting
   inventory and what each sounds like".
7. No item on **the "3 original announcement lines per station"**: the review
   brief asks who voices them, what tech, and *what they say*. 4.4 and 12.2
   partially cover voice/tech (offline TTS, baked WAV) but no section commits the
   actual line content, the register, the word count, or a legal-pass rule for
   the phrasing beyond 15.2's "avoid 'mind the gap'".
8. No item on **music for the end-of-run screen** except one clause in 12.1 ("a
   run-end piece ... the only proper musical cue"). The one place the game
   *wants* music is under-specified.
9. No item on **audio performance budget**: voice count, streaming vs loaded,
   the cost of 44 spatialised zombie loops + occlusion traces + reverb submixes.
   Ties to the missing performance section.

**Weak defaults:**
- **12.1** "minimal, diegetic-leaning music. No continuous score. A low evolving
  drone bed that rises with round and heat." This is a defensible tonal position
  and matches "silence is a tool" (brief-v3). But the review brief flags the user
  "specifically asked about 'sample music'", and the audit's answer is
  essentially "almost no music, and no sample packs beyond Sonniss stems as
  texture". That may be right, but it should be presented as a genuine fork for
  the user (A: near-silent tension design; B: a restrained but present score with
  a station motif) rather than a default that quietly closes the door the user
  opened.
- **12.1** "a train-arrival motif (3 notes, descending, on the sodium board)".
  Three descending notes is close to the actual TfL/NR two-and-three-tone
  station chimes in cultural association; "on the sodium board" also implies a
  UI sound tied to a diegetic object. Needs a legal-pass note (15.2 territory)
  and a decision on whether the board makes sound at all.
- **12.2** "a single synthesised voice for all announcements, deliberately flat
  and slightly artificial". Reasonable and legally safe, but a fully robotic
  TTS voice across every station undercuts the "lived-in, something has happened"
  tone the reference frame and 14.4 are going for. Consider one flat synth voice
  for the *automated* lines (train arrival, stand clear) and a sparse, degraded
  human PA for the *station-character* lines, if a voice can be sourced.
- **12.3** ambient params ported as "0 to 1 floats" with Canary Wharf at "hum
  0.4, drip 0.7, wind 0.15". Fine starting values, but these are lifted straight
  from `debug-yard.ts` (a test fixture) and the audit says so; a test fixture is
  not a tuned reference.

**Wrong/miscategorised:**
- **12.4** "`FireSound` `USoundBase` field (one sound per weapon, currently
  unset)" - correct. But the audit does not note `MuzzleFlash` is typed
  `UParticleSystem` (Cascade), which is deprecated and mismatched with the
  Niagara-based free-assets FX plan; that is an audio-adjacent code-reality gap
  the audit is positioned to catch and does not.
- Section 12 is 5 items for what brief-v3 makes an entire phase (Phase 8) with a
  demanding accept criterion. The review brief asks "did the audit treat audio as
  seriously as systems?" - on the evidence of 5 items vs 10 for the train and 9
  for zombie AI, no.

### Section 13: Menus, persistence, progression

**Missed:**
1. No item on **the GameMode / GameInstance / GameState architecture**. Travel
   (4.7) persists "via the game instance / a travel payload struct", stats (13.2)
   need a persistence store, the round manager needs a station identity (6.5):
   all of this implies a `ULTGameInstance` and a `ULTGameMode` that do not exist
   in the module (verified: no GameMode, GameInstance, GameState, PlayerState in
   `Source/`). That is the backbone every persistence item hangs off and it is
   not an item.
2. No item on **`USaveGame` schema and versioning**. 13.3 says "one `USaveGame`
   written on every station arrival, every round end, and on quit". No schema, no
   version field, no corruption handling, no "what if the save is from an older
   build" - which matters because there is no CI and builds are hand-compiled.
3. No item on **first-run detection** (no save present) driving the onboarding
   (11.6) and the title screen ("New Run" vs "Continue" - 13.1 says no Continue).
4. No item on **the loadout picker existing or not in v1**. brief-v3 Phase 9
   lists it; 8.2 says "no loadout picker in v1, fixed pistol start"; 13.x never
   reconciles, so "is there a pre-run screen" is answered only implicitly and
   contradictorily.
5. No item on **settings persistence format** and when it applies (13.3 lumps
   settings into the meta save; a brightness slider needs to apply live and
   survive a crash before the first save).
6. No item on **quit-to-desktop vs quit-to-menu vs alt-F4** run handling: "quit =
   run over" (13.3) but what actually happens to the meta save on a hard kill
   mid-round?

**Weak defaults:**
- **13.3** "v1 has NO mid-run save". This is a real structural decision and the
  audit is right to force it, but it directly contradicts brief-v3 Phase 10's
  accept criterion as the audit quotes it ("continue at a second station with its
  round counter intact") only if you read that as in-memory travel. The audit
  makes that read but a stricter reading of "continue" is "resume after
  quitting". The audit should flag that Phase 10's wording needs a fix, not just
  assume the charitable reading.
- **13.4** "meta-progression is deliberately thin: Oyster Credit unlocks
  attachments and nothing else". Defensible, but it is asserted as a default when
  it is one of the biggest identity decisions for the game (arcade vs
  progression spine) and the user should own it. Also inconsistent with 7.6 and
  4.8, which between them build a fairly elaborate Credit economy with two earn
  paths, per-attachment costs, and a 15-to-20-run completion curve - that is not
  "thin".
- **13.5** "v1 has no win condition; the run ends only in death; document that
  v1 is endless". Probably correct for an arcade survival game, but it means the
  2-station line (6.1) has no destination, no "you reached the end", nothing to
  *complete*. For a game called LAST TRAIN with a line to travel, "there is
  nowhere the line goes" is a thematic hole the audit notes only as "the
  end-of-line screen is dormant".
- **13.6** gunsmith "does NOT pause the breather timer". Combined with 1.3's
  proposal to shrink the breather to 5s at round 20, the gunsmith becomes
  unusable in the breather at exactly the point in the run when re-kitting
  matters most, funnelling all gunsmith use onto the train and thereby
  strengthening the "always board" pull the heat mechanic is supposed to
  counter. The two defaults fight.

**Wrong/miscategorised:**
- **13.2** "kills by type ... no per-type tracking exists ... wire per-type kill
  counting into `OnZombieDied` when `ULTZombieData` lands (add a type enum to the
  broadcast)". Accurate, but this is a *Phase C code change to a delegate
  signature* (`FOnZombieDied` currently carries `(ALTZombieCharacter*, bool)`),
  and it should be listed as a Phase C dependency, not a Phase 9/G stats-screen
  detail, because the round manager and any Phase C telemetry also consume that
  broadcast.

### Section 14: Art direction gaps

**Missed:**
1. No item on **the grey-box-to-art-pass handoff mechanics**. 14.1 asks whether
   Phases C to E are grey, but not *how* the art pass consumes the grey box: is
   the blockout geometry replaced module by module, is it a reference to build
   alongside, does the grey box `L_CanaryWharf_Greybox` become `L_CanaryWharf`
   or a new map? brief-v3 Phase 7 has a priority order (platform, train, signage,
   lighting...) but the audit never turns that into a handoff process.
2. No item on **the VFX inventory**, which the review brief explicitly lists:
   muzzle flash, blood spray, death dissolve, water splash/ripple, sparks, dust,
   train headlight volumetrics, the flood water surface shader, screamer scream
   FX, gas cloud (3.3). free-assets points at the Niagara Examples pack. There is
   no item that enumerates what VFX the game needs and which are Phase C-
   functional vs Phase F-final. 14.6 covers lighting numbers only.
3. No item on **decal strategy**: blood pools (14.6 mentions a cap of 40),
   grime, water staining, signage-as-decal vs signage-as-mesh, the "Way out"
   panel substitute. `canary-wharf-research/materials-and-surfaces.md` exists and
   is not referenced by any 14.x item.
4. No item on **the post-process stack for the UE build**. brief-v2 had a full
   named chain; brief-v3 says "Lumen plus a tuned post process volume" and 14.6
   says "exposure locked, slight filmic tonemap". What else: bloom threshold,
   vignette, film grain, chromatic aberration (yes/no - it was in brief-v2), motion
   blur (yes/no), sharpen? A one-line "PPV contents" spec is missing.
5. No item on **whether the single-mesh-varied-by-scale-and-colour zombie reads
   as five types visually** - the review brief asks this directly and it is a
   real risk (brief-v3's honest-ceiling section admits per-zombie variation is
   not solo-achievable at fidelity). 3.x owns the stats; 14.x should own the
   *legibility* and does not.
6. No item on **lighting per mechanic**: blackout is covered (14.6), but flood
   ("flood reflections", the review brief's phrase) and interchange (a lit
   mezzanine vs a dark platform) have no lighting note. A blackout *and* a flood
   station lit the same way is a missed contrast.
7. No item on **the reference frame's mood without the branding** - the review
   brief's phrasing. 14.7 handles the HUD carve-out; nothing handles "the frame's
   atmosphere comes partly from the roundel's violet, the Johnston signage
   rhythm, the line livery - strip those and re-create the *feeling* with what?"

**Weak defaults:**
- **14.2 / 14.3** the station mark and typeface: the audit's defaults (a 5:1
  violet bar, Overpass) are sensible and match `free-assets.md` section E and
  `signage-and-wayfinding.md`. But these are "someone should design this in
  Phase F" items, not resolved questions, and they are counted in the 98 as if
  answered. The *decisions* the audit can actually force now are: (a) commit
  Overpass + Overpass Mono + Public Sans and import them (14.3 is right to push
  this), (b) name the line (15.1). The rest is a design brief for later.
- **14.4** naming three fictional brands including "a fintech (Sable & Crossline,
  violet-keyed)" and writing "~12 poster concepts". This is the audit doing
  worldbuilding. A fintech brand name and a lore thread about an outbreak are
  creative decisions for the user; the audit should say "the advertising needs 3
  recurring brands and a lore thread - user to define" and stop.
- **14.5** rating target "PEGI 16 / ESRB M". Fine, but asserted with no analysis
  of what in the game drives it (zombie violence, blood decals, no gore extremes
  per the restraint brief) and no note that "no dismemberment" is itself the
  decision that *keeps* it at 16 rather than 18. State the causal link.
- **14.6** torch "~1500 lm, ~35 degree cone, ~20m useful range, always on". The
  audit itself flags that brief-v3 does not mention a torch and it may be a
  headlamp in FP. Committing specific photometric numbers to a light source
  whose *existence* is unconfirmed is premature. Resolve "is there a torch/
  headlamp in the UE build" first (a yes/no the audit should escalate), then
  number it.

**Wrong/miscategorised:**
- **14.1** "grey box only" as the current visual state - correct, but the audit
  does not note that `L_CanaryWharf_Greybox` is *already built* (17 of 18
  NeoStack items, `NEXT.md`) with specific primitive choices (train shell as
  carriage boxes with door gaps, escalators as 30-degree slabs, flood as a
  marked slab). The "grey box plus signal colour" default for Phases C to E has
  to work *with* that committed geometry, which the audit treats as not yet
  existing.
- **14.3** "Overpass ... is not in the project or the engine". Correct per
  art-direction.md section 7, and the audit's push to import it is the single
  most actionable art item. But it is listed at the same priority as
  worldbuilding items; it should be near the top of the priority list (it blocks
  every future HUD and signage task).

### Section 15: Legal and naming

**Missed:**
1. No item on **the Elizabeth-line naming leak in existing docs**. `check_hygiene.py`
   "rejects TfL trademark leakage" (CLAUDE.md), and brief-v2, art-direction.md
   and `canary-wharf-grid.md` all contain "Elizabeth line" / "Jubilee line" in
   committed text. 15.1 says "'Elizabeth line' in shipped strings is a CI risk"
   but frames it as future; it is arguably *already* in the repo (in docs, not
   strings). Clarify whether the checker scopes to source/strings only or all
   tracked files.
2. No item on **the rolling-stock research folder's specificity**.
   `canary-wharf-research/rolling-stock.md` gives exact dimensions of the 1996
   Stock and Class 345 "shape, structure and wear are fair game". But it also
   describes the platform-screen-door arrangement and the "blank screen wall when
   the train has gone" - copying that arrangement closely is fine legally, but
   the audit never confirms the *train profile* choice (1996 vs 345) which is
   both a legal-adjacent and a gameplay decision (see section 4).
3. No item on **map / schematic legal specifics**. 6.5 and 15.5 touch the
   network schematic; neither states the concrete rules: no roundel-derived
   station markers, no Johnston, no the-real-diagram's-angles-and-colours, an
   original line colour that is not the Elizabeth purple or Jubilee grey (the
   project violet `#6C4C9C` is close to Elizabeth-line purple - worth a note).
4. No item on **ISO 7010 pictograms** as the safe signage route
   (`free-assets.md` section E recommends them, Wikimedia PD). The audit's
   signage discussion (14.2) never mentions the one concrete legally-clean
   pictogram source the research already found.
5. No item on **audio legal specifics**: the "doors closing" chime must not be
   the TfL/NR three-tone; announcement phrasing rules (15.2 covers "mind the
   gap" only); no transcribed recordings (in CLAUDE.md, not re-stated as an audit
   rule for Phase 8).
6. No item on **Sonniss licence constraints** flowing into the design: "not for
   AI training, no reselling individual sounds" (`free-assets.md`). If any audio
   is generated/processed by AI tools, that is a licence problem the audio design
   should know about.

**Weak defaults:**
- **15.1** line name: the audit recommends "Meridian line". Reasonable, but note
  `#6C4C9C` violet is visually close to the real Elizabeth-line purple, so a
  fictional line that is purple *and* serves Canary Wharf and Whitechapel (both
  real Elizabeth-line stations) reads as "the Elizabeth line with a different
  name". If the intent is genuine distance, either the line's accent colour or
  its station set should differ more. The audit picks the name and ignores the
  colour adjacency it flagged itself in passing.
- **15.2** "add 'mind the gap' (exact phrase) to the hygiene checker's reject
  list". Fine, but `signage-and-wayfinding.md` already lists "Mind the gap" as
  standard safety information the game will convey ("Same information, original
  panel geometry"). So the rule needs to be "the exact set phrase 'Mind the gap'
  as a standalone catchphrase is avoided; conveying the gap hazard in other
  words is fine" - which the audit says, but the checker rule as proposed (reject
  the substring) would also reject a legitimate rephrase that quotes it. Checker
  scope needs care.
- **15.6** "treat LAST TRAIN as a working title, check before release". Fine, but
  the audit could note now that "Last Train" is a very common title (multiple
  games, films, an album) and a distinguishing element (the line name, per the
  audit's own suggestion) is likely needed, so the line-name decision (15.1) and
  the title decision are linked and worth doing together.

**Wrong/miscategorised:**
- **15.5** "the in-world 'Meridian line' is explicitly a fictional route that
  happens to serve real stations in an order real lines do not ... same license
  Watch Dogs: Legion and others take". Slight mis-cite: `branding-precedent.md`
  says Legion *licensed* the roundel and used a mix of real and invented
  stations; the unlicensed precedent is *Modern Warfare 3* ("shipped a
  recognisable Underground level with zero TfL marks"). The audit should cite MW3
  here, which is the route LAST TRAIN actually takes, not Legion.

### Section 16: Doc contradictions and dead design

**Missed:**
1. **The flow-field contradiction.** `docs/reference/reference-frame-notes.md`
   section 4 still lists "Flow field plus spawn routes down the tunnel mouths" as
   the Phase B/D mechanism for the horde. brief-v3 discards the flow field
   entirely and the code uses `MoveToActor`. The audit catches this obliquely in
   2.1 ("the flow field was a web-build concept, discarded") but does not list it
   as a live doc contradiction in section 16, and reference-frame-notes is a doc
   a fresh art/AI session is explicitly told to read first (CLAUDE.md, README).
2. **The Elizabeth-vs-Jubilee rolling-stock contradiction** (see section 4).
   brief-v2/art call it a fictionalised Elizabeth line; the research folder and
   reference frame are Jubilee-line Canary Wharf (1996 Stock, platform screen
   doors). These imply different trains, platforms and boarding. Not in section
   16.
3. **The phase-letter vs phase-number confusion.** `docs/tasks/README.md` uses
   Phase A to G with a *different mapping* than brief-v3's Phase 0 to 11 (README
   Phase C = brief-v3 Phases 5+6+parts; README Phase G = brief-v3 Phases 8+9+10+
   11). The audit itself uses "Phase C", "Phase 9", "Phase 11", "phase-b3"
   interchangeably (e.g. 11.1 says "Phase C adds train, heat, perks" but README
   Phase C is rounds+types+train+board+heat and perks are README Phase E). A
   locked spec needs one phase vocabulary; the audit does not flag that there are
   two.
4. **`docs/tasks/README.md` supersedes brief-v3 Part 3** ("The plan below
   supersedes the roadmap in `brief-v3-unreal.md` Part 3 where they differ").
   The audit repeatedly cites "brief-v3 Phase N" as authoritative without noting
   that the tasks README is the live roadmap and brief-v3's phase list is itself
   partly superseded.
5. **The `00`-`08` Project Knowledge docs are cited as still-authoritative-in-
   spirit** by both briefs (`08_CLASSIC_ZOMBIES_STYLE_GUIDE.md`,
   `04_DEVELOPMENT_ROADMAP.md`, `01_GAME_VISION.md`, etc.) but are not in the
   repo. Multiple audit items lean on guidance "per `08_...STYLE_GUIDE.md`"
   (e.g. 11.1, 13.5) that no one can actually read. The audit should flag that
   several of its own citations point at absent documents.
6. No item on **`CommonUI` being a forced dependency** (`NEXT.md`: it is in the
   uproject "because NeoStack enabled it ... which SIGSEGV'd PIE ... Do not
   remove that entry"). That is a live constraint on any HUD work and a
   doc/reality note the audit's section 11 or 16 should carry.
7. No item on **the NeoStack trial expiry** (`NEXT.md`: "expires around
   2026-09-07"). Several audit defaults assume editor-side work by a NeoStack
   agent (2.4, 6.x, 11.x); after expiry that work needs a human or another tool.
   A planning doc dated 2026-09-05/06 should note the tool it leans on is about
   to be gone.

**Weak defaults:**
- **16.1** "add a prominent header to brief-v2 ... Better: extract the live
  gameplay numbers into a single new `docs/design/gameplay-canon.md`". The
  "better" option is the right one and should be *the* recommendation, not a
  hedge. As long as brief-v2 stays a "half-live" document, every future session
  re-derives which half. This should be a top-5 priority action, not buried in
  16.1.
- **16.5** "port the web `TILE_COST`, `validateStation` rules, `StationDef`
  shape". Sound, but this is a substantial engineering task (a `ULTStationData`
  asset, a UE data validator, nav-area classes) presented as a documentation
  note. It is really a Phase C work item and should be sized as one.
- **16.8** "downgrade the language: `phase-03` is a 'preserved rendering
  prototype', not a fallback plan". Correct and worth doing, but minor; it is
  listed with the same weight as the brief-v2-is-a-landmine item.

**Wrong/miscategorised:**
- **16.2** "brief-v3's first-person is canonical ... the C++ `ALTPlayerCharacter`
  is first-person". Verified correct (`Camera` on capsule at Z 68,
  `bUsePawnControlRotation`, `ViewModel` `SetOnlyOwnerSee`). Good.
- **16.3** duplicates 6.1 (the audit says so). Two item numbers for one
  contradiction inflates the 98 count.
- **16.7** ASCII grid role: the audit says "v3 wins, the grid is a blockout
  sketch". Correct, but then 16.5 wants to port `validateStation` (a *parser
  validator* for the grid/StationDef) into UE, which is in tension with "there is
  no generator, the grid is human-only". Porting the station *data* validator is
  fine; porting grid-shape validation is pointless if nothing parses grids. The
  audit does not separate these two.

## Cross-cutting gaps (things no section owns)

1. **Audio as a discipline.** 5 items for a whole phase with a hard accept
   criterion. No submix/bus architecture, no ducking model, no reverb/interior-
   acoustics model, no voice-count budget, no train-audio owner, no low-health
   audio, no UI-sound inventory, no sting inventory, no decision on the "sample
   music" question the user raised. Needs its own expanded section (see below).

2. **Accessibility.** Almost nothing. Colourblind is unaddressed and the palette
   is a deutan/protan trap (crimson heat, crimson damage, crimson emergency,
   crimson special-round vs violet accent). No subtitle spec despite the game
   putting the timer in audio. No "reduce camera shake" detail beyond a settings
   line. No difficulty option (13.4 rules it out - itself an accessibility
   decision made in passing). No remapping spec. No FOV/motion-sickness note
   beyond the slider. For a dark, audio-dependent, red-heavy FP horde game this
   is a serious hole.

3. **Performance budget and the crowd design constraint.** brief-v3 sets "60fps
   at 1080p, mid-range GPU, 24 zombies". Every "wall of zombies" default (2.7's
   28-to-44 cap, heat's +4/level, 8.1's penetration weapons, 44 spatialised audio
   sources, corpse pools) spends against a budget no section tracks. No item on:
   the crowd system choice (per-agent AIController + RVO as now, vs DetourCrowd,
   vs Mass), the animation budget (skeletal mesh LODs, update-rate optimisation,
   the "one mesh" vs City Sample Crowds question), draw calls, the corpse/decal/
   VFX cost, or the profiling method. This is a whole missing section.

4. **Co-op / networking.** Answered once, late, as item 10.4 ("solo only"). This
   should be a stated scope boundary up front that explicitly gates HUD, revive,
   targeting, travel and economy design, not a player-mechanics footnote.

5. **Onboarding / tutorial.** 11.6 is the only item and its default (a slightly
   longer prompt the first time you look at a wall buy) does not meet brief-v3's
   "first ten minutes feel coherent" bar. No spec for teaching: the train is an
   escape, heat exists and why you'd stay, wall buys, perks, the downed state,
   the no-minimap wayfinding. Needs its own small section.

6. **Anti-training / kiting-exploit AI design.** The defining problem of
   round-based survival: the player runs a loop and the horde trails in a
   conga line, making high rounds trivial. brief-v3's reference frame *is* "a
   horde funnelled down a corridor" - the exact geometry that enables a train
   loop. Nothing in the audit addresses: horde spread/flanking to punish
   predictable loops, spawn-point selection that puts zombies *ahead* of a
   running player, the sprinter's role as the anti-kite answer (and its speed
   must therefore beat the loop - section 3's 340 does not), or the escalator
   conveyor (6.2) which the audit designs as an exploitable one-way perch.
   Needs its own section.

7. **Telemetry / balance methodology.** brief-v3 Phase 11 and brief-v2 Phase 8
   both say "profile first, report every number you change and why", and the
   target is "round 12 to 15 on a first serious run". There is no item on how
   that is measured: what events are logged, is there a dev telemetry overlay
   (brief-v2 wanted a profiler overlay), a playtest protocol, a way to A/B a
   number. Every "tune in Phase 11" hand-wave in the audit (dozens of them) has
   no methodology behind it.

8. **Run integrity / save-scumming.** 13.3 says no mid-run save (quit = over),
   which mostly closes this, but there is no leaderboard/best-round-persistence
   integrity note (13.2 persists "best round" - can the player edit the save?),
   no anti-cheese statement beyond 11.3's pause-disable (which the review argues
   against), and no decision on whether alt-F4 mid-down counts as a death.

9. **The GameMode/GameInstance/GameState backbone.** No item anywhere. Travel,
   persistence, station identity, the round manager's station-awareness, stats
   aggregation, first-run detection all need it and the module has none.

10. **VFX inventory** (folded into art above but genuinely cross-cutting): no
    single list of every particle/decal/shader effect the game needs, which are
    Phase C-functional, which Phase F-final, and where each comes from
    (Niagara Examples pack vs bespoke).

## Priority list correction

The audit's top 10, assessed:

| Audit # | Item | Verdict |
|---|---|---|
| 1 | 3.1 per-type stat block | **Keep, #1.** Correct: Phase C cannot start without it. |
| 2 | 6.2 mechanic definitions | **Keep, but narrow to #3.** Only flood/interchange/blackout/platform_split matter for v1; the other 6 are expansion docs. |
| 3 | 6.1/16.3 station count | **Keep, #4.** Right that it drives everything; the decision itself (2) is quick. |
| 4 | 4.5/4.6/4.7 train flow | **Keep, but split.** The train *arena/profile* question (Elizabeth vs Jubilee, platform screen doors) is a hidden top-5 blocker; the boarding-flow UI is important but secondary. |
| 5 | 5.1/5.3 heat | **Keep, #6.** But the real problem is the *three inconsistent number sets*, not the missing payoff table. |
| 6 | 8.1/8.7 weapon roster + 2-weapon carry | **Downgrade to #9.** The roster is Phase C/E and mostly Phase 11 tuning; 2-weapon carry is unimplemented-feature not bug. Not a Phase C blocker. |
| 7 | 1.7/9.3 downed/revive | **Keep, #7.** Correct that the run has no ending. Note the `bDead` refactor scope. |
| 8 | 10.2 sprint stamina | **Drop from top 10.** This is the audit proposing a new system, not resolving an ambiguity. The genuine issue (horde can be outrun) is better fixed via sprinter speed (part of #1) and arena design. |
| 9 | 16.1/16.5 brief-v2 half-dead + web data port | **Keep, #5.** Split: "make a gameplay-canon doc" is a 1-hour top-5 action; "port StationDef/validator to UE" is Phase C engineering. |
| 10 | 15.1 line name | **Keep, #8.** Cheap, wide reach, blocks announcement text. Bundle with the title decision (15.6) and the accent-colour-vs-Elizabeth-purple note (15.5). |

**Corrected top 10 (ranked):**

1. **3.1 (+ 2.2, 3.4) per-type zombie stat block, grounded in the player speeds
   and the HP compounding curve.** Phase C blocker. Fix sprinter speed against
   `WalkSpeed 420` / `SprintSpeed 640`, not a bare 340.
2. **NEW: Train profile and platform arena - Elizabeth line (Class 345, no PSDs,
   walk-through, main-line gauge) vs Jubilee line (1996 Stock, full-height
   platform screen doors, deep tube).** Decides the arena, the "train as a wall"
   read, the boarding geometry, trackbed access (4.3), and whether there is a
   trackbed to fall into at all. Phase C blocker, currently a silent contradiction
   between brief-v2 and the research folder.
3. **6.2 mechanic definitions for the 4 v1 mechanics only (flood, interchange,
   blackout, platform_split), with flood's rise rate and fire-block reconsidered
   and the interchange escalator de-exploited.**
4. **6.1/16.3 commit to 2 stations and name station 2 (and its mechanic pair -    reconsider blackout as #2).**
5. **16.1 extract a single `docs/design/gameplay-canon.md` from brief-v2's live
   numbers; demote brief-v2 to a historical note.** 1 hour, removes the landmine
   under every future session.
6. **5.1/5.3/2.7 pick ONE heat model and ONE `MaximumAlive` story**, expressed as
   a single rate multiplier + a single cap add, with the round-20-heat-6 worked
   number shown, and left as "profile-pending" not "28".
7. **1.7/9.3/10.5 downed-and-revive as a coherent solo mechanic**, with the
   `bDead` -> `bDowned` refactor scoped, and a disengage-based bleed-out instead
   of "kill 4 while crawling".
8. **15.1 name the line** (+ reconcile the violet accent against Elizabeth-line
   purple; bundle the title decision).
9. **8.1 roster *shape* only** (count, archetypes, tiers, which need code:
   projectile system for crossbow/joke weapon is NOT small), all numbers
   explicitly deferred to a Phase 11 tuning doc. Plus 8.7 2-weapon carry as
   scoped Phase C/E work.
10. **NEW: performance budget + crowd system decision** (per-agent AIController+RVO
    vs DetourCrowd vs Mass; animation LOD/update-rate budget; the "one mesh" vs
    City Sample Crowds question from free-assets), because #1, #3 and #6 all spend
    against it and there is no ledger.

Buried-but-should-be-higher: the **GameMode/GameInstance backbone** (no section,
needed for #2/#6/#7 and all of section 13), and **importing the typeface**
(14.3 - blocks every future HUD/signage task, trivial to do now).

Over-rated: **10.2 stamina** (new system, not an ambiguity), **7.4 points cap**
(non-issue), **6.6 grid normalisation** and **10.3 interact range** (cleanup
tickets padding the count, not open questions).

## New sections the audit needs

- **17. Performance budget and the crowd system.** The frame budget, the crowd
  architecture choice, animation LOD and update-rate strategy, the one-mesh vs
  crowd-variation decision, draw-call/decal/VFX/corpse ceilings, the profiling
  method and the dev overlay. ~10 items.
- **18. Onboarding and the first run.** Teaching the train-as-escape, heat, wall
  buys, perks, downed, wayfinding without a minimap; first-run-only beats vs
  every-run; the "how to play" screen and how it is actually surfaced. ~6 items.
- **19. Anti-training AI and arena design.** Horde spread/flank behaviour vs
  predictable loops, spawn selection that gets ahead of a running player, the
  sprinter as the anti-kite answer and the speed that requires, escalator/choke
  exploits, the "conga line" problem the reference-frame geometry invites. ~7
  items.
- **20. Telemetry and balance methodology.** What is logged, the dev telemetry/
  profiler overlay, the playtest protocol for "round 12 to 15 on a first serious
  run", how numbers get A/B'd, where the tuning doc lives. ~6 items.
- **21. Core game framework (GameMode / GameInstance / GameState / SaveGame).**
  The backbone objects that do not exist: travel payload, station identity
  injection into the round manager, stats aggregation, persistence schema and
  versioning, first-run detection. ~8 items.
- **22. Audio architecture** (or a major expansion of section 12): submix/bus
  layout, ducking rules, reverb/interior-acoustics model, voice-count budget,
  the train-audio spec, low-health audio, UI-sound and sting inventories, the
  announcement line content and its legal-pass rule, and a genuine A/B fork on
  the "sample music" question. ~12 items.
- **23. Accessibility.** Colourblind (the palette is a red/violet trap),
  subtitles and caption UI, camera-shake and motion, input remapping, difficulty
  as an accessibility lever, brightness calibration (partly in 11.4). ~7 items.

## Bottom line

- Audit items that are solid as-is: **~58 of 98** (mostly sections 7, 8, 9, 13,
  16 where the audit is confirming coded values or making clean scope calls;
  weakest coverage is 12, 14, and anything touching feel, audio architecture or
  performance).
- Items whose default needs rework before locking: **1.3, 1.4, 1.5, 2.1, 2.6,
  2.7, 3.1, 3.2, 3.3, 4.1, 4.7, 4.8, 5.1, 5.2, 6.1, 6.2, 6.7, 7.1, 7.3, 7.6,
  8.1, 8.3, 9.1, 9.3, 9.4, 10.1, 10.2, 11.1, 11.2, 11.3, 11.6, 12.1, 12.2, 13.3,
  13.4, 13.6, 14.4, 14.6, 15.1, 16.1** (~39 items).
- Estimated missing items: **~55 to 65** (roughly: 6 new sections at ~7 items
  each = ~46, plus ~12 to 18 gaps inside the existing 16 sections listed above).
  Call it **~98 covered, ~60 missing - the audit is roughly 60% complete by
  question count and less than that by risk coverage**, because the missing
  material is concentrated in the highest-uncertainty areas (audio architecture,
  performance, the anti-training problem, the framework backbone).
- **Recommended: one more sweep pass, then lock.** Specifically, before locking:
  (a) add sections 17 to 23 above; (b) reconcile the heat / `MaximumAlive`
  numbers into one model; (c) resolve the two silent contradictions the audit
  missed (flow-field in reference-frame-notes, Elizabeth-vs-Jubilee rolling
  stock) and the phase-letter-vs-number vocabulary; (d) re-label every "suggested
  default" that introduces new design (stamina, melee, self-revive budgets,
  signal flare, advertising brands, the Inspector boss, the full weapon numbers)
  as "PROPOSAL - needs sign-off" so a spec-writer does not enshrine them; (e)
  pull the corrected top 10 and the "make gameplay-canon.md" + "import the
  typeface" quick wins forward. The audit does not need to be redone; it needs
  its blind spots filled and its proposals flagged as proposals.

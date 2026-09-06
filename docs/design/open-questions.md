# Open design questions, consolidated sweep, 2026-09-06

Consolidated version, dated 2026-09-06. This merges the original ambiguity audit
(2026-09-05, `open-questions.md`) with the adversarial fresh-eyes review
(`open-questions-review.md`, kept as the record of that review). It is the single
authoritative list the project owner works through decision by decision.

Each item is something a future implementation session would have to guess at.
Two label styles are used, and the difference matters:

- **Suggested default** means "confirm the obvious reading of an ambiguity". A
  tick is enough.
- **PROPOSAL - needs sign-off** means the item is inventing a system or a design
  direction that is not implied by any existing doc. These need an actual design
  decision, not just a tick. They are called out so the owner can see instantly
  which is which.

## How to use this

- Read each item. Mark it AGREED, CHANGED (write the new answer), or DISCUSS.
- PROPOSAL items specifically need a design decision, not just an AGREED tick.
  If a PROPOSAL is accepted as-is, write AGREED and note that the design is now
  owned; if not, it needs a replacement direction, not a deferral.
- Once resolved, decisions fold into a new `docs/design/gameplay-canon.md`
  extracted from brief-v2's live numbers (see 16.1), then into
  brief-v3-unreal.md by section. Decisions are NOT folded in by this document;
  a separate process does that.
- Items are numbered SECTION.N so they can be referenced in later chats and commits.

## Source key

- **brief-v2** = `docs/brief-v2.md` (LIVE only for round loop, train timing,
  heat, zombie types, mechanic library, economy, legal; DEAD for engine,
  renderer, camera, phases, gates, asset strategy - see 16.1).
- **brief-v3** = `docs/brief-v3-unreal.md` (engine, camera, model split; its
  Part 3 phase list is partly superseded by `docs/tasks/README.md` - see 16.11).
- **tasks README** = `docs/tasks/README.md` (the live roadmap, Phases A to G).
- **art** = `docs/art-direction.md`. **notes** = `docs/reference/reference-frame-notes.md`.
- **research** = `docs/reference/canary-wharf-research/` (architecture, materials,
  signage, rolling stock).
- **grid** = `docs/reference/canary-wharf-grid.md`. **legend** = `web/src/data/legend.ts`.
- **schemas** = `web/src/data/schemas.ts` (web-era TypeScript, unported).
  **debug-yard** = `web/src/data/stations/debug-yard.ts` (a test fixture, not a
  tuned reference).
- C++ file references are to `Source/LastTrain/`, verified against source for
  this consolidation.

## Phase vocabulary

This document uses the **Phase A to G letters from `docs/tasks/README.md`** as
canonical (see 16.11 for the proposal to standardise on them). Rough mapping to
the old brief-v2/v3 numbers, for reading older docs:

| Letter | Content | Old numbers (approx) |
|---|---|---|
| A | Foundation, first playable grey box | 0 to 3 |
| B | Engine core hardening (repath, interaction, HUD) | 4, parts of 1 |
| C | Rounds, five zombie types, the train, the departure board, station heat | 5, 6 (train/heat/types) |
| D | Grey box Canary Wharf | 5 to 7 (blockout) |
| E | Perks, upgrade bench, lost property, downed and revive | 6 |
| F | Art pass, Fable led, against the reference frame | 7 |
| G | Audio, restrained HUD, second station, balance | 8, 9, 10, 11 |

---

## 1. Game start and round loop presentation

### 1.1 No game start sequence

**Ambiguity:** Nothing describes what happens between the level loading and round
1 spawning. `ALTRoundManager::BeginRounds` calls `StartRound(1)` which immediately
sets `PendingSpawns` and broadcasts `OnRoundStarted`; spawning begins on the next
tick after `BaseSpawnIntervalSeconds` (1.6s). brief-v3 Phase 1 to 2 only require
"rounds 1 to 5 play without touching the editor". No title card, no pre-round
countdown, no player control lockout, no establishing beat.
**Currently in code:** `BeginRounds()` starts round 1 instantly. `L_GreyboxTest`
level Blueprint calls it on `BeginPlay`.
**Why it matters:** the first ten minutes feeling coherent is brief-v3's stated
success test. A cold start straight into spawns has no chance to read the space
or the fiction.
**Suggested default:** on `BeginPlay`, hold the round manager for a 5s
pre-round beat. During it: player has full movement but weapons disabled, a
single platform announcement plays ("This service is not in operation. Please
leave the platform." or similar station-specific line), the departure board shows
the first train countdown already running. At 5s, a low sodium title fade of the
station name (2s, no box, no "ROUND 1" text) then `BeginRounds()`.
**Rationale:** 5s is long enough to hear one announcement line and read the
platform without stalling a player who just wants to fight; it is not a
cutscene.
**The first-train zero (resolves the 1.1 / 4.2 gap the review flagged):** there
has been no prior departure, so the first train cannot count "departure to
arrival" like every later one. Define it as: the inbound slide begins at level
load + 20s (so the 5s pre-round hold is absorbed, not added), the train STOPS at
level load + ~30s, giving a deliberately short first cycle as a teach, then the
normal 100s departure-to-arrival interval governs every train after. Expose
`FirstTrainStopSeconds = 30` on the train actor.
**First run vs subsequent run (the review's missing item):** a first-ever run
(no save present, see 21.x) gets a longer establishing hold (8s, not 5s) and one
extra announcement line ("This platform is closed. The next service is not
scheduled to stop."). Run 2 onward uses the 5s beat. First-run detection comes
from the meta save (see 21.x).

### 1.2 Round-start feedback undefined

**Ambiguity:** brief-v3 Phase 9 and phase-b3 say "No banner, no round change
animation beyond a quiet fade" for the HUD number. That covers the widget. It
does not cover audio (a round-start sting? brief-v3 Phase 8 lists "round
stingers" with no spec), light change, or any diegetic signal.
**Currently in code:** `OnRoundStarted(int32)` fires; `WBP_HUD` fades the number.
Nothing else.
**Why it matters:** in a dark station with a diegetic-first design, the player
needs to know a wave has begun without a HUD banner. Silence plus a fading number
is easy to miss.
**Suggested default:** a single low sub-bass swell (0.8s, procedural, ~40Hz
rising to ~80Hz) at round start, plus the platform emergency lighting (crimson)
pulses once down the tunnel. No voice.
**Rationale / change from the original:** the original reused one motif for
round-start, breather-end and (pitched up) special rounds, which is three
meanings on one sound. Use three distinct short motifs instead: round-start is
the rising sub-bass swell above; breather-end is a short two-note falling figure
(the "get ready" tell); special rounds keep the pitched-up round-start variant
(1.8). Log the full sting inventory in section 22 (audio). For colourblind
players the crimson pulse must not be the only round-start signal - the audio
swell carries it (see 23.x).

### 1.3 Breather presentation undefined

**Ambiguity:** brief-v2 and the round manager define a 10s breather
(`BreatherSeconds = 10.f`) but nothing says what the player experiences. Is the
station quieter? Does music play? Can the player still be attacked by a straggler
(there are none: `EndRound` only fires when `LiveZombies.Num() == 0`)?
**Currently in code:** `bInBreather = true`, `BreatherRemaining = 10.f`, ticks
down, then `StartRound(CurrentRound + 1)`. No presentation.
**Why it matters:** the breather is the only window for wall buys, reloading,
repositioning and (later) the gunsmith. Its length and feel is a core pacing
lever and it is currently invisible.
**Suggested default:** on `EndRound`, drop the combat ambient bed to a quiet
station hum, raise station lighting ~15% (a brief respite feel), and show the
breather as a thin sodium countdown ring around the crosshair for the last 3s
only (not the full 10). Stragglers are impossible by design; keep it that way.
**Breather length (changed from the original):** hold `BreatherSeconds = 10`
through the whole run and NEVER go below 8s. Add a longer breather (14s) on the
round immediately AFTER a special round (6, 11, 16, ...) so the player can
recover and re-kit. Expose `BreatherSeconds` and `PostSpecialBreatherSeconds`
as `EditDefaultsOnly`.
**Rationale:** the breather is the only off-train window for wall buys, reload,
repositioning and the gunsmith (13.6, which explicitly does not pause the
breather). Shrinking it to 5s at round 20, as the original proposed, makes the
gunsmith almost unusable off the train and funnels all re-kitting onto the
train, which strengthens the "always board" pull that heat (section 5) exists to
counter. Late-game tension comes from spawn interval, count and heat, not from a
starved breather.

### 1.4 Round-end trigger has no grace for unreachable zombies

**Ambiguity:** `EndRound` requires `LiveZombies.Num() == 0`. If a zombie is stuck
on geometry, falls out of the navmesh, or a spawn point is walled off, the round
never ends. brief-v3 Phase 2 accept criterion is "the round ends exactly once"
with no mention of the stuck-zombie failure mode.
**Currently in code:** no timeout, no teleport-to-player, no stuck detection.
`EndRound` fires only when `PendingSpawns == 0 && LiveZombies.Num() == 0`. Note
the current `DriveTowardsTarget()` direct-push fallback (see 2.5) already keeps a
zombie that cannot path *moving*, so the pure "frozen on geometry" case is
partly mitigated, but a zombie shoved into an unreachable pocket by the push,
or one that falls off the navmesh entirely, still stalls the round.
**Why it matters:** a single stuck zombie soft-locks the run. Canary Wharf grey
box has debris doors, a flood slab and escalator ramps, all navmesh edge cases.
**Suggested default:** add a per-zombie "unreachable" check: if a live zombie has
not reduced its straight-line distance to the player for 8s AND is more than 40m
away, teleport it to the nearest spawn point out of the player's view. If that
fails twice, destroy it and decrement the live count. Log every such event with
`LT_LOG(Warning, ...)`.
**Changed from the original:** drop the "hard 4-minute round-length ceiling". A
legitimate high-round fight against a full concurrent cap with a weak weapon can
genuinely exceed 4 minutes, and force-killing the horde hands the player free
points and breaks the difficulty contract. Replace it with genuine stall
detection: if no zombie has died AND the player has neither dealt nor taken
damage for 90s, run the unreachable check on every live zombie immediately, and
if that clears nothing, destroy the remaining live zombies and end the round
with an `LT_LOG(Warning, ...)`. This fires only on a true stall, never on a slow
fight.
**Rationale:** 90s of zero deaths and zero damage in either direction is not a
fight, it is a bug; a fight-length timer punishes the player for the game's own
pathing failure.
**Spawn-point starvation (the review's missing case):** `TrySpawnOne` bails
silently if no point `IsAvailable` (all on cooldown, all `FirstRound` in the
future, all `bEnabled == false` behind shut doors). On a small station with
doors closed, the round can stall with `PendingSpawns > 0` and nothing able to
spawn. Add: if `PendingSpawns > 0` and no point has been available for 15s,
temporarily lift the `CooldownSeconds` gate (not the door gate) so at least one
point can fire, and `LT_LOG(Warning, ...)`. If even that fails (every point is
door-gated shut), reduce `PendingSpawns` toward the current `LiveZombies` count
so the round can end. Expose `SpawnStarvationGraceSeconds = 15`.

### 1.5 First-round zombie count is a placeholder

**Ambiguity:** `OpeningRoundCounts = {6, 8, 10, 12, 14}` in `LTRoundManager.h`.
brief-v2 says only "spawn rate climbing" and does not give opening counts.
NEXT.md notes the placed `L_GreyboxTest` instance overrides this to `(6,8,10,...)`
as well. No design doc justifies 6 as the round 1 count.
**Currently in code:** `{6, 8, 10, 12, 14}` then `CountGrowthPerRound = 2.4` per
round past index 5. `ComputeRoundCount` rounds `Last + 2.4 * (Round - 5)`.
**Why it matters:** brief-v3 Phase 11 targets "round 12 to 15 on a first serious
run". The opening curve sets whether that target is even in range.
**Suggested default (changed from the original):** leave the coded values
`OpeningRoundCounts = {6, 8, 10, 12, 14}` and `CountGrowthPerRound = 2.4`
exactly as they are, marked explicitly PROVISIONAL, and defer the opening-curve
shape to the Phase G balance pass with the concurrent-zombie cap (5.1, 2.7) as a
joint tuning target. Do not invent a second set of placeholder numbers
(`{6,8,12,16,22}`) to replace the first: with the cap itself unresolved, the two
curves cannot be tuned independently in prose, and a second guess is not more
authoritative than the first.
**Rationale:** the round total is a budget, not a difficulty in itself - the
concurrent cap (5.1) sets the pressure, the count sets how long the round runs.
brief-v3's "reaches round 12 to 15 on a first serious run" target is measured,
not derived (see section 20); it cannot be back-solved into an opening curve
without playtest data.
**Fix the instance override:** per NEXT.md the placed `GreyboxTest_RoundManager`
in `L_GreyboxTest` carries a stale per-instance `OpeningRoundCounts` override at
`(6,8,10,12,14)`. Reset it to inherit the asset. This is a cleanup task, not a
design question.

### 1.6 Spawn interval decay unjustified

**Ambiguity:** `BaseSpawnIntervalSeconds = 1.6`, `SpawnIntervalDecay = 0.96`
compounding, `MinimumSpawnInterval = 0.35`. No doc states the intent. By round
~30 the interval is at the floor. brief-v2 says only "spawn rate climbing".
**Currently in code:** `ComputeSpawnInterval = 1.6 * 0.96^(Round-1)`, floored at
0.35.
**Why it matters:** this is the main "pressure over time" curve alongside
`MaximumAlive` and heat. Its shape decides whether late rounds feel like a
grind or a wall.
**Suggested default:** keep the values but state the intent: round 1 spawns one
zombie every 1.6s, halving to ~0.8s by round 17, hitting the 0.35s floor around
round 30. Heat multiplies the effective rate through the single heat model in
5.1 (one multiplier on the final spawn rate, not a separate compounding
decay). Add a note that the concurrent cap (5.1) is the real late-game limiter
and the interval floor should always be able to refill the cap within ~10s of
kills.
**Worked figure (per the review's request to show the compounded number):** at
round 20, heat 6, with the 5.1 model (heat as a single `x0.6` multiplier on the
final rate, cap add capped by the perf budget), the effective spawn interval is
`1.6 * 0.96^19 * 0.6` which is approximately `0.44s`, above the 0.35 floor. This
is the number Phase G sanity-checks against a real profile.

### 1.7 Death and downed presentation undefined

**Ambiguity:** `ALTPlayerCharacter::TakeDamage` sets `bDead = true` and calls
`OnDied()` (a `BlueprintImplementableEvent` with no implementation). brief-v3
Phase 6 lists "downed and revive" with no spec. Solo play: what does downed even
mean with no second player?
**Currently in code:** `bDead` blocks input in `Move`/`StartFire`/etc, `OnDied()`
is an empty Blueprint hook. No respawn, no restart, no score screen.
**Why it matters:** the run has no ending. Phase 6 accept is "a complete survival
session start to death is possible" and there is nothing at "death".
**PROPOSAL - needs sign-off:** the whole "downed / last-stand / self-revive"
concept is new design. brief-v3 names "downed and revive" for a solo game and
nothing else; there is no coded downed state. Proposal: on reaching 0 HP the
player enters a `bDowned` state (separate from `bDead`): crawl at 30% `WalkSpeed`,
camera lowered to ~30cm, free-look still works, ADS is force-released and cannot
re-engage, no sprint, no jump, can fire the sidearm only. A bleed-out timer
shows as a crimson vignette closing in. Revive by **disengagement**: get more
than 12m from every live zombie for 6 continuous seconds, which restores 40 HP.
This is once per round free; a second down the same round, or the bleed-out
expiring, ends the run (fade to black, end-of-run screen, 13.5).
**Rationale for disengage-based revive (changed from "kill 4 while crawling"):**
a downed player on 30% move speed with a pistol, while a full horde closes,
almost never kills 4, and "survive to the next breather" can be 2+ minutes away
on a high round, so the free revive was usually unachievable and every down was
effectively run-ending, making the whole state theatre. A crawl speed CAN break
12m of contact if the player disengages cleanly, so the mechanic rewards the
right instinct (get out) rather than the wrong one (keep shooting).
**Scope note (the review's point):** `bDead` is a single bool that already
permanently blocks `Move`, `StartFire`, `StartAim`, `Reload` and `Interact` in
`LTPlayerCharacter.cpp`. Implementing `bDowned` means untangling every one of
those `if (bDead)` guards into a three-state check (alive / downed / dead). This
is a non-trivial Phase E refactor, not a hook to fill. Size it as such.
**Bleed-out length:** see 9.3 (25s baseline, PROPOSAL).

### 1.8 No "wave composition" telegraph

**Ambiguity:** brief-v2 says "Sprinter horde every 5th round, brute pair every
10th". Nothing tells the player a special round is coming. Classic zombies uses a
distinct round-change sound for the "dog round" equivalent.
**Currently in code:** no per-type spawning at all yet (Phase C). Round manager
spawns one `ZombieClass`.
**Why it matters:** an unannounced sprinter horde on round 5 is a difficulty
cliff with no counterplay signalling.
**Suggested default:** a special round (sprinter horde, brute pair) gets a
distinct announcement line the round before ("The next service will be fast
moving" style, station-agnostic, original phrasing) played during the breather,
plus the round-start swell (1.2) pitched up a fifth. The departure board shows a
small warning glyph. No HUD banner.

---

## 2. Zombie AI and locomotion

### 2.1 Pathfinding is bare MoveToActor, no crowd texture

**Ambiguity:** `ALTZombieCharacter::Tick` calls `AI->MoveToActor(CurrentTarget,
AttackRange * 0.5f)` on the repath cadence (0.35s +/- jitter). RVO
(`AvoidanceConsiderationRadius = 45`) is the only separation. free-assets.md
notes "a StateTree or Behaviour Tree plus EQS would be a cheap upgrade later".
No spec for path weave, wander, flanking, or lane preference, so the horde is a
line down the shortest path.
**Currently in code:** `DriveTowardsTarget()` calls `AI->MoveToActor(CurrentTarget,
ContactRange)` (ContactRange 15, edge to edge) on the repath cadence
(`RepathIntervalSeconds` 0.35s +/- `RepathJitterFraction` 0.4), with a
direct-`AddMovementInput` push fallback when there is no controller or the path
request returns `Failed`. RVO at `AvoidanceConsiderationRadius` 45. No behaviour
tree, no EQS, no flow field. Note: the flow field was a web-build concept,
discarded here, but `reference-frame-notes.md` section 4 still lists it as the
Phase B/D horde mechanism - a stale doc reference, flagged in 16.9.
**Why it matters:** the reference frame is "a horde funnelled down the corridor".
A single-file line of zombies on the navmesh shortest path is not that, and the
flood-split and barrier geometry in Canary Wharf exists specifically to break the
line up but does nothing if every zombie hugs one edge.
**Suggested default (changed from the original):** do NOT apply a lateral offset
to the goal location - that makes every zombie weave toward the same moving
phantom point and the whole horde lists to one side. Instead: (a) each zombie
picks a lateral lane bias (-1, 0, +1) at spawn, applied as a path-follow lateral
offset in the movement component (offset along the path, not on the goal); (b)
accept that real crowd texture (individual wander, flanking, encirclement) needs
`DetourCrowd` or a StateTree and is a Phase C+ system decision, tracked in
section 19 (anti-training AI) and section 17 (crowd system choice), not a
one-line goal hack. Cost-weight water and barrier tiles via `UNavArea` classes
carrying the web `TILE_COST` values (water 3, barrier 4, escalator 1.4, per
`legend.ts` and 16.5) so the horde splits around them - but see 6.2's note that
the grey box may already be built without cost weighting, which must be verified.
Keep `MoveToActor` for v1; do not add a behaviour tree.
**Attack-slot / encirclement rule (the review's missing item):** `MoveToActor(target,
15)` gives every zombie the same tiny acceptance ring, so the whole horde funnels
to one point and the back rows shove the front rows into the player. Add a simple
attack-slot reservation: divide the ring around the player into N angular slots
(N ~ 8), a zombie within ~3m claims the nearest free slot as its goal offset,
zombies with no free slot hold at ~2.5m until one opens. This is the minimum
needed to stop the conga-pile; the fuller anti-kite behaviour is section 19.

### 2.2 No per-zombie gait variation

**Ambiguity:** every zombie of a type moves at exactly `BaseWalkSpeed +
SpeedPerStep * Steps`. No per-instance speed jitter, no animation playback rate
variation. brief-v2 v1 crowd notes (now discarded) had "per-instance playback
rate". Nothing carried into brief-v3.
**Currently in code:** `ApplyRoundScaling` sets `MaxWalkSpeed` to a single value
for all zombies of that round.
**Why it matters:** uniform speed makes the horde read as one organism sliding
forward, not individuals. It also means the horde never spreads out along the
corridor.
**Suggested default:** on spawn, multiply each zombie's `MaxWalkSpeed` by
`FRandRange(0.88, 1.12)` and set its anim play rate to match. Store the
multiplier so `ApplyRoundScaling` re-applies it. Sprinters get a tighter band
(0.95 to 1.08) so they stay a coherent threat.

### 2.3 Target is only ever the player

**Ambiguity:** `CurrentTarget = UGameplayStatics::GetPlayerPawn(this, 0)` in
`BeginPlay` and re-fetched if null in `Tick`. No target reselection rules. brief
never mentions zombies attacking barriers, debris doors, boarded windows, or the
train.
**Currently in code:** single hardcoded target, the player pawn.
**Why it matters:** classic zombies has zombies tear down boarded windows and
barriers, which paces the early game. The grid has `B` (boarded) tiles and `D`
(debris doors) with no interaction. If zombies ignore them, the boards are
decoration.
**Suggested default:** for Phase C keep target = player only. Note as a Phase D/E
follow-up: boarded windows (`B` tiles) become breakable spawn covers that a
zombie in contact with (and no direct path to the player through) will attack
over ~4s, then step through. This gives the grey box a reason to have the boards.
Debris doors stay player-only purchasable (Phase E). The train is never a target.

### 2.4 Spawn-in behaviour undefined

**Ambiguity:** brief-v3 notes/reference say zombies "emerge from the tunnel". The
round manager spawns them at `Chosen->GetActorLocation()` and they immediately
`MoveToActor`. No spec for whether they fade in, climb out of the trackbed, walk
out of a dark tunnel mouth, or just appear.
**Currently in code:** `SpawnActor` at the spawn point transform,
`AdjustIfPossibleButAlwaysSpawn` collision handling. Instant.
**Why it matters:** popping in inside the player's view breaks the fiction badly,
and the reference frame's whole premise is the horde walking out of the dark.
**Suggested default:** spawn points are placed inside tunnel mouths / behind the
`T` openings, out of direct line of sight, per the grid. Zombie spawns with a
0.4s spawn-protection (no damage dealt or taken) and a short "emerge" state:
walks forward 2m at half speed before normal steering engages. If a spawn point
is in view of the player when it fires, skip that point that tick and pick
another (add a `bInPlayerView` check to `LTSpawnPoint::IsAvailable`, needs the
player camera).

### 2.5 No lunge or attack telegraph, and the direct-push fallback has no design home

**Ambiguity:** `TryAttack` (verified in the current `LTZombieCharacter.cpp`)
sets `AttackCooldown = AttackCooldownSeconds` and then calls
`UGameplayStatics::ApplyDamage` on the SAME tick the range check passes. No
wind-up, no animation gate, no lunge. There are `// DIAGNOSTIC` `LT_LOG` blocks
in `TryAttack` right now and an open PIE finding in NEXT.md ("zombie attack lands
nothing"), so the attack path is under active debugging and may change.
**Currently in code:** instant damage on `Distance <= AttackRange` (130) and
`AttackCooldown <= 0`. `OnHitReaction` and `OnDeathPresentation` are Blueprint
hooks; there is no `OnAttack` / `OnAttackWindUp` hook. Separately,
`DriveTowardsTarget()` has a **direct-push fallback**: when the AI controller is
missing (freshly spawned, possession not settled) or `MoveToActor` returns
`Failed`, the zombie calls `AddMovementInput` straight at the target. This moves
the zombie with no navmesh and no path following.
**Why it matters:** an instant, un-telegraphed melee at 130cm range is unreadable
and unfair with multiple zombies. And the direct-push fallback is an
undocumented movement mode with no design decision behind it.
**Suggested default (telegraph):** add an `OnAttackWindUp()` Blueprint hook. On
entering attack range with cooldown ready, the zombie plays a ~0.35s wind-up
(arm raise) during which it stops moving, THEN `ApplyDamage` lands if the player
is still in range, else the swing whiffs and a shorter cooldown (~1.0s) applies
(`AttackCooldownSeconds` 1.3s on a hit). Small forward lunge (`AddImpulse` ~200)
on the wind-up. Expose `AttackWindUpSeconds = 0.35`. Coordinate this with the
open attack bug: fix the bug first, then layer the wind-up.
**PROPOSAL - needs sign-off (the direct-push fallback needs a design home):**
decide explicitly what the fallback is allowed to be. Proposal: the direct push
is a **last-resort anti-freeze only**, capped at 2 seconds of continuous use per
zombie, at 60% `MaxWalkSpeed`, and it still routes through
`CharacterMovementComponent` so RVO separation still applies (it does today,
since `AddMovementInput` feeds the same movement component). A zombie that is
still pushing after 2s is handed to the unreachable check (1.4). A pushed zombie
plays the normal walk animation (it is still "walking", just off-mesh), so there
is no un-animated slide. If design wants pushed zombies to be visibly different
(a stagger-walk), that is an animation task, not a movement one. The key ruling
needed: **is a zombie that cannot path allowed to reach and damage the player by
push alone?** Proposed answer: yes, but only within that 2s window, so a brief
nav gap near the player does not make a zombie harmless.

### 2.6 Stagger tuning is a placeholder

**Ambiguity:** `ReceiveShot` does `Movement->AddImpulse(ShotDirection *
400.f, true)` with the comment "Placeholder until hit reaction montages land in
the art pass". brief-v2 lists a "stagger" animation. No spec for stagger
threshold (every shot? heavy hits only?), magnitude by weapon, or whether it
interrupts an attack.
**Currently in code:** flat 400 impulse on every non-lethal shot, no threshold.
**Why it matters:** 400 impulse on every pellet of a shotgun blast, or every
round of an SMG mag dump, turns the horde into a physics rag pile and trivialises
melee threat. It also currently does not interrupt `TryAttack`.
**Suggested default (changed threshold):** replace the flat 400 impulse with a
stagger budget, but set the threshold so sustained SMG fire does NOT stagger.
Each shot adds `Damage` (not `Damage * 0.5`) to a stagger meter that decays at
50/s; the walker threshold is **140** (about 4 hits from the coded SMG's 34
damage, or one shotgun blast, or one DMR/sniper round). Crossing it triggers a
0.5s stagger (no move, no attack, hit react, cancels a wind-up) and resets the
meter. Headshots add double. Brute threshold 400 (near stagger-immune to small
arms), crawler 40. Drop the raw `AddImpulse` entirely; keep only a tiny cosmetic
`Damage * 2` impulse capped at 120 for hit feel.
**Rationale:** the original's threshold of 40 with `Damage * 0.5` meant two SMG
rounds in 0.2s crossed it, so the "budget" barely changed the every-shot
stagger it was meant to fix and the horde stayed a physics toy. A threshold that
only heavy single hits or a sustained burst can reach keeps the horde a real
melee threat while still rewarding shotguns and marksman rifles with visible
knockback.

### 2.7 Concurrent zombie cap - folded into the single heat + cap model (5.1)

**Ambiguity:** brief-v2 says "live cap 40 plus 6 per heat level". C++
`MaximumAlive = 24`. The web-era number was 40. brief-v3 says "Target 60fps with
24 zombies". Three numbers, picked three different ways, none grounded in a
profile because there is no profile yet.
**Currently in code:** `MaximumAlive = 24` on `ALTRoundManager`, `EditDefaultsOnly`,
no heat wiring.
**Why it matters:** the concurrent cap is the single biggest feel and perf lever
and it drives the reference-frame "wall of zombies" read.
**Resolution:** this item is now subsumed by the single heat + concurrent-cap
model stated once in **5.1** and cross-referenced everywhere the cap is used
(1.5, 1.6, 3.2, 6.2 flood/narrow/open_concourse, section 17 perf budget, section
19 anti-kite). The short version:

- The base cap is a per-station `EditDefaultsOnly` property, derived from the
  frame budget in section 18, NOT guessed. Until section 18's profile exists it
  stays at the safe coded value **24** so nothing is tuned against a fantasy
  number. Do not ship 28.
- Heat adds a **single flat +N** to the cap (5.1 sets N and its ceiling from the
  perf headroom above the base, not an arbitrary 6).
- Heat's spawn-rate effect is a **single multiplier on the final spawn rate**
  (5.1), not a second compounding decay stacked on `SpawnIntervalDecay`.

Supersede the brief-v2 "40 plus 6" line. See 5.1 for the formula and the
worked round-20-heat-6 figure.

### 2.8 Corpse handling and count semantics

**Ambiguity:** `Die` calls `SetLifeSpan(CorpseLifetime)` (6s) and
`HandleZombieDied` removes the zombie from `LiveZombies` immediately on the death
broadcast. So a corpse lingers 6s but does not count against `MaximumAlive`.
Fine, but undocumented, and there is no cap on simultaneous corpses.
**Currently in code:** corpse lives 6s, freed from live count on death.
**Why it matters:** on a big round, dozens of corpses in 6s windows is a draw
call and physics cost the perf budget never accounted for.
**Suggested default:** keep 6s lifetime, add a corpse pool cap of 12: the oldest
corpse is destroyed early when a 13th dies. Corpses disable collision and
capsule (already done) and should also disable tick and detach the AI controller
(already unpossessed). Note this in the perf section.

### 2.9 No zombie audio spec (vocals, footsteps)

**Ambiguity:** brief-v3 Phase 8 lists "zombie vocals" with no detail.
free-assets.md says "Generate ... with TTS or a friend's voice" for
announcements but says nothing concrete for zombie vocals beyond "creatures" in
the Sonniss bundle.
**Currently in code:** no zombie audio at all.
**Why it matters:** in a dark station, audio is how you track the horde behind
you (Phase 8 accept: "tell what is happening behind you with your eyes closed").
**Suggested default:** per-zombie looping breath/groan bed (spatialised,
`Attenuation` ~15m), pitched by the per-zombie speed multiplier (2.2), 3 to 4
source variations from Sonniss creature stems, layered so a crowd is a wash not a
chorus of clones. A distinct one-shot on attack wind-up (a snarl). Sprinters get
a faster, higher, panting bed. Screamers get the special (3.x). Footsteps: a
single shared surface-typed footstep set, volume scaled down hard so 28 of them
do not drown everything.

---

## 3. The five zombie types

### 3.1 No per-type stat block exists

**Ambiguity:** brief-v2 names walker, sprinter, brute, crawler, screamer and
gives only "sprinter horde every 5th round, brute pair every 10th". schemas.ts
lists the same five names as a comment-level concept. There is NO per-type HP
multiplier, speed, damage, size, hitbox, special ability, spawn weight,
first-appearance round, or death behaviour anywhere. brief-v3 defers this to
Phase C ("Decide the approach first" per NEXT.md).
**Currently in code:** one `ALTZombieCharacter`, no subclasses, no data asset.
`BaseHealth = 150`, `BaseWalkSpeed = 130`, `AttackDamage = 24`,
`AttackCooldownSeconds = 1.3`, `AttackRange = 130`.
**Why it matters:** this is an entire subsystem with a name list and nothing
else. Every number below is currently a guess a Phase C session would invent
alone.
**Suggested default (mechanism):** a data-driven `ULTZombieData` `UPrimaryDataAsset`
(mirrors `ULTWeaponData`), one asset per type, with `ALTZombieCharacter` reading
it on spawn. This part is a clean confirmation.

**PROPOSAL - needs sign-off (the numbers):** the stat table below is invented.
The walker column IS the current C++ default (`BaseHealth 150`, `BaseWalkSpeed
130`, `AttackDamage 24`, `AttackCooldownSeconds 1.3`, `AttackRange 130`). Every
other row is new design and needs sign-off, not a tick.

| Type | HP mult | Speed (base) | Dmg | Cooldown | Scale | First round | Spawn weight (normal round) | Signature trait (pick ONE, not three) |
|---|---|---|---|---|---|---|---|---|
| Walker | 1.0 (150) | 130 | 24 | 1.3s | 1.0 | 1 | 100 | none |
| Sprinter | 0.55 (83) | **500** | 18 | 1.0s | 0.95 | 5 | 0 normal, 100 on sprinter rounds | genuinely fast: between player `WalkSpeed 420` and `SprintSpeed 640`, so it catches a walking or reloading player and is only lost by a clean sprint. Wind-up telegraph stays 0.35s. |
| Brute | 5.0 (750) | 95 | 45 | 2.2s | 1.5 | 10 | 0 normal, pair on x10 rounds | armour: a front plate that absorbs body shots until broken (~200 accumulated body damage), after which it takes normal damage. No knockback-on-hit, no blanket stagger immunity. Headshots and shots to the exposed back always land. |
| Crawler | 0.35 (52) | 150 (crawl) | 20 | 1.1s | 0.5 (prone) | 8 | 12 from round 8 | low profile: head only reachable by a crouched player (see 10.x crouch), body reachable standing. No death gas in the base design (that is 3.3, a separate PROPOSAL). |
| Screamer | 0.8 (120) | 110 | 10 | 1.5s | 1.0 | 12 | 6 from round 12, max 1 alive | on line-of-sight to the player for 2s, screams: summons one extra wave of 4 walkers, and the scream itself is loud and directional so it naturally masks other audio while it lasts (no submix trickery, no post-death deafen). Killing it fast is the counterplay. |

**Rationale for the sprinter change (490 to 520, settling on 500):** the original's
340 is slower than the player's 420 walk speed, so a 340 sprinter never catches a
mobile player and only threatens someone already cornered - a worse walker, not a
distinct threat. A round-based-survival sprinter's job is to punish a predictable
kite loop (section 19), which means it must beat a walking or reloading player.
500 does that and still loses to a committed sprint, so the counterplay is
"stop kiting lazily, actually run".
**Rationale for the brute change:** the original gave the brute three punishing
traits at once (750 to 900 HP sponge, 55 damage, knockback-on-hit, stagger
immunity under 120) with no stated counterplay. One signature trait (breakable
armour) with a clear weak point (the back, the head) is a fight the player can
learn.
**Rationale for dropping the screamer deafen:** brief-v3's Phase 8 accept
criterion is literally "tell what is happening behind you with your eyes closed".
A mechanic that removes the player's hearing removes the sense the entire audio
design exists to serve, and 3.5 already hedged that the audio system might not
support it. A loud scream that masks other sound while it happens is the same
threat without breaking the pillar.
**Worked effective-HP figures (the review's request):** `ApplyRoundScaling` sets
`Health = BaseHealth * 1.1^(round-1)`, so the type multiplier stacks on a
compounding base. Walker effective HP: round 10 ~354, round 15 ~570. Brute
(x5.0): round 10 ~1770, round 15 ~2850 - which against the coded SMG (34 dmg)
is ~52 body rounds at round 10, most of two mags, so the armour weak-point
counterplay is essential, not optional. These are the numbers Phase G balances.
Speeds scale with `SpeedStepRounds {5,10,20}` as now; all values `EditDefaultsOnly`.

**Line-of-sight test for the screamer (the review's missing item):** the trigger
is a line trace from the screamer's head to the player camera on
`ECC_Visibility`, blocked by geometry and by the flood water plane above 120cm,
NOT blocked by other zombies. It must hold clear for 2 continuous seconds. In a
blackout round the readability floor still lets the trace succeed (darkness is
not occlusion).

### 3.2 Special round composition undefined

**Ambiguity:** "sprinter horde every 5th round" - is the WHOLE round sprinters,
or a wave within it? "brute pair every 10th" - two brutes plus the normal round,
or instead of it?
**Currently in code:** nothing.
**Why it matters:** "round 5 is entirely sprinters" is a very different fight
from "round 5 has a sprinter wave then normal walkers".
**Suggested default:** every 5th round (5, 15, 25, ...): the entire round roster
is sprinters, count reduced to **75%** of the normal curve. Every 10th round
(10, 20, ...): normal walker round PLUS 2 brutes that spawn at 30% and 70% of
the way through the count. Round 20, 30 etc are both: sprinter round plus 2
brutes. Screamers and crawlers mix into normal rounds per their spawn weights,
never on special rounds.
**Rationale for 75% not 60%:** this is contingent on the sprinter speed being
fixed to 500 (3.1). With a proper anti-kite sprinter, a whole round of them at a
heavily reduced count could be EASIER than a walker round, inverting the intended
spike. 75% keeps the round a real threat. Tune the exact fraction in Phase G
once sprinter speed is locked - this number only makes sense after 3.1 resolves.

### 3.3 Death behaviours undefined per type

**Ambiguity:** `Die` plays `OnDeathPresentation(bHeadshot)` (Blueprint hook, no
impl) and sets a 6s lifespan. No type has a special death effect. The crawler
"gas cloud on death" in 3.1 is my invention; nothing in the docs.
**Currently in code:** uniform death, 6s corpse.
**Why it matters:** classic zombies uses on-death effects (crawler gas, etc) as a
positioning pressure. Without a spec every type dies identically.
**Suggested default (the plain part):** Walker/Sprinter: snap-to-settled-pose
death (no full ragdoll, per art 14.5), 6s corpse. Brute: falls forward, a small
screen shake if within 6m, 10s corpse (it is a landmark). Screamer: on death
mid-scream, the summoned wave is cancelled if the screamer is killed within the
first 0.5s of the scream, otherwise it still arrives.
**PROPOSAL - needs sign-off (crawler death gas):** the "3m radius sodium gas
cloud, 4 HP/s for 4s" is new design with nothing behind it in the docs. It is a
reasonable positioning-pressure idea but it interacts with the flood mechanic
(gas over waist-deep water?), the downed state (gas on a crawling player?) and
colourblind readability (a sodium cloud vs sodium lighting). If accepted, spec
those interactions. If not, the crawler's threat is just its low profile and
pack numbers, which is enough.

### 3.4 Sprinter / crawler navmesh and RVO behaviour

**Ambiguity:** the current RVO tuning (`AvoidanceConsiderationRadius = 45`) and
repath cadence (0.35s) were tuned for a slow walker. A 340-speed sprinter on a
0.35s repath overshoots its goal and the acceptance radius maths (`AttackRange *
0.5`) may park it wrong. Crawlers at capsule half-height need a different capsule.
**Currently in code:** one capsule size (`38, 90` inherited from Character
default, actually the zombie constructor does not resize it), one RVO tuning, one
repath cadence for all.
**Why it matters:** the B1 repath tuning already caused the "zombies park outside
AttackRange" bug once. Faster and shorter zombies will re-break it.
**Suggested default:** per-type overrides on `ULTZombieData`:
`RepathIntervalSeconds` (sprinter 0.2, crawler 0.4, others 0.35),
`CapsuleHalfHeight` (crawler 45, brute 130, others 90; the constructor does not
currently resize the capsule, so this is a new per-type step),
`AvoidanceConsiderationRadius` (brute 70, others 45), and `ContactRange`
(sprinter 25 not 15, to stop a 500-speed agent overshooting the acceptance point
on a 0.2s repath). Re-run the section 17 crowd frame check per type, not just for
the walker.
**Anims per type (the review's missing item):** each type needs its own
locomotion set - a brute at scale 1.5, a crawler prone at scale 0.5, and a
500-speed sprinter cannot share one walk cycle, and the crawler needs a
genuinely different rig pose. `free-assets.md` leans on Game Animation Sample +
Mixamo for the base and "the undead clips GASP does not have". This is a Phase
C/F animation blocker the type table does not cover; track it in section 17
(animation budget) and section 14 (VFX/anim inventory).
**Leash / de-spawn (the review's missing item):** if a zombie (especially a
sprinter) loses the player entirely - player rides the mezzanine, or boards -
it keeps pathing to the last known player location for 20s, then walks to the
nearest spawn point and de-spawns silently, decrementing `LiveZombies`. This
keeps `LiveZombies.Num() == 0` reachable so the round can end.

### 3.5 Screamer "deafen" and audio-muffle mechanic

**Ambiguity:** 3.1 proposes a screamer that muffles player audio. This touches
the audio system (Phase 8) which does not exist and has no spec for dynamic
submix effects.
**Currently in code:** no audio system.
**Why it matters:** a screamer whose only effect is "spawns more zombies" is just
a spawner; the audio-denial is what makes it a distinct threat in a
sound-dependent game. But it needs the audio system designed for it.
**Suggested default (changed):** drop the "deafen the player" effect entirely
(see 3.1 rationale - it breaks the "hear what is behind you" pillar). The
screamer's audio threat is that its scream is genuinely loud and directional, so
while it screams it masks other sounds the way any loud nearby sound does - a
natural mix consequence, no player-affecting submix, no low-pass, no ring. This
also removes the dependency on an audio feature that section 22 might not deliver
cheaply. The summon plus the masking-while-screaming is the whole mechanic; kill
it fast.

---

## 4. The train

### 4.1 No arrival / departure animation spec

**Ambiguity:** brief-v2: "real geometry arriving in the trackbed with headlights,
door animation and rumble". notes maps this to `ALTTrainActor` in Phase C. No
timing, no distance, no easing for the arrival slide, no departure.
**Currently in code:** nothing. No `ALTTrain` class exists.
**Why it matters:** the arrival is the game's signature beat. "It slides in" is
not enough to build.
**Suggested default:** the train enters from the far (vanishing-point) end of the
platform down the reserved trackbed. Headlights visible ~2s before the nose
enters frame, rumble bed ramps over the last 3s, brake-squeal one-shot at 0.5s
from stop. Doors begin opening 1.0s after full stop. Departure: doors close,
1.5s hold, slide out ease-in accelerating, rumble fades.
**Slide time as a function of visible train length (changed from a flat 4s):**
`ArrivalSlideSeconds` is derived, not fixed. For the grey box 2-to-3-car stub,
~4.0s ease-out is fine. The full train is a Class 345 "Aventra" 9-car at ~205 m
(4.11, RESOLVED), which is long, so the slide must show visible deceleration
over ~10 to 12s or it reads as a teleport, not a train stopping. Expose
`ArrivalSlideSeconds` and `DepartureSlideSeconds` as `EditDefaultsOnly` and set
them per the train length actually built, with a comment that 4s is a stub
value. `DoorHoldAfterArrivalSeconds = 1.0`.
**Train profile:** RESOLVED in 4.11: the Class 345 "Aventra" silhouette,
main-line loading gauge, walk-through, 9-car ~205 m. The platform is a
main-line-gauge box (a tall tiled hall), not a deep-tube bore.

### 4.2 Dwell vs door-open timing

**Ambiguity:** brief-v2: interval 100s, dwell 25s. Within the 25s dwell, when do
doors open and close? If the whole 25s is doors-open the arrival/departure slide
eats into boarding time or extends beyond the dwell.
**Currently in code:** nothing.
**Why it matters:** the boarding window is a hard tactical resource. "25s dwell"
could mean 25s of open doors or 25s door-to-door including slides.
**Suggested default:** `TRAIN_DWELL_MS` (25000) is measured stop-to-start (train
stationary time). Doors open 1.0s after stop, stay open until 3.0s before
departure, so the actual boarding window is ~21s. The 15s-before announcement
(4.4) fires 15s before the train STOPS, i.e. during the inbound slide. Interval
(100s) is measured departure-to-next-arrival-start, so trains are ~100s apart
plus dwell. Document all three reference points explicitly.

### 4.3 Player on the tracks

**Ambiguity:** the grid reserves the trackbed (1.1m below floor) for the train.
Nothing says what happens if the player jumps down there, or is there when a
train arrives.
**Currently in code:** nothing; grey box has no trackbed drop, it is flat.
**Why it matters:** players will jump down. An arriving train through the player
with no handling is a bug report; instant-kill with no warning is also bad.
**Suggested default:** the trackbed is a kill volume ONLY in the ~4s from the
arrival announcement to the train reaching full stop, and again during
departure: a "stand clear" klaxon plays, screen edges flash crimson, and contact
with the moving train is an instant down (not instant run-end; the 1.7 last-stand
still applies). Outside those windows the trackbed is just low ground the player
can climb back out of via a marked ramp at each platform end. A zombie in the
trackbed when the train arrives is destroyed silently.

### 4.4 Announcement timing and content

**Ambiguity:** brief-v2: "Announcement fires 15 seconds before arrival and on
arrival." art says the countdown lives on the departure board, not the HUD, and
"The T-minus-15-seconds announcement then does real work". No content spec, no
voice source decision, no "doors closing" announcement.
**Currently in code:** nothing. StationDef schema has `announcements: [string,
string, string]` (three lines per station).
**Why it matters:** the three-line announcement set per station is the entire
diegetic timer UX and one of the named legal risk areas (phrasing must not copy
TfL).
**Suggested default:** four announcement moments, drawing on the station's three
lines plus two global lines: (1) T-15s inbound: global line "The next service
will arrive shortly. This service terminates at [adjacent station names]." (2) on
stop: station line 1. (3) T-3s before doors close: global line "This train is
ready to depart. Please stand clear of the doors." (4) departure: station line 2.
Station line 3 is the idle/ambient line played once per breather. Voice: a single
synthesised TTS voice (system TTS or a pre-baked neutral synth), deliberately
flat and slightly artificial, no named announcer, phrasing written fresh per
`branding-precedent.md`. Record the exact global lines in brief-v2.

### 4.5 Boarding trigger volume and confirm

**Ambiguity:** brief-v2: "Boarding during dwell ends the round". notes: "you can
board it during dwell". phase-b2 lists "boarding the train" as a future
interactable. Is it a walk-in trigger, an interact prompt, a hold-to-confirm?
**Currently in code:** nothing. `ILTInteractableInterface` exists and is the
obvious mechanism.
**Why it matters:** an accidental board (walk into the volume while kiting) would
end a run against the player's will. A confirm adds friction in a 21s window.
**Suggested default:** boarding is an interact (`E`) on a door-aligned trigger,
available only while doors are open. Prompt: "Board train to [station]" or, if
travel target not yet chosen, "Board train". First press opens the
station-select (4.6) if there is a choice; on a single-adjacency station or after
selection, a second press within 3s commits and a 1.0s hold-to-board bar
prevents fat-fingering. Once committed the player is locked in (no un-board) and
the travel transition (4.7) starts.

### 4.6 Station-select UI

**Ambiguity:** brief-v2: "travels to a chosen adjacent station". schemas has
`adjacent: string[]`. Canary Wharf has "flood plus interchange" and in the brief
is adjacent to Whitechapel and Paddington (both in the 3-station set). No spec
for how the player chooses.
**Currently in code:** nothing.
**Why it matters:** this is the only branching-navigation UI in the game and it
is a modern-HUD-adjacent element the restrained style guide would push back on.
**Suggested default:** on boarding with >1 adjacency, time freezes (or slows to
0.2x) and a minimal diegetic panel appears: the original network schematic
(the HUD map substitute, art section 3) with the current station lit and
adjacent stations as selectable nodes showing name and tier. Left/right or mouse
to pick, `E` to confirm. No previews, no stats beyond tier. On a single-adjacency
station this is skipped. This panel is also the "map" the player can open any
time (see 11.x).

### 4.7 Travel transition

**Ambiguity:** brief-v2: "travels to a chosen adjacent station ... Travel is
bidirectional and free." No spec for the transition: cut to black? A ride? A
loading screen? Does the round counter carry (brief-v3 Phase 10 accept: "round
counter intact")?
**Currently in code:** nothing. `ALTRoundManager::CurrentRound` "never resets"
per its comment and brief-v2.
**Why it matters:** whether travel is a 2s cut or a 20s scripted ride changes
pacing enormously, and single-level-load vs streamed-station is an architecture
decision (brief-v3 says "one station done properly, then expand" and Phase 10 is
"second station").
**Suggested default (honest about the load, changed from the original):** travel
is a loading transition, not a seamless cut. Doors close, 0.5s fade to black,
then a **held black or a minimal loading state** (a single sodium line, no spinner,
no tips) while the destination `.umap` loads, then 1.0s fade in at that station's
platform with a train present and doors just closing. On a mid-spec machine a
full UE5 level load with Lumen is multiple seconds, not 1.5s, so do not pretend
the cut is instant.
**Architecture decision this forces:** either (a) separate `.umap` per station
with an honest short loading state as above (simplest, v1 recommendation), or
(b) World Partition / level streaming so the next station streams in behind the
black (no hitch, more engineering). Pick (a) for v1 and state it; (b) is a
post-v1 upgrade. This is a real architecture call, not a fade duration.
**State that persists:** round counter, points, weapons and their upgrade state,
perks, attachments, magazine and reserve, the self-revive budget - all via a
travel payload struct on the game instance (section 21). Heat resets to 0
(brief-v2). A 3s post-arrival grace before the destination round manager resumes
at `CurrentRound` (never round 1). No scripted ride in v1; a ride is a Phase F+
nicety.
**Reconciling brief-v3 Phase 10's "continue ... with its round counter intact":**
that accept criterion is in-memory travel, not resume-after-quit. Flag that its
wording should be tightened when it folds into gameplay-canon (16.1), because a
strict reading of "continue" implies a mid-run save that 13.3 explicitly rules
out for v1.

### 4.8 "Banks points" mechanically undefined

**Ambiguity:** brief-v2: boarding "banks points". brief-v3 Phase 6 mentions
"Oyster Credit: 1 per round survived, persistent". Is "banking points" the same
as Oyster Credit, a separate meta-currency, or just "you keep your points"?
**Currently in code:** `ULTPointsComponent::Points` is a single runtime int, no
persistence, no bank.
**Why it matters:** "bank" implies points are otherwise at risk (lost on death?
lost on staying?). That is a major economy rule currently absent.
**Suggested default (the clarification part):** points are NOT lost on death or
on staying. "Banks points" in brief-v2 is legacy wording; there is no
points-at-risk mechanic. Confirm this and drop the phrase from gameplay-canon.
**PROPOSAL - needs sign-off (a points-to-Credit conversion on board):** the idea
that boarding converts a slice of unspent points into permanent Oyster Credit
(e.g. 1 Credit per 500 points held at board, capped at 5 per board) is new
meta-economy design. It adds a second Credit earn path on top of brief-v2's "1
Credit per round survived" (7.6), each with its own cap, which is more
meta-economy than a round-loop clarification should lock, and it interacts with
13.4's "meta-progression is deliberately thin". If the owner wants an extra
incentive to leave, this is one shape; it needs a decision, not a default.

### 4.9 Ammo refill on board

**Ambiguity:** brief-v2: boarding "refills ammo". `ULTWeaponComponent::
RefillAmmunition()` sets `Reserve = MaxReserve` (full). Is that the intent, or a
partial top-up?
**Currently in code:** `RefillAmmunition()` = full reserve, magazine untouched.
**Why it matters:** a full free reserve every ~100s if you board is a strong
pull toward always leaving, which fights the heat incentive to stay.
**Suggested default:** boarding refills reserve to full (keep current behaviour)
but NOT the magazine, and does not refill equipment or grenades (Phase 6). The
full-reserve refill is a real reason to leave when dry; the heat/Oyster tension
is the counter-pull. State this explicitly in brief-v2.

### 4.10 Train interior and boarding destination

**Ambiguity:** when the player boards, are they inside a modelled carriage during
the transition, or does the screen just cut? rolling-stock.md has extensive
interior notes.
**Currently in code:** nothing.
**Why it matters:** decides whether the carriage interior is an asset that must
exist for Phase C or a Phase 7 art nicety.
**Suggested default:** Phase C: boarding is a cut (4.7), no interior needed, the
train is a solid shell with animated doors. Phase F: model one carriage interior
as the travel-transition backdrop (a 3s static shot through the window as the
station slides away) and as the visible interior through the open doors on the
platform. Not walkable in v1.

### 4.11 Rolling stock: which line - Elizabeth (Class 345) or Jubilee (1996 Stock)

**RESOLVED 2026-09-06 (owner decision).** The fictional line is a fictionalised
Crossrail-scale line: main-line loading gauge, modelled on the Elizabeth line,
NOT a deep-level tube. The train is the **Class 345 "Aventra" silhouette**:
walk-through with full open gangways, 9-car about 205 m, near-vertical box
sides, curved wraparound cab front, 3 double-leaf sliding plug doors per side
per car, floor 1.145 m above rail. The platform is a main-line loading-gauge
box (a tall tiled hall, near-vertical walls), NOT a cramped ~3.8 m bore. The
line name stays renamed in-world. This reverses the earlier "suggested default"
below (which leaned 1996 Stock deep-tube); that default is superseded. Off
limits and safe livery: see `docs/brief-v3-unreal.md` Part 1. Full physical
detail: `docs/reference/canary-wharf-research/rolling-stock.md`. The text below
is kept as the audit trail of the question.

**Ambiguity:** brief-v2 and art-direction call the line "a fictionalised
Elizabeth line". But `research/rolling-stock.md` describes BOTH the Jubilee line
1996 Stock (deep tube, ~2.63m body at waist, curved tumblehome profile, 126m
7-car, external sliding doors, **full-height platform screen doors at Canary
Wharf**, floor ~1.0 to 1.1m above rail) AND the Elizabeth line Class 345 Aventra
(main-line gauge, ~2.77m near-vertical sides, walk-through, 205m 9-car, plug
doors, platform screen doors in the central tunnel section only, floor 1.145m,
bright airy interior). These are different trains, different platform
arrangements and different boarding geometries. The research file itself
recommends the 1996 Stock profile as the primary target because "the arena grid
and reference frame are the low, tight, receding-corridor Jubilee look".
**Currently in code:** grey box train shell only; no `ALTTrain` class.
**Why it matters:** this decides the arena geometry - platform width, ceiling
height, whether there is an open trackbed to fall into (4.3) or a glass screen
wall, how the "train as a wall" reads (a curved deep-tube worm vs a tall box),
how boarding animates (one door set vs two synchronised sets, see 4.12), and the
whole reference-frame silhouette. It is currently a silent contradiction between
the stated fiction ("Elizabeth line") and the reference material (Jubilee-line
Canary Wharf). **Phase C blocker.**
**Suggested default:** commit to the **1996 Stock deep-tube profile** (the
Jubilee-line Canary Wharf), keep the in-world line name original (15.1), and
retcon the "fictionalised Elizabeth line" wording in brief-v2 and art to
"fictionalised deep-tube line" or just the chosen line name.
**Rationale:** the reference frame, the ASCII grid (1.1m trackbed drop, tight
corridor), the already-built grey box blockout, and `reference-frame-notes.md`
("Train as a full height wall down one side, platform receding to a vanishing
point") are all the low tight Jubilee read. The Class 345 is a bright
walk-through suburban EMU that does not match a single piece of the existing
visual target. Choosing the 345 would mean rebuilding the arena premise.
**Consequence for 4.3 (trackbed):** if 4.12 also adopts platform screen doors
(the Jubilee-at-Canary-Wharf arrangement), there is no open trackbed for the
player to fall into on the platform - see 4.12. If PSDs are dropped for
gameplay reasons, keep 4.3's kill-volume-during-arrival rule.
**Cross-reference:** also logged as a doc contradiction in 16.10.

### 4.12 Platform screen doors

**Ambiguity:** `research/rolling-stock.md` states Canary Wharf Jubilee has
"platform screen doors, full height, so from the platform you see the train
through glass screen doors when stopped, and a blank screen wall when it has
gone". No audit item, no design doc, and 4.1 to 4.10 all implicitly assume an
open LU-style platform with the train directly exposed.
**Currently in code:** nothing.
**Why it matters:** PSDs change section 4 substantially. "The train as a wall"
becomes a glass screen-door wall that is present even when no train is (a
different, arguably better, claustrophobic read). Boarding is through two door
sets (screen door + car door) that must animate in sync. A zombie cannot fall
into the trackbed from the platform (4.3) because the platform is sealed. The
arrival headlight beat (4.1) is seen through glass. Heat's crimson-creep and
blackout both interact with a lit glass wall.
**Suggested default:** adopt full-height platform screen doors (matches the
Jubilee-at-Canary-Wharf research and gives a stronger sealed-in feel).
Consequences to spec: (a) the screen-door wall is a permanent piece of level
geometry with its own emissive edge lighting; (b) boarding: the screen doors and
car doors open together 1.0s after the train stops and close together 3.0s
before departure, one combined `OnDoorsOpen` / `OnDoorsClosed` event; (c) the
platform side is fully sealed, so 4.3's trackbed kill volume is removed and the
only trackbed access is via the interchange/depot mechanics where a station is
NOT screened; (d) a zombie in the trackbed when the train arrives is still
destroyed silently, but zombies can only reach the trackbed on unscreened
stations. Expose `bHasPlatformScreenDoors` on `ULTStationData` so a future
unscreened station (depot, open_air) can turn it off.
**Rationale:** it removes an entire class of edge case (player and zombies on
the tracks) while making the arena read tighter, and it is faithful to the real
station the reference frame is of.

### 4.13 What the train does to spawns, nav and live zombies during dwell

**Ambiguity:** for the 25s the train is stopped it is a new solid object in the
arena. Nothing says whether spawns pause, whether the spawn interval and round
timer keep running, or whether live zombies re-path around the newly arrived
train.
**Currently in code:** nothing; no train.
**Why it matters:** if the train arrives mid-round and zombies do not re-path
around it, they walk into a wall; if spawns keep firing from track-side points
that the train now blocks, zombies spawn inside the train.
**Suggested default:** the round timer and spawn interval keep running during
dwell (staying is meant to be tense, not paused). Track-side spawn points are
automatically disabled while the train occupies the platform (a `bTrainPresent`
gate on `LTSpawnPoint::IsAvailable`, added alongside the existing `bEnabled`
and `FirstRound` gates). The stopped train is a dynamic nav obstacle: mark the
train actor to affect navigation only while stopped, so the navmesh carves
around it and live zombies re-path; when it departs, the navmesh restores. If a
dynamic nav rebuild on arrival/departure hitches (see section 17), fall back to
pre-baked nav-link toggling: the train volume is a permanently-carved nav hole
and track-side spawns are simply never used on a screened station.
**Multiple trains over a long run (the review's missing item):** a round-20 run
sees ~15 to 20 train cycles. The first ~3 arrivals get the full 4s slide +
headlights + brake squeal + doors spectacle; after that, degrade to a shorter
cycle (2s slide, no brake squeal, quicker doors) so a 30-second set piece does
not repeat every 100s. Expose a `bFullArrivalPresentation` that the train actor
sets false after the third arrival.

---

## 5. Station heat

### 5.1 THE heat + concurrent-cap model (single source of truth)

**Ambiguity:** brief-v2: "Staying raises station heat by 1, adding 6 to the live
cap and 12% to spawn rate; heat resets on travel." No ceiling, no "per what", no
other effects. The original audit then gave a different set of numbers in 2.7
(+4 to a ceiling of 44) and yet another in the old 5.1 (+4 and a compounding
`x0.90` interval on top of the coded `SpawnIntervalDecay`). Three inconsistent
models.
**Currently in code:** `MaximumAlive = 24` on `ALTRoundManager`, `EditDefaultsOnly`.
No heat component. Spawn interval is `1.6 * 0.96^(round-1)` floored at 0.35.
**Why it matters:** heat is the mechanic that makes staying a choice with
stakes, and the concurrent cap is the biggest feel/perf lever. They must be one
model, stated once, cross-referenced everywhere.

**This is the single model. Everywhere else in this doc that touches the cap or
heat (1.5, 1.6, 2.7, 3.2, 5.2, 5.3, 6.2, section 17, section 19) refers here.**

**Concurrent cap:**

```
concurrent_cap(round, heat) = BASE_CAP + HEAT_CAP_ADD * heat
```

- `BASE_CAP` is a per-station `EditDefaultsOnly` int on the round manager,
  **derived from the section 18 frame budget**, not guessed. Until that profile
  exists it stays at the coded **24**. Do not ship 28 or any other guess.
  Per-station because a narrow platform holds fewer than an open concourse
  (6.2: narrow ~18, open_concourse ~36 are provisional station overrides of
  BASE_CAP, also pending the profile).
- `HEAT_CAP_ADD` and the heat ceiling are set from the **perf headroom above
  `BASE_CAP`** that section 18 measures, NOT an arbitrary 6. Provisional until
  the profile: `HEAT_CAP_ADD = 4`, `HEAT_MAX = 5`, so the worst case is
  `24 + 20 = 44` concurrent. If section 18 says the machine can only hold 30,
  then `HEAT_CAP_ADD` and `HEAT_MAX` shrink so `BASE_CAP + HEAT_CAP_ADD *
  HEAT_MAX <= 30`. The cap is a hardware fact first, a design knob second.

**Spawn rate:**

```
effective_interval(round, heat) =
    max(MIN_INTERVAL, BASE_INTERVAL * ROUND_DECAY^(round-1) * HEAT_RATE_MULT^heat)
```

- `BASE_INTERVAL = 1.6`, `ROUND_DECAY = 0.96`, `MIN_INTERVAL = 0.35` - all
  unchanged from code.
- `HEAT_RATE_MULT = 0.90` per heat level, ONE multiplier applied to the final
  rate. There is no second compounding decay. At `HEAT_MAX = 5` this is
  `0.9^5 ~= 0.59x` the round's interval.
- Worked figure: round 20, heat 5: `1.6 * 0.96^19 * 0.9^5 ~= 1.6 * 0.460 *
  0.590 ~= 0.43s`, comfortably above the 0.35 floor. This is the number Phase G
  sanity-checks against a real profile.

**Heat accrual:** heat is an int on a `ULTStationHeatComponent` on the round
manager. It increments by 1 each time a train departs without the player aboard
(so it ticks roughly once per 100s, which drifts from the round number as rounds
lengthen - see 5.x on that drift). Resets to 0 on travel. Persists through a
down and self-revive (you survived the station, heat keeps climbing).

**Other heat effects:** from heat 3+, the special-type spawn weights (sprinter,
screamer, crawler) roughly double, so a high-heat normal round mixes in more
threats. No other mechanical effects; heat's job is pressure, not new rules.

**Supersede** brief-v2's "+6 cap, +12%" line and both of the original audit's
conflicting versions.

### 5.2 Heat visual and audio indication

**Ambiguity:** phase-b3 explicitly says the heat indicator is "Phase C or later".
art says nothing about how heat reads. Is it a HUD number, a station state, a
lighting shift?
**Currently in code:** nothing.
**Why it matters:** if the player cannot perceive heat rising, the stay/go choice
has no readable feedback loop.
**Suggested default (changed):** heat reads on the **station**, never on the
board. Each heat level: the emergency (crimson) lighting creeps further up the
platform from the tunnel ends, the ambient bed gains a layer (a rising drone,
see section 22 for the audio signature), station emissives flicker more, litter
and staining near the horde spread. Do NOT glitch or shorten the departure
board's service list: the board is the one instrument the player must trust to
make the stay/go decision, and degrading it is self-defeating. A single small
HUD element is allowed: a segmented sodium bar near the round number (max
`HEAT_MAX` segments, i.e. 5 provisionally), filling as heat rises. No number.
For colourblind players the crimson creep must be paired with the audio drone
and the segmented bar so heat is not a crimson-only signal (section 23).

### 5.3 Strategic pull to stay vs go

**Ambiguity:** the docs assert a tension exists but never lay out the payoff on
each side. Boarding refills ammo (4.9), resets heat, converts points to Oyster
Credit (4.8), changes mechanic/arena. Staying raises pressure and... gives what?
**Currently in code:** nothing.
**Why it matters:** if staying has no upside beyond "not loading a new level",
every optimal player always boards and heat is dead content.
**Suggested default:** staying's upside: (a) per-station "best stand" stat and
Oyster Credit earn both scale with the heat level reached, so high-heat survival
is the scoring and progression play; (b) the Lost Property office gets a small
better-roll bias per heat level (a heat-3+ pull skews toward higher-tier
weapons), a concrete reason to farm heat before rolling; (c) some wall buys get
a small discount per heat level. Boarding's upside is survival, a full ammo
reserve (4.9), a fresh arena, and heat reset.
**Caveat the review raised, folded in:** "points-per-minute is higher at high
heat" is NOT reliably true. Past the concurrent cap (5.1), more heat adds
pressure without adding kill throughput, so points-per-minute can plateau while
danger keeps rising - which would make staying strictly worse. So do not sell
staying on raw points income. Sell it on the scoring stat, the Credit scaling,
and the Lost Property bias, all of which keep climbing with heat regardless of
the cap. Write this trade table into gameplay-canon (16.1).

---

## 6. Stations and the mechanic library

### 6.1 Station count contradiction (3 vs 41 vs "one then expand")

**Ambiguity:** brief-v2 says "Exactly THREE hand-authored stations" (Canary
Wharf, Whitechapel, Paddington) for Phase 5, then Phase 6 adds "the remaining 38
stations" (41 total). brief-v3 says "One station done properly, then expand ...
Two good stations beats forty grey ones" and its Phase 10 is "second station".
tasks/README Phase G is "second station". So the real target is 2, the brief-v2
target is 3, and the aspirational is 41.
**Currently in code:** zero stations. `L_GreyboxTest` (combat testbed) and
`L_CanaryWharf_Greybox` (blockout) exist as maps.
**Why it matters:** every downstream estimate (art budget, StationDef authoring,
adjacency graph, the network schematic UI) depends on N.
**Suggested default:** commit to **2 stations for v1**: Canary Wharf (flood +
interchange) and one second station. Design the systems (StationDef, travel,
schematic UI, heat, mechanic library) to scale to N but author 2. Paddington and
the other 38 become a documented post-launch expansion path.
**Second station - changed from the original:** do NOT pick `blackout` as the
second mechanic. LAST TRAIN's named risk is "fight what you cannot see" (art
section 5); a dark station taught right after a dark-ish flooded one teaches the
player the game has one register. A second station that contrasts **tonally**
teaches range. Two options, owner to pick:
- **Whitechapel** with `[platform_split, open_concourse]` - two platforms and a
  wide bright concourse, run-and-gun, the footbridge as the pinch. Real East
  London station, plausibly one hop from Canary Wharf.
- **Paddington** with `[open_concourse, interchange]` - already has brief-v2
  backing, wide and bright, reuses the interchange system Canary Wharf already
  needs so it is cheaper to build.
Hold `blackout` for a station 3. Update gameplay-canon to say "2 stations,
expansion list retained".
**Blocker the review raised:** whichever station 2 is, it has no grid, no
research folder (only Canary Wharf has one), and no blockout. That is real Phase
D/G work that this decision creates; size it when the station is chosen.

### 6.2 The 10 mechanics have zero mechanical definition

**Ambiguity:** schemas.ts and brief-v2 name flood, blackout, narrow,
escalator_rush, platform_split, open_concourse, depot, open_air, interchange,
terminus. brief-v2 gives a one-line arena description for the 3 reference
stations' mechanics only (flood "water rises each round shrinking the arena",
blackout "every third round kills all emissives", open_concourse "wide, low
cover, high spawn count"). The other 7 have nothing. legend.ts assigns cost
multipliers to water (3), barrier (4), escalator (1.4) tiles but that is grid
cost, not mechanic behaviour.
**Currently in code:** nothing. No mechanic system.
**Why it matters:** a mechanic is meant to be "no new engine code beyond these"
(brief-v2) yet each is undefined, so a Phase C/10 session cannot implement even
the two Canary Wharf needs (flood, interchange) without inventing the rules.
**Suggested default:** define the **4 mechanics v1 actually needs** as bounded,
data-parameterised behaviours. The other 6 (narrow, escalator_rush, depot,
open_air, terminus, and open_concourse if station 2 is not it) are
**documentation for the expansion, not v1 scope** - half of them need a
different modular kit (daylight, street stairs, weather) that v1 will not have,
so they are defined-but-dead until then. Keep their one-line sketches below for
the expansion doc.

v1 mechanics:

- **flood (changed from the original):** a water plane over the `~` tiles rises
  `RisePerRound` cm each round (default **4cm**, not 12), to a ceiling of
  **90cm**, not 150. Effect on player: >40cm slows move to 70% and disables
  sprint; >70cm adds a heavy handling and accuracy penalty (sway up, bloom up,
  ADS slower) but **does NOT block firing**. Effect on zombies: water tiles cost
  3x (legend), crawlers are removed above 60cm depth. Resets to 0 on travel.
  Below 40cm it is cosmetic.
  **Rationale:** the original's 12cm/round to a 150cm ceiling meant waist-deep
  and "blocks fire" by round ~10 on the one tier-4 station - that is not a
  shrinking arena, it is the game switching off its core verb on a timer. A
  slow rise to a lower ceiling that penalises movement and accuracy without
  disabling the gun keeps flood a pressure on positioning, which is its job.
  Also note (review): the `~` block in the grid is a fixed rectangle (rows ~17
  to 22), so "shrinking arena" is really "two lanes get progressively wetter and
  slower", and the grey box may already be built without the nav cost weighting
  this depends on - verify before relying on it.
- **interchange (changed from the original):** a mezzanine reachable by the
  escalator banks, with its own spawn points that activate from a set round
  (default 6). Escalators give the player a **modest** boost in the rush
  direction (1.4x, not 2x) and zombies use them at near-parity (1.2x), so the
  escalator is a **vertical loop the player runs**, not a one-way valve the
  player rides up to a safe perch while the horde bottlenecks at the foot. See
  section 19 - a 2x conveyor the player cannot fight against is exactly the
  training exploit this section is meant to design out.
- **blackout:** every `BlackoutInterval` rounds (default 3), for that whole
  round, station lighting (art layer 2) drops to nothing and the readability
  floor (layer 1) to ~20% (art section 5). Torch/headlamp (14.6) and muzzle
  flash only. Zombie spawn rate +15% during a blackout round (a single-round
  multiplier, separate from heat). Held for station 3, not v1's second station.
- **platform_split:** two platforms joined only by a footbridge. Spawns on both;
  the player must pick a side and the bridge is the pinch point. The bridge can
  optionally be a purchasable debris clear.

Expansion-only sketches (not v1):

- **narrow:** a single tight corridor, no fallback space, one spawn direction,
  `BASE_CAP` set low (~18, a per-station override of 5.1). A pure hold-the-line
  station.
- **escalator_rush:** escalators run fast in one direction and zombies pour down
  them as the primary spawn route; the mechanic is that the down-escalator is a
  conveyor delivering the horde, and riding up against it is slow.
- **platform_split:** two platforms joined only by a footbridge (brief-v2
  Whitechapel). Spawns on both; the player must pick a side and the bridge is the
  pinch point. Optionally the bridge is a purchasable debris clear.
- **open_concourse:** wide, low cover, `BASE_CAP` high (~36, per-station
  override), best wall buys, more spawn points active from round 1. A run-and-gun
  station. (Promoted to v1 IF it is station 2's second mechanic, per 6.1.)
- **escalator_rush:** escalators run fast in one direction and zombies pour down
  them as the primary spawn route; riding up against it is slow.
- **depot:** an above-ground/surface siding with train shells as cover, multiple
  short platforms, spawns from tunnel mouths and from between parked trains.
  Needs the surface kit.
- **open_air:** a surface station, directional light enabled, rain affecting
  visibility, spawns from street-level stairs. Needs the surface kit and weather.
- **terminus:** end of a line, one tunnel mouth only, a boss (`bossId`) at a set
  round, boarding here is the only exit and travel is one-way back. The
  "end of line" screen shows here.

Each becomes a `ULTStationMechanicConfig` with its 1 or 2 numeric params. Phase
C implements **flood + interchange** (Canary Wharf's two); blackout and
platform_split land with station 2 in Phase D/G.

### 6.3 Which mechanic pairs with which station

**Ambiguity:** brief-v2 assigns mechanics only to the 3 reference stations. Phase
6's "38 stations" instruction says "one or two mechanics from the fixed library"
but gives no per-station assignment, only "outer suburban are tier 1 to 2,
central are tier 3 to 4. Termini get a boss id."
**Currently in code:** debug-yard.ts uses `['flood', 'interchange']` (same as
Canary Wharf) as a test.
**Why it matters:** for a 2-station v1 this is small, but the second station's
mechanic pair must be chosen now.
**Suggested default:** Canary Wharf = `[flood, interchange]` (fixed by brief-v2
and the grid). Whitechapel (station 2) = `[blackout, platform_split]` (brief-v2).
If a third is ever added, Paddington = `[open_concourse, interchange]`
(brief-v2). Defer the other 38 assignments to the expansion doc.

### 6.4 StationDef schema is TypeScript, not UE

**Ambiguity:** schemas.ts defines a rich `StationDef` (displayName, lineIndex,
adjacent, tier, mechanics, grid, seed, accent, hdri, exposure, fogDensity,
fogColour, wetness, announcements[3], ambient{}, bossId, wallbuys{}, perks{},
debrisCosts{}). brief-v3 demotes the grid to "a layout sketch". There is no UE
equivalent data asset. The C++ has `ULTWeaponData` but no `ULTStationData`.
**Currently in code:** no station data asset. Round manager is "station agnostic"
(finds spawn points by world iteration). Stations are just `.umap` files.
**Why it matters:** travel, heat, the schematic UI, per-station round rosters,
per-station announcements and ambient all need a station identity object. The
web schema captured design intent that has not been ported.
**Suggested default:** create `ULTStationData` (`UPrimaryDataAsset`) carrying the
still-relevant fields: `DisplayName`, `Tier` (1 to 4), `AdjacentStations`
(soft refs to other `ULTStationData`), `Mechanics` (1 or 2
`ULTStationMechanicConfig`), `AccentColour`, `AnnouncementLines` (3x `FText`),
`AmbientParams` (a struct: hum, drip, wind, rumbleDistance), `BossClass`
(optional). Drop the web-only rendering fields (hdri file, exposure, fogDensity,
fogColour, wetness, seed) since Lumen + a per-level post process volume replace
them. Each `.umap` has one `ALTStationInfo` actor holding a ref to its
`ULTStationData`. Document the field-by-field port.

### 6.5 Adjacency graph and line identity

**Ambiguity:** schemas has `adjacent` and `lineIndex` and a `validateRegistry`
that checks reciprocity. brief-v2 references the "fictionalised Elizabeth line".
brief-v3 says "Rename the line in-world." No line name is chosen anywhere. For 2
stations the graph is trivial (A <-> B) but the schematic UI (4.6) needs
something to draw.
**Currently in code:** nothing.
**Why it matters:** the network schematic is both the map UI and (per art) an
original-artwork legal substitute for the Tube diagram. It needs a name and a
shape.
**Suggested default:** name the in-world line. Proposal: **"the Dock Line"** or
**"Meridian line"** (Canary Wharf/Greenwich meridian association, evokes an East
London route without being "Elizabeth" or "Jubilee"). Pick one, record in
brief-v2 and art-direction. For v1 the schematic is a 2-node stub with room to
extend. Station geography stays factual (Canary Wharf, Whitechapel are real and
on real routes); only the line's name and its diagram styling are original.

### 6.6 Grid normalisation issues are real

**Ambiguity:** the NeoStack run (neostack-run-2026-09-05.md) found the Canary
Wharf grid in `canary-wharf-grid.md` is "not a strict 74 wide rectangle, rows
ranged 72 to 76 characters", row 9 is prose text not tiles, `~` rows are 18 to 22
not the documented 17 to 22, the `X` run is row 14 not 13, and bottom spawn
clusters are single tiles not clusters. `validateStation` in schemas.ts would
reject a ragged grid outright.
**Currently in code:** the grey box was built by normalising the grid ad hoc
(trim/pad to 74, treat prose row as `=`).
**Why it matters:** the grid is meant to be the shared vocabulary. If it does not
parse cleanly, every future use re-guesses the fixes differently.
**Suggested default:** correct `canary-wharf-grid.md` in place: pad/trim every
row to exactly 74 chars, replace the prose "TRAIN VOLUME" row with a real `=`
row, fix the row-number references in the prose to match the tile data. This is a
one-time cleanup, not a design decision, but it must happen before the grid is
used again. (Out of scope for this audit doc, flagged as a task.)

### 6.7 Anchor-to-content mapping (wallbuys/perks/debrisCosts)

**Ambiguity:** schemas has `wallbuys: Record<string,string>`, `perks:
Record<string,PerkId>`, `debrisCosts: Record<string,number>` keyed by "anchor
label ... assigned in grid reading order". debug-yard leaves all three `{}`. The
grid has `W`, `P`, `L`, `U`, `D` anchor tiles but no doc says which weapon is on
which `W`, which perk on which `P`, or what each `D` costs.
**Currently in code:** `ALTWallBuy` has a `Weapon` ref set per-instance in the
editor. No perk machine class, no debris door class.
**Why it matters:** Canary Wharf grey box placed 4 `W`, 2 `P`, 1 `L`, 1 `U`, 6
`D` anchor cubes with no assigned content. Someone has to decide what each is.
**Suggested default:** for Canary Wharf, the 4 wall buys are: **buy the SMG**
(platform, 500pts - the SMG is no longer a starting weapon per 8.2, so one `W`
anchor is where you buy it, not "refill a gun you have"), a pump shotgun
(platform, 1000pts), a service rifle (concourse, 1500pts), a light machine gun
(far mezzanine, 2500pts). The 2 perk anchors: one on the platform, one in the
concourse.
**Perk split caveat (the review's point):** "2 perks here, 2 at station 2" as a
travel incentive hard-couples the perk economy to the 2-station count. With a
3-of-4 cap (9.2), you get 2 here, must travel for a 3rd, and never get the 4th.
That is a fine v1 shape but it breaks if station 3 ever ships - flag the
coupling in gameplay-canon so a future expansion re-splits deliberately.
Lost Property (`L`): the concourse, 950pts base. Upgrade bench (`U`): the far
concourse corner, 5000pts. The 6 debris doors: tunnel-approach doors 750pts each
(early containment), concourse-entry doors 1250pts each (open the fallback
space). Record as the Canary Wharf `ULTStationData` content maps.

### 6.8 Terminus / boss content

**Ambiguity:** schemas has `bossId?: string`. brief-v2 Phase 6: "Termini get a
boss id." No boss is designed anywhere: no boss type, HP, mechanic, arena, music,
reward.
**Currently in code:** nothing. Round manager has no boss concept.
**Why it matters:** for a 2-station v1 with neither being a terminus, this can be
deferred, but the schema field implies it is planned.
**Suggested default:** no boss in v1 (neither station is a terminus). Keep
`BossClass` on `ULTStationData` as an optional hook, unused.
**PROPOSAL - needs sign-off (the Inspector, if a boss is ever wanted):** a
single boss concept for the expansion, not v1: "the Inspector", an oversized
armoured figure that walks the platform on a fixed patrol, immune to body shots,
weak to sustained headfire, spawns at a set round on terminus stations, drops a
free Upgrade Bench use on death. This is invented, not implied by anything; it
is here so a future terminus has a starting point, and it needs its own design
pass (HP, arena, music, reward) when that station is built.

---

## 7. Economy and pricing

### 7.1 Point award values are unbacked

**Ambiguity:** `ULTPointsComponent`: `StartingPoints = 500`, `PointsPerHit = 10`,
`PointsPerKill = 60`, `PointsPerHeadshotKill = 130`. brief-v2 lists exactly these
numbers ("500 start, 10 hit, 60 kill, 130 headshot") so they ARE the design, but
no doc explains the ratios or whether they hold for the UE build's pacing.
**Currently in code:** as above. `AwardHit` on every pellet hit that does not
kill; `AwardKill(bHeadshot)` from the zombie death broadcast.
**Why it matters:** these drive the whole purchase economy. Note the shotgun
edge: `AwardHit` fires per pellet, so a point-blank shotgun kill can pay
`hit*pellets + kill` which is much more than a rifle kill.
**Suggested default (confirmation part):** confirm 500 / 10 / 60 / 130 as
intended and record them as design values in gameplay-canon (not placeholders).
**Code change, flagged as such (not a doc confirmation):** `TracePellet`
currently calls `Points->AwardHit()` once per connecting pellet (verified:
`if (Points && !Zombie->IsDead())` inside the pellet loop). This means an
8-pellet point-blank shotgun hit pays `8 x 10 + 60 = 140` where a rifle body
kill pays `10 + 60 = 70`. Change: award `PointsPerHit` **once per shot** if any
pellet connected, not per pellet. But do not just delete the shotgun's income -
model its intended points-per-shot deliberately: a shotgun that pays the same
as a rifle per trigger pull is under-rewarded for being a close-range risk
weapon. Proposal: shotguns award `PointsPerHit * 2` per connecting shot (still
far below per-pellet), tuned in Phase G. This is a `ULTWeaponData` flag
(`bAwardHitPerShot` + a multiplier), not a global rule.
**Assist award:** add a 20pt assist if a zombie you damaged is killed by another
source (`Die` currently only pays the `Killer`). Small `ULTZombieCharacter`
change to track last-damager-that-was-not-the-killer.

### 7.2 Wall buy prices

**Ambiguity:** `ALTWallBuy`: `WeaponCost = 500`, `AmmunitionCost = 250` as class
defaults. `ULTWeaponData` also has `WallPrice = 500` and `AmmoPrice = 250`. The
test map (neostack-build.md) uses 500/250. brief-v2 Phase 6 says "Assign min
station tiers and wall prices" but assigns none. Two sources of truth (the
weapon data and the wall buy actor) for the same price.
**Currently in code:** `ALTWallBuy` reads its own `WeaponCost`/`AmmunitionCost`,
NOT the weapon data's `WallPrice`/`AmmoPrice`. So the data asset's price fields
are currently dead.
**Why it matters:** price is per-weapon design and there is a wiring bug in
waiting (two price fields, one unused).
**Suggested default:** make `ALTWallBuy` read `Weapon->WallPrice` and
`Weapon->AmmoPrice` unless a per-instance override is set (add a
`bOverridePrice` bool). Set per-weapon prices on the data assets: starting SMG
n/a (you start with it), pump shotgun 1000, service rifle 1500, LMG 2500, sniper
2000, per 8.2. Ammo price = ~25% of weapon price, rounded to 50. Record in the
weapon roster table.

### 7.3 Perk, mystery box, bench, door prices

**Ambiguity:** brief-v2 Phase 6: "Lost property office: random gun, base 950,
+10% per use, resets on travel. Upgrade bench: 5000 points". So mystery box (950,
+10%/use) and bench (5000) ARE specified. Perk prices are NOT ("The four perks:
..." with no cost). Debris door costs are NOT (`debrisCosts` is a per-station map
with no values given).
**Currently in code:** no perk machine, no lost property, no bench, no debris
door classes exist.
**Why it matters:** perks are the biggest single spend category and have no
price.
**Suggested default (the specified prices):** Lost Property 950 base, `*1.1` per
use, min 950 on travel-reset (keep brief-v2). Upgrade Bench 5000 (keep brief-v2).
Debris doors 750 to 1500 by role (see 6.7). All `EditDefaultsOnly`, overridable
per station.
**PROPOSAL - needs sign-off (perk prices):** perk prices are not in any doc, and
they cannot be set sensibly before the perk effects are locked. Second Wind in
particular is contingent on the stamina decision (10.2, itself a PROPOSAL) and
9.1 has it doing three things at once. Provisional starting numbers, to be
revisited after 10.2 and 9.1 resolve: Commuter's Constitution 2500, Quick Hands
2000, Double Tap 2000, Second Wind 3000 (or higher if it keeps its three
effects - a perk doing regen + revive budget + sprint for less than a wall LMG's
2500 is underpriced). Ordering note: resolve 9.1 and 10.2 first, then price.

### 7.4 Points cap

**Ambiguity:** `AddPoints` does `Points = FMath::Max(0, Points + Amount)` with no
upper bound. No doc mentions a cap. Classic zombies caps at 1,000,000.
**Currently in code:** no upper cap, int32 (overflow at ~2.1 billion, not a real
concern).
**Why it matters:** minor, but a stated cap prevents weird HUD layout at huge
numbers and signals "points are spent, not hoarded".
**Suggested default (changed):** no points cap. A round-15 run does not approach
one, and a farming run that would hit it is better served by the number just
continuing. The HUD weapon/points block is laid out for 6 digits; if a 7th digit
ever appears it wraps or scales, which is a HUD layout detail, not an economy
rule. Drop the item. (If a real anti-cheese or leaderboard-integrity reason for
a cap emerges later, set it then, for that reason, with that number.)

### 7.5 Downed / revive economy

**Ambiguity:** brief-v3 Phase 6: "downed and revive". brief-v2 Phase 6 does not
mention it. Solo self-revive (my 1.7 proposal) has no cost. Classic zombies:
Quick Revive perk costs points solo.
**Currently in code:** nothing.
**Why it matters:** if self-revive is free and unlimited the last-stand has no
teeth.
**Suggested default:** the first self-revive per round is free (the "survive to
breather or kill 4" from 1.7). A second down in the same round can only be
recovered by a perk: "Second Wind" (already in the roster) additionally grants
one extra self-revive per round and speeds the bleed-out kill requirement (4 to
2). No points cost for revive itself; the cost is the perk slot.

### 7.6 Oyster Credit earn and spend detail

**Ambiguity:** brief-v2 Phase 6: "Oyster Credit: 1 per round survived,
persistent, spends on attachment unlocks." brief-v3 carries it forward. 4.8
proposes an additional earn on boarding. No spec for: total earn rate balance,
what an attachment unlock costs in Credit, whether Credit is per-attachment or a
pool, whether it is ever lost.
**Currently in code:** nothing.
**Why it matters:** this is the only meta-progression currency and its rate sets
how many runs to unlock the attachment tree.
**Suggested default (the confirmed part):** 1 Credit per round survived (keep
brief-v2). Never lost. Credit is a single pool, not per-attachment.
**PROPOSAL - needs sign-off (the unlock costs and the grind length):** what an
attachment unlock costs in Credit, and therefore how many runs to unlock the
whole tree, is a retention-design decision with nothing behind it in the docs,
and it interacts with 13.4's "meta-progression is deliberately thin". A
provisional table (optic 3, extended mag 5, barrel 8 per weapon, ~16 per weapon,
~160 for everything, ~15 to 20 runs) is one shape. The owner should decide
whether v1 even has a 15-to-20-run grind or something much shorter (a handful of
runs), and whether the 4.8 board-conversion earn path exists at all. Do not
data-enter the cost table until this is decided.

---

## 8. Weapons and the gunsmith

### 8.1 Only one weapon exists, all defaults

**Ambiguity:** one `DA_Weapon_SMG` ("Stag Compact") at every `ULTWeaponData`
class default (`BaseDamage 34`, `RPM 600`, `MagazineSize 30`, `MaxReserve 240`,
`HipSpread 3.4`, `AimedSpread 0.5`, etc). brief-v2 Phase 5 wants "a service
pistol and a pump shotgun" as proof guns; Phase 6 wants "the remaining 10 guns".
brief-v3 says the spread model "transfers directly as design" from the web build
but no roster, no per-weapon numbers.
**Currently in code:** `ULTWeaponData` defaults only. No pistol, no shotgun, no
roster.
**Why it matters:** the weapon roster is the core of the mid-game economy and
"round 12 to 15" balance. It is entirely unspecified.
**Suggested default (roster SHAPE only):** lock only the roster shape - how many
weapons, which archetypes, the tier spread, and which need code beyond the
existing hitscan model. Do NOT lock the per-weapon numbers.
**PROPOSAL - needs sign-off / DO NOT DATA-ENTER (the stat table):** every number
in the table below is invented. Presenting ~130 specific values as a "suggested
default" invites a bulk data-entry pass that enshrines fantasy stats as data
assets before a single weapon is playtested. Treat the table as a starting-shape
sketch. All numbers are deferred to a Phase G tuning doc; the balance pass
(section 20) sets them against the effective-HP curve (3.1) and the horde
density, neither of which is known yet. In particular the "Terminus" bolt sniper
at 260 damage / `Pen 5` is a horde-deleter in a corridor and almost certainly
breaks the mid-game economy - it is a flag, not a value.

**Code scope, corrected:** there is NO projectile system - the weapon is pure
hitscan (verified). The crossbow and the joke weapon are not "small code
additions"; a projectile weapon is a new movement/collision/impact subsystem.
Either cut them from v1 or scope the projectile system explicitly as Phase C/E
work. Also: `MuzzleFlash` on `ULTWeaponData` is typed `UParticleSystem` (legacy
Cascade), which is mismatched with the Niagara FX plan in free-assets - flag for
a type change to a Niagara system ref (section 14).

Starting-shape sketch (numbers provisional, unlocked):

| Weapon | Class | Dmg | RPM | Mag | Reserve | Hip/ADS spread | Pellets | Pen | Tier | Wall price |
|---|---|---|---|---|---|---|---|---|---|---|
| Stag Compact | starting SMG | 34 | 600 | 30 | 240 | 3.4 / 0.5 | 1 | 1 | 1 | n/a |
| (pistol) "Warden" | sidearm | 40 | 300 | 12 | 96 | 2.6 / 0.3 | 1 | 1 | 1 | 500 |
| (pump shotgun) "Docker" | shotgun | 22 x8 | 70 | 6 | 48 | 9.0 / 6.0 | 8 | 2 | 1 | 1000 |
| (machine pistol) "Sprite" | machine pistol | 26 | 900 | 24 | 210 | 4.2 / 1.2 | 1 | 1 | 2 | 900 |
| (SMG 2) "Relay" | SMG | 32 | 700 | 34 | 300 | 3.6 / 0.6 | 1 | 1 | 2 | 1200 |
| (bullpup) "Verge" | bullpup rifle | 38 | 620 | 30 | 270 | 2.8 / 0.4 | 1 | 2 | 3 | 1500 |
| (assault rifle) "Junction" | AR | 42 | 560 | 30 | 300 | 2.6 / 0.35 | 1 | 2 | 3 | 1500 |
| (semi shotgun) "Breaker" | semi-auto shotgun | 20 x7 | 160 | 8 | 56 | 8.0 / 5.0 | 7 | 2 | 3 | 1750 |
| (LMG) "Gantry" | LMG | 46 | 650 | 75 | 375 | 4.0 / 1.5 | 1 | 3 | 4 | 2500 |
| (DMR) "Meridian" | DMR | 90 | 260 | 15 | 150 | 1.4 / 0.1 | 1 | 3 | 4 | 2000 |
| (bolt sniper) "Terminus" | bolt sniper | 260 | 45 | 6 | 60 | 1.0 / 0.0 | 1 | 5 | 4 | 2000 |
| (crossbow) "Latch" | crossbow (projectile, special) | 200 + explosive | 30 | 1 | 20 | 1.0 / 0.0 | 1 | 3 | 4 | 2500 |
| (joke) "Oyster Reader" | original joke weapon | fires ticket stubs, 5 dmg, RPM 1200, knocks back hard | | 50 | 500 | | | | 3 | mystery box only |

**Tone check on the joke weapon (the review's point):** "Oyster Reader that
fires ticket stubs" is close to the CoD-furniture whimsy the brief says to avoid
("deliberately NOT Call of Duty furniture", "restrained tone"). If a joke weapon
exists at all in v1 it needs to fit the register - a malfunctioning
crowd-control tool, a repurposed maintenance device - not a gag gun. Owner call
on whether v1 has one; default is to cut it and the crossbow, leaving a
pure-hitscan roster with no projectile system needed.

### 8.2 Starting loadout

**Ambiguity:** brief-v3 Phase 7 (web era) mentions a "pre-run loadout picker".
brief-v3 (UE) Phase 9 lists "pre-run loadout picker" too. No spec for what you
pick from, or whether v1 has a fixed start.
**Currently in code:** `BP_PlayerCharacter` has `DA_Weapon_SMG` assigned to the
Weapon component. So the game starts you with the SMG, full mag and reserve, and
nothing else.
**Why it matters:** classic zombies starts you with a weak pistol and knife by
design (early rounds are a points-farming teach). Starting with a 30-round SMG
changes round 1 to 5 pacing.
**PROPOSAL - needs sign-off:** starting with a weak pistol instead of the SMG is
a real design pivot (the classic-zombies opening), not a clarification. It
changes round 1 to 5 pacing, it invalidates 6.7's wall-buy plan (one `W` anchor
becomes "buy the SMG"), and it is a change to `BP_PlayerCharacter`, which
currently starts with `DA_Weapon_SMG` (verified). Proposal: v1 has a fixed
start, the "Warden" pistol only, no loadout picker; the SMG is a 500pt platform
wall buy. A loadout picker with 2 to 3 choices (each with a points penalty for
the stronger start) is a post-v1 addition. If the owner prefers to keep the SMG
start, that is also coherent - it just means the early game is less of a
points-farming teach. Ripple: 6.7, the grey box wall-buy placement.

### 8.3 No melee

**Ambiguity:** classic zombies has a knife (instant-kill early rounds, the
economic backbone of round 1 to 3). brief-v2/v3 never mention melee. Player has
no melee input, no melee code.
**Currently in code:** no melee anywhere.
**Why it matters:** without melee, ammo economy in early rounds is much tighter
and the "shoot to soften, knife to kill for full points" loop is gone. Also
affects 1.7 (downed can only fire).
**PROPOSAL - needs sign-off:** adding melee is new design. brief-v2/v3 never
mention it and there is no melee code. It is a sound call - round-based survival
assumes a knife-equivalent as the round 1 to 3 economic backbone - but it is a
new mechanic, not an ambiguity. Proposal: a fixed input (`V`), ~1.4m range, 0.8s
cooldown, awards `PointsPerKill` on a melee kill, as a small `ULTMelee`
component. On the damage curve, commit melee as an **explicitly early-game
tool**: instant-kill to round 5, then it **scales** as `150 * 1.1^(round-5)` so
it stays a viable finisher (not dead weight by round 8, which a flat 150 would
be against the compounding HP curve), capped so it never one-shots a mid-round
walker. Or, simpler, keep it flat-150 and accept it as a round 1 to 6 tool only,
stated as such. Owner picks which. Also feeds 1.7 (a downed player could melee).

### 8.4 Attachments / gunsmith undefined

**Ambiguity:** brief-v3 mentions "gunsmith, attachment swapping only during a
breather". brief-v2 Phase 6: "Three functional attachment slots per gun (optic,
magazine, barrel) with real stat effects, plus round-threshold unlock
requirements." So the slot names (optic/magazine/barrel) and the swap-timing rule
exist. The actual stat effects, the specific attachments per slot, and the unlock
round thresholds do NOT.
**Currently in code:** `ULTWeaponData` has no attachment fields at all. No
gunsmith UI, no attachment data.
**Why it matters:** attachments are the Oyster Credit sink (7.6) and a
progression layer. Currently zero design.
**Suggested default:** each weapon has 3 slots. Generic attachments (not
per-weapon, to keep it small):

- **Optic:** Iron (default), Reflex (faster ADS, small aim-spread reduction),
  Scope (large zoom, DMR/sniper only, tighter aim spread, slower ADS).
- **Magazine:** Standard (default), Extended (+50% mag, -10% reload speed),
  Fast (-30% reload time, no capacity change).
- **Barrel:** Standard (default), Long (less damage falloff, tighter hip
  spread, slower ADS), Compact (faster move while ADS, more falloff).

Stat effects as flat multipliers on the existing `ULTWeaponData` fields, applied
at `SetWeapon` from an equipped-attachment struct. Unlock thresholds: each
attachment unlocks by reaching a round with that weapon equipped (Reflex round 5,
Extended round 8, Long barrel round 12, etc), then is permanently available and
costs Oyster Credit (7.6) to add to the gunsmith pool. Swapping only in a
breather or on the train (brief-v3). Add the full table to brief-v2 Phase 6.

### 8.5 Upgrade bench (Pack-a-Punch equivalent) ruleset

**Ambiguity:** brief-v2 Phase 6: "Upgrade bench: 5000 points, double damage, +50%
mag, renamed variant, one gun at a time." `ULTWeaponData` has `UpgradedName`.
That is the whole spec. No: can you re-upgrade for more? does it add an effect
(fire, explosive)? does it persist through travel and death? can both your
weapons be upgraded (one-at-a-time = the bench processes one, or you can only
ever have one upgraded)?
**Currently in code:** `UpgradedName` field exists, unused. No bench class.
**Why it matters:** the bench is the win-condition goal for a farming player and
its rules are half-stated.
**Suggested default:** 5000 points, applies to the currently-held weapon:
`BaseDamage *2`, `MagazineSize *1.5` (round up), `MaxReserve *1.5`, name becomes
`UpgradedName`, and a small cosmetic effect (sodium tracer). "One gun at a time"
means only one of your carried weapons may be upgraded; taking a new wall buy
into an upgraded slot loses the upgrade. Upgrade persists through travel and
through death (you keep it on respawn/next-run? no: through travel yes, through
run-end no). No re-upgrade tier in v1. Add to brief-v2.

### 8.6 Ammo economy and pickups

**Ambiguity:** zombies do not drop ammo (`Die` awards points only). Reserve is
refilled by wall buy `RefillAmmunition` (full) or board (full). No max-ammo
drop, no per-round ammo. brief never addresses ammo scarcity as a design lever.
**Currently in code:** ammo comes only from wall buys and (later) boarding. No
drops.
**Why it matters:** with no ammo drops and expensive wall-buy ammo, a player can
be stranded with a dry primary and only points to solve it, which is the intended
tension OR a frustration depending on tuning.
**Suggested default:** no random ammo drops (keep the wall-buy dependency, it is
the economy). Add ONE relief valve: every 5th round, a "Max Ammunition" pickup
spawns at a random anchor for 20s, refilling all carried weapons' reserves free.
This is the classic power-up cadence, restrained to just this one. No other
power-ups (no instakill, no nuke, no double points) in v1, matching the "no CoD
furniture" tone.

### 8.7 Weapon switching and carry limit (UNIMPLEMENTED FEATURE, not a bug)

**Ambiguity:** `ULTWeaponComponent` holds one `WeaponData`. `SetWeapon` replaces
it wholesale (verified). There is no second slot, no switch input, no carry
limit.
**Currently in code:** one weapon, replaced on wall buy. No switch.
**Recategorised (was called an economy bug):** this is NOT a bug. The current
code is a Phase A/B combat slice that only ever had one weapon; a 2-weapon carry
is an unimplemented feature request, scoped Phase C/E work. Calling it "a serious
economy bug that DELETES your SMG" overstates it - there is no regression, the
feature was never built. Buying a second wall weapon before the carry system
exists just swaps your gun, which is the expected behaviour of a one-slot
component.
**Why it matters:** the mid-game economy assumes you build a loadout of two
guns; that system has to be built for wall buys past the first to make sense.
**Suggested default:** carry 2 weapons. Add a `SecondaryWeaponData` slot and a
switch input (`Q` or mouse wheel, ~0.4s swap). Wall buy fills the active slot;
if both slots are full and both differ from the wall weapon, buying prompts
"replace [active weapon]". "Quick Hands" speeds swap by 40% (matches brief-v2).
`ULTWeaponComponent` change, Phase C/E scope. Until then, one slot is the
correct behaviour, not a defect.

---

## 9. Perks, bench, lost property, revive

### 9.1 Perk list is incomplete and effects are fuzzy

**Ambiguity:** brief-v2 Phase 6 lists four: "Commuter's Constitution +75 max HP,
Quick Hands 40% faster reload and swap, Second Wind infinite sprint and faster
regen, Double Tap +30% fire rate and +15% damage." schemas.ts PerkId enum:
`constitution | quick_hands | second_wind | double_tap`. So there are exactly 4
and they have rough effects. Gaps: "faster regen" (how much faster?), "infinite
sprint" (does it also remove the stamina system that does not exist?), whether
Constitution's +75 is +75 flat or to a new max of 175, interaction with the
downed state.
**Currently in code:** no perk system, no stamina system. Player `MaxHealth =
100`, `RegenerationPerSecond = 20`, `RegenerationDelaySeconds = 4`.
**Why it matters:** perks are a major spend and a major power spike. Fuzzy
effects mean inconsistent implementation.
**Suggested default:** exactly 4 perks (no expansion in v1). Precise effects:

- **Commuter's Constitution:** `MaxHealth` 100 to 175 (heals the +75 on
  purchase) AND `RegenerationPerSecond` 20 to 35, so the bigger pool does not
  heal slower in real terms (a Juggernog-style perk speeds regen too, not just
  raises the ceiling; the original left regen flat, which is a subtle nerf on a
  175 pool).
- **Quick Hands:** reload time `*0.6`, weapon swap time `*0.6`, interact/buy
  time `*0.6`.
- **Second Wind:** its sprint effect is undefined until the stamina question
  (10.2, a PROPOSAL) is resolved - if stamina is added, Second Wind removes the
  drain; if not, the sprint clause is dead and the perk is regen + revive only.
  Do not spec the sprint clause here. Regen clause: `RegenerationDelaySeconds`
  4 to 2, `RegenerationPerSecond` 20 to 32. Revive clause: per 9.3 (an extra
  self-revive and a faster disengage window). Note: a perk doing three strong
  things needs its price (7.3) set accordingly.
- **Double Tap:** `RoundsPerMinute *1.3`, `BaseDamage *1.15` (faithful port of
  brief-v2's "+30% fire rate, +15% damage"). Caveat: with `BloomPerShotDegrees`
  unchanged, +30% RPM means the gun blooms to `BloomMaxDegrees` faster and is
  LESS accurate in sustained fire, so Double Tap can be a downgrade for a spray
  weapon. Phase G should check whether Double Tap also needs a small bloom
  reduction to be a clean upgrade.

### 9.2 Perk cap and loss rules

**Ambiguity:** brief-v2 Phase 6: "Max three equipped, lost on death not on
travel." So the cap (3) and loss-on-death exist. Gaps: with only 4 perks and a
cap of 3, you can nearly get all; is that intended? "Lost on death" - all of
them, on the run-ending death? On a down? Do you re-buy at the same station or
does the machine move?
**Currently in code:** nothing.
**Why it matters:** "lost on death" needs to define which death (down vs
run-end) and the re-acquisition cost.
**Suggested default:** cap 3 of 4 (keep brief-v2). You keep perks through
travel, through a down-and-self-revive, and lose ALL of them only on a run-ending
death (which in v1 = the run is over anyway, so "lost on death" mostly matters if
a continue/checkpoint is ever added; keep the rule as written for that future).
Perk machines are per-station and fixed; the 4 perks are split 2-and-2 across the
2 stations (6.7) so you must travel to get more than 2, and re-buying a perk you
already have is a no-op refund-free.

### 9.3 Revive timing and mechanic

**Ambiguity:** brief-v3 Phase 6: "downed and revive". Solo, revive-by-teammate is
impossible. 1.7 proposes self-revive. No doc states bleed-out time, revive
health, or movement while downed.
**Currently in code:** `bDead` blocks all input, `OnDied()` empty hook. No
crawl state, no bleed-out.
**Why it matters:** the entire "downed" concept in a solo game needs defining
from scratch; brief-v3 just names it.
**PROPOSAL - needs sign-off:** as 1.7 (which is also a PROPOSAL). On 0 HP, enter
`bDowned` (crawl 30% speed, sidearm only, no jump/sprint, camera at ~30cm,
free-look retained, ADS force-released). 25s bleed-out shown as a closing
crimson vignette (paired with a low-health audio bed for colourblind
readability, section 23). **Revive by disengagement:** more than 12m from every
live zombie for 6 continuous seconds restores 40 HP. Once per round free; a
second down that round, or the bleed-out expiring, is run-ending and triggers
`OnDied` and the end-of-run screen. Second Wind grants one extra self-revive per
round and shortens the disengage window (6s to 4s) and distance (12m to 9m).
**Rationale (changed from "kill 4 while downed"):** see 1.7 - a crawling
pistol-only player almost never kills 4 while a horde closes, so the free revive
was usually unreachable and every down was effectively run-ending. A disengage
condition rewards the correct instinct and a crawl speed can actually achieve
it.
**Bleed-out length:** 25s is a starting number; Phase G tunes it against how
long a clean disengage actually takes under round-15 pressure.

### 9.4 Lost Property (mystery box) weapon pool and rules

**Ambiguity:** brief-v2 Phase 6: "Lost property office: random gun, base 950,
+10% per use, resets on travel." Gaps: which weapons are in the pool (all 10? a
subset? station-tier-gated?), can you get a weapon you already have, is there a
"fire sale" / box-move mechanic, does it give the joke weapon, is there a reroll.
**Currently in code:** nothing.
**Why it matters:** the box is a gamble that can hand you a round-30 weapon on
round 4 or waste 950 on a downgrade. Pool composition is the whole feel.
**Suggested default (changed - add soft tier weighting):** the pool is all
weapons except your two starting-tier ones. You CAN roll a weapon you hold (it
gives a full-ammo refill instead, a mercy). The weapon appears for 12s to be
taken with `E`; the cost only charges on a successful take. No box move, no fire
sale, no reroll in v1. Price 950 `*1.1` per use, resets to 950 on travel (keep
brief-v2).
**Soft tier weighting by current round (changed from "no tier gating"):** a pure
no-gating pool lets a round-4 player pull the top-tier sniper and trivialise
rounds 4 to 15, which LAST TRAIN's "round 12 to 15 is a serious achievement"
target does not survive. Weight the pool so tier-4 weapons are very unlikely
before round ~10 and common after ~15, tier-3 unlikely before ~6. It is still a
gamble (you might get a tier-3 on round 6), just not a run-warping one. Classic
zombies accepts the unbounded pull; this game's difficulty contract does not.
**Joke weapon / crossbow in the pool:** only if 8.1 keeps them at all (default
is to cut both). If kept, low weight.

### 9.5 Equipment / grenades

**Ambiguity:** brief-v3 Phase 6 accept mentions "equipment" in passing ("Perks,
the weapon upgrade bench, the lost property office ..., downed and revive,
equipment"). No equipment is designed anywhere: no grenade, no tactical, no
input, no data.
**Currently in code:** nothing.
**Why it matters:** "equipment" is listed as a Phase 6 deliverable with zero
spec.
**PROPOSAL - needs sign-off:** brief-v3 names "equipment" once and nothing else.
Whether v1 has any, and what it is, is a design decision. Proposal: one piece, a
"signal flare" (input `G`), sticks where it lands and draws all zombies within
~15m to it for 4s (a re-position tool, not damage), 2 carried, refilled at the
Max Ammunition pickup (8.6) and on the train. It fits the transit fiction and
the restrained tone better than a frag. If the owner wants a lethal option
instead, or none at all for v1, that is a valid answer - this is not implied by
any doc. No tactical equipment tree in v1 regardless.

---

## 10. Player mechanics

### 10.1 No fall damage

**Ambiguity:** nothing mentions fall damage. `ALTPlayerCharacter` has
`JumpZVelocity = 420`, `AirControl = 0.25`, and `TakeDamage` only fires from
zombie `ApplyDamage`. The interchange mechanic (6.2) has a mezzanine you could
fall off.
**Currently in code:** no fall damage.
**Why it matters:** with a mezzanine and escalator banks, players will drop off
edges; no fall damage makes the vertical layout consequence-free (which may be
fine) but it is an unstated choice.
**PROPOSAL - needs sign-off:** fall damage is not in any doc. The real question
is whether the mezzanine drop (6.2 interchange) is a **deliberate escape route**
or a **risky one**. Two coherent answers, owner picks:
- No fall damage. The mezzanine drop is a clean escape valve, part of the
  vertical kite loop by design. Simplest, and consistent with an arcade feel.
- A small flat tax: 15 HP for any drop over ~4m, no scaling. Enough to make
  spamming the drop a real cost, cheap enough that it is not a coin-flip.
Do NOT use the original's "8 HP/metre capped at 60" - 60 damage is 60% of base
health from one drop, which makes an escape route a gamble and reads as unfair
(random-feeling) in a kiting game. If fall damage exists at all it should be
predictable and small. Interaction to spec if adopted: no fall damage while
downed (a crawling player rolling off an edge should not die).

### 10.2 Sprint stamina contradiction

**Ambiguity:** brief-v2 Fable prompt Phase 4: "Shift sprint with stamina". The
web build had stamina. brief-v3 and the C++ have NO stamina: `StartSprint` just
sets `bSprinting = true` and sprint is unlimited. "Second Wind" perk grants
"infinite sprint", which only makes sense if base sprint is limited.
**Currently in code:** unlimited sprint, no stamina meter, no stamina anywhere.
**Why it matters:** unlimited sprint in a kiting game trivialises the horde (you
outrun it forever). The perk that removes a limit is pointless if there is no
limit.
**PROPOSAL - needs sign-off:** adding a stamina system is inventing a core
mechanic the game does not have. brief-v3 Part 1 lists sprint with no stamina;
the only pro-stamina source is a stale brief-v2 Fable prompt for the discarded
web build. The genuine problem - unlimited sprint lets a player outrun the horde
forever - is real, but stamina is only ONE fix. Others: a genuinely fast
sprinter (3.1, now 500, which is the primary anti-kite answer), tighter arenas,
and sprint already cancelling aim/fire (coded). A stamina meter plus a regen
delay plus a low-stamina vignette plus a Second Wind interaction is a large new
system, and it deserves its own top-level decision, not an "answer" buried in a
player-mechanics item.
**If stamina IS wanted**, a starting shape: 6s sprint from full, drains while
sprinting, refills at 4/s after a 1s delay, cannot start under 1s of stamina, a
thin desaturating vignette at low stamina, Second Wind removes the drain. Add
`SprintStaminaSeconds`, `StaminaRegenPerSecond`, `StaminaRegenDelaySeconds`.
**If stamina is NOT wanted**, then Second Wind's sprint clause is dead (9.1) and
the anti-kite job falls entirely on the sprinter (3.1, section 19) and arena
design, which is a defensible position. Either way, reconcile brief-v2 and
brief-v3 to say the same thing.

### 10.3 Interact range mismatch

**Ambiguity:** `ULTInteractionComponent::InteractionRange = 250` (2.5m).
`ALTZombieCharacter::AttackRange = 130` (1.3m). So you can interact with a wall
buy from further than a zombie can hit you, which is fine, but no doc states the
intended interact range or whether it should require facing precisely.
**Currently in code:** 250cm sphere sweep, radius 12cm, on `ECC_Visibility`.
**Why it matters:** minor, but 2.5m interact through the crosshair with a 12cm
sweep can feel either generous or finicky depending on anchor size, and the
sweep on `ECC_Visibility` means any collision geometry between you and the anchor
blocks it.
**Suggested default:** keep 250cm and 12cm radius. Add a facing check: the anchor
must be within ~20 degrees of the view centre (the sweep already enforces this
loosely). Confirm anchors get a dedicated collision profile so the sweep is
reliable. Good enough; no change needed beyond documenting it.

### 10.4 Revive-others / co-op

**Ambiguity:** the audit brief asks about "revive-others". brief-v2/v3 describe a
solo game throughout ("a competent player", singular). Nothing mentions co-op.
**Currently in code:** `GetPlayerPawn(this, 0)` everywhere, single-player
assumptions throughout.
**Why it matters:** if co-op is ever intended, the target-selection, revive, and
round-scaling code all need it designed in now; if not, say so.
**Suggested default:** v1 is single-player only. State it explicitly in brief-v3.
Co-op is not an expansion path without significant rework (zombie targeting, HUD,
travel, revive, scaling all assume one player). Do not design for it.

### 10.5 Movement while downed

**Ambiguity:** covered by 1.7/9.3 proposals but the docs say nothing.
`bDead` currently blocks `Move` entirely.
**Currently in code:** downed = no movement (because `bDead` blocks `Move`).
**Why it matters:** a downed player who cannot move at all is just watching a
timer; the crawl gives agency.
**Suggested default:** downed player crawls at 30% `WalkSpeed`, camera lowered to
~30cm, no jump, no sprint, can turn and fire the pistol. Implement as a
`bDowned` state separate from `bDead`, with `bDead` reserved for the true
run-end. (This is the same as 1.7; recorded here for the player-mechanics
section.)

### 10.6 Starting health, regen values unbacked

**Ambiguity:** `MaxHealth = 100`, `RegenerationDelaySeconds = 4`,
`RegenerationPerSecond = 20`. brief-v2 Fable prompt Phase 4 states exactly "100
HP, regen begins 4 seconds after last damage at 20 HP per second" so these ARE
design. But `AttackDamage = 24` means ~4 hits kill, and with regen at 20/s you
recover a hit in ~1.2s after the 4s delay: a fast zombie hitting every 1.3s
(`AttackCooldownSeconds`) out-damages regen and kills in ~5 hits / ~6.5s.
**Currently in code:** as above.
**Why it matters:** the health/regen/zombie-damage triangle IS the core survival
difficulty and it is currently only implicitly tuned.
**Suggested default:** keep 100 / 4s / 20/s as the design baseline (they are in
the brief). Note the derived combat maths: 4 to 5 walker hits kill, regen fully
heals in ~5s of no contact past the 4s delay. This means "never take more than 3
hits in a row" is the skill expression. Tune `AttackDamage` per zombie type
(3.1) rather than the player health. Record the triangle in brief-v2 so Phase 11
balances it as a unit.

---

## 11. HUD and UI

### 11.1 HUD scope: what is in for v1 vs deferred

**Ambiguity:** phase-b3 built a barebones 4-corner HUD (round, name, points,
health bar, weapon block, crosshair, hit marker, interact prompt, damage
vignette) and explicitly defers "Perk icons, equipment slots, the challenge
tracker, the special weapon meter, the station schematic, the train countdown,
the exfil prompt and the heat indicator ... to Phase C or later". brief-v3 Phase
9 lists a fuller set including "perks" and "the station network schematic as the
map substitute". So there are two HUD specs (minimal now, fuller Phase 9) and the
boundary between them is only loosely drawn.
**Currently in code / the real state of the health bar (rewritten):** the
`WBP_HUD` EventGraph wiring for the health bar IS present and correct - recent
editor testing confirms `OnHealthChanged` is bound to `HandleHealthChanged`,
which calls `SetPercent` on the bar. The player character does broadcast
`OnHealthChanged(GetHealthFraction())` on `BeginPlay`, on regen and on
`TakeDamage` (verified in `LTPlayerCharacter.cpp`). The bug is a **layout** one:
the `HealthTrackBox` SizeBox has `bOverride_WidthOverride = False`, so it renders
at zero width and the bar is invisible regardless of the percent value. Fix:
enable the width override (or remove the SizeBox and let the bar fill its slot).
This is a NeoStack/UMG one-line fix, not a wiring rebuild. (The earlier NEXT.md
note "HealthBar.Percent has zero bindings" was from before the EventGraph route
was traced; the value flows through `HandleHealthChanged`, not a direct property
binding.)
**Why it matters:** Phase C adds train, heat, perks, all of which want HUD
representation, and the "no reserved empty slots" rule means each is added
exactly when its system lands.
**Suggested default:** the v1 HUD (through Phase G) is: round number (TL), player
name + points + health bar (BL), weapon block mag/reserve/name (BR), crosshair +
hit marker (centre), interact prompt (bottom centre), damage vignette (post
process). ADD in Phase C: a perk row (BL, above name, only appears once you own
1+ perk, one flat sodium glyph per perk), the heat bar (TL, next to round, 5.2),
and NOTHING for the train (diegetic on the board per art). ADD in Phase E:
equipment count (BR, below weapon, only if you carry the flare). Layout rule:
fixed anchor positions per element, elements FADE in place when their system
lands, nothing reflows the HUD mid-combat. Never add: minimap, kill feed,
challenge tracker, XP, exfil banner. Record this final list in gameplay-canon,
superseding the web-era HUD list.
**CommonUI constraint (from NEXT.md):** `CommonUI` is a forced uproject
dependency (removing it SIGSEGV'd PIE). Any HUD work must live with it present.

### 11.2 Train countdown is diegetic-only, confirmed but incomplete

**Ambiguity:** art is emphatic: "Do not put the train countdown in the HUD. Put
it on the board, on the platform displays, and in the announcements." But if the
player is deep in the concourse or on the mezzanine, they cannot see the board.
Is there a secondary indicator? An audio cue only?
**Currently in code:** nothing (no board, no train).
**Why it matters:** a purely diegetic timer the player often cannot see makes the
boarding decision a guess unless the audio does all the work.
**Suggested default:** keep it diegetic. Coverage: the departure board is
repeated (per signage-and-wayfinding.md, next-train indicators repeat down a long
platform and at decision points), so place one at each escalator top and in the
concourse. The T-15s and doors-closing announcements (4.4) are the audio
backstop and play station-wide. When the train is in the station (doors open),
a subtle sodium glow spills from the platform into adjacent areas. No HUD
element. This is a deliberate friction: to catch a train you must be near the
platform, which is the whole risk.

### 11.3 No pause menu / in-game menu spec

**Ambiguity:** brief-v3 Phase 9 lists "pause" in the menus list. `IA_Jump` etc
exist but there is no pause input, no pause menu. brief-v2 Fable prompt: "Escape
pause".
**Currently in code:** no pause input, no pause menu.
**Why it matters:** a shooter with no pause is a bug. Also the settings screen
(11.4) is usually reached from pause.
**Suggested default (changed):** `Esc` pauses (true pause, `SetGamePaused`),
showing: Resume, Settings, Restart Run, Quit to Menu. No "save and quit" (v1 has
no mid-run save). **Pause is always available**, including during the pre-round
beat, the breather, train windows and the boarding hold. Do NOT disable pause to
prevent "cheese": in a single-player game with no mid-run save and no
leaderboard (13.x), pausing to gain an advantage is a non-issue, and disabling
pause is a legitimacy and accessibility problem (a player who needs to stop
cannot). If run integrity ever matters (a leaderboard ships), handle it then,
specifically. Add `IA_Pause`.

### 11.4 Settings screen contents

**Ambiguity:** brief-v3 Phase 9 (web era) had "A settings screen exposing every
post-processing toggle and the quality preset". brief-v3 (UE) says master
brightness must be a setting (art section 5: "Expose master brightness as a
setting"). No other settings enumerated for the UE build.
**Currently in code:** nothing.
**Why it matters:** the brightness setting is explicitly required by art
direction (a dark game with a "fight what you cannot see" risk).
**Suggested default:** v1 settings: Master Brightness (the required one, a gamma
slider with a calibration image), Master / SFX / Music / Voice volume, FOV slider
(90 to 110, default 95 per `BaseFieldOfView`), mouse sensitivity, ADS
sensitivity multiplier, invert Y, sprint toggle/hold, aim toggle/hold, a
"reduce camera shake" accessibility option, and the graphics preset (Low/Med/
High, mapped to Lumen quality, shadow res, VSM). Subtitles for announcements
(on by default given the diegetic-audio dependence). Persist to a settings save.

### 11.5 Crosshair spread drive detail

**Ambiguity:** phase-b3: "Drive its spread from `GetCurrentSpreadDegrees()` so it
opens on movement and recoil bloom". `GetCurrentSpreadDegrees` returns
base-lerp + movement + bloom in degrees. No mapping from degrees to screen
pixels, no clamp.
**Currently in code:** the value is exposed; the widget mapping is a NeoStack/UMG
task not yet verified.
**Why it matters:** a linear degrees-to-pixels map makes a 9-degree shotgun
crosshair huge and a 0.5-degree ADS one invisible, which may or may not be
wanted.
**Suggested default:** map spread degrees to crosshair gap with a gentle curve:
`gap_px = 6 + 14 * sqrt(spread_deg / 5)`, clamped 6 to 40px at 1080p, scaled by
resolution. At full `GetAimAlpha()` the four lines collapse to a 2px dot
regardless. Record the formula in phase-b3 / the HUD doc.

### 11.6 No objective / onboarding text

**Ambiguity:** a new player dropped into a dark platform with a pistol has no
idea the train is an escape, that heat exists, or what wall buys do. The
restrained HUD has no tutorial layer and no doc mentions onboarding.
**Currently in code:** nothing.
**Why it matters:** "the first ten minutes feel coherent" (brief-v3 success test)
is hard if the player does not understand the core loop.
**Suggested default:** minimal diegetic onboarding: the pre-round announcement
(1.1) says the train is not in service and to leave the platform (establishing
the train matters). The first time the player looks at a wall buy, the interact
prompt is slightly longer for 5s ("Hold E to buy [weapon] - [price] points").
The first train's T-15s announcement adds "This service will accept passengers"
once, ever. No tutorial pop-ups, no forced sequence. A one-screen "how to play"
on the main menu covers the rest.

---

## 12. Audio and music

This section is the original audit's audio coverage (music, voice source, ambient
params, weapon audio, occlusion). It is superseded and extended by **section 22
(Audio architecture)**, which adds the submix/bus layout, ducking, reverb model,
the train-audio spec, low-health audio, the sting inventory, the announcement
line content and its legal-pass rule, per-type vocalisation, the heat audio
signature, UI sound and the audio performance budget. Read 12 and 22 together;
where they overlap, 22 is the fuller version.

### 12.1 Is there music at all

**Ambiguity:** brief-v3 Phase 8: "Ambience, train hum, original announcements,
zombie vocals, weapon audio, interaction, round stingers. ... Silence is a tool."
"Round stingers" is the only musical element named, and "silence is a tool"
suggests very little music. The audit brief notes the user "specifically asked
about 'sample music'". No music design exists.
**Currently in code:** no audio at all.
**Why it matters:** whether the game has a score, stings only, or is
music-silent is a defining tonal choice and the user has raised it.
**PROPOSAL - genuine fork, owner to choose (the user specifically asked about
"sample music"):** the amount of music is a defining tonal decision and the
owner raised it, so this is a fork, not a default:

- **Fork A - near-silent tension design.** No continuous score. A low evolving
  drone bed (procedural MetaSounds: layered pads, filtered noise, a slow pulse)
  that rises with round and heat, plus discrete stings only. This matches
  brief-v3's "silence is a tool". No licensed tracks, no sample packs beyond
  Sonniss stems as texture.
- **Fork B - a restrained but present score.** The drone bed as above, PLUS a
  sparse recurring musical motif per station keyed to its accent colour, that
  surfaces at round start and fades under combat, and a proper ~40s piece for
  the end-of-run screen. Still no wall-to-wall score, but music is a felt
  presence, not just absence. This is where "sample music" would live - as
  licensed or self-made stems used compositionally, not as texture.

The sting inventory (round start, breather end, special-round warning, a
train-arrival motif, a downed sting, a run-end piece) is common to both forks
and is specified fully in section 22. Note the train-arrival motif must get a
legal pass (section 22 / 15.2) so three descending notes do not read as the
real NR/TfL station chimes.
Recommendation if forced: Fork A for v1 (cheapest, on-tone), with Fork B's
per-station motif as a post-v1 addition. But the owner opened this door and
should decide.

### 12.2 Announcement voice source

**Ambiguity:** brief-v3 Phase 8: "original announcements". free-assets.md:
"Generate station announcements with TTS or a friend's voice." brief-v2 Fable
prompt: "SpeechSynthesis for platform announcements ... Do not transcribe real
recordings or imitate any named announcer." So: original phrasing, synthesised or
amateur voice, not a named announcer. Not resolved which.
**Currently in code:** nothing.
**Why it matters:** TTS vs recorded is a production pipeline decision affecting
every station's 3 lines plus the global lines (4.4).
**Suggested default (changed - two voice registers):** generate all announcement
audio offline and bake to WAV per line (no runtime TTS dependency, full control
over phrasing and pacing). But split the register:
- **Automated lines** (train arrival, "stand clear of the doors", doors closing)
  use a single flat synthesised voice, deliberately artificial - "the system,
  not a person".
- **Station-character lines** (the 3 per-station lines, the idle/ambient line)
  use a sparse, slightly degraded human PA read if a voice can be sourced (a
  friend, per free-assets), because a fully robotic voice across every station
  undercuts the "lived-in, something has happened" tone the reference frame and
  14.4 are going for. If no human voice is available, fall back to the synth
  voice with heavier processing (compression, a bandpass, tape wobble) so it at
  least sounds like a decades-old tannoy, not a modern TTS.
One synth voice across all stations for the automated lines; the human lines can
vary per station. All are drop-in WAV replacements. Subtitles on by default
(11.4, section 23). The actual line CONTENT is specified in section 22, with the
legal-pass rule (15.2).

### 12.3 Ambient beds and station audio parameters

**Ambiguity:** schemas.ts `StationDef.ambient: { hum, drip, wind, rumbleDistance
}` and brief-v2 Fable prompt "per-station ambient beds driven by each station's
audio parameters". So four named parameters exist as a web schema. brief-v3
Phase 8 says "Ambience" with no parameter list. The four params have no defined
ranges or effects.
**Currently in code:** nothing (the schema is web-era TypeScript, not ported).
**Why it matters:** if each station's atmosphere is parameter-driven, the
parameters and their audio mapping must be defined for the port to `ULTStationData`.
**Suggested default:** port the four params to `ULTStationData.AmbientParams` as
0 to 1 floats (rumbleDistance in metres): `hum` drives the electrical/ventilation
bed level, `drip` drives random water-drip one-shot frequency (raised by the
flood mechanic), `wind` drives a low airflow whoosh (high for open_air/depot),
`rumbleDistance` sets how far a distant unseen train rumble is spatialised. Canary
Wharf: hum 0.4, drip 0.7 (flooded), wind 0.15, rumbleDistance 30 (matching
debug-yard's values as a starting point). Add a `station tone` low pad keyed to
`AccentColour`.

### 12.4 Weapon audio per archetype

**Ambiguity:** brief-v2 Fable prompt: "Web Audio for gunfire per archetype".
`ULTWeaponData` has a `FireSound` `USoundBase` field (one sound per weapon,
currently unset). No archetype grouping, no reload/empty-click/tail spec.
**Currently in code:** `FireOnce` plays `WeaponData->FireSound` at the origin if
set. Nothing else.
**Why it matters:** 10 weapons need audio; per-archetype (pistol/SMG/rifle/
shotgun/LMG/sniper) is cheaper than 10 bespoke sets.
**Suggested default:** 6 archetype fire sounds from Sonniss (pistol, SMG, rifle,
shotgun, LMG, sniper), assigned per weapon via a `FireSoundArchetype` enum on
`ULTWeaponData` with `FireSound` as an optional override. Plus a shared dry-fire
click, a per-archetype reload foley set, and an indoor tail/reverb send tuned per
station (a platform is very reverberant). Spatialised via attenuation.

### 12.5 Diegetic audio occlusion / "behind you" fidelity

**Ambiguity:** brief-v3 Phase 8 accept: "you can tell what is happening behind
you with your eyes closed." That requires spatialised zombie audio with some
occlusion so a zombie round a corner sounds different from one in the open. No
spec for occlusion approach.
**Currently in code:** nothing.
**Why it matters:** the accept criterion is specific and demanding and needs an
audio-occlusion decision (UE's built-in audio occlusion trace, or a simpler
distance-only model).
**Suggested default:** use UE's audio occlusion (a single trace per active sound
source to the listener, low-pass + attenuation when blocked), enabled on zombie
vocal beds, footsteps, the train, and interaction sounds. Cap concurrent
occlusion traces at ~16 (nearest sources). This is enough to meet the accept
criterion without a full acoustics system. The harder half of "hear what is
behind you" is the concurrency and voice-stealing model for 44 vocal loops
competing with gunfire and the train rumble - that is specified in section 22
(mix, ducking, voice budget), which this item now depends on.

---

## 13. Menus, persistence, progression

### 13.1 Title screen

**Ambiguity:** brief-v3 Phase 9: "title with a slow camera orbit of a generated
station" (web era, "generated" no longer applies). brief-v3 (UE) has no title
screen spec beyond the menu list. No spec for what it shows, what options
(Play, Continue?, Settings, Stats, Quit), or the backdrop.
**Currently in code:** nothing. `L_GreyboxTest` loads straight into gameplay.
**Why it matters:** first thing the player sees, and it sets whether there is a
"Continue" (mid-run save) at all.
**Suggested default:** a title screen in a dedicated `L_MainMenu` map: a slow
locked-off or gently drifting camera on the Canary Wharf platform (art-dressed
in Phase 7, grey box until then), the game title as an original wordmark (not
Johnston), and: New Run, Settings, Stats, How to Play, Quit. No "Continue" (no
mid-run save in v1, see 13.3). The station schematic (6.5) is a small motif on
the menu.

### 13.2 Stats screen

**Ambiguity:** brief-v3 Phase 9: "Stats screen: rounds survived, kills by type,
headshot percentage, accuracy, points earned, stations visited, longest single-
station stand." So the stat LIST exists. Not specified: per-run vs all-time,
where kills-by-type data comes from (no per-type tracking exists), whether it is
its own screen or part of the end-of-run screen.
**Currently in code:** nothing. `OnZombieDied(zombie, bHeadshot)` is the only
death signal; no type, no aggregation.
**Why it matters:** "kills by type" needs per-type tracking wired from Phase C
(when types exist), and the stats need a persistence store.
**Suggested default:** two views: an end-of-run summary (this run's numbers, on
the death/end screen) and an all-time Stats screen from the main menu (bests and
totals). Track per-run: rounds survived, kills total and by type, headshot %,
shots-fired / shots-hit accuracy, points earned, stations visited, longest single-
station stand (rounds without boarding), best heat reached, Oyster Credit earned.
Persist all-time: best round, best single-station stand, total runs, total kills,
Oyster Credit balance, per-station best round. Wire per-type kill counting into
`OnZombieDied` when `ULTZombieData` lands (add a type enum to the broadcast).

### 13.3 Save system and mid-run persistence

**Ambiguity:** brief-v2 Fable prompt Phase 7: "Save in localStorage,
checkpointing on every arrival at a new station." brief-v3 (UE) Phase 10 accept:
"continue at a second station with its round counter intact" (that is in-memory
travel, not a save). brief-v3 lists no save system in its phases at all. So the
web build had per-station checkpoint saves; the UE build's phase plan dropped
save entirely.
**Currently in code:** no save. `ULTPointsComponent`, round, perks all runtime.
**Why it matters:** whether a run can be resumed after quitting is a
fundamental structure question. "Checkpoint on arrival" implies you can quit at
station 2 round 8 and resume there.
**Suggested default:** v1 has NO mid-run save (arcade run: quit = run over).
Persist only: settings, all-time stats, Oyster Credit, unlocked attachments, per-
station best round. Use one `USaveGame` written on every station arrival, every
round end, and on quit (for the meta-progression fields only, not run state). A
resumable-run checkpoint is a documented post-v1 addition (it needs the full run
state serialised: weapons, attachments, perks, points, heat, round, station,
downed-budget). State this explicitly in brief-v3, superseding the web-era
localStorage checkpoint model.

### 13.4 Meta-progression scope

**Ambiguity:** Oyster Credit + attachment unlocks (brief-v2 Phase 6) is the only
between-run progression. brief-v3 carries it forward. Is that the whole
meta-game, or are there unlockable weapons, perks, stations, cosmetics, difficulty
tiers?
**Currently in code:** nothing.
**Why it matters:** decides whether the game is "pure arcade with a small unlock
tree" or has a progression spine, which affects retention design and scope.
**PROPOSAL - needs sign-off:** "arcade-first, meta-progression is deliberately
thin" is one of the biggest identity decisions for the game (arcade vs
progression spine) and the owner should own it explicitly, not receive it as a
default. Proposal: v1 meta is Oyster Credit unlocking attachments (7.6, 8.4) and
nothing else; all weapons, perks and stations available from run 1 (gated only
by in-run points and station tier); no prestige, no cosmetics, no difficulty
selection; the all-time stats screen is the retention hook.
**Internal inconsistency to resolve:** this "thin" claim sits against 7.6 and
4.8, which between them build a two-earn-path Credit economy with per-attachment
costs and a 15-to-20-run completion curve - that is not thin. Pick one: either
the meta really is minimal (a handful of unlocks, a few runs), or it is a real
progression spine and should be designed as one. The two items must agree.

### 13.5 End-of-run and end-of-line screens

**Ambiguity:** brief-v3 Phase 9: "death screen, end-of-line screen at termini".
Two distinct screens. Neither is specified. "End of line" implies reaching a
terminus is a form of victory/completion.
**Currently in code:** nothing.
**Why it matters:** the game needs a defined ending state(s). "End of line" as a
win condition is a significant structural implication mentioned only in passing.
**Suggested default:** Death screen: on run-ending death, fade to black, then a
summary (round reached, key stats from 13.2, Oyster Credit earned this run),
options Retry / Main Menu. End-of-line screen: reached by boarding at a terminus
station (none in v1, so this is dormant), a slightly more celebratory version of
the same summary framed as "you made it to the end of the line", the intended
"beat the game" state for the expansion. For v1, there is no win condition; the
run ends only in death, and the goal is a high round / high per-station stand.
Document that v1 is endless.

### 13.6 Gunsmith screen access and rules

**Ambiguity:** brief-v3: "gunsmith (attachment swapping, only during a breather
or on the train, never mid-wave)". So the WHEN is specified. The screen itself,
what it shows, whether it costs points or only Oyster Credit (unlock vs equip),
and whether it pauses the breather timer are not.
**Currently in code:** nothing.
**Why it matters:** a gunsmith that pauses the 10s breather is very different from
one you must use fast within it.
**Suggested default:** the gunsmith is a screen opened with a dedicated key
during a breather or while on the train (blocked mid-wave). It shows your 2
carried weapons and their 3 slots each; equipping/swapping an already-unlocked
attachment is free and instant; unlocking costs Oyster Credit and can be done
here or on the main menu.
**Breather timer (reconciled with 1.3):** the gunsmith does NOT pause the
breather - but 1.3 now holds the breather at 10s (never below 8s) and gives a
14s breather after special rounds, specifically so the gunsmith stays usable off
the train. The tension is "you can re-kit one weapon per breather, or do it
properly on the train", not "the breather is too short to open the screen". If
the breather were shrunk (do not), this default would break, which is why 1.3
changed. On the train the full travel-transition time is available (no timer
pressure), making the train the comfortable place to re-kit.

---

## 14. Art direction gaps

### 14.1 Grey box vs reference frame: no interim visual bar

**Ambiguity:** notes and README are clear the reference frame is a Phase F/7
target, not to be measured against the grey box. But there is no stated interim
bar for Phases C to E (train, heat, perks) which will have some visual
representation (the train geometry, departure board, lighting shifts for heat and
blackout). Are those grey too?
**Currently in code:** grey box only.
**Why it matters:** the train and the heat/blackout lighting are gameplay-
readable elements; if they are pure grey primitives the mechanics may not read in
playtests, muddying whether the mechanic or the art is the problem.
**Suggested default:** Phases C to E stay grey-box geometry BUT get functional
(not final) lighting and colour coding: the train is a dark shell with working
headlights and a lit interior spill; heat/blackout use the real crimson/sodium
palette shifts (not final materials, just coloured lights); the departure board
is readable text on a dark panel. This is "grey box plus signal colour", enough
to playtest the mechanics honestly without starting the art pass. State it in
tasks/README.

### 14.2 Original station mark not designed

**Ambiguity:** art and notes both specify the substitute for the roundel: "a
violet horizontal bar over a charcoal field, station name in the project
typeface. Distinct silhouette, no circle-and-bar." That is a brief, not a design.
No actual mark exists, no sizes, no where-it-appears rhythm (signage-and-
wayfinding.md gives real roundel placement rhythm to substitute into).
**Currently in code:** nothing.
**Why it matters:** the station mark is the single most-repeated identity element
and a named legal substitution; every station needs it and it is undesigned.
**Suggested default:** design one mark in Phase F: a solid violet horizontal bar
(aspect ~5:1) sitting slightly below centre on a charcoal square field, station
name in the project typeface (Overpass) centred on the bar in charcoal knockout,
a thin sodium underline. No circle, no target. Sizes: 1m square on the trackbed-
opposite wall repeated every ~15m (matching real roundel rhythm), 400mm on
platform totems, 200mm on concourse flags. Record the spec and an SVG in
art-direction once drawn.

### 14.3 Project typeface not finalised

**Ambiguity:** art section 7 says Overpass was intended but is not in the project;
`WBP_HUD` currently uses `DroidSansMono` + `Roboto` as engine-shipped stand-ins.
free-assets.md and signage-and-wayfinding.md both recommend Overpass (+ Overpass
Mono + Public Sans). The choice is "float ed" but not committed and not imported.
**Currently in code:** `WBP_HUD` uses `/Engine/EngineFonts/DroidSansMono` and
`/Engine/EngineFonts/Roboto`.
**Why it matters:** every downstream UI and all in-world signage must match, so
this should be locked before Phase F and ideally before more HUD work.
**Suggested default:** commit to **Overpass** (UI body + signage), **Overpass
Mono** (HUD numerics, departure board, platform numbers), **Public Sans** (dense
byelaw/safety notice posters only). Import all three `.ttf` under
`/Game/LastTrain/Fonts/` with their OFL.txt, swap the two `WBP_HUD`
`FontObject` refs, and record the decision in art-direction section 7 as final.

### 14.4 Advertising content spec

**Ambiguity:** art section 4 and advertising-and-dressing.md give strong guidance
on advert CATEGORIES, placement rhythm, aging, and "one in four carries lore" and
"two or three recurring fictional brands". No actual brands are named, no lore
thread is written, no poster artwork brief exists.
**Currently in code:** nothing.
**Why it matters:** posters are "one of the better lore channels" (art) and cover
a lot of wall area; leaving them undesigned means Phase F invents the world's
backstory ad hoc.
**PROPOSAL - needs sign-off (this is worldbuilding, owner's call):** the
advertising needs 3 recurring fictional brands and a lore thread that hints at
the outbreak across the timeline - that structural requirement is confirmed by
art section 4. But naming the brands (a fintech, an energy-drink/streaming
brand, a public-body campaign) and writing the outbreak lore is a creative
decision for the owner, not something the audit should invent. Shape to fill in:
(1) a Canary Wharf-appropriate financial brand, violet-keyed; (2) a consumer
brand that appears everywhere and degrades over the timeline, sodium-keyed; (3)
a public-safety campaign, crimson-keyed, that is the lore thread. ~12 poster
concepts split mundane/lore per the 1-in-4 rule. All brand names then need the
resemblance check in 15.3. Record in a new `docs/reference/advertising-content.md`
once the owner defines them.

### 14.5 Gore extent and rating target

**Ambiguity:** art section 6: "Restraint ... Blood concentrated in the immediate
combat area and on the zombies themselves ... Impact decals pooled and capped."
No dismemberment decision, no gib decision, no rating target (PEGI 16/18, ESRB
M). brief-v2 crowd notes (discarded) said "Ragdoll is out of scope; use a death
animation with a snap to a settled pose."
**Currently in code:** `OnDeathPresentation` and `OnHitReaction` are empty
Blueprint hooks. No blood, no dismemberment.
**Why it matters:** dismemberment is a big art/anim/perf decision and a rating
driver, and it is unaddressed.
**Suggested default:** no dismemberment, no gibs in v1 (matches the restraint
brief and the "snap to settled pose" death). Blood: a muzzle-direction spray
decal on hit (pooled, capped ~40, oldest recycled), a small Niagara burst
(Niagara Examples pack), a ground pool under a corpse. Headshots get a bigger
burst. Rating target PEGI 16 / ESRB M.
**Causal link (the review's point):** it is PEGI 16 rather than 18
*specifically because* there is no dismemberment and no gore extremes - that
"no dismemberment" decision is what keeps the rating down, so if dismemberment
is ever added the rating target moves with it. State that link in art-direction
so the two are not decided separately.

### 14.6 Lighting numeric targets

**Ambiguity:** art section 5 and strategy.md section 4 describe the three-layer
lighting scheme (readability floor, station lighting, torch) and that blackout
drops layer 2 to nothing and layer 1 "to roughly a fifth". These are relative,
not numeric. No lux targets, no exposure values, no torch cone angle/intensity/
range.
**Currently in code:** `L_CanaryWharf_Greybox` has default Directional Light,
Sky Atmosphere, Sky Light, Height Fog "to see by". No torch. brief-v3 (UE) does
not mention a torch at all (it was a web-build signature effect; the FP camera
may make it a headlamp).
**Why it matters:** "fight what you cannot see" is the named risk. The lighting
needs concrete targets for Phase F, and the torch's existence in the UE build is
unconfirmed.
**Suggested default (decide the torch exists first):** the first decision, which
the owner must make and which is not a number, is **does the UE build have a
player torch/headlamp at all?** The torch was a web-build signature effect;
brief-v3 (UE) never mentions one, and in first person it would be a headlamp
parented to the camera. If YES: a spotlight on the camera, ~35 degree cone,
slight sway, always on - photometric numbers (intensity, range) are Phase F
tuning, not lockable now. If NO: blackout rounds and dark stations rely entirely
on muzzle flash, emergency lighting and the readability floor, which is a
harder, more claustrophobic design and needs the readability floor set higher.
Resolve the yes/no, then number it.
**Readability-floor and station-lighting targets (once the torch question is
settled):** readability floor ~0.3 lux equivalent so silhouettes and the
platform edge always read; station lighting (emissive strips + point lights) ~40
to 80 lux at floor; blackout drops station lighting to 0 and the floor to ~0.06
lux (a fifth). Post process volume per level, exposure locked (no auto-exposure
surprises), slight filmic tonemap. Phase F starting numbers.
**PPV contents (the review's missing item):** the full per-level post process
volume spec is undefined. Confirm for v1: exposure locked, slight filmic
tonemap, bloom at a low threshold, a subtle vignette, film grain that ramps with
low health (brief-v2's "grain intensity ramp"), NO chromatic aberration (it was
in brief-v2, drop it), NO motion blur, light sharpen. Record in art-direction.

### 14.7 Reference frame's HUD is explicitly rejected but the replacement is thin

**Ambiguity:** notes and brief-v3 Phase 9 both say the HUD is "explicitly not the
reference frame's furniture: no permanent minimap, no challenge tracker, no kill
feed, no exfil banner." The positive spec (11.1) is the 4 corners. That is
agreed. But the reference frame IS the art target for everything else, so there
is a tension: the frame's atmosphere is the goal, its HUD is banned, and the
seam between "copy this" and "reject this" runs through the same image.
**Currently in code:** phase-b3 HUD follows the restrained spec.
**Why it matters:** art tasks referencing the frame need the HUD carve-out
restated every time (art already warns "every art task must restate these").
**Suggested default:** no change to the decision (restrained HUD, atmosphere from
the frame). Add a one-line standing instruction to every art/HUD task template:
"Match the frame's lighting, materials and composition. Ignore its HUD entirely;
the HUD spec is brief-v3 Phase 9 / this doc 11.1." Already partly done; make it a
checklist item.

---

## 15. Legal and naming

### 15.1 The in-world line name is not chosen

**Ambiguity:** brief-v2 calls it "a fictionalised Elizabeth line" / "fictionalised
version of London's Elizabeth line". brief-v3: "Rename the line in-world. Station
names stay factual." art: "Rename the line in world." No name has been picked
anywhere in the repo. CLAUDE.md's `check_hygiene.py` "rejects ... TfL trademark
leakage" so "Elizabeth line" in shipped strings is a CI risk.
**Currently in code:** the project title is "LAST TRAIN"; the line is unnamed.
**Why it matters:** the line name appears on every departure board, in every
announcement (4.4), on the network schematic, and is a named legal substitution.
It must be decided before Phase C writes any announcement text.
**Suggested default:** name the line. Candidates that evoke East London /
Docklands without being a real line: **"the Dock Line"**, **"Meridian line"**,
**"the Reach"**, **"Isle line"**. Recommend **"Meridian line"** (Greenwich
meridian, plausible modern line name, no collision with
Jubilee/Elizabeth/DLR/Overground). Record in CLAUDE.md's legal section,
gameplay-canon and art-direction, and add it to the hygiene checker's allowed
list.
**Accent-colour caveat (fold in, the review flagged it):** the project violet
`#6C4C9C` is visually close to the real Elizabeth-line purple. A fictional line
that is purple AND serves Canary Wharf and Whitechapel (both real Elizabeth-line
stations, and the 4.11 train choice is the Jubilee stock which is grey-lined)
reads as "the Elizabeth line with the serial numbers filed off". If genuine
distance is the intent, either shift the line's accent colour away from
Elizabeth purple on the schematic and rolling stock (a distinctly different
violet, or lean sodium), or accept the resemblance as deliberate homage and note
it. Bundle this decision with 15.6 (the title needs a distinguishing element
too, likely the line name).

### 15.2 How close can announcement phrasing get to real TfL wording

**Ambiguity:** the brief repeatedly says "original phrasing", "do not transcribe",
"no recorded announcements", "imitate no named announcer". But common transit
phrases ("mind the gap", "stand clear of the doors", "this station is", "the next
station is") are near-universal and functional. branding-precedent.md says the
protected things are "the roundel, the typefaces, the diagram, the recorded
announcements" - phrasing itself is not listed as protected there.
**Currently in code:** no announcement text yet. materials/rolling-stock notes
say "Recreate as an original mark; do not reproduce the exact 'mind the gap'
artwork or the recorded announcement."
**Why it matters:** Phase C needs a clear rule so announcement writing does not
stall on legal caution, and so `check_hygiene.py` knows what to flag.
**Suggested default:** rule: functional transit phrases in common use across many
operators worldwide are fine ("stand clear of the doors", "the next station is",
"this is a request stop"). The specific TfL-associated catchphrase "Mind the gap"
is avoided as a set phrase (use "Mind the gap between the train and the platform"
rephrased, or "Watch the gap"). No phrase is lifted verbatim from a known TfL
recording. Announcements are written fresh, in a slightly clipped
system-generated register. Add "mind the gap" (exact phrase) to the hygiene
checker's reject list alongside the trademark terms.

### 15.3 Advertising accidental-resemblance pass

**Ambiguity:** art section 4: "No real company names, logos, or recognisable
campaign artwork." branding-precedent.md: "the fictional advertising needs a pass
for accidental resemblance to real campaigns" - but flags this as a
"before any paid release" concern, not now.
**Currently in code:** no adverts.
**Why it matters:** 14.4 proposes naming 3 fictional brands; those names and any
poster artwork need a resemblance check.
**Suggested default:** when the 3 brands (14.4) are named, run a basic check: the
names are not real companies (trademark search), the logos are not close to real
marks, the campaign lines are not real slogans. Keep the brands abstract enough
(a coined portmanteau, not "Barclays-but-spelled-differently"). Full legal review
only before any commercial release, per branding-precedent. Document the check in
`advertising-content.md`.

### 15.4 Rolling stock livery originality

**Ambiguity:** art: "Original livery in the project palette. Violet is fine; the
specific stripe arrangement is not." rolling-stock.md: "No operator livery,
colour stripe arrangement or logo is described or reproduced; the shape,
structure and wear are fair game." So the SHAPE (the Class 345 "Aventra"
main-line-gauge silhouette, per 4.11 RESOLVED) is fine to copy closely, the
livery must be original. No original livery is designed. The safe livery is
now specified (charcoal `#16161C` bodyshell, sodium `#E0A030` cab band, violet
`#6C4C9C` door surrounds, original typeface, made-up operator mark) in
`brief-v3-unreal.md` Part 1; a train that is simply purple-and-white and
unmarked still reads as Elizabeth line and is off limits.
**Currently in code:** grey box train shell only.
**Why it matters:** the train "fills one whole side as a wall" and is the most-
looked-at surface; its livery is a defining, legally-sensitive art element.
**Suggested default:** design an original livery in Phase F: bare brushed-
aluminium body (realistic and neutral), a single violet band at waist height
(different proportion and position from any real line's stripe: make it thin,
high, and interrupted at the doors), charcoal roof and skirt, sodium door-edge
accents, an original 2-to-4-character car-number scheme. No wordmark on the body
beyond the number. Record with a reference sketch in art-direction.

### 15.5 Real station names on invented geography

**Ambiguity:** the rule is "station names and geography are factual and fine".
But brief-v2's 3-station set (Canary Wharf, Whitechapel, Paddington) are not all
mutually adjacent on any one real line, and the invented "Meridian line" (15.1)
would connect them in an order no real line does. Is a factually-impossible
adjacency graph still "factual geography"?
**Currently in code:** debug-yard.ts has `adjacent: ['debug-yard']` (self, a test
hack).
**Why it matters:** the schematic UI (6.5) draws the line; if it shows Canary
Wharf next to Paddington that is visibly not real, which may or may not matter
legally/tonally.
**Suggested default:** the in-world "Meridian line" is explicitly a fictional
route that happens to serve real stations in an order real lines do not. The
unlicensed precedent for this is **Modern Warfare 3**, which shipped a
recognisable Underground level with zero TfL marks (per branding-precedent.md) -
that is the route LAST TRAIN takes. (Watch Dogs: Legion actually *licensed* the
roundel and mixed real and invented stations, so it is the wrong cite for the
no-licence approach; the original audit had this backwards.) State in
gameplay-canon that adjacency is a game-design graph, not a claim about real
routes. For v1's 2 stations the adjacency is plausible anyway, so this is mostly
an expansion note.

### 15.6 Project / game title clearance

**Ambiguity:** "LAST TRAIN" is the project name (CLAUDE.md). No note on whether
"Last Train" is clear as a game title (it is a common phrase; there may be other
games or media with the name).
**Currently in code:** used as the module name (`LastTrain`), the uproject name,
and throughout.
**Why it matters:** if the game is ever released, the title needs to be clear;
"Last Train" alone is very generic and possibly taken.
**Suggested default:** treat "LAST TRAIN" as the working title. "Last Train" is a
very common title (multiple games, films, an album), so a distinguishing element
is likely needed - the line name is the obvious one ("Last Train: Meridian
Line"). Bundle this with the line-name decision (15.1) since they resolve
together. Before any release, check for conflicting game titles. Not a v1
blocker; flag in branding-precedent.md's "if the project ever goes commercial"
section.

---

## 16. Doc contradictions and dead design (web/ build)

### 16.1 Engine: v2 is Three.js, v3 is UE5

**RESOLVED 2026-09-06 (partial: the prominent header).** `docs/brief-v2.md` now
carries a prominent header near the top marking it PARTLY SUPERSEDED: DEAD are
engine, renderer, camera, phase plan, gates, asset strategy and the Fable/Opus
prompts (Parts 0 to 4 and 6); LIVE are the round loop, train timing (100s / 25s),
station heat, the five zombie types, the mechanic library, the economy numbers
and the legal constraints. The separate `gameplay-canon.md` extraction is still
a future owner task. Text below is the audit trail.

**Ambiguity:** brief-v2 is entirely written for "Three.js, WebGPU with WebGL2
fallback" with a full custom render graph, VAT crowd, procedural geometry
generator. brief-v3 discards all of it ("The entire src/ tree ... Around 12,000
lines ... kept on main under the phase-03 tag"). brief-v2 remains "authoritative
for the round loop, the train mechanic, station tiering, the mechanic library,
the economy". So brief-v2 is half-live, half-dead and a reader must know which
half.
**Currently in code:** UE5.8 C++ chassis. `web/` is tagged `phase-03`, not built.
**Why it matters:** brief-v2 is a landmine: its Fable prompt (Part 4), its
credit/gate structure (Part 3), its rendering spec (Part 2), its asset strategy
(Part 1) are all moot, but its gameplay numbers are canonical. A session pointed
at brief-v2 could implement the wrong things.
**Suggested default:** add a prominent header to brief-v2: "DEAD (engine,
renderer, camera, phases, gates, asset strategy, the Fable/Opus prompts, Parts 0
to 4 and 6). LIVE (round loop, train timing, station heat, the five zombie types,
the mechanic library, the economy, the legal constraints - scattered through
Parts 4 and 5)." Better: extract the live gameplay numbers into a single new
`docs/design/gameplay-canon.md` and reduce brief-v2 to a historical note. The
current cross-references ("still authoritative for X") are too easy to miss.

### 16.2 Camera: third-person 55-degree vs first-person

**RESOLVED 2026-09-06.** brief-v3's first person (FOV 95, per the C++
`ALTPlayerCharacter`) is canonical. `docs/brief-v2.md` now carries DEAD notes on
Part 2 and Part 2.7 flagging that the 55-degree third person, spring arm, cursor
lookahead and occlusion fade are superseded. Text below is the audit trail.

**Ambiguity:** brief-v2: "Steep angled third person at roughly 55 degrees, spring
arm". brief-v3: "First person. Wide default FOV, around 95 to 100 horizontal".
The C++ `ALTPlayerCharacter` is first-person (`Camera` on the capsule at Z 68,
`bUsePawnControlRotation`, `ViewModel` set `OnlyOwnerSee`). brief-v3 wins and the
code follows it.
**Currently in code:** first person, `BaseFieldOfView = 95`.
**Why it matters:** anything in brief-v2 that assumes a top-down-ish tactical view
(the "player must never be occluded, fade any wall between camera and player",
the minimap-heavy HUD, "cursor lookahead") is wrong for FP.
**Suggested default:** brief-v3's first-person is canonical. Note in brief-v2 that
its Part 2.7 camera section and every "cursor"/"lookahead"/"occlusion fade"
reference is dead. The FP camera changes several downstream things: no cursor aim
(mouse-look), the "torch as fog-of-war" concern becomes a headlamp (14.6),
crosshair-driven interaction (10.3) replaces cursor-hover.

### 16.3 Station count: 3 (v2 Phase 5) vs 41 (v2 Phase 6) vs 2 (v3)

**RESOLVED 2026-09-06.** v1 commits to **2 stations**. `docs/brief-v3-unreal.md`
Part 5 (Risks) and `docs/tasks/README.md` now state this explicitly and mark any
"41 stations" reference an aspirational expansion list, not a plan. Text below is
the audit trail.

**Ambiguity:** covered in 6.1. Recorded here as a doc contradiction: brief-v2
says 3 then 41; brief-v3 says "one then expand ... two good stations beats forty";
tasks/README Phase G says "second station". Three docs, three numbers.
**Currently in code:** 2 grey box maps, 0 real stations.
**Why it matters:** as 6.1.
**Suggested default:** 2 for v1 (6.1). Add a line to brief-v3 Part 5 (Risks) and
tasks/README making 2 the explicit committed number and 41 an "aspirational
expansion list, not a plan".

### 16.4 Model split: v2 vs v3 vs strategy.md

**RESOLVED 2026-09-06.** `CLAUDE.md` plus `brief-v3-unreal.md` Part 4 are the
live model-split guidance. `docs/brief-v2.md` Part 3 now carries a DEAD note, and
`docs/strategy.md` carries a header at the top noting its model-split and engine
sections predate the Unreal move and are superseded. Text below is the audit
trail.

**Ambiguity:** brief-v2 Part 3: Fable does Phases 1 to 5, Opus does 6 to 8.
brief-v3 Part 4: Opus for C++/data/Blueprints/HUD/docs, Fable for materials/
Lumen/animBP/crowd-perf/stubborn-bugs. strategy.md: "Supersedes the credit split
in brief-v2.md Part 3" and gives yet another breakdown, then also notes it
"predates the move to Unreal Engine 5". CLAUDE.md: Opus for C++/data/Blueprint/
layout/balance/docs, Fable for Lumen/post/material-graphs/animBP/crowd-perf/
two-attempt-bugs. So CLAUDE.md and brief-v3 broadly agree; brief-v2 and
strategy.md are stale.
**Currently in code:** n/a (process doc).
**Why it matters:** a session reading strategy.md or brief-v2 for the model split
gets stale guidance (e.g. "spend Fable on the volumetric raymarch" - there is no
volumetric raymarch in UE, Lumen does it).
**Suggested default:** treat CLAUDE.md + brief-v3 Part 4 as the live model-split
guidance. Mark brief-v2 Part 3 and strategy.md sections 1 to 2 as superseded
(strategy.md already hedges this; make it explicit). Consider deleting or
heavily trimming strategy.md since almost all of it is web-era.

### 16.5 Web build design data not carried into UE docs

**Ambiguity:** `web/src/data/` holds real design intent the UE docs lack:
`legend.ts` has `TILE_COST` (water 3, barrier 4, escalator 1.4), `SKIRTING_HEIGHT
0.15`, `TRACK_DROP 1.1`; `schemas.ts` has the full `StationDef` shape including
`ambient{hum,drip,wind,rumbleDistance}`, `fogDensity/fogColour/wetness/exposure`,
`hdri{}`, `debrisCosts` map, `perks`/`wallbuys` maps, and a `validateStation`
with concrete rules (grid >= 8x8, must have S, must have B or T, flood needs ~,
exactly 3 announcements); `debug-yard.ts` has a worked example with sample
announcement lines and ambient values.
**Currently in code:** the UE `ULTWeaponData` exists but there is no
`ULTStationData`, no mechanic config, no ambient params. The tile costs live only
in the TypeScript legend.
**Why it matters:** this is authored design that will be re-invented (probably
inconsistently) when Phase C/6 needs it. The `validateStation` rules in
particular are a free spec for a UE station validator.
**Suggested default:** port deliberately (6.4): create `ULTStationData` from the
`StationDef` shape (dropping web-render fields), carry `TILE_COST` into the nav
area classes (2.1, 6.2), carry the `validateStation` rules into a UE data
validator run on `ULTStationData` assets, and keep `debug-yard`'s announcement
lines and ambient values as the Canary Wharf starting point. Add a
`docs/design/station-data-port.md` mapping each web field to its UE home or
"dropped, reason". The web `TILE`, `WALL_HEIGHT`, `PLATFORM_LIP`, `TRACK_DROP`
constants already match `canary-wharf-grid.md` (1.5m, 3.6m, 0.2m, 1.1m); keep
them as the canonical dimensions.

### 16.6 debug-yard adjacency is a self-loop hack

**Ambiguity:** `debug-yard.ts` has `adjacent: ['debug-yard']` (adjacent to
itself) to pass `validateRegistry`'s "no adjacency" check. If a `ULTStationData`
validator is ported (16.5) it needs to allow or handle a testbed station with no
real neighbours.
**Currently in code:** the TypeScript hack; no UE equivalent.
**Why it matters:** small, but the ported validator should not choke on the
grey-box test maps which have no adjacency.
**Suggested default:** the UE validator treats a station with an empty
`AdjacentStations` array as valid only if it is flagged `bIsTestbed = true`;
otherwise it is an error (a real station with no neighbours is unreachable).
`L_GreyboxTest` and `L_CanaryWharf_Greybox` (until wired into the line) are
testbeds.

### 16.7 brief-v2 vs brief-v3 on the ASCII grid's role

**Ambiguity:** brief-v2: the ASCII grid is "the authoring format" and "the sole
input to the geometry generator". brief-v3: "demoted from an authoring format to
a layout sketch used to block out geometry in the editor." grid: "This is a
sketch to block out geometry against, not an authoring format for a generator."
So v3 wins, but the grid file still uses the full web legend and reads like an
authoring artefact.
**Currently in code:** the Canary Wharf grey box was hand-built from the grid as
a sketch (with the normalisation issues in 6.6).
**Why it matters:** a session might try to build a generator from the grid, or
treat grid ragged-ness as a bug in a parser that does not exist.
**Suggested default:** confirmed: the grid is a human blockout reference only,
there is NO generator in the UE build. The shared legend is kept purely so the
vocabulary is consistent across the grid file, `canary-wharf-grid.md`, and
`debug-yard`. Fix the grid's raggedness (6.6) for human readability, not for a
parser. State this at the top of `canary-wharf-grid.md`.

### 16.8 phase-03 web build as a fallback

**Ambiguity:** brief-v3 Part 5: "If UE proves too heavy, phase-03 on main is a
working, typechecked, browser-deployable fallback." strategy.md echoes it. But
brief-v3 also says the web build "made that mistake in reverse and produced a
renderer with no game in it" - so the "fallback" is a renderer with no gameplay.
**Currently in code:** `web/` tagged `phase-03`, not maintained.
**Why it matters:** calling it a "fallback" overstates it; it is a renderer demo,
not a playable game, and reviving it would mean building the entire game loop in
TypeScript (Phases 4 to 8 of brief-v2, never done).
**Suggested default:** downgrade the language: `phase-03` is a "preserved
rendering prototype", not a fallback plan. The realistic fallback if UE is too
heavy is to simplify the UE game (fewer zombies, simpler lighting), not to
switch engines. Note this in brief-v3 Part 5.

### 16.9 reference-frame-notes.md still cites flow-field pathfinding

**RESOLVED 2026-09-06.** `docs/reference/reference-frame-notes.md` section 4 has
been corrected: the horde mechanism now reads as per-zombie UE5 navmesh pathing
(`AIController::MoveToActor` per `LTZombieCharacter`, plus a per-zombie
stall-recovery nudge) along spawn routes down the tunnel-mouth openings. There
is no flow field; that was the discarded web build. The text below is the audit
trail.

**Ambiguity:** `docs/reference/reference-frame-notes.md` section 4 ("How the
frame maps to the phases") lists the horde mechanism as "Flow field plus spawn
routes down the tunnel mouths", built in "Phase B, D". The flow field was the
discarded web build's system. The UE build uses navmesh + `MoveToActor` per
zombie (verified in `LTZombieCharacter.cpp`); there is no flow field and none is
planned. The original audit noticed this obliquely in 2.1 but never flagged it
as a live doc contradiction.
**Currently in code:** `AIController::MoveToActor` on a jittered repath cadence,
RVO for separation. No flow field.
**Why it matters:** `reference-frame-notes.md` is a doc that CLAUDE.md and the
tasks README explicitly tell a fresh art or AI session to read FIRST. A stale
"flow field" line there will send someone building the wrong system, or treating
the absence of a flow field as a gap to fill.
**Suggested default:** correct `reference-frame-notes.md` section 4 in place:
change "Flow field plus spawn routes down the tunnel mouths" to "Per-zombie
navmesh pathing (`MoveToActor`) plus lane-bias steering (2.1) and spawn routes
down the tunnel mouths". One-line fix, no design change. (Out of scope for this
doc to edit; flagged as a task.)

### 16.10 Elizabeth line vs Jubilee line rolling stock (silent contradiction)

**RESOLVED 2026-09-06 (owner decision), see 4.11.** The line is a fictionalised
Crossrail-scale line (main-line loading gauge, modelled on the Elizabeth line,
NOT a deep-level tube); the train is the Class 345 "Aventra" silhouette; the
platform is a main-line loading-gauge box. brief-v2, brief-v3, art-direction,
reference-frame-notes and the canary-wharf-research folder have been updated to
this framing. This reverses the "suggested default" below (which leaned 1996
Stock deep-tube). Text below is the audit trail.

**Ambiguity:** brief-v2 and art-direction call the line "a fictionalised
Elizabeth line". `research/rolling-stock.md` describes the reference-frame
station as Jubilee-line Canary Wharf: 1996 Stock deep-tube (small curved body,
126m, external sliding doors), with full-height platform screen doors. The
Elizabeth line at Canary Wharf is Class 345 Aventra: main-line gauge,
walk-through, 205m, no PSDs in the same way, bright airy interior. These are
completely different trains, arenas and boarding geometries. Neither the
original audit nor an initial read caught this as a contradiction; the audit's
4.1 just picked "1996 Stock deep-tube profile" without noting it conflicts with
the stated line.
**Currently in code:** grey box train shell only.
**Why it matters:** this decides the whole arena geometry - platform width,
ceiling height, screen doors vs open trackbed, the "train as a wall" silhouette,
and the boarding animation. It is a **Phase C blocker**. Fully specified as an
item in 4.11 (the design decision) and 4.12 (platform screen doors); recorded
here as the doc contradiction it also is.
**Suggested default:** per 4.11 - commit to the **1996 Stock deep-tube profile**
(the Jubilee-line Canary Wharf that the reference frame, the grid and the grey
box all already are), keep the in-world line name original (15.1), and retcon
the "fictionalised Elizabeth line" wording in brief-v2 and art to the chosen
line name or "fictionalised deep-tube line". Adopt platform screen doors (4.12)
unless a gameplay reason argues against them.

### 16.11 Phase-letter (A to G) vs phase-number (0 to 8 / 0 to 11) vocabulary clash

**Ambiguity:** `docs/tasks/README.md` uses Phase A to G. brief-v2 uses Phase 0
to 8. brief-v3 uses Phase 0 to 11. The mappings are not one-to-one (README
Phase C = brief-v3 Phases 5, 6 and parts; README Phase G = brief-v3 Phases 8, 9,
10, 11). The original audit itself used "Phase C", "Phase 9", "Phase 11" and
"phase-b3" interchangeably in the same items (e.g. old 11.1 said "Phase C adds
train, heat, perks" but README Phase C is rounds+types+train+board+heat and
perks are README Phase E). A locked spec needs one vocabulary.
**Currently in code:** n/a (process/doc).
**Why it matters:** every cross-reference in this document and every "tune in
Phase N" hand-wave is ambiguous until one scheme wins.
**Suggested default:** standardise on the **Phase A to G letters from
`docs/tasks/README.md`**, because that file explicitly "supersedes the roadmap
in brief-v3-unreal.md Part 3 where they differ" and is the live roadmap. This
consolidated document already uses the letters throughout, with the mapping
table in the header. Action: add the same mapping table to the top of
`docs/tasks/README.md`, and add a one-line note to brief-v2 and brief-v3 that
their phase numbers are historical and the letters are canonical. Also note
(from NEXT.md) two live constraints a planning doc should carry: `CommonUI` is a
forced uproject dependency (do not remove), and the NeoStack editor tooling
trial expired around 2026-09-07, so editor-side work several items assume (2.4,
6.x, 11.x) now needs a human or another tool.

---

## 17. Core game framework (GameMode, GameInstance, game state machine)

The module has no `AGameModeBase`, `UGameInstance`, `AGameStateBase` or
`APlayerState` subclass (verified: nothing of the sort in `Source/LastTrain/`).
Travel (4.7), persistence (13.3, 21.x), station identity injection into the
round manager (6.4), stats aggregation (13.2), first-run detection (1.1, 20.x)
and the run lifecycle all hang off a backbone that does not exist. This is the
single largest structural gap in the codebase and no section owned it.

### 17.1 No ULTGameInstance

**Ambiguity:** nothing persists across a level load. `ULTPointsComponent`, the
round counter, perks and weapon state all live on runtime actors that are
destroyed on travel. brief-v3's travel model and 4.7's "travel payload struct"
both assume a game instance that survives level loads.
**Currently in code:** the default `UGameInstance`; no project subclass.
**Why it matters:** travel between stations (the signature loop) cannot carry
round, points, perks, weapons or heat without it.
**Suggested default:** create `ULTGameInstance`. It owns: the current run state
(a `FLTRunState` struct - round, points, perks, both weapon slots with upgrade
and attachment state, magazine/reserve, self-revive budget, Oyster Credit earned
this run, stations visited, best heat reached); the meta save handle (21.x); and
a `bIsFirstRun` flag read from the meta save on startup. Travel serialises
`FLTRunState` into the instance, loads the destination `.umap`, and the
destination's GameMode rehydrates from it.

### 17.2 No ULTGameMode

**Ambiguity:** who spawns the round manager, tells it which station it is, wires
`OnRoundEnded` to the train, starts the pre-round beat (1.1), and handles the
end-of-run transition (13.5)? Right now the round manager finds spawn points by
world iteration on `BeginPlay` and is started by a Level Blueprint calling
`BeginRounds` (per NEXT.md, and not even wired in `L_CanaryWharf_Greybox` yet).
**Currently in code:** the default game mode; Level Blueprint glue.
**Why it matters:** Level-Blueprint glue does not scale to two stations, a
pre-round beat, travel, or an end-of-run flow, and it is already a known gap
(NEXT.md item 3.14).
**Suggested default:** create `ALTGameMode`. On `BeginPlay` it: finds the
`ALTStationInfo` actor and reads its `ULTStationData` (6.4); finds or spawns the
`ALTRoundManager` and injects the station data (roster, `BASE_CAP` override,
mechanics); runs the pre-round beat (1.1) then calls `BeginRounds`; owns the
train actor and wires `OnRoundEnded` -> train scheduling; listens for the
player's `OnDied` and drives the end-of-run screen (13.5); handles a board event
-> `ULTGameInstance` travel. `L_MainMenu` uses a separate trivial game mode.

### 17.3 The run lifecycle / game state machine is undefined

**Ambiguity:** there is no explicit state machine for "in menu / establishing
beat / round active / breather / train dwelling / boarding / travelling / downed
/ run over". Behaviour that should key off state (pause availability, spawn
gating, input lockout, HUD elements) is scattered across `bDead`, `bInBreather`,
`bRunning` bools on different actors.
**Currently in code:** `ALTRoundManager` has `bRunning` and `bInBreather`;
`ALTPlayerCharacter` has `bDead`; no shared authority.
**Why it matters:** the pre-round beat, travel, downed and end-of-run all need a
single source of truth for "what state is the run in", and multiple systems need
to read it.
**Suggested default:** a `ULTGameStateComponent` (or `ALTGameState`) holding an
`ELTRunPhase` enum: `MainMenu`, `Establishing`, `RoundActive`, `Breather`,
`TrainDwell`, `Boarding`, `Travelling`, `Downed`, `RunOver`. The GameMode owns
transitions; the round manager, player, train, HUD and pause menu all read it.
`bDead` becomes `RunOver`, `bDowned` becomes `Downed`, `bInBreather` becomes
`Breather` - the bools collapse into the enum.

### 17.4 Round manager station-awareness wiring

**Ambiguity:** 6.4 proposes `ULTStationData` but never says how the round
manager gets it. Today the round manager is "station agnostic".
**Currently in code:** `ALTRoundManager::BeginPlay` iterates the world for spawn
points; `ZombieClass`, `MaximumAlive`, `OpeningRoundCounts` are per-instance
editor values.
**Why it matters:** per-station roster, `BASE_CAP`, mechanics and announcements
all need to reach the round manager, and doing it via per-instance editor values
on every station's map instance is error-prone (the `L_GreyboxTest` stale
override in NEXT.md is exactly this failure).
**Suggested default:** the GameMode (17.2) reads the `ALTStationInfo` ->
`ULTStationData` and calls a new `ALTRoundManager::ConfigureFromStation(const
ULTStationData*)` on `BeginPlay`, which sets the roster, `BASE_CAP`, mechanic
configs and announcement set. Per-instance editor overrides stay possible for
testbeds but are no longer the primary path.

### 17.5 First-run vs returning-player branch

**Ambiguity:** 1.1 and 20.x both need to know if this is the player's first-ever
run. Nothing detects it.
**Currently in code:** nothing.
**Why it matters:** the establishing beat length (1.1), the onboarding lines
(20.x) and the title screen ("New Run" only, no "Continue" - 13.1) all branch on
it.
**Suggested default:** `ULTGameInstance::bIsFirstRun` is true if the meta save
(21.x) has zero completed runs. Set false the first time a run ends (by death or
quit). Onboarding beats (20.x) key off this flag.

### 17.6 What "the run" is scoped to

**Ambiguity:** the original audit assumed endless-until-death but section 1 never
stated whether a session is one station or the whole line, whether there is a
target round, or what "win" means. 13.5 confirms v1 is endless but the framing
belongs up front.
**Currently in code:** `CurrentRound` "never resets"; no win condition.
**Why it matters:** every section 1 item hangs off this.
**Suggested default:** a run is: start at Canary Wharf, round 1, survive as many
rounds as possible, travelling between the 2 stations freely, until a run-ending
death. There is no win condition and no target round in v1 (the "end of line"
screen, 13.5, is dormant until a terminus exists). The goal is a high round
and/or a high single-station stand. State this as the first line of
gameplay-canon.

### 17.7 Round-number display cap / rollover

**Ambiguity:** nothing defines the round readout past round 100, which a farming
player reaches. (7.4 dropped the points cap; the round number has the same
non-issue.)
**Currently in code:** `int32 CurrentRound`, no cap, HUD unspecified at 3 digits.
**Why it matters:** trivial, but the TL round element's layout should not break
at "100+".
**Suggested default:** no cap on the round number. The HUD round element is laid
out for 3 digits; a 4th digit (round 1000, essentially never) scales the text.
No rollover, no prestige. Not worth more than this line.

### 17.8 Pause during the establishing beat and breather

**Ambiguity:** 11.3 (now) allows pause always, but the establishing beat (1.1)
and travel transition (4.7) are edge cases worth stating.
**Currently in code:** no pause at all.
**Why it matters:** a player who needs to stop during the 5s intro or a level
load should be able to.
**Suggested default:** pause works during `Establishing` and `Breather`. During
`Travelling` (a level load) pause is meaningless (the game is loading) so `Esc`
is buffered and the pause menu opens on the destination. During `Boarding`'s
1s hold, `Esc` cancels the hold rather than pausing (so you do not commit to a
board you were interrupted mid-decision on).

---

## 18. Performance and the crowd system

brief-v3 sets "60fps at 1080p, mid-range GPU, 24 zombies". Every "wall of
zombies" number in this document (the 5.1 cap and its heat add, penetration
weapons, spatialised zombie audio, corpse pools, decals) spends against a budget
no section tracked. `MaximumAlive` was picked three different ways precisely
because there is no ledger. This section is that ledger.

### 18.1 The frame budget is not written down

**Ambiguity:** "60fps with 24 zombies" is a target, not a budget. There is no
per-frame ms allocation across game thread, render thread, GPU; no split within
that for AI, animation, navigation, audio, VFX, Lumen.
**Currently in code:** no profiling infrastructure; the round manager has
`// DIAGNOSTIC` spawn logging.
**Why it matters:** `BASE_CAP` (5.1) is supposed to be derived from this, and
right now it is a guess.
**Suggested default:** define the budget as: 16.6ms total at 60fps, target
platform floor a 2020-era mid GPU (RTX 2060 / RX 5600 XT class) at 1080p with
the "Medium" preset. Provisional per-frame allocation: game thread 8ms (of which
crowd AI + navigation <= 3ms, everything else 5ms), render thread 8ms, GPU the
gate under Lumen. `BASE_CAP` is set to the largest concurrent zombie count that
holds 60fps in a real Phase C profile at Canary Wharf with functional lighting,
and `HEAT_CAP_ADD * HEAT_MAX` (5.1) is capped so the worst case still holds
55fps. Until that profile exists, `BASE_CAP` stays 24.

### 18.2 Crowd system: per-agent AIController + RVO, DetourCrowd, or Mass

**Ambiguity:** each zombie runs `MoveToActor` off its own `Tick` with its own
`AAIController` and per-agent RVO. At a cap of 24 to 44 that is 24 to 44
controllers + 24 to 44 ticking characters + 24 to 44 RVO agents + the
pathfinder's own query concurrency. free-assets.md notes "a StateTree or
Behaviour Tree plus EQS would be a cheap upgrade later". No decision on whether
to stay per-agent or move to `DetourCrowd` (shared avoidance, path following) or
Mass (ECS crowd).
**Currently in code:** per-agent `AAIController` + `MoveToActor` +
`bUseRVOAvoidance`.
**Why it matters:** this is the single biggest determinant of how many zombies
the frame budget allows, and it must be decided before `BASE_CAP` can be
profiled meaningfully.
**Suggested default:** for v1's cap range (24 to ~44), stay **per-agent
AIController + `MoveToActor`**, but switch separation from per-agent RVO to
**`DetourCrowd`** (via `UCrowdFollowingComponent` on the AI controller) - it
scales better at 40+ agents and handles the shared-goal clumping (2.1's
attack-slot problem) more gracefully. Do NOT adopt Mass for v1 (large rewrite,
the zombie needs per-instance health/hit-reaction/attack state that Mass makes
awkward). Re-evaluate Mass only if a profile shows per-agent ticking is the
bottleneck at the target cap. Throttle path queries: the repath jitter (0.35s
+/- 40%) already spreads them; add a hard per-frame cap on new path requests
(~4) with a queue.

### 18.3 Zombie mesh: skeletal vs Vertex Animation Texture

**Ambiguity:** brief-v2's discarded web crowd used VAT ("per-instance playback
rate"). The UE build has one skeletal `ALTZombieCharacter`. free-assets.md
section B proposes City Sample Crowds (skeletal, with the animation-sharing and
LOD system). No decision on skeletal vs VAT/animation-instancing for the far
field.
**Currently in code:** plain skeletal mesh characters, no LOD strategy stated,
no animation update-rate optimisation.
**Why it matters:** 40 fully-skeletal characters animating and casting shadows
at full rate is a large chunk of the budget.
**Suggested default:** skeletal for v1 (VAT loses the per-instance ragdoll-snap
death and per-bone headshot hitbox, both of which the design needs). Apply:
`UROAnimInstance` / Animation Budget Allocator so far zombies tick their anim
less often; aggressive skeletal mesh LODs (LOD3 a very cheap proxy by ~15m);
disable dynamic shadow casting on zombies beyond ~10m (a blob shadow or none);
`bEnableUpdateRateOptimizations` on the mesh. If a Phase C profile still cannot
hold the cap, the fallback is animation instancing (Niagara mesh renderer or a
crowd plugin) for zombies beyond ~12m, keeping skeletal only for the near field.

### 18.4 "One mesh varied by scale and colour" vs a crowd-variation system

**Ambiguity:** brief-v2 Part 1 and free-assets both start from "one mesh, vary
by scale, colour, animation speed", but free-assets section B then proposes City
Sample Crowds (6 bodies, 12 heads) for "5 to 8 distinct undead types". So the
plan quietly shifted from one mesh to a crowd-variation system, and it is
unresolved which, which decides whether the five types (3.1) read visually at
all. brief-v3's honest-ceiling section flags exactly this ("per-zombie variation
... not achievable solo at that fidelity").
**Currently in code:** one `ALTZombieCharacter`, no mesh variation.
**Why it matters:** it is both a visual-legibility decision (do the types read?)
and a perf decision (mesh/material variety costs draw calls and memory).
**Suggested default:** the five gameplay TYPES (3.1) are distinguished primarily
by silhouette (scale), speed, and one strong visual tell each (the crawler's
pose, the brute's mass, the sprinter's animation, the screamer's mouth), NOT by
being five bespoke characters. Within a type, use City Sample Crowds-style
body/head/material variation (a handful of bodies, a set of heads, a small
palette of grimed materials) so a crowd of walkers is not clones. Budget:
<= 6 unique skeletal meshes total across all types, <= 12 head variants, <= 8
material instances, so the draw-call and memory cost stays bounded. Confirm the
types read in a Phase C grey-box test before the art pass.

### 18.5 Corpse, decal and VFX ceilings

**Ambiguity:** the original 2.8 proposed a corpse pool cap of 12; 14.5 caps
blood decals at ~40. Nothing ties these to the frame budget or lists the other
transient costs.
**Currently in code:** `SetLifeSpan(CorpseLifetime)` (6s) per corpse, no pool
cap; no decal system; `MuzzleFlash` unset.
**Why it matters:** on a big round, dozens of 6s corpses plus muzzle flashes
plus impact particles plus blood decals is a cost the "24 zombies" target never
accounted for.
**Suggested default:** corpse pool cap 10 (oldest destroyed when an 11th dies),
6s lifetime, corpses disable tick + collision + shadow. Blood decal pool 32,
oldest recycled. Impact particles pooled, hard cap ~24 concurrent. Muzzle flash
one per weapon, Niagara (not Cascade - see 8.1). One shared footstep
surface-typed sound set for the whole crowd, volume-scaled hard. All of these
are `EditDefaultsOnly` so a profile can adjust them.

### 18.6 Navmesh build strategy and the door-open rebuild

**Ambiguity:** brief-v3 Phase 3 accept says "navigation rebuild on door open"
and `LTSpawnPoint` has `AreaTag` for door gating, but nothing says whether the
rebuild is a full `RebuildNavigation`, a dirty-region rebuild, or runtime
nav-link toggling, or what it costs. 4.13 adds the arriving train as a dynamic
obstacle.
**Currently in code:** navmesh is whatever the grey box maps have baked; no
runtime rebuild code.
**Why it matters:** a full navmesh rebuild mid-round is a multi-frame hitch; a
door opening or a train arriving every ~100s cannot cause one.
**Suggested default:** static navmesh baked per station. Debris doors and the
arriving train use **runtime dirty-region rebuilds** (`RebuildNavigationData` on
a bounded volume) or, if that still hitches, pre-baked nav areas that are
toggled navigable/not-navigable with no geometry rebuild (the door's opening is
a nav-modifier volume switched off). Measure the hitch in Phase C; if a
dirty-region rebuild is over ~2ms, use the toggle approach. The train (4.13) is
best handled as a permanently-carved nav hole plus track-side spawn gating, not
a live obstacle.

### 18.7 AI tick and path-query budget under the cap

**Ambiguity:** every live zombie ticks its own `Tick`, decrements timers, and
periodically issues a path request. At 44 alive that is 44 full character ticks
plus 44/0.35s ~= 125 path requests per second competing for the pathfinder.
**Currently in code:** per-agent `Tick`, jittered repath, no global throttle.
**Why it matters:** the pathfinder has its own concurrency limit; blowing past
it queues requests and zombies path late.
**Suggested default:** a global path-request throttle (18.2), plus significance-
based tick: zombies beyond ~25m from the player and not in the player's view
tick their AI at half rate (repath 0.7s) and their animation via the budget
allocator. Near zombies stay at full rate. This is a distance/visibility LOD on
behaviour, separate from the mesh LOD.

### 18.8 The dev profiling overlay

**Ambiguity:** brief-v2 wanted a profiler overlay; brief-v3 Phase 11 says
"profile first, report every number you change and why". No overlay exists.
**Currently in code:** `stat unit`, `stat game`, `LT_LOG` spawn diagnostics.
**Why it matters:** section 20's balance loop and this section's cap derivation
both need a live readout during PIE.
**Suggested default:** a debug HUD widget (toggle key, dev builds only) showing:
frame ms (game/render/GPU), live zombie count, cap, heat, current round, spawn
interval, path requests/sec, corpse count, active decals, draw calls. This is
the instrument for 18.1 and 20.x. Build it in Phase C alongside the crowd work.

---

## 19. Anti-training / anti-kiting AI design

The defining problem of round-based survival: the player runs a predictable loop
and the horde trails in a single-file "conga line", making high rounds trivial.
brief-v3's reference frame *is* "a horde funnelled down a corridor" - the exact
geometry that enables a train loop. Nothing in the original audit addressed it.
The 2.1 attack-slot rule and the 3.1 sprinter speed are the pieces; this section
is the design.

### 19.1 The conga-line problem

**Ambiguity:** `MoveToActor` sends every zombie down the navmesh shortest path
to one point, so a moving player is chased by a line hugging one edge. The
lane-bias steering in 2.1 spreads the line slightly but does not break the
follow-the-leader behaviour.
**Currently in code:** shortest-path `MoveToActor`, RVO separation, no flanking.
**Why it matters:** a conga line is trivially kited - the player runs a circle
and shoots the front of the line forever. This is the failure mode the whole
section exists to prevent.
**Suggested default:** a lightweight flanking bias. On each repath, a fraction
of the horde (scaling with round and heat, ~20% at round 5, ~50% by round 15)
does not path to the player's current position but to a **predicted intercept
point**: the player's position plus their velocity times an estimated
time-to-reach. So while the player runs a loop, part of the horde cuts the
chord and meets them. Implemented as a per-zombie flag set at spawn or on
repath, changing the `MoveToActor` goal from the player to a computed intercept
actor/location. No behaviour tree needed.

### 19.2 Spawn selection that gets ahead of a running player

**Ambiguity:** `TrySpawnOne` picks a weighted-random available point with no
regard for where the player is or which way they are moving. A kiting player
runs away from every spawn.
**Currently in code:** weighted random among `IsAvailable` points.
**Why it matters:** if spawns are always behind the player's motion, the loop is
safe; spawns need to sometimes appear ahead.
**Suggested default:** bias spawn-point weighting by the player's movement: a
point roughly ahead of the player's velocity vector (within ~60 degrees, and
out of direct line of sight per 2.4) gets a weight multiplier (~2x) that scales
with round and heat. A point directly behind gets a slight reduction. Points in
view are still skipped (2.4). This makes a predictable loop run *into* fresh
spawns without teleporting anything.

### 19.3 The sprinter as the anti-kite answer

**Ambiguity:** 3.1 sets the sprinter to 500 (between player walk 420 and sprint
640). Its design role is to punish a lazy kite - but the original audit's 340
made it slower than a walking player, so it did nothing.
**Currently in code:** no types.
**Why it matters:** the sprinter is the primary tool that forces the player to
actually sprint (and therefore manage stamina, if 10.2 adds it, or at least
commit and lose ground elsewhere) rather than jog a safe circle.
**Suggested default:** confirm sprinter base 500 (3.1), first appearing round 5,
and mixing into normal rounds from heat 3+ (5.1) so a high-heat player who is
kiting always has a few sprinters in the mix breaking their loop. On sprinter
rounds (3.2) the whole loop premise breaks, which is the intended spike. A
sprinter that loses line of sight for >3s drops to walker speed until it
re-acquires (so it does not laser onto a player who broke contact cleanly - it
is a punish for predictability, not an unshakeable heat-seeker).

### 19.4 Choke-point and escalator exploits

**Ambiguity:** 6.2's original interchange gave the player a 2x one-way escalator
they could ride up to a perch and shoot down at a bottlenecked horde forever.
That is the classic training exploit.
**Currently in code:** nothing.
**Why it matters:** any one-way vertical element the player can hold and the
horde cannot flank becomes the optimal, boring strategy.
**Suggested default:** per the 6.2 rework - escalators give a modest boost
(1.4x) and zombies use them at near-parity (1.2x), so the mezzanine is a loop
the player runs, not a fort. Additionally: the mezzanine has its own spawn
points (activating from round 6) so a player who camps up there gets flanked
from above too. No single position on any v1 station should let the player face
only one approach direction; every hold spot has at least two threat vectors.
State this as a level-design rule for Canary Wharf and station 2.

### 19.5 Rewarding movement without punishing it into a loop

**Ambiguity:** the tension is: the game wants the player mobile (kiting is the
core verb) but not running one safe circle. Nothing states where that line is.
**Currently in code:** nothing.
**Why it matters:** over-correcting (flanking so aggressive the player can never
move) kills the feel; under-correcting leaves the loop optimal.
**Suggested default:** the target feel is "you must keep moving and keep
*changing* your movement". The flanking fraction (19.1), ahead-spawn bias
(19.2) and sprinters (19.3) all scale with round and heat, so early rounds a
simple loop works (a teach) and late rounds it gets punished progressively. A
player who varies their route, uses the two stations, and manages the train
should always outperform a player running one circle. Phase G measures whether
a fixed-loop strategy caps out several rounds below a mobile-varied strategy;
if it does not, increase the flanking fraction.

### 19.6 De-leash vs the round-end condition

**Ambiguity:** 3.4's leash (a zombie that loses the player for 20s de-spawns)
interacts with anti-kite: if flanking zombies get stuck trying to intercept,
they must still be reachable for `LiveZombies.Num() == 0`.
**Currently in code:** no leash, no flanking.
**Why it matters:** an intercept-pathing zombie that the player outruns forever
must not soft-lock the round.
**Suggested default:** an intercept-pathing zombie that has not closed distance
on the player for 12s reverts to direct pathing; the 1.4 unreachable check and
the 3.4 leash apply on top. So flanking is a bias, never a permanent state that
can strand a zombie off the round-end condition.

---

## 20. Onboarding and the first run

brief-v3's success test is "the first ten minutes feel coherent". A new player
dropped onto a dark platform with a pistol has no idea the train is an escape,
that heat exists, or what wall buys do, and the restrained HUD has no tutorial
layer. The original 11.6 was the only item and its default (a slightly longer
prompt the first time you look at a wall buy) does not meet the bar - a player
who never looks at a wall buy never learns they exist.

### 20.1 No onboarding layer at all

**Ambiguity:** nothing teaches the core loop. No tutorial, no forced sequence,
no "how to play" surfaced anywhere the player will see it.
**Currently in code:** nothing.
**Why it matters:** the whole first-ten-minutes success criterion.
**Suggested default:** a first-run-only diegetic onboarding, no pop-ups, keyed
off `bIsFirstRun` (17.5):
- The establishing beat (1.1) is longer on a first run (8s) and the announcement
  explicitly frames the train ("This platform is closed. The next service is not
  scheduled to stop. Please do not attempt to board.") - so the first thing the
  player hears is that the train matters and boarding is a thing.
- The first time a wall buy is within interact range on a first run, a short
  one-time diegetic caption appears near it ("PRESS E - PURCHASE") for 6s,
  independent of whether the player is looking at it, so it cannot be missed.
- The first train's approach on a first run adds one line: "This service will
  accept passengers. Please have your travel ready." - said once, ever.
- The first time heat reaches 1 on a first run, one line: "Service disruption.
  Expect increased activity on the platform."
- A "How to Play" screen on the main menu AND auto-shown once before the
  first-ever run (skippable), covering: the train is an optional escape that
  ends the round and travels to the next station; staying raises heat (more
  zombies); points buy weapons off walls and open doors; there is no minimap,
  read the station; you can go down once per round and crawl to safety.

### 20.2 Teaching wayfinding with no minimap

**Ambiguity:** 11.1 bans the minimap; nothing replaces the wayfinding it would
provide. A new player does not know where the wall buys, the concourse, the
mezzanine or the exits are.
**Currently in code:** nothing.
**Why it matters:** "read the station" only works if the station is legible.
**Suggested default:** wayfinding is entirely diegetic and is a level-design
requirement, not a HUD feature: original directional signage (14.2 station mark
and "Way out" substitute) at every junction, the departure board visible from
decision points (11.2), wall buys lit with a distinct sodium glow readable from
across the platform, the interchange escalators visually obvious. The station
schematic panel (4.6) is openable any time as the "map". First-run onboarding
(20.1) does not tour the layout; the layout must teach itself.

### 20.3 First zombie / first round as a teach

**Ambiguity:** brief-v3 Phase 1's accept is "a zombie reaches you and damages
you". Nothing says whether round 1, zombie 1 should be a guaranteed gentle
single approach or just the normal spawn logic.
**Currently in code:** normal spawn logic; `OpeningRoundCounts[0] = 6`.
**Why it matters:** a first-ever player's first contact sets the tone; six
zombies converging from multiple points in the first 30s is a lot.
**Suggested default:** on a first run only, round 1 spawns from a single point,
one at a time with a longer interval (2.5s), so the first contact is a clean
one-on-one the player can learn the shoot/back-pedal rhythm against. From round 2
(or any non-first run) normal spawn logic. This is a `bGentleFirstRound` flag on
the round manager set by the GameMode from `bIsFirstRun`.

### 20.4 Onboarding for the downed state

**Ambiguity:** the first time a first-run player goes down, they do not know they
can crawl or how to revive (disengage, per 9.3).
**Currently in code:** nothing.
**Why it matters:** without knowing the disengage rule, a first down feels like
a death and reads as unfair.
**Suggested default:** the first time the player enters `Downed` on a first run,
one diegetic caption: "GET CLEAR - MOVE AWAY FROM THE CROWD" for the first 4s of
the bleed-out. Once, ever.

### 20.5 Not gating the "How to Play" behind a menu a masher skips

**Ambiguity:** the original 11.6 put "how to play" only on the main menu, where a
Play-button masher never sees it.
**Currently in code:** nothing.
**Why it matters:** the players who most need it are the ones who skip menus.
**Suggested default:** as 20.1 - the How to Play screen auto-shows once before
the first-ever run (a single skippable screen, not a wall), then lives on the
main menu for re-reading. After the first run it never auto-shows again.

### 20.6 First-run vs run-50 opening beat

**Ambiguity:** section 1 never asked whether run 50 gets the same opening as run
1.
**Currently in code:** nothing.
**Why it matters:** the establishing beat and onboarding lines are welcome once
and tedious the fiftieth time.
**Suggested default:** run 2+ uses the short 5s establishing beat (1.1), no
onboarding captions, normal round 1 spawn logic, no auto How to Play. All the
teaching in this section is `bIsFirstRun`-gated.

---

## 21. Telemetry and balance methodology

brief-v3 Phase 11 and brief-v2 Phase 8 both say "profile first, report every
number you change and why", and the target is "reaches round 12 to 15 on a first
serious run". There is no item on how that is measured. Every "tune in Phase G"
in this document (dozens) has no methodology behind it.

### 21.1 What "round 12 to 15 on a first serious run" actually means

**Ambiguity:** "first serious run" is undefined - the first run ever? The first
run after understanding the systems? A median player or a good one?
**Currently in code:** nothing measured.
**Why it matters:** it is the single balance target and it is unfalsifiable as
worded.
**Suggested default:** define it: a "first serious run" is a player's 3rd to 5th
run (past the first-run onboarding, understands the loop, has not yet mastered
it), playing to survive rather than to farm. The target: the median such run
ends between round 12 and 15. A skilled farming run should reach round 25+; a
brand-new first run might end round 6 to 9 and that is acceptable. Record this
definition in gameplay-canon so Phase G has a real bar.

### 21.2 What gets logged

**Ambiguity:** no event logging exists beyond `LT_LOG` diagnostics.
**Currently in code:** `LT_LOG` spawn logging, `stat` commands.
**Why it matters:** balance passes need data, not vibes.
**Suggested default:** a dev telemetry sink (writes a JSON/CSV line per event to
a local file in dev builds, off in shipping) logging: run start/end (with cause,
round reached, stations visited, duration); per round (number, duration, zombies
spawned, peak concurrent, player HP low-water-mark, points earned, shots
fired/hit); each down (round, cause, location, whether recovered); each board
(round, heat at board, points held); each purchase (item, price, round); each
heat change. Plus a session id so multiple runs aggregate.

### 21.3 The dev telemetry overlay

**Ambiguity:** covered by 18.8 (the perf overlay); the balance overlay is
adjacent but distinct.
**Currently in code:** nothing.
**Why it matters:** tuning wants a live readout of the balance-relevant state,
not just frame ms.
**Suggested default:** extend the 18.8 overlay with a balance panel: current
round, heat, concurrent/cap, spawn interval, player HP and time-since-damage,
points, points-per-minute (rolling), shots-hit %, and the count of each zombie
type alive. Toggleable separately from the perf panel.

### 21.4 The balance-pass loop

**Ambiguity:** "report every number you change and why" describes discipline,
not a process.
**Currently in code:** n/a.
**Why it matters:** without a loop, balance is one person's guesses.
**Suggested default:** the loop is: (1) play ~10 runs with telemetry on,
targeting the 21.1 definition; (2) look at where the median run actually ends
and why (the telemetry says: ran out of ammo? got stunlocked by a brute? died
to a sprinter round?); (3) change ONE cluster of numbers (e.g. sprinter speed,
or the flood rise rate, or the cap) with a written rationale; (4) re-run 10;
(5) compare. Numbers live in the data assets (`EditDefaultsOnly`), never in
Blueprints or C++ literals, so a pass is data edits. Record each pass's before/
after/rationale in a running `docs/design/balance-log.md`.

### 21.5 A/B-ing a number

**Ambiguity:** no way to compare two values without a rebuild.
**Currently in code:** n/a.
**Why it matters:** faster iteration on the many provisional numbers.
**Suggested default:** the key balance knobs are `EditDefaultsOnly` on data
assets, so two variant assets (e.g. `DA_Zombie_Sprinter_A` at 480,
`DA_Zombie_Sprinter_B` at 520) can be swapped on the round manager between PIE
sessions without a compile. A console command (`lt.balance.set <knob> <value>`)
that pokes the active data asset's transient copy for a single session is a
nice-to-have for Phase G.

### 21.6 Playtest protocol

**Ambiguity:** who plays, how many runs, on what hardware, is undefined.
**Currently in code:** n/a.
**Why it matters:** "a first serious run" (21.1) needs actual first-serious
players, and the perf target needs the target hardware.
**Suggested default:** for balance: at least 2 or 3 people who have not played
before, 5 runs each, notes on where and why they died, telemetry on. For perf:
at least one run on the 18.1 target-floor machine at the target preset. Neither
needs to be formal; both need to actually happen and be written up, per Phase G.

### 21.7 Run-integrity / save-scumming for stats

**Ambiguity:** 13.2 persists "best round" and "best single-station stand".
Nothing says whether the player can edit the save, or whether alt-F4 mid-down
counts as a death.
**Currently in code:** no save.
**Why it matters:** minor for a single-player game with no leaderboard, but the
stats should be honest.
**Suggested default:** the meta save is plain (no obfuscation, no anti-tamper) -
it is single-player, there is no leaderboard, a player editing their own stats
file only cheats themselves. alt-F4 or quit-to-desktop mid-run counts as a
run-ending death for stats purposes (the run is over, the round reached is
recorded). If a leaderboard ever ships, revisit with a signed save.

---

## 22. Audio architecture

brief-v3 makes audio a whole phase (Phase G) with a hard accept criterion ("tell
what is happening behind you with your eyes closed"). The original audit gave it
5 items. This is the full section. Nothing here is coded (no audio at all
today).

### 22.1 Submix / bus architecture

**Ambiguity:** no bus layout. 3.5 needed a "player-affecting submix", 12.5 needs
occlusion, section 5 needs a heat drone - none works without the buses defined.
**Why it matters:** it is the thing every other audio item builds on.
**Suggested default:** submixes: Master -> [SFX, Ambient, Music, Voice, UI].
SFX further split into Weapons, Zombies, World, Train. A limiter on Master. A
reverb submix per station volume (22.6). Everything routes through this; no
`PlaySoundAtLocation` straight to Master.

### 22.2 Ducking rules

**Ambiguity:** nothing says what ducks what.
**Why it matters:** without ducking, 40 groans + gunfire + train rumble + an
announcement are an undifferentiated wall and you cannot hear anything behind
you.
**Suggested default:** an announcement (Voice) ducks Ambient -6dB and Music
-9dB for its duration. Gunfire (Weapons) ducks Ambient -3dB briefly (a sidechain
on the ambient bed) so shots punch. Low health (22.8) ducks World and Zombies
-4dB and lifts a heartbeat/breath bed. The train arrival (22.7) ducks Music
-6dB. Nothing ducks Zombies hard except low health - hearing the crowd is the
priority.

### 22.3 Music presence and where it plays

**Ambiguity:** 12.1 is a fork (near-silent vs a restrained score). This item
records where music plays regardless of which fork wins.
**Why it matters:** "music" without placement is meaningless.
**Suggested default:** music (whichever fork) plays: under the main menu; as the
rising drone bed during rounds (intensity keyed to round + heat); a brief motif
at round start that fades under the first shots; the run-end piece over the
death/end screen (the one unambiguous "proper" cue). Music does NOT play during
the breather (that is the quiet-station respite, 1.3) or during train dwell (the
train's own audio owns that moment).

### 22.4 The sting inventory

**Ambiguity:** stings are scattered across 1.2, 1.8, 12.1 with no single list.
**Why it matters:** a spec-writer needs one place that says what every
non-musical cue is.
**Suggested default:** the complete sting list: (1) round start - rising
sub-bass swell, 0.8s; (2) breather end / "here they come" - short falling
two-note figure; (3) special-round warning (played in the prior breather) -
round-start swell pitched up a fifth; (4) train-arrival motif - three notes,
distinct from any real station chime, legal-passed (15.2); (5) doors closing -
a single soft tone, original, NOT the NR/TfL three-tone; (6) downed - a low
descending sting under the vignette; (7) revive success - a short rising
resolve; (8) purchase confirm (`OnPurchased`) - a soft mechanical clunk + a
sodium chime; (9) can't afford - a flat negative buzz; (10) perk acquired - a
warm two-note; (11) hit marker - a short tick, headshot a brighter variant, kill
a slightly fuller variant; (12) Max Ammunition pickup - a bright ascending
figure. Each is one short baked asset.

### 22.5 The three announcement lines per station and how they are voiced

**Ambiguity:** 4.4 and 12.2 cover moments and voice tech (offline TTS / degraded
human, baked WAV) but no section commits the actual line content, register or
word count.
**Why it matters:** the announcements are the entire diegetic train timer and a
named legal risk (15.2).
**Suggested default:** per station, three lines plus the shared global lines:
- Station line 1 (played on arrival): identifies the station in the fiction, ~8
  to 12 words, clipped register. e.g. "This is [Station]. Change here for the
  [fictional connection]. This platform is not in scheduled service."
- Station line 2 (played on departure): ~6 to 10 words. e.g. "The service has
  left [Station]. Please stand back from the platform edge."
- Station line 3 (idle, once per breather): world-flavour, ~8 to 15 words,
  where the lore leaks. e.g. "Passengers are reminded that the station remains
  under a service disruption notice. Staff are not available."
- Global line, T-15 inbound: "The next service will arrive shortly. This service
  terminates at [adjacent station names]." Include an approximate number
  ("in about fifteen seconds") once, so a player who mishears has a recovery.
- Global line, T-3 before doors close: "This train is ready to depart. Please
  stand clear of the doors."
Legal-pass rule (15.2): functional multi-operator phrases are fine ("stand clear
of the doors", "the next service", "stand back from the platform edge"); the
exact standalone catchphrase "Mind the gap" is avoided as a set phrase; nothing
is lifted verbatim from a known TfL recording; the register is a clipped,
slightly degraded automated tannoy, no named announcer. `check_hygiene.py` gets
"mind the gap" (exact standalone phrase) added to its reject list, with care
that a legitimate rephrase quoting it in a longer sentence is not falsely
rejected.

### 22.6 Reverb / interior acoustics model

**Ambiguity:** 12.4 says weapon tails are "tuned per station" and stops there. A
gunshot in a tiled tunnel is mostly reverb tail; the model decides "toy gun" vs
"real place".
**Why it matters:** it carries a large part of the "this is a real place" feel
and it affects the "hear what is behind you" criterion (a shot round a corner
should sound different).
**Suggested default:** a reverb submix per station, set by an audio volume on
the platform / concourse / mezzanine / tunnel-mouth, each with a different
decay and pre-delay (platform very reverberant, ~2s decay; tunnel mouth a long
slap; concourse boomier). Use UE's built-in submix reverb, not convolution
(cheaper, enough). Weapon fire routes a wet send to the local reverb submix so
the tail matches the space the player is in. Zombie vocals and footsteps get a
lighter wet send.

### 22.7 The train's audio arrival (owned here)

**Ambiguity:** 4.1 mentions "rumble bed" and "brake-squeal one-shot" in passing;
no audio item owns the train.
**Why it matters:** the arrival is the game's signature beat and half of it is
sound; for a player who cannot see the board (11.2) it is the primary signal.
**Suggested default:** the train audio sequence: distant rail rumble that grows
over ~8s on approach (spatialised from the tunnel mouth, low-passed at
distance); a sub-bass floor-shake that ramps over the last 3s (a low sine on the
World bus, felt more than heard); an original brake squeal one-shot at ~0.5s
from stop; the doors-open mechanical clunk + the doors-closing tone (22.4 #5);
a rising traction whine on departure with a Doppler shift as the nose passes;
the rumble fading as it leaves. On a screened platform (4.12) the whole thing is
slightly muffled by the glass wall until the doors open. After the third arrival
(4.13) the sequence degrades to a shorter version.

### 22.8 Low-health audio

**Ambiguity:** brief-v2 wants a "grain intensity ramp as health drops"; the
audio equivalent is unaddressed.
**Why it matters:** a non-visual health tell matters for a dark game and for
colourblind players (the damage vignette is crimson, section 23).
**Suggested default:** below ~40 HP: a slow heartbeat bed fades in, breathing
becomes audible and quickens as HP drops, and the ducking in 22.2 pulls the
world back so the player feels isolated. At `Downed` it goes further - a dull
ringing, very muffled world, loud heartbeat. Recovers as HP regenerates. This is
a submix effect chain on Master driven by the health fraction.

### 22.9 Per-zombie-type vocalisation

**Ambiguity:** 2.9 specced per-zombie beds; this ties them to the types (3.1).
**Why it matters:** hearing which type is behind you is part of the accept
criterion.
**Suggested default:** each type has a distinct vocal bed: walker - low
irregular groan; sprinter - fast panting/snarling, higher; brute - deep
infrequent bellow, loud, carries far; crawler - wet dragging + a low gurgle
near the floor; screamer - mostly quiet until the scream (22.4-adjacent, a
loud sustained shriek that masks other audio while it lasts, per 3.1/3.5). 3 to
4 source variations per type from Sonniss creature stems, pitched by the
per-instance speed multiplier (2.2), layered so a crowd is a wash not a chorus
of clones.

### 22.10 Weapon audio per archetype

**Ambiguity:** carried from the original 12.4 - `FireSound` is one field per
weapon, unset; `MuzzleFlash` is Cascade (mismatch, see 8.1).
**Why it matters:** the roster needs audio and per-archetype is far cheaper than
per-weapon.
**Suggested default:** 6 archetype fire sounds (pistol, SMG, rifle, shotgun,
LMG, sniper) from Sonniss, assigned via a `FireSoundArchetype` enum on
`ULTWeaponData` with `FireSound` as an optional per-weapon override. Shared
dry-fire click, per-archetype reload foley, tail via the 22.6 reverb send.
Change `MuzzleFlash` to a Niagara system ref.

### 22.11 Heat's audio signature

**Ambiguity:** 5.2 says "a rising drone" and stops.
**Why it matters:** 5.2 itself says heat should not be a HUD number, so audio
carries the tell, and it must be distinguishable from the round-intensity drone
(22.3).
**Suggested default:** heat adds a distinct layer to the ambient bed per level -
a detuned low pad plus an intermittent metallic groan (the station "under
strain"), rising in level and density with heat. It sits in a different
frequency band from the round drone so a player learns "this specific texture
means heat is high" separately from "the round is getting hard". Resets with
heat on travel.

### 22.12 UI / interaction sound

**Ambiguity:** `OnPurchased` is a hook with a "flash and audio" comment and no
spec; menu/HUD sounds unaddressed.
**Why it matters:** in a game with no kill feed, the points tick and the
purchase sound are the only confirmation an action registered.
**Suggested default:** covered in the sting inventory (22.4 #8 to #12) plus:
menu navigation a soft tick, menu confirm a warmer tone, menu back a lower tone,
the interact-available blip a very quiet tick when an interactable enters range.
All on the UI submix, unaffected by world reverb.

### 22.13 Ambient beds driven by station audio parameters

**Ambiguity:** carried from the original 12.3 - `StationDef.ambient {hum, drip,
wind, rumbleDistance}` is a web schema, unported.
**Why it matters:** per-station atmosphere is parameter-driven and the mapping
must exist for the `ULTStationData` port.
**Suggested default:** port to `ULTStationData.AmbientParams` as 0 to 1 floats
(rumbleDistance in metres): `hum` -> electrical/ventilation bed level; `drip` ->
random water-drip one-shot frequency (raised further by the flood mechanic);
`wind` -> low airflow whoosh level; `rumbleDistance` -> spatialisation distance
of the distant unseen-train rumble. Canary Wharf provisional: hum 0.4, drip 0.7,
wind 0.15, rumbleDistance 30 - lifted from `debug-yard.ts` as a starting point
only (it is a test fixture, not tuned). Plus a low station-tone pad keyed to
`AccentColour`.

### 22.14 Audio performance budget

**Ambiguity:** no voice-count budget; 44 spatialised zombie loops + occlusion
traces + reverb sends + gunfire is a real cost the perf target ignores.
**Why it matters:** ties audio to section 18.
**Suggested default:** cap concurrent voices at ~64, with priority: player
weapon and low-health beds highest, then the train, then the nearest ~16 zombie
vocal beds (the rest virtualised - tracked, not rendered), then world ambient,
then distant one-shots. Occlusion traces capped at 16 (nearest sources, 12.5).
Zombie vocal beds beyond ~20m are virtualised. Budget ~1.5ms game thread for
audio at the cap; measure in Phase C.

---

## 23. Accessibility

The palette is charcoal + violet + sodium + crimson, and crimson-vs-violet is a
classic deutan/protan confusion. Heat reads crimson (5.2), emergency lighting is
crimson, the damage vignette is crimson, the special-round tell was crimson
(1.2). The game puts its timer in audio and bans the minimap. The original audit
had nothing on any of this.

### 23.1 Colourblind support

**Ambiguity:** several distinct signals are colour-only and several are crimson,
which a deutan/protan player cannot separate from violet or from each other.
**Currently in code:** nothing.
**Why it matters:** a colourblind player loses heat, damage direction,
special-round warning and emergency-vs-accent distinction all at once.
**Suggested default:** no signal is colour-only. Every colour-coded cue is
paired with a second channel: heat = crimson creep + the 22.11 audio texture +
the segmented HUD bar (5.2); damage = crimson vignette + directional
indicator shape + the 22.8 audio; special round = the pitched sting (22.4 #3) +
a departure-board glyph (not colour). Provide three colourblind modes (deuter,
protan, tritan) that remap the four palette accents to a
distinguishable set for HUD and gameplay-critical elements ONLY (not the whole
world render - the art keeps its palette; the HUD, the heat bar, the hit marker,
the vignette, wall-buy glow and interactable highlights remap). A brightness /
gamma calibration screen (already required by art, 11.4). Test the four accents
under a deutan simulation before Phase F locks materials.

### 23.2 Subtitles and captions

**Ambiguity:** 11.4 and 12.2 both say "subtitles on by default" but no section
specs the UI. The game puts the train timer in audio, so a player who needs
captions needs them to be good.
**Currently in code:** nothing.
**Why it matters:** first-class UI surface for a game this audio-dependent.
**Suggested default:** a caption system rendering: all announcement lines (22.5)
with a speaker label ("PLATFORM ANNOUNCEMENT"), timed to the audio; the T-15 and
doors-closing lines are captioned with their approximate countdown so a
caption-reliant player gets the same information; key non-speech cues optionally
captioned ("[train approaching]", "[screamer]") behind a "caption sound effects"
toggle. Bottom-centre, above the interact prompt, max 2 lines, ~4s hold,
adjustable text size. On by default.

### 23.3 Text scaling

**Ambiguity:** no HUD/UI text scale option; 11.5 mentions crosshair scaling
only.
**Currently in code:** `WBP_HUD` fixed sizes.
**Why it matters:** the HUD is deliberately minimal and small; a player needs to
be able to enlarge it.
**Suggested default:** a UI text scale setting (100% to 150%) affecting the HUD
numerics, captions and menus. The HUD's fixed anchor layout (11.1) must reflow
gracefully at 150%. Safe-area margins for the HUD corners (a 5% inset option for
TVs).

### 23.4 Remappable controls

**Ambiguity:** 11.4 lists only "invert Y". The input actions are
`EditDefaultsOnly` and the prompts hardcode "E" etc (`GetInteractionPrompt` is
`"Buy {0}: {1}"` with no key glyph).
**Currently in code:** Enhanced Input, fixed mappings, hardcoded prompt strings.
**Why it matters:** full remapping is a baseline accessibility expectation, and
button-prompt strings must follow the binding.
**Suggested default:** full key/button rebinding for every action via an
Enhanced Input mapping the player can edit, persisted to the settings save.
Prompt strings pull the current binding's glyph, not a literal "E". Include
common toggles: sprint hold/toggle, aim hold/toggle, crouch hold/toggle (crouch
is a new verb, see 10.x). Controller support: at minimum a sensible default
gamepad map; aim assist against the horde behind a toggle (a FP console shooter
needs it - light bullet magnetism + slowdown near a target).

### 23.5 Camera shake and motion

**Ambiguity:** 11.4 lists "reduce camera shake"; nothing on FOV/motion sickness
beyond the slider, or on the downed camera drop (9.3) and headbob.
**Currently in code:** no shake yet, no headbob, FOV 95.
**Why it matters:** dark FP horde game with a low downed camera is a
motion-sickness risk profile.
**Suggested default:** a "reduce motion" setting that: cuts camera shake to
~20%, disables or minimises headbob and weapon sway, softens the downed camera
drop (9.3) to a fade rather than a lurch, and reduces the damage vignette's
movement. FOV slider 90 to 110 (11.4). No forced camera animations that the
player cannot damp.

### 23.6 Difficulty options vs the fixed arcade curve

**Ambiguity:** 13.4 rules out difficulty selection; that is itself an
accessibility decision made in passing.
**Currently in code:** one fixed curve.
**Why it matters:** a fixed arcade curve locks out players who cannot hit round
12 to 15 but want to see the game.
**Suggested default:** v1 keeps one canonical arcade curve for the scored,
Oyster-Credit-earning run (so the stats and progression mean one thing). BUT add
an optional "Assist" toggle set (separate from difficulty tiers) - individual
switches for: slower zombies (~85%), reduced damage taken (~75%), longer
bleed-out, auto-disengage revive assist. An Assist run is flagged in stats and
earns reduced or no Oyster Credit (so it does not distort progression) but is
otherwise the full game. This is an accessibility feature, not a difficulty
menu, and it fits the arcade framing better than easy/normal/hard tiers.

### 23.7 The "no minimap" decision's wayfinding consequence

**Ambiguity:** 11.1 bans the minimap; 20.2 makes wayfinding diegetic. For a
player with cognitive or visual load issues, a dark station with no map is hard.
**Currently in code:** nothing.
**Why it matters:** "read the station" is a higher accessibility bar than a
minimap.
**Suggested default:** the station schematic panel (4.6) is openable any time
and doubles as an orientation aid (current station lit, you-are-here). As an
accessibility option only, allow a small persistent objective marker toward the
nearest platform / the train door when it is boarding (off by default, not part
of the intended experience, but available). Diegetic signage (20.2) must be
legible at the brightness floor for everyone; test it.

---

## Summary

- **Total items: 167** (across 23 sections). Was 98 across 16 in the original
  audit; consolidation added 7 sections (17 to 23) and items inside sections 4
  and 16.
- **By section:**
  - 1. Game start and round loop presentation: 8
  - 2. Zombie AI and locomotion: 9
  - 3. The five zombie types: 5
  - 4. The train: 13
  - 5. Station heat: 3
  - 6. Stations and the mechanic library: 8
  - 7. Economy and pricing: 6
  - 8. Weapons and the gunsmith: 7
  - 9. Perks, bench, lost property, revive: 5
  - 10. Player mechanics: 6
  - 11. HUD and UI: 6
  - 12. Audio and music: 5
  - 13. Menus, persistence, progression: 6
  - 14. Art direction gaps: 7
  - 15. Legal and naming: 6
  - 16. Doc contradictions and dead design: 11
  - 17. Core game framework (GameMode / GameInstance / state machine): 8
  - 18. Performance and the crowd system: 8
  - 19. Anti-training / anti-kiting AI design: 6
  - 20. Onboarding and the first run: 6
  - 21. Telemetry and balance methodology: 7
  - 22. Audio architecture: 14
  - 23. Accessibility: 7

- **Label split:** roughly **21 items are now `PROPOSAL - needs sign-off`**
  (design being invented, not an ambiguity being confirmed): 1.1 (first-run
  beat), 1.7 (downed/last-stand), 2.5 (direct-push design home), 3.1 (the
  zombie stat numbers), 3.3 (crawler death gas), 4.8 (points-to-Credit
  conversion), 6.8 (the Inspector boss), 7.3 (perk prices), 7.6 (unlock costs /
  grind length), 8.1 (the weapon stat table), 8.2 (pistol start), 8.3 (melee),
  9.3 (self-revive mechanic), 9.5 (the signal flare / equipment), 10.1 (fall
  damage), 10.2 (the stamina system), 12.1 (the sample-music fork), 13.4
  (meta-progression scope), 14.4 (advertising brands and lore). Plus the several
  new-section items that propose systems rather than confirm readings (17.x
  framework shape, 18.2 to 18.4 crowd/mesh decisions, 19.x anti-kite design,
  20.x onboarding design, 21.1 the balance-target definition, 22.3/22.5 audio
  direction, 23.6 the Assist toggles) - treat any item whose body says
  "PROPOSAL" or "fork, owner to choose" as needing a decision, not a tick.
  Everything else (~146 items) is `Suggested default` - confirm the obvious
  reading. The review's estimate that ~58 of the original 98 were solid as-is
  still holds; those were left alone except for cross-reference and consistency
  fixes.

### The highest-impact items (resolve these first)

Re-ranked per the fresh-eyes review, and re-numbered against the larger doc.

1. **3.1 (with 2.2, 3.4, 19.3) per-type zombie stat block, grounded.** Phase C
   blocker: the five types are a name list with no numbers. Fix the sprinter
   against the real player speeds (`WalkSpeed 420`, `SprintSpeed 640`) - it is
   now 500, not the ungrounded 340. Numbers are a PROPOSAL; the data-asset
   mechanism is a confirm.
2. **4.11 / 16.10 Train profile and platform arena: 1996 Stock (Jubilee,
   deep-tube, platform screen doors) vs Class 345 (Elizabeth, main-line gauge).**
   Phase C blocker, and a silent contradiction between the stated "fictionalised
   Elizabeth line" and the Jubilee-line reference material. Decides the arena
   geometry, the "train as a wall" read, boarding, and whether there is an open
   trackbed at all (4.3, 4.12). Recommend 1996 Stock + screen doors.
3. **6.2 mechanic definitions for the 4 v1 mechanics only** (flood, interchange,
   blackout, platform_split), with flood's rise rate slowed and its fire-block
   removed, and the interchange escalator de-exploited (19.4). The other 6
   mechanics are expansion documentation, not v1 scope.
4. **6.1 / 16.3 commit to 2 stations and name station 2 and its mechanic pair.**
   Drives every art, data and UI estimate. Reconsider `blackout` as the second
   mechanic - a tonally contrasting station (bright, run-and-gun) teaches range.
5. **16.1 extract `docs/design/gameplay-canon.md` from brief-v2's live numbers;
   demote brief-v2 to a historical note.** About an hour of work, removes the
   landmine under every future session. The "add a header to brief-v2" half-
   measure is not enough.
6. **5.1 (folding in 2.7 and the section 4 heat items) the single heat +
   concurrent-cap model.** One cap formula as a function of round and heat,
   stated once (in 5.1), cross-referenced everywhere. `BASE_CAP` derived from
   the section 18 frame budget, not guessed - stays 24 until that profile
   exists. One rate multiplier, one flat cap add, worked round-20-heat-6 figure
   shown.
7. **17.1 / 17.2 the GameMode / GameInstance / game-state backbone.** No such
   class exists in the module. Travel, persistence, station identity, the
   pre-round beat, stats and first-run detection all hang off it. Buried by the
   original audit (no section owned it); it is a Phase C prerequisite for the
   train and travel.
8. **1.7 / 9.3 / 10.5 downed-and-revive as a coherent solo mechanic** (PROPOSAL),
   with the `bDead` -> `bDowned` refactor scoped as real work, and a
   disengage-based bleed-out instead of "kill 4 while crawling". The run
   currently has no proper ending.
9. **15.1 name the line** (bundle 15.6 the title, and the violet-vs-Elizabeth-
   purple accent-colour note). Cheap, wide reach, blocks all announcement text
   (22.5) and the schematic UI, and is a CI/legal risk.
10. **8.1 weapon roster SHAPE only** (count, archetypes, tiers, which need code -
    the crossbow/joke weapon need a whole projectile system, NOT a small
    addition). All ~130 stat numbers are a PROPOSAL and must NOT be data-entered
    until the Phase G balance pass. Plus 8.7 2-weapon carry as scoped Phase C/E
    work (an unimplemented feature, not a bug).
11. **18.2 to 18.4 performance budget and crowd-system decision** (per-agent
    AIController + DetourCrowd vs Mass; skeletal vs VAT; the "one mesh" vs crowd-
    variation question). Items 1, 3 and 6 all spend against this budget and there
    was no ledger. Also: **import the typeface** (14.3, Overpass + Overpass Mono
    + Public Sans) - trivial now, blocks every future HUD and signage task.

**Explicitly NOT in the top set** (the original audit over-rated these): 10.2
sprint stamina (it is a PROPOSAL to add a new system, not an ambiguity - the
real problem is better solved by the sprinter and arena design), 7.4 points cap
(dropped, non-issue), 6.6 grid normalisation and 10.3 interact range (cleanup
tickets, not open design questions).

### What changed in consolidation

- **Reconciled the heat model.** Sections 5.1, 2.7 and the section 4 heat items
  now point at one formula in 5.1: a single cap as a function of round and heat,
  `BASE_CAP` derived from a perf budget (section 18) not a guess, one rate
  multiplier, one flat cap add, with the compounded worked figure shown. The
  original had three inconsistent versions.
- **Grounded the weak defaults.** ~40 defaults got a concrete `Rationale` line
  and a better number: sprinter speed 340 -> 500 (tied to the player's actual
  420/640); the breather held at 10s (never below 8s) instead of shrinking to
  5s, so the gunsmith stays usable; the flood mechanic no longer disables
  firing; the 4-minute round ceiling replaced with genuine stall detection; the
  stagger threshold raised so sustained SMG fire does not stagger the horde.
- **Corrected the wrong-in-code claims.** 8.7 recategorised from "serious economy
  bug" to unimplemented feature; 11.1 rewritten - the health bar EventGraph
  wiring is present and correct, the real bug is a zero-width SizeBox
  (`bOverride_WidthOverride = False`); 15.5 fixed to cite Modern Warfare 3, not
  Watch Dogs: Legion; 2.5 rewritten against the current zombie .cpp
  (`DriveTowardsTarget`, `ContactRange`, the direct-push fallback) with the
  fallback given a design home.
- **Resolved the silent contradictions.** The flow-field reference in
  reference-frame-notes.md (16.9); the Elizabeth-vs-Jubilee rolling-stock
  contradiction (4.11, 4.12, 16.10) as a Phase C blocker; the
  phase-letter-vs-number vocabulary clash (16.11), standardising on the A to G
  letters.
- **Added the 7 missing sections** (17 to 23): the framework backbone, the perf
  and crowd budget, anti-kite AI, onboarding, telemetry and balance
  methodology, a full audio architecture, and accessibility.
- **Re-labelled ~21 items as PROPOSAL** so the owner can see instantly which
  items are "yes, that is what I meant" versus "you are asking me to make a
  design decision".

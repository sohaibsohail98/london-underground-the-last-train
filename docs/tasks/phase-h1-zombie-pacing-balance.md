# Phase H1: zombie approach pacing and difficulty balance

Lane: Opus in the editor (CC-in-Unreal) for the data asset tuning and PIE
verification. Likely no `Source/` change: every field this task tunes already
exists as a `UPROPERTY` on `ULTZombieTypeData` or `ALTRoundManager`. If a
genuinely new mechanic turns out to be needed (a telegraph VFX, a distinct
spawn-to-visible delay), stop and write that up as its own small spec rather
than improvising it in Blueprint.

Depends on: nothing blocking, can run any time. Sequenced ahead of Phase H
(weapon presentation) and F5 to F7 per the 2026-09-10 reorder, because a
horde that is instant and unkillable makes every other polish pass hard to
judge honestly.

## What playtesting found, 2026-09-10

Two related complaints, both balance not missing systems:

1. **No build up.** Zombies go from "not visible" to "attacking" with too
   little warning: there is no sense of a horde approaching, just presence
   then contact.
2. **Too tanky.** Zombies are "not easy to kill" at a pace that reads as
   unbalanced this early in a round.

## Root cause, from reading the code

Nothing is missing. `ALTRoundManager` already exposes:

- `BaseSpawnIntervalSeconds` (1.6s), `SpawnIntervalDecay` (0.96 per spawn),
  `MinimumSpawnInterval` (0.35s), `MaximumAlive` (24), `BreatherSeconds`
  (10s between rounds). These four numbers alone set how many zombies exist
  at once and how fast they arrive: the "instant" complaint is very likely
  these defaults reading as too aggressive at round 1, not a missing
  telegraph mechanic.
- Zombie spawn points (`Rounds/LTSpawnPoint`) use weighted choice, but the
  weighting and placement relative to typical player sightlines has not been
  balance passed since Phase D's grey box placement. A spawn point close to
  or within the player's likely first sightline reads as "instant" even at a
  reasonable spawn interval.
- `ULTZombieTypeData` has `HealthMultiplier` per type, layered on top of
  `ApplyRoundScaling`'s own per-round health curve on `ALTZombieCharacter`.
  Nobody has tuned these since the five types were split out: check the base
  values on `ALTZombieCharacter` and the round scaling curve itself before
  assuming a data asset multiplier is the fix, the base numbers may simply be
  too high for a starting pistol (see `phase-h1-starting-loadout.md`) to feel
  fair against.

## Scope

1. **Read before touching anything.** `docs/design/gameplay-canon.md`
   section 6 has the authoritative and provisional zombie/round numbers, and
   states which are locked and which are free to retune. Do not change a
   number marked authoritative without flagging it in this task's handover
   row.
2. **Spawn pacing.** With a fresh PIE session on `L_CanaryWharf_Greybox`,
   play round 1 to round 3 and time, from round start, how long until the
   first zombie is both spawned and within the player's sightline from a
   natural standing position. Compare against the design intent in
   `gameplay-canon.md`. Retune `BaseSpawnIntervalSeconds` and spawn point
   placement/weighting if the gap between "round starts" and "first contact"
   is too short to read as a build up. This is the primary lever, try it
   before touching health or damage.
3. **Spawn point sightlines.** Check whether any weighted spawn point sits
   inside or very near the player's likely opening sightline (near the wall
   buy, near the platform centre) rather than from the tunnel mouths as the
   composition rules intend. A zombie appearing already close reads as
   "instant" regardless of the timer. Reweight or reposition, do not move
   any other gameplay actor while doing this (confirm by read-back).
4. **Time to kill.** With the starting weapon (the pistol from
   `phase-h1-starting-loadout.md`, or `DA_Weapon_SMG` if that spec has not
   landed yet), measure body shots and headshots to kill a round 1 walker.
   Compare against `docs/design/gameplay-canon.md`'s stated economy and
   damage intent. If walkers are absorbing noticeably more hits than the
   design states, the round scaling curve or `HealthMultiplier` is the
   likely culprit: retune the smallest number of values that brings round 1
   walkers back in line, do not rebalance every type and every round at
   once. Later rounds and the four other types are out of scope for this
   pass; only round 1 to 3 walker pacing and time to kill are in scope, so
   the fix is testable in one sitting.
5. **Verify, do not just retune and stop.** Play at least two full rounds
   after any change and confirm the "build up, then a fair fight" read
   holds, not just that a number moved.

## Rules

- Data asset and round manager property changes only. No `Source/` change
  expected.
- Never mutate an actor or asset while PIE is running: stop PIE cleanly
  first.
- `MAP CHECK` as a console exec is banned, use `MAP CHECKDEP NOCLEARLOG`.
- Confirm every gameplay actor's transform is unchanged if spawn points are
  repositioned rather than reweighted: read back before and after.

## Accept

- Round 1: a clear, readable gap between the round starting and the first
  zombie becoming visible and closing on the player, long enough to read as
  a build up, not instant contact.
- Round 1 walkers die in a number of hits that matches
  `docs/design/gameplay-canon.md`'s stated intent with the starting weapon.
- No spawn point sits inside the player's immediate opening sightline.
- Every gameplay actor confirmed unmoved unless the task explicitly
  repositioned a spawn point, in which case the new transform is recorded in
  the handover row.
- A full round plays and reads as fair: tense but not instant, tough but not
  a damage sponge.

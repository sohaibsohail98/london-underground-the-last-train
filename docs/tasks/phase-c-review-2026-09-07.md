# Phase C review pass, 2026-09-07

A code-only session: no editor, no PIE, no engine, so **nothing here has been
compiled**. Four bounded jobs, all against files NeoStack does not have open.

1. Review `phase-c-special-rounds.md` (`b1424bf`) against its own spec.
2. Work the open findings in `handover.md` that the specs put out of scope.
3. Give the corridor stall edge case a verdict.
4. Cross-check the five zombie stat blocks in `neostack.md` before they are
   entered.

Everything below is in the working tree. The first local build is still the gate,
per `handover.md`.

## 1. Special rounds, diff against the spec

`ALTRoundManager` carries the plan layer the spec asks for: `FLTRoundPlan`,
`BuildRoundPlan` once per round in `StartRound`, `PendingSpawns` set from the
plan, and a three-way branch in `ChooseTypeForSpawn` reached from `TrySpawnOne`
(guaranteed group, then forced single type, then the weighted roster).
`IsSpecialRound` and `GetSpecialRoundTag` are there, and `OnRoundStarted` kept
its signature. The special types are found on the roster by type id rather than
duplicated as `SprinterType` and `BruteType` properties, which is what the spec's
own "reuse whatever `phase-c-zombie-types.md` uses" line asks for.

### The two recorded deviations

**Brutes spread through the round, not grouped up front: sound, keep.** The spec
says spawn the pair first; `gameplay-canon.md` section 6 says "spawned at roughly
30% and 70% through the count". Canon wins, and the implementation is
self-scaling: brute *n* of *N* lands at `(n / (N + 1))` of the round's spawns, so
2 brutes sit at 33 and 67 per cent and a `BruteRoundBruteCount` of 3 would give
25, 50 and 75.

**A round that is both: the deviation is real but it is wider than the handover
recorded, and it was changed.** The handover describes it as rounds 20 and 30
becoming a sprinter round that also carries the pair. On the shipped intervals
that is not a special case for 20 and 30: **every** brute round is divisible by 5,
so round 10 was an all-sprinter round carrying two brutes. That contradicts three
places that all say the same thing:

- `gameplay-canon.md` section 6 lists the sprinter rounds as "5, 15, 25, ..." and
  calls every 10th round "a normal walker round plus 2 brutes".
- `phase-c-zombie-types.md` acceptance 5: "Round 10 spawns exactly two brutes on
  top of the normal walker round".
- `phase-c-special-rounds.md` acceptance: "Reach round 10: a normal walker round
  plus exactly 2 brutes", and its scope section asks for that precedence
  explicitly.

Only one sentence in canon reads the other way ("Rounds 20, 30, ... are both a
sprinter round and a brute pair"), and it contradicts canon's own sprinter list
two lines above it, since 20 is a multiple of 5 exactly as 10 is. There is no
arithmetic that makes 10 a walker round and 20 a sprinter round off two intervals,
so this is canon disagreeing with itself rather than a rule anyone can implement
as written.

Resolved as a tick rather than a hard coded rule:

```cpp
/** ... Set, the brute round wins and rounds 10, 20, 30 are a walker round plus
    the pair, which is what every acceptance list tests for. Clear it to stack
    the two instead. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
bool bBruteRoundOverridesSprinterRound = true;
```

Default `true`, which is the reading the acceptance lists test. Clearing it on
`BP_RoundManager` restores exactly the behaviour that was in `b1424bf`. **If the
intent really was a sprinter horde carrying two brutes on round 10, this is one
tick, and canon section 6 wants the contradiction resolved either way.**

### Bugs found and fixed in the same file

- **A brute could silently lose its slot.** `BuildRoundPlan` walks forward off the
  end of the round when two guaranteed fractions collide on one index, and an
  index of `TotalSpawns` is never reached by a spawn: `PendingSpawns` still
  counted the brute, so the round was the right length but one brute short and it
  arrived as an ordinary roster roll. Now it walks back from the end when there is
  no room ahead, and `GuaranteedCount` is set from the slots actually placed so
  the count cannot promise a brute the round has nowhere to put. Only bites on a
  very short round, which is exactly what a tester drops `OpeningRoundCounts` to.
- **The special-round banner outlived the run.** `StopRounds` left `CurrentPlan`
  populated, so `GetSpecialRoundTag` kept reporting "Sprinters" after death or
  boarding. It is reset there now, alongside `PendingScreamSpawns`.
- **The spawn log printed the wrong cap**: `MaximumAlive` rather than
  `GetEffectiveMaximumAlive()`, so with station heat up the diagnostic disagreed
  with the cap actually enforced one line earlier.

Not changed, and worth knowing: on a sprinter round the forced type bypasses
`MaxAliveOfThisType`, which is right while the sprinter has no cap, and a screamer
summon lands after the guaranteed indices are fixed, so a summon cannot displace a
brute.

## 2. The open findings from the merge reviews

`handover.md` "Open code items", the six that needed a file the specs had put out
of scope. Item 1 is section 3 below; item 5 is a design and editor decision, not
code, and is untouched.

| # | Finding | Done |
|---|---|---|
| 2 | `NavProjectionExtent` Z of 500 let a spawn point in the void "succeed" | A new `NavProjectionWarnDistance` (200) logs a warning naming the point and how far it snapped. That is the warning that never fired on the Canary Wharf points at world origin. The extent itself is left generous on purpose: section 5 below argues the tightening back out again. |
| 3 | Arrival flashed the points HUD as a spend | `ULTPointsComponent::SetPoints(int32)` assigns the total and broadcasts a **zero** delta. `RehydrateFromTravel` uses it instead of `AddPoints(Carried - Current)`. |
| 4 | A downed player still got interaction prompts | `ULTInteractionComponent::SetInteractionEnabled(bool)` stops the sweep and clears the live prompt on the way down, rather than freezing it on screen. `Down()` and `Die()` disable it, `Revive()` re-enables it, and `TryInteract` respects it too. |
| 6 | `ALTGameState::SetStationName` had no callers | `ALTGameMode` now carries `StationDisplayNames` (a map from map asset name to label, keyed like `StationRoutes` because one game mode Blueprint serves every station) plus a `StationDisplayName` fallback, stamped onto the game state in `BeginPlay`. With neither filled the label reads blank, as it always did, and says so once in the log. |
| 7 | `ALTTrain`'s class comment said travel was a later task | One line. It now says the game mode owns the travel. |
| 8 | `ALTPlayerCharacter::TakeDamage` subtracted the raw damage | It subtracts what `Super::TakeDamage` returns, and a hit a modifier absorbed entirely now returns 0 without restarting the regeneration delay. |

Editor follow-ups this creates: `StationDisplayNames` wants two entries on
`BP_GameMode` ("Greybox Test" and "Canary Wharf", or whatever the fiction settles
on) or the station label stays blank, and the HUD can now show
`GetStationName()`.

## 3. Corridor stall edge case: verdict

**It is a gate-order bug, and the gate is not in `UpdateStallRecovery`. It is the
repath in `Tick`.**

`BeginStallRecovery` cancels the AI move request on purpose, because path
following owns velocity while a request is live and would clobber the
`AddMovementInput` shove. Nothing stopped the next repath re-issuing it. Within at
most `RepathIntervalSeconds` (0.35s) of the shove starting, `DriveTowardsTarget`
called `MoveToActor` again, path following took velocity back, and the zombie
returned to the stall it had been shoved out of, for the rest of its life. The
nudge did fire; it never got to do anything. That matches the symptom exactly: a
zombie a short way outside `AttackRange` at zero velocity with recovery apparently
inert.

Two further faults in the same path, both able to hide the nudge on their own:

- **`StallTimer` could not accumulate.** It was reset to zero on any single frame
  above `StallSpeedThreshold`. A zombie pinned against another capsule twitches
  over 8 cm/s every few frames as the crowd shifts, so a 0.5s grace was never
  reached. It now decays by `DeltaSeconds` instead of resetting, so jitter costs
  the timer time rather than wiping it.
- **Recovery ended on one jittery frame.** The exit test was velocity alone, so
  the same twitch that reset the timer also ended the shove, straight back into
  the stall. Exit now needs the zombie to be moving **and** to have carried
  itself `StallRecoveryProgress` (40 units) from where the shove began.

Fixes, all in `ALTZombieCharacter`:

- `Tick` holds the repath back while `bStallRecovering`. `EndStallRecovery`
  already zeroes `RepathTimer`, so the move re-issues the moment the shove ends.
- `StallRecoverySeconds` (1.5) caps a single shove. Without a cap, suppressing the
  repath would leave a zombie pressed into geometry shoving for ever with RVO off.
  On a timeout it hands control back to path following, which can route around,
  and flips `StallLateralSign` so it leads with the other shoulder next time.
- `BeginStallRecovery` banks where the shove started; `EndStallRecovery` clears
  the clock.

What to watch in PIE, since none of this is compiled: a corridor queue should
visibly fan out rather than freeze, no zombie should sit still outside
`AttackRange` for more than about 2s, and `LogLastTrain` at Verbose should show
"entering stall recovery" repeating at most every couple of seconds for a given
zombie rather than every frame. Per `CLAUDE.md` this was a Fable-tier bug (it
survived one fix); if it survives this one, that is where it goes, with the note
that the repath gate is now closed so the remaining suspect is the nudge itself
being too weak against RVO neighbours that still have avoidance on.

## 4. The five stat blocks, data-entry cross-check

Read-only check of the `neostack.md` table against `gameplay-canon.md` section 6,
`phase-c-zombie-types.md` "Resolved values", and the actual `ULTZombieTypeData`
class. Verdict: **enter it as written**. All 28 field names in the table exist on
the class with exactly those names and types, one row per editable property with
none of the class's own left out, and every value traces to canon or to a
resolved value in the spec. No drift found. Five notes:

1. **Speeds are held as 2dp multipliers of `BaseWalkSpeed` 130**, so they land
   near, not on, canon's numbers: sprinter 3.85 gives 500.5, crawler 1.15 gives
   149.5, screamer 0.85 gives 110.5, brute 0.73 gives 94.9. Under a unit each,
   fine to enter. If Phase G wants exact speeds, the multiplier is the wrong place
   to hold them.
2. **Nothing sets the "others 90" capsule half height** from the spec's resolved
   list. The zombie constructor never resizes the capsule, so a 0 override leaves
   `ACharacter`'s 88, and `ALTRoundManager::SpawnCapsuleLift` lifts every spawn by
   90 regardless. A walker therefore arrives two units high and settles, which is
   harmless. The brute (130) and crawler (45) overrides shift the actor by their
   delta from 88, which is correct.
3. **Canon's sprinter "0.35s wind-up telegraph" cannot be entered anywhere.**
   There is no field for it and no code: `TryAttack` applies the damage in the
   same frame it calls `OnAttackWindUp`. `phase-c-zombie-types.md` deliberately
   defers the wind-up state machine, so this is a known gap rather than a
   data-entry error, but no combination of asset values creates the telegraph.
   Every type's swing lands instantly, not just the sprinter's.
4. **`bRagdollOnDeath` false on all five is right.** Canon's death behaviours ask
   for a settled pose rather than a full ragdoll; the brute's forward fall and its
   6 m shake are the Blueprint's job, off `DeathScreenShakeRadius` 600.
5. **Crawlers can now appear on round 10** alongside the brute pair, because the
   brute round is a normal mixed round under the default in section 1 above.
   Canon bars crawlers and screamers from the sprinter round specifically, and
   round 10 is a walker round in every acceptance list, so this reads correct. It
   is a visible change from `b1424bf`, where round 10 was all sprinters.

## 5. Review of the above, same session, fresh pass

Three findings against this pass's own work, all fixed in place.

- **The stall exit test was the wrong measurement.** It asked whether the zombie
  had closed 40 units on the target. A zombie that shoulders clear of the queue
  while the player runs away closes nothing, so it would have kept shoving for the
  full 1.5s with RVO off and the repath suppressed, then timed out and flipped its
  shoulder for no reason. It now measures the zombie's own displacement from where
  the shove began, which is what "broken free" actually means and is no less
  immune to jitter.
- **Tightening `NavProjectionExtent` on Z was a regression risk with no payoff.**
  At Z 150 a spawn point sitting 300 units above the floor stops resolving, and a
  failed projection spawns the zombie at the raw point with nothing under it: a
  point that used to work would start dropping zombies into the void. The finding
  it was meant to close was the **silence**, not the reach, and
  `NavProjectionWarnDistance` closes that on its own. The extent is back at 500.
- **The stat block count was wrong**: 28 fields and 28 rows, not 27. Corrected in
  section 4.

One thing to watch in PIE rather than change now: in a permanently jammed crowd a
zombie can cycle shove (up to 1.5s, RVO off) then path (0.5s grace) indefinitely,
so a large stuck crowd could sit with avoidance off most of the time and
interpenetrate visibly. That is the intended trade for breaking a pin, but if the
crowd reads as merging rather than queueing, `StallRecoverySeconds` is the dial.

## What this pass did not do

No compile, no PIE, no editor asset touched, no `neostack.md` edit, no acceptance
list signed off. `handover.md`'s open items are updated to match.

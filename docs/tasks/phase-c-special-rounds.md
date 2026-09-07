# Phase C - special rounds (sprinter round, brute round)

**C++ WRITTEN, NOT COMPILED** (`b1424bf`). `FLTRoundPlan`, `BuildRoundPlan`, the
three-way branch in `TrySpawnOne`, `IsSpecialRound` and `GetSpecialRoundTag` are
all in `ALTRoundManager`. All five CI gates pass; nothing has been compiled.
**Still open:** the roster wiring that makes it observable, per `neostack.md`.

Reviewed against the body below on 2026-09-07:
`docs/tasks/phase-c-review-2026-09-07.md`. One deviation stands: the brute pair
lands at roughly 30 and 70 per cent through the round rather than as a group up
front, because `docs/design/gameplay-canon.md` section 6 says so. The other was
reverted to this spec's precedence rule, because it went further than anyone had
recorded: every brute round is divisible by 5, so round 10 was an all-sprinter
round carrying two brutes. `bBruteRoundOverridesSprinterRound` now decides it,
default set, which is the reading this spec's acceptance list and both other
acceptance lists test. Clear it for a stacked round. The special types are looked
up on the roster by type id rather than duplicated as `SprinterType` and
`BruteType` properties, which is what this spec's own "reuse whatever
phase-c-zombie-types.md uses" instruction asks for. Body below is the original
spec.

**Engine:** Unreal Engine 5.8, macOS. Xcode on `/Volumes/DriveSohaib` mounted
(`xcode-select -p`). If not, STOP.

**Build command** (repo root):
```
"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"
```
`-Werror` on, no CI compile, compile locally. `clang-format` 20 on touched files.

**Model:** Opus. C++ against the existing convention.

**Prerequisite:** `phase-c-zombie-types.md` has landed - `ULTZombieTypeData` (or
whatever that task named it) exists, `ALTZombieCharacter` has an `ApplyTypeData`
path, and `ALTRoundManager` has a roster (a `TArray` of type entries with
weights and first-appearance rounds) with `ZombieClass` kept as the
empty-roster fallback. **If that task has NOT landed, STOP** - this one has
nothing to hook into. Read `phase-c-zombie-types.md` for the exact names it
introduced and use those, not the placeholders below.

## The problem

`docs/design/gameplay-canon.md` section 6 defines two special rounds and the
current `ALTRoundManager` has no concept of them:

- Every 5th round (5, 15, 25, ...): the whole roster is **sprinters**, count
  reduced to 75% of the normal curve. Crawlers and screamers do not appear.
- Every 10th round (10, 20, ...): a normal walker round **plus 2 brutes**
  spawned as a pair.

`ALTRoundManager::StartRound` today just computes a count and spawns from the
roster by weight every round, identically. There is no per-round roster override,
no count modifier, no "spawn N of a specific type up front" path.

This task adds a small special-round layer to `ALTRoundManager`: on
`StartRound`, decide if this round is special, and if so override the roster
selection and count for the round, and optionally queue a fixed set of "guaranteed"
spawns (the brute pair). It is data-driven off `EditDefaultsOnly` properties so
the cadence and the multipliers stay tuneable.

## Scope

**In scope**
- On `ALTRoundManager`, `EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds"`:
  ```cpp
  /** Every Nth round is an all-sprinter round. 0 disables. */
  int32 SprinterRoundInterval = 5;

  /** Sprinter rounds spawn this fraction of the normal round count. */
  float SprinterRoundCountScale = 0.75f;

  /** Every Nth round adds a fixed group of brutes on top of a normal round.
      0 disables. */
  int32 BruteRoundInterval = 10;

  /** How many brutes the brute round adds, spawned as a group early in the round. */
  int32 BruteRoundCount = 2;
  ```
- Two `TSubclassOf` or type-data references so the special rounds know which type
  to force. Reuse whatever `phase-c-zombie-types.md` uses to identify a type:
  if types are `ULTZombieTypeData` assets, add
  `TObjectPtr<ULTZombieTypeData> SprinterType` and
  `TObjectPtr<ULTZombieTypeData> BruteType` as `EditDefaultsOnly`. If types are
  subclasses, use `TSubclassOf<ALTZombieCharacter>`. Match the existing model.
- A private helper `struct FLTRoundPlan` computed once per round in `StartRound`:
  ```cpp
  struct FLTRoundPlan
  {
      int32 TotalCount = 0;
      bool bForceSingleType = false;
      const ULTZombieTypeData* ForcedType = nullptr;   // or TSubclassOf
      int32 GuaranteedTypeCount = 0;                    // brutes to spawn as a group
      const ULTZombieTypeData* GuaranteedType = nullptr;
  };
  FLTRoundPlan BuildRoundPlan(int32 Round) const;
  ```
  - Normal round: `TotalCount = ComputeRoundCount(Round)`, no forcing.
  - Sprinter round (`SprinterRoundInterval > 0 && Round % SprinterRoundInterval
    == 0`): `TotalCount = FMath::Max(1, FMath::RoundToInt(ComputeRoundCount(Round)
    * SprinterRoundCountScale))`, `bForceSingleType = true`, `ForcedType =
    SprinterType`.
  - Brute round (`BruteRoundInterval > 0 && Round % BruteRoundInterval == 0`):
    `TotalCount = ComputeRoundCount(Round)` (normal walkers),
    `GuaranteedTypeCount = BruteRoundCount`, `GuaranteedType = BruteType`. The
    guaranteed brutes are IN ADDITION to `TotalCount`, so `PendingSpawns` for
    the round is `TotalCount + GuaranteedTypeCount`.
  - If a round is somehow both (e.g. round 10 with interval 5 and 10): the
    brute-round rule wins for the guaranteed group, and the base round stays
    walkers, not sprinters. Round 10 is a brute round, not a sprinter round.
    Encode this precedence explicitly.
- `StartRound` uses the plan: set `PendingSpawns` from the plan, stash the plan
  (a member `FLTRoundPlan CurrentPlan`) for `TrySpawnOne` to read.
- `TrySpawnOne` respects the plan:
  - If `CurrentPlan.GuaranteedTypeCount > 0` and not yet all placed, this spawn
    is a `GuaranteedType` (decrement a counter). Spawn the guaranteed group
    first, at the start of the round.
  - Else if `CurrentPlan.bForceSingleType`, this spawn is `ForcedType`,
    bypassing the weighted roster roll.
  - Else, the existing weighted roster selection.
  - Whichever type is chosen, apply it via the `ApplyTypeData` (or subclass
    spawn) path `phase-c-zombie-types.md` established.
- A `BlueprintImplementableEvent` or a field on the `OnRoundStarted` broadcast so
  the HUD can show "SPRINTERS" / "BRUTES". Simplest: add
  `UFUNCTION(BlueprintPure) bool IsSpecialRound() const` and
  `UFUNCTION(BlueprintPure) FName GetSpecialRoundTag() const` (returns
  `"Sprinters"`, `"Brutes"`, or `NAME_None`) reading `CurrentPlan`. Do not change
  the `OnRoundStarted` delegate signature.

**Out of scope**
- Screamer and crawler special behaviour, the "roster pressure from heat 3+"
  weight shift (`gameplay-canon.md` line 127) - separate task.
- The HUD banner itself. This task exposes `GetSpecialRoundTag`; the widget is
  Blueprint.
- Rebalancing the type stats. Numbers come from `phase-c-zombie-types.md`.
- Any change to `ALTZombieCharacter` beyond calling its existing `ApplyTypeData`.
  If you need a new method on it, STOP and report.
- Changing `ComputeRoundCount` / `ComputeSpawnInterval`. The special layer sits
  on top of them.

## Constraints

- Edit only `Source/LastTrain/Public/Rounds/LTRoundManager.h` and
  `Source/LastTrain/Private/Rounds/LTRoundManager.cpp`. No new files. No change to
  `LTZombieCharacter`, `LTSpawnPoint`, `LTStationHeat`, or the zombie type data
  asset class. If the task appears to need one, STOP and report.
- `FLTRoundPlan` is a plain internal struct (no `USTRUCT` needed unless you want
  it inspectable; `USTRUCT()` without `BlueprintType` is fine and cheap).
- British spelling everywhere including comments and log strings. No em or en
  dashes.
- `#pragma once` first, `*.generated.h` last. `LT_LOG` not `UE_LOG`. `TObjectPtr`
  for the `SprinterType` / `BruteType` members.
- No `TODO`/`FIXME`/`HACK`/`XXX` markers.
- `LT` prefix, tab indent, clang-format 20.
- Do NOT run git.

## Acceptance

Static:
1. Compiles clean, zero warnings under `-Werror`.
2. `clang-format -i` clean on both files.
3. All four `tools/ci/` checkers pass.
4. Read the diff: `StartRound` sets `PendingSpawns` from `BuildRoundPlan`, and
   `TrySpawnOne` has a clear three-way branch (guaranteed group, then forced
   single type, then weighted roster). Round 10 precedence (brutes, not
   sprinters) is explicit in `BuildRoundPlan`.

Runtime (editor task, later, note it): with `SprinterType` and `BruteType` set on
the round manager, PIE and reach round 5 (drop `OpeningRoundCounts` and the
breather right down for testing): the whole round should be sprinters at ~75%
count. Reach round 10: a normal walker round plus exactly 2 brutes spawned early.
`GetSpecialRoundTag()` should read `"Sprinters"` on 5, `"Brutes"` on 10, none
otherwise.

## On pass

Update `docs/tasks/handover.md` and the Phase C row in `docs/tasks/README.md`. Note
the remaining special-behaviour work (screamer alert, crawler low profile, the
heat-3+ roster-pressure weight shift) as follow-ups.

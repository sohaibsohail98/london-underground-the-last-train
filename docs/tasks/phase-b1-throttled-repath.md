# Phase B1 — throttled zombie repath

**Engine:** Unreal Engine 5.8, macOS, external Xcode on `/Volumes/DriveSohaib`
mounted. Compile after the change with the batch build in `CLAUDE.md`.

**Model:** Opus. C++ against the existing convention.

**Prerequisite:** Phase A passed. `L_GreyboxTest` plays and a zombie walks toward
the player.

## The problem

`ALTZombieCharacter::Tick` in
`Source/LastTrain/Private/Zombies/LTZombieCharacter.cpp` calls
`AI->MoveToActor(CurrentTarget, AttackRange * 0.75f)` on every tick, for every
live zombie:

```cpp
if (AAIController* AI = Cast<AAIController>(GetController()))
{
	AI->MoveToActor(CurrentTarget, AttackRange * 0.75f);
}
```

Each `MoveToActor` issues a fresh pathfind request. At the Phase B target of 24
to 40 zombies that is 24 to 40 full navmesh queries per frame, all in lockstep,
which is the load the Phase B gate measures. It also thrashes the path following
component, since a new request pre-empts the one in flight before the character
has moved along it.

## The change

Throttle the repath to a fixed cadence, with per-instance jitter so the queries
spread across frames instead of spiking on one. Between repaths the existing
path following continues to steer the character, so movement stays smooth.

### `LTZombieCharacter.h`

Add, in the `Combat` category with the other tunables:

```cpp
/** Seconds between navmesh repath requests. Path following steers between them. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
float RepathIntervalSeconds = 0.35f;

/** Fraction of the interval added as a random per-instance offset, so repaths
    across the crowd do not land on the same frame. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
float RepathJitterFraction = 0.4f;
```

Add to the private state block, next to `AttackCooldown`:

```cpp
float RepathTimer = 0.f;
```

### `LTZombieCharacter.cpp`

In `BeginPlay`, after `CurrentTarget` is set, seed the timer with a random
offset so instances start out of phase:

```cpp
RepathTimer = FMath::FRandRange(0.f, RepathIntervalSeconds);
```

In `Tick`, replace the unconditional `MoveToActor` block with a timed one. Keep
the target refresh and `TryAttack` exactly as they are:

```cpp
RepathTimer -= DeltaSeconds;
if (RepathTimer <= 0.f)
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->MoveToActor(CurrentTarget, AttackRange * 0.75f);
	}

	const float Jitter = RepathIntervalSeconds * RepathJitterFraction;
	RepathTimer = RepathIntervalSeconds + FMath::FRandRange(-Jitter, Jitter);
}
```

Do not change `TryAttack`, `ReceiveShot`, `Die`, or the movement component
setup. `bUseRVOAvoidance` already handles local separation between repaths.

## Constraints

- British spelling, no em or en dashes, `LT_LOG` not `UE_LOG`, tab indent,
  `X.generated.h` last. Match the file.
- No new includes needed. `FMath` is already available through `CoreMinimal`.
- Keep the diff to the two files above.

## Acceptance

1. Compiles clean with the batch build.
2. In `L_GreyboxTest`, a single zombie still walks to the player and attacks as
   before. No visible hitch or stutter in its path.
3. Raise the round manager `OpeningRoundCounts` first entry to 30, or let rounds
   climb, so 24 or more are alive at once. With `stat unit` open, the game
   thread time is materially lower than on the pre-change build at the same
   count, and the frame rate holds at or near 60fps on the grey box.
4. Zombies still converge on the player and do not walk into walls or freeze at
   spawn.

## On pass

Update `docs/tasks/NEXT.md`. Next Phase B task is the interaction system
(`Interaction/LTInteractableInterface` has no implementation or caller yet):
write `docs/tasks/phase-b2-interaction.md`.

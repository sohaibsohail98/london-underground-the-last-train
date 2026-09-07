# Phase C3 - travel between stations

**C++ WRITTEN, NOT COMPILED** (`64535f6`, hardened in `b1424bf`).
`ULTGameInstance` and the `ALTGameMode` travel path are in, along with the
`GameInstanceClass` line in `Config/DefaultEngine.ini`. All five CI gates pass;
nothing has been compiled. **Still open:** reparent `BP_GameMode` to
`ALTGameMode` and fill `StationRoutes`, per `neostack.md`, then run the boarding
test. Divergences from the body below, all deliberate: `StationRoutes` (a map of
this map's name to the destination's) was added alongside `NextStationMap`,
because one shared game mode Blueprint cannot hold two destinations; travel is
deferred by `TravelDelaySeconds` so the train's boarding hooks have frames; the
rehydrate runs on the first tick after `StartRun` and retries for a few ticks
rather than dropping the carry if the pawn is not possessed yet; and an unclaimed
payload is dropped one map load on. The reserve carries to full, the compromise
the spec pre-authorised. Body below is the original spec.

**Engine:** Unreal Engine 5.8, macOS. Xcode on `/Volumes/DriveSohaib` must be
mounted (`xcode-select -p`). If not, STOP.

**Build command** (repo root):
```
"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"
```
`-Werror` on, no CI compile, compile locally. `clang-format` 20 on every `.h`/`.cpp`
touched (see `phase-c2-departure-board.md` for the venv install line).

**Model:** Opus. C++ against the existing convention, plus one new
`UGameInstance` subclass and level-load flow.

**Prerequisite:** Phase C1 landed. `ALTGameMode::NotifyPlayerBoarded(AActor*
Boarder)` exists and currently does the in-arena part only: stops rounds, refills
the weapon reserve, resets `ULTStationHeat`, sets `ELTRunState::Boarded`, and
logs "Travel to next station is not wired yet." `ELTRunState` already has a
`Boarded` value. `ALTTrain::TryBoard` calls `NotifyPlayerBoarded` with no
destination argument (C1 resolved the station-select UI as out of scope, "the
seam moved").

## The problem

`docs/brief-v2.md` line 406: "Boarding during dwell ends the round, banks points,
refills ammo and travels to a chosen adjacent station ... heat resets on travel.
Travel is bidirectional and free." C1 built everything up to "travels to". The
run sits in `Boarded` forever. There is no level transition, no carry-over of
points or the held weapon into the next arena, no `UGameInstance` to hold that
payload across a level load, and no destination choice.

This task builds the travel transition and the cross-level persistence for a
**two-station v1** (`README.md`: "v1 ships 2 stations"). It does not build a
station-select radial or a map screen; the destination is "the other station",
picked automatically, because with two stations there is only one other.

## Scope

**In scope**
- `ULTGameInstance`, a `UGameInstance` subclass, holding a travel payload:
  carried points, the held `ULTWeaponData` and its current reserve, the accrued
  visited-station list, and a `bTravelling` flag so the destination arena knows
  it is a hop, not a fresh start.
- `ULTGameInstance::BeginStationTravel(const FLTTravelPayload& Payload, FName
  DestinationMap)` - stores the payload, then `UGameplayStatics::OpenLevel`.
- `ALTGameMode::NotifyPlayerBoarded` extended: after the existing in-arena work,
  build the payload from the boarder's components and call
  `BeginStationTravel`. The destination is resolved from a new
  `EditDefaultsOnly` `TMap<FName, FName>` on the game mode (this map's asset name
  -> the other map's asset name), or a single `NextStationMap` `FName` if you
  prefer the simpler two-station form. Two-station form is fine and preferred.
- On the destination arena's `ALTGameMode::BeginPlay` (or `StartRun`): if the
  game instance reports `bTravelling`, rehydrate - grant the carried points to
  the player's `ULTPointsComponent`, set the carried weapon and reserve on the
  `ULTWeaponComponent`, clear `bTravelling`, and start rounds at round 1 with
  heat at 0 (heat resets on travel, per the brief). If not travelling, behave
  exactly as today (a cold start).
- A `FLTTravelPayload` USTRUCT, `BlueprintType`, in the game instance header or
  its own small header.

**Out of scope**
- A station-select UI, a map screen, a destination radial. Two stations, one
  "other", auto-resolved. Note where the picker would hook in.
- More than two stations, tiers, the 3-station brief roster, the 41-station
  aspirational list. `TMap` form is mentioned only as the >2 future shape.
- A loading screen widget. `OpenLevel` with a black frame is acceptable for v1;
  expose a `BlueprintImplementableEvent` `OnTravelStarted` the game instance
  fires just before `OpenLevel` so a Blueprint can fade out, and let the
  destination arena's existing BeginPlay flow fade back in.
- Saving to disk. The payload lives in memory in the game instance for the
  session only. A real save game is a later task.
- Any change to `ALTTrain`, `ULTStationHeat`, `ULTPointsComponent`,
  `ULTWeaponComponent` public surface. Read their existing getters/setters. If a
  needed setter genuinely does not exist, STOP and report which one, do not add
  it as a side quest.

## What already exists to build on

- `ULTPointsComponent` (`Economy/LTPointsComponent.h`): `int32 GetPoints() const`
  reads the balance, `void AddPoints(int32 Amount)` grants it, `StartingPoints`
  defaults to 500 and is applied in `BeginPlay`. So the carried balance is
  `GetPoints()` on the way out and `AddPoints(Payload.CarriedPoints)` on the way
  in. Note: `BeginPlay` sets the balance to `StartingPoints`, so the rehydrate
  must run AFTER the pawn's components have had `BeginPlay` (it will, if you
  rehydrate in the game mode's `StartRun` rather than its `BeginPlay`); if you
  see 500 + carried instead of just carried, that ordering is why - subtract
  `StartingPoints` or set the pawn's `StartingPoints` to 0 for the travel case.
  Simplest: in the rehydrate, `AddPoints(Payload.CarriedPoints - Points->GetPoints())`
  so the final balance is exactly the carried amount regardless of the 500 seed.
- `ULTWeaponComponent` (`Weapons/LTWeaponComponent.h`): `SetWeapon(ULTWeaponData*
  NewWeapon, bool bRefillReserve)`, `RefillAmmunition()`, `GetReserve()`, and a
  public `WeaponData` member. Setting the carried weapon is
  `SetWeapon(Carried, false)` then, if you need the exact reserve rather than a
  refill, whatever setter exists. If only `RefillAmmunition` exists, carrying "to
  full" is acceptable for v1 - note the compromise in a comment.
- `ULTStationHeat` (`Rounds/LTStationHeat.h`): `ResetHeat()`, `GetHeat()`.
- `ALTGameMode`: `StartRun()`, `NotifyPlayerBoarded(AActor*)`, private
  `FindRoundManager()`, `FindStationHeat()`, `bRunStarted`, `SetState()`.
- `ALTGameState`: `SetRunState(ELTRunState)`, `GetRunState()`.

## The change, in order, compile between each

### 1. `FLTTravelPayload` and `ULTGameInstance`

New files:
- `Source/LastTrain/Public/Core/LTGameInstance.h`
- `Source/LastTrain/Private/Core/LTGameInstance.cpp`

```cpp
USTRUCT(BlueprintType)
struct FLTTravelPayload
{
	GENERATED_BODY()

	/** Points the player leaves the last station with. */
	UPROPERTY(BlueprintReadOnly) int32 CarriedPoints = 0;

	/** The weapon held on boarding. Null means the destination uses its default. */
	UPROPERTY(BlueprintReadOnly) TObjectPtr<class ULTWeaponData> CarriedWeapon = nullptr;

	/** Reserve ammunition count for CarriedWeapon at the moment of boarding. */
	UPROPERTY(BlueprintReadOnly) int32 CarriedReserve = 0;

	/** Asset names of stations visited this run, oldest first, including the one
		just left. For a future "no immediate backtrack" rule and a run summary. */
	UPROPERTY(BlueprintReadOnly) TArray<FName> VisitedStations;
};
```

`ULTGameInstance`:
- `UPROPERTY() FLTTravelPayload PendingPayload;`
- `UPROPERTY() bool bTravelling = false;`
- `UFUNCTION(BlueprintCallable) void BeginStationTravel(const FLTTravelPayload&
  Payload, FName DestinationMap);` - copies `Payload` to `PendingPayload`, sets
  `bTravelling = true`, fires `OnTravelStarted` (a
  `BlueprintImplementableEvent`), then
  `UGameplayStatics::OpenLevel(this, DestinationMap)`.
- `UFUNCTION(BlueprintPure) bool IsTravelling() const { return bTravelling; }`
- `UFUNCTION(BlueprintCallable) FLTTravelPayload ConsumePayload();` - returns
  `PendingPayload` and sets `bTravelling = false`. The game mode calls this once
  on the destination side.

Register it: set `GameInstance` class in `Config/DefaultEngine.ini` under
`[/Script/EngineSettings.GameMapsSettings]` `GameInstanceClass=/Script/LastTrain.LTGameInstance`.
That ini edit is the one config change this task makes. Note it in the acceptance
report so a human can confirm it took.

### 2. `ALTGameMode::NotifyPlayerBoarded` - build and send the payload

After the existing in-arena work (rounds stopped, reserve refilled, heat reset,
state Boarded, log line), add:

```cpp
ULTGameInstance* GI = GetGameInstance<ULTGameInstance>();
if (!GI)
{
	LT_LOG(Warning, TEXT("NotifyPlayerBoarded with no ULTGameInstance, cannot travel."));
	return;
}

FLTTravelPayload Payload;
if (const ULTPointsComponent* Points = Boarder ? Boarder->FindComponentByClass<ULTPointsComponent>() : nullptr)
{
	Payload.CarriedPoints = <the existing current-points getter>;
}
if (const ULTWeaponComponent* Weapon = Boarder ? Boarder->FindComponentByClass<ULTWeaponComponent>() : nullptr)
{
	Payload.CarriedWeapon = Weapon->WeaponData;
	Payload.CarriedReserve = Weapon->GetReserve();
}
Payload.VisitedStations = <game instance's running list, plus this map's name>;

const FName Destination = NextStationMap;   // EditDefaultsOnly FName on the game mode
if (Destination.IsNone())
{
	LT_LOG(Warning, TEXT("NotifyPlayerBoarded: NextStationMap is unset, staying put."));
	return;
}

GI->BeginStationTravel(Payload, Destination);
```

Change the existing `RefillAmmunition()` line: with travel wired, the reserve is
carried and rehydrated on the far side, so the mid-air refill here is redundant.
Leave it - it is harmless and covers the "no game instance" path. A comment
noting it is now belt-and-braces is enough.

### 3. Rehydrate on the destination side

In `ALTGameMode::StartRun` (or `BeginPlay` before `StartRun`), before the normal
`SetState(Active)` / `BeginRounds()`:

```cpp
if (ULTGameInstance* GI = GetGameInstance<ULTGameInstance>())
{
	if (GI->IsTravelling())
	{
		const FLTTravelPayload Payload = GI->ConsumePayload();

		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (ULTPointsComponent* Points = PlayerPawn->FindComponentByClass<ULTPointsComponent>())
			{
				<grant Payload.CarriedPoints via the existing add path>;
			}
			if (ULTWeaponComponent* Weapon = PlayerPawn->FindComponentByClass<ULTWeaponComponent>())
			{
				if (Payload.CarriedWeapon)
				{
					Weapon->SetWeapon(Payload.CarriedWeapon, false);
					<set reserve to Payload.CarriedReserve if a setter exists, else RefillAmmunition and note the compromise>;
				}
			}
		}

		if (ULTStationHeat* Heat = FindStationHeat())
		{
			Heat->ResetHeat();   // heat resets on travel, brief-v2
		}

		LT_LOG(Log, TEXT("Arrived by train. Points %d, weapon %s, reserve %d carried. Rounds from 1, heat 0."),
			Payload.CarriedPoints,
			Payload.CarriedWeapon ? *Payload.CarriedWeapon->GetName() : TEXT("none"),
			Payload.CarriedReserve);
	}
}
```

Rounds still start at 1: v1 does not carry round number across stations (the
brief does not ask for it, and "rounds 1 to 10 play untouched" is the C gate).
Note this as a design choice in the log and the spec update.

## Constraints

- New files: `LTGameInstance.{h,cpp}` in `Core/`. Edits allowed only in
  `ALTGameMode.{h,cpp}` and `Config/DefaultEngine.ini` (the `GameInstanceClass`
  line). No change to `ALTTrain`, `ALTGameState`, `ULTStationHeat`,
  `ULTPointsComponent`, `ULTWeaponComponent`, `ALTRoundManager` public surface.
  If you need a new getter or setter on one of those, STOP and report exactly
  which, with the reason.
- British spelling everywhere including comments and log strings. No em or en
  dashes.
- `#pragma once` first, `*.generated.h` last include. `LT_LOG` not `UE_LOG`.
  `TObjectPtr` for all `UObject` members and struct fields.
- No `TODO`/`FIXME`/`HACK`/`XXX` markers.
- `LT` prefix, tab indent, clang-format 20.
- Do NOT run git. Leave the tree for review.

## Acceptance

Static:
1. Compiles clean, zero warnings under `-Werror`.
2. `clang-format -i` clean on all touched `.h`/`.cpp`.
3. All four `tools/ci/` checkers pass.
4. `Config/DefaultEngine.ini` has the `GameInstanceClass` line and it points at
   `/Script/LastTrain.LTGameInstance`. State this explicitly in the report.
5. Header review: guards, generated.h last, `TObjectPtr`, no `UE_LOG`, British
   spelling, no dashes, no markers.
6. If any "STOP and report" condition was hit (a missing points-add path, a
   missing reserve setter), the report says so and the task was NOT worked
   around.

Runtime (editor task, later, note it): with `NextStationMap` set to
`L_CanaryWharf_Greybox` on the `L_GreyboxTest` game mode (and vice versa), board
the train in one arena; the level should load the other, the player should keep
their points and weapon, reserve should be the carried amount (or full, if that
compromise was taken), heat should read 0, rounds should start at 1.

## On pass

Update `docs/tasks/handover.md` and the Phase C row in `docs/tasks/README.md`. Record
the two design choices made here: rounds restart at 1 on travel, and (if taken)
reserve carries "to full" rather than exact. Note the station-select picker as
the hook point for a >2-station future, and that a disk save game is a separate
later task.

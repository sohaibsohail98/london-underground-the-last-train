#include "Core/LTGameMode.h"

#include "Core/LTGameInstance.h"
#include "Core/LTGameState.h"
#include "Economy/LTPointsComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "Rounds/LTRoundManager.h"
#include "Rounds/LTStationHeat.h"
#include "TimerManager.h"
#include "Weapons/LTWeaponComponent.h"
#include "Weapons/LTWeaponData.h"

namespace
{
	/** Ticks the game mode will wait for a player pawn before giving up on
		delivering a travel payload. Ten frames is far longer than a possess takes. */
	constexpr int32 MaximumRehydrateAttempts = 10;
} // namespace

ALTGameMode::ALTGameMode()
{
	GameStateClass = ALTGameState::StaticClass();
}

void ALTGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartRun();
	}
}

void ALTGameMode::StartRun()
{
	if (bRunStarted)
	{
		return;
	}

	bRunStarted = true;

	SetState(ELTRunState::Active);

	if (ALTRoundManager* Rounds = FindRoundManager())
	{
		Rounds->BeginRounds();
	}
	else
	{
		LT_LOG(Warning, TEXT("Game mode found no round manager in the level. Rounds will not start."));
	}

	if (ULTGameInstance* GameInstance = GetGameInstance<ULTGameInstance>())
	{
		if (GameInstance->IsTravelling())
		{
			// A train arrival carries points and a weapon into this arena. The
			// pawn's own components stamp their starting values in their BeginPlay,
			// and that order against the game mode's is not guaranteed, so the carry
			// lands next tick where it cannot be overwritten.
			RehydrateAttempts = 0;
			GetWorldTimerManager().SetTimerForNextTick(this, &ALTGameMode::RehydrateFromTravel);
		}
		else
		{
			// A cold start is a new run, so any station history from a previous run
			// in this session goes.
			GameInstance->ClearRunHistory();
		}
	}
}

void ALTGameMode::RehydrateFromTravel()
{
	ULTGameInstance* GameInstance = GetGameInstance<ULTGameInstance>();
	if (!GameInstance || !GameInstance->IsTravelling())
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		// Do not consume a payload that cannot be delivered. The pawn is normally
		// possessed well before now, but a late possess must not cost the player
		// their carry, so try again for a few more ticks.
		RehydrateAttempts += 1;

		if (RehydrateAttempts <= MaximumRehydrateAttempts)
		{
			GetWorldTimerManager().SetTimerForNextTick(this, &ALTGameMode::RehydrateFromTravel);
			return;
		}

		LT_LOG(Warning, TEXT("Arrived by train but no player pawn appeared. The carry stays pending."));
		return;
	}

	const FLTTravelPayload Payload = GameInstance->ConsumePayload();

	if (ULTPointsComponent* Points = PlayerPawn->FindComponentByClass<ULTPointsComponent>())
	{
		// Land on exactly the carried balance whatever the component seeded itself
		// with, so the 500 opening float is not paid a second time. This does show
		// on the HUD as a points delta, which reads as a spend when the carry is
		// under the 500 seed: a SetPoints on ULTPointsComponent would fix it, and
		// that component is out of scope for this task.
		Points->AddPoints(Payload.CarriedPoints - Points->GetPoints());
	}

	if (ULTWeaponComponent* Weapon = PlayerPawn->FindComponentByClass<ULTWeaponComponent>())
	{
		if (Payload.CarriedWeapon)
		{
			// The reserve carries to full, not to the exact count the payload
			// recorded: ULTWeaponComponent has no reserve setter, only
			// RefillAmmunition. CarriedReserve is banked for when one lands.
			Weapon->SetWeapon(Payload.CarriedWeapon, true);
		}
	}

	// Heat resets on travel, per brief-v2. A fresh arena starts at zero anyway,
	// so this is belt and braces against a persistent heat component.
	if (ULTStationHeat* Heat = FindStationHeat())
	{
		Heat->ResetHeat();
	}

	// Rounds restart at 1 on travel: v1 does not carry the round number across
	// stations, and "rounds 1 to 10 play untouched" is the Phase C gate.
	LT_LOG(
		Log,
		TEXT("Arrived by train. Carried %d points and weapon %s, reserve refilled to full. Rounds from 1, heat 0."),
		Payload.CarriedPoints, Payload.CarriedWeapon ? *Payload.CarriedWeapon->GetName() : TEXT("none"));
}

void ALTGameMode::NotifyPlayerDied()
{
	SetState(ELTRunState::Dead);

	if (ALTRoundManager* Rounds = FindRoundManager())
	{
		Rounds->StopRounds();
	}
}

void ALTGameMode::NotifyPlayerDowned()
{
	SetState(ELTRunState::Downed);
}

void ALTGameMode::NotifyPlayerRevived()
{
	if (const ALTGameState* State = GetGameState<ALTGameState>())
	{
		if (State->GetRunState() == ELTRunState::Downed)
		{
			SetState(ELTRunState::Active);
		}
	}
}

void ALTGameMode::NotifyPlayerBoarded(AActor* Boarder)
{
	if (!bRunStarted)
	{
		return;
	}

	if (const ALTGameState* State = GetGameState<ALTGameState>())
	{
		const ELTRunState RunState = State->GetRunState();
		if (RunState == ELTRunState::Dead || RunState == ELTRunState::Boarded)
		{
			return;
		}
	}

	if (ALTRoundManager* Rounds = FindRoundManager())
	{
		Rounds->StopRounds();
	}

	// Snapshot the carry before the refill below, so CarriedReserve records what
	// the player actually boarded with rather than a full magazine.
	FLTTravelPayload Payload;

	if (const ULTPointsComponent* Points = Boarder ? Boarder->FindComponentByClass<ULTPointsComponent>() : nullptr)
	{
		Payload.CarriedPoints = Points->GetPoints();
	}

	if (const ULTWeaponComponent* HeldWeapon = Boarder ? Boarder->FindComponentByClass<ULTWeaponComponent>() : nullptr)
	{
		Payload.CarriedWeapon = HeldWeapon->WeaponData;
		Payload.CarriedReserve = HeldWeapon->GetReserve();
	}

	// Reserve back to full, magazine untouched, per RefillAmmunition. Belt and
	// braces now that travel rehydrates the reserve on the far side: this is what
	// the player gets if there is no game instance to travel with.
	if (ULTWeaponComponent* Weapon = Boarder ? Boarder->FindComponentByClass<ULTWeaponComponent>() : nullptr)
	{
		Weapon->RefillAmmunition();
	}

	// brief-v2's "banks points" resolved to a no-op: there is no points-at-risk
	// mechanic, points are never lost, so boarding does nothing to the points
	// component. A points to credit conversion would be a separate meta-economy task.

	if (ULTStationHeat* Heat = FindStationHeat())
	{
		Heat->ResetHeat();
	}

	SetState(ELTRunState::Boarded);

	LT_LOG(Log, TEXT("Player boarded. Run state Boarded, rounds stopped, reserve refilled, heat reset."));

	ULTGameInstance* GameInstance = GetGameInstance<ULTGameInstance>();
	if (!GameInstance)
	{
		LT_LOG(Warning, TEXT("NotifyPlayerBoarded with no ULTGameInstance, cannot travel."));
		return;
	}

	const FName Destination = ResolveDestinationMap();
	if (Destination.IsNone())
	{
		// The arena is now finished: rounds stopped, state Boarded, the train
		// frozen. Nothing more happens here, so a station meant to be travelled
		// from needs a route or a NextStationMap.
		LT_LOG(
			Warning,
			TEXT(
				"Player boarded but this station has no destination. Set StationRoutes or NextStationMap on the "
				"game mode."));
		return;
	}

	Payload.VisitedStations = GameInstance->GetVisitedStations();
	Payload.VisitedStations.Add(FName(*UGameplayStatics::GetCurrentLevelName(this, true)));

	PendingTravelPayload = Payload;
	PendingTravelDestination = Destination;

	// Let the train's boarding hooks and a fade have their frames. The run state is
	// already Boarded, so rounds are stopped and the train is frozen for the wait.
	if (TravelDelaySeconds > 0.f)
	{
		GetWorldTimerManager().SetTimer(TravelTimer, this, &ALTGameMode::BeginPendingTravel, TravelDelaySeconds, false);
	}
	else
	{
		BeginPendingTravel();
	}
}

void ALTGameMode::BeginPendingTravel()
{
	if (PendingTravelDestination.IsNone())
	{
		return;
	}

	ULTGameInstance* GameInstance = GetGameInstance<ULTGameInstance>();
	if (!GameInstance)
	{
		LT_LOG(Warning, TEXT("Travel timer fired with no ULTGameInstance. Staying put."));
		return;
	}

	const FLTTravelPayload Payload = PendingTravelPayload;
	const FName Destination = PendingTravelDestination;

	PendingTravelPayload = FLTTravelPayload();
	PendingTravelDestination = NAME_None;

	GameInstance->BeginStationTravel(Payload, Destination);
}

FName ALTGameMode::ResolveDestinationMap() const
{
	const FName ThisMap(*UGameplayStatics::GetCurrentLevelName(this, true));

	if (const FName* Routed = StationRoutes.Find(ThisMap))
	{
		if (!Routed->IsNone())
		{
			return *Routed;
		}
	}

	return NextStationMap;
}

void ALTGameMode::SetState(const ELTRunState NewState)
{
	if (ALTGameState* State = GetGameState<ALTGameState>())
	{
		State->SetRunState(NewState);
	}
}

ALTRoundManager* ALTGameMode::FindRoundManager() const
{
	TActorIterator<ALTRoundManager> It(GetWorld());
	return It ? *It : nullptr;
}

ULTStationHeat* ALTGameMode::FindStationHeat() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ALTRoundManager> It(World); It; ++It)
	{
		if (ULTStationHeat* Heat = It->FindComponentByClass<ULTStationHeat>())
		{
			return Heat;
		}
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (ULTStationHeat* Heat = It->FindComponentByClass<ULTStationHeat>())
		{
			return Heat;
		}
	}

	return nullptr;
}

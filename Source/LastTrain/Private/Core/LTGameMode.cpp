#include "Core/LTGameMode.h"

#include "Core/LTGameState.h"
#include "EngineUtils.h"
#include "LastTrain.h"
#include "Rounds/LTRoundManager.h"
#include "Rounds/LTStationHeat.h"
#include "Weapons/LTWeaponComponent.h"

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

	// Reserve back to full, magazine untouched, per RefillAmmunition.
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

	LT_LOG(
		Log,
		TEXT(
			"Player boarded. Run state Boarded, rounds stopped, reserve refilled, heat reset. Travel to next station "
			"is not wired yet."));
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

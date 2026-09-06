#include "Core/LTGameMode.h"

#include "Core/LTGameState.h"
#include "EngineUtils.h"
#include "LastTrain.h"
#include "Rounds/LTRoundManager.h"

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

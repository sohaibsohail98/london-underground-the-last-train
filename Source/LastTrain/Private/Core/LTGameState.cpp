#include "Core/LTGameState.h"

#include "LastTrain.h"

void ALTGameState::SetRunState(const ELTRunState NewState)
{
	if (NewState == RunState)
	{
		return;
	}

	const ELTRunState OldState = RunState;
	RunState = NewState;

	LT_LOG(Log, TEXT("Run state %d to %d."), static_cast<int32>(OldState), static_cast<int32>(NewState));

	OnRunStateChanged.Broadcast(NewState, OldState);
}

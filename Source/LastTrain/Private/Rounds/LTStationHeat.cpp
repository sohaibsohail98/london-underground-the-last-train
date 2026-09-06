#include "Rounds/LTStationHeat.h"

#include "LastTrain.h"

ULTStationHeat::ULTStationHeat()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULTStationHeat::IncrementHeat()
{
	SetHeat(Heat + 1);
}

void ULTStationHeat::ResetHeat()
{
	SetHeat(0);
}

void ULTStationHeat::SetHeat(const int32 NewHeat)
{
	const int32 Clamped = FMath::Clamp(NewHeat, 0, MaximumHeat);
	if (Clamped == Heat)
	{
		return;
	}

	Heat = Clamped;
	LT_LOG(
		Log, TEXT("Station heat now %d. Live cap bonus %d, spawn rate x%.2f."), Heat, GetLiveCapBonus(),
		GetSpawnRateMultiplier());

	OnHeatChanged.Broadcast(Heat);
}

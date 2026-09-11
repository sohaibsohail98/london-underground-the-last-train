#include "Rounds/LTStationHeat.h"

#include "Audio/LTSubtitleSubsystem.h"
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

	const bool bRising = Clamped > Heat;

	Heat = Clamped;
	LT_LOG(
		Log, TEXT("Station heat now %d. Live cap bonus %d, spawn rate x%.2f."), Heat, GetLiveCapBonus(),
		GetSpawnRateMultiplier());

	if (bRising)
	{
		// Beside HeatRiseSound on branch claude/phase-g1-audio-prompt, and on a
		// rise only: the reset to zero on travel is silent there and silent here.
		// This is the only feedback a player gets that letting the train go cost
		// them something, so it is worth a caption to somebody who cannot hear it.
		ULTSubtitleSubsystem::ShowSubtitleForWorld(
			GetWorld(), LTSubtitleKeys::HeatRise, ELTSubtitleCategory::Round,
			NSLOCTEXT("LastTrain", "SubtitleHeatRise", "[Rising tone: the station is growing restless]"));
	}

	OnHeatChanged.Broadcast(Heat);
}

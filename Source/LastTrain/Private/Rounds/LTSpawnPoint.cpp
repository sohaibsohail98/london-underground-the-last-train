#include "Rounds/LTSpawnPoint.h"

ALTSpawnPoint::ALTSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
}

bool ALTSpawnPoint::IsAvailable(const int32 Round, const float WorldTime) const
{
	if (!bEnabled || Round < FirstRound)
	{
		return false;
	}

	return WorldTime - LastUsedTime >= CooldownSeconds;
}

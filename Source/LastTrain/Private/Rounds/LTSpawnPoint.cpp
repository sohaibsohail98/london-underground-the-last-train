#include "Rounds/LTSpawnPoint.h"

#if WITH_EDITORONLY_DATA
#include "Components/ArrowComponent.h"
#endif

ALTSpawnPoint::ALTSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif

	// An actor with no root cannot be placed or moved. Without this the ten hand
	// placed points in a level stay welded to the world origin, the round manager
	// projects (0,0,0) onto the navmesh and zombies spawn under the floor.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

#if WITH_EDITORONLY_DATA
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	if (DirectionArrow)
	{
		DirectionArrow->SetupAttachment(Root);
		DirectionArrow->SetHiddenInGame(true);
		DirectionArrow->ArrowColor = FColor(224, 160, 48);
		DirectionArrow->bIsScreenSizeScaled = true;
	}
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

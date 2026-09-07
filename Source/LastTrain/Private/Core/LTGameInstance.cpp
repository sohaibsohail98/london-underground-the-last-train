#include "Core/LTGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "UObject/UObjectGlobals.h"
#include "Weapons/LTWeaponData.h"

void ULTGameInstance::Init()
{
	Super::Init();

	PostLoadMapHandle =
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULTGameInstance::HandlePostLoadMap);
}

void ULTGameInstance::Shutdown()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	Super::Shutdown();
}

void ULTGameInstance::BeginStationTravel(const FLTTravelPayload& Payload, const FName DestinationMap)
{
	if (DestinationMap.IsNone())
	{
		LT_LOG(Warning, TEXT("BeginStationTravel called with no destination map. Staying put."));
		return;
	}

	PendingPayload = Payload;
	VisitedStations = Payload.VisitedStations;
	bTravelling = true;
	bPayloadLoadSeen = false;

	LT_LOG(
		Log, TEXT("Travelling to %s. Carrying %d points, weapon %s, reserve %d."), *DestinationMap.ToString(),
		Payload.CarriedPoints, Payload.CarriedWeapon ? *Payload.CarriedWeapon->GetName() : TEXT("none"),
		Payload.CarriedReserve);

	OnTravelStarted();

	UGameplayStatics::OpenLevel(this, DestinationMap);
}

FLTTravelPayload ULTGameInstance::ConsumePayload()
{
	const FLTTravelPayload Payload = PendingPayload;

	// Let go of the carried weapon reference as soon as the arena has it, so a
	// stale payload cannot keep an asset loaded for the rest of the session.
	PendingPayload = FLTTravelPayload();
	bTravelling = false;
	bPayloadLoadSeen = false;

	return Payload;
}

void ULTGameInstance::ClearRunHistory()
{
	VisitedStations.Reset();
}

void ULTGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!bTravelling)
	{
		bPayloadLoadSeen = false;
		return;
	}

	if (!bPayloadLoadSeen)
	{
		// The destination's own load. The arena has this session to claim it.
		bPayloadLoadSeen = true;
		return;
	}

	// A second load with the payload still here means nothing claimed it: a map
	// with no ALTGameMode, or one whose run never starts. Drop it rather than
	// granting a stale carry to whatever loads next.
	LT_LOG(
		Warning, TEXT("A travel payload went unclaimed for a whole map. Dropping %d points and the carried weapon."),
		PendingPayload.CarriedPoints);

	PendingPayload = FLTTravelPayload();
	VisitedStations.Reset();
	bTravelling = false;
	bPayloadLoadSeen = false;
}

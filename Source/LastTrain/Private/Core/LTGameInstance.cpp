#include "Core/LTGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "Weapons/LTWeaponData.h"

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

	return Payload;
}

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LTGameInstance.generated.h"

class ULTWeaponData;

/** What a boarding player carries from one station to the next. Held in memory on
	the game instance for the session only: a disk save game is a later task. */
USTRUCT(BlueprintType)
struct FLTTravelPayload
{
	GENERATED_BODY()

	/** Points the player leaves the last station with. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	int32 CarriedPoints = 0;

	/** The weapon held on boarding. Null means the destination uses its default. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	TObjectPtr<ULTWeaponData> CarriedWeapon = nullptr;

	/** Reserve ammunition count for CarriedWeapon at the moment of boarding. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	int32 CarriedReserve = 0;

	/** Asset names of stations visited this run, oldest first, including the one
		just left. For a future no-immediate-backtrack rule and a run summary. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	TArray<FName> VisitedStations;
};

/** Survives the level load between stations, so boarding a train can carry the
	run's points and weapon into the next arena. Travel is the only thing on it:
	a run's rules stay with the game mode. */
UCLASS()
class LASTTRAIN_API ULTGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/** Stores the payload and opens DestinationMap. The game mode calls this from
		NotifyPlayerBoarded. Does nothing if DestinationMap is unset. */
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void BeginStationTravel(const FLTTravelPayload& Payload, FName DestinationMap);

	/** True between boarding and the destination game mode taking the payload. */
	UFUNCTION(BlueprintPure, Category = "Travel")
	bool IsTravelling() const { return bTravelling; }

	/** Hands the payload to the destination arena and clears the travelling flag.
		The game mode calls this exactly once on arrival. */
	UFUNCTION(BlueprintCallable, Category = "Travel")
	FLTTravelPayload ConsumePayload();

	/** Stations visited this run, oldest first. Accumulates across hops. */
	UFUNCTION(BlueprintPure, Category = "Travel")
	TArray<FName> GetVisitedStations() const { return VisitedStations; }

protected:
	/** Fired just before the level opens, so a Blueprint can fade the screen out.
		The destination arena fades back in on its own BeginPlay. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Travel")
	void OnTravelStarted();

private:
	UPROPERTY()
	FLTTravelPayload PendingPayload;

	/** The run's station history, kept beyond a single hop's payload. */
	UPROPERTY()
	TArray<FName> VisitedStations;

	UPROPERTY()
	bool bTravelling = false;
};

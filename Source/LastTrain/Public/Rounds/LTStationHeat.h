#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTStationHeat.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeatChanged, int32, NewHeat);

/** Station heat. Rises by one each time the player lets a train leave without
    boarding, and resets to zero on travel. Heat widens the live zombie cap and
    quickens the spawn rate. The round manager reads the two getters here. */
UCLASS(ClassGroup = (LastTrain), meta = (BlueprintSpawnableComponent))
class LASTTRAIN_API ULTStationHeat : public UActorComponent
{
	GENERATED_BODY()

public:
	ULTStationHeat();

	UPROPERTY(BlueprintAssignable, Category = "Rounds")
	FOnHeatChanged OnHeatChanged;

	/** Extra live zombies allowed per heat level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	int32 LiveCapPerHeat = 6;

	/** Spawn rate increase per heat level, as a fraction. 0.12 is plus 12 per cent. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float SpawnRateFractionPerHeat = 0.12f;

	/** Upper bound on heat, so the numbers stay sane on a very long stay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	int32 MaximumHeat = 10;

	/** Raises heat by one, clamped to MaximumHeat. Call when a train departs
	    with the player still in the station. */
	UFUNCTION(BlueprintCallable, Category = "Rounds")
	void IncrementHeat();

	/** Returns heat to zero. Call on travel. */
	UFUNCTION(BlueprintCallable, Category = "Rounds")
	void ResetHeat();

	UFUNCTION(BlueprintPure, Category = "Rounds")
	int32 GetHeat() const { return Heat; }

	/** Additional live zombies the current heat allows. */
	UFUNCTION(BlueprintPure, Category = "Rounds")
	int32 GetLiveCapBonus() const { return Heat * LiveCapPerHeat; }

	/** Multiplier on the spawn rate for the current heat. 1.0 at heat zero. */
	UFUNCTION(BlueprintPure, Category = "Rounds")
	float GetSpawnRateMultiplier() const { return 1.f + Heat * SpawnRateFractionPerHeat; }

private:
	void SetHeat(int32 NewHeat);

	UPROPERTY(VisibleInstanceOnly, Category = "Rounds")
	int32 Heat = 0;
};

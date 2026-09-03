#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LTSpawnPoint.generated.h"

/** A hand placed zombie spawn. Carries an area for door gating and a weight for traffic. */
UCLASS()
class LASTTRAIN_API ALTSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ALTSpawnPoint();

	/** Area name matching the door that unlocks it. Empty means always live. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FName AreaTag;

	/** Relative likelihood of being chosen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0.1"))
	float Weight = 1.f;

	/** Earliest round this point becomes active. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "1"))
	int32 FirstRound = 1;

	/** Seconds this point must wait after use before spawning again. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float CooldownSeconds = 0.9f;

	/** Set false while the area behind a purchasable door is still shut. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bEnabled = true;

	UFUNCTION(BlueprintPure, Category = "Spawning")
	bool IsAvailable(int32 Round, float WorldTime) const;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void MarkUsed(float WorldTime) { LastUsedTime = WorldTime; }

private:
	float LastUsedTime = -1000.f;
};

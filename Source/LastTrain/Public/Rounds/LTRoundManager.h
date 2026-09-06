#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LTRoundManager.generated.h"

class ALTZombieCharacter;
class ALTSpawnPoint;
class ULTStationHeat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStarted, int32, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnded, int32, Round);

/** The round loop. Spawn points come from the level, so this is station agnostic. */
UCLASS()
class LASTTRAIN_API ALTRoundManager : public AActor
{
	GENERATED_BODY()

public:
	ALTRoundManager();

	UPROPERTY(BlueprintAssignable, Category = "Rounds")
	FOnRoundStarted OnRoundStarted;

	UPROPERTY(BlueprintAssignable, Category = "Rounds")
	FOnRoundEnded OnRoundEnded;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Rounds")
	void BeginRounds();

	UFUNCTION(BlueprintCallable, Category = "Rounds")
	void StopRounds();

	UFUNCTION(BlueprintPure, Category = "Rounds")
	int32 GetCurrentRound() const { return CurrentRound; }

	UFUNCTION(BlueprintPure, Category = "Rounds")
	int32 GetZombiesRemaining() const { return PendingSpawns + LiveZombies.Num(); }

	/** MaximumAlive plus the current station heat's live cap bonus. */
	UFUNCTION(BlueprintPure, Category = "Rounds")
	int32 GetEffectiveMaximumAlive() const;

	/** Set per station to vary the roster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rounds")
	TSubclassOf<ALTZombieCharacter> ZombieClass;

	/** Formula takes over past the end of this array. EditAnywhere so a placed
		round manager can be tuned per map without a new Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rounds")
	TArray<int32> OpeningRoundCounts = {6, 8, 10, 12, 14};

	/** Additional zombies per round once the formula applies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float CountGrowthPerRound = 2.4f;

	/** Never exceed this many alive at once, whatever the round total. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	int32 MaximumAlive = 24;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float BreatherSeconds = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float BaseSpawnIntervalSeconds = 1.6f;

	/** Spawn interval multiplier per round, compounding downward. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float SpawnIntervalDecay = 0.96f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float MinimumSpawnInterval = 0.35f;

	/** Search box for snapping a spawn point onto the navmesh. Generous on Z so
		a point placed a little above or below the floor still resolves. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	FVector NavProjectionExtent = FVector(200.f, 200.f, 500.f);

	/** Added to the projected navmesh Z so the spawned capsule rests on the
		floor rather than half sunk into it. Roughly a character half height. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float SpawnCapsuleLift = 90.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	void StartRound(int32 Round);
	void EndRound();
	void TrySpawnOne();
	int32 ComputeRoundCount(int32 Round) const;
	float ComputeSpawnInterval(int32 Round) const;

	UFUNCTION()
	void HandleZombieDied(ALTZombieCharacter* Zombie, bool bHeadshot);

	UPROPERTY() TArray<TObjectPtr<ALTSpawnPoint>> SpawnPoints;
	UPROPERTY() TArray<TObjectPtr<ALTZombieCharacter>> LiveZombies;

	/** Optional. Found in the level on BeginPlay. Null means base cap and rate. */
	UPROPERTY() TObjectPtr<ULTStationHeat> Heat;

	int32 CurrentRound = 0;
	int32 PendingSpawns = 0;
	float SpawnTimer = 0.f;
	float BreatherRemaining = 0.f;

	bool bRunning = false;
	bool bInBreather = false;
};

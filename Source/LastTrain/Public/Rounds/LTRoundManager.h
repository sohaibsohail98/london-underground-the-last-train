#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LTRoundManager.generated.h"

class ALTZombieCharacter;
class ALTSpawnPoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStarted, int32, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnded, int32, Round);

/**
 * The round loop. Spawn points are supplied by the level rather than hardcoded
 * here, per the architecture document, so the same manager drives every
 * station without modification.
 *
 * Round composition uses a formula rather than a table beyond the opening
 * rounds, which are hand set because the first five rounds do most of the work
 * teaching the player what the game is.
 */
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

	/** Zombie class to spawn. Set per station to vary the roster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rounds")
	TSubclassOf<ALTZombieCharacter> ZombieClass;

	/** Hand set counts for the opening rounds. Formula takes over after. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	TArray<int32> OpeningRoundCounts = { 6, 8, 10, 12, 14 };

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

protected:
	virtual void BeginPlay() override;

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

	int32 CurrentRound = 0;
	int32 PendingSpawns = 0;
	float SpawnTimer = 0.f;
	float BreatherRemaining = 0.f;

	bool bRunning = false;
	bool bInBreather = false;
};

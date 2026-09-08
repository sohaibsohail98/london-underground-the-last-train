#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LTRoundManager.generated.h"

class ALTZombieCharacter;
class ALTSpawnPoint;
class ULTStationHeat;
class ULTZombieTypeData;
enum class ELTZombieType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStarted, int32, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnded, int32, Round);

/** One spawnable zombie type on a station's roster. A struct rather than a bare
	pointer so per-station overrides can be added later without touching maps. */
USTRUCT(BlueprintType)
struct FLTZombieRosterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	TObjectPtr<ULTZombieTypeData> TypeData;
};

/** What one round is made of. Decided once in StartRound and read by every spawn,
	so a round's composition cannot drift halfway through it. */
USTRUCT()
struct FLTRoundPlan
{
	GENERATED_BODY()

	/** Zombies in the normal or forced composition, before the guaranteed group. */
	UPROPERTY()
	int32 TotalCount = 0;

	/** True on a sprinter round: every spawn in TotalCount is ForcedType. */
	UPROPERTY()
	bool bForceSingleType = false;

	UPROPERTY()
	TObjectPtr<ULTZombieTypeData> ForcedType = nullptr;

	/** Brutes placed on top of TotalCount, at GuaranteedSpawnIndices. */
	UPROPERTY()
	int32 GuaranteedCount = 0;

	UPROPERTY()
	TObjectPtr<ULTZombieTypeData> GuaranteedType = nullptr;

	/** Spawn indices within the round that the guaranteed group lands on, so the
		pair arrives spread through the round rather than together at the start. */
	UPROPERTY()
	TArray<int32> GuaranteedSpawnIndices;
};

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

	/** True on a sprinter or a brute round. For a HUD banner. */
	UFUNCTION(BlueprintPure, Category = "Rounds")
	bool IsSpecialRound() const;

	/** "Sprinters", "Brutes", "SprintersAndBrutes" on a round that is both, or
		None on a normal round. A tag rather than an enum so a widget can switch on
		it without the round manager owning the presentation. */
	UFUNCTION(BlueprintPure, Category = "Rounds")
	FName GetSpecialRoundTag() const;

	/** Set per station to vary the roster. Every zombie is spawned from this class
		whatever its type: the roster drives type data, not the spawned class. There
		is one mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rounds")
	TSubclassOf<ALTZombieCharacter> ZombieClass;

	/** The station's zombie types. When empty, the round manager spawns
		ZombieClass with no type data, exactly as before the roster existed, so a
		map with no roster keeps working unchanged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rounds")
	TArray<FLTZombieRosterEntry> Roster;

	/** Every Nth round is an all-sprinter round. 0 disables. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	int32 SprinterRoundInterval = 5;

	/** A sprinter round spawns this fraction of the normal round count. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float SprinterRoundCountFraction = 0.75f;

	/** Every Nth round adds a fixed group of brutes on top of the normal count.
		0 disables. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	int32 BruteRoundInterval = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	int32 BruteRoundBruteCount = 2;

	/** What a round divisible by both intervals is. gameplay-canon.md section 6
		reads two ways: it lists the sprinter rounds as 5, 15, 25 and calls every
		10th round a normal walker round plus the pair, then calls rounds 20 and 30
		a sprinter round carrying the pair as well. Set, the brute round wins and
		rounds 10, 20, 30 are a walker round plus the pair, which is what every
		acceptance list tests for. Clear it to stack the two instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	bool bBruteRoundOverridesSprinterRound = true;

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
		a point placed a little above or below the floor still resolves. Left
		generous on purpose: tightening it would turn a point that used to snap
		into one that spawns a zombie into the void, and it is the silence that
		hid the Canary Wharf points at world origin, not the reach. The warning
		below is what breaks that silence. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	FVector NavProjectionExtent = FVector(200.f, 200.f, 500.f);

	/** How far a projection may move a spawn point before it is worth a warning.
		A point nudged onto the floor moves a few units; one resolving to a floor
		nobody placed it near moves hundreds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
	float NavProjectionWarnDistance = 200.f;

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

	/** Decides a round's count and composition. Special rounds are data driven off
		the four interval properties, and a round can be both: canon has rounds 20,
		30 and so on as a sprinter round carrying a brute pair as well. */
	FLTRoundPlan BuildRoundPlan(int32 Round) const;

	/** The type for one spawn: the guaranteed group first, then a forced single
		type, then the normal weighted mix. Null leaves a plain walker on the coded
		defaults, which is what an empty roster gets. */
	const ULTZombieTypeData* ChooseTypeForSpawn(int32 InSpawnIndex, int32 Round) const;

	/** The roster's entry for a type, or null when the station does not carry it.
		Mutable, because the round plan stores it in a TObjectPtr. */
	ULTZombieTypeData* FindRosterType(ELTZombieType Type) const;

	int32 CountAliveOfType(ELTZombieType Type) const;

	UFUNCTION()
	void HandleZombieDied(ALTZombieCharacter* Zombie, bool bHeadshot);

	/** A screamer asks for an extra wave. WalkerCount zero is a cancel: the
		screamer died inside its cancel window. */
	UFUNCTION()
	void HandleZombieScreamed(ALTZombieCharacter* Screamer, int32 WalkerCount);

	UPROPERTY() TArray<TObjectPtr<ALTSpawnPoint>> SpawnPoints;
	UPROPERTY() TArray<TObjectPtr<ALTZombieCharacter>> LiveZombies;

	/** Optional. Found in the level on BeginPlay. Null means base cap and rate. */
	UPROPERTY() TObjectPtr<ULTStationHeat> Heat;

	UPROPERTY()
	FLTRoundPlan CurrentPlan;

	int32 CurrentRound = 0;
	int32 PendingSpawns = 0;

	/** Spawns made this round, so the guaranteed group lands on its planned
		indices. Reset in StartRound. */
	int32 SpawnIndex = 0;

	/** Of PendingSpawns, how many came from a scream and can still be cancelled. */
	int32 PendingScreamSpawns = 0;

	float SpawnTimer = 0.f;
	float BreatherRemaining = 0.f;

	bool bRunning = false;
	bool bInBreather = false;
};

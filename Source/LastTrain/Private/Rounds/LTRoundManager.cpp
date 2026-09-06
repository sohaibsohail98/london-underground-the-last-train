#include "Rounds/LTRoundManager.h"

#include "EngineUtils.h"
#include "LastTrain.h"
#include "NavigationSystem.h"
#include "Rounds/LTSpawnPoint.h"
#include "Rounds/LTStationHeat.h"
#include "Zombies/LTZombieCharacter.h"

ALTRoundManager::ALTRoundManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALTRoundManager::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ALTSpawnPoint> It(GetWorld()); It; ++It)
	{
		SpawnPoints.Add(*It);
	}

	if (SpawnPoints.Num() == 0)
	{
		LT_LOG(Warning, TEXT("Round manager found no spawn points in the level. No rounds will run."));
	}

	// Station heat is optional. Prefer one on this actor, else the first found in
	// the level. With none, the cap and spawn interval stay at their base values.
	Heat = FindComponentByClass<ULTStationHeat>();
	if (!Heat)
	{
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (ULTStationHeat* Found = It->FindComponentByClass<ULTStationHeat>())
			{
				Heat = Found;
				break;
			}
		}
	}
}

void ALTRoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Drop every reference this actor holds into the world before teardown. The
	// bound OnZombieDied delegates and the LiveZombies array otherwise keep PIE
	// actors alive into garbage collection, which the editor's transaction buffer
	// then trips over on EndPlayMap.
	StopRounds();

	for (const TObjectPtr<ALTZombieCharacter>& Zombie : LiveZombies)
	{
		if (Zombie)
		{
			Zombie->OnZombieDied.RemoveAll(this);
		}
	}

	LiveZombies.Reset();
	SpawnPoints.Reset();
	Heat = nullptr;

	Super::EndPlay(EndPlayReason);
}

void ALTRoundManager::BeginRounds()
{
	if (bRunning)
	{
		return;
	}

	bRunning = true;
	StartRound(1);
}

void ALTRoundManager::StopRounds()
{
	bRunning = false;
	bInBreather = false;
	PendingSpawns = 0;
}

int32 ALTRoundManager::ComputeRoundCount(const int32 Round) const
{
	if (OpeningRoundCounts.IsValidIndex(Round - 1))
	{
		return OpeningRoundCounts[Round - 1];
	}

	const int32 Opening = OpeningRoundCounts.Num();
	const int32 Last = Opening > 0 ? OpeningRoundCounts.Last() : 6;
	const float Extra = CountGrowthPerRound * static_cast<float>(Round - Opening);

	return Last + FMath::RoundToInt(Extra);
}

float ALTRoundManager::ComputeSpawnInterval(const int32 Round) const
{
	float Interval = BaseSpawnIntervalSeconds * FMath::Pow(SpawnIntervalDecay, static_cast<float>(Round - 1));

	// Heat quickens spawns. A higher rate is a shorter interval.
	if (Heat)
	{
		Interval /= FMath::Max(0.01f, Heat->GetSpawnRateMultiplier());
	}

	return FMath::Max(MinimumSpawnInterval, Interval);
}

int32 ALTRoundManager::GetEffectiveMaximumAlive() const
{
	return MaximumAlive + (Heat ? Heat->GetLiveCapBonus() : 0);
}

void ALTRoundManager::StartRound(const int32 Round)
{
	CurrentRound = Round;
	PendingSpawns = ComputeRoundCount(Round);
	SpawnTimer = 0.f;
	bInBreather = false;

	LT_LOG(Log, TEXT("Round %d starting with %d zombies."), Round, PendingSpawns);

	OnRoundStarted.Broadcast(Round);
}

void ALTRoundManager::EndRound()
{
	// Exactly once per round.
	OnRoundEnded.Broadcast(CurrentRound);

	bInBreather = true;
	BreatherRemaining = BreatherSeconds;
}

void ALTRoundManager::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bRunning)
	{
		return;
	}

	if (bInBreather)
	{
		BreatherRemaining -= DeltaSeconds;
		if (BreatherRemaining <= 0.f)
		{
			StartRound(CurrentRound + 1);
		}
		return;
	}

	if (PendingSpawns > 0)
	{
		SpawnTimer -= DeltaSeconds;
		if (SpawnTimer <= 0.f && LiveZombies.Num() < GetEffectiveMaximumAlive())
		{
			TrySpawnOne();
			SpawnTimer = ComputeSpawnInterval(CurrentRound);
		}
	}
	else if (LiveZombies.Num() == 0)
	{
		EndRound();
	}
}

void ALTRoundManager::TrySpawnOne()
{
	if (!ZombieClass || SpawnPoints.Num() == 0)
	{
		return;
	}

	const float WorldTime = GetWorld()->GetTimeSeconds();

	// Weighted choice among available points.
	float TotalWeight = 0.f;
	TArray<ALTSpawnPoint*> Available;

	for (const TObjectPtr<ALTSpawnPoint>& Point : SpawnPoints)
	{
		if (Point && Point->IsAvailable(CurrentRound, WorldTime))
		{
			Available.Add(Point);
			TotalWeight += Point->Weight;
		}
	}

	if (Available.Num() == 0)
	{
		return;
	}

	float Roll = FMath::FRand() * TotalWeight;
	ALTSpawnPoint* Chosen = Available.Last();

	for (ALTSpawnPoint* Point : Available)
	{
		Roll -= Point->Weight;
		if (Roll <= 0.f)
		{
			Chosen = Point;
			break;
		}
	}

	// Snap the spawn to the navmesh. A hand placed point that sits slightly
	// above, below or beside walkable ground would otherwise drop a zombie into
	// the void on spawn, since the character has no floor to catch it.
	FVector SpawnLocation = Chosen->GetActorLocation();
	if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation Projected;
		if (Nav->ProjectPointToNavigation(SpawnLocation, Projected, NavProjectionExtent))
		{
			SpawnLocation = Projected.Location;
			// Lift by the capsule half height so the capsule rests on the floor
			// rather than clipping through it.
			SpawnLocation.Z += SpawnCapsuleLift;
		}
		else
		{
			LT_LOG(
				Warning, TEXT("Spawn point %s is not near the navmesh. Zombie may fall through."), *Chosen->GetName());
		}
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ALTZombieCharacter* Zombie =
		GetWorld()->SpawnActor<ALTZombieCharacter>(ZombieClass, SpawnLocation, Chosen->GetActorRotation(), Params);

	if (!Zombie)
	{
		return;
	}

	Zombie->ApplyRoundScaling(CurrentRound);
	Zombie->OnZombieDied.AddDynamic(this, &ALTRoundManager::HandleZombieDied);

	LiveZombies.Add(Zombie);
	Chosen->MarkUsed(WorldTime);
	PendingSpawns -= 1;

	LT_LOG(
		Verbose, TEXT("Spawned zombie. Alive %d, pending %d, cap %d, round %d."), LiveZombies.Num(), PendingSpawns,
		MaximumAlive, CurrentRound);
}

void ALTRoundManager::HandleZombieDied(ALTZombieCharacter* Zombie, const bool bHeadshot)
{
	if (Zombie)
	{
		Zombie->OnZombieDied.RemoveAll(this);
	}

	LiveZombies.Remove(Zombie);
}

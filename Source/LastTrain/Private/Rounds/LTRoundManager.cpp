#include "Rounds/LTRoundManager.h"

#include "EngineUtils.h"
#include "LastTrain.h"
#include "Rounds/LTSpawnPoint.h"
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
	const float Interval = BaseSpawnIntervalSeconds * FMath::Pow(SpawnIntervalDecay, static_cast<float>(Round - 1));
	return FMath::Max(MinimumSpawnInterval, Interval);
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
		if (SpawnTimer <= 0.f && LiveZombies.Num() < MaximumAlive)
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

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ALTZombieCharacter* Zombie = GetWorld()->SpawnActor<ALTZombieCharacter>(
		ZombieClass, Chosen->GetActorLocation(), Chosen->GetActorRotation(), Params);

	if (!Zombie)
	{
		return;
	}

	Zombie->ApplyRoundScaling(CurrentRound);
	Zombie->OnZombieDied.AddDynamic(this, &ALTRoundManager::HandleZombieDied);

	LiveZombies.Add(Zombie);
	Chosen->MarkUsed(WorldTime);
	PendingSpawns -= 1;
}

void ALTRoundManager::HandleZombieDied(ALTZombieCharacter* Zombie, const bool bHeadshot)
{
	LiveZombies.Remove(Zombie);
}

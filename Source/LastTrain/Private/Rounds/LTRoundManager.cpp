#include "Rounds/LTRoundManager.h"

#include "EngineUtils.h"
#include "LastTrain.h"
#include "NavigationSystem.h"
#include "Rounds/LTSpawnPoint.h"
#include "Rounds/LTStationHeat.h"
#include "Zombies/LTZombieCharacter.h"
#include "Zombies/LTZombieTypeData.h"

namespace
{
	/** Heat at or above which the special types roughly double their share of a
		normal round. Provisional, from gameplay-canon section 6. */
	constexpr int32 HighHeatWeightThreshold = 3;

	/** A type's weight in a normal round's mix, with the high heat shift applied. */
	float NormalRoundWeight(const ULTZombieTypeData& Type, const bool bHighHeat)
	{
		const float Base = FMath::Max(0.f, Type.SpawnWeightNormalRound);
		const float HeatMultiplier = bHighHeat ? FMath::Max(0.f, Type.HighHeatWeightMultiplier) : 1.f;

		return Base * HeatMultiplier;
	}
} // namespace

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

	int32 UsableTypes = 0;
	for (const FLTZombieRosterEntry& Entry : Roster)
	{
		if (Entry.TypeData)
		{
			UsableTypes += 1;
		}
	}

	if (Roster.Num() == 0)
	{
		LT_LOG(Log, TEXT("Round manager has no zombie roster. Every spawn is a plain ZombieClass walker."));
	}
	else if (UsableTypes == 0)
	{
		LT_LOG(
			Warning,
			TEXT(
				"Round manager roster has %d entries but no type data on any of them. Every spawn is a plain "
				"walker."),
			Roster.Num());
	}
	else
	{
		LT_LOG(Log, TEXT("Round manager roster carries %d zombie type(s)."), UsableTypes);
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
			Zombie->OnZombieScreamed.RemoveAll(this);
		}
	}

	CurrentPlan = FLTRoundPlan();

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
	PendingScreamSpawns = 0;

	// The plan describes a round that is no longer running, so a HUD reading
	// GetSpecialRoundTag would keep flying a SPRINTERS banner over a stopped run.
	CurrentPlan = FLTRoundPlan();
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

bool ALTRoundManager::IsSpecialRound() const
{
	return !GetSpecialRoundTag().IsNone();
}

FName ALTRoundManager::GetSpecialRoundTag() const
{
	const bool bSprinters = CurrentPlan.bForceSingleType && CurrentPlan.ForcedType != nullptr;
	const bool bBrutes = CurrentPlan.GuaranteedCount > 0 && CurrentPlan.GuaranteedType != nullptr;

	if (bSprinters && bBrutes)
	{
		return TEXT("SprintersAndBrutes");
	}
	if (bSprinters)
	{
		return TEXT("Sprinters");
	}
	if (bBrutes)
	{
		return TEXT("Brutes");
	}

	return NAME_None;
}

FLTRoundPlan ALTRoundManager::BuildRoundPlan(const int32 Round) const
{
	FLTRoundPlan Plan;
	Plan.TotalCount = ComputeRoundCount(Round);

	const bool bSprinterRound = SprinterRoundInterval > 0 && Round % SprinterRoundInterval == 0;
	const bool bBruteRound = BruteRoundInterval > 0 && Round % BruteRoundInterval == 0;

	// Every brute round is also a sprinter round on the shipped intervals, and
	// canon reads both ways on what that round should be. The flag decides:
	// set, round 10 is the walker round plus the pair every acceptance list
	// describes; clear, it is a sprinter round carrying the pair as well.
	const bool bForceSprinters = bSprinterRound && !(bBruteRound && bBruteRoundOverridesSprinterRound);

	// A station with no sprinter or brute on its roster simply plays a normal
	// round, so a map that has not been given one is unaffected.
	if (bForceSprinters)
	{
		if (ULTZombieTypeData* Sprinter = FindRosterType(ELTZombieType::Sprinter))
		{
			Plan.bForceSingleType = true;
			Plan.ForcedType = Sprinter;
			Plan.TotalCount =
				FMath::Max(1, FMath::RoundToInt(static_cast<float>(Plan.TotalCount) * SprinterRoundCountFraction));
		}
	}

	if (bBruteRound && BruteRoundBruteCount > 0)
	{
		if (ULTZombieTypeData* Brute = FindRosterType(ELTZombieType::Brute))
		{
			// Spread the group through the round rather than dropping it at the
			// gate: canon puts a pair at roughly 30 and 70 per cent of the count.
			const int32 TotalSpawns = Plan.TotalCount + BruteRoundBruteCount;

			for (int32 Placed = 0; Placed < BruteRoundBruteCount; ++Placed)
			{
				const float Fraction = static_cast<float>(Placed + 1) / static_cast<float>(BruteRoundBruteCount + 1);

				int32 Index = FMath::Clamp(
					FMath::RoundToInt(Fraction * static_cast<float>(TotalSpawns)), 0, FMath::Max(0, TotalSpawns - 1));

				// A very short round can collide two fractions on one index. Walk
				// forward for a free slot, then back from the end if the round has
				// run out of room ahead. Walking off the end silently cost a brute
				// its slot: the spawn happened, but as an ordinary roster roll.
				while (Index < TotalSpawns && Plan.GuaranteedSpawnIndices.Contains(Index))
				{
					Index += 1;
				}

				if (Index >= TotalSpawns)
				{
					Index = TotalSpawns - 1;
					while (Index >= 0 && Plan.GuaranteedSpawnIndices.Contains(Index))
					{
						Index -= 1;
					}
				}

				if (Index >= 0)
				{
					Plan.GuaranteedSpawnIndices.Add(Index);
				}
			}

			// Count what actually got a slot, so PendingSpawns cannot promise a
			// brute the round has nowhere to put.
			if (Plan.GuaranteedSpawnIndices.Num() > 0)
			{
				Plan.GuaranteedType = Brute;
				Plan.GuaranteedCount = Plan.GuaranteedSpawnIndices.Num();
			}
		}
	}

	return Plan;
}

ULTZombieTypeData* ALTRoundManager::FindRosterType(const ELTZombieType Type) const
{
	for (const FLTZombieRosterEntry& Entry : Roster)
	{
		if (Entry.TypeData && Entry.TypeData->Type == Type)
		{
			return Entry.TypeData;
		}
	}

	return nullptr;
}

int32 ALTRoundManager::CountAliveOfType(const ELTZombieType Type) const
{
	int32 Count = 0;

	for (const TObjectPtr<ALTZombieCharacter>& Zombie : LiveZombies)
	{
		if (Zombie && !Zombie->IsDead() && Zombie->GetZombieType() == Type)
		{
			Count += 1;
		}
	}

	return Count;
}

const ULTZombieTypeData* ALTRoundManager::ChooseTypeForSpawn(const int32 InSpawnIndex, const int32 Round) const
{
	if (Roster.Num() == 0)
	{
		return nullptr;
	}

	// The guaranteed group first: a brute round's pair lands on planned indices.
	if (CurrentPlan.GuaranteedType && CurrentPlan.GuaranteedSpawnIndices.Contains(InSpawnIndex))
	{
		return CurrentPlan.GuaranteedType;
	}

	// Then a forced single type: a sprinter round is nothing but sprinters, so no
	// crawler or screamer mixes in.
	if (CurrentPlan.bForceSingleType && CurrentPlan.ForcedType)
	{
		return CurrentPlan.ForcedType;
	}

	// Then the normal weighted mix.
	const bool bHighHeat = Heat && Heat->GetHeat() >= HighHeatWeightThreshold;

	float TotalWeight = 0.f;
	TArray<const ULTZombieTypeData*> Available;

	for (const FLTZombieRosterEntry& Entry : Roster)
	{
		const ULTZombieTypeData* Type = Entry.TypeData;

		// A special-only type (weight zero) never appears in a normal round.
		if (!Type || Type->IsSpecialOnly() || Round < Type->FirstRoundAvailable)
		{
			continue;
		}

		if (Type->MaxAliveOfThisType > 0 && CountAliveOfType(Type->Type) >= Type->MaxAliveOfThisType)
		{
			continue;
		}

		Available.Add(Type);
		TotalWeight += NormalRoundWeight(*Type, bHighHeat);
	}

	if (Available.Num() > 0 && TotalWeight > 0.f)
	{
		float Roll = FMath::FRand() * TotalWeight;

		for (const ULTZombieTypeData* Type : Available)
		{
			Roll -= NormalRoundWeight(*Type, bHighHeat);
			if (Roll <= 0.f)
			{
				return Type;
			}
		}

		return Available.Last();
	}

	// Nothing weighted is available: every candidate is capped out or not yet in
	// play. Fall back to the roster's plain walker so the round still runs.
	for (const FLTZombieRosterEntry& Entry : Roster)
	{
		if (Entry.TypeData && Entry.TypeData->Behaviour == ELTZombieBehaviour::None)
		{
			return Entry.TypeData;
		}
	}

	return nullptr;
}

void ALTRoundManager::StartRound(const int32 Round)
{
	CurrentRound = Round;
	CurrentPlan = BuildRoundPlan(Round);

	// The guaranteed group is on top of the round's own count, so a brute round is
	// a full walker round plus the pair.
	PendingSpawns = CurrentPlan.TotalCount + CurrentPlan.GuaranteedCount;
	SpawnIndex = 0;
	PendingScreamSpawns = 0;
	SpawnTimer = 0.f;
	bInBreather = false;

	const FName SpecialTag = GetSpecialRoundTag();

	if (SpecialTag.IsNone())
	{
		LT_LOG(Log, TEXT("Round %d starting with %d zombies."), Round, PendingSpawns);
	}
	else
	{
		LT_LOG(
			Log, TEXT("Round %d starting with %d zombies. Special round: %s."), Round, PendingSpawns,
			*SpecialTag.ToString());
	}

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
			// A projection that moves the point a long way found a floor nobody
			// placed this spawn on. It still counts as a success, so say so: an
			// unplaced point sitting at world origin is otherwise silent until
			// zombies start arriving somewhere nobody expects them.
			const float Moved = FVector::Dist(SpawnLocation, Projected.Location);
			if (NavProjectionWarnDistance > 0.f && Moved > NavProjectionWarnDistance)
			{
				LT_LOG(
					Warning, TEXT("Spawn point %s snapped %.0f units onto the navmesh. Check where it is placed."),
					*Chosen->GetName(), Moved);
			}

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

	if (const ULTZombieTypeData* ChosenType = ChooseTypeForSpawn(SpawnIndex, CurrentRound))
	{
		Zombie->ApplyTypeData(ChosenType);
	}
	else if (Roster.Num() > 0)
	{
		LT_LOG(
			Warning, TEXT("Round %d spawn %d found no usable roster type. Spawned a plain walker."), CurrentRound,
			SpawnIndex);
	}

	Zombie->OnZombieDied.AddDynamic(this, &ALTRoundManager::HandleZombieDied);
	Zombie->OnZombieScreamed.AddDynamic(this, &ALTRoundManager::HandleZombieScreamed);

	LiveZombies.Add(Zombie);
	Chosen->MarkUsed(WorldTime);
	PendingSpawns -= 1;
	SpawnIndex += 1;

	// Summoned walkers are ordinary pending spawns, so a cancel can only drop the
	// ones still queued. Which of them this spawn was does not matter.
	PendingScreamSpawns = FMath::Max(0, PendingScreamSpawns - 1);

	LT_LOG(
		Verbose, TEXT("Spawned zombie. Alive %d, pending %d, cap %d, round %d."), LiveZombies.Num(), PendingSpawns,
		GetEffectiveMaximumAlive(), CurrentRound);
}

void ALTRoundManager::HandleZombieDied(ALTZombieCharacter* Zombie, const bool bHeadshot)
{
	if (Zombie)
	{
		Zombie->OnZombieDied.RemoveAll(this);
		Zombie->OnZombieScreamed.RemoveAll(this);
	}

	LiveZombies.Remove(Zombie);
}

void ALTRoundManager::HandleZombieScreamed(ALTZombieCharacter* Screamer, const int32 WalkerCount)
{
	if (!bRunning || bInBreather)
	{
		return;
	}

	if (WalkerCount > 0)
	{
		// Ordinary pending spawns: they obey the live cap and the spawn interval
		// like anything else, so a scream raises pressure rather than bypassing it.
		PendingSpawns += WalkerCount;
		PendingScreamSpawns += WalkerCount;

		LT_LOG(Log, TEXT("Screamer called %d extra walkers. Pending now %d."), WalkerCount, PendingSpawns);
		return;
	}

	// A cancel: the screamer died inside its window. Drop whatever of its wave has
	// not spawned yet.
	const int32 Dropped = FMath::Min(PendingScreamSpawns, PendingSpawns);

	PendingSpawns -= Dropped;
	PendingScreamSpawns = 0;

	LT_LOG(Log, TEXT("Screamer cancelled. Dropped %d queued walkers, pending now %d."), Dropped, PendingSpawns);
}

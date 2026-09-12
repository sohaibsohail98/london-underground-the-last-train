#include "Combat/LTGoreDecalSubsystem.h"

#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "LastTrain.h"
#include "Materials/MaterialInterface.h"

void ULTGoreDecalSubsystem::SpawnBloodDecalForWorld(
	const UWorld* World, const FVector& Location, const FVector& Normal, const bool bHeadshot)
{
	if (ULTGoreDecalSubsystem* Subsystem = Get(World))
	{
		Subsystem->SpawnBloodDecal(Location, Normal, bHeadshot);
	}
}

void ULTGoreDecalSubsystem::SpawnDeathPoolForWorld(const UWorld* World, const FVector& Location, const bool bHeadshot)
{
	if (ULTGoreDecalSubsystem* Subsystem = Get(World))
	{
		Subsystem->SpawnDeathPool(Location, bHeadshot);
	}
}

ULTGoreDecalSubsystem* ULTGoreDecalSubsystem::Get(const UWorld* World)
{
	return World ? World->GetSubsystem<ULTGoreDecalSubsystem>() : nullptr;
}

bool ULTGoreDecalSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// Gameplay worlds only. An editor preview or an asset thumbnail world has no
	// combat in it and should not be carrying a decal pool.
	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr && World->IsGameWorld();
}

void ULTGoreDecalSubsystem::Deinitialize()
{
	ClearDecals();

	Super::Deinitialize();
}

void ULTGoreDecalSubsystem::SetDecalMaterials(UMaterialInterface* Impact, UMaterialInterface* DeathPool)
{
	ImpactDecalMaterial = Impact;
	DeathPoolDecalMaterial = DeathPool;

	LT_LOG(
		Log, TEXT("Gore decal materials set: impact %s, death pool %s"), *GetNameSafe(Impact), *GetNameSafe(DeathPool));
}

void ULTGoreDecalSubsystem::SpawnBloodDecal(const FVector& Location, const FVector& Normal, const bool bHeadshot)
{
	if (!ImpactDecalMaterial)
	{
		return;
	}

	FVector SurfaceLocation = FVector::ZeroVector;
	FVector SurfaceNormal = FVector::UpVector;
	if (!FindSpatterSurface(Location, Normal, SurfaceLocation, SurfaceNormal))
	{
		// Nothing within reach to mark. Drawing nothing is the restrained answer,
		// and the better one: a decal with no surface under it floats in mid air.
		return;
	}

	const int32 Count = FMath::Max(1, bHeadshot ? HeadshotDecalCount : ImpactDecalCount);
	const float Multiplier = bHeadshot ? HeadshotSizeMultiplier : 1.f;

	for (int32 Index = 0; Index < Count; ++Index)
	{
		FVector Offset = FVector::ZeroVector;
		if (Index > 0)
		{
			// Lay the extra spatters along the surface rather than stacking them
			// on the first, so a headshot reads as a spray and not as one blot.
			const FVector Tangent = FVector::CrossProduct(SurfaceNormal, FMath::VRand()).GetSafeNormal();
			Offset = Tangent * FMath::FRandRange(10.f, 26.f);
		}

		PlaceDecal(
			ImpactDecalMaterial, SurfaceLocation + Offset, SurfaceNormal, JitterSize(ImpactDecalSize, Multiplier));
	}
}

void ULTGoreDecalSubsystem::SpawnDeathPool(const FVector& Location, const bool bHeadshot)
{
	if (!DeathPoolDecalMaterial)
	{
		return;
	}

	FVector GroundLocation = FVector::ZeroVector;
	FVector GroundNormal = FVector::UpVector;
	if (!FindGroundSurface(Location, GroundLocation, GroundNormal))
	{
		return;
	}

	// One pool per corpse. The death moment is pronounced because the decal is
	// larger and sits under the body, not because there are more of them.
	const float Multiplier = bHeadshot ? HeadshotSizeMultiplier : 1.f;
	PlaceDecal(DeathPoolDecalMaterial, GroundLocation, GroundNormal, JitterSize(DeathPoolDecalSize, Multiplier));
}

void ULTGoreDecalSubsystem::ClearDecals()
{
	for (TObjectPtr<UDecalComponent>& Decal : Decals)
	{
		if (Decal)
		{
			Decal->DestroyComponent();
		}
	}

	Decals.Reset();
	NextDecalIndex = 0;
}

UDecalComponent* ULTGoreDecalSubsystem::PlaceDecal(
	UMaterialInterface* Material, const FVector& Location, const FVector& Normal, const FVector& Size)
{
	UWorld* World = GetWorld();
	if (!Material || !World)
	{
		return nullptr;
	}

	const int32 Cap = FMath::Max(1, MaxActiveDecals);

	// A cap lowered at runtime takes effect on the next placement rather than
	// waiting for the ring to wrap round to the surplus entries.
	while (Decals.Num() > Cap)
	{
		const int32 LastIndex = Decals.Num() - 1;
		if (UDecalComponent* Surplus = Decals[LastIndex])
		{
			Surplus->DestroyComponent();
		}
		Decals.RemoveAt(LastIndex);
	}

	UDecalComponent* Decal = nullptr;
	if (Decals.Num() < Cap)
	{
		Decal = NewObject<UDecalComponent>(World, NAME_None, RF_Transient);
		if (!Decal)
		{
			return nullptr;
		}

		Decal->bAllowAnyoneToDestroyMe = true;
		Decal->SetUsingAbsoluteScale(true);
		Decal->RegisterComponentWithWorld(World);
		Decals.Add(Decal);
	}
	else
	{
		if (!Decals.IsValidIndex(NextDecalIndex))
		{
			NextDecalIndex = 0;
		}

		Decal = Decals[NextDecalIndex];

		// A pooled component can still go away underneath the ring, so rebuild
		// the slot rather than losing it for the rest of the round.
		if (!Decal)
		{
			Decal = NewObject<UDecalComponent>(World, NAME_None, RF_Transient);
			if (!Decal)
			{
				return nullptr;
			}

			Decal->bAllowAnyoneToDestroyMe = true;
			Decal->SetUsingAbsoluteScale(true);
			Decal->RegisterComponentWithWorld(World);
			Decals[NextDecalIndex] = Decal;
		}

		NextDecalIndex = (NextDecalIndex + 1) % Decals.Num();
	}

	// The decal projects down its own negative X, so the rotation is built from
	// the surface normal. A random roll keeps repeated hits from stamping the
	// same orientation over and over.
	FRotator Rotation = Normal.Rotation();
	Rotation.Roll = FMath::FRandRange(-180.f, 180.f);

	Decal->SetDecalMaterial(Material);
	Decal->DecalSize = Size;
	Decal->FadeScreenSize = DecalFadeScreenSize;
	Decal->SetWorldLocationAndRotation(Location, Rotation);
	Decal->SetVisibility(true);
	Decal->SetFadeOut(DecalLifetimeSeconds, DecalFadeSeconds, false);
	Decal->MarkRenderStateDirty();

	return Decal;
}

bool ULTGoreDecalSubsystem::FindSpatterSurface(
	const FVector& Location, const FVector& Normal, FVector& OutLocation, FVector& OutNormal) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Static and dynamic world geometry only. Pawns are deliberately excluded:
	// a deferred decal projected onto a skeletal mesh slides about with the mesh,
	// and the blood on the zombies themselves is the job of the zombie material's
	// own blood layer, not of this pool.
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LTGoreSpatterTrace), false);

	const FVector Direction = Normal.GetSafeNormal();
	if (!Direction.IsNearlyZero())
	{
		// Carry on the way the shot was travelling, which is away from the
		// surface normal the hit reported, and mark whatever is behind the body.
		FHitResult Hit;
		const FVector End = Location - Direction * SpatterTraceDistance;
		if (World->LineTraceSingleByObjectType(Hit, Location, End, ObjectParams, Params))
		{
			OutLocation = Hit.ImpactPoint;
			OutNormal = Hit.ImpactNormal;
			return true;
		}
	}

	// Nothing behind the target, so let it fall to the platform instead.
	return FindGroundSurface(Location, OutLocation, OutNormal);
}

bool ULTGoreDecalSubsystem::FindGroundSurface(const FVector& Origin, FVector& OutLocation, FVector& OutNormal) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LTGoreGroundTrace), false);

	const FVector Start = Origin + FVector(0.f, 0.f, 20.f);
	const FVector End = Origin - FVector(0.f, 0.f, FMath::Max(1.f, GroundTraceDistance));

	FHitResult Hit;
	if (!World->LineTraceSingleByObjectType(Hit, Start, End, ObjectParams, Params))
	{
		return false;
	}

	OutLocation = Hit.ImpactPoint;
	OutNormal = Hit.ImpactNormal;
	return true;
}

FVector ULTGoreDecalSubsystem::JitterSize(const FVector& Size, const float Multiplier) const
{
	const float Jitter = FMath::Clamp(SizeJitterFraction, 0.f, 0.9f);
	const float Scale = Multiplier * FMath::FRandRange(1.f - Jitter, 1.f + Jitter);

	// X is the projection depth and wants to stay put: a jittered depth makes a
	// decal miss a surface it was sitting on.
	return FVector(Size.X, Size.Y * Scale, Size.Z * Scale);
}

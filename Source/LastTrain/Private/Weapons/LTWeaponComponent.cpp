#include "Weapons/LTWeaponComponent.h"

#include "Economy/LTPointsComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "Weapons/LTWeaponData.h"
#include "Zombies/LTZombieCharacter.h"

ULTWeaponComponent::ULTWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULTWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		Points = Owner->FindComponentByClass<ULTPointsComponent>();
	}

	if (WeaponData)
	{
		SetWeapon(WeaponData);
	}
}

void ULTWeaponComponent::SetWeapon(ULTWeaponData* NewWeapon, const bool bRefillReserve)
{
	if (!NewWeapon)
	{
		return;
	}

	WeaponData = NewWeapon;
	bReloading = false;
	ReloadRemaining = 0.f;
	BloomDegrees = 0.f;
	TimeUntilNextShot = 0.f;

	Magazine = NewWeapon->MagazineSize;
	if (bRefillReserve)
	{
		Reserve = NewWeapon->MaxReserve;
	}

	OnAmmoChanged.Broadcast(Magazine, Reserve);
}

void ULTWeaponComponent::TickComponent(
	const float DeltaSeconds, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

	if (!WeaponData)
	{
		return;
	}

	// Sprinting is handled by the character, which drops aiming before this sees it.
	const float TransitionRate = DeltaSeconds / FMath::Max(WeaponData->AimTransitionSeconds, KINDA_SMALL_NUMBER);
	AimAlpha = FMath::Clamp(AimAlpha + (bAiming ? TransitionRate : -TransitionRate), 0.f, 1.f);

	BloomDegrees = FMath::Max(0.f, BloomDegrees - WeaponData->BloomDecayPerSecond * DeltaSeconds);

	if (bReloading)
	{
		ReloadRemaining -= DeltaSeconds;
		if (ReloadRemaining <= 0.f)
		{
			FinishReload();
		}
		return;
	}

	TimeUntilNextShot = FMath::Max(0.f, TimeUntilNextShot - DeltaSeconds);

	if (bFiring && TimeUntilNextShot <= 0.f)
	{
		FireOnce();

		// A single shot weapon requires the trigger to be released.
		if (!WeaponData->bAutomatic)
		{
			bFiring = false;
		}
	}
}

void ULTWeaponComponent::StartFiring()
{
	if (!WeaponData || bReloading)
	{
		return;
	}

	if (Magazine <= 0)
	{
		StartReload();
		return;
	}

	bFiring = true;
}

void ULTWeaponComponent::StopFiring()
{
	bFiring = false;
}

void ULTWeaponComponent::SetAiming(const bool bNewAiming)
{
	bAiming = bNewAiming;
}

void ULTWeaponComponent::StartReload()
{
	if (!WeaponData || bReloading || Reserve <= 0 || Magazine >= WeaponData->MagazineSize)
	{
		return;
	}

	bReloading = true;
	bFiring = false;
	ReloadRemaining = WeaponData->ReloadSeconds;
}

void ULTWeaponComponent::FinishReload()
{
	bReloading = false;
	ReloadRemaining = 0.f;

	if (!WeaponData)
	{
		return;
	}

	const int32 Wanted = WeaponData->MagazineSize - Magazine;
	const int32 Taken = FMath::Min(Wanted, Reserve);

	Magazine += Taken;
	Reserve -= Taken;

	OnAmmoChanged.Broadcast(Magazine, Reserve);
}

void ULTWeaponComponent::RefillAmmunition()
{
	if (!WeaponData)
	{
		return;
	}

	Reserve = WeaponData->MaxReserve;
	OnAmmoChanged.Broadcast(Magazine, Reserve);
}

float ULTWeaponComponent::GetCurrentSpreadDegrees() const
{
	if (!WeaponData)
	{
		return 0.f;
	}

	const float Base = FMath::Lerp(WeaponData->HipSpreadDegrees, WeaponData->AimedSpreadDegrees, AimAlpha);

	float MovementFraction = 0.f;
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		const float Speed = Pawn->GetVelocity().Size2D();
		MovementFraction = FMath::Clamp(Speed / 600.f, 0.f, 1.f);
	}

	return Base + WeaponData->MovementSpreadDegrees * MovementFraction + BloomDegrees;
}

float ULTWeaponComponent::GetMoveScale() const
{
	return WeaponData ? FMath::Lerp(1.f, WeaponData->AimedMoveScale, AimAlpha) : 1.f;
}

void ULTWeaponComponent::GetViewPoint(FVector& OutLocation, FVector& OutDirection) const
{
	OutLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	OutDirection = GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;

	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		FRotator ViewRotation;
		Pawn->GetActorEyesViewPoint(OutLocation, ViewRotation);
		OutDirection = ViewRotation.Vector();
	}
}

FVector ULTWeaponComponent::ApplySpread(const FVector& Direction, const float SpreadDegrees) const
{
	if (SpreadDegrees <= 0.f)
	{
		return Direction;
	}

	// Square root of a uniform variable gives uniform density over the disc.
	// Without it, pellets cluster centrally and a shotgun reads as one hole.
	const float MaxAngle = FMath::DegreesToRadians(SpreadDegrees);
	const float Angle = MaxAngle * FMath::Sqrt(FMath::FRand());

	return FMath::VRandCone(Direction, Angle);
}

void ULTWeaponComponent::FireOnce()
{
	if (!WeaponData || Magazine <= 0)
	{
		return;
	}

	Magazine -= 1;
	TimeUntilNextShot = WeaponData->GetShotInterval();

	FVector Origin;
	FVector Forward;
	GetViewPoint(Origin, Forward);

	const float Spread = GetCurrentSpreadDegrees();
	const int32 Pellets = FMath::Max(1, WeaponData->PelletsPerShot);

	for (int32 Pellet = 0; Pellet < Pellets; ++Pellet)
	{
		bool bHeadshot = false;
		if (TracePellet(Origin, ApplySpread(Forward, Spread), bHeadshot))
		{
			OnHitConfirmed.Broadcast(bHeadshot);
		}
	}

	BloomDegrees = FMath::Min(WeaponData->BloomMaxDegrees, BloomDegrees + WeaponData->BloomPerShotDegrees);

	if (WeaponData->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, Origin);
	}

	OnAmmoChanged.Broadcast(Magazine, Reserve);

	if (Magazine <= 0)
	{
		StartReload();
	}
}

bool ULTWeaponComponent::TracePellet(const FVector& Origin, const FVector& Direction, bool& bOutHeadshot)
{
	bOutHeadshot = false;

	UWorld* World = GetWorld();
	if (!World || !WeaponData)
	{
		return false;
	}

	const FVector End = Origin + Direction * WeaponData->MaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LTWeaponTrace), true, GetOwner());
	Params.bReturnPhysicalMaterial = false;
	Params.bTraceComplex = true;

	// Trace through zombies until the penetration budget is spent. Geometry always stops the shot.
	int32 Remaining = FMath::Max(1, WeaponData->Penetration);
	FVector TraceStart = Origin;
	bool bHitAnything = false;

	while (Remaining > 0)
	{
		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, TraceStart, End, ECC_GameTraceChannel1, Params))
		{
			break;
		}

		ALTZombieCharacter* Zombie = Cast<ALTZombieCharacter>(Hit.GetActor());
		if (!Zombie)
		{
			// Walls stop the shot.
			break;
		}

		const bool bHeadshot = Zombie->IsHeadBone(Hit.BoneName);
		const float Distance = FVector::Dist(Origin, Hit.ImpactPoint);

		float Damage = WeaponData->BaseDamage;
		if (Distance > WeaponData->FalloffStart)
		{
			const float Span = FMath::Max(WeaponData->FalloffEnd - WeaponData->FalloffStart, 1.f);
			const float Alpha = FMath::Clamp((Distance - WeaponData->FalloffStart) / Span, 0.f, 1.f);
			Damage = FMath::Lerp(Damage, Damage * 0.45f, Alpha);
		}

		if (bHeadshot)
		{
			Damage *= WeaponData->HeadshotMultiplier;
		}

		Zombie->ReceiveShot(Damage, bHeadshot, Hit, Direction, GetOwner());

		bHitAnything = true;
		bOutHeadshot = bOutHeadshot || bHeadshot;

		// Kills are awarded from the zombie's death broadcast, so this is the hit award only.
		if (Points && !Zombie->IsDead())
		{
			Points->AwardHit();
		}

		Params.AddIgnoredActor(Zombie);
		TraceStart = Hit.ImpactPoint;
		Remaining -= 1;
	}

	return bHitAnything;
}

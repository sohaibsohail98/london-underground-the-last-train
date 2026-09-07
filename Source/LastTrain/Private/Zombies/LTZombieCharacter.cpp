#include "Zombies/LTZombieCharacter.h"

#include "AIController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Economy/LTPointsComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "Navigation/PathFollowingComponent.h"

ALTZombieCharacter::ALTZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Per-bone collision, so head shots are distinguishable.
	if (USkeletalMeshComponent* Mesh = GetMesh())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
		Mesh->SetGenerateOverlapEvents(false);
	}

	// The capsule must ignore weapon traces or every shot reports a body hit.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = BaseWalkSpeed;
		// RVO keeps the crowd from stacking on one navmesh point. Kept tight so it
		// separates neighbouring zombies without braking the final approach to the
		// player, which was leaving them parked outside AttackRange.
		Movement->bUseRVOAvoidance = true;
		Movement->AvoidanceConsiderationRadius = 45.f;
	}
}

void ALTZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Health <= 0.f)
	{
		Health = BaseHealth;
	}

	CurrentTarget = UGameplayStatics::GetPlayerPawn(this, 0);

	RepathTimer = FMath::FRandRange(0.f, RepathIntervalSeconds);

	// A fixed per-instance sign, so a stalled zombie always shoulders past on the
	// same side and the queue fans out instead of oscillating in place.
	StallLateralSign = FMath::RandBool() ? 1.f : -1.f;
}

void ALTZombieCharacter::ApplyRoundScaling(const int32 Round)
{
	const int32 Effective = FMath::Max(1, Round);

	AppliedRound = Effective;

	// The type multipliers are 1 until a type asset is applied, so this is the
	// unchanged Phase B curve for a zombie with no type.
	Health = BaseHealth * FMath::Pow(HealthGrowthPerRound, static_cast<float>(Effective - 1)) * TypeHealthMultiplier;

	int32 Steps = 0;
	for (const int32 StepRound : SpeedStepRounds)
	{
		if (Effective >= StepRound)
		{
			Steps += 1;
		}
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// The type scales the base, then the round steps are added on top, so a
		// sprinter does not have its steps multiplied as well.
		Movement->MaxWalkSpeed = BaseWalkSpeed * TypeWalkSpeedMultiplier + SpeedPerStep * static_cast<float>(Steps);
	}
}

void ALTZombieCharacter::ApplyTypeData(const ULTZombieTypeData* Data)
{
	if (!Data)
	{
		return;
	}

	ZombieType = Data->Type;
	Behaviour = Data->Behaviour;

	TypeHealthMultiplier = FMath::Max(0.01f, Data->HealthMultiplier);
	TypeWalkSpeedMultiplier = FMath::Max(0.01f, Data->WalkSpeedMultiplier);

	// Re-run the round curve rather than multiplying whatever health and speed
	// happen to be set. Applying a type is then idempotent and does not care
	// whether ApplyRoundScaling ran before or after it.
	ApplyRoundScaling(AppliedRound);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (Data->AvoidanceConsiderationRadiusOverride > 0.f)
		{
			Movement->AvoidanceConsiderationRadius = Data->AvoidanceConsiderationRadiusOverride;
		}
	}

	if (Data->AttackDamageOverride > 0.f)
	{
		AttackDamage = Data->AttackDamageOverride;
	}
	if (Data->AttackCooldownOverride > 0.f)
	{
		AttackCooldownSeconds = Data->AttackCooldownOverride;
	}
	if (Data->AttackRangeOverride > 0.f)
	{
		AttackRange = Data->AttackRangeOverride;
	}
	if (Data->RepathIntervalOverride > 0.f)
	{
		RepathIntervalSeconds = Data->RepathIntervalOverride;
		RepathTimer = FMath::Min(RepathTimer, RepathIntervalSeconds);
	}
	if (Data->ContactRangeOverride > 0.f)
	{
		ContactRange = Data->ContactRangeOverride;
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		const float OldRadius = Capsule->GetUnscaledCapsuleRadius();
		const float OldHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
		const float NewRadius = Data->CapsuleRadiusOverride > 0.f ? Data->CapsuleRadiusOverride : OldRadius;
		const float NewHalfHeight =
			Data->CapsuleHalfHeightOverride > 0.f ? Data->CapsuleHalfHeightOverride : OldHalfHeight;

		if (!FMath::IsNearlyEqual(NewRadius, OldRadius) || !FMath::IsNearlyEqual(NewHalfHeight, OldHalfHeight))
		{
			Capsule->SetCapsuleSize(NewRadius, NewHalfHeight);

			// Keep the feet on the capsule base, the Character default relationship.
			// Only Z moves, so a Blueprint's own mesh offset survives.
			if (USkeletalMeshComponent* Mesh = GetMesh())
			{
				FVector MeshOffset = Mesh->GetRelativeLocation();
				MeshOffset.Z = -NewHalfHeight;
				Mesh->SetRelativeLocation(MeshOffset);
			}

			// The round manager lifts every spawn by one walker half height, so a
			// taller capsule would arrive part sunk in the floor and a shorter one
			// hovering. Shift the actor by the difference.
			const float HalfHeightDelta = NewHalfHeight - OldHalfHeight;
			if (!FMath::IsNearlyZero(HalfHeightDelta))
			{
				AddActorWorldOffset(FVector(0.f, 0.f, HalfHeightDelta));
			}
		}
	}

	// Silhouette is how a type reads at a glance. The tint is deliberately not
	// applied here: it comes from a shared tintable material with one instance per
	// type, picked by the spawn Blueprint, never a dynamic instance per spawn.
	if (Data->MeshScale > 0.f)
	{
		if (USkeletalMeshComponent* Mesh = GetMesh())
		{
			Mesh->SetWorldScale3D(FVector(Data->MeshScale));
		}
	}

	AnimPlayRate = Data->AnimPlayRate;
	bRagdollOnDeath = Data->bRagdollOnDeath;
	DeathScreenShakeRadius = Data->DeathScreenShakeRadius;
	TypeCorpseLifetime = Data->CorpseLifetimeOverride;

	switch (Behaviour)
	{
	case ELTZombieBehaviour::Sprint:
		LungeImpulse = Data->SprintLungeImpulse;
		break;
	case ELTZombieBehaviour::ArmourPlate:
		ArmourRemaining = Data->ArmourBodyDamageToBreak;
		break;
	case ELTZombieBehaviour::Scream:
		ScreamSightSeconds = Data->ScreamLineOfSightSeconds;
		ScreamWalkerCount = Data->ScreamSummonCount;
		ScreamCancelSeconds = Data->ScreamCancelWindowSeconds;
		ScreamSightTimer = 0.f;
		bHasScreamed = false;
		break;
	case ELTZombieBehaviour::None:
	case ELTZombieBehaviour::LowProfile:
		// Nothing to seed. The low profile is entirely capsule and pose.
		break;
	}

	const UCharacterMovementComponent* Movement = GetCharacterMovement();

	LT_LOG(
		Log, TEXT("Applied type %s: health %.0f, speed %.0f, scale %.2f."), *Data->DisplayName.ToString(), Health,
		Movement ? Movement->MaxWalkSpeed : 0.f, Data->MeshScale);
}

void ALTZombieCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
	{
		return;
	}

	AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaSeconds);

	if (!CurrentTarget)
	{
		CurrentTarget = UGameplayStatics::GetPlayerPawn(this, 0);
		return;
	}

	RepathTimer -= DeltaSeconds;
	if (RepathTimer <= 0.f)
	{
		DriveTowardsTarget();

		const float Jitter = RepathIntervalSeconds * RepathJitterFraction;
		RepathTimer = RepathIntervalSeconds + FMath::FRandRange(-Jitter, Jitter);
	}

	UpdateStallRecovery(DeltaSeconds);

	if (Behaviour == ELTZombieBehaviour::Scream)
	{
		UpdateScream(DeltaSeconds);
	}

	TryAttack();
}

void ALTZombieCharacter::DriveTowardsTarget()
{
	if (!CurrentTarget)
	{
		return;
	}

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI)
	{
		// No controller yet. Push straight at the target so a freshly spawned
		// zombie still closes while possession settles.
		AddMovementInput((CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal());
		return;
	}

	// Re-issued every repath interval, in contact or not, so the zombie tracks a
	// moving player instead of parking on a stale arrival. ContactRange is edge to
	// edge, small enough that the zombie presses right up to the player.
	const EPathFollowingRequestResult::Type Result = AI->MoveToActor(CurrentTarget, ContactRange);

	if (Result == EPathFollowingRequestResult::Failed)
	{
		// Navmesh could not path there this frame (player off the mesh, or the
		// crowd is blocking every poly). Fall back to a direct push so the zombie
		// never freezes in place waiting on a path that will not come.
		AddMovementInput((CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal());
	}
}

void ALTZombieCharacter::UpdateStallRecovery(const float DeltaSeconds)
{
	if (!CurrentTarget)
	{
		EndStallRecovery();
		return;
	}

	// One in melee range is meant to be stationary while it swings. Only a
	// zombie with ground still to cover counts as stalled.
	const float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance <= AttackRange)
	{
		EndStallRecovery();
		return;
	}

	const bool bBarelyMoving = GetVelocity().SizeSquared2D() < FMath::Square(StallSpeedThreshold);

	if (bStallRecovering)
	{
		// Already shoving. Keep going until the zombie is moving again or has
		// closed to melee range, then hand control back to path following.
		if (!bBarelyMoving)
		{
			EndStallRecovery();
			return;
		}
		DriveStallNudge();
		return;
	}

	if (bBarelyMoving)
	{
		StallTimer += DeltaSeconds;
	}
	else
	{
		StallTimer = 0.f;
		return;
	}

	if (StallTimer >= StallGraceSeconds)
	{
		BeginStallRecovery();
		DriveStallNudge();
	}
}

void ALTZombieCharacter::BeginStallRecovery()
{
	bStallRecovering = true;

	// The path following component sets velocity every frame while a MoveTo is
	// active, which clobbers AddMovementInput. Cancel the request so the
	// movement component honours the direct push.
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}

	// RVO braking is what pins a boxed in zombie at zero velocity. Drop it for
	// the duration of the shove so the zombie can push through the crowd.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bUseRVOAvoidance = false;
	}

	LT_LOG(Verbose, TEXT("%s entering stall recovery."), *GetName());
}

void ALTZombieCharacter::EndStallRecovery()
{
	StallTimer = 0.f;

	if (!bStallRecovering)
	{
		return;
	}

	bStallRecovering = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bUseRVOAvoidance = true;
	}

	// Next Tick repath will re-issue a MoveToActor.
	RepathTimer = 0.f;
}

void ALTZombieCharacter::DriveStallNudge()
{
	if (!CurrentTarget)
	{
		return;
	}

	// Shove toward the target with a lateral bias, so the blocked zombie slides
	// around the one in front rather than pressing straight into its back.
	const FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	const FVector Lateral = FVector::CrossProduct(ToTarget, FVector::UpVector) * StallLateralSign;
	const FVector Nudge = (ToTarget + Lateral * StallLateralFraction).GetSafeNormal2D();
	AddMovementInput(Nudge, StallNudgeScale);
}

void ALTZombieCharacter::TryAttack()
{
	if (AttackCooldown > 0.f || !CurrentTarget)
	{
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > AttackRange)
	{
		return;
	}

	// AttackRange is a forgiving root to root gate. It is wide on purpose: it must
	// still catch a player strafing out of a stationary zombie's reach. Capsule
	// contact sits near 72 units root to root, so the slack up to 130 is the
	// window a moving player can be clipped in.

	AttackCooldown = AttackCooldownSeconds;

	OnAttackWindUp();

	// A short forward lunge, so a sprinter's swing closes the last of the gap
	// rather than swiping at air the player has already left.
	if (Behaviour == ELTZombieBehaviour::Sprint && LungeImpulse > 0.f)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->AddImpulse(GetActorForwardVector() * LungeImpulse, true);
		}
	}

	UGameplayStatics::ApplyDamage(CurrentTarget, AttackDamage, GetController(), this, nullptr);
}

void ALTZombieCharacter::UpdateScream(const float DeltaSeconds)
{
	if (bHasScreamed || ScreamSightSeconds <= 0.f)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!World || !CameraManager)
	{
		return;
	}

	// Geometry blocks the sight line. Other zombies do not: their capsules are on
	// the Pawn profile and their meshes answer only the weapon channel, so
	// ECC_Visibility passes straight through a crowd. Darkness is not occlusion.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LTZombieScreamSight), false, this);
	if (CurrentTarget)
	{
		Params.AddIgnoredActor(CurrentTarget);
	}

	FHitResult Blocker;
	const bool bBlocked = World->LineTraceSingleByChannel(
		Blocker, GetHeadLocation(), CameraManager->GetCameraLocation(), ECC_Visibility, Params);

	if (bBlocked)
	{
		ScreamSightTimer = 0.f;
		return;
	}

	ScreamSightTimer += DeltaSeconds;
	if (ScreamSightTimer < ScreamSightSeconds)
	{
		return;
	}

	bHasScreamed = true;
	ScreamStartTime = World->GetTimeSeconds();

	OnScream();

	if (ScreamWalkerCount > 0)
	{
		OnZombieScreamed.Broadcast(this, ScreamWalkerCount);
	}

	LT_LOG(
		Log, TEXT("%s screamed after %.1fs of sight. Calling %d walkers."), *GetName(), ScreamSightSeconds,
		ScreamWalkerCount);
}

FVector ALTZombieCharacter::GetHeadLocation() const
{
	if (const USkeletalMeshComponent* Mesh = GetMesh())
	{
		for (const FName& BoneName : HeadBoneNames)
		{
			if (Mesh->DoesSocketExist(BoneName))
			{
				return Mesh->GetSocketLocation(BoneName);
			}
		}
	}

	return GetActorLocation() + FVector(0.f, 0.f, BaseEyeHeight);
}

bool ALTZombieCharacter::IsHeadBone(const FName BoneName) const
{
	return HeadBoneNames.Contains(BoneName);
}

void ALTZombieCharacter::ReceiveShot(
	const float Damage, const bool bHeadshot, const FHitResult& Hit, const FVector& ShotDirection,
	AActor* ShotInstigator)
{
	if (bDead)
	{
		return;
	}

	// The plate is on the front only. A shot travelling roughly the way the brute
	// is facing came from behind it, so it lands normally, as does a headshot.
	if (ArmourRemaining > 0.f && !bHeadshot)
	{
		const bool bFrontHit = FVector::DotProduct(ShotDirection.GetSafeNormal(), GetActorForwardVector()) < 0.f;
		if (bFrontHit)
		{
			ArmourRemaining = FMath::Max(0.f, ArmourRemaining - Damage);

			// No health lost and no impulse. A hit reaction Blueprint reads
			// GetArmourRemaining to play a blocked response rather than a wounded one.
			OnHitReaction(Hit, bHeadshot);
			return;
		}
	}

	Health -= Damage;

	OnHitReaction(Hit, bHeadshot);

	if (Health <= 0.f)
	{
		Die(bHeadshot, ShotInstigator);
		return;
	}

	// Placeholder until hit reaction montages land in the art pass.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->AddImpulse(ShotDirection.GetSafeNormal() * 400.f, true);
	}
}

void ALTZombieCharacter::Die(const bool bHeadshot, AActor* Killer)
{
	if (bDead)
	{
		return;
	}

	bDead = true;

	if (AController* MyController = GetController())
	{
		MyController->StopMovement();
		MyController->UnPossess();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* Mesh = GetMesh())
	{
		Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	}

	if (Killer)
	{
		if (ULTPointsComponent* KillerPoints = Killer->FindComponentByClass<ULTPointsComponent>())
		{
			KillerPoints->AwardKill(bHeadshot);
		}
	}

	// Killing a screamer quickly is the counterplay, so a death inside the cancel
	// window drops the wave it called. Broadcast before OnZombieDied, which is
	// what makes the round manager unbind from this zombie.
	if (bHasScreamed && ScreamCancelSeconds > 0.f)
	{
		const UWorld* World = GetWorld();
		if (World && World->GetTimeSeconds() - ScreamStartTime <= ScreamCancelSeconds)
		{
			OnZombieScreamed.Broadcast(this, 0);
		}
	}

	OnDeathPresentation(bHeadshot);
	OnZombieDied.Broadcast(this, bHeadshot);

	SetLifeSpan(TypeCorpseLifetime > 0.f ? TypeCorpseLifetime : CorpseLifetime);
}

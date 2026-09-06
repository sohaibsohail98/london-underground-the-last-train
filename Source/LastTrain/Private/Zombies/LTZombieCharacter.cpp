#include "Zombies/LTZombieCharacter.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Economy/LTPointsComponent.h"
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
}

void ALTZombieCharacter::ApplyRoundScaling(const int32 Round)
{
	const int32 Effective = FMath::Max(1, Round);

	Health = BaseHealth * FMath::Pow(HealthGrowthPerRound, static_cast<float>(Effective - 1));

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
		Movement->MaxWalkSpeed = BaseWalkSpeed + SpeedPerStep * static_cast<float>(Steps);
	}
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

void ALTZombieCharacter::TryAttack()
{
	if (AttackCooldown > 0.f || !CurrentTarget)
	{
		// DIAGNOSTIC, remove after the attack bug is confirmed.
		LT_LOG(Verbose, TEXT("TryAttack early out: cooldown %.2f target %s"), AttackCooldown,
			CurrentTarget ? TEXT("set") : TEXT("null"));
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > AttackRange)
	{
		// DIAGNOSTIC, remove after the attack bug is confirmed.
		LT_LOG(Verbose, TEXT("TryAttack out of range: distance %.1f range %.1f"), Distance, AttackRange);
		return;
	}

	// AttackRange is a forgiving root to root gate. It is wide on purpose: it must
	// still catch a player strafing out of a stationary zombie's reach. Capsule
	// contact sits near 72 units root to root, so the slack up to 130 is the
	// window a moving player can be clipped in.

	AttackCooldown = AttackCooldownSeconds;

	// DIAGNOSTIC, remove after the attack bug is confirmed.
	LT_LOG(Log, TEXT("TryAttack firing ApplyDamage %.1f on %s at distance %.1f"), AttackDamage,
		*CurrentTarget->GetName(), Distance);

	UGameplayStatics::ApplyDamage(CurrentTarget, AttackDamage, GetController(), this, nullptr);
}

bool ALTZombieCharacter::IsHeadBone(const FName BoneName) const
{
	return HeadBoneNames.Contains(BoneName);
}

void ALTZombieCharacter::ReceiveShot(
	const float Damage, const bool bHeadshot, const FHitResult& Hit, const FVector& ShotDirection, AActor* ShotInstigator)
{
	if (bDead)
	{
		return;
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

	OnDeathPresentation(bHeadshot);
	OnZombieDied.Broadcast(this, bHeadshot);

	SetLifeSpan(CorpseLifetime);
}

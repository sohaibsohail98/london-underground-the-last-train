#include "Zombies/LTZombieCharacter.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Economy/LTPointsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"

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
		Movement->bUseRVOAvoidance = true;
		Movement->AvoidanceConsiderationRadius = 90.f;
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

	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->MoveToActor(CurrentTarget, AttackRange * 0.75f);
	}

	TryAttack();
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

	AttackCooldown = AttackCooldownSeconds;

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

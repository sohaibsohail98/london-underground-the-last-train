#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LTZombieCharacter.generated.h"

class ULTPointsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZombieDied, ALTZombieCharacter*, Zombie, bool, bHeadshot);

/** Health, damage, attack timing and death. Navigation belongs to the AI controller. */
UCLASS()
class LASTTRAIN_API ALTZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALTZombieCharacter();

	/** Broadcast once, on death. The round manager counts these. */
	UPROPERTY(BlueprintAssignable, Category = "Zombie")
	FOnZombieDied OnZombieDied;

	virtual void Tick(float DeltaSeconds) override;

	/** Applies a hit from the weapon component. */
	UFUNCTION(BlueprintCallable, Category = "Zombie")
	void
	ReceiveShot(float Damage, bool bHeadshot, const FHitResult& Hit, const FVector& ShotDirection, AActor* ShotInstigator);

	/** Scales health and speed for the given round. Called on spawn. */
	UFUNCTION(BlueprintCallable, Category = "Zombie")
	void ApplyRoundScaling(int32 Round);

	UFUNCTION(BlueprintPure, Category = "Zombie")
	bool IsDead() const { return bDead; }

	/** True if the bone belongs to the head hitbox. */
	UFUNCTION(BlueprintPure, Category = "Zombie")
	bool IsHeadBone(FName BoneName) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float BaseHealth = 150.f;

	/** Compounded per round. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float HealthGrowthPerRound = 1.1f;

	/** Rounds at which the walk speed steps up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	TArray<int32> SpeedStepRounds = {5, 10, 20};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float BaseWalkSpeed = 130.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float SpeedPerStep = 55.f;

	/** Root to root reach for the melee gate. Generous, since capsule contact is
	    about 72 units root to root and a strafing player should still be hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 130.f;

	/** Acceptance radius passed to MoveToActor, edge to edge. Kept small so the
	    zombie presses into contact instead of stopping short and freezing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float ContactRange = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackDamage = 24.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackCooldownSeconds = 1.3f;

	/** Seconds between navmesh repath requests. Path following steers between them.
	    The request is re-issued every interval even in contact, so the zombie
	    tracks a moving player rather than sitting on a stale arrival. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float RepathIntervalSeconds = 0.35f;

	/** Fraction of the interval added as a random per-instance offset, so repaths
	    across the crowd do not land on the same frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float RepathJitterFraction = 0.4f;

	/** Ground speed below which the zombie counts as stalled. Path following can
	    report success while a capsule ahead pins it at zero velocity. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StallSpeedThreshold = 8.f;

	/** Seconds of near-zero velocity outside AttackRange before the direct nudge
	    kicks in. Short enough that a corridor queue never visibly freezes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StallGraceSeconds = 0.5f;

	/** How much of the stall nudge is lateral rather than straight at the target,
	    so a blocked zombie slides around the one in front. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StallLateralFraction = 0.6f;

	/** Bone names treated as the head. Set to match the imported skeleton. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<FName> HeadBoneNames = {TEXT("head"), TEXT("Head"), TEXT("neck_01")};

	/** Seconds the corpse remains before being destroyed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float CorpseLifetime = 6.f;

protected:
	virtual void BeginPlay() override;

	/** Blueprint hook for hit reaction montages and effects. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnHitReaction(const FHitResult& Hit, bool bHeadshot);

	/** Blueprint hook for the death montage and gore. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnDeathPresentation(bool bHeadshot);

	/** Read only so the attack state is inspectable in the editor and PIE. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat", Transient)
	float Health = 0.f;

	/** Counts down from AttackCooldownSeconds after each hit. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat", Transient)
	float AttackCooldown = 0.f;

private:
	void Die(bool bHeadshot, AActor* Killer);
	void TryAttack();

	/** Issues the move request toward CurrentTarget, with a direct-push fallback
	    if the navmesh cannot path there, so the zombie never freezes. */
	void DriveTowardsTarget();

	/** Detects the path-following-succeeds-but-pinned-by-a-pawn case and applies a
	    lateral direct nudge, so a corridor queue does not stall behind the front
	    attacker. */
	void UpdateStallRecovery(float DeltaSeconds);

	float RepathTimer = 0.f;
	float StallTimer = 0.f;

	/** Fixed +1 or -1 per instance, so a stalled zombie shoulders past on a
	    consistent side and the queue fans out. */
	float StallLateralSign = 1.f;

	bool bDead = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat", Transient,
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> CurrentTarget;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Zombies/LTZombieTypeData.h"
#include "LTZombieCharacter.generated.h"

class ULTPointsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZombieDied, ALTZombieCharacter*, Zombie, bool, bHeadshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZombieScreamed, ALTZombieCharacter*, Screamer, int32, WalkerCount);

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

	/** Fired when a screamer completes its scream, so the round manager queues the
		extra wave. A second broadcast with WalkerCount zero means cancel: the
		screamer died inside its cancel window. */
	UPROPERTY(BlueprintAssignable, Category = "Zombie")
	FOnZombieScreamed OnZombieScreamed;

	virtual void Tick(float DeltaSeconds) override;

	/** Applies a hit from the weapon component. */
	UFUNCTION(BlueprintCallable, Category = "Zombie")
	void ReceiveShot(
		float Damage, bool bHeadshot, const FHitResult& Hit, const FVector& ShotDirection, AActor* ShotInstigator);

	/** Scales health and speed for the given round. Called on spawn. */
	UFUNCTION(BlueprintCallable, Category = "Zombie")
	void ApplyRoundScaling(int32 Round);

	/** Applies every property of the type asset: stats, scale, capsule, navigation
		overrides and the behaviour hook. Called on spawn straight after
		ApplyRoundScaling, so the round-scaled Health and MaxWalkSpeed are already
		set and this multiplies or replaces them. Null is a no-op, leaving a plain
		walker on the coded defaults. */
	UFUNCTION(BlueprintCallable, Category = "Zombie")
	void ApplyTypeData(const ULTZombieTypeData* Data);

	UFUNCTION(BlueprintPure, Category = "Zombie")
	ELTZombieType GetZombieType() const { return ZombieType; }

	/** Anim play rate multiplier from the type asset. The anim Blueprint reads this
		to speed or slow the single shared locomotion set. */
	UFUNCTION(BlueprintPure, Category = "Zombie")
	float GetAnimPlayRate() const { return AnimPlayRate; }

	/** Front body damage the armour plate will still absorb. Zero on a type
		without a plate, and zero once it is broken, so a hit reaction Blueprint can
		read this to play a blocked rather than a wounded response. */
	UFUNCTION(BlueprintPure, Category = "Zombie")
	float GetArmourRemaining() const { return ArmourRemaining; }

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

	/** Movement input scale while shoving out of a stall. Full strength, since
		the point is to break contact with whatever is pinning the zombie. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StallNudgeScale = 1.f;

	/** Longest one shove runs before path following gets the zombie back. Without
		a ceiling, a zombie shoving into geometry would hold RVO off and keep its
		repath suppressed for the rest of its life. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StallRecoverySeconds = 1.5f;

	/** Ground the shove has to cover before the zombie counts as freed. Its own
		displacement, not the distance to the target, so a zombie that shoulders
		clear while the player runs still counts as free. Velocity alone is not
		enough: one pinned against a capsule twitches over StallSpeedThreshold
		without going anywhere. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StallRecoveryProgress = 40.f;

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

	/** Blueprint hook for the death montage and gore. Reads bRagdollOnDeath and
		DeathScreenShakeRadius for the per-type treatment rather than taking them as
		parameters, so the existing BP_Zombie event node keeps its signature. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnDeathPresentation(bool bHeadshot);

	/** Blueprint hook: the sprinter's short lunge, or any per-type attack tell. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnAttackWindUp();

	/** Blueprint hook: the screamer's animation and audio. Fired once, when the
		line-of-sight timer completes. The summon request itself is done in C++. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
	void OnScream();

	/** From the type asset. The death Blueprint reads it: a walker or sprinter
		snaps to a settled pose, a crawler is already prone. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Zombie", Transient)
	bool bRagdollOnDeath = false;

	/** From the type asset. Above zero, the death Blueprint shakes the camera
		within this radius. The brute falling. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Zombie", Transient)
	float DeathScreenShakeRadius = 0.f;

	/** Read only so the attack state is inspectable in the editor and PIE. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat", Transient)
	float Health = 0.f;

	/** Counts down from AttackCooldownSeconds after each hit. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat", Transient)
	float AttackCooldown = 0.f;

private:
	void Die(bool bHeadshot, AActor* Killer);
	void TryAttack();

	/** Screamer only. Accumulates clear line of sight to the player camera and
		fires the scream once it holds long enough. */
	void UpdateScream(float DeltaSeconds);

	/** Trace origin for the line-of-sight test: the first head bone the skeleton
		actually has, or the eye height if it has none of them. */
	FVector GetHeadLocation() const;

	/** Issues the move request toward CurrentTarget, with a direct-push fallback
		if the navmesh cannot path there, so the zombie never freezes. */
	void DriveTowardsTarget();

	/** Detects the path-following-succeeds-but-pinned-by-a-pawn case and drives a
		lateral direct nudge, so a corridor queue does not stall behind the front
		attacker. */
	void UpdateStallRecovery(float DeltaSeconds);

	/** Cancels the AI move request and drops RVO so AddMovementInput can push
		the zombie clear of whatever is pinning it. Banks where the shove started,
		which is the mark it has to beat to count as having worked. */
	void BeginStallRecovery();

	/** Restores RVO and forces a repath. Safe to call when not recovering. */
	void EndStallRecovery();

	/** One frame of the lateral shove toward CurrentTarget. */
	void DriveStallNudge();

	float RepathTimer = 0.f;
	float StallTimer = 0.f;
	bool bStallRecovering = false;

	/** Seconds the current shove has run, against StallRecoverySeconds. */
	float StallRecoveryElapsed = 0.f;

	/** Where the shove began, against StallRecoveryProgress. */
	FVector StallRecoveryStartLocation = FVector::ZeroVector;

	/** Kept so ApplyRoundScaling re-applies the type's health and speed multipliers
		on top of the round curve rather than losing them. Both are 1 until a type
		asset is applied, so a zombie with no type behaves exactly as before. */
	float TypeHealthMultiplier = 1.f;
	float TypeWalkSpeedMultiplier = 1.f;

	/** The round last passed to ApplyRoundScaling, so applying a type can re-run
		the scaling rather than multiplying whatever is there. That keeps
		ApplyTypeData idempotent and free of any ordering requirement. */
	int32 AppliedRound = 1;

	float AnimPlayRate = 1.f;

	/** Above zero, replaces CorpseLifetime when this zombie dies. */
	float TypeCorpseLifetime = 0.f;

	/** Sprint behaviour: forward impulse on the attack wind-up. */
	float LungeImpulse = 0.f;

	/** ArmourPlate behaviour: front body damage still to absorb before the plate
		breaks. Zero once broken, and zero on every other type. */
	float ArmourRemaining = 0.f;

	/** Scream behaviour, from the type asset. */
	float ScreamSightSeconds = 0.f;
	float ScreamCancelSeconds = 0.f;
	int32 ScreamWalkerCount = 0;

	/** Seconds of continuous clear line of sight accumulated. */
	float ScreamSightTimer = 0.f;
	bool bHasScreamed = false;

	/** World time the scream fired, for the cancel window. */
	float ScreamStartTime = 0.f;

	/** Fixed +1 or -1 per instance, so a stalled zombie shoulders past on a
		consistent side and the queue fans out. */
	float StallLateralSign = 1.f;

	bool bDead = false;

	UPROPERTY(
		VisibleInstanceOnly, BlueprintReadOnly, Category = "Zombie", Transient, meta = (AllowPrivateAccess = "true"))
	ELTZombieType ZombieType = ELTZombieType::Walker;

	/** Which special hook this zombie runs. Read from the type asset rather than
		inferred from ZombieType, so a later type can reuse a behaviour. */
	UPROPERTY(
		VisibleInstanceOnly, BlueprintReadOnly, Category = "Zombie", Transient, meta = (AllowPrivateAccess = "true"))
	ELTZombieBehaviour Behaviour = ELTZombieBehaviour::None;

	UPROPERTY(
		VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat", Transient, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> CurrentTarget;
};

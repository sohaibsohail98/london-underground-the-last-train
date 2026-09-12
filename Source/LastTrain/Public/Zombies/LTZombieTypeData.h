#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LTZombieTypeData.generated.h"

class USoundBase;

/** The five types brief-v2 names. There is one rigged humanoid mesh: a type is
	scale, tint, play rate and behaviour, not a bespoke character. */
UENUM(BlueprintType)
enum class ELTZombieType : uint8
{
	Walker,
	Sprinter,
	Brute,
	Crawler,
	Screamer
};

/** Which special hook the character runs. Separate from the type id so a later
	type can reuse a behaviour without the character switching on an identity. */
UENUM(BlueprintType)
enum class ELTZombieBehaviour : uint8
{
	/** Walker. No special. */
	None,
	/** Sprinter. Fast, tighter gait band, optional short lunge on the attack wind-up. */
	Sprint,
	/** Brute. Front plate absorbs body shots until broken, then normal damage. */
	ArmourPlate,
	/** Crawler. Prone, low capsule, head only reachable by a crouched player. */
	LowProfile,
	/** Screamer. On sustained line of sight, calls an extra wave. */
	Scream
};

/** Every tunable property of one zombie type. One asset per type under
	Content/LastTrain/Zombies/. Multipliers scale the round-scaled values on
	ALTZombieCharacter; every "Override" field falls through to the character's
	own value when it is zero, so a walker asset of all zeroes and all ones is
	byte-for-byte the coded default. The numbers are provisional: they settle in
	the Phase G balance pass. See docs/design/gameplay-canon.md section 6. */
UCLASS(BlueprintType)
class LASTTRAIN_API ULTZombieTypeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** The id. Drives roster logging and the special-round rules. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	ELTZombieType Type = ELTZombieType::Walker;

	/** Original, generic. "Walker", "Sprinter" and the rest are fine. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	ELTZombieBehaviour Behaviour = ELTZombieBehaviour::None;

	/** Multiplies BaseHealth, and re-applied on top of every round scaling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float HealthMultiplier = 1.f;

	/** Multiplies BaseWalkSpeed before the SpeedStepRounds steps are added. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float WalkSpeedMultiplier = 1.f;

	/** Above zero, replaces AttackDamage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float AttackDamageOverride = 0.f;

	/** Above zero, replaces AttackCooldownSeconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float AttackCooldownOverride = 0.f;

	/** Above zero, replaces AttackRange. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float AttackRangeOverride = 0.f;

	/** Uniform scale applied to the character mesh on spawn. Silhouette is the
		primary way a type reads at a glance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	float MeshScale = 1.f;

	/** Read by the spawn Blueprint to pick this type's material instance. It is
		deliberately NOT applied in C++: the tint must come from a shared tintable
		material with one instance per type, never a dynamic instance per spawn,
		or a 40 strong mixed crowd pays for it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	FLinearColor ColourTint = FLinearColor::White;

	/** Play rate multiplier the anim Blueprint reads through GetAnimPlayRate, so
		the one shared locomotion set plays faster or slower per type. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	float AnimPlayRate = 1.f;

	/** Above zero, replaces RepathIntervalSeconds. A fast type wants a shorter
		cadence, a crawler a longer one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	float RepathIntervalOverride = 0.f;

	/** Above zero, resizes the capsule half height on spawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	float CapsuleHalfHeightOverride = 0.f;

	/** Above zero, resizes the capsule radius on spawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	float CapsuleRadiusOverride = 0.f;

	/** Above zero, replaces the avoidance consideration radius the constructor
		sets. A brute wants a wider berth. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	float AvoidanceConsiderationRadiusOverride = 0.f;

	/** Above zero, replaces ContactRange. A 500 speed agent overshoots a 15 unit
		acceptance point on a short repath. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	float ContactRangeOverride = 0.f;

	/** Relative weight in a normal round's composition. Zero means this type
		never appears in a normal round: it is special-round only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roster")
	float SpawnWeightNormalRound = 100.f;

	/** Earliest round this type may spawn at all. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roster")
	int32 FirstRoundAvailable = 1;

	/** From the high heat threshold upward, multiplies SpawnWeightNormalRound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roster")
	float HighHeatWeightMultiplier = 1.f;

	/** Above zero, never more than this many of this type alive at once. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roster")
	int32 MaxAliveOfThisType = 0;

	/** Sprint behaviour: forward impulse on the attack wind-up. Zero disables
		the lunge. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behaviour")
	float SprintLungeImpulse = 0.f;

	/** ArmourPlate behaviour: accumulated front body damage the plate absorbs
		before normal damage applies. Headshots and rear hits always bypass it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behaviour")
	float ArmourBodyDamageToBreak = 0.f;

	/** Scream behaviour: continuous clear line of sight to the player camera
		before the scream fires. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behaviour")
	float ScreamLineOfSightSeconds = 2.f;

	/** Scream behaviour: extra walkers the scream asks the round manager for. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behaviour")
	int32 ScreamSummonCount = 4;

	/** Scream behaviour: kill the screamer inside this window of the scream and
		the summoned wave is cancelled. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behaviour")
	float ScreamCancelWindowSeconds = 0.5f;

	/** When set, replaces the character's IdleVocalSound. The periodic breath or
		moan that tells the player something is behind them. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> IdleVocalSound;

	/** When set, replaces the character's AggroVocalSound. Played once on spawn:
		a zombie here is hostile from the moment it exists, so the spawn is the
		aggro moment and there is no separate acquisition event to hang it on. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> AggroVocalSound;

	/** When set, replaces the character's AttackVocalSound. Played on the attack
		wind-up, with OnAttackWindUp. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> AttackVocalSound;

	/** When set, replaces the character's DeathVocalSound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> DeathVocalSound;

	/** Above zero, replaces IdleVocalIntervalSeconds. A brute wants a slower,
		heavier cadence than a sprinter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	float IdleVocalIntervalOverride = 0.f;

	/** Read by the death Blueprint. A walker or sprinter snaps to a settled pose
		rather than going fully limp, and a crawler is already prone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	bool bRagdollOnDeath = false;

	/** Above zero, replaces CorpseLifetime. A brute is a landmark and wants
		longer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float CorpseLifetimeOverride = 0.f;

	/** Above zero, asks the death Blueprint for a small shake within this radius. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float DeathScreenShakeRadius = 0.f;

	/** True when this type only appears on its own special round or when
		explicitly requested, never in a normal round's weighted mix. */
	UFUNCTION(BlueprintPure, Category = "Roster")
	bool IsSpecialOnly() const { return SpawnWeightNormalRound <= 0.f; }
};

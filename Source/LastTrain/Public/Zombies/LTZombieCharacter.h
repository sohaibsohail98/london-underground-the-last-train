#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LTZombieCharacter.generated.h"

class ULTPointsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZombieDied, ALTZombieCharacter*, Zombie, bool, bHeadshot);

/**
 * The base zombie. Threatening because of numbers and positioning rather than
 * because it is a bullet sponge, per the style guide: health scales, but
 * modestly, and the early rounds must stay readable.
 *
 * Navigation is delegated to the AI controller and the navigation mesh rather
 * than calculated here. This class owns health, damage, attack timing and
 * death only.
 */
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
	void ReceiveShot(float Damage, bool bHeadshot, const FHitResult& Hit, const FVector& ShotDirection, AActor* Instigator);

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

	/** Health multiplier compounded per round. 1.1 is the design value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float HealthGrowthPerRound = 1.1f;

	/** Rounds at which the walk speed steps up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	TArray<int32> SpeedStepRounds = { 5, 10, 20 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float BaseWalkSpeed = 130.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie")
	float SpeedPerStep = 55.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 130.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackDamage = 24.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackCooldownSeconds = 1.3f;

	/** Bone names treated as the head. Set to match the imported skeleton. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<FName> HeadBoneNames = { TEXT("head"), TEXT("Head"), TEXT("neck_01") };

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

private:
	void Die(bool bHeadshot, AActor* Killer);
	void TryAttack();

	float Health = 0.f;
	float AttackCooldown = 0.f;
	bool bDead = false;

	UPROPERTY() TObjectPtr<AActor> CurrentTarget;
};

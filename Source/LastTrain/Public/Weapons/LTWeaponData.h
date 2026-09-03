#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LTWeaponData.generated.h"

/** Every tunable property of a weapon. See docs/brief-v3-unreal.md for the aiming model. */
UCLASS(BlueprintType)
class LASTTRAIN_API ULTWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Original names only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText UpgradedName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float BaseDamage = 34.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float HeadshotMultiplier = 2.5f;

	/** Zombies a single shot passes through before stopping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	int32 Penetration = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float MaxRange = 8000.f;

	/** Damage falls off linearly between these two distances. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float FalloffStart = 2500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float FalloffEnd = 6000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing")
	int32 RoundsPerMinute = 600;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing")
	bool bAutomatic = true;

	/** Above one, this is a shotgun. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing")
	int32 PelletsPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition")
	int32 MagazineSize = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition")
	int32 MaxReserve = 240;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition")
	float ReloadSeconds = 2.1f;

	/** Cone half angle in degrees while hip firing, before penalties. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread")
	float HipSpreadDegrees = 3.4f;

	/** Cone half angle in degrees while fully aimed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread")
	float AimedSpreadDegrees = 0.5f;

	/** Additional degrees at full movement speed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread")
	float MovementSpreadDegrees = 2.2f;

	/** Degrees added per shot fired. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread")
	float BloomPerShotDegrees = 0.55f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread")
	float BloomMaxDegrees = 4.5f;

	/** Degrees of bloom recovered per second once firing stops. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread")
	float BloomDecayPerSecond = 6.f;

	/** Movement speed multiplier while fully aimed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aiming")
	float AimedMoveScale = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aiming")
	float AimTransitionSeconds = 0.18f;

	/** Hip fire value comes from the character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aiming")
	float AimedFieldOfView = 65.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 WallPrice = 500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 AmmoPrice = 250;

	/** Lowest station tier this weapon may appear at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy", meta = (ClampMin = "1", ClampMax = "4"))
	int32 MinimumStationTier = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UParticleSystem> MuzzleFlash;

	/** Seconds between shots, derived from the fire rate. */
	UFUNCTION(BlueprintPure, Category = "Firing")
	float GetShotInterval() const { return RoundsPerMinute > 0 ? 60.f / static_cast<float>(RoundsPerMinute) : 0.1f; }
};

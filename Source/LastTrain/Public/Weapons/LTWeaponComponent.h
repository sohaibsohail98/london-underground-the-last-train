#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTWeaponComponent.generated.h"

class ULTWeaponData;
class ULTPointsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, Magazine, int32, Reserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitConfirmed, bool, bHeadshot);

/** Firing, spread, reloading and hitscan resolution. */
UCLASS(ClassGroup = (LastTrain), meta = (BlueprintSpawnableComponent))
class LASTTRAIN_API ULTWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULTWeaponComponent();

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnHitConfirmed OnHitConfirmed;

	/** The weapon currently held. Swapping this rearms the component. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ULTWeaponData> WeaponData;

	virtual void
	TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeapon(ULTWeaponData* NewWeapon, bool bRefillReserve = true);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartReload();

	/** Used by wall buys and the train. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RefillAmmunition();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAiming(bool bNewAiming);

	/** 0 fully hip fired, 1 fully aimed. Smoothed, for camera and animation. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetAimAlpha() const { return AimAlpha; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsAiming() const { return bAiming; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bReloading; }

	/** Cone half angle: aim state base, plus movement penalty, plus recoil bloom. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentSpreadDegrees() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetMagazine() const { return Magazine; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetReserve() const { return Reserve; }

	/** Movement scale the character should apply, from the aim blend. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetMoveScale() const;

protected:
	virtual void BeginPlay() override;

private:
	void FireOnce();
	void FinishReload();

	/** Traces one pellet and applies damage. Returns true on a zombie hit. */
	bool TracePellet(const FVector& Origin, const FVector& Direction, bool& bOutHeadshot);

	/** Uniform over the cone disc, so shotgun patterns read correctly. */
	FVector ApplySpread(const FVector& Direction, float SpreadDegrees) const;

	/** Camera location and forward vector, or the actor's if there is no view. */
	void GetViewPoint(FVector& OutLocation, FVector& OutDirection) const;

	UPROPERTY() int32 Magazine = 0;
	UPROPERTY() int32 Reserve = 0;

	bool bFiring = false;
	bool bAiming = false;
	bool bReloading = false;

	float TimeUntilNextShot = 0.f;
	float ReloadRemaining = 0.f;
	float BloomDegrees = 0.f;
	float AimAlpha = 0.f;

	UPROPERTY() TObjectPtr<ULTPointsComponent> Points;
};

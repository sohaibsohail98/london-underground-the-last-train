#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTWeaponComponent.generated.h"

class ULTWeaponData;
class ULTPointsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, Magazine, int32, Reserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitConfirmed, bool, bHeadshot);

/**
 * Firing, spread, reloading and hit resolution. Lives on the player character
 * as a component so that the same logic can later drive a second equipped
 * weapon without duplication.
 *
 * Hitscan rather than projectiles: at these ranges the difference is not
 * perceptible and the cost of a line trace is far lower than a spawned actor
 * per pellet, which matters once a shotgun is firing eight pellets into a
 * horde of twenty four.
 */
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

	virtual void TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeapon(ULTWeaponData* NewWeapon, bool bRefillReserve = true);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartReload();

	/** Tops the reserve up to its maximum, used by wall buys and the train. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RefillAmmunition();

	/** Aim state is driven by the character, which owns the input binding. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAiming(bool bNewAiming);

	/** 0 fully hip fired, 1 fully aimed. Smoothed, for camera and animation. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetAimAlpha() const { return AimAlpha; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsAiming() const { return bAiming; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bReloading; }

	/**
	 * Total current cone half angle in degrees: base for the aim state, plus a
	 * movement penalty, plus accumulated recoil bloom. The HUD reticle should
	 * be sized from this so the circle is an honest promise about where the
	 * shot can land.
	 */
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

	/** Uniform distribution over the cone disc, so shotgun patterns read right. */
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

	/** Cached so a kill can award points without a lookup per shot. */
	UPROPERTY() TObjectPtr<ULTPointsComponent> Points;
};

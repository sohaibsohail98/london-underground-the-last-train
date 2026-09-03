#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LTPlayerCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
class UInputAction;
class UInputMappingContext;
class ULTWeaponComponent;
class ULTPointsComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, HealthFraction);

/** First person player. Sprinting forces hip fire. */
UCLASS()
class LASTTRAIN_API ALTPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALTPlayerCharacter();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(
		float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Player")
	ULTWeaponComponent* GetWeapon() const { return Weapon; }

	UFUNCTION(BlueprintPure, Category = "Player")
	ULTPointsComponent* GetPoints() const { return Points; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthFraction() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	/** Seconds after the last damage before regeneration begins. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float RegenerationDelaySeconds = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float RegenerationPerSecond = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 420.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 640.f;

	/** Wide, so the platform reads. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float BaseFieldOfView = 95.f;

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void StartFire();
	void StopFire();
	void StartAim();
	void StopAim();
	void Reload();

	/** Blueprint hook for the damage vignette and audio. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDamageTaken(float Fraction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDied();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USkeletalMeshComponent> ViewModel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ULTWeaponComponent> Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	TObjectPtr<ULTPointsComponent> Points;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ReloadAction;

private:
	float Health = 0.f;
	float TimeSinceDamage = 0.f;
	bool bSprinting = false;
	bool bDead = false;
};

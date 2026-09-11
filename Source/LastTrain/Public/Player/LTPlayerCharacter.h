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
class ULTInteractionComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, HealthFraction);

/** Shape deliberately identical to ULTWeaponComponent's FOnHitConfirmed, so the
	HUD's existing hit marker binding works off either without a second path. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeHitConfirmed, bool, bHeadshot);

/** First person player. Sprinting forces hip fire. Zero health goes down rather
	than dead: bleed-out runs, and a revive brings the run back. Carries the
	contextual melee bash, which is a property of the player rather than of the
	weapon: see PerformMelee. */
UCLASS()
class LASTTRAIN_API ALTPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALTPlayerCharacter();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	/** A melee strike connected with a zombie. Not fired on a miss, and not
		fired on a strike the cooldown or the run state refused. */
	UPROPERTY(BlueprintAssignable, Category = "Melee")
	FOnMeleeHitConfirmed OnMeleeHitConfirmed;

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(
		float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Player")
	ULTWeaponComponent* GetWeapon() const { return Weapon; }

	UFUNCTION(BlueprintPure, Category = "Player")
	ULTPointsComponent* GetPoints() const { return Points; }

	UFUNCTION(BlueprintPure, Category = "Player")
	ULTInteractionComponent* GetInteraction() const { return Interaction; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthFraction() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	/** True while the player is down but not yet dead. Rounds carry on around
		them: the run only ends when bleed-out expires. */
	UFUNCTION(BlueprintPure, Category = "Downed")
	bool IsDowned() const { return bDowned; }

	/** Seconds left before bleed-out kills the player. 0 when not downed. */
	UFUNCTION(BlueprintPure, Category = "Downed")
	float GetBleedOutRemaining() const { return bDowned ? BleedOutRemaining : 0.f; }

	/** Brings the player back up at ReviveHealthFraction health. No-op if not
		downed. The seam for a self-revive item or a co-op revive. */
	UFUNCTION(BlueprintCallable, Category = "Downed")
	void Revive();

	/** One contextual bash with whatever is already held. Deliberately not a
		weapon slot, not a knife and not an equip state: a short trace straight
		out of the view for a flat MeleeDamage, so it reads the same whatever is
		in the player's hands. Ungated by ammunition on purpose, since the whole
		point of it is the moment the magazine and the reserve are both empty.
		A no-op while down, dead, on cooldown, or once the run is over. */
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void PerformMelee();

	/** Seconds until the next strike is allowed, 0 when ready. For a HUD or an
		animation Blueprint that wants to grey the prompt out. */
	UFUNCTION(BlueprintPure, Category = "Melee")
	float GetMeleeCooldownRemaining() const { return MeleeCooldownRemaining; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	/** Seconds after the last damage before regeneration begins. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float RegenerationDelaySeconds = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float RegenerationPerSecond = 20.f;

	/** Seconds a downed player has before the run ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed")
	float BleedOutSeconds = 30.f;

	/** Health a revive returns, as a fraction of MaxHealth. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed")
	float ReviveHealthFraction = 0.5f;

	/** A solo run has nobody to revive it, so v1 gets up on its own. Clear this
		once a self-revive item or a second player can call Revive. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed")
	bool bSoloAutoRevive = true;

	/** Seconds down before the solo auto-revive fires. Shorter than
		BleedOutSeconds or it never gets the chance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed")
	float SoloReviveDelaySeconds = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 420.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 640.f;

	/** Wide, so the platform reads. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float BaseFieldOfView = 95.f;

	/** Trace length for the bash, from the view point. Short by design: the
		zombie's own AttackRange is 130 root to root, so 150 reaches anything
		already close enough to be hitting the player and nothing beyond it.
		Far inside every weapon's FalloffStart, so no distance falloff applies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
	float MeleeRange = 150.f;

	/** Flat, and never scaled off the held weapon's BaseDamage: the bash is a
		constant fallback rather than a second damage curve to balance per gun.
		Three strikes kill a round one walker at BaseHealth 150, which is a real
		way out of a corner and still far below the starting pistol's output. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
	float MeleeDamage = 50.f;

	/** Seconds between strikes. Long enough that spamming it cannot stand in
		for a weapon's rate of fire, short enough to land one swing inside a
		zombie's 1.3s AttackCooldownSeconds and break a pin. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
	float MeleeCooldownSeconds = 0.8f;

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
	void Interact();

	/** Blueprint hook for the damage vignette and audio. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDamageTaken(float Fraction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDied();

	/** The player has gone down. For the downed screen treatment, the get-up
		prompt and a last-stand view model swap. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Downed")
	void OnDowned();

	/** The player is back up. Undoes whatever OnDowned dressed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Downed")
	void OnRevived();

	/** Bleed-out ran out. Fired immediately before the death path. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Downed")
	void OnBleedOutExpired();

	/** The bash animation and its audio. Fired on every strike the cooldown and
		the run state allowed, hit or miss, because the swing plays either way.
		OnMeleeHitConfirmed is the separate, narrower "it landed" signal. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Melee")
	void OnMeleeSwing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USkeletalMeshComponent> ViewModel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ULTWeaponComponent> Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
	TObjectPtr<ULTPointsComponent> Points;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<ULTInteractionComponent> Interaction;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MeleeAction;

private:
	/** Zero health goes here, not straight to Die. Immobile, no weapon, bleeding
		out. No-op if already downed or dead. */
	void Down();

	/** Ends the run. Reached from bleed-out expiry, never from damage directly. */
	void Die();

	float Health = 0.f;
	float TimeSinceDamage = 0.f;
	float BleedOutRemaining = 0.f;
	float SoloReviveRemaining = 0.f;

	/** Counted down in Tick, checked before a strike. The same one-clock shape
		ULTWeaponComponent uses for TimeUntilNextShot rather than a timer handle. */
	float MeleeCooldownRemaining = 0.f;
	bool bSprinting = false;
	bool bDowned = false;
	bool bDead = false;
};

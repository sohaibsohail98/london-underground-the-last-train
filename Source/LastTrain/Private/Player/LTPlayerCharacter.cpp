#include "Player/LTPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/LTGameMode.h"
#include "Core/LTGameState.h"
#include "Economy/LTPointsComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/LTInteractionComponent.h"
#include "LastTrain.h"
#include "Weapons/LTWeaponComponent.h"
#include "Weapons/LTWeaponData.h"

ALTPlayerCharacter::ALTPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->InitCapsuleSize(38.f, 90.f);
	}

	bUseControllerRotationYaw = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 68.f));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(BaseFieldOfView);

	ViewModel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ViewModel"));
	ViewModel->SetupAttachment(Camera);
	ViewModel->SetOnlyOwnerSee(true);
	ViewModel->bCastDynamicShadow = false;
	ViewModel->CastShadow = false;
	ViewModel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Not needed in single player, and it would show in the view model's shadow.
	if (USkeletalMeshComponent* Body = GetMesh())
	{
		Body->SetOwnerNoSee(true);
	}

	Weapon = CreateDefaultSubobject<ULTWeaponComponent>(TEXT("Weapon"));
	Points = CreateDefaultSubobject<ULTPointsComponent>(TEXT("Points"));
	Interaction = CreateDefaultSubobject<ULTInteractionComponent>(TEXT("Interaction"));

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = WalkSpeed;
		Movement->bOrientRotationToMovement = false;
		Movement->JumpZVelocity = 420.f;
		Movement->AirControl = 0.25f;
	}
}

void ALTPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	TimeSinceDamage = RegenerationDelaySeconds;
	OnHealthChanged.Broadcast(GetHealthFraction());

	if (Camera)
	{
		Camera->SetFieldOfView(BaseFieldOfView);
	}

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InputMapping)
			{
				Subsystem->AddMappingContext(InputMapping, 0);
			}
		}
	}
}

void ALTPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		LT_LOG(Error, TEXT("Expected an EnhancedInputComponent. Check the project input settings."));
		return;
	}

	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALTPlayerCharacter::Move);
	}
	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALTPlayerCharacter::Look);
	}
	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (SprintAction)
	{
		Input->BindAction(SprintAction, ETriggerEvent::Started, this, &ALTPlayerCharacter::StartSprint);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ALTPlayerCharacter::StopSprint);
	}
	if (FireAction)
	{
		Input->BindAction(FireAction, ETriggerEvent::Started, this, &ALTPlayerCharacter::StartFire);
		Input->BindAction(FireAction, ETriggerEvent::Completed, this, &ALTPlayerCharacter::StopFire);
	}
	if (AimAction)
	{
		Input->BindAction(AimAction, ETriggerEvent::Started, this, &ALTPlayerCharacter::StartAim);
		Input->BindAction(AimAction, ETriggerEvent::Completed, this, &ALTPlayerCharacter::StopAim);
	}
	if (ReloadAction)
	{
		Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &ALTPlayerCharacter::Reload);
	}
	if (InteractAction)
	{
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ALTPlayerCharacter::Interact);
	}
}

void ALTPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bDead || bDowned)
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ALTPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ALTPlayerCharacter::StartSprint()
{
	if (bDead || bDowned)
	{
		return;
	}

	bSprinting = true;

	// Cancel rather than block, so the player is never left in a half state.
	if (Weapon)
	{
		Weapon->SetAiming(false);
	}
}

void ALTPlayerCharacter::StopSprint()
{
	bSprinting = false;
}

void ALTPlayerCharacter::StartFire()
{
	if (!bDead && !bDowned && Weapon)
	{
		Weapon->StartFiring();
	}
}

void ALTPlayerCharacter::StopFire()
{
	if (Weapon)
	{
		Weapon->StopFiring();
	}
}

void ALTPlayerCharacter::StartAim()
{
	if (!bDead && !bDowned && Weapon && !bSprinting)
	{
		Weapon->SetAiming(true);
	}
}

void ALTPlayerCharacter::StopAim()
{
	if (Weapon)
	{
		Weapon->SetAiming(false);
	}
}

void ALTPlayerCharacter::Reload()
{
	if (!bDead && !bDowned && Weapon)
	{
		Weapon->StartReload();
	}
}

void ALTPlayerCharacter::Interact()
{
	if (!bDead && !bDowned && Interaction)
	{
		Interaction->TryInteract();
	}
}

void ALTPlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
	{
		return;
	}

	// The aim blend keeps running even while down. The weapon component decays its
	// own alpha when Down clears aiming, and if nothing applied it the camera would
	// stay at the aimed field of view for the whole bleed-out.
	if (Camera && Weapon)
	{
		const ULTWeaponData* Data = Weapon->WeaponData;
		const float Target = Data ? Data->AimedFieldOfView : BaseFieldOfView;
		Camera->SetFieldOfView(FMath::Lerp(BaseFieldOfView, Target, Weapon->GetAimAlpha()));
	}

	if (bDowned)
	{
		// Nothing else runs while down: no movement scaling and no regeneration.
		// Only the two clocks.
		BleedOutRemaining -= DeltaSeconds;

		if (BleedOutRemaining <= 0.f)
		{
			BleedOutRemaining = 0.f;
			OnBleedOutExpired();
			Die();
			return;
		}

		if (bSoloAutoRevive)
		{
			SoloReviveRemaining -= DeltaSeconds;

			if (SoloReviveRemaining <= 0.f)
			{
				Revive();
			}
		}

		return;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		const float Base = bSprinting ? SprintSpeed : WalkSpeed;
		const float Scale = Weapon ? Weapon->GetMoveScale() : 1.f;
		Movement->MaxWalkSpeed = Base * Scale;
	}

	TimeSinceDamage += DeltaSeconds;

	if (TimeSinceDamage >= RegenerationDelaySeconds && Health < MaxHealth)
	{
		Health = FMath::Min(MaxHealth, Health + RegenerationPerSecond * DeltaSeconds);
		OnHealthChanged.Broadcast(GetHealthFraction());
	}
}

float ALTPlayerCharacter::TakeDamage(
	const float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// A downed player is already at zero and on the bleed-out clock. Further hits
	// cannot take them lower and must not shorten it.
	if (bDead || bDowned || Damage <= 0.f)
	{
		return 0.f;
	}

	const float Applied = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (Applied <= 0.f)
	{
		// A damage modifier or an immunity absorbed the whole hit. Nothing lands,
		// and the regeneration clock must not restart on a hit that did nothing.
		return 0.f;
	}

	// The modified value, not the raw request. Reporting one figure to the caller
	// and subtracting another made every damage modifier a lie.
	Health = FMath::Max(0.f, Health - Applied);
	TimeSinceDamage = 0.f;

	OnHealthChanged.Broadcast(GetHealthFraction());
	OnDamageTaken(GetHealthFraction());

	if (Health <= 0.f)
	{
		// Zero health is a down, never a death. Only bleed-out reaches Die.
		Down();
	}

	return Applied;
}

void ALTPlayerCharacter::Down()
{
	if (bDowned || bDead)
	{
		return;
	}

	// A boarded or finished run is over. A parting hit on the platform must not
	// flip the run state back to Downed: that would restart the train's cycle and
	// leave a half-alive arena behind a departing player.
	if (const UWorld* World = GetWorld())
	{
		if (const ALTGameState* State = World->GetGameState<ALTGameState>())
		{
			const ELTRunState RunState = State->GetRunState();
			if (RunState == ELTRunState::Boarded || RunState == ELTRunState::Dead)
			{
				return;
			}
		}
	}

	bDowned = true;
	Health = 0.f;
	BleedOutRemaining = BleedOutSeconds;
	SoloReviveRemaining = SoloReviveDelaySeconds;

	// Immobile, but the camera stays free so the player can watch the room. The
	// regeneration in Tick is frozen by the downed branch, so there is no timer to
	// cancel.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	bSprinting = false;

	if (Weapon)
	{
		Weapon->StopFiring();
		Weapon->SetAiming(false);
	}

	// Interact is already gated, but the sweep is not: without this the prompt
	// stays on screen through the whole bleed-out with an inert key under it.
	if (Interaction)
	{
		Interaction->SetInteractionEnabled(false);
	}

	if (const UWorld* World = GetWorld())
	{
		if (ALTGameMode* GameMode = World->GetAuthGameMode<ALTGameMode>())
		{
			GameMode->NotifyPlayerDowned();
		}
	}

	OnDowned();

	LT_LOG(Log, TEXT("Player downed. Bleed-out in %.0fs."), BleedOutSeconds);
}

void ALTPlayerCharacter::Revive()
{
	if (!bDowned || bDead)
	{
		return;
	}

	bDowned = false;
	BleedOutRemaining = 0.f;
	SoloReviveRemaining = 0.f;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// The component's own default rather than a forced Walking, so a player
		// revived off the ground falls instead of standing on air.
		Movement->SetDefaultMovementMode();
	}

	Health = FMath::Clamp(ReviveHealthFraction, 0.f, 1.f) * MaxHealth;

	// Regeneration waits the usual delay from the moment of the revive.
	TimeSinceDamage = 0.f;

	if (Interaction)
	{
		Interaction->SetInteractionEnabled(true);
	}

	OnHealthChanged.Broadcast(GetHealthFraction());

	if (const UWorld* World = GetWorld())
	{
		if (ALTGameMode* GameMode = World->GetAuthGameMode<ALTGameMode>())
		{
			GameMode->NotifyPlayerRevived();
		}
	}

	OnRevived();

	LT_LOG(Log, TEXT("Player revived at %.0f%% health."), FMath::Clamp(ReviveHealthFraction, 0.f, 1.f) * 100.f);
}

void ALTPlayerCharacter::Die()
{
	if (bDead)
	{
		return;
	}

	bDead = true;
	bDowned = false;
	Health = 0.f;

	if (Weapon)
	{
		Weapon->StopFiring();
	}

	if (Interaction)
	{
		Interaction->SetInteractionEnabled(false);
	}

	OnHealthChanged.Broadcast(GetHealthFraction());
	OnDied();

	if (const UWorld* World = GetWorld())
	{
		if (ALTGameMode* GameMode = World->GetAuthGameMode<ALTGameMode>())
		{
			GameMode->NotifyPlayerDied();
		}
	}

	LT_LOG(Log, TEXT("Player bled out. Run over."));
}

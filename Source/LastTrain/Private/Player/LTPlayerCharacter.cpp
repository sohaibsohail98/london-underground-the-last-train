#include "Player/LTPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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
	if (bDead)
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
	if (!bDead && Weapon)
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
	if (!bDead && Weapon && !bSprinting)
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
	if (!bDead && Weapon)
	{
		Weapon->StartReload();
	}
}

void ALTPlayerCharacter::Interact()
{
	if (!bDead && Interaction)
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

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		const float Base = bSprinting ? SprintSpeed : WalkSpeed;
		const float Scale = Weapon ? Weapon->GetMoveScale() : 1.f;
		Movement->MaxWalkSpeed = Base * Scale;
	}

	if (Camera && Weapon)
	{
		const ULTWeaponData* Data = Weapon->WeaponData;
		const float Target = Data ? Data->AimedFieldOfView : BaseFieldOfView;
		Camera->SetFieldOfView(FMath::Lerp(BaseFieldOfView, Target, Weapon->GetAimAlpha()));
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
	if (bDead || Damage <= 0.f)
	{
		return 0.f;
	}

	const float Applied = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Max(0.f, Health - Damage);
	TimeSinceDamage = 0.f;

	OnHealthChanged.Broadcast(GetHealthFraction());
	OnDamageTaken(GetHealthFraction());

	if (Health <= 0.f)
	{
		bDead = true;
		if (Weapon)
		{
			Weapon->StopFiring();
		}
		OnDied();
	}

	return Applied;
}

#include "Player/LTPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/LTGameInstance.h"
#include "Core/LTGameMode.h"
#include "Core/LTGameState.h"
#include "Economy/LTPointsComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/LTInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "Weapons/LTWeaponComponent.h"
#include "Weapons/LTWeaponData.h"
#include "Zombies/LTZombieCharacter.h"

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

	// The player's saved field of view outranks the Blueprint's default. Pulled
	// here as well as pushed by ULTGameInstance on a map load, because a pawn
	// possessed late would otherwise miss the push.
	if (const ULTGameInstance* GameInstance = GetGameInstance<ULTGameInstance>())
	{
		SetBaseFieldOfView(GameInstance->GetGameSettings().FieldOfView);
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

void ALTPlayerCharacter::SetBaseFieldOfView(const float NewFieldOfView)
{
	if (NewFieldOfView <= 0.f)
	{
		return;
	}

	BaseFieldOfView = NewFieldOfView;

	if (Camera)
	{
		// Tick re-applies the aim blend from here every frame while the player is
		// alive, but a dead or weaponless pawn never reaches that, so set it now.
		Camera->SetFieldOfView(BaseFieldOfView);
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
	if (PauseAction)
	{
		Input->BindAction(PauseAction, ETriggerEvent::Started, this, &ALTPlayerCharacter::TogglePause);
	}
	if (MeleeAction)
	{
		Input->BindAction(MeleeAction, ETriggerEvent::Started, this, &ALTPlayerCharacter::PerformMelee);
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

bool ALTPlayerCharacter::CanPause() const
{
	// Dead and Boarded each already own the whole screen: the run-over card and
	// the travel fade. A pause overlay on top of either is two menus and a frozen
	// transition. PreGame, Active and Downed all pause.
	if (bDead)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (const ALTGameState* State = World->GetGameState<ALTGameState>())
	{
		const ELTRunState RunState = State->GetRunState();
		if (RunState == ELTRunState::Dead || RunState == ELTRunState::Boarded)
		{
			return false;
		}
	}

	return true;
}

void ALTPlayerCharacter::TogglePause()
{
	if (UGameplayStatics::IsGamePaused(this))
	{
		RequestResume();
	}
	else
	{
		RequestPause();
	}
}

void ALTPlayerCharacter::RequestPause()
{
	if (UGameplayStatics::IsGamePaused(this))
	{
		return;
	}

	if (!CanPause())
	{
		LT_LOG(Log, TEXT("Pause refused: the run is over or the player has boarded."));
		return;
	}

	// Ask first, dress after. A game mode with bPauseable cleared refuses, and
	// handing input to a menu that never opens would lock the player out.
	if (!UGameplayStatics::SetGamePaused(this, true))
	{
		LT_LOG(Warning, TEXT("SetGamePaused refused the pause. Input mode left alone."));
		return;
	}

	ApplyPauseInputMode(true);

	OnPauseStateChanged.Broadcast(true);
}

void ALTPlayerCharacter::RequestResume()
{
	if (!UGameplayStatics::IsGamePaused(this))
	{
		return;
	}

	if (!UGameplayStatics::SetGamePaused(this, false))
	{
		LT_LOG(Warning, TEXT("SetGamePaused refused the resume. The game is still paused."));
		return;
	}

	ApplyPauseInputMode(false);

	OnPauseStateChanged.Broadcast(false);
}

void ALTPlayerCharacter::ApplyPauseInputMode(const bool bPaused)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// Cursor first, then the input mode, the same order BP_MenuGameMode uses on
	// the main menu. No focus widget is named here: the pause widget sets its own
	// desired focus when it is built, and this class must not know about it.
	PC->bShowMouseCursor = bPaused;

	if (bPaused)
	{
		PC->SetInputMode(FInputModeUIOnly());
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void ALTPlayerCharacter::PerformMelee()
{
	// Note what is deliberately absent from this gate: any reference to the
	// weapon's Magazine or Reserve. The bash exists for the moment both are
	// empty, so ammunition must never be able to refuse it.
	if (bDead || bDowned || MeleeCooldownRemaining > 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Refused in the same two run states the pause action is refused in: the run
	// is over, or the player is aboard and travelling. Both already own the
	// screen, and neither wants a swing going on behind it. PreGame, Active and
	// Downed are all fine, and Downed is already caught by the flag above, the
	// same way firing, aiming and interacting are.
	if (const ALTGameState* State = World->GetGameState<ALTGameState>())
	{
		const ELTRunState RunState = State->GetRunState();
		if (RunState == ELTRunState::Dead || RunState == ELTRunState::Boarded)
		{
			return;
		}
	}

	MeleeCooldownRemaining = MeleeCooldownSeconds;

	// The animation plays on a miss too, so this fires before the trace. The
	// delegate below is the narrower signal and only fires on a connection.
	OnMeleeSwing();

	// The same view point ULTWeaponComponent::GetViewPoint resolves for a pawn
	// owner, so a bash starts where a shot would.
	FVector Origin;
	FRotator ViewRotation;
	GetActorEyesViewPoint(Origin, ViewRotation);

	const FVector Direction = ViewRotation.Vector();
	const FVector End = Origin + Direction * MeleeRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LTMeleeTrace), true, this);
	Params.bReturnPhysicalMaterial = false;
	Params.bTraceComplex = true;

	// One trace, no pellets and no penetration budget: a shove stops at the
	// first thing it meets, where a bullet may pass through.
	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, Origin, End, ECC_GameTraceChannel1, Params))
	{
		return;
	}

	ALTZombieCharacter* Zombie = Cast<ALTZombieCharacter>(Hit.GetActor());
	if (!Zombie)
	{
		// A wall, a wall buy or the train. It swung, it just met nothing that bleeds.
		return;
	}

	// Reported honestly rather than forced false, so the hit marker, the gore
	// and the kill award all read the same as they do for a shot. The damage
	// itself carries no headshot multiplier: a bash to the skull is still a
	// bash, and MeleeDamage is meant to stay one number.
	const bool bHeadshot = Zombie->IsHeadBone(Hit.BoneName);

	// Exactly the call ULTWeaponComponent::TracePellet makes, with a flat figure
	// in place of the weapon's scaled one. Routing through ReceiveShot rather
	// than a melee-only damage path is what keeps the brute's armour plate, the
	// hit reaction hook, Die() and the FOnZombieDied broadcast all working.
	Zombie->ReceiveShot(MeleeDamage, bHeadshot, Hit, Direction, this);

	// Gore is deliberately not spawned here. ALTZombieCharacter::ReceiveShot,
	// which the line above has just gone through, already calls the shared
	// ULTGoreDecalSubsystem on the sibling branch claude/blood-decals-prompt,
	// and it does so after the brute's armour early-return. Spawning again from
	// this side would draw two spatters for one strike and would put blood on a
	// plate that stopped the blow. Routing melee damage through ReceiveShot is
	// what satisfies the shared gore path: one entry point, one hit, one decal.

	// Kills are awarded from the zombie's death broadcast, so this is the hit
	// award only, the same way TracePellet does it.
	if (Points && !Zombie->IsDead())
	{
		Points->AwardHit();
	}

	OnMeleeHitConfirmed.Broadcast(bHeadshot);
}

void ALTPlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
	{
		return;
	}

	// Counted down before the downed branch returns, so a player who goes down
	// mid-cooldown comes back up with the bash ready rather than frozen.
	MeleeCooldownRemaining = FMath::Max(0.f, MeleeCooldownRemaining - DeltaSeconds);

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

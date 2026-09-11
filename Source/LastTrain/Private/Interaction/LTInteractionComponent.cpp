#include "Interaction/LTInteractionComponent.h"

#include "GameFramework/Pawn.h"
#include "Interaction/LTInteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"

ULTInteractionComponent::ULTInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULTInteractionComponent::GetViewPoint(FVector& OutLocation, FVector& OutDirection) const
{
	OutLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	OutDirection = GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;

	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		FRotator ViewRotation;
		Pawn->GetActorEyesViewPoint(OutLocation, ViewRotation);
		OutDirection = ViewRotation.Vector();
	}
}

void ULTInteractionComponent::TickComponent(
	const float DeltaSeconds, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (!World || !bInteractionEnabled)
	{
		return;
	}

	FVector Origin;
	FVector Direction;
	GetViewPoint(Origin, Direction);

	const FVector End = Origin + Direction * InteractionRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LTInteractionTrace), false, GetOwner());

	// Sweep multi and take the nearest hit that is an interactable, not the nearest
	// hit of anything. A single blocking sweep stops on the first Visibility blocker,
	// so cosmetic geometry in front of an interactable (a train bodyshell panel in
	// front of the boarding actor, a pillar in front of a wall buy) hides the prompt
	// entirely. Hits come back sorted near to far, so the first match is the closest.
	TArray<FHitResult> Hits;
	AActor* NewTarget = nullptr;

	if (World->SweepMultiByChannel(
			Hits, Origin, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TraceRadius), Params))
	{
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor->Implements<ULTInteractableInterface>() &&
				ILTInteractableInterface::Execute_CanInteract(HitActor, GetOwner()))
			{
				NewTarget = HitActor;
				break;
			}
		}
	}

	CurrentInteractable = NewTarget;

	FText NewPrompt;
	const bool bAvailable = NewTarget != nullptr;
	if (NewTarget)
	{
		NewPrompt = ILTInteractableInterface::Execute_GetInteractionPrompt(NewTarget, GetOwner());
	}

	if (bAvailable != bCurrentAvailable || !NewPrompt.IdenticalTo(CurrentPrompt))
	{
		bCurrentAvailable = bAvailable;
		CurrentPrompt = NewPrompt;
		OnInteractableChanged.Broadcast(CurrentPrompt, bCurrentAvailable);
	}
}

void ULTInteractionComponent::SetInteractionEnabled(const bool bEnabled)
{
	if (bInteractionEnabled == bEnabled)
	{
		return;
	}

	bInteractionEnabled = bEnabled;

	if (bInteractionEnabled)
	{
		// The next tick sweeps and broadcasts whatever is under the crosshair now.
		return;
	}

	CurrentInteractable = nullptr;

	if (bCurrentAvailable || !CurrentPrompt.IsEmpty())
	{
		CurrentPrompt = FText::GetEmpty();
		bCurrentAvailable = false;

		OnInteractableChanged.Broadcast(CurrentPrompt, bCurrentAvailable);
	}
}

void ULTInteractionComponent::TryInteract()
{
	if (!bInteractionEnabled || !CurrentInteractable)
	{
		return;
	}

	if (!ILTInteractableInterface::Execute_CanInteract(CurrentInteractable, GetOwner()))
	{
		return;
	}

	// Before the interact, not after. Boarding the train tears the arena down and
	// travels to the next station, so a confirm played afterwards would be cut
	// off by the level change on exactly the interact that most wants one.
	// 2D: it is the press being confirmed, not a thing happening in the world.
	if (InteractSound)
	{
		UGameplayStatics::PlaySound2D(this, InteractSound);
	}

	ILTInteractableInterface::Execute_Interact(CurrentInteractable, GetOwner());
}

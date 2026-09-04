#include "Interaction/LTInteractionComponent.h"

#include "GameFramework/Pawn.h"
#include "Interaction/LTInteractableInterface.h"
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
	if (!World)
	{
		return;
	}

	FVector Origin;
	FVector Direction;
	GetViewPoint(Origin, Direction);

	const FVector End = Origin + Direction * InteractionRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LTInteractionTrace), false, GetOwner());

	FHitResult Hit;
	AActor* NewTarget = nullptr;

	if (World->SweepSingleByChannel(
			Hit, Origin, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TraceRadius), Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<ULTInteractableInterface>()
			&& ILTInteractableInterface::Execute_CanInteract(HitActor, GetOwner()))
		{
			NewTarget = HitActor;
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

void ULTInteractionComponent::TryInteract()
{
	if (!CurrentInteractable)
	{
		return;
	}

	if (!ILTInteractableInterface::Execute_CanInteract(CurrentInteractable, GetOwner()))
	{
		return;
	}

	ILTInteractableInterface::Execute_Interact(CurrentInteractable, GetOwner());
}

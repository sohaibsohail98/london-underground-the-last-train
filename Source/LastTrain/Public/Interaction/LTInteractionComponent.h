#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableChanged, const FText&, Prompt, bool, bAvailable);

/** Traces ahead of the owning pawn for an interactable and holds the current one. */
UCLASS(ClassGroup = (LastTrain), meta = (BlueprintSpawnableComponent))
class LASTTRAIN_API ULTInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULTInteractionComponent();

	/** The HUD binds to this. Broadcast only when the prompt or availability changes. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableChanged OnInteractableChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float InteractionRange = 250.f;

	/** Sphere sweep radius, so small anchors are not fiddly to look at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float TraceRadius = 12.f;

	virtual void
	TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Uses the current target if it still passes CanInteract. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractable; }

private:
	/** Camera or eyes location and forward vector, or the actor's if there is no view. */
	void GetViewPoint(FVector& OutLocation, FVector& OutDirection) const;

	UPROPERTY() TObjectPtr<AActor> CurrentInteractable;

	FText CurrentPrompt;
	bool bCurrentAvailable = false;
};

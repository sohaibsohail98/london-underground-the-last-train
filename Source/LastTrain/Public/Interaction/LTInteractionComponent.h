#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTInteractionComponent.generated.h"

class USoundBase;

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

	/** The generic confirm, played on any interact that actually goes through.
		One sound for every interactable: a wall buy that wants its own purchase
		sound plays that from its own Interact, this only says the press landed.
		Null is silent. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> InteractSound;

	virtual void
	TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Uses the current target if it still passes CanInteract. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractable; }

	/** Disabled, the component stops sweeping and clears whatever prompt is up.
		A downed player would otherwise sit out the bleed-out looking at a live
		"Board train" with an inert key under it. Disabling the tick from outside
		would freeze that last prompt on screen instead of clearing it. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInteractionEnabled() const { return bInteractionEnabled; }

private:
	/** Camera or eyes location and forward vector, or the actor's if there is no view. */
	void GetViewPoint(FVector& OutLocation, FVector& OutDirection) const;

	UPROPERTY() TObjectPtr<AActor> CurrentInteractable;

	FText CurrentPrompt;
	bool bCurrentAvailable = false;
	bool bInteractionEnabled = true;
};

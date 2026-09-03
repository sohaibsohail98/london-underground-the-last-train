#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LTInteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class ULTInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/** One interaction path for wall buys, doors, perk machines, the bench and lost property. */
class LASTTRAIN_API ILTInteractableInterface
{
	GENERATED_BODY()

public:
	/** Whether this can be used right now by the given actor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor) const;

	/** Prompt text, including cost. Shown while looked at and in range. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt(AActor* Interactor) const;

	/** Performs the interaction. Must validate affordability itself. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
};

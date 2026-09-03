#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LTInteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class ULTInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Shared by wall buys, purchasable doors, perk machines, the upgrade bench and
 * the lost property office, so the player has one interaction path rather than
 * five special cases. This is the pattern the architecture document specifies.
 */
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

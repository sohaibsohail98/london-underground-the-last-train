#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/LTInteractableInterface.h"
#include "LTWallBuy.generated.h"

class ULTWeaponData;
class UStaticMeshComponent;

/** A wall mounted purchase for a weapon, then ammunition once held. */
UCLASS()
class LASTTRAIN_API ALTWallBuy : public AActor, public ILTInteractableInterface
{
	GENERATED_BODY()

public:
	ALTWallBuy();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall buy")
	TObjectPtr<ULTWeaponData> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall buy")
	int32 WeaponCost = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall buy")
	int32 AmmunitionCost = 250;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	/** Blueprint hook for the purchase flash and audio. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Wall buy")
	void OnPurchased();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall buy")
	TObjectPtr<UStaticMeshComponent> Plate;
};

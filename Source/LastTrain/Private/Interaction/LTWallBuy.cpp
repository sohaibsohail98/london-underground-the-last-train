#include "Interaction/LTWallBuy.h"

#include "Components/StaticMeshComponent.h"
#include "Economy/LTPointsComponent.h"
#include "LastTrain.h"
#include "Weapons/LTWeaponComponent.h"
#include "Weapons/LTWeaponData.h"

ALTWallBuy::ALTWallBuy()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// The plate is the visual and the trace target, so it keeps the default block-visibility setup.
	Plate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plate"));
	Plate->SetupAttachment(Root);
}

bool ALTWallBuy::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Weapon || !Interactor)
	{
		return false;
	}

	return Interactor->FindComponentByClass<ULTPointsComponent>() != nullptr;
}

FText ALTWallBuy::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	const FText WeaponName = Weapon ? Weapon->DisplayName : FText::GetEmpty();

	const ULTWeaponComponent* HeldWeapon = Interactor ? Interactor->FindComponentByClass<ULTWeaponComponent>() : nullptr;
	const bool bAlreadyHeld = HeldWeapon && HeldWeapon->WeaponData == Weapon;

	if (bAlreadyHeld)
	{
		return FText::Format(
			NSLOCTEXT("LastTrain", "WallBuyAmmunitionPrompt", "Ammunition: {0}  ({1})"),
			FText::AsNumber(AmmunitionCost),
			WeaponName);
	}

	return FText::Format(
		NSLOCTEXT("LastTrain", "WallBuyPurchasePrompt", "Buy {0}: {1}"), WeaponName, FText::AsNumber(WeaponCost));
}

void ALTWallBuy::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || !Weapon)
	{
		return;
	}

	ULTPointsComponent* PointsComponent = Interactor->FindComponentByClass<ULTPointsComponent>();
	ULTWeaponComponent* WeaponComponent = Interactor->FindComponentByClass<ULTWeaponComponent>();
	if (!PointsComponent || !WeaponComponent)
	{
		return;
	}

	// TrySpend refuses when unaffordable, so affordability is never tested separately.
	if (WeaponComponent->WeaponData == Weapon)
	{
		if (PointsComponent->TrySpend(AmmunitionCost))
		{
			WeaponComponent->RefillAmmunition();
			OnPurchased();
		}
		return;
	}

	if (PointsComponent->TrySpend(WeaponCost))
	{
		WeaponComponent->SetWeapon(Weapon, true);
		OnPurchased();
	}
}

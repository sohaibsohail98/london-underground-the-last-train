#include "Economy/LTPointsComponent.h"

ULTPointsComponent::ULTPointsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULTPointsComponent::BeginPlay()
{
	Super::BeginPlay();

	Points = StartingPoints;
	OnPointsChanged.Broadcast(Points, StartingPoints);
}

void ULTPointsComponent::AwardHit()
{
	AddPoints(PointsPerHit);
}

void ULTPointsComponent::AwardKill(const bool bHeadshot)
{
	AddPoints(bHeadshot ? PointsPerHeadshotKill : PointsPerKill);
}

void ULTPointsComponent::AddPoints(const int32 Amount)
{
	if (Amount == 0)
	{
		return;
	}

	Points = FMath::Max(0, Points + Amount);
	OnPointsChanged.Broadcast(Points, Amount);
}

void ULTPointsComponent::SetPoints(const int32 NewTotal)
{
	const int32 Clamped = FMath::Max(0, NewTotal);

	if (Points == Clamped)
	{
		return;
	}

	Points = Clamped;

	// Zero delta on purpose: a HUD renders a negative delta as a spend, and a
	// balance arriving with the player from the last station is not one.
	OnPointsChanged.Broadcast(Points, 0);
}

bool ULTPointsComponent::TrySpend(const int32 Cost)
{
	if (Cost <= 0 || !CanAfford(Cost))
	{
		return false;
	}

	AddPoints(-Cost);
	return true;
}

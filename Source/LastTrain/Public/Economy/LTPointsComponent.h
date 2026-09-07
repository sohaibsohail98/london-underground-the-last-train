#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTPointsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPointsChanged, int32, NewTotal, int32, Delta);

/** The points economy. Award values are data, never literals in Blueprints. */
UCLASS(ClassGroup = (LastTrain), meta = (BlueprintSpawnableComponent))
class LASTTRAIN_API ULTPointsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULTPointsComponent();

	UPROPERTY(BlueprintAssignable, Category = "Economy")
	FOnPointsChanged OnPointsChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 StartingPoints = 500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 PointsPerHit = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 PointsPerKill = 60;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Economy")
	int32 PointsPerHeadshotKill = 130;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AwardHit();

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AwardKill(bool bHeadshot);

	/** Adds or removes points directly. Negative values are allowed. */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddPoints(int32 Amount);

	/** Assigns the total outright and broadcasts a zero delta. For a travel carry
		landing, which is neither an award nor a spend and must not be coloured as
		one on the HUD. */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void SetPoints(int32 NewTotal);

	/** Does not spend. */
	UFUNCTION(BlueprintPure, Category = "Economy")
	bool CanAfford(int32 Cost) const { return Points >= Cost; }

	/** Spends if affordable. Returns false and changes nothing otherwise. */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool TrySpend(int32 Cost);

	UFUNCTION(BlueprintPure, Category = "Economy")
	int32 GetPoints() const { return Points; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Economy")
	int32 Points = 0;
};

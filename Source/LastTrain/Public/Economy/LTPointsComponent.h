#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTPointsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPointsChanged, int32, NewTotal, int32, Delta);

/**
 * The points economy. Awards are data driven rather than scattered through
 * Blueprints, per the architecture document. Values are the ones established
 * in the design brief: 10 per hit, 60 per kill, 130 per headshot kill.
 */
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

	/** True if the player can afford the cost. Does not spend. */
	UFUNCTION(BlueprintPure, Category = "Economy")
	bool CanAfford(int32 Cost) const { return Points >= Cost; }

	/**
	 * Spends points if affordable. Returns false and changes nothing if not,
	 * which is the boundary case the testing document calls out: exactly the
	 * purchase price must succeed.
	 */
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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Train/LTTrain.h"
#include "LTDepartureBoard.generated.h"

/** Reads the station train's countdown and phase and drives presentation hooks a
	Blueprint uses to update a sign. Finds the train on BeginPlay; the visual is
	the Blueprint's job. */
UCLASS()
class LASTTRAIN_API ALTDepartureBoard : public AActor
{
	GENERATED_BODY()

public:
	ALTDepartureBoard();

	virtual void Tick(float DeltaSeconds) override;

	/** Which whole second to show now: seconds to arrival while Away or
		Approaching, seconds to departure while Dwelling, 0 while Departing. */
	UFUNCTION(BlueprintPure, Category = "Departure board")
	int32 GetDisplaySeconds() const { return DisplaySeconds; }

	UFUNCTION(BlueprintPure, Category = "Departure board")
	ELTTrainPhase GetDisplayPhase() const { return DisplayPhase; }

	/** True while the doors are open on a dwelling train, so the sign can show a
		boarding state rather than a countdown. */
	UFUNCTION(BlueprintPure, Category = "Departure board")
	bool IsBoardingOpen() const;

	/** False on a train-less level, so a Blueprint can show a blank or a no
		service state rather than a stuck zero. */
	UFUNCTION(BlueprintPure, Category = "Departure board")
	bool HasTrain() const { return Train != nullptr; }

	/** Optional. Leave null to bind the first ALTTrain found in the level. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Departure board")
	TObjectPtr<ALTTrain> TrainOverride;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** The whole second to show changed. The only hook a sign needs to redraw. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Departure board")
	void OnCountdownChanged(int32 WholeSeconds, ELTTrainPhase Phase);

	/** Forwarded from the train, for a state light or a colour change. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Departure board")
	void OnPhaseChanged(ELTTrainPhase NewPhase, ELTTrainPhase OldPhase);

private:
	UFUNCTION()
	void HandleTrainPhaseChanged(ELTTrainPhase NewPhase, ELTTrainPhase OldPhase);

	/** Reads the train, recomputes DisplaySeconds and fires OnCountdownChanged if
		the whole-second value moved. */
	void Refresh();

	UPROPERTY(VisibleInstanceOnly, Category = "Departure board")
	TObjectPtr<ALTTrain> Train;

	/** Starts off any real value, so the first Refresh always fires the hook, even
		when the countdown genuinely reads zero. */
	int32 DisplaySeconds = -1;

	ELTTrainPhase DisplayPhase = ELTTrainPhase::Away;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/LTInteractableInterface.h"
#include "LTTrain.generated.h"

class UBoxComponent;
class ULTStationHeat;

/** Where the train is in its cycle. Drives presentation and the boarding window. */
UENUM(BlueprintType)
enum class ELTTrainPhase : uint8
{
	/** No train. Counting down to the next arrival. */
	Away,
	/** Inbound. The arrival slide is playing. Doors shut, no boarding. */
	Approaching,
	/** Stopped at the platform. Doors open for part of this. Boarding is live. */
	Dwelling,
	/** Doors shut, pulling out. No boarding. Heat has already been applied. */
	Departing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrainPhaseChanged, ELTTrainPhase, NewPhase, ELTTrainPhase, OldPhase);

/** The train for one station arena. Owns the arrive, dwell, depart and away
	timing, the presentation hooks a station Blueprint drives its mesh, lights,
	doors and audio from, and the boarding interact. It stays thin: boarding
	calls one game mode hook and lets the game mode orchestrate the consequences.
	Travel to another station is a later task. */
UCLASS()
class LASTTRAIN_API ALTTrain : public AActor, public ILTInteractableInterface
{
	GENERATED_BODY()

public:
	ALTTrain();

	UPROPERTY(BlueprintAssignable, Category = "Train")
	FOnTrainPhaseChanged OnTrainPhaseChanged;

	/** Seconds from one departure to the next arrival. brief-v2 TRAIN_INTERVAL_MS is 100000. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float TrainInterval = 100.f;

	/** Seconds the train sits stopped at the platform. brief-v2 TRAIN_DWELL_MS is 25000.
		Measured stop to start, not door to door. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float DwellDuration = 25.f;

	/** Seconds the inbound slide takes. Presentation only: the state is Approaching
		for this long before the train counts as stopped. 4s is a grey box stub, a
		full train needs 10 to 12s of visible deceleration. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float ArrivalSlideSeconds = 4.f;

	/** Seconds the outbound slide takes. Departing for this long, then Away. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float DepartureSlideSeconds = 4.f;

	/** After the train stops, wait this long before the doors open. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float DoorOpenDelaySeconds = 1.f;

	/** Doors close this many seconds before the train starts to depart, so the
		open-door boarding window is DwellDuration - DoorOpenDelaySeconds - this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float DoorCloseLeadSeconds = 3.f;

	/** Seconds before the train STOPS that the inbound announcement fires. brief-v2:
		"15 seconds before arrival". Fires during the Approaching slide if
		ArrivalSlideSeconds is under this, otherwise late in Away. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float InboundAnnouncementLeadSeconds = 15.f;

	/** The first arrival has no preceding departure to count from, so the first
		cycle instead stops at level load plus this many seconds. TrainInterval
		governs every cycle after it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
	float FirstTrainStopSeconds = 30.f;

	/** True while the arrival still warrants the full spectacle. Cleared after the
		third arrival so a Blueprint can pick a shorter cycle. Timing is unaffected. */
	UPROPERTY(BlueprintReadOnly, Category = "Train")
	bool bFullArrivalPresentation = true;

	UFUNCTION(BlueprintPure, Category = "Train")
	ELTTrainPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category = "Train")
	bool AreDoorsOpen() const { return bDoorsOpen; }

	/** Seconds until the train next comes to a stop. Zero while it is stopped or
		inbound past the stop point. For the departure board. */
	UFUNCTION(BlueprintPure, Category = "Train")
	float GetSecondsUntilArrival() const;

	/** Seconds until the train next starts to pull away. Zero unless it is Dwelling.
		For the departure board. */
	UFUNCTION(BlueprintPure, Category = "Train")
	float GetSecondsUntilDeparture() const;

	/** The boarding trigger calls this. Returns false and does nothing if boarding
		is not currently allowed (wrong phase, doors shut, run already over). */
	UFUNCTION(BlueprintCallable, Category = "Train")
	bool TryBoard(AActor* Boarder);

	virtual void Tick(float DeltaSeconds) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;

	/** T-15s inbound (or on entering Approaching if the slide is long). Diegetic
		platform announcement. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnInboundAnnouncement();

	/** The inbound slide begins. Headlights on, rumble bed ramps, the mesh starts
		its move down the trackbed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnArrivalStarted();

	/** The train has stopped. Brake screech one-shot, the "on arrival" announcement. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnArrivalComplete();

	/** Doors open. One event whether the station has car doors only or car plus
		screen doors. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnDoorsOpen();

	/** Doors close. Fires DoorCloseLeadSeconds before departure. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnDoorsClose();

	/** "Stand clear of the doors" announcement, fired with OnDoorsClose. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnDepartureAnnouncement();

	/** The outbound slide begins. Rumble fades, the mesh pulls away. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnDepartureStarted();

	/** The train has fully left. The trackbed is empty again. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnTrainAway();

	/** A successful board. Blueprint plays a door-chime and a board confirm before
		the screen goes to the travel transition a later task owns. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnPlayerBoarded();

	/** The train left without the player. A Blueprint plays the heat-rise sting and
		the station's crimson creep step. brief-v2: staying raises heat by 1. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void OnTrainDeparted_NotBoarded();

	/** Scopes the boarding interact to the door aperture. Overlap only, no block. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<UBoxComponent> BoardingVolume;

private:
	/** Moves to a new phase, sets its timer, logs and broadcasts the change. */
	void EnterPhase(ELTTrainPhase NewPhase, float NewPhaseTimer);

	/** True while the run is still playable, so the train should keep cycling. */
	bool IsRunLive() const;

	/** The round manager's heat component if there is one, otherwise any heat
		component in the level. Null on a heat-less test level. */
	ULTStationHeat* FindStationHeat() const;

	void TickAway();
	void TickApproaching();
	void TickDwelling();
	void TickDeparting();

	UPROPERTY(VisibleInstanceOnly, Category = "Train")
	ELTTrainPhase Phase = ELTTrainPhase::Away;

	/** Counts down within the current phase. Meaning depends on Phase. */
	float PhaseTimer = 0.f;

	/** True once the current cycle's inbound announcement hook has fired. */
	bool bInboundAnnouncementFired = false;

	/** True while the doors are open this dwell. Gates the boarding trigger. */
	bool bDoorsOpen = false;

	/** True once IncrementHeat has run for the current departure, so a long
		Departing phase cannot double count. */
	bool bDepartureHeatApplied = false;

	/** Arrivals so far this run. Clears bFullArrivalPresentation after the third. */
	int32 ArrivalsStarted = 0;
};

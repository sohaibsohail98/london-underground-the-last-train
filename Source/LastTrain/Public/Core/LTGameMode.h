#pragma once

#include "CoreMinimal.h"
#include "Core/LTGameInstance.h"
#include "Core/LTGameState.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/GameModeBase.h"
#include "LTGameMode.generated.h"

class ALTRoundManager;
class ULTStationHeat;

/** Owns the run lifecycle for one station arena. Thin: it flips ALTGameState
	between run states and starts the round manager. Boarding a train ends the run
	here and hands a travel payload to ULTGameInstance, which opens the next
	station; the menu flow is not here yet. */
UCLASS()
class LASTTRAIN_API ALTGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALTGameMode();

	/** Flips the run to Active and starts rounds. Safe to call once. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void StartRun();

	/** Called by the player character when its health reaches zero. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void NotifyPlayerDied();

	/** Called when the player is downed but not yet dead. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void NotifyPlayerDowned();

	/** Called when a revive brings a downed player back up. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void NotifyPlayerRevived();

	/** Called by ALTTrain when the player boards during the dwell. In this arena it
		ends the run: stops rounds, banks points, refills the reserve, resets heat,
		and flips the run state to Boarded. It then builds the travel payload and
		asks the game instance to open NextStationMap. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void NotifyPlayerBoarded(AActor* Boarder);

	/** If true, StartRun is called automatically on BeginPlay. Off for a build
		that opens on a menu or a countdown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	bool bAutoStart = true;

	/** The station label for each map, by map asset name, stamped onto the game
		state on BeginPlay. Keyed like StationRoutes and for the same reason: one
		game mode Blueprint serves every station, so a lone name would label them
		all the same. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	TMap<FName, FText> StationDisplayNames;

	/** Label for a map with no entry of its own, and the whole answer for a
		project that gives each station its own game mode. Empty leaves the label
		blank, which is what a station with no name set has always read as. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	FText StationDisplayName;

	/** Where boarding a train goes from each station: this map's asset name to the
		destination's. Checked first, because one game mode Blueprint serves every
		station and EditDefaultsOnly is a default on the whole game mode, so a lone
		NextStationMap would send every station to the same place. v1 ships two
		stations, so this is two entries pointing at each other. A station-select
		picker for a third station onwards replaces the value with a choice. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Travel")
	TMap<FName, FName> StationRoutes;

	/** Fallback destination for a station with no route of its own, and the whole
		answer for a project that gives each station its own game mode. Unset, with
		no route either, means boarding ends the run where it stands. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Travel")
	FName NextStationMap;

	/** Seconds between a successful board and the level load, so the train's
		boarding hooks and a fade have frames to play. The run state is already
		Boarded, so the arena is frozen for the wait. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Travel")
	float TravelDelaySeconds = 1.5f;

protected:
	virtual void BeginPlay() override;

private:
	void SetState(ELTRunState NewState);
	ALTRoundManager* FindRoundManager() const;

	/** Puts this map's display name on the game state, so GetStationName has an
		answer for the HUD and the departure board. */
	void StampStationName();

	/** Grants the carried points and weapon from a train arrival, then clears the
		payload. Runs a tick after StartRun so the pawn's own components have had
		their BeginPlay and cannot stamp their starting values over the carry. */
	void RehydrateFromTravel();

	/** Hands the payload to the game instance and leaves this level. Deferred off
		NotifyPlayerBoarded by TravelDelaySeconds. */
	void BeginPendingTravel();

	/** The route for this map, else NextStationMap, else None. */
	FName ResolveDestinationMap() const;

	/** Hands the finished run to the game instance, which folds it into the
		progression save and writes it out. Called on a death and on a boarding,
		the two ends of a run ELTRunState already distinguishes. */
	void RecordRunOutcome(bool bBoarded) const;

	/** The round manager's heat component if there is one, otherwise any heat
		component in the level. Null on a heat-less test level. */
	ULTStationHeat* FindStationHeat() const;

	/** Snapshotted at the moment of boarding, spent when the travel timer fires. */
	UPROPERTY()
	FLTTravelPayload PendingTravelPayload;

	FName PendingTravelDestination;

	FTimerHandle TravelTimer;

	/** Ticks spent waiting for a player pawn to rehydrate into. */
	int32 RehydrateAttempts = 0;

	bool bRunStarted = false;
};

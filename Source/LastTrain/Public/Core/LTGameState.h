#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LTGameState.generated.h"

/** The run lifecycle. One station arena at a time. */
UENUM(BlueprintType)
enum class ELTRunState : uint8
{
	/** Level loaded, rounds not started. Waiting on the first round or a menu. */
	PreGame,
	/** Rounds running. The normal play state. */
	Active,
	/** The player is downed. Rounds continue. A revive returns to Active. */
	Downed,
	/** The player is dead. The run is over. */
	Dead,
	/** The player boarded the train. The run continues at the next station,
	    handled by a travel transition, not by this arena. */
	Boarded
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRunStateChanged, ELTRunState, NewState, ELTRunState, OldState);

/** Authoritative run state for one station arena. The game mode drives it, the
    HUD and systems read it. Travel between stations is a separate transition
    that spawns a fresh arena, so this only ever describes the current one. */
UCLASS()
class LASTTRAIN_API ALTGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRunStateChanged OnRunStateChanged;

	UFUNCTION(BlueprintPure, Category = "Run")
	ELTRunState GetRunState() const { return RunState; }

	UFUNCTION(BlueprintPure, Category = "Run")
	bool IsRunOver() const { return RunState == ELTRunState::Dead; }

	/** Display name of the station this arena represents. Set on travel in. */
	UFUNCTION(BlueprintPure, Category = "Run")
	FText GetStationName() const { return StationName; }

	/** Game mode only. Moves the run to a new state and broadcasts the change. */
	void SetRunState(ELTRunState NewState);

	/** Game mode only. Sets the station label for this arena. */
	void SetStationName(const FText& InName) { StationName = InName; }

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Run")
	ELTRunState RunState = ELTRunState::PreGame;

	UPROPERTY(VisibleInstanceOnly, Category = "Run")
	FText StationName;
};

#pragma once

#include "CoreMinimal.h"
#include "Core/LTGameState.h"
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

	/** The map boarding a train travels to, as the asset name, for example
		L_CanaryWharf_Greybox. v1 ships two stations, so there is exactly one other
		and no destination to choose: a station-select picker for a third station
		onwards would replace this single name with a per-station map. Unset means
		boarding ends the run where it stands. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Travel")
	FName NextStationMap;

protected:
	virtual void BeginPlay() override;

private:
	void SetState(ELTRunState NewState);
	ALTRoundManager* FindRoundManager() const;

	/** Grants the carried points and weapon from a train arrival, then clears the
		payload. Runs a tick after StartRun so the pawn's own components have had
		their BeginPlay and cannot stamp their starting values over the carry. */
	void RehydrateFromTravel();

	/** The round manager's heat component if there is one, otherwise any heat
		component in the level. Null on a heat-less test level. */
	ULTStationHeat* FindStationHeat() const;

	bool bRunStarted = false;
};

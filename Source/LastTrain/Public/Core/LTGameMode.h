#pragma once

#include "CoreMinimal.h"
#include "Core/LTGameState.h"
#include "GameFramework/GameModeBase.h"
#include "LTGameMode.generated.h"

class ALTRoundManager;

/** Owns the run lifecycle for one station arena. Thin: it flips ALTGameState
	between run states and starts the round manager. Travel between stations,
	boarding, and the menu flow are not here yet, they are Phase C decisions. */
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

	/** If true, StartRun is called automatically on BeginPlay. Off for a build
		that opens on a menu or a countdown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	bool bAutoStart = true;

protected:
	virtual void BeginPlay() override;

private:
	void SetState(ELTRunState NewState);
	ALTRoundManager* FindRoundManager() const;

	bool bRunStarted = false;
};

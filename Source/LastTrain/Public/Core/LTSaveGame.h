#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LTSaveGame.generated.h"

/** What one station remembers between runs. Three counters, each one a thing the
	game already measures: the round manager's round number, and the two ends of
	the run lifecycle the game mode already detects. Nothing here is a score, an
	achievement or an unlock, because nothing in the design docs asks for one. */
USTRUCT(BlueprintType)
struct FLTStationRecord
{
	GENERATED_BODY()

	/** Highest round reached at this station across every run. 0 means the
		station has never been played. */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 BestRound = 0;

	/** Runs that began at this station on a cold start. Arriving by train
		continues the run it came from, so an arrival is not counted again here. */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 RunsStarted = 0;

	/** Runs that got out of this station on the train. Counted once per run,
		however many times that one run boards here. */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 RunsBoarded = 0;
};

/** The disk save for progression, the later task FLTTravelPayload's doc comment
	names. One profile, one slot: v1 has no profile picker, so there is nothing to
	choose between. Settings live in ULTSettingsSaveGame and a separate slot, so a
	lost or reset profile does not also cost the player their volume and field of
	view.

	ULTGameInstance owns the only instance of this and is the only thing that
	writes to it. Everything here is per station map asset name, keyed exactly as
	ULTGameInstance::VisitedStations and ALTGameMode::StationRoutes already key
	their maps. */
UCLASS()
class LASTTRAIN_API ULTSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Bumped whenever the meaning of a field below changes, so a load can tell a
		stale file from a current one. 1 is the first schema. */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 SchemaVersion = 1;

	/** Station map asset name to what that station remembers. A station with no
		entry has never been played. */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	TMap<FName, FLTStationRecord> StationRecords;

	/** Best round at one station. 0 when it has never been played. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetBestRound(FName StationMap) const;

	/** Best round at any station, for a single main menu line. 0 when nothing has
		been recorded yet. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetBestRoundAnywhere() const;

	/** This station's record, all zeroes when it has none. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	FLTStationRecord GetStationRecord(FName StationMap) const;

	/** Counts a cold start at this station. Game instance only. */
	void NoteRunStarted(FName StationMap);

	/** Folds the end of a run into this station's record and returns true when
		something actually changed, so the caller can skip a pointless disk write.
		bCountBoarding is false on a death, and false on a second board in the same
		run, which the game instance tracks. */
	bool NoteRunEnded(FName StationMap, int32 RoundReached, bool bCountBoarding);
};

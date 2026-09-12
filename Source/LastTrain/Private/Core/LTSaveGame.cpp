#include "Core/LTSaveGame.h"

int32 ULTSaveGame::GetBestRound(const FName StationMap) const
{
	const FLTStationRecord* Record = StationRecords.Find(StationMap);
	return Record ? Record->BestRound : 0;
}

int32 ULTSaveGame::GetBestRoundAnywhere() const
{
	int32 Best = 0;

	for (const TPair<FName, FLTStationRecord>& Pair : StationRecords)
	{
		Best = FMath::Max(Best, Pair.Value.BestRound);
	}

	return Best;
}

FLTStationRecord ULTSaveGame::GetStationRecord(const FName StationMap) const
{
	const FLTStationRecord* Record = StationRecords.Find(StationMap);
	return Record ? *Record : FLTStationRecord();
}

void ULTSaveGame::NoteRunStarted(const FName StationMap)
{
	if (StationMap.IsNone())
	{
		return;
	}

	FLTStationRecord& Record = StationRecords.FindOrAdd(StationMap);
	Record.RunsStarted += 1;
}

bool ULTSaveGame::NoteRunEnded(const FName StationMap, const int32 RoundReached, const bool bCountBoarding)
{
	if (StationMap.IsNone())
	{
		return false;
	}

	if (RoundReached <= 0 && !bCountBoarding)
	{
		// Nothing worth an entry, and adding one here would leave the map holding
		// a record the caller is about to decide is not worth a disk write.
		return false;
	}

	// A station the player only ever died at still earns a record: a best round of
	// 1 is a real answer, and RunsBoarded staying 0 is the interesting part of it.
	FLTStationRecord& Record = StationRecords.FindOrAdd(StationMap);

	bool bChanged = false;

	if (RoundReached > Record.BestRound)
	{
		Record.BestRound = RoundReached;
		bChanged = true;
	}

	if (bCountBoarding)
	{
		Record.RunsBoarded += 1;
		bChanged = true;
	}

	return bChanged;
}

#pragma once

#include "CoreMinimal.h"
#include "Core/LTSettingsSaveGame.h"
#include "Engine/GameInstance.h"
#include "LTGameInstance.generated.h"

class ULTSaveGame;
class ULTWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsApplied, FLTGameSettings, Settings);

/** What a boarding player carries from one station to the next. Held in memory on
	the game instance for the session only, and deliberately still so: a carry
	belongs to the run that earned it. What outlives a run goes to disk in
	ULTSaveGame instead. */
USTRUCT(BlueprintType)
struct FLTTravelPayload
{
	GENERATED_BODY()

	/** Points the player leaves the last station with. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	int32 CarriedPoints = 0;

	/** The weapon held on boarding. Null means the destination uses its default. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	TObjectPtr<ULTWeaponData> CarriedWeapon = nullptr;

	/** Reserve ammunition count for CarriedWeapon at the moment of boarding. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	int32 CarriedReserve = 0;

	/** Asset names of stations visited this run, oldest first, including the one
		just left. For a future no-immediate-backtrack rule and a run summary. */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	TArray<FName> VisitedStations;
};

/** Survives the level load between stations, so boarding a train can carry the
	run's points and weapon into the next arena, and outlives every level, so it
	is also where the two disk saves live: ULTSaveGame for progression and
	ULTSettingsSaveGame for settings. A run's rules still stay with the game mode;
	this only records what a finished run leaves behind. */
UCLASS()
class LASTTRAIN_API ULTGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	/** Fired by ApplySettingsNow, so a Blueprint can push a value somewhere the
		C++ has nothing to push it to yet. The obvious one is master volume once
		G1 audio has a real sound mix: all the C++ here does is set the audio
		device's primary volume, which is as far as a project with no mix asset
		can go. */
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingsApplied OnSettingsApplied;

	/** Stores the payload and opens DestinationMap. The game mode calls this from
		NotifyPlayerBoarded. Does nothing if DestinationMap is unset. */
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void BeginStationTravel(const FLTTravelPayload& Payload, FName DestinationMap);

	/** True between boarding and the destination game mode taking the payload. */
	UFUNCTION(BlueprintPure, Category = "Travel")
	bool IsTravelling() const { return bTravelling; }

	/** Hands the payload to the destination arena and clears the travelling flag.
		The game mode calls this exactly once on arrival. */
	UFUNCTION(BlueprintCallable, Category = "Travel")
	FLTTravelPayload ConsumePayload();

	/** Stations visited this run, oldest first. Accumulates across hops. */
	UFUNCTION(BlueprintPure, Category = "Travel")
	TArray<FName> GetVisitedStations() const { return VisitedStations; }

	/** Forgets the visited-station list. A cold start at a station is a new run,
		so the game mode calls this when it starts one that is not an arrival. */
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void ClearRunHistory();

	/** The loaded progression save. Never null after Init: a missing or unreadable
		file becomes a fresh one in memory rather than nothing at all. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	ULTSaveGame* GetProgress() const { return Progress; }

	/** Best round at one station map, 0 when it has never been played. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetBestRoundForStation(FName StationMap) const;

	/** Best round at any station, for a main menu line. 0 when nothing has been
		recorded yet. */
	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetBestRoundAnywhere() const;

	/** Counts a cold start at this station and writes it out. The game mode calls
		this from StartRun, and only when the run is not an arrival by train: an
		arrival continues the run it came from. */
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RecordRunStarted(FName StationMap);

	/** Folds a finished run into this station's record and writes it out. The game
		mode calls this from NotifyPlayerDied with bBoarded false and from
		NotifyPlayerBoarded with it true. Boarding is counted once per run per
		station however many times one run comes back through. */
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RecordRunEnded(FName StationMap, int32 RoundReached, bool bBoarded);

	/** Writes the progression save to its slot. Returns false if the write failed.
		Called for you by the two Record functions. */
	UFUNCTION(BlueprintCallable, Category = "Progress")
	bool SaveProgressToDisk();

	/** The settings in force. Defaults until Init has loaded, then whatever the
		slot held, clamped. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FLTGameSettings GetGameSettings() const;

	/** The one call a settings panel needs: clamp, keep, apply, write. Explicit
		rather than a hook on every slider, so the panel decides when a change
		counts. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAndSaveSettings(const FLTGameSettings& NewSettings);

	/** Pushes the settings in force at whatever is live right now, without
		writing anything. Called on load, on every map load, and by
		ApplyAndSaveSettings. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplySettingsNow();

	/** Writes the settings save to its slot. Returns false if the write failed.
		Called for you by ApplyAndSaveSettings. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool SaveSettingsToDisk();

	/** Writes Enhanced Input's own key mappings to their own slot. That system
		saves itself and is not part of ULTSettingsSaveGame, so a remapping panel
		calls this alongside ApplyAndSaveSettings. Returns false, and says why in
		the log, when Enable User Settings is off in the Enhanced Input project
		settings, which is where it stands today. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool SaveKeyBindings();

protected:
	/** Fired just before the level opens, so a Blueprint can fade the screen out.
		The destination arena fades back in on its own BeginPlay. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Travel")
	void OnTravelStarted();

private:
	/** A payload is good for the one map load it was made for. If the destination
		never claims it, the next load drops it rather than letting a stale carry
		reach an unrelated run. */
	void HandlePostLoadMap(UWorld* LoadedWorld);

	/** Reads the progression slot, or makes an empty save when there is nothing
		to read. Leaves Progress non-null either way. */
	void LoadProgress();

	/** Reads the settings slot, or makes a default one. Leaves SettingsSave
		non-null either way, with its values clamped. */
	void LoadSettings();

	/** Master volume onto the audio device. Transient, hence the reapply on every
		map load: a project with no sound mix asset has nothing more durable to
		set, and G1 audio replaces this with a real mix. */
	void ApplyMasterVolume() const;

	/** Field of view onto the local player pawn, if there is one to push it at. */
	void ApplyCameraSettings() const;

	UPROPERTY()
	FLTTravelPayload PendingPayload;

	/** Loaded in Init, written at the end of a run. Never null after Init. */
	UPROPERTY()
	TObjectPtr<ULTSaveGame> Progress;

	/** Loaded in Init, written when a settings panel applies. Never null after
		Init. */
	UPROPERTY()
	TObjectPtr<ULTSettingsSaveGame> SettingsSave;

	/** Stations this run has already been credited a boarding at, so a run that
		ping-pongs between two stations counts each of them once. Cleared by
		ClearRunHistory, which a cold start already calls. */
	TSet<FName> BoardedStationsThisRun;

	/** The run's station history, kept beyond a single hop's payload. */
	UPROPERTY()
	TArray<FName> VisitedStations;

	UPROPERTY()
	bool bTravelling = false;

	/** True once the destination map has loaded with the payload still pending. */
	bool bPayloadLoadSeen = false;

	FDelegateHandle PostLoadMapHandle;
};

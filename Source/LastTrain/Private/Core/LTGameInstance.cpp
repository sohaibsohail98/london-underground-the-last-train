#include "Core/LTGameInstance.h"

#include "AudioDevice.h"
#include "Core/LTSaveGame.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "Player/LTPlayerCharacter.h"
#include "UObject/UObjectGlobals.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Weapons/LTWeaponData.h"

namespace
{
	/** One profile, one slot each. v1 has no profile picker, so there is nothing
		to choose between, and two slots rather than one keeps a lost progression
		file from also resetting the player's settings. */
	const TCHAR* ProgressSlotName = TEXT("LastTrainProgress");
	const TCHAR* SettingsSlotName = TEXT("LastTrainSettings");

	/** Single local player, so the platform user index never moves off 0. */
	constexpr int32 SaveUserIndex = 0;
} // namespace

void ULTGameInstance::Init()
{
	Super::Init();

	PostLoadMapHandle =
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULTGameInstance::HandlePostLoadMap);

	LoadProgress();
	LoadSettings();

	// There is no world yet, so this reaches the delegate and nothing else. Every
	// map load reapplies, which is what actually puts the volume on the device.
	ApplySettingsNow();
}

void ULTGameInstance::Shutdown()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	Super::Shutdown();
}

void ULTGameInstance::BeginStationTravel(const FLTTravelPayload& Payload, const FName DestinationMap)
{
	if (DestinationMap.IsNone())
	{
		LT_LOG(Warning, TEXT("BeginStationTravel called with no destination map. Staying put."));
		return;
	}

	PendingPayload = Payload;
	VisitedStations = Payload.VisitedStations;
	bTravelling = true;
	bPayloadLoadSeen = false;

	LT_LOG(
		Log, TEXT("Travelling to %s. Carrying %d points, weapon %s, reserve %d."), *DestinationMap.ToString(),
		Payload.CarriedPoints, Payload.CarriedWeapon ? *Payload.CarriedWeapon->GetName() : TEXT("none"),
		Payload.CarriedReserve);

	OnTravelStarted();

	UGameplayStatics::OpenLevel(this, DestinationMap);
}

FLTTravelPayload ULTGameInstance::ConsumePayload()
{
	const FLTTravelPayload Payload = PendingPayload;

	// Let go of the carried weapon reference as soon as the arena has it, so a
	// stale payload cannot keep an asset loaded for the rest of the session.
	PendingPayload = FLTTravelPayload();
	bTravelling = false;
	bPayloadLoadSeen = false;

	return Payload;
}

void ULTGameInstance::ClearRunHistory()
{
	VisitedStations.Reset();
	BoardedStationsThisRun.Reset();
}

void ULTGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
	// The audio device's primary volume is transient and the pawn is new, so the
	// settings in force have to be pushed again at every arena.
	ApplySettingsNow();

	if (!bTravelling)
	{
		bPayloadLoadSeen = false;
		return;
	}

	if (!bPayloadLoadSeen)
	{
		// The destination's own load. The arena has this session to claim it.
		bPayloadLoadSeen = true;
		return;
	}

	// A second load with the payload still here means nothing claimed it: a map
	// with no ALTGameMode, or one whose run never starts. Drop it rather than
	// granting a stale carry to whatever loads next.
	LT_LOG(
		Warning, TEXT("A travel payload went unclaimed for a whole map. Dropping %d points and the carried weapon."),
		PendingPayload.CarriedPoints);

	PendingPayload = FLTTravelPayload();
	VisitedStations.Reset();
	bTravelling = false;
	bPayloadLoadSeen = false;
}

void ULTGameInstance::LoadProgress()
{
	if (UGameplayStatics::DoesSaveGameExist(ProgressSlotName, SaveUserIndex))
	{
		Progress = Cast<ULTSaveGame>(UGameplayStatics::LoadGameFromSlot(ProgressSlotName, SaveUserIndex));
	}

	if (Progress)
	{
		LT_LOG(
			Log, TEXT("Loaded progress for %d station(s). Best round anywhere is %d."), Progress->StationRecords.Num(),
			Progress->GetBestRoundAnywhere());
		return;
	}

	// Nothing on disk, or a file this build cannot read. Either way the player
	// plays: an empty save in memory is written out the first time a run ends.
	Progress = Cast<ULTSaveGame>(UGameplayStatics::CreateSaveGameObject(ULTSaveGame::StaticClass()));

	LT_LOG(Log, TEXT("No progress save to load. Starting a fresh one."));
}

void ULTGameInstance::LoadSettings()
{
	if (UGameplayStatics::DoesSaveGameExist(SettingsSlotName, SaveUserIndex))
	{
		SettingsSave = Cast<ULTSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SettingsSlotName, SaveUserIndex));
	}

	if (!SettingsSave)
	{
		SettingsSave =
			Cast<ULTSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(ULTSettingsSaveGame::StaticClass()));

		LT_LOG(Log, TEXT("No settings save to load. Starting on the defaults."));
	}

	if (SettingsSave)
	{
		// A hand-edited or stale file must not reach the camera or the mixer with
		// a value neither can honour.
		SettingsSave->Settings.Sanitise();
	}
}

FLTGameSettings ULTGameInstance::GetGameSettings() const
{
	return SettingsSave ? SettingsSave->Settings : FLTGameSettings();
}

void ULTGameInstance::ApplyAndSaveSettings(const FLTGameSettings& NewSettings)
{
	if (!SettingsSave)
	{
		LT_LOG(Warning, TEXT("ApplyAndSaveSettings before Init loaded a settings save. Nothing applied."));
		return;
	}

	SettingsSave->Settings = NewSettings;
	SettingsSave->Settings.Sanitise();

	ApplySettingsNow();
	SaveSettingsToDisk();
}

void ULTGameInstance::ApplySettingsNow()
{
	ApplyMasterVolume();
	ApplyCameraSettings();

	OnSettingsApplied.Broadcast(GetGameSettings());
}

void ULTGameInstance::ApplyMasterVolume() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (FAudioDeviceHandle Device = World->GetAudioDevice())
	{
		Device->SetTransientPrimaryVolume(GetGameSettings().MasterVolume);
	}
}

void ULTGameInstance::ApplyCameraSettings() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ALTPlayerCharacter* Player = Cast<ALTPlayerCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)))
	{
		Player->SetBaseFieldOfView(GetGameSettings().FieldOfView);
	}
}

bool ULTGameInstance::SaveSettingsToDisk()
{
	if (!SettingsSave)
	{
		return false;
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SettingsSave, SettingsSlotName, SaveUserIndex);
	if (!bSaved)
	{
		LT_LOG(Warning, TEXT("Could not write the settings save. The player's choices last only this session."));
	}

	return bSaved;
}

bool ULTGameInstance::SaveKeyBindings()
{
	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (!LocalPlayer)
	{
		LT_LOG(Warning, TEXT("SaveKeyBindings with no local player. Nothing saved."));
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* Input =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!Input)
	{
		LT_LOG(Warning, TEXT("SaveKeyBindings found no Enhanced Input subsystem. Nothing saved."));
		return false;
	}

	UEnhancedInputUserSettings* UserSettings = Input->GetUserSettings();
	if (!UserSettings)
	{
		// Enhanced Input only builds its user settings when the project asks it
		// to, and this project has not yet. Rebinding is inert until it does, and
		// the rest of the settings are unaffected.
		LT_LOG(Warning, TEXT("Enhanced Input user settings are off, so there are no key bindings to save."));
		LT_LOG(Warning, TEXT("Tick Enable User Settings in the Enhanced Input project settings to turn them on."));
		return false;
	}

	UserSettings->ApplySettings();
	UserSettings->SaveSettings();

	LT_LOG(Log, TEXT("Key bindings written to the Enhanced Input user settings slot."));

	return true;
}

int32 ULTGameInstance::GetBestRoundForStation(const FName StationMap) const
{
	return Progress ? Progress->GetBestRound(StationMap) : 0;
}

int32 ULTGameInstance::GetBestRoundAnywhere() const
{
	return Progress ? Progress->GetBestRoundAnywhere() : 0;
}

void ULTGameInstance::RecordRunStarted(const FName StationMap)
{
	if (!Progress || StationMap.IsNone())
	{
		return;
	}

	Progress->NoteRunStarted(StationMap);
	SaveProgressToDisk();
}

void ULTGameInstance::RecordRunEnded(const FName StationMap, const int32 RoundReached, const bool bBoarded)
{
	if (!Progress || StationMap.IsNone())
	{
		return;
	}

	// A run that boards here twice is still one run that got out of here, so the
	// second boarding updates the best round and nothing else.
	const bool bCountBoarding = bBoarded && !BoardedStationsThisRun.Contains(StationMap);
	if (bCountBoarding)
	{
		BoardedStationsThisRun.Add(StationMap);
	}

	if (!Progress->NoteRunEnded(StationMap, RoundReached, bCountBoarding))
	{
		return;
	}

	LT_LOG(
		Log, TEXT("Run ended at %s on round %d, boarded %s. Best round there is now %d."), *StationMap.ToString(),
		RoundReached, bBoarded ? TEXT("yes") : TEXT("no"), Progress->GetBestRound(StationMap));

	SaveProgressToDisk();
}

bool ULTGameInstance::SaveProgressToDisk()
{
	if (!Progress)
	{
		return false;
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(Progress, ProgressSlotName, SaveUserIndex);
	if (!bSaved)
	{
		LT_LOG(Warning, TEXT("Could not write the progress save. This run will not be remembered."));
	}

	return bSaved;
}

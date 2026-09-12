#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LTSettingsSaveGame.generated.h"

/** Every player-facing setting the game currently has something to apply it to.
	Deliberately three values: each one either drives a property that already
	exists in Source/LastTrain, or is the agreed seam for a system being built on
	another branch. A setting with nothing behind it is worse than no setting, so
	nothing speculative belongs here.

	Key bindings are not in this struct. Enhanced Input's own
	UEnhancedInputUserSettings is itself a USaveGame with its own slot, so
	duplicating a key map here would give the player two files that disagree. See
	ULTGameInstance::SaveKeyBindings. */
USTRUCT(BlueprintType)
struct FLTGameSettings
{
	GENERATED_BODY()

	/** 0 silent to 1 full. One float, not a mix bus layout: G1 audio has not
		decided its bus structure yet, and this is the value a slider moves
		whatever that structure turns out to be. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MasterVolume = 1.f;

	/** Horizontal field of view in degrees at the hip. Lands on
		ALTPlayerCharacter::BaseFieldOfView, so the aim lerp narrows from whatever
		the player chose rather than from the coded default. The default here is
		that coded default, 95. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float FieldOfView = 95.f;

	/** Subtitles for the announcements and any other spoken line. Nothing reads
		this yet: the subtitle system is being built separately and this is the
		single bool it expects to find here, so the two meet without a schema
		change on either side. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bSubtitlesEnabled = false;

	/** Clamps every value into the range the game can actually honour. Called on
		load, so a hand-edited or stale file cannot hand the camera a field of view
		of zero, and on apply, so a widget cannot either. */
	void Sanitise();
};

/** The disk save for settings, in its own slot away from ULTSaveGame. Two slots
	rather than one because the two are written on completely different cadences,
	progression at the end of a run and settings the moment a panel applies, and
	because losing a profile should not also reset the player's volume.

	ULTGameInstance owns the only instance of this. A settings panel calls
	ULTGameInstance::ApplyAndSaveSettings rather than writing here directly, so
	applying and saving stay one explicit call, the way every other cross-system
	call in this project is explicit. */
UCLASS()
class LASTTRAIN_API ULTSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Bumped whenever the meaning of a field in FLTGameSettings changes. 1 is
		the first schema. */
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	int32 SchemaVersion = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FLTGameSettings Settings;
};

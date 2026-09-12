#pragma once

#include "CoreMinimal.h"
#include "Audio/LTSubtitleLine.h"
#include "Subsystems/WorldSubsystem.h"
#include "LTSubtitleSubsystem.generated.h"

class UDataTable;

/** The one line on screen, or empty text when it has been cleared. A widget
	binds this and never polls: the same delegate driven shape the HUD already
	uses for ULTInteractionComponent::OnInteractableChanged. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubtitleChanged, const FText&, Subtitle);

/** Captions for the things the game says and the things it makes a noise about.

	This is a UWorldSubsystem, matching ULTGoreDecalSubsystem, and for the same
	reason arrived at independently rather than for consistency alone: what it
	holds is world scoped. The line on screen describes something that just
	happened in this station, the widget bound to OnSubtitleChanged belongs to
	this world's player, and the expiry timer runs on this world's timer manager.
	Boarding the train travels to NextStationMap through OpenLevel, and a caption
	about doors closing at the station you left has no business surviving into
	the next one, any more than the decal pool's components do. One instance per
	world, created with it and torn down with it, is the lifetime this wants.

	What is deliberately NOT world scoped is the on or off switch. That is the
	player's setting, it lives in FLTGameSettings on the game instance and is
	written to disk, and this subsystem reads it there every time rather than
	caching a copy: see AreSubtitlesEnabled.

	Nothing here needs an actor or a component, so any caller with a world can
	drive it through the static ShowSubtitleForWorld, null safe at every step. */
UCLASS()
class LASTTRAIN_API ULTSubtitleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Subtitles")
	FOnSubtitleChanged OnSubtitleChanged;

	/** The one line entry point for a gameplay call site. Looks Key up in the
		table, falls back to FallbackText when the table has no such row, and does
		nothing at all if there is no world, no subsystem or no player setting
		saying yes. A caller never has to check first. */
	static void
	ShowSubtitleForWorld(const UWorld* World, FName Key, ELTSubtitleCategory Category, const FText& FallbackText);

	/** The subsystem for this world, or null. */
	static ULTSubtitleSubsystem* Get(const UWorld* World);

	/** Puts a line up, if subtitles are on, if it has text, if it is at least as
		important as whatever is already on screen, and if the same key has not
		just been shown. Returns true only when the line actually reached the
		screen. */
	UFUNCTION(BlueprintCallable, Category = "Subtitles")
	bool ShowSubtitle(const FLTSubtitleLine& Line);

	/** A line with no table row behind it. Duration zero derives a reading time
		from the length of the text. */
	UFUNCTION(BlueprintCallable, Category = "Subtitles")
	bool ShowSubtitleText(const FText& Text, float DurationSeconds, ELTSubtitleCategory Category);

	/** The table row named Key, or FallbackText if the table has no such row.
		Every C++ call site goes through this, so the shipped caption script can
		be rewritten in the table without touching code, and a missing table is
		still legible rather than silent. */
	UFUNCTION(BlueprintCallable, Category = "Subtitles")
	bool ShowSubtitleByKey(FName Key, ELTSubtitleCategory Category, const FText& FallbackText);

	/** Takes the current line down early and broadcasts the clear. */
	UFUNCTION(BlueprintCallable, Category = "Subtitles")
	void ClearSubtitle();

	/** The player's setting, read from ULTGameInstance every time it is asked
		rather than cached, so there is exactly one copy of this bool in the
		project and a settings panel needs no wiring to this subsystem at all. */
	UFUNCTION(BlueprintPure, Category = "Subtitles")
	bool AreSubtitlesEnabled() const;

	/** The line on screen, or empty text. For a widget created after the
		broadcast it missed. */
	UFUNCTION(BlueprintPure, Category = "Subtitles")
	FText GetCurrentSubtitle() const { return CurrentText; }

	/** Assigns the caption script at runtime. A subsystem has no editable
		archetype in the editor, so this is the practical route, exactly as the
		gore pool takes its decal materials: the game mode or a Blueprint calls it
		once on BeginPlay. Null is allowed and simply leaves every call site on
		its own fallback text. */
	UFUNCTION(BlueprintCallable, Category = "Subtitles")
	void SetSubtitleTable(UDataTable* Table);

	/** A UDataTable of FLTSubtitleLine rows, keyed by the names in
		LTSubtitleKeys. Null until SetSubtitleTable is called. */
	UPROPERTY(BlueprintReadOnly, Category = "Subtitles")
	TObjectPtr<UDataTable> SubtitleTable;

	/** Floor on time to read, so a two word caption does not blink. */
	UPROPERTY(BlueprintReadOnly, Category = "Subtitles")
	float MinimumDurationSeconds = 1.6f;

	/** Ceiling, so a long line cannot sit over the fight indefinitely. */
	UPROPERTY(BlueprintReadOnly, Category = "Subtitles")
	float MaximumDurationSeconds = 6.f;

	/** Reading time per character for a line that carries no duration of its
		own. 0.055s is roughly 200 words a minute, the usual captioning rate. */
	UPROPERTY(BlueprintReadOnly, Category = "Subtitles")
	float SecondsPerCharacter = 0.055f;

	/** The same key cannot come back within this. The horde is the reason it
		exists: the live cap is 24 before heat and 42 at heat 3, so a per zombie
		cue with no suppression is a caption changing several times a second,
		which is unreadable and is not what the restrained HUD is for. */
	UPROPERTY(BlueprintReadOnly, Category = "Subtitles")
	float RepeatSuppressionSeconds = 4.f;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

private:
	/** Shared tail of the three Show functions: priority test, suppression test,
		expiry timer, broadcast. */
	bool Present(const FLTSubtitleLine& Line, FName Key);

	/** The line's own duration, or a reading time derived from its length,
		clamped either way. */
	float ResolveDuration(const FLTSubtitleLine& Line) const;

	/** The line's own priority, or the category default. */
	int32 ResolvePriority(const FLTSubtitleLine& Line) const;

	/** True if this line has been on screen too recently to show again. */
	bool IsSuppressed(FName Key, const FText& Text) const;

	/** The expiry timer fired: clear and broadcast. */
	void HandleExpiry();

	/** Cleared to empty text on expiry, which is what the widget hides on. */
	FText CurrentText;

	/** The key of the line on screen, None for a keyless one. */
	FName CurrentKey;

	/** Resolved priority of the line on screen, so a lesser one cannot cut in. */
	int32 CurrentPriority = 0;

	/** World time each key was last shown, for the repeat suppression. Small by
		construction: one entry per distinct key the game uses. */
	TMap<FName, float> LastShownTimes;

	FTimerHandle ExpiryTimerHandle;
};

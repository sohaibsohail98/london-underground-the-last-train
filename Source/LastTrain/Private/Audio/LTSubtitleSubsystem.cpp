#include "Audio/LTSubtitleSubsystem.h"

#include "Core/LTGameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "LastTrain.h"
#include "TimerManager.h"

namespace
{
	/** Where a category sits when a line does not name its own priority. Speech
		beats a state change beats a noise in the world, and the interact confirm
		is at the bottom because it is the one caption the HUD already says. */
	int32 DefaultPriorityForCategory(const ELTSubtitleCategory Category)
	{
		switch (Category)
		{
		case ELTSubtitleCategory::Announcement:
			return 40;
		case ELTSubtitleCategory::Round:
			return 30;
		case ELTSubtitleCategory::Train:
			return 20;
		case ELTSubtitleCategory::Zombie:
			return 15;
		case ELTSubtitleCategory::Weapon:
			return 10;
		case ELTSubtitleCategory::Interaction:
			return 5;
		}

		return 10;
	}
} // namespace

void ULTSubtitleSubsystem::ShowSubtitleForWorld(
	const UWorld* World, const FName Key, const ELTSubtitleCategory Category, const FText& FallbackText)
{
	if (ULTSubtitleSubsystem* Subsystem = Get(World))
	{
		Subsystem->ShowSubtitleByKey(Key, Category, FallbackText);
	}
}

ULTSubtitleSubsystem* ULTSubtitleSubsystem::Get(const UWorld* World)
{
	return World ? World->GetSubsystem<ULTSubtitleSubsystem>() : nullptr;
}

bool ULTSubtitleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// Gameplay worlds only. An editor preview or a thumbnail world has nobody to
	// read a caption.
	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr && World->IsGameWorld();
}

void ULTSubtitleSubsystem::Deinitialize()
{
	// The world is going, and with it the widget bound to OnSubtitleChanged, so
	// there is nothing to tell: drop the timer and the state without broadcasting.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpiryTimerHandle);
	}

	CurrentText = FText::GetEmpty();
	CurrentKey = NAME_None;
	CurrentPriority = 0;
	LastShownTimes.Empty();

	Super::Deinitialize();
}

bool ULTSubtitleSubsystem::AreSubtitlesEnabled() const
{
	const UWorld* World = GetWorld();
	const ULTGameInstance* GameInstance = World ? World->GetGameInstance<ULTGameInstance>() : nullptr;
	if (!GameInstance)
	{
		return false;
	}

	// The only cross branch line in this file. bSubtitlesEnabled is a field of
	// FLTGameSettings, which arrives with branch claude/save-settings-prompt and
	// is the one place the player's choice is kept and written to disk. Reading
	// it here every time, rather than caching a copy, is what keeps this system
	// from owning a second flag that could disagree with the saved one, and means
	// a settings panel needs no wiring to this subsystem at all. Until both
	// branches are on main this call does not compile: expected, and correct.
	return GameInstance->GetGameSettings().bSubtitlesEnabled;
}

void ULTSubtitleSubsystem::SetSubtitleTable(UDataTable* Table)
{
	SubtitleTable = Table;

	LT_LOG(Log, TEXT("Subtitle table set: %s"), *GetNameSafe(Table));
}

bool ULTSubtitleSubsystem::ShowSubtitleByKey(
	const FName Key, const ELTSubtitleCategory Category, const FText& FallbackText)
{
	FLTSubtitleLine Line;
	Line.Category = Category;
	Line.DisplayText = FallbackText;

	if (SubtitleTable)
	{
		// bWarnIfMissing off: a key with no row is the normal state until the
		// caption script is written, and the fallback covers it.
		if (const FLTSubtitleLine* Row = SubtitleTable->FindRow<FLTSubtitleLine>(Key, TEXT("LTSubtitle"), false))
		{
			if (Row->HasText())
			{
				Line.DisplayText = Row->DisplayText;
				Line.DurationSeconds = Row->DurationSeconds;
				Line.Priority = Row->Priority;

				// The row's own Category is deliberately not read back here. The
				// call site is code and knows what it is captioning, so it stays
				// authoritative and a row cannot give a moan announcement
				// priority by being left on the enum's first value. A row that
				// wants to out-rank its category says so in Priority.
			}
		}
	}

	return Present(Line, Key);
}

bool ULTSubtitleSubsystem::ShowSubtitle(const FLTSubtitleLine& Line)
{
	return Present(Line, NAME_None);
}

bool ULTSubtitleSubsystem::ShowSubtitleText(
	const FText& Text, const float DurationSeconds, const ELTSubtitleCategory Category)
{
	FLTSubtitleLine Line;
	Line.DisplayText = Text;
	Line.DurationSeconds = DurationSeconds;
	Line.Category = Category;

	return Present(Line, NAME_None);
}

void ULTSubtitleSubsystem::ClearSubtitle()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpiryTimerHandle);
	}

	if (CurrentText.IsEmpty())
	{
		return;
	}

	CurrentText = FText::GetEmpty();
	CurrentKey = NAME_None;
	CurrentPriority = 0;

	OnSubtitleChanged.Broadcast(CurrentText);
}

float ULTSubtitleSubsystem::ResolveDuration(const FLTSubtitleLine& Line) const
{
	const float Requested =
		Line.DurationSeconds > 0.f ? Line.DurationSeconds : Line.DisplayText.ToString().Len() * SecondsPerCharacter;

	return FMath::Clamp(Requested, MinimumDurationSeconds, MaximumDurationSeconds);
}

int32 ULTSubtitleSubsystem::ResolvePriority(const FLTSubtitleLine& Line) const
{
	return Line.Priority > 0 ? Line.Priority : DefaultPriorityForCategory(Line.Category);
}

bool ULTSubtitleSubsystem::IsSuppressed(const FName Key, const FText& Text) const
{
	if (Key.IsNone())
	{
		// A keyless line has nothing to remember it by, so the only repeat worth
		// catching is the one already on screen.
		return !CurrentText.IsEmpty() && CurrentText.IdenticalTo(Text);
	}

	const UWorld* World = GetWorld();
	const float* LastShown = LastShownTimes.Find(Key);
	if (!World || !LastShown)
	{
		return false;
	}

	return World->GetTimeSeconds() - *LastShown < RepeatSuppressionSeconds;
}

bool ULTSubtitleSubsystem::Present(const FLTSubtitleLine& Line, const FName Key)
{
	if (!Line.HasText() || !AreSubtitlesEnabled())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (IsSuppressed(Key, Line.DisplayText))
	{
		return false;
	}

	// Equal priority replaces, so a second announcement follows the first. Lower
	// priority does not, so the horde cannot bury the line that matters.
	const int32 Priority = ResolvePriority(Line);
	if (!CurrentText.IsEmpty() && Priority < CurrentPriority)
	{
		return false;
	}

	CurrentText = Line.DisplayText;
	CurrentKey = Key;
	CurrentPriority = Priority;

	if (!Key.IsNone())
	{
		LastShownTimes.Add(Key, World->GetTimeSeconds());
	}

	World->GetTimerManager().ClearTimer(ExpiryTimerHandle);
	World->GetTimerManager().SetTimer(
		ExpiryTimerHandle, this, &ULTSubtitleSubsystem::HandleExpiry, ResolveDuration(Line), false);

	LT_LOG(Verbose, TEXT("Subtitle: %s"), *CurrentText.ToString());

	OnSubtitleChanged.Broadcast(CurrentText);
	return true;
}

void ULTSubtitleSubsystem::HandleExpiry()
{
	CurrentText = FText::GetEmpty();
	CurrentKey = NAME_None;
	CurrentPriority = 0;

	OnSubtitleChanged.Broadcast(CurrentText);
}

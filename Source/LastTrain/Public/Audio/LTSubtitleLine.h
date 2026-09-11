#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LTSubtitleLine.generated.h"

/** What kind of moment a line captions. Two jobs: it gives a line a default
	priority without every caller having to pick a number, and it is the handle a
	later filter needs if the restrained answer turns out to be "announcements
	only". Heat has no category of its own: a heat rise is a run level stinger
	played the same way a round stinger is, so it shares Round. */
UENUM(BlueprintType)
enum class ELTSubtitleCategory : uint8
{
	/** A spoken platform announcement. The only category that captions speech,
		and the reason the settings bool exists at all. Wins over everything. */
	Announcement,
	/** The round and heat stingers. The player is told the run changed state. */
	Round,
	/** The train cycle: the approach, the doors, the departure slide. */
	Train,
	/** A zombie vocal or a screamer's call. Lowest of the world sounds, because
		there can be forty of them alive at once. */
	Zombie,
	/** The weapon: the reload, an empty magazine. Mostly duplicated by the HUD,
		so it sits at the bottom. */
	Weapon,
	/** The interact confirm. */
	Interaction
};

/** One caption. A USTRUCT deriving from FTableRowBase rather than a data asset
	per line, deliberately.

	ULTWeaponData and ULTZombieTypeData are one asset per instance because each
	carries dozens of fields and is referenced individually by a spawner or a
	wall buy. A subtitle line carries a sentence and a duration, there will be a
	few dozen of them, and nothing ever references one by asset pointer: the code
	asks for a row by key. One UDataTable of these rows is therefore the right
	shape, and it keeps the whole caption script in one place a writer can read
	top to bottom instead of scattered over thirty tiny assets and thirty LFS
	pointers.

	Deriving from FTableRowBase costs nothing and the struct is still usable
	inline, so a Blueprint that wants to hand ShowSubtitle a one off line can. */
USTRUCT(BlueprintType)
struct FLTSubtitleLine : public FTableRowBase
{
	GENERATED_BODY()

	/** The caption. Original wording in the project's own voice: nothing here
		transcribes or imitates a real announcement. A non speech sound is written
		in square brackets, the usual caption convention, so a player can tell a
		description of a noise from a line somebody said. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	FText DisplayText;

	/** Seconds on screen. Zero, the default, derives a reading time from the
		length of the text, which is what almost every line wants. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	float DurationSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	ELTSubtitleCategory Category = ELTSubtitleCategory::Announcement;

	/** Above zero, replaces the category's default priority. A line only reaches
		the screen if it is at least as important as the one already on it, so
		this is how a horde of moans fails to bury an announcement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	int32 Priority = 0;

	/** An empty line is a row somebody has not written yet. Never shown. */
	bool HasText() const { return !DisplayText.IsEmpty(); }
};

/** Row names for the subtitle table, shared by the call sites and by whoever
	authors the table, so the two cannot drift apart over a typo.

	TCHAR literals rather than FName constants on purpose: a namespace scope
	FName runs its constructor at static initialisation time, before the engine's
	name table is guaranteed to exist. FName takes a TCHAR pointer implicitly, so
	a call site reads the same either way and nothing runs before main. */
namespace LTSubtitleKeys
{
	/** Wired on this branch. */
	inline const TCHAR* const TrainInboundAnnouncement = TEXT("Train.InboundAnnouncement");
	inline const TCHAR* const TrainArrival = TEXT("Train.Arrival");
	inline const TCHAR* const TrainArrivalAnnouncement = TEXT("Train.ArrivalAnnouncement");
	inline const TCHAR* const TrainDoorsOpen = TEXT("Train.DoorsOpen");
	inline const TCHAR* const TrainDepartureAnnouncement = TEXT("Train.DepartureAnnouncement");
	inline const TCHAR* const TrainDeparture = TEXT("Train.Departure");
	inline const TCHAR* const RoundStart = TEXT("Round.Start");
	inline const TCHAR* const RoundEnd = TEXT("Round.End");
	inline const TCHAR* const HeatRise = TEXT("Round.HeatRise");
	inline const TCHAR* const InteractConfirm = TEXT("Interaction.Confirm");
	inline const TCHAR* const ZombieScream = TEXT("Zombie.Scream");

	/** Named here, called from nowhere yet. Each one belongs at a sound call site
		that arrives with branch claude/phase-g1-audio-prompt, so the session that
		merges the two wires these rather than inventing new names. The list and
		the reasoning are in docs/tasks/phase-g6-subtitles.md. */
	inline const TCHAR* const ZombieAggro = TEXT("Zombie.Aggro");
	inline const TCHAR* const ZombieAttack = TEXT("Zombie.Attack");
	inline const TCHAR* const ZombieDeath = TEXT("Zombie.Death");
	inline const TCHAR* const WeaponDryFire = TEXT("Weapon.DryFire");
	inline const TCHAR* const WeaponReloadStart = TEXT("Weapon.ReloadStart");
	inline const TCHAR* const WeaponReloadComplete = TEXT("Weapon.ReloadComplete");
} // namespace LTSubtitleKeys

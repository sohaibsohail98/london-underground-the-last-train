#include "Train/LTTrain.h"

#include "Audio/LTSubtitleSubsystem.h"
#include "Components/BoxComponent.h"
#include "Core/LTGameMode.h"
#include "Core/LTGameState.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "LastTrain.h"
#include "Rounds/LTRoundManager.h"
#include "Rounds/LTStationHeat.h"

namespace
{
	/** Log-friendly name for a train phase. */
	const TCHAR* PhaseName(const ELTTrainPhase InPhase)
	{
		switch (InPhase)
		{
		case ELTTrainPhase::Away:
			return TEXT("Away");
		case ELTTrainPhase::Approaching:
			return TEXT("Approaching");
		case ELTTrainPhase::Dwelling:
			return TEXT("Dwelling");
		case ELTTrainPhase::Departing:
			return TEXT("Departing");
		}

		return TEXT("Unknown");
	}
} // namespace

ALTTrain::ALTTrain()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Overlap only. The phase and the door flag gate boarding, so the volume needs
	// no channel of its own, it only scopes where the aperture is.
	BoardingVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BoardingVolume"));
	BoardingVolume->SetupAttachment(Root);
	BoardingVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoardingVolume->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void ALTTrain::BeginPlay()
{
	Super::BeginPlay();

	// The first arrival has no preceding departure, so the opening Away phase is
	// sized to stop the train at load plus FirstTrainStopSeconds.
	const float OpeningAway = FMath::Max(0.f, FirstTrainStopSeconds - ArrivalSlideSeconds);

	Phase = ELTTrainPhase::Away;
	PhaseTimer = OpeningAway;
	bInboundAnnouncementFired = false;
	bDoorsOpen = false;
	bDepartureHeatApplied = false;
}

float ALTTrain::GetSecondsUntilArrival() const
{
	switch (Phase)
	{
	case ELTTrainPhase::Away:
		return FMath::Max(0.f, PhaseTimer) + ArrivalSlideSeconds;
	case ELTTrainPhase::Approaching:
		return FMath::Max(0.f, PhaseTimer);
	default:
		return 0.f;
	}
}

float ALTTrain::GetSecondsUntilDeparture() const
{
	return Phase == ELTTrainPhase::Dwelling ? FMath::Max(0.f, PhaseTimer) : 0.f;
}

bool ALTTrain::IsRunLive() const
{
	const ALTGameState* State = GetWorld() ? GetWorld()->GetGameState<ALTGameState>() : nullptr;
	if (!State)
	{
		// A test level with no LastTrain game state still runs the train.
		return true;
	}

	const ELTRunState RunState = State->GetRunState();
	return RunState != ELTRunState::Dead && RunState != ELTRunState::Boarded;
}

ULTStationHeat* ALTTrain::FindStationHeat() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ALTRoundManager> It(World); It; ++It)
	{
		if (ULTStationHeat* Heat = It->FindComponentByClass<ULTStationHeat>())
		{
			return Heat;
		}
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (ULTStationHeat* Heat = It->FindComponentByClass<ULTStationHeat>())
		{
			return Heat;
		}
	}

	return nullptr;
}

void ALTTrain::EnterPhase(const ELTTrainPhase NewPhase, const float NewPhaseTimer)
{
	const ELTTrainPhase OldPhase = Phase;

	Phase = NewPhase;
	PhaseTimer = NewPhaseTimer;

	LT_LOG(Log, TEXT("Train phase %s -> %s"), PhaseName(OldPhase), PhaseName(NewPhase));

	OnTrainPhaseChanged.Broadcast(NewPhase, OldPhase);
}

void ALTTrain::PlayTrainSound(USoundBase* Sound) const
{
	// Optional, like every other presentation hook on this actor: the train runs
	// its cycle silently until a Blueprint assigns the sounds.
	if (!Sound)
	{
		return;
	}

	// Positioned at the train actor, which is the platform edge. A sound that has
	// to come from one specific door leaf is Blueprint work on the matching hook,
	// not something this state machine can know about.
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
}

void ALTTrain::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// A boarded or dead run has no more trains. Freeze the whole state machine.
	if (!IsRunLive())
	{
		return;
	}

	PhaseTimer -= DeltaSeconds;

	switch (Phase)
	{
	case ELTTrainPhase::Away:
		TickAway();
		break;
	case ELTTrainPhase::Approaching:
		TickApproaching();
		break;
	case ELTTrainPhase::Dwelling:
		TickDwelling();
		break;
	case ELTTrainPhase::Departing:
		TickDeparting();
		break;
	}
}

void ALTTrain::TickAway()
{
	// The lead is measured to the stop, so the slide length counts towards it.
	const float SecondsUntilStop = PhaseTimer + ArrivalSlideSeconds;
	if (!bInboundAnnouncementFired && SecondsUntilStop <= InboundAnnouncementLeadSeconds)
	{
		bInboundAnnouncementFired = true;
		OnInboundAnnouncement();

		// The three announcement hooks carry no sound property, by design: their
		// wording is original work and a later authoring task. They are still the
		// most important thing on this actor to caption, because they are the only
		// speech in the game and speech is what a subtitle is chiefly for. The
		// fallback text below is a placeholder in the project's voice, replaced by
		// the table row once the announcements themselves are written.
		ULTSubtitleSubsystem::ShowSubtitleForWorld(
			GetWorld(), LTSubtitleKeys::TrainInboundAnnouncement, ELTSubtitleCategory::Announcement,
			NSLOCTEXT("LastTrain", "SubtitleTrainInbound", "Platform announcement: the next train is approaching."));
	}

	if (PhaseTimer <= 0.f)
	{
		++ArrivalsStarted;
		if (ArrivalsStarted > 3)
		{
			bFullArrivalPresentation = false;
		}

		EnterPhase(ELTTrainPhase::Approaching, ArrivalSlideSeconds);
		OnArrivalStarted();
		PlayTrainSound(ArrivalSound);
		ULTSubtitleSubsystem::ShowSubtitleForWorld(
			GetWorld(), LTSubtitleKeys::TrainArrival, ELTSubtitleCategory::Train,
			NSLOCTEXT("LastTrain", "SubtitleTrainArrival", "[The train rumbles into the platform]"));
	}
}

void ALTTrain::TickApproaching()
{
	if (PhaseTimer <= 0.f)
	{
		// Stopped. The doors stay shut until DoorOpenDelaySeconds into the dwell.
		EnterPhase(ELTTrainPhase::Dwelling, DwellDuration);
		OnArrivalComplete();

		ULTSubtitleSubsystem::ShowSubtitleForWorld(
			GetWorld(), LTSubtitleKeys::TrainArrivalAnnouncement, ELTSubtitleCategory::Announcement,
			NSLOCTEXT("LastTrain", "SubtitleTrainArrived", "Platform announcement: this train is ready to board."));
	}
}

void ALTTrain::TickDwelling()
{
	const float ElapsedDwell = DwellDuration - PhaseTimer;

	if (!bDoorsOpen && ElapsedDwell >= DoorOpenDelaySeconds && PhaseTimer > DoorCloseLeadSeconds)
	{
		bDoorsOpen = true;
		OnDoorsOpen();
		PlayTrainSound(DoorOpenSound);
		ULTSubtitleSubsystem::ShowSubtitleForWorld(
			GetWorld(), LTSubtitleKeys::TrainDoorsOpen, ELTSubtitleCategory::Train,
			NSLOCTEXT("LastTrain", "SubtitleTrainDoorsOpen", "[Carriage doors slide open]"));
	}
	else if (bDoorsOpen && PhaseTimer <= DoorCloseLeadSeconds)
	{
		bDoorsOpen = false;
		OnDoorsClose();
		PlayTrainSound(DoorCloseSound);
		OnDepartureAnnouncement();

		// One caption for this moment, not two. DoorCloseSound and the departure
		// announcement fire on the same frame, and the announcement says what the
		// door close says and more, so the closing doors are folded into it rather
		// than flashing a caption the announcement would immediately replace.
		ULTSubtitleSubsystem::ShowSubtitleForWorld(
			GetWorld(), LTSubtitleKeys::TrainDepartureAnnouncement, ELTSubtitleCategory::Announcement,
			NSLOCTEXT("LastTrain", "SubtitleTrainDoorsClosing", "Platform announcement: the doors are closing."));
	}

	if (PhaseTimer <= 0.f)
	{
		EnterPhase(ELTTrainPhase::Departing, DepartureSlideSeconds);
		OnDepartureStarted();
		PlayTrainSound(DepartureSound);

		// Beside DepartureSound on branch claude/phase-g1-audio-prompt.
		ULTSubtitleSubsystem::ShowSubtitleForWorld(
			GetWorld(), LTSubtitleKeys::TrainDeparture, ELTSubtitleCategory::Train,
			NSLOCTEXT("LastTrain", "SubtitleTrainDeparture", "[The train pulls away into the tunnel]"));

		if (!bDepartureHeatApplied)
		{
			bDepartureHeatApplied = true;

			// IsRunLive gated the tick already, so reaching here means the player
			// is still in the station and let the train go.
			if (ULTStationHeat* Heat = FindStationHeat())
			{
				Heat->IncrementHeat();
				LT_LOG(Log, TEXT("Train departed, player did not board. Heat now %d"), Heat->GetHeat());
			}
			else
			{
				LT_LOG(Warning, TEXT("Train departed with no station heat component in the level. Heat not raised."));
			}

			OnTrainDeparted_NotBoarded();
		}
	}
}

void ALTTrain::TickDeparting()
{
	if (PhaseTimer <= 0.f)
	{
		EnterPhase(ELTTrainPhase::Away, TrainInterval);
		OnTrainAway();

		bInboundAnnouncementFired = false;
		bDepartureHeatApplied = false;
	}
}

bool ALTTrain::TryBoard(AActor* Boarder)
{
	if (Phase != ELTTrainPhase::Dwelling || !bDoorsOpen)
	{
		return false;
	}

	if (!IsRunLive())
	{
		return false;
	}

	ALTGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALTGameMode>() : nullptr;
	if (!GameMode)
	{
		LT_LOG(Warning, TEXT("TryBoard with no ALTGameMode, cannot board"));
		return false;
	}

	OnPlayerBoarded();
	GameMode->NotifyPlayerBoarded(Boarder);
	return true;
}

bool ALTTrain::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Interactor)
	{
		return false;
	}

	return Phase == ELTTrainPhase::Dwelling && bDoorsOpen && IsRunLive();
}

FText ALTTrain::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	(void)Interactor;
	return NSLOCTEXT("LastTrain", "TrainBoardPrompt", "Board train");
}

void ALTTrain::Interact_Implementation(AActor* Interactor)
{
	TryBoard(Interactor);
}

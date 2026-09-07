#include "Train/LTDepartureBoard.h"

#include "EngineUtils.h"
#include "LastTrain.h"

ALTDepartureBoard::ALTDepartureBoard()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ALTDepartureBoard::BeginPlay()
{
	Super::BeginPlay();

	Train = TrainOverride;

	if (!Train)
	{
		if (const UWorld* World = GetWorld())
		{
			TActorIterator<ALTTrain> It(World);
			Train = It ? *It : nullptr;
		}
	}

	if (!Train)
	{
		LT_LOG(Warning, TEXT("Departure board found no train in the level."));
		return;
	}

	Train->OnTrainPhaseChanged.AddDynamic(this, &ALTDepartureBoard::HandleTrainPhaseChanged);

	DisplayPhase = Train->GetPhase();
	Refresh();
}

void ALTDepartureBoard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Drop the delegate and the reference before teardown, so a PIE stop does not
	// leave the board holding a train the editor is collecting.
	if (Train)
	{
		Train->OnTrainPhaseChanged.RemoveAll(this);
		Train = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ALTDepartureBoard::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Refresh();
}

bool ALTDepartureBoard::IsBoardingOpen() const
{
	return Train && Train->GetPhase() == ELTTrainPhase::Dwelling && Train->AreDoorsOpen();
}

void ALTDepartureBoard::HandleTrainPhaseChanged(const ELTTrainPhase NewPhase, const ELTTrainPhase OldPhase)
{
	DisplayPhase = NewPhase;

	OnPhaseChanged(NewPhase, OldPhase);

	// So the number and the wording change on the same frame.
	Refresh();
}

void ALTDepartureBoard::Refresh()
{
	if (!Train)
	{
		return;
	}

	// Read the live phase rather than trusting the cached one, so a missed
	// delegate cannot leave the number and the wording disagreeing.
	DisplayPhase = Train->GetPhase();

	float Raw = 0.f;
	switch (DisplayPhase)
	{
	case ELTTrainPhase::Away:
	case ELTTrainPhase::Approaching:
		Raw = Train->GetSecondsUntilArrival();
		break;
	case ELTTrainPhase::Dwelling:
		Raw = Train->GetSecondsUntilDeparture();
		break;
	case ELTTrainPhase::Departing:
		Raw = 0.f;
		break;
	}

	// Ceil, so the sign reads 1 until the second is truly spent rather than
	// sitting on 0 for the last of it.
	const int32 Whole = FMath::Max(0, FMath::CeilToInt(Raw));

	if (Whole != DisplaySeconds)
	{
		DisplaySeconds = Whole;
		OnCountdownChanged(DisplaySeconds, DisplayPhase);
	}
}

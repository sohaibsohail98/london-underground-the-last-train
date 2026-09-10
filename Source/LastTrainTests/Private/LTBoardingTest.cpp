#include "LTBoardingTest.h"

#include "Core/LTGameMode.h"
#include "Core/LTGameState.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/LTInteractableInterface.h"
#include "Interaction/LTInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "Train/LTTrain.h"

#if WITH_EDITOR && WITH_AUTOMATION_TESTS

// Drives a full boarding round in PIE against the real Canary Wharf greybox:
// waits for the train to reach Dwelling, confirms ULTInteractionComponent
// resolves it as the current interactable (this is the regression coverage for
// the SweepMultiByChannel fix in af0a152), calls TryInteract through the same
// ILTInteractableInterface path a keypress would, then confirms the run state
// flips to Boarded. No input device is simulated: this is a functional test of
// the interaction and boarding logic, not of Enhanced Input bindings.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLTBoardingTest, "LastTrain.Boarding.TrainInteractableAndBoard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace LTBoardingTestPrivate
{
const TCHAR* GreyboxMapPackagePath = TEXT("/Game/LastTrain/Maps/L_CanaryWharf_Greybox");
constexpr float StandOffsetFromTrainUU = 150.f;
constexpr float MaxDwellWaitSeconds = 40.f;
constexpr float MaxBoardWaitSeconds = 5.f;

ALTTrain* FindTrain(const UWorld* World)
{
	TActorIterator<ALTTrain> It(World);
	return It ? *It : nullptr;
}

/** The running PIE world, or null if no PIE session is active. */
UWorld* GetPIEWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			return Context.World();
		}
	}
	return nullptr;
}
} // namespace LTBoardingTestPrivate

/** Waits for the train actor to enter Dwelling, places the player pawn at a
	standing spot in front of the doors, then checks the interaction and boards.
	Hand written rather than DEFINE_LATENT_AUTOMATION_COMMAND so the wait deadline
	and settle timer are real member state, not function statics that would leak
	into a second run of the same test within one process. */
class FLTWaitForDwellingAndBoardCommand : public IAutomationLatentCommand
{
public:
	explicit FLTWaitForDwellingAndBoardCommand(FAutomationTestBase* InTest) : Test(InTest)
	{
	}

	virtual bool Update() override
	{
		using namespace LTBoardingTestPrivate;

		UWorld* World = GetPIEWorld();
		if (!World)
		{
			Test->AddError(TEXT("No PIE world running for the boarding test."));
			return true;
		}

		if (StartTime < 0.0)
		{
			StartTime = FPlatformTime::Seconds();
		}

		ALTTrain* Train = FindTrain(World);
		if (!Train)
		{
			if (FPlatformTime::Seconds() - StartTime > MaxDwellWaitSeconds)
			{
				Test->AddError(TEXT("No ALTTrain found in the level within the wait window."));
				return true;
			}
			return false;
		}

		if (Train->GetPhase() != ELTTrainPhase::Dwelling)
		{
			if (FPlatformTime::Seconds() - StartTime > MaxDwellWaitSeconds)
			{
				Test->AddError(TEXT("Train never reached Dwelling within the wait window."));
				return true;
			}
			return false;
		}

		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			Test->AddError(TEXT("No player pawn possessed in PIE."));
			return true;
		}

		if (!bPlacedPawn)
		{
			// Stand in front of the train, facing it, offset toward the platform
			// side, at the pawn's own height so a capsule half height difference
			// cannot wedge it. The train's forward vector runs along its length
			// (direction of travel); the doors face sideways, along its right
			// vector, toward whichever side the platform sits on. The origin sits
			// at the train's Y centre with the platform on the low-Y side (west of
			// the trackbed), so step out along -Right, which for a yaw 0 train
			// (forward +X, right +Y) is -Y, matching the platform.
			const FVector TrainLocation = Train->GetActorLocation();
			const FVector TrainRight = Train->GetActorRightVector();
			FVector StandLocation = TrainLocation - TrainRight.GetSafeNormal2D() * StandOffsetFromTrainUU;
			StandLocation.Z = Pawn->GetActorLocation().Z;
			const FRotator FacingRotation = (TrainLocation - StandLocation).GetSafeNormal2D().Rotation();
			Pawn->SetActorLocation(StandLocation, false, nullptr, ETeleportType::TeleportPhysics);
			Pawn->SetActorRotation(FacingRotation);
			// ULTInteractionComponent traces from the possessing controller's
			// ControlRotation (via GetActorEyesViewPoint), not the pawn's actor
			// rotation, so the controller has to be turned to face the train too.
			if (PC)
			{
				PC->SetControlRotation(FacingRotation);
			}
			bPlacedPawn = true;
			SettleStart = FPlatformTime::Seconds();

			Test->AddInfo(FString::Printf(
				TEXT("Train at %s (right %s). Placed pawn at %s, actual location after move %s, distance to train %.1f."),
				*TrainLocation.ToString(), *TrainRight.ToString(), *StandLocation.ToString(),
				*Pawn->GetActorLocation().ToString(), FVector::Dist(Pawn->GetActorLocation(), TrainLocation)));
			return false;
		}

		// Dwelling starts with the doors still shut: ALTTrain::CanInteract
		// requires bDoorsOpen, which only flips DoorOpenDelaySeconds into the
		// dwell. Wait for the doors themselves rather than a fixed settle
		// timer, so this does not depend on guessing that delay's value.
		if (!Train->AreDoorsOpen())
		{
			if (FPlatformTime::Seconds() - StartTime > MaxDwellWaitSeconds)
			{
				Test->AddError(TEXT("Train reached Dwelling but its doors never opened within the wait window."));
				return true;
			}
			return false;
		}

		// One tick has to run with the pawn in its new spot before the
		// component's own sweep has picked it up. Wait a short beat rather than
		// assert on this frame.
		if (FPlatformTime::Seconds() - SettleStart < 0.5)
		{
			return false;
		}

		ULTInteractionComponent* Interaction = Pawn->FindComponentByClass<ULTInteractionComponent>();
		if (!Interaction)
		{
			Test->AddError(TEXT("Player pawn has no ULTInteractionComponent."));
			return true;
		}

		AActor* CurrentInteractable = Interaction->GetCurrentInteractable();
		Test->AddInfo(FString::Printf(TEXT("At assertion time: pawn at %s, rotation %s, resolved interactable %s."),
			*Pawn->GetActorLocation().ToString(), *Pawn->GetActorRotation().ToString(),
			CurrentInteractable ? *CurrentInteractable->GetName() : TEXT("null")));
		Test->TestEqual(TEXT("ULTInteractionComponent resolves the train as the current interactable"),
			CurrentInteractable, static_cast<AActor*>(Train));

		if (CurrentInteractable != Train)
		{
			// The occluder fault this test guards against: stop here rather than
			// call TryInteract against a null target.
			return true;
		}

		Interaction->TryInteract();

		ALTGameState* GameState = World->GetGameState<ALTGameState>();
		Test->TestNotNull(TEXT("Game state exists"), GameState);
		if (GameState)
		{
			Test->TestEqual(
				TEXT("Boarding flips the run state to Boarded"), GameState->GetRunState(), ELTRunState::Boarded);
		}

		return true;
	}

private:
	FAutomationTestBase* Test;
	double StartTime = -1.0;
	double SettleStart = -1.0;
	bool bPlacedPawn = false;
};

bool FLTBoardingTest::RunTest(const FString& Parameters)
{
	using namespace LTBoardingTestPrivate;

	// PIE without a CommonGameViewportClient logs this every session; it is a
	// known, harmless gap (the HUD has no CommonUI screens routed through it
	// yet), not something this boarding test is checking for. Automation fails
	// a test on any logged error by default, so without this the test would
	// fail on an unrelated engine warning even when boarding itself works.
	// Occurrences -1 means any number of matches (including none) are fine;
	// the exact count varies with how many CommonUI subsystems check the
	// viewport client on a given run.
	AddExpectedError(
		TEXT("Using CommonUI without a CommonGameViewportClient derived game viewport client"),
		EAutomationExpectedErrorFlags::Contains, -1);

	// Load the greybox map, start a real PIE session on it (this is what possesses
	// the player pawn and runs BeginPlay, exactly what a manual playtest does), then
	// wait for the train to dwell and attempt to board.
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(GreyboxMapPackagePath));

	FRequestPlaySessionParams PlaySessionParams;
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIEForAutomationCommand(PlaySessionParams));

	ADD_LATENT_AUTOMATION_COMMAND(FLTWaitForDwellingAndBoardCommand(this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_EDITOR && WITH_AUTOMATION_TESTS

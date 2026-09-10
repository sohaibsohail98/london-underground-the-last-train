#include "LTPlaytestToolset.h"

#include "Editor.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "UObject/UnrealType.h"

namespace LTPlaytestToolsetPrivate
{
/** The running PIE world, or null if no PIE session is active. Mirrors the
	same lookup a test or another toolset would use: PIE runs as its own
	FWorldContext entry alongside the editor world, so GEngine's world list is
	the only reliable place to find it. */
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

/** The possessed pawn of PIE's first local player controller, or null. */
APawn* GetPossessedPawn(UWorld* World, APlayerController*& OutController)
{
	OutController = nullptr;
	if (!World)
	{
		return nullptr;
	}
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			OutController = PC;
			return PC->GetPawn();
		}
	}
	return nullptr;
}

/** Finds a UInputAction* property by name on the pawn's class, walking the
	inheritance chain. Returns null and fills OutError if the property does
	not exist or is not a UInputAction pointer. */
UInputAction* FindActionProperty(const APawn* Pawn, const FString& PropertyName, FString& OutError)
{
	if (!Pawn)
	{
		OutError = TEXT("No pawn possessed in the current PIE session.");
		return nullptr;
	}

	const FProperty* Property = Pawn->GetClass()->FindPropertyByName(FName(*PropertyName));
	if (!Property)
	{
		OutError = FString::Printf(TEXT("Pawn class %s has no property named '%s'."),
			*Pawn->GetClass()->GetName(), *PropertyName);
		return nullptr;
	}

	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
	if (!ObjectProperty || !ObjectProperty->PropertyClass->IsChildOf(UInputAction::StaticClass()))
	{
		OutError = FString::Printf(
			TEXT("Property '%s' on %s is not a UInputAction pointer."), *PropertyName, *Pawn->GetClass()->GetName());
		return nullptr;
	}

	UObject* Value = ObjectProperty->GetObjectPropertyValue_InContainer(Pawn);
	UInputAction* Action = Cast<UInputAction>(Value);
	if (!Action)
	{
		OutError = FString::Printf(
			TEXT("Property '%s' on %s is currently null. The owning Blueprint has not assigned an input action asset "
				 "to it."),
			*PropertyName, *Pawn->GetClass()->GetName());
		return nullptr;
	}

	return Action;
}

/** The EnhancedPlayerInput driving Enhanced Input for this controller's local
	player, or null with OutError filled in. */
UEnhancedPlayerInput* GetEnhancedPlayerInput(APlayerController* PC, FString& OutError)
{
	if (!PC)
	{
		OutError = TEXT("No player controller in the current PIE session.");
		return nullptr;
	}
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		OutError = TEXT("The player controller has no local player (not a listen host client?).");
		return nullptr;
	}
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		OutError = TEXT("No UEnhancedInputLocalPlayerSubsystem on the local player. Is Enhanced Input enabled?");
		return nullptr;
	}
	UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	if (!PlayerInput)
	{
		OutError = TEXT("UEnhancedInputLocalPlayerSubsystem has no player input yet.");
		return nullptr;
	}
	return PlayerInput;
}

/** Injects Value for the named action on the current PIE session's possessed
	pawn. Shared by InjectButtonAction and InjectAxis2DAction. */
FString InjectValue(const FString& ActionPropertyName, const FInputActionValue& Value)
{
	UWorld* World = GetPIEWorld();
	if (!World)
	{
		return TEXT("No PIE session is running. Start PIE first.");
	}

	APlayerController* PC = nullptr;
	APawn* Pawn = GetPossessedPawn(World, PC);

	FString Error;
	UInputAction* Action = FindActionProperty(Pawn, ActionPropertyName, Error);
	if (!Action)
	{
		return Error;
	}

	UEnhancedPlayerInput* PlayerInput = GetEnhancedPlayerInput(PC, Error);
	if (!PlayerInput)
	{
		return Error;
	}

	PlayerInput->InjectInputForAction(Action, Value);
	return FString();
}
} // namespace LTPlaytestToolsetPrivate

FString ULTPlaytestToolset::InjectButtonAction(const FString& ActionPropertyName, bool bPressed)
{
	return LTPlaytestToolsetPrivate::InjectValue(ActionPropertyName, FInputActionValue(bPressed));
}

FString ULTPlaytestToolset::InjectAxis2DAction(const FString& ActionPropertyName, float X, float Y)
{
	return LTPlaytestToolsetPrivate::InjectValue(ActionPropertyName, FInputActionValue(FVector2D(X, Y)));
}

FString ULTPlaytestToolset::ListInjectableActions()
{
	using namespace LTPlaytestToolsetPrivate;

	UWorld* World = GetPIEWorld();
	if (!World)
	{
		return TEXT("[]");
	}

	APlayerController* PC = nullptr;
	APawn* Pawn = GetPossessedPawn(World, PC);
	if (!Pawn)
	{
		return TEXT("[]");
	}

	TArray<FString> Names;
	for (TFieldIterator<FObjectProperty> It(Pawn->GetClass()); It; ++It)
	{
		const FObjectProperty* ObjectProperty = *It;
		if (ObjectProperty->PropertyClass->IsChildOf(UInputAction::StaticClass()))
		{
			Names.Add(ObjectProperty->GetName());
		}
	}

	FString Result = TEXT("[");
	for (int32 Index = 0; Index < Names.Num(); ++Index)
	{
		Result += FString::Printf(TEXT("\"%s\""), *Names[Index]);
		if (Index + 1 < Names.Num())
		{
			Result += TEXT(", ");
		}
	}
	Result += TEXT("]");
	return Result;
}

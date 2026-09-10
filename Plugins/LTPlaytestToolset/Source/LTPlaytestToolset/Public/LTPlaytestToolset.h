#pragma once

#include "ToolsetRegistry/ToolsetDefinition.h"

#include "LTPlaytestToolset.generated.h"

/**
 * Injects Enhanced Input actions into the possessed pawn of a running Play In
 * Editor session. Fills the gap left when the project's third party playtest
 * bridge (NeoStack AI) lost its licence: there is no way through Epic's own
 * built in ModelContextProtocol toolsets to simulate a keypress or an axis
 * move reaching a live PIE session, only to start or stop the session, query
 * state and run automation tests.
 *
 * Resolves the action by property name on the possessed pawn rather than by
 * asset path, so it works against whichever Blueprint assigned the input
 * actions (LTPlayerCharacter's Enhanced Input properties are
 * EditDefaultsOnly and null until a Blueprint sets them) without this plugin
 * depending on the LastTrain module at all.
 *
 * Typical workflow:
 *   1. Start PIE (EditorToolset.EditorAppToolset.StartPIE).
 *   2. InjectAxis2DAction("MoveAction", 0, 1) to walk forward, repeated across
 *      ticks for as long as the move should hold, then (0, 0) to stop.
 *   3. InjectButtonAction("InteractAction", true) then a later
 *      InjectButtonAction("InteractAction", false) to press and release,
 *      mirroring a real keypress's Enhanced Input trigger lifecycle.
 */
UCLASS(BlueprintType, MinimalAPI)
class ULTPlaytestToolset : public UToolsetDefinition
{
	GENERATED_BODY()

public:
	/** Injects a boolean (press or release) value for a named Enhanced Input
	 * action property on the current PIE session's possessed pawn.
	 * @param ActionPropertyName  The pawn's UPROPERTY name holding the
	 *   UInputAction pointer, for example "InteractAction" or "FireAction".
	 * @param bPressed  True for a press, false for a release.
	 * @return  Empty string on success, otherwise a human readable error. */
	UFUNCTION(meta = (AICallable), Category = "LTPlaytestToolset")
	static LTPLAYTESTTOOLSET_API FString InjectButtonAction(const FString& ActionPropertyName, bool bPressed);

	/** Injects a 2D axis value (movement or look) for a named Enhanced Input
	 * action property on the current PIE session's possessed pawn.
	 * @param ActionPropertyName  The pawn's UPROPERTY name holding the
	 *   UInputAction pointer, for example "MoveAction" or "LookAction".
	 * @param X  The axis X component, typically -1 to 1.
	 * @param Y  The axis Y component, typically -1 to 1.
	 * @return  Empty string on success, otherwise a human readable error. */
	UFUNCTION(meta = (AICallable), Category = "LTPlaytestToolset")
	static LTPLAYTESTTOOLSET_API FString InjectAxis2DAction(const FString& ActionPropertyName, float X, float Y);

	/** Lists the pawn UPROPERTY names this toolset can find a UInputAction on,
	 * so a client can discover the exact ActionPropertyName strings to pass
	 * without reading the project's C++ source.
	 * @return  A JSON array of property names, for example
	 *   ["MoveAction", "LookAction", "InteractAction"]. */
	UFUNCTION(meta = (AICallable), Category = "LTPlaytestToolset")
	static LTPLAYTESTTOOLSET_API FString ListInjectableActions();
};

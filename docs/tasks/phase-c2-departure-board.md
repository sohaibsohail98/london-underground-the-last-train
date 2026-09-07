# Phase C2 - the departure board actor

**Engine:** Unreal Engine 5.8, macOS. Xcode is on an external drive at
`/Volumes/DriveSohaib/Applications/Xcode.app`, which must be mounted to compile.
Check with `xcode-select -p`. If `/Volumes/DriveSohaib` is not mounted, STOP.

**Build command** (from the repo root, the only supported compile):
```
"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"
```
`-Werror` is on: zero warnings. There is no CI compile, the engine is not in CI.
Compile locally after the change.

**clang-format 20** governs `.h`/`.cpp` (`.clang-format` at the repo root:
`UseTab: Always`, `TabWidth: 4`, `ColumnLimit: 120`, Allman braces). If a
`clang-format` binary is not on PATH, install one into a venv:
`python3 -m venv /tmp/cf-venv && /tmp/cf-venv/bin/pip install clang-format==20.1.0`
then run `/tmp/cf-venv/bin/clang-format -i` on the files you touch.

**Model:** Opus. C++ against the existing convention.

**Prerequisite:** Phase C1 landed. `Source/LastTrain/Public/Train/LTTrain.h`
exists with `ALTTrain` exposing, as `BlueprintPure`:
- `ELTTrainPhase GetPhase() const`
- `bool AreDoorsOpen() const`
- `float GetSecondsUntilArrival() const`  (0 while stopped or inbound past the stop)
- `float GetSecondsUntilDeparture() const`  (0 unless Dwelling)
and the multicast `FOnTrainPhaseChanged OnTrainPhaseChanged` (NewPhase, OldPhase).
`ELTTrainPhase` is `{ Away, Approaching, Dwelling, Departing }`.

## The problem

`docs/art-direction.md` and `docs/brief-v2.md` put the train countdown on a
platform departure board: a diegetic sign showing how long until the next train,
and the current train state (inbound, at platform, departing). `ALTTrain` exposes
the numbers to read but nothing reads them. There is no board actor.

This task builds `ALTDepartureBoard`: an `AActor` that finds the station's
`ALTTrain`, polls its getters, and drives `BlueprintImplementableEvent` hooks a
Blueprint uses to update a text render or a widget component. No UMG in C++, no
fonts, no materials. The visual is the Blueprint's job. This is the data pump.

## Scope

**In scope**
- `ALTDepartureBoard`, an `AActor`, new folder `Source/LastTrain/*/Train/` already
  exists from C1, put the files beside `LTTrain`.
- On `BeginPlay`, find the `ALTTrain` in the level (first one via
  `TActorIterator`). An `EditInstanceOnly` `TObjectPtr<ALTTrain>` override so a
  level with two boards and one train, or one board and a specific train, can be
  wired by hand; fall back to the iterator when it is null.
- A poll on `Tick` (or a timer, your call, Tick is fine and simpler) that reads
  the train getters and fires an update hook when the displayed value changes by
  more than a threshold, so the Blueprint is not asked to rebuild a texture every
  frame.
- `BlueprintImplementableEvent` hooks:
  - `OnCountdownChanged(int32 WholeSeconds, ELTTrainPhase Phase)` - the number to
    show, already floored to a whole second, plus the phase so the Blueprint can
    pick "inbound in 0:45" vs "at platform 0:12" vs "departing" wording.
  - `OnPhaseChanged(ELTTrainPhase NewPhase, ELTTrainPhase OldPhase)` - forwarded
    from the train's delegate, for a state light or a colour change.
- `BlueprintPure` passthroughs so a widget can also pull directly:
  `int32 GetDisplaySeconds() const`, `ELTTrainPhase GetDisplayPhase() const`,
  `bool IsBoardingOpen() const` (Phase == Dwelling and AreDoorsOpen).
- A `bool bHasTrain` the Blueprint can read to show a blank or "no service" state
  when no train was found.

**Out of scope**
- The widget, the text render component, the font, the flip-dot or LED look.
  Blueprint and the art pass.
- Multiple platforms or multiple destinations on one board.
- Any change to `ALTTrain`. If this task seems to need one, STOP and say so.

## The change

New files:
- `Source/LastTrain/Public/Train/LTDepartureBoard.h`
- `Source/LastTrain/Private/Train/LTDepartureBoard.cpp`

### Header shape

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Train/LTTrain.h"   // for ELTTrainPhase in the event signatures
#include "LTDepartureBoard.generated.h"

class ALTTrain;

/** Reads the station train's countdown and phase and drives presentation hooks a
	Blueprint uses to update a sign. Finds the train on BeginPlay; the visual is
	the Blueprint's job. */
UCLASS()
class LASTTRAIN_API ALTDepartureBoard : public AActor
{
	GENERATED_BODY()

public:
	ALTDepartureBoard();

	virtual void Tick(float DeltaSeconds) override;

	/** Which whole second to show now: seconds to arrival while Away or
		Approaching, seconds to departure while Dwelling, 0 while Departing. */
	UFUNCTION(BlueprintPure, Category = "Departure board")
	int32 GetDisplaySeconds() const { return DisplaySeconds; }

	UFUNCTION(BlueprintPure, Category = "Departure board")
	ELTTrainPhase GetDisplayPhase() const { return DisplayPhase; }

	UFUNCTION(BlueprintPure, Category = "Departure board")
	bool IsBoardingOpen() const;

	UFUNCTION(BlueprintPure, Category = "Departure board")
	bool HasTrain() const { return Train != nullptr; }

	/** Optional. Leave null to bind the first ALTTrain found in the level. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Departure board")
	TObjectPtr<ALTTrain> TrainOverride;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Departure board")
	void OnCountdownChanged(int32 WholeSeconds, ELTTrainPhase Phase);

	UFUNCTION(BlueprintImplementableEvent, Category = "Departure board")
	void OnPhaseChanged(ELTTrainPhase NewPhase, ELTTrainPhase OldPhase);

private:
	UFUNCTION()
	void HandleTrainPhaseChanged(ELTTrainPhase NewPhase, ELTTrainPhase OldPhase);

	/** Reads the train, recomputes DisplaySeconds and fires OnCountdownChanged if
		the whole-second value moved. */
	void Refresh();

	UPROPERTY() TObjectPtr<ALTTrain> Train;

	int32 DisplaySeconds = 0;
	ELTTrainPhase DisplayPhase = ELTTrainPhase::Away;
};
```

### Behaviour

- Constructor: `PrimaryActorTick.bCanEverTick = true`. Create a `USceneComponent`
  root and `SetRootComponent` (match `ALTWallBuy` / `ALTTrain`), so the board is
  placeable and movable.
- `BeginPlay`: `Train = TrainOverride ? TrainOverride : <first ALTTrain via
  TActorIterator>`. If found, bind `HandleTrainPhaseChanged` to
  `Train->OnTrainPhaseChanged` with `AddDynamic`, seed `DisplayPhase` from
  `Train->GetPhase()`, call `Refresh()`. If not found,
  `LT_LOG(Warning, TEXT("Departure board found no train in the level."))`.
- `Refresh()`:
  - If no `Train`, return.
  - Compute the raw float: `Phase == Dwelling` -> `GetSecondsUntilDeparture()`;
    `Phase == Away || Approaching` -> `GetSecondsUntilArrival()`; `Departing` ->
    `0.f`.
  - `const int32 Whole = FMath::Max(0, FMath::CeilToInt(Raw));` (ceil so "1"
    shows until it truly hits zero, not "0" for the last second).
  - If `Whole != DisplaySeconds`, set `DisplaySeconds = Whole` and call
    `OnCountdownChanged(DisplaySeconds, DisplayPhase)`.
- `Tick`: just call `Refresh()`. It is cheap (a few float reads and an int
  compare) and only fires the hook on a whole-second change.
- `HandleTrainPhaseChanged`: set `DisplayPhase = NewPhase`, call
  `OnPhaseChanged(NewPhase, OldPhase)`, then `Refresh()` so the number and the
  phase update on the same frame.
- `IsBoardingOpen()`: `Train && Train->GetPhase() == ELTTrainPhase::Dwelling &&
  Train->AreDoorsOpen()`.
- `EndPlay` override: if `Train`, `Train->OnTrainPhaseChanged.RemoveAll(this)`,
  `Train = nullptr`. Matches the teardown discipline in `ALTRoundManager::EndPlay`.

## Constraints

- New files only: `LTDepartureBoard.{h,cpp}` in the existing `Train/` folders.
  The single include of `Train/LTTrain.h` in the header is allowed (it needs
  `ELTTrainPhase` in the `UFUNCTION` signatures). No edit to `LTTrain.{h,cpp}` or
  anything else. If the task appears to need one, STOP and report.
- British spelling everywhere including comments and log strings. No em or en
  dashes, plain punctuation only.
- `#pragma once` first, `LTDepartureBoard.generated.h` last include.
- `LT_LOG(Verbosity, TEXT("..."))`, never `UE_LOG`. `LT_LOG` is in
  `Private/LastTrain.h`, include `"LastTrain.h"` in the cpp.
- `TObjectPtr` for every `UObject` member.
- No `TODO`/`FIXME`/`HACK`/`XXX` markers. `tools/ci/check_hygiene.py` rejects them.
- `LT` prefix on the type. Tab indent.
- Do NOT run any git commands. Leave the tree for the human to review and commit.

## Acceptance

Static (you cannot PIE):
1. Compiles clean with the batch build, zero warnings under `-Werror`.
2. `clang-format -i` run on both files, no diff after.
3. All four `tools/ci/` checkers pass:
   `python3 tools/ci/check_hygiene.py && python3 tools/ci/check_cpp_conventions.py && python3 tools/ci/check_docs.py && python3 tools/ci/check_content.py`
4. Header review: `#pragma once` first, generated.h last, all `UObject` members
   `TObjectPtr`, no `UE_LOG`, British spelling, no dashes, no markers.

Runtime (a later editor task, note it, do not do it): a `BP_DepartureBoard` from
`ALTDepartureBoard` placed in `L_GreyboxTest` near the `BP_Train`, with a text
render child driven by `OnCountdownChanged`, should show a live countdown that
matches the train's cycle and switches wording on `OnPhaseChanged`.

## On pass

Update `docs/tasks/NEXT.md` and the Phase C row in `docs/tasks/README.md`. Note
that the runtime acceptance is an editor task pending `BP_DepartureBoard`.

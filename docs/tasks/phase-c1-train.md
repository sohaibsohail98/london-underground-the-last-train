# Phase C1 - the train actor

**Engine:** Unreal Engine 5.8, macOS, external Xcode on `/Volumes/DriveSohaib`
mounted. Compile after the change with the batch build in `CLAUDE.md`.

**Model:** Opus. C++ against the existing convention, with several open design
questions carried in as decision markers. Read those before starting and get the
owner to settle them rather than guessing.

**Prerequisite:** Phase B landed. `L_GreyboxTest` plays, the round manager runs,
the interaction system (`ULTInteractionComponent`, `ILTInteractableInterface`,
`ALTWallBuy`) works. `Source/LastTrain/Public/Core/LTGameMode.h`,
`LTGameState.h` and `Source/LastTrain/Public/Rounds/LTStationHeat.h` exist as
written in the just-landed core framework pass.

## The problem

`docs/brief-v2.md` line 372: "Train timer: a train arrives every
TRAIN_INTERVAL_MS (100000) and dwells TRAIN_DWELL_MS (25000), both single
tuneable constants. Announcement fires 15 seconds before arrival and on arrival.
The train is real geometry arriving in the trackbed with headlights, door
animation and rumble. Boarding during dwell ends the round, banks points,
refills ammo and travels to a chosen adjacent station. Staying raises station
heat by 1 ... heat resets on travel. Travel is bidirectional and free."

None of that exists. There is no `ALTTrain` class. `ELTRunState` has a `Boarded`
state that nothing ever sets. `ULTStationHeat::IncrementHeat` has no caller. The
departure board that `docs/art-direction.md` and brief-v2 put the countdown on
has nothing to read a countdown from. The signature beat of the game, a train
sliding into the trackbed on a timer that you may or may not board, is unbuilt.

This task builds the timing and state machine, the presentation hooks, and the
boarding path for a **single station arena**: the train approaches, dwells,
departs, and comes back on the interval, indefinitely. It does not build travel
to another station.

## Scope

**In scope**

- `ALTTrain`, an `AActor` in a new `Source/LastTrain/*/Train/` folder.
- The arrive / dwell / depart / away timing state machine on the interval and
  dwell constants from brief-v2.
- `BlueprintImplementableEvent` presentation hooks for the arrival slide,
  headlights, doors, rumble and screech. No art, no meshes, no timelines in C++.
- The boarding trigger, live only while the doors are open during the dwell.
- What "board" does inside this arena: flip run state, stop rounds, refill ammo,
  reset heat, bank points, and call one game mode hook a travel system will
  later implement.
- Heat increment when a train departs with the player not aboard.
- `BlueprintPure` getters for a separate departure board actor to read the
  countdown from.
- A small addition to `ALTGameMode`: a `NotifyPlayerBoarded` hook.

**Out of scope, later tasks**

- Actual travel to another station: the level transition, the loading state, the
  `ULTGameInstance` travel payload, rehydrating a fresh arena. `NotifyPlayerBoarded`
  is the seam a later "Phase C travel" task implements. For now it does the
  in-arena part only (`ALTGameState::SetRunState(ELTRunState::Boarded)` and stop
  the rounds) and logs that travel is not wired.
- The departure board actor itself. This task only exposes the getters it reads.
  A later "Phase C departure board" task builds `ALTDepartureBoard`.
- The station-select UI. See the decision marker in "The boarding trigger".
- The trackbed as a kill volume, "stand clear" klaxon, player-on-the-tracks
  handling. See `docs/design/open-questions.md` 4.3 and 4.12. Not this task.
- Platform screen doors as level geometry (open-questions 4.12). This task fires
  one combined `OnDoorsOpen` / `OnDoorsClose` pair so a Blueprint can drive
  either a single car-door set or a screen-door plus car-door pair; it does not
  build the screen wall.
- Degrading the arrival spectacle after the first few cycles (open-questions
  4.13, "Multiple trains over a long run"). Note it, do not build it.
- Multiple zombie types, the special-round roster. Separate Phase C task.

## The change

Three pieces. Build them in this order and compile between each.

### 1. `ALTTrain`

New files:

- `Source/LastTrain/Public/Train/LTTrain.h`
- `Source/LastTrain/Private/Train/LTTrain.cpp`

An `AActor` that owns the train timing state machine, the presentation hooks and
the boarding path. One instance per station, placed in the level at the platform
edge with its mesh child positioned in the trackbed.

#### The timing state machine

```cpp
/** Where the train is in its cycle. Drives presentation and the boarding window. */
UENUM(BlueprintType)
enum class ELTTrainPhase : uint8
{
	/** No train. Counting down to the next arrival. */
	Away,
	/** Inbound. The arrival slide is playing. Doors shut, no boarding. */
	Approaching,
	/** Stopped at the platform. Doors open for part of this. Boarding is live. */
	Dwelling,
	/** Doors shut, pulling out. No boarding. Heat has already been applied. */
	Departing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrainPhaseChanged, ELTTrainPhase, NewPhase, ELTTrainPhase, OldPhase);
```

Tuneables, all `EditDefaultsOnly, BlueprintReadOnly`, category `Train`, so a
station can retune them per platform:

```cpp
/** Seconds from one departure to the next arrival. brief-v2 TRAIN_INTERVAL_MS is 100000. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
float TrainInterval = 100.f;

/** Seconds the train sits stopped at the platform. brief-v2 TRAIN_DWELL_MS is 25000.
    Measured stop to start, not door to door. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
float DwellDuration = 25.f;

/** Seconds the inbound slide takes. Presentation only: the state is Approaching
    for this long before the train counts as stopped. 4s is a grey box stub, a
    full train needs 10 to 12s of visible deceleration (open-questions 4.1). */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
float ArrivalSlideSeconds = 4.f;

/** Seconds the outbound slide takes. Departing for this long, then Away. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
float DepartureSlideSeconds = 4.f;

/** After the train stops, wait this long before the doors open. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
float DoorOpenDelaySeconds = 1.f;

/** Doors close this many seconds before the train starts to depart, so the
    open-door boarding window is DwellDuration - DoorOpenDelaySeconds - this. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
float DoorCloseLeadSeconds = 3.f;

/** Seconds before the train STOPS that the inbound announcement fires. brief-v2:
    "15 seconds before arrival". Fires during the Approaching slide if
    ArrivalSlideSeconds is under this, otherwise late in Away. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train")
float InboundAnnouncementLeadSeconds = 15.f;
```

State, private:

```cpp
UPROPERTY(VisibleInstanceOnly, Category = "Train")
ELTTrainPhase Phase = ELTTrainPhase::Away;

/** Counts down within the current phase. Meaning depends on Phase. */
float PhaseTimer = 0.f;

/** True once the current cycle's inbound announcement hook has fired. */
bool bInboundAnnouncementFired = false;

/** True while the doors are open this dwell. Gates the boarding trigger. */
bool bDoorsOpen = false;

/** True once IncrementHeat has run for the current departure, so a long
    Departing phase cannot double count. */
bool bDepartureHeatApplied = false;
```

`[DECISION NEEDED: open-questions 1.1 / 4.2 first-train zero - the first arrival
cannot count "departure to arrival" because there was no prior departure.
Proposed default: expose FirstTrainStopSeconds = 30, and on BeginPlay start the
first cycle so the train STOPS at level load + FirstTrainStopSeconds (inbound
slide starting FirstTrainStopSeconds - ArrivalSlideSeconds in), then TrainInterval
governs every cycle after. If the pre-round establishing beat lands first, the
beat is absorbed into that 30s, not added.]`

Public surface:

```cpp
ALTTrain();

UPROPERTY(BlueprintAssignable, Category = "Train")
FOnTrainPhaseChanged OnTrainPhaseChanged;

UFUNCTION(BlueprintPure, Category = "Train")
ELTTrainPhase GetPhase() const { return Phase; }

UFUNCTION(BlueprintPure, Category = "Train")
bool AreDoorsOpen() const { return bDoorsOpen; }

/** Seconds until the train next comes to a stop. Zero while it is stopped or
    inbound past the stop point. For the departure board. */
UFUNCTION(BlueprintPure, Category = "Train")
float GetSecondsUntilArrival() const;

/** Seconds until the train next starts to pull away. Zero unless it is Dwelling.
    For the departure board. */
UFUNCTION(BlueprintPure, Category = "Train")
float GetSecondsUntilDeparture() const;

/** The boarding trigger calls this. Returns false and does nothing if boarding
    is not currently allowed (wrong phase, doors shut, run already over). */
UFUNCTION(BlueprintCallable, Category = "Train")
bool TryBoard(AActor* Boarder);
```

`GetSecondsUntilArrival` computes from `Phase` and `PhaseTimer`: in `Away` it is
`PhaseTimer + ArrivalSlideSeconds`; in `Approaching` it is `PhaseTimer` (the
slide remaining); in `Dwelling` and `Departing` it is `0.f`. Mirror the
reasoning for `GetSecondsUntilDeparture`: `0.f` in every phase except `Dwelling`,
where it is `PhaseTimer`.

`PrimaryActorTick.bCanEverTick = true` in the constructor. Set
`bDepartureHeatApplied = false` and `bInboundAnnouncementFired = false` at the
start of each `Away` phase.

#### Tick and transitions

`Tick(float DeltaSeconds)`:

1. Decrement `PhaseTimer` by `DeltaSeconds`.
2. Per phase:
   - **Away**: if not `bInboundAnnouncementFired` and
     `PhaseTimer <= InboundAnnouncementLeadSeconds`, call
     `OnInboundAnnouncement()` and set the flag. When `PhaseTimer <= 0`, go to
     `Approaching` (`PhaseTimer = ArrivalSlideSeconds`), call `OnArrivalStarted()`.
     Edge case: if `ArrivalSlideSeconds >= InboundAnnouncementLeadSeconds` the
     announcement instead fires on entering `Approaching`, once, from the same
     flag.
   - **Approaching**: when `PhaseTimer <= 0`, the train has stopped. Go to
     `Dwelling` (`PhaseTimer = DwellDuration`), call `OnArrivalComplete()` (the
     brake screech and the "on arrival" announcement hang off this). Doors are
     still shut. Do not open them here.
   - **Dwelling**: open the doors when `DwellDuration - PhaseTimer >=
     DoorOpenDelaySeconds` and `!bDoorsOpen`: set `bDoorsOpen = true`, call
     `OnDoorsOpen()`. Close them when `PhaseTimer <= DoorCloseLeadSeconds` and
     `bDoorsOpen`: set `bDoorsOpen = false`, call `OnDoorsClose()`, call
     `OnDepartureAnnouncement()`. When `PhaseTimer <= 0`, go to `Departing`
     (`PhaseTimer = DepartureSlideSeconds`), call `OnDepartureStarted()`, and
     apply the not-boarded consequence (next section) exactly once via
     `bDepartureHeatApplied`.
   - **Departing**: when `PhaseTimer <= 0`, go to `Away`
     (`PhaseTimer = TrainInterval`), call `OnTrainAway()`, clear the per-cycle
     flags.
3. On any phase change, broadcast `OnTrainPhaseChanged` with the new and old
   value, and `LT_LOG(Log, TEXT("Train phase %s -> %s"), ...)`.

Guard the whole tick on the run being live: if the `ALTGameState` run state is
`Dead` or `Boarded`, freeze the state machine (early return before step 1). A
boarded or dead run has no more trains.

#### Presentation hooks

All `BlueprintImplementableEvent`, category `Train`, no bodies. A station
Blueprint drives the mesh slide, lights, audio and door animation off these. No
timelines, no `USceneComponent` slide logic, no `UAudioComponent` in C++.

```cpp
/** T-15s inbound (or on entering Approaching if the slide is long). Diegetic
    platform announcement. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnInboundAnnouncement();

/** The inbound slide begins. Headlights on, rumble bed ramps, the mesh starts
    its move down the trackbed. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnArrivalStarted();

/** The train has stopped. Brake screech one-shot, the "on arrival" announcement. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnArrivalComplete();

/** Doors open. One event whether the station has car doors only or car plus
    screen doors. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnDoorsOpen();

/** Doors close. Fires DoorCloseLeadSeconds before departure. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnDoorsClose();

/** "Stand clear of the doors" announcement, fired with OnDoorsClose. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnDepartureAnnouncement();

/** The outbound slide begins. Rumble fades, the mesh pulls away. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnDepartureStarted();

/** The train has fully left. The trackbed is empty again. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnTrainAway();

/** A successful board. Blueprint plays a door-chime and a board confirm before
    the screen goes to the travel transition a later task owns. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnPlayerBoarded();
```

`[DECISION NEEDED: open-questions 4.4 announcement content and voice - the exact
lines and the voice source are not settled. Proposed default: this task fires
the four hooks (inbound, on-arrival, departure, and the breather idle line which
is out of scope here) and leaves the strings and voice to the Blueprint and a
later audio task. Do not put announcement text in C++.]`

`[DECISION NEEDED: open-questions 4.13 multiple trains over a long run - the full
arrival spectacle repeating every 100s for a 20 round run is too much. Proposed
default: expose bFullArrivalPresentation, default true, and set it false after
the third OnArrivalStarted so a Blueprint can pick a shorter cycle. This task
only exposes the flag and flips it; it does not change any timing.]`

#### The boarding trigger

A `UBoxComponent` named `BoardingVolume`, child of the root, `EditAnywhere`
transform so it can be placed over the door aperture in the level. Overlap only,
no block. It is the trigger, and the boarding check gates on `Phase` and
`bDoorsOpen`, so no new trace or object channel is needed.

`[DECISION NEEDED: open-questions 4.5 boarding trigger and confirm - walk-in vs
interact prompt vs hold-to-confirm. Proposed default: boarding is an interact.
ALTTrain also implements ILTInteractableInterface so the existing
ULTInteractionComponent drives the prompt. CanInteract returns true only while
Phase == Dwelling and bDoorsOpen and the run is not over. GetInteractionPrompt
returns "Board train". Interact calls TryBoard(Interactor). BoardingVolume then
exists only to scope the interactable (the plate/aperture the player looks at);
if the owner prefers a pure walk-in trigger, BoardingVolume's OnComponentBeginOverlap
calls TryBoard directly and the interface is dropped. Pick one before coding.]`

`[DECISION NEEDED: open-questions 4.5 fat-finger guard - a 21s window with an
accidental board ends the run against the player's will. Proposed default: a
1.0s hold-to-board. Since ULTInteractionComponent binds Interact to
ETriggerEvent::Started only, a hold needs either a Held trigger on the action or
a confirm re-press within 3s. Simplest for this task: no hold, TryBoard commits
on the single interact, and the confirm is deferred to the travel task. Confirm
whether that is acceptable for the grey box.]`

`[DECISION NEEDED: open-questions 4.6 station-select UI - "travels to a chosen
adjacent station" needs a picker when there is more than one adjacency. Proposed
default: OUT of scope for C1. TryBoard commits immediately with no destination.
NotifyPlayerBoarded takes no target station argument yet. The travel task adds
the picker and the argument. Note this so the later task knows the seam moved.]`

`TryBoard(AActor* Boarder)`:

```cpp
bool ALTTrain::TryBoard(AActor* Boarder)
{
	if (Phase != ELTTrainPhase::Dwelling || !bDoorsOpen)
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
```

The train does not itself touch the round manager, the weapon component, the
points component or heat. It calls one game mode hook and lets the game mode
orchestrate, matching how `NotifyPlayerDied` already works. Keep the train thin.

### 2. `ALTGameMode::NotifyPlayerBoarded`

`Source/LastTrain/Public/Core/LTGameMode.h` and its `.cpp`.

Add next to `NotifyPlayerDied`:

```cpp
/** Called by ALTTrain when the player boards during the dwell. In this arena it
    ends the run: stops rounds, banks points, refills the reserve, resets heat,
    and flips the run state to Boarded. Travel to the next station is a later
    task that hooks in here. */
UFUNCTION(BlueprintCallable, Category = "Run")
void NotifyPlayerBoarded(AActor* Boarder);
```

Implementation, matching the style of the existing notify handlers:

1. If `bRunStarted` is false or the game state is already `Dead` or `Boarded`,
   return.
2. `ALTRoundManager* RoundManager = FindRoundManager();` if valid,
   `RoundManager->StopRounds();`
3. Resolve the boarder's components and apply the board rewards:
   - `ULTWeaponComponent* Weapon = Boarder ? Boarder->FindComponentByClass<ULTWeaponComponent>() : nullptr;`
     if valid, `Weapon->RefillAmmunition();` (reserve to full, magazine
     untouched, per `RefillAmmunition`'s existing behaviour and open-questions
     4.9).
   - Heat: find the `ULTStationHeat` the round manager holds, or iterate for it,
     and call `ResetHeat()`.
4. `[DECISION NEEDED: open-questions 4.8 "banks points" - brief-v2 says boarding
   "banks points" but there is no points-at-risk mechanic in code and 4.8's
   suggested default is that the phrase is legacy and points are never lost.
   Proposed default: NotifyPlayerBoarded does nothing to ULTPointsComponent, and
   a code comment records that "banks points" resolved to a no-op. If the owner
   instead wants a points-to-Oyster-Credit conversion on board (4.8's PROPOSAL),
   that is a separate meta-economy task and needs sign-off, not a default.]`
5. `GetLTGameState()->SetRunState(ELTRunState::Boarded);`
6. `LT_LOG(Log, TEXT("Player boarded. Run state Boarded, rounds stopped, reserve refilled, heat reset. Travel to next station is not wired yet."));`

Do not add travel, level loading or a `ULTGameInstance` payload here. That is the
seam; the later task fills it.

`[DECISION NEEDED: open-questions 4.7 travel transition and 17.1 ULTGameInstance
- what actually happens after Boarded (fade, load, rehydrate) is a real
architecture call. Proposed default: OUT of scope for C1. The run simply sits in
Boarded with rounds stopped. A later "Phase C travel" task owns the transition
and the persistence.]`

### 3. Not boarding raises heat

When the state machine leaves `Dwelling` for `Departing` and the run state is
not `Boarded`, the player let the train go. Apply the consequence once per
departure, in `ALTTrain::Tick` at the `Dwelling -> Departing` transition, gated
by `bDepartureHeatApplied`:

```cpp
if (!bDepartureHeatApplied)
{
	bDepartureHeatApplied = true;

	if (RunStateIsLive())	// not Dead, not Boarded
	{
		if (ULTStationHeat* Heat = FindStationHeat())
		{
			Heat->IncrementHeat();
			LT_LOG(Log, TEXT("Train departed, player did not board. Heat now %d"), Heat->GetHeat());
		}
		OnTrainDeparted_NotBoarded();
	}
}
```

Add one more presentation hook for the feedback:

```cpp
/** The train left without the player. A Blueprint plays the heat-rise sting and
    the station's crimson creep step. brief-v2: staying raises heat by 1. */
UFUNCTION(BlueprintImplementableEvent, Category = "Train")
void OnTrainDeparted_NotBoarded();
```

`FindStationHeat()` is a private helper: try the round manager's held
`ULTStationHeat` first (it is private on the round manager, so iterate actors for
`ALTRoundManager`, then `FindComponentByClass<ULTStationHeat>()` on it), fall
back to iterating all actors for any `ULTStationHeat`. `LT_LOG(Warning, ...)` and
carry on if none is found, so a heat-less test level does not crash.

`[DECISION NEEDED: open-questions 5.1 heat model - ULTStationHeat as written uses
LiveCapPerHeat = 6, SpawnRateFractionPerHeat = 0.12, MaximumHeat = 10, matching
brief-v2's "+6 to the live cap and 12% to spawn rate". open-questions 5.1
supersedes those with a profile-derived model (HEAT_CAP_ADD = 4, HEAT_MAX = 5,
HEAT_RATE_MULT = 0.90). Proposed default for C1: leave ULTStationHeat's numbers
as they are, this task does not retune it, and note that 5.1's retune is its own
task once the section 18 frame profile exists.]`

### Interaction with the round loop

brief-v2: boarding "ends the round". `ALTRoundManager::StopRounds` is the tool.

`[DECISION NEEDED: open-questions 4.13 / no clean source - what happens to the
round in progress, the pending spawns and the live zombies when the player
boards. Proposed default: NotifyPlayerBoarded calls RoundManager->StopRounds()
and nothing else. Live zombies are left in the world (harmless, the player is
about to leave the arena on travel); PendingSpawns stop because StopRounds
clears bRunning. The travel task, when it destroys and rebuilds the arena, takes
the live zombies with it. If the owner wants live zombies despawned on board for
a cleaner grey box, that is a one-line addition to NotifyPlayerBoarded, confirm
it.]`

`[DECISION NEEDED: open-questions 4.13 train as a nav obstacle and track-side
spawn gating - a stopped train blocks track-side spawn points and should carve
the navmesh. Proposed default: OUT of scope for C1. The grey box trackbed is
flat and has no track-side spawns. When Canary Wharf gets track-side spawns, a
follow-up adds a bTrainPresent gate to ALTSpawnPoint::IsAvailable and marks the
train a dynamic nav obstacle while Dwelling. Not this task.]`

## Constraints

- One new folder, `Source/LastTrain/Public/Train/` and
  `Source/LastTrain/Private/Train/`, holding `LTTrain.h` and `LTTrain.cpp`.
- The only edits outside that folder are `ALTGameMode` (the `NotifyPlayerBoarded`
  hook, header and cpp) and, if the interaction route is chosen for boarding,
  `ALTTrain` implementing `ILTInteractableInterface` (which lives entirely in
  the new files). Do not modify `ALTGameState`, `ALTRoundManager`,
  `ULTStationHeat`, `ULTWeaponComponent`, `ULTPointsComponent`,
  `ULTInteractionComponent` or `ALTSpawnPoint`. If this task appears to need a
  change in one of those, stop and say so.
- Compiles clean with the batch build in `CLAUDE.md`. `-Werror` is on: no
  unreachable code, no unused variables or parameters, no shadowed parameters.
  Name unused interface parameters or `(void)` them.
- No new trace or object channels. NeoStack cannot create them and a human would
  have to. Use `Phase` plus `bDoorsOpen` to gate boarding, and a `UBoxComponent`
  overlap if a walk-in trigger is chosen. Reuse `ECC_GameTraceChannel1` only if
  the interaction route needs the plate to be look-at traceable, which it
  already is through `ULTInteractionComponent`'s existing `ECC_Visibility` sweep,
  so no channel work at all.
- British spelling everywhere including comments and the log strings. No em or en
  dashes, plain punctuation only. `#pragma once` first, `LTTrain.generated.h`
  last include. `LT_LOG(Verbosity, TEXT("..."))`, never `UE_LOG`.
- `TObjectPtr` for every `UObject` member and every `UObject` in a container,
  never a raw `UObject*`.
- No `TODO`, `FIXME`, `HACK`, `XXX` markers. `tools/ci/check_hygiene.py` rejects
  them. The `[DECISION NEEDED: ...]` markers in this spec are for the owner to
  resolve before coding; they must not appear in the source.
- Interface calls from C++, if the interaction route is chosen, go through the
  generated `Execute_` statics.
- `LT` prefix on every type. Tab indent for `.h` and `.cpp`. clang-format 20.
- Legal: no roundel, no Johnston or New Johnston typeface, no reproduction of
  the official line diagram, no operator livery or logo, no transcribed
  announcement recordings, no Call of Duty names anywhere. This task adds no
  strings that touch any of that; the announcement text is a later task's
  problem and must be written fresh.

## Acceptance

Run in PIE once `ALTGameMode` is the level's game mode, an `ALTRoundManager` is
present and running, a `ULTStationHeat` component is on the round manager (or an
actor in the level), and a placeholder `ALTTrain` with any box mesh child is
placed at the platform edge. Temporary print or log nodes on the delegates make
each step observable.

1. Compiles clean with the batch build. No warnings.
2. On `BeginPlay` the train is in `Away`. `GetSecondsUntilArrival` returns a
   value counting down. With `TrainInterval` temporarily set to 20 for the test,
   the train reaches `Approaching` at roughly load + `FirstTrainStopSeconds -
   ArrivalSlideSeconds` (or load + `TrainInterval - ArrivalSlideSeconds` if the
   first-train-zero marker was resolved against a special first cycle), and
   `OnTrainPhaseChanged` fires `Away -> Approaching`.
3. `OnInboundAnnouncement` fires once, `InboundAnnouncementLeadSeconds` before
   the train stops (during the inbound slide with the default 15s lead and 4s
   slide, that is 11s into `Away`'s tail, i.e. slide start minus 11s). It does
   not fire again that cycle.
4. `OnArrivalComplete` fires when the train stops, `ArrivalSlideSeconds` after
   `OnArrivalStarted`. `GetSecondsUntilArrival` reads 0 from this point until
   the next `Away`.
5. Doors: `OnDoorsOpen` fires `DoorOpenDelaySeconds` after the stop.
   `OnDoorsClose` fires `DoorCloseLeadSeconds` before departure. The gap between
   them is `DwellDuration - DoorOpenDelaySeconds - DoorCloseLeadSeconds` (21s on
   the defaults), and both events land strictly inside the `Dwelling` phase, not
   during `Approaching` or `Departing`. `AreDoorsOpen` is true only in that gap.
6. `TryBoard` (or the interact prompt, per the resolved marker) does nothing and
   returns false during `Approaching`, during `Dwelling` before `OnDoorsOpen`,
   after `OnDoorsClose`, and during `Departing`. It succeeds only in the
   open-door window.
7. Boarding in the open-door window: `OnPlayerBoarded` fires,
   `ALTGameMode::NotifyPlayerBoarded` runs, the round manager stops (no new
   spawns, `GetZombiesRemaining` stops climbing), the run state flips to
   `Boarded` (`ALTGameState::GetRunState`), the weapon reserve goes to full
   (`ULTWeaponComponent::GetReserve` equals its max, magazine unchanged), and
   `ULTStationHeat::GetHeat` is 0.
8. After boarding, the train state machine is frozen: no further phase changes,
   no further announcement hooks.
9. Not boarding: let a full dwell elapse without boarding. On the `Dwelling ->
   Departing` transition `ULTStationHeat::GetHeat` increments by exactly 1 and
   `OnTrainDeparted_NotBoarded` fires once. Letting a second full cycle pass
   without boarding takes heat to 2. A very long `DepartureSlideSeconds` does
   not double-count.
10. `OnTrainAway` fires when the outbound slide finishes, the phase returns to
    `Away`, and `GetSecondsUntilArrival` counts down again from roughly
    `TrainInterval + ArrivalSlideSeconds`.
11. `GetSecondsUntilDeparture` returns 0 in every phase except `Dwelling`, where
    it counts down from `DwellDuration` to 0.
12. With the player dead (`ELTRunState::Dead`) before a train arrives, the state
    machine freezes and no train arrives.

## On pass

Update `docs/tasks/NEXT.md` and the Phase C row in `docs/tasks/README.md`. The
next Phase C tasks, in rough order: the departure board actor `ALTDepartureBoard`
reading `GetSecondsUntilArrival` / `GetSecondsUntilDeparture` (write
`docs/tasks/phase-c2-departure-board.md`); the travel transition and
`ULTGameInstance` payload that fills the `NotifyPlayerBoarded` seam (write
`docs/tasks/phase-c3-travel.md`); the five zombie types; and the heat retune
against the section 18 frame profile once that profile exists.

# Handover 2026-09-06 PM

Context capture before a usage-limit cutoff. Two subagents were running when this
was written. This doc is the resume point.

## Repo state at capture

- Branch `main`, HEAD `93de8e8`, up to date with `origin/main`.
- Working tree (uncommitted):
  - `M Source/LastTrain/Public/Core/LTGameMode.h` - Opus agent added `NotifyPlayerBoarded` + `FindStationHeat` declaration. Looks correct, matches spec section 2.
  - `M Source/LastTrain/Private/Core/LTGameMode.cpp` - Opus agent added `NotifyPlayerBoarded` body + `FindStationHeat` helper. Looks correct: guards on `bRunStarted`, Dead/Boarded early-return, stops rounds, refills reserve, resets heat via the round-manager-first-then-any-actor lookup, flips to Boarded, logs. "banks points" no-op comment present per RESOLVED note.
  - `?? Source/LastTrain/Public/Train/LTTrain.h` (8057 bytes) - new, Opus agent.
  - `?? Source/LastTrain/Private/Train/LTTrain.cpp` (6544 bytes) - new, Opus agent.
  - `M docs/tasks/NEXT.md`, `M docs/tasks/README.md` - Opus agent updating the "On pass" docs.
  - `D Content/LastTrain/Maps/L_CanaryWharf_Greybox.umap` - **the local file was deleted.**
    It is committed at `f76e0a4` on `origin/main`, so it is fully recoverable:
    `git checkout -- Content/LastTrain/Maps/L_CanaryWharf_Greybox.umap`
    Do this before opening the editor on Canary Wharf. Cause unknown (editor op or NeoStack).
- Stash: empty.
- `Binaries/Mac/libUnrealEditor-LastTrain-0001.dylib` timestamped 21:40 - the Opus
  agent's build may be in progress or done. Check its final report.

## Subagent 1: Opus - build ALTTrain (Phase C1)

- **Spec:** `docs/tasks/phase-c1-train.md` (558 lines, all decisions resolved inline).
- **Scope:** `ALTTrain` AActor - arrive/dwell/depart/away state machine on brief-v2
  timing (100s interval, 25s dwell), `BlueprintImplementableEvent` presentation hooks,
  boarding via `ILTInteractableInterface`, `FirstTrainStopSeconds = 30` first cycle,
  heat +1 on depart-not-boarded gated by `bDepartureHeatApplied`. Plus
  `ALTGameMode::NotifyPlayerBoarded(AActor*)`.
- **Files it may touch:** new `Source/LastTrain/{Public,Private}/Train/LTTrain.{h,cpp}`
  and `Core/LTGameMode.{h,cpp}` ONLY. It was told to STOP and report if it needs any
  other file, not work around it.
- **It was given:** the build command, external Xcode path check, the clang-format 20
  binary path (`.../scratchpad/cf-venv/bin/clang-format`), verified API signatures for
  `ALTGameMode`/`ALTGameState`/`ALTRoundManager`/`ULTStationHeat`/`ULTWeaponComponent`/
  `ILTInteractableInterface`, and `ALTWallBuy` as the interface-impl template.
- **It will NOT run git.** Leaves the tree for review.
- **On its completion, this agent (or the next session) must:**
  1. Read its report: did `/Volumes/DriveSohaib` mount, did `Build.sh` say BUILD SUCCESSFUL,
     which files changed, was clang-format run, did it hit a forbidden-file wall.
  2. Review `LTTrain.h` / `LTTrain.cpp` against the spec's 12-point acceptance list
     (can't PIE, so review is static: state machine transitions, the `GetSecondsUntilArrival`/
     `GetSecondsUntilDeparture` math per spec lines 208-212, the `bDepartureHeatApplied`
     double-count guard, `TryBoard` phase+doors gate, interface `_Implementation` overrides,
     `TObjectPtr` on all UObject members, British spelling, no dashes, `#pragma once` first
     / `LTTrain.generated.h` last, `LT_LOG` not `UE_LOG`, no TODO markers).
  3. Run the 4 CI checkers locally:
     `python3 tools/ci/check_hygiene.py && python3 tools/ci/check_cpp_conventions.py && python3 tools/ci/check_docs.py && python3 tools/ci/check_content.py`
     (clang-format check needs the venv binary on PATH or invoke directly).
  4. If clean: `git checkout -- Content/LastTrain/Maps/L_CanaryWharf_Greybox.umap` to
     restore the map, then commit the train work:
     `git add Source/LastTrain/Train/ Source/LastTrain/Public/Train/ Source/LastTrain/Private/Train/ Source/LastTrain/Public/Core/LTGameMode.h Source/LastTrain/Private/Core/LTGameMode.cpp docs/tasks/NEXT.md docs/tasks/README.md`
     commit message style: `feat(train): Phase C1 train actor, timing state machine, boarding path`
     with the Co-Authored-By / Claude-Session trailers from CLAUDE.md.
  5. Push `main` only with explicit user go-ahead (the user has approved pushes this
     session in general, but confirm for this one since it is a fresh feature).
  6. Phase C1 acceptance (the 12-point PIE list) is then a NeoStack or user task -
     it needs a placeholder ALTTrain placed in a level with a box mesh child, ALTGameMode
     as the level game mode, a round manager + station heat present.

## Subagent 2: Sonnet - zombie asset pack research

- **Task:** top 5 free/cheap zombie character packs for UE5.8 matching the game's art
  brief (grounded modern-London civilian realism, not cartoon/fantasy/military; wet/grime;
  realistic proportions; readable at mid-range low light; palette charcoal/violet/sodium/crimson).
- **Wants per pack:** name/creator/marketplace/URL, price (flag free + Fab free-of-month),
  licence one-liner (ships commercial? royalties?), what you get (unique chars, LODs,
  **Epic Skeleton / Mannequin compat** - matters most, game uses standard Mannequin),
  animations included, texture res, rig quality, art-style honesty (say what is off),
  polycount/perf (needs ~24 alive @ 60fps). Plus: is there a strong free baseline
  (Mixamo + free mesh, Epic free packs) for a prototype before spending.
- **Output:** ranked table + one-paragraph "use X for Sunday prototype, buy Y for the
  vertical slice" recommendation.
- **On its completion:** save the ranked list into a new doc
  `docs/reference/zombie-asset-options.md` (British spelling, no dashes) and add a line
  to `docs/tasks/NEXT.md`. This does not block anything - it is input for the art pass.

## NeoStack (separate tool, user runs it)

- **Prompt is at:** `scratchpad/neostack-prompt-canary-spawn.txt` (also was on clipboard).
  Rendered version: `scratchpad/neostack-prompt.html`.
- **Model to pick in NeoStack:** Opus, Medium effort, Bypass Permissions.
- **The prompt's job:** Canary Wharf spawn points not firing. `ALTRoundManager::TrySpawnOne`
  finds `Available.Num() == 0` because `ALTSpawnPoint::IsAvailable` returns false for all
  10 points (gated by `bEnabled` / `AreaTag` / `FirstRound` / `CooldownSeconds`), OR
  `ZombieClass` is None on `CW_RoundManager`.
- **Step zero the prompt adds:** NeoStack must `git pull` on `main`, fully quit UnrealEditor,
  rebuild the editor target, relaunch. Its last run reported the crowd gate, corridor
  stall, and free-fall as "still broken" because it tested a binary built BEFORE the
  fixes in `bb0ec42` / `f76e0a4`. All three are fixed on `main` already.
- **Live NeoStack doc:** `docs/tasks/neostack-run-2026-09-06.md` - NeoStack updates this.
- **NeoStack rules:** never edits `Source/`, never runs git (except the one `git pull` in
  step zero), stops PIE cleanly, never mutates actors/assets during PIE.

## The three fixes already on main (context for why NeoStack was confused)

Commit `bb0ec42`:
1. `LTRoundManager.h` `OpeningRoundCounts` changed `EditDefaultsOnly` -> `EditAnywhere`
   so a placed round manager instance can be tuned (crowd gate was unmeasurable).
2. `LTZombieCharacter.cpp` corridor stall rewritten as an explicit state machine:
   `BeginStallRecovery` cancels the AI move request (`AAIController::StopMovement`)
   AND disables `bUseRVOAvoidance`, because `PathFollowingComponent` sets velocity
   every frame while a MoveTo is active (clobbering `AddMovementInput`) and RVO brakes
   a boxed-in zombie to zero. `DriveStallNudge` does a lateral shove. `EndStallRecovery`
   restores RVO + forces a repath.
3. `LTRoundManager.cpp` `TrySpawnOne` now projects the spawn location onto the navmesh
   (`UNavigationSystemV1::ProjectPointToNavigation`) and lifts by `SpawnCapsuleLift`
   before spawning, so a hand-placed spawn point slightly off the mesh does not drop
   a zombie through the floor. Warns if not near the navmesh.

Commit `f76e0a4`: `L_CanaryWharf_Greybox.umap` - added/sized a NavMeshBoundsVolume over
`CW_Floor`, moved the 10 `CW_SpawnPoint` actors from world origin (0,0,0) to a line
across the platform (~y=5600, x 900..10200, z=100). Navmesh now builds (378 tiles,
green with `P`). Scripts used: `scratchpad/place_nav.py`.

## Immediate next actions in priority order

1. **[blocking-playable]** User pastes the NeoStack prompt, NeoStack pulls + rebuilds +
   un-gates the Canary Wharf spawn points. Result: zombies spawn -> first playable round.
2. **[this agent, on subagent completion]** Review + CI + commit the Opus `ALTTrain` work.
   Restore the deleted `L_CanaryWharf_Greybox.umap` first.
3. **[this agent, on subagent completion]** Save the zombie asset research into
   `docs/reference/zombie-asset-options.md`.
4. **[user]** Play one full round in Canary Wharf once zombies spawn, note the top 2-3
   feel-breakers.
5. **[this agent]** Fix the feel-breakers.
6. **[later]** Phase C2 departure board, C3 travel, the 5 zombie types (Sonnet, bulk
   data entry once `ULTZombieTypeData` schema exists), art pass (Fable), HDD migration.

## Model guidance recap (from CLAUDE.md)

- Opus: C++ against convention, data assets, Blueprint setup, layout, balance, docs.
- Fable: Lumen/post tuning, materials, animBP blend logic, crowd perf once profiled,
  any bug surviving two Opus attempts. The corridor stall has survived one fix that
  did not fire - if NeoStack's Opus run does not crack it, that item goes to Fable.
- Sonnet: bulk data entry against a fixed schema.
- No single model finishes the game end to end: editor work is NeoStack or the user,
  C++ is this agent / Opus, art is Fable later, and the biggest lever (a zombie asset
  pack + animations) is integration work gated on the research subagent.

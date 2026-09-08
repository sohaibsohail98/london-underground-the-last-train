# Handover - the resume point

Last updated 2026-09-07. This file replaces the old `handover.md` and the dated
handover: one place for where the project stands and what is left. **Update it
whenever a task finishes**, so a cold session can pick up without re-reading the
history.

## The one line

Phase A is done. Phase B's code is all in and Phase B is one measurement away
from signable. **Every Phase C system now exists in C++**, plus E1 downed and
revive from Phase E. Everything after C1 was written in remote sessions with no
Unreal engine and **has never been compiled**, so the first local build is the
gate on the lot. Once it compiles, the remaining work is almost entirely editor
work: five data assets, a material, three Blueprints, a reparent, and ten spawn
points to move. That brief is `neostack.md`.

## The gate: compile first

```
./tools/ci/compile.sh
```

That wrapper is the supported build. It checks the external Xcode mount, then
runs the engine's `Build.sh` for `LastTrainEditor Mac Development`. Override the
engine path with `LASTTRAIN_ENGINE_ROOT` if the drive migration has moved it.

Roughly 1,900 lines of C++ across `64535f6` and `b1424bf` have not seen a
compiler. Fix whatever it finds before touching the editor, because
`Config/DefaultEngine.ini` now pins the game instance class: a `ULTGameInstance`
that fails to compile takes PIE with it, in every map.

Three things about that ini line nobody had written down:

- An editor that was already open when it landed reads the old value until it
  restarts.
- An editor on a pre-C2 binary logs a load error for the missing class and
  silently loses travel, with no in-game symptom pointing at the ini.
- A packaged build will eventually need both maps in the cook set. `OpenLevel` by
  name is the first thing in this project that depends on a map no asset
  references.

## Exact next actions, in order

1. **Compile** (above). Nothing below is worth starting first.
2. **Reparent `BP_GameMode` to `ALTGameMode`.** It is still on plain
   `GameModeBase`, so no run lifecycle, no boarding and no travel runs at all.
   This is the single biggest gap between the C++ and what the editor does.
3. **Fill `StationRoutes`** on that game mode: `L_GreyboxTest` to
   `L_CanaryWharf_Greybox`, and `L_CanaryWharf_Greybox` back to `L_GreyboxTest`.
   With neither a route nor `NextStationMap`, boarding stops the run in a dead
   station with no way out.
4. **Work through `neostack.md`, Phase C.** The five `ULTZombieTypeData` assets
   with the full value table, the tintable material and its five instances, the
   `BP_Zombie` graph additions, the roster wiring, `BP_Train`,
   `BP_DepartureBoard`.
5. **Move the ten Canary Wharf spawn points** off world origin, per
   `neostack.md`. The C++ half of that bug is fixed; only placement is left.
6. **Measure the Phase B crowd gate.** Details in `neostack.md`, Phase B
   leftovers. This is the last thing between Phase B and done.
7. **Run the acceptance lists** in the table below.
8. **Mark Phase B done** in `README.md` and here once 6 and the B2 and B3 items
   pass.

## What has landed

### Phase A, done 2026-09-04

`L_GreyboxTest` plays end to end. The A5 acceptance passed on every testable
check. Records: `phase-a4-editor-setup.md`, `phase-a5-acceptance.md`.

### Phase B, code complete, one measurement short

B1 throttled repath, B2 interaction and the first wall buy, B3 the HUD widget.
The zombie attack fix and the HUD health bar are verified in PIE. **Open**: the
24 to 40 crowd frame gate has never been measured (it is reachable now, see
action 6), the wall-buy flow has never been exercised in PIE, and the HUD polish
has never been reviewed. The corridor stall edge case has a verdict and a fix as
of 2026-09-07, unverified in PIE: open code item 1 below.

### Phase C, all C++ written, none of it compiled

- **C1 `ALTTrain`** (`phase-c1-train.md`): arrive, dwell, depart, away on the
  100s interval and 25s dwell, nine presentation hooks, the boarding interact,
  and the heat increment on a not-boarded departure. **This is the only Phase C
  code that has ever compiled.** Its 12-point PIE list needs a `BP_Train`.
- **C2 `ALTDepartureBoard`** (`phase-c2-departure-board.md`): finds the level's
  train or takes an instance override, polls the countdown getters, fires
  `OnCountdownChanged` only when the whole second moves, forwards
  `OnPhaseChanged`. `ALTTrain` untouched.
- **C3 travel** (`phase-c3-travel.md`): `ULTGameInstance` carries points, weapon,
  reserve and the visited-station list across an `OpenLevel`;
  `NotifyPlayerBoarded` snapshots the carry and travels to the routed map after
  `TravelDelaySeconds`; `RehydrateFromTravel` grants it on the far side on the
  first tick after `StartRun`, retrying for a few ticks if the pawn is not
  possessed yet. An unclaimed payload is dropped one map load on, so a stale
  carry cannot reach an unrelated run. Two design choices: rounds restart at 1 on
  travel, and the reserve carries **to full** rather than to the exact count,
  because `ULTWeaponComponent` has no reserve setter. `CarriedReserve` is
  recorded for when one lands.
- **Five zombie types** (`phase-c-zombie-types.md`): `ULTZombieTypeData` plus
  `ApplyTypeData`, the brute's front armour plate, the sprinter's lunge, the
  screamer's sight line and summon with its cancel window, and the weighted
  roster with `FirstRoundAvailable`, per-type live caps and the heat-3 weight
  shift. `ZombieClass` is still the only class spawned: one mesh. An empty roster
  is byte-for-byte the old single-walker behaviour.
- **Special rounds** (`phase-c-special-rounds.md`): `FLTRoundPlan` decided once
  per round, a three-way branch in `TrySpawnOne`, and `GetSpecialRoundTag` for a
  banner. Reviewed against its spec on 2026-09-07, write up in
  `phase-c-review-2026-09-07.md`. The first deviation stands: the brute pair
  lands at roughly 30 and 70 per cent through the round rather than as a group up
  front, because `docs/design/gameplay-canon.md` section 6 says so. The second
  was wider than recorded here, and changed: every brute round is divisible by 5,
  so round 10 was an all-sprinter round carrying two brutes, against canon's own
  "a normal walker round plus 2 brutes" and all three acceptance lists. It is now
  `bBruteRoundOverridesSprinterRound`, default set, so a brute round is a walker
  round plus the pair. Clearing it on `BP_RoundManager` restores `b1424bf`. Canon
  section 6 contradicts itself here and wants settling either way.

### Phase E, started

**E1 downed and revive** (`phase-e1-downed-revive.md`): zero health calls
`Down()`, never the death path. Movement is disabled and fire, aim, reload,
interact and sprint no-op while down; `Look` stays free. `BleedOutSeconds` (30)
runs in `Tick`, and expiry fires `OnBleedOutExpired` then `Die()`, now the only
route to `NotifyPlayerDied`. `bSoloAutoRevive` stands the player up at half
health after 8s. Damage while down is ignored so it cannot shorten the clock.
`Down()` is also a no-op once the run state is `Boarded` or `Dead`. Seams left
open: `Revive()` for an item or a co-op revive, `OnDowned`/`OnRevived` for a
last-stand weapon swap, and the downed screen treatment for the HUD and art pass.

Perks, the upgrade bench and lost property are untouched.

## Acceptance lists that have never been run

| What | Where | Needs |
|---|---|---|
| Crowd frame gate, 24 to 40 at 60fps | `phase-b1-throttled-repath.md` | PIE, `stat unit` |
| Wall buy prompt and purchase | `phase-b2-interaction.md` steps 3 to 7 | a human at the keyboard |
| HUD polish: hit marker, spread, prompt anchor | `phase-b3-feedback-widgets.md` | PIE |
| Train, 12 points | `phase-c1-train.md` | `BP_Train` |
| Live countdown on a sign | `phase-c2-departure-board.md` | `BP_DepartureBoard` |
| Board and arrive with the carry | `phase-c3-travel.md` | reparent plus routes |
| Zombie types, 11 points | `phase-c-zombie-types.md` | five assets, material, roster |
| Sprinters on 5 and 15, brutes on 10 and 20 | `phase-c-special-rounds.md` | roster |
| Down, bleed out, auto revive | `phase-e1-downed-revive.md` | PIE only |

## Open code items

Seven of the eight are closed in the working tree by the 2026-09-07 review pass,
written up in `phase-c-review-2026-09-07.md`. **None of it is compiled**, so the
first local build still gates the lot.

1. **The corridor stall edge case: verdict reached, fixed.** It was a gate order
   bug, and the gate was the repath in `Tick`, not `UpdateStallRecovery`.
   `BeginStallRecovery` cancels the AI move on purpose, and nothing stopped the
   next repath re-issuing it 0.35s later, handing velocity back to path following
   and returning the zombie to the stall. Two more faults in the same path:
   `StallTimer` was reset to zero by any single frame of jitter above
   `StallSpeedThreshold`, so the grace never accumulated, and recovery ended on
   one jittery frame without the zombie having gone anywhere. The repath is now
   held back while recovering, the timer decays instead of resetting, exit needs
   40 units of the zombie's own displacement, and `StallRecoverySeconds` (1.5)
   caps a shove so a
   zombie pressed into geometry hands control back and leads with the other
   shoulder. Unverified in PIE.
2. **`NavProjectionExtent` Z: fixed.** A `NavProjectionWarnDistance` (200) logs
   which spawn point snapped and how far. That is the warning the Canary Wharf
   points at world origin never fired. The extent stays generous: tightening it
   would turn a point that used to snap into one that drops a zombie into the
   void, and the silence was the bug.
3. **Arrival flashing the points HUD as a spend: fixed.**
   `ULTPointsComponent::SetPoints(int32)` assigns and broadcasts a zero delta,
   and `RehydrateFromTravel` uses it.
4. **Interaction prompts while downed: fixed.**
   `ULTInteractionComponent::SetInteractionEnabled(bool)` stops the sweep and
   clears the live prompt. `Down` and `Die` disable it, `Revive` re-enables it.
5. **Solo death is unreachable on the shipped defaults.** Auto-revive at 8s
   against a 30s bleed-out, damage ignored while down, no cap on repeated downs.
   That is exactly what E1 specified, but Phase E's own gate ("a full survival
   session start to death is possible") cannot be met until a revive item, a
   per-run cap, or a cleared `bSoloAutoRevive` lands. Clearing the flag on
   `BP_PlayerCharacter` is the one-click version. **Still open**: it is a design
   and editor call, not code.
6. **`ALTGameState::SetStationName`: fixed.** `ALTGameMode` carries
   `StationDisplayNames`, a map from map asset name to label keyed like
   `StationRoutes`, plus a `StationDisplayName` fallback, stamped on `BeginPlay`.
   **Editor follow-up**: fill two entries on `BP_GameMode` or the label stays
   blank.
7. **`ALTTrain`'s class comment: fixed.** One line.
8. **`ALTPlayerCharacter::TakeDamage`: fixed.** It subtracts what
   `Super::TakeDamage` returns, and a fully absorbed hit no longer restarts the
   regeneration delay.

## Build, CI and toolchain

- **Compile:** `./tools/ci/compile.sh`. There is still no hosted compile: Unreal
  cannot be installed on a GitHub runner. The workflow carries an opt-in
  `compile` job that runs the same script on a self-hosted macOS runner, enabled
  by registering a runner with labels `self-hosted, macOS, unreal` and setting
  the repository variable `UNREAL_SELF_HOSTED` to `true`. Without that variable
  the job is skipped, so it never sits queued against a runner that does not
  exist.
- **Five CI gates**, on every push to `main`, `claude/**` and the phase branches,
  and on every pull request: `check_hygiene.py`, `check_cpp_conventions.py` plus
  clang-format 20, `check_cpp_reflection.py`, `check_docs.py`,
  `check_content.py`.
- `check_cpp_reflection.py` is the compiler stand-in: reflected types without
  `GENERATED_BODY()`, headers that declare reflected types without including
  their own `generated.h`, translation units that do not include their own header
  first, and `LT_LOG`/`UE_LOG` calls whose format specifiers and arguments
  disagree.
- **External Xcode** on `/Volumes/DriveSohaib` must be mounted to compile.
- **Pending: move the engine to the external drive**, `drive-migration.md`. Do it
  with the editor fully closed. Frees about 43 GiB.
- **`LastTrain.uproject` no longer lists the `NeoStackAI` plugin.** The folder is
  gitignored and the trial has expired, so the entry only produced a
  missing-plugin prompt for anyone cloning. `CommonUI` must stay: NeoStack once
  enabled it as a dependency without its modules built, which SIGSEGV'd PIE.
- The editor rewrites `LastTrain.uproject` on launch, reformatting it and
  re-adding plugin entries. Check `git diff LastTrain.uproject` before committing
  after an editor session.
- **Never mutate actors or assets while PIE is running**, and stop PIE with
  `playtest.stop` or Escape rather than closing the window.
  `editor-crash-endplaymap.md` has the write up.

## Content in git

Our own assets under `Content/LastTrain/` are committed via Git LFS per
`.gitattributes`. Imported third-party packs are gitignored by their landing
folders, because UE-Only Content and Fab Standard licences forbid re-hosting raw
assets in a public repo. Vetted free asset list: `docs/reference/free-assets.md`.
Canary Wharf reference research: `docs/reference/canary-wharf-research/`. If a
pack imports to a new folder, add it to `.gitignore` before committing.

## The docs, and what each one is for

- `README.md` here - the phase plan and the per-task status tables.
- `handover.md` - this file. The resume point.
- `neostack.md` - every outstanding editor task, with the numbers.
- `phase-*.md` - one bounded spec each, with its acceptance list. Every one is
  now implemented in code; the acceptance lists are what remain live.
- `phase-c-review-2026-09-07.md` - the code-only review pass: the special rounds
  diff against its spec, the open findings above, the corridor stall verdict and
  the zombie stat block cross-check.
- `drive-migration.md` - the pending engine move.
- `editor-crash-endplaymap.md` - the PIE teardown crash and the rules that avoid
  it.

Design canon is `docs/design/gameplay-canon.md`, and it wins over any spec that
contradicts it. The old 167-item open-questions sweep is in git history at
`0b7e35f`.

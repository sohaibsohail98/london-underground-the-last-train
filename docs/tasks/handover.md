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
work: five data assets, a material, two Blueprints and a reparent. That brief is
`neostack.md`.

The 2026-09-07 editor pass (`bf1878c`, log in `neostack-run-2026-09-07.md`)
closed two things this file used to list: every spawn point in both maps is
placed and verified, and the corridor stall fix now has a verdict. It also found
that `L_GreyboxTest`'s five points had never been placed either, which is why the
crowd always converged on world origin.

## Where the work is

All of it is on `main`. The Phase C and E1 C++ merged in `c4a6369` (from
`64535f6`, `b1424bf` and `ace4b14`), and `bf1878c` is a NeoStack editor pass on
top of it. All five CI gates pass on `main`.

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
5. **Measure the Phase B crowd gate, by hand.** It cannot be done over the
   NeoStack bridge: the editor throttles its tick while the window is unfocused,
   so every delta sample came back at exactly 0.33333s (3 fps) regardless of
   load, which would be a meaningless reading against a 60 fps gate. It needs
   `stat unit` read on screen with the editor window focused, or a packaged
   build. Set the placed `GreyboxTest_RoundManager`'s `OpeningRoundCounts` index
   0 to 30, measure with 24 or more alive, then put it back to 6. This is the
   last thing between Phase B and done.
6. **Run the acceptance lists** in the table below.
7. **Mark Phase B done** in `README.md` and here once 5 and the B2 and B3 items
   pass.

## What has landed

### Phase A, done 2026-09-04

`L_GreyboxTest` plays end to end. The A5 acceptance passed on every testable
check. Records: `phase-a4-editor-setup.md`, `phase-a5-acceptance.md`.

### Phase B, code complete, one measurement short

B1 throttled repath, B2 interaction and the first wall buy, B3 the HUD widget.
The zombie attack fix and the HUD health bar are verified in PIE. The 2026-09-07
pass cleared the corridor stall question: across three runs no zombie stayed
frozen more than 2 seconds outside melee range while the crowd was genuinely
pathing, and the apparent freezes were outer ranks queued behind a pile, every
one of which resumed within 2 seconds once the player moved. One anomaly is
flagged rather than called a defect: in a dense run with a dead player,
`BP_Zombie_C_19` logged `entering stall recovery` 122 times at 1 Hz for three
minutes while the other nineteen logged 6 to 12 times each. It did not reproduce
in a clean run. **Open**: the 24 to 40 crowd frame gate (see action 5), the
wall-buy flow, and the HUD polish review.

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
  banner. Two deliberate deviations from that spec, both because
  `docs/design/gameplay-canon.md` lines 218 to 223 say otherwise: the brute pair
  lands at roughly 30 and 70 per cent through the round rather than as a group up
  front, and a round that is both (20, 30) is a sprinter round that also carries
  the pair, rather than brutes taking precedence.

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
| Crowd frame gate, 24 to 40 at 60fps | `phase-b1-throttled-repath.md` | PIE with the editor window **focused**, or a packaged build |
| Wall buy prompt and purchase | `phase-b2-interaction.md` steps 3 to 7 | a human at the keyboard |
| HUD polish: hit marker, spread, prompt anchor | `phase-b3-feedback-widgets.md` | PIE |
| Train, 12 points | `phase-c1-train.md` | `BP_Train` |
| Live countdown on a sign | `phase-c2-departure-board.md` | `BP_DepartureBoard` |
| Board and arrive with the carry | `phase-c3-travel.md` | reparent plus routes |
| Zombie types, 11 points | `phase-c-zombie-types.md` | five assets, material, roster |
| Sprinter round 5, brutes on 10, both on 20 | `phase-c-special-rounds.md` | roster |
| Down, bleed out, auto revive | `phase-e1-downed-revive.md` | PIE only |

## Open code items

Small, none of them specced, all real.

1. **The corridor stall recovery, now only an anomaly.** The behaviour itself
   tested clean on 2026-09-07 (see Phase B above), so this is no longer a
   suspected defect. What is left is the 1 Hz re-entry pattern one zombie showed
   for three minutes in a dense run with a dead player: `BeginStallRecovery` and
   `EndStallRecovery` may be able to alternate every frame at the attack
   position, which would also explain the four re-entries inside 1.6 seconds seen
   at melee range on Canary Wharf. Worth reading the gate order in
   `UpdateStallRecovery` against a stationary target before the art pass makes it
   audible.
2. **`NavProjectionExtent` is too generous on Z** (500). It let
   `ProjectPointToNavigation` "succeed" at world origin, which is precisely what
   hid the Canary Wharf bug: the "not near the navmesh" warning never fired.
   Tighten it, or warn when the projected point moves an implausible distance.
3. **Arrival flashes the points HUD as a spend.** `RehydrateFromTravel` grants
   the carry with `AddPoints(Carried - Current)`, and the HUD renders a negative
   delta crimson, so arriving under the 500 seed reads as a purchase. The total
   is right. Wants a `SetPoints(int32)` on `ULTPointsComponent` that assigns and
   broadcasts a zero delta; C3 put that component out of scope.
4. **A downed player still gets interaction prompts.** `Interact` is gated, but
   `ULTInteractionComponent` keeps sweeping and broadcasting, so "Board train"
   can sit on screen through the bleed-out with `E` inert. Wants a
   `SetInteractionEnabled(bool)` on the component, called from `Down` and
   `Revive`; E1 put that component out of scope, and disabling its tick from
   outside would freeze the last prompt on screen instead of clearing it.
5. **Solo death is unreachable on the shipped defaults.** Auto-revive at 8s
   against a 30s bleed-out, damage ignored while down, no cap on repeated downs.
   That is exactly what E1 specified, but Phase E's own gate ("a full survival
   session start to death is possible") cannot be met until a revive item, a
   per-run cap, or a cleared `bSoloAutoRevive` lands. Clearing the flag on
   `BP_PlayerCharacter` is the one-click version.
6. **`ALTGameState::SetStationName` has no callers.** Its comment says "set on
   travel in", travel is now built, and nothing sets it, so a station label reads
   blank. Either stamp it from an `EditDefaultsOnly` name on the game mode, or
   delete the setter.
7. **`ALTTrain`'s class comment still says travel is a later task.** C2 and C3
   both forbid touching `LTTrain.{h,cpp}`, so it was left. One line.
8. **`ALTPlayerCharacter::TakeDamage` subtracts the raw damage**, not the value
   `Super::TakeDamage` returns, so a damage modifier is reported to the caller
   and ignored for health. Pre-existing, unrelated to E1, still wrong.

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
- `neostack-run-2026-09-07.md` - the editor run that placed the spawn points and
  read the stall recovery. The only dated log still here: fold it into this file
  and `neostack.md` and delete it once its open items close.
- `drive-migration.md` - the pending engine move.
- `editor-crash-endplaymap.md` - the PIE teardown crash and the rules that avoid
  it.

Design canon is `docs/design/gameplay-canon.md`, and it wins over any spec that
contradicts it. The old 167-item open-questions sweep is in git history at
`0b7e35f`.

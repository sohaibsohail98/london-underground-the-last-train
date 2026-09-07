# Phase E1 - downed and revive

**Engine:** Unreal Engine 5.8, macOS. Xcode on `/Volumes/DriveSohaib` must be
mounted (`xcode-select -p`). If not, STOP.

**Build command** (repo root):
```
"/Users/Shared/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"
```
`-Werror` on, no CI compile, compile locally. `clang-format` 20 on every touched
`.h`/`.cpp`.

**Model:** Opus. C++ against the existing convention.

**Prerequisite:** Phase B landed. `ALTPlayerCharacter` has health with a 4s
delayed regen, `OnHealthChanged` (float fraction) and the
`BlueprintImplementableEvent`s `OnDamageTaken(float)` and `OnDied()`.
`ALTGameMode` has `NotifyPlayerDied()`, `NotifyPlayerDowned()` and
`NotifyPlayerRevived()`, and `ELTRunState` already has `Downed` ("Rounds
continue. A revive returns to Active."). The comment in `LTGameState.h` says the
mechanism is designed; the wiring is not built - `NotifyPlayerDowned` today only
calls `SetState(Downed)` and nothing puts the player into that state.

## The problem

`docs/design/gameplay-canon.md` section 3 and `brief-v2.md` describe a downed
state: at zero health the player does not die outright, they go down, immobile,
with a bleed-out timer and a last-stand pistol; a nearby friendly interact (solo:
a self-revive item, or simply the bleed-out timer for a solo run) brings them
back. Death is only when the bleed-out timer expires. None of that exists.
`ALTPlayerCharacter` currently must be calling `NotifyPlayerDied()` straight from
zero health (check `Private/Player/LTPlayerCharacter.cpp` around the damage
handler and confirm), so there is no down, no bleed-out, no revive.

This task builds the solo downed-and-revive loop in C++: zero health puts the
player in `Downed`, movement and firing are locked to a reduced set, a bleed-out
timer counts, expiry calls `NotifyPlayerDied`, and a revive path (a timer-based
auto-revive for v1, with the seam for an interact-driven revive) returns to
`Active` with partial health. Co-op revive by a second player is out of scope.

## Scope

**In scope**
- `ALTPlayerCharacter`: a `Down()` path taken instead of `Die()` when health
  first reaches zero. It:
  - sets an internal `bDowned`, stops regen,
  - locks movement: zero walk speed, or `DisableMovement()`, your call; the
    player can still turn the camera,
  - blocks `StartFire` / `StartAim` / `Reload` / `Interact` for the normal
    weapon, or swaps to a "last stand" state (v1: simplest is block fire
    entirely and rely on the timer; expose the hook for a last-stand weapon),
  - starts a `BleedOutSeconds` countdown (`EditDefaultsOnly`, default 30),
  - calls `GetWorld()->GetAuthGameMode<ALTGameMode>()->NotifyPlayerDowned()`.
  - fires a `BlueprintImplementableEvent OnDowned()`.
- A bleed-out tick: while `bDowned`, count `BleedOutSeconds` down each `Tick`. On
  expiry, call the existing `Die()` path (which already broadcasts and calls
  `NotifyPlayerDied`). Fire `OnBleedOutExpired()` just before.
- A revive path: `void Revive()` (BlueprintCallable) - clears `bDowned`, restores
  movement and input, sets health to `ReviveHealthFraction * MaxHealth`
  (`EditDefaultsOnly`, default 0.5), restarts regen eligibility, calls
  `NotifyPlayerRevived()`, fires `OnRevived()`.
- v1 auto-revive: an `EditDefaultsOnly bool bSoloAutoRevive = true` and
  `SoloReviveDelaySeconds` (default 8). While `bDowned`, if `bSoloAutoRevive`,
  after `SoloReviveDelaySeconds` of being down, call `Revive()` automatically -
  UNLESS bleed-out already expired. This gives a solo run a survivable loop
  without a revive item asset. The seam for a real interact-driven or item-driven
  revive is `Revive()` itself, called from elsewhere.
- Guard everything so a second down while already downed, or a revive while not
  downed, is a no-op.

**Out of scope**
- Co-op, a second player, reviving another pawn. Solo only.
- A self-revive item as an inventory asset, a wall-buy for it, the economy of it.
  `Revive()` is the seam; wiring an item to it is a later task.
- A last-stand pistol as a real weapon swap with its own `ULTWeaponData`. Expose
  `OnDowned` / `OnRevived` so a Blueprint could swap a view model, but do not
  build the weapon.
- The downed screen desaturation, vignette, the "get up" prompt UI. Blueprint and
  the art/HUD pass. This task fires the events; the look is not its job.
- Any change to `ALTGameMode` beyond confirming the three `NotifyPlayer*` hooks
  are called at the right moments (they already exist; you are calling them, not
  changing them). If `NotifyPlayerDowned` / `NotifyPlayerRevived` need a body
  change, STOP and report.

## The change

Edit only:
- `Source/LastTrain/Public/Player/LTPlayerCharacter.h`
- `Source/LastTrain/Private/Player/LTPlayerCharacter.cpp`

Add, private state:
```cpp
bool bDowned = false;
float BleedOutRemaining = 0.f;
float SoloReviveRemaining = 0.f;
```

Add, `EditDefaultsOnly, BlueprintReadOnly, Category = "Downed"`:
```cpp
float BleedOutSeconds = 30.f;
float ReviveHealthFraction = 0.5f;
bool bSoloAutoRevive = true;
float SoloReviveDelaySeconds = 8.f;
```

Add, methods:
```cpp
public:
	UFUNCTION(BlueprintPure, Category = "Downed")
	bool IsDowned() const { return bDowned; }

	/** Seconds left before bleed-out kills the player. 0 when not downed. */
	UFUNCTION(BlueprintPure, Category = "Downed")
	float GetBleedOutRemaining() const { return bDowned ? BleedOutRemaining : 0.f; }

	/** Bring the player back up at ReviveHealthFraction health. No-op if not
		downed. The seam for an item or a co-op revive. */
	UFUNCTION(BlueprintCallable, Category = "Downed")
	void Revive();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Downed")
	void OnDowned();

	UFUNCTION(BlueprintImplementableEvent, Category = "Downed")
	void OnRevived();

	UFUNCTION(BlueprintImplementableEvent, Category = "Downed")
	void OnBleedOutExpired();

private:
	void Down();
```

In the damage handler where health hits zero: today it likely calls a death
path directly. Change it to: if `!bDowned`, call `Down()`; the existing death
call moves into the bleed-out-expiry branch of `Tick`. Keep the existing
`OnDied()` event and `NotifyPlayerDied()` call on that expiry path, do not
duplicate them.

`Down()`:
- `if (bDowned) return;`
- `bDowned = true;`
- stop regen: whatever the existing 4s-delay regen uses (a timer handle or a
  countdown), cancel or freeze it.
- lock movement: `GetCharacterMovement()->DisableMovement();` (revive restores
  with `SetMovementMode(MOVE_Walking)`).
- `BleedOutRemaining = BleedOutSeconds;`
- `SoloReviveRemaining = SoloReviveDelaySeconds;`
- `if (ALTGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ALTGameMode>() : nullptr) { GM->NotifyPlayerDowned(); }`
- `OnDowned();`
- `LT_LOG(Log, TEXT("Player downed. Bleed-out in %.0fs."), BleedOutSeconds);`

`Tick`, add a block, only when `bDowned`:
```cpp
BleedOutRemaining -= DeltaSeconds;
if (BleedOutRemaining <= 0.f)
{
	OnBleedOutExpired();
	<the existing death path: set health 0, call OnDied(), NotifyPlayerDied()>;
	bDowned = false;
	return;
}
if (bSoloAutoRevive)
{
	SoloReviveRemaining -= DeltaSeconds;
	if (SoloReviveRemaining <= 0.f)
	{
		Revive();
	}
}
```

`Revive()`:
- `if (!bDowned) return;`
- `bDowned = false;`
- restore movement: `GetCharacterMovement()->SetMovementMode(MOVE_Walking);`
- set health: `Health = ReviveHealthFraction * MaxHealth;` then broadcast
  `OnHealthChanged` the same way the damage path does, so the HUD bar updates.
- re-enable regen eligibility (reset the delay countdown so regen can resume
  after the normal 4s).
- `if (ALTGameMode* GM = ...) { GM->NotifyPlayerRevived(); }`
- `OnRevived();`
- `LT_LOG(Log, TEXT("Player revived at %.0f%% health."), ReviveHealthFraction * 100.f);`

Block player input while downed: in `Move`, `StartFire`, `StartAim`, `Reload`,
`Interact`, `StartSprint`, add `if (bDowned) { return; }` at the top. `Look`
stays free so the player can watch the room while down.

## Constraints

- Two files only: `LTPlayerCharacter.{h,cpp}`. No new files, no change to
  `ALTGameMode`, `ALTGameState`, `ULTWeaponComponent`, or anything else. If the
  task appears to need one, STOP and report.
- British spelling everywhere including comments and log strings. No em or en
  dashes.
- `#pragma once` first, `*.generated.h` last. `LT_LOG` not `UE_LOG`. `TObjectPtr`
  for any new `UObject` member (there should be none).
- No `TODO`/`FIXME`/`HACK`/`XXX` markers.
- `LT` prefix, tab indent, clang-format 20.
- Do NOT run git. Leave the tree for review.

## Acceptance

Static:
1. Compiles clean, zero warnings under `-Werror`.
2. `clang-format -i` clean on both files.
3. All four `tools/ci/` checkers pass.
4. Header review: guards, generated.h last, no `UE_LOG`, British spelling, no
   dashes, no markers.
5. The damage-at-zero-health path calls `Down()` on the first hit and only
   reaches the death path via bleed-out expiry - confirm by reading the diff,
   there must be no path that calls the death code directly from the damage
   handler any more.

Runtime (editor task, later, note it): in PIE, take damage to zero - the player
should go down, not die: immobile, camera still free, a bleed-out countdown
readable via `GetBleedOutRemaining()`. With `bSoloAutoRevive` true, after
`SoloReviveDelaySeconds` the player stands back up at half health. With it false,
after `BleedOutSeconds` the run ends (`OnDied`, run state `Dead`).

## On pass

Update `docs/tasks/NEXT.md` and add a Phase E row entry in
`docs/tasks/README.md`. Note the seams left open: `Revive()` for an item or co-op
revive, `OnDowned` / `OnRevived` for a last-stand weapon swap, and the downed
screen treatment for the HUD/art pass.

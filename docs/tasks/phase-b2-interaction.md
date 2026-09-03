# Phase B2 — interaction system and the first wall buy

**Engine:** Unreal Engine 5.8, macOS, external Xcode on `/Volumes/DriveSohaib`
mounted. Compile after the change with the batch build in `CLAUDE.md`.

**Model:** Opus or Sonnet. This is C++ against an existing convention with a
fully specified design, so Sonnet is viable here, but read the constraints
section before starting and match the surrounding files exactly rather than
writing idiomatic UE from memory.

**Prerequisite:** Phase A passed and B1 landed. `L_GreyboxTest` plays.

## The problem

`Source/LastTrain/Public/Interaction/LTInteractableInterface.h` declares three
`BlueprintNativeEvent` functions and nothing implements or calls them. There is
no way to look at a thing and buy it, which blocks wall buys, doors, perk
machines, the bench, lost property and boarding the train. Everything in Phases C
and E depends on this one path existing.

The player also has no `InteractAction`. Its input actions stop at `ReloadAction`.

## The change

Three pieces. Build them in this order and compile between each.

### 1. `ULTInteractionComponent`

New files:

- `Source/LastTrain/Public/Interaction/LTInteractionComponent.h`
- `Source/LastTrain/Private/Interaction/LTInteractionComponent.cpp`

An `UActorComponent` that ticks, traces for an interactable in front of the
owner, and holds the current one.

Public surface:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractableChanged, const FText&, Prompt, bool, bAvailable);
```

- `FOnInteractableChanged OnInteractableChanged`, `BlueprintAssignable`. The HUD
  binds to this. Broadcast only when the prompt text or availability actually
  changes, not every tick.
- `float InteractionRange = 250.f`, `EditDefaultsOnly`, category `Interaction`.
- `float TraceRadius = 12.f`, `EditDefaultsOnly`. A sphere sweep rather than a
  line, so small anchors are not fiddly to look at.
- `void TryInteract()`, `BlueprintCallable`. Calls
  `ILTInteractableInterface::Execute_Interact` on the current target if there is
  one and it still passes `CanInteract`.
- `AActor* GetCurrentInteractable() const`, `BlueprintPure`.

Tick behaviour:

1. Get the view point using the same pattern as
   `ULTWeaponComponent::GetViewPoint`: `GetActorEyesViewPoint` on the owning
   pawn, falling back to actor location and forward vector. Copy that pattern,
   do not invent a new one.
2. `SweepSingleByChannel` on `ECC_Visibility` with
   `FCollisionShape::MakeSphere(TraceRadius)`, from the view point to
   `Origin + Direction * InteractionRange`. Use
   `FCollisionQueryParams Params(SCENE_QUERY_STAT(LTInteractionTrace), false, GetOwner());`
   to match the weapon component's style.
3. If the hit actor implements `ULTInteractableInterface` and
   `Execute_CanInteract(HitActor, GetOwner())` is true, that is the current
   target. Otherwise the current target is null.
4. Recompute the prompt with `Execute_GetInteractionPrompt` and broadcast only on
   change. Compare with `FText::IdenticalTo` or by comparing
   `ToString()`, and also broadcast when availability flips.

Use `ECC_Visibility`, not a new channel. `ECC_GameTraceChannel1` is the weapon
channel and adding a second custom channel means another editor step.

Set `PrimaryComponentTick.bCanEverTick = true` in the constructor.

### 2. Wire it to the player

`Source/LastTrain/Public/Player/LTPlayerCharacter.h` and its `.cpp`.

- Forward declare `class ULTInteractionComponent;`.
- Add a `TObjectPtr<ULTInteractionComponent> Interaction` alongside `Weapon` and
  `Points`, `VisibleAnywhere, BlueprintReadOnly`, category `Interaction`.
  Create it in the constructor with `CreateDefaultSubobject` exactly as `Weapon`
  and `Points` are created.
- Add a `BlueprintPure` getter `GetInteraction()` matching `GetWeapon()`.
- Add `TObjectPtr<UInputAction> InteractAction`, `EditDefaultsOnly`, category
  `Input`, after `ReloadAction`.
- Add `protected: void Interact();` next to `Reload()`. It calls
  `Interaction->TryInteract()` with a null guard.
- Bind it in `SetupPlayerInputComponent` with `ETriggerEvent::Started`, matching
  how `ReloadAction` is bound. Guard the binding on the action being non null,
  the same way the existing bindings are guarded.

### 3. `ALTWallBuy`, the first concrete interactable

New files:

- `Source/LastTrain/Public/Interaction/LTWallBuy.h`
- `Source/LastTrain/Private/Interaction/LTWallBuy.cpp`

```cpp
class LASTTRAIN_API ALTWallBuy : public AActor, public ILTInteractableInterface
```

Components: a `USceneComponent` root and a `UStaticMeshComponent` named `Plate`.
The plate is the visual and the trace target, so leave its collision at the
default block-visibility setup.

Properties, all `EditAnywhere`, category `Wall buy`:

- `TObjectPtr<ULTWeaponData> Weapon`
- `int32 WeaponCost = 500`
- `int32 AmmunitionCost = 250`

Override the three implementations:

- `CanInteract_Implementation`: true when `Weapon` is set and the interactor has
  a `ULTPointsComponent`. Affordability is deliberately not checked here, so the
  prompt still shows when the player cannot yet afford it. That is how the
  player learns the price.
- `GetInteractionPrompt_Implementation`: if the interactor's weapon component
  already holds this `ULTWeaponData`, return an ammunition prompt at
  `AmmunitionCost`, otherwise a purchase prompt at `WeaponCost`. Use
  `FText::Format` with `NSLOCTEXT`, and read the display name from
  `ULTWeaponData` rather than the actor label. British spelling: "Ammunition",
  not "Ammo", in user facing text.
- `Interact_Implementation`: resolve the interactor's `ULTPointsComponent` and
  `ULTWeaponComponent`. If either is missing, return. If the held weapon is
  already this one, `TrySpend(AmmunitionCost)` and on success call
  `RefillAmmunition()`. Otherwise `TrySpend(WeaponCost)` and on success call
  `SetWeapon(Weapon, true)`. `TrySpend` already refuses when unaffordable and
  returns false, so never test affordability separately before calling it.

Add a `BlueprintImplementableEvent OnPurchased()` so a Blueprint can drive audio
and a flash later. Call it after a successful spend.

## Constraints

- British spelling, no em or en dashes, `LT_LOG` not `UE_LOG`, tab indent,
  `#pragma once` first, `X.generated.h` last include. Match the surrounding
  files.
- `TObjectPtr` for every `UObject` member, never a raw pointer.
- No `TODO`, `FIXME` or `HACK` markers. `tools/ci/check_hygiene.py` rejects them.
- Interface calls from C++ go through the generated `Execute_` statics, for
  example `ILTInteractableInterface::Execute_Interact(Target, GetOwner())`.
  Calling the virtual directly will compile and then silently skip any
  Blueprint override.
- Do not modify `LTWeaponComponent`, `LTPointsComponent`, `LTZombieCharacter` or
  the round manager. If this task appears to need a change in one of those,
  stop and say so rather than making it.
- Keep the diff to the six files listed above plus the two player files.

## Acceptance

1. Compiles clean with the batch build.
2. In the editor: add an `Interact` Input Action, bind it to `E` in the input
   mapping context, and assign it on the player Blueprint. Place an `ALTWallBuy`
   in `L_GreyboxTest` with a cube mesh on the plate and the existing weapon data
   asset assigned.
3. Walking up to the plate and looking at it fires `OnInteractableChanged` with a
   prompt naming the weapon and its price. Looking away fires it again with
   `bAvailable` false. Verify with a temporary print node bound to the delegate.
4. Pressing `E` with fewer points than `WeaponCost` does nothing and the point
   total is unchanged.
5. Pressing `E` with enough points deducts exactly `WeaponCost` once and the held
   weapon changes.
6. Pressing `E` again on the same plate now offers ammunition at
   `AmmunitionCost` and refills the reserve rather than rebuying.
7. The trace does not pick up zombies, the floor or the player's own capsule.

## On pass

Update `docs/tasks/NEXT.md`. The remaining Phase B work is the hit marker and
prompt widget in `docs/tasks/phase-b3-feedback-widgets.md`, which is editor work
and needs no C++.

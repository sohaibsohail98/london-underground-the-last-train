# Phase C — the five zombie types

**Engine:** Unreal Engine 5.8, macOS, external Xcode on `/Volumes/DriveSohaib`
mounted. Compile after each step with the batch build in `CLAUDE.md`.

**Model:** Opus. C++ against the existing convention plus a new `UPrimaryDataAsset`
schema. Read the constraints and the decision markers before starting: this
task defines the STRUCTURE that holds the type numbers, it does not lock the
numbers. Every actual value is a `[DECISION NEEDED]` marker carrying the
`docs/design/open-questions.md` section 3 proposal as its default.

**Prerequisite:** Phase B landed. `L_GreyboxTest` plays, the round manager spawns
one `ZombieClass` on a jittered repath, wall buys work. The two open Phase B PIE
findings (zombie attack lands nothing, spawn count plateau) are not blockers for
this structure work but must be closed before the acceptance pass here.

## The problem

`brief-v2.md` names five zombie types (walker, sprinter, brute, crawler,
screamer) and gives exactly one rule about them: "Sprinter horde every 5th
round, brute pair every 10th". There is no per-type health, speed, damage, size,
hitbox, special ability, spawn weight, first-appearance round or death behaviour
anywhere in the code or the docs.

`Source/LastTrain/` has one `ALTZombieCharacter` with a single flat stat block
(`BaseHealth = 150`, `BaseWalkSpeed = 130`, `AttackDamage = 24`,
`AttackCooldownSeconds = 1.3`, `AttackRange = 130`), no subclasses and no data
asset. `ALTRoundManager::ZombieClass` is a single `TSubclassOf<ALTZombieCharacter>`
and `TrySpawnOne` spawns exactly that class every time. Nothing varies.

`docs/design/open-questions.md` section 3 flags the whole subsystem as almost
entirely **PROPOSAL - needs sign-off**: the mechanism (a data asset) is a clean
confirmation, but every number in the stat table is invented and needs an actual
design decision. This task builds the mechanism and carries the numbers as
markers.

The brief is explicit that there is **one rigged humanoid zombie mesh**. The
five types are distinguished by scale, colour tint, animation play rate and
behaviour, not by five separate models. Do not propose five bespoke characters.

## The change

Four pieces. Build them in this order and compile between each.

### 1. `ULTZombieTypeData`

New files:

- `Source/LastTrain/Public/Zombies/LTZombieTypeData.h`
- `Source/LastTrain/Private/Zombies/LTZombieTypeData.cpp` (thin, or omit if the
  header carries no function bodies; `ULTWeaponData` keeps its one helper inline
  in the header, so header-only is consistent if there is nothing to define)

A `UPrimaryDataAsset`, `BlueprintType`, mirroring `ULTWeaponData` exactly:
`#include "Engine/DataAsset.h"`, `X.generated.h` last, every field
`EditDefaultsOnly, BlueprintReadOnly` with a `Category`. One asset per type,
authored in the editor under `Content/LastTrain/Zombies/`.

Introduce the type enum in this header:

```cpp
UENUM(BlueprintType)
enum class ELTZombieType : uint8
{
	Walker,
	Sprinter,
	Brute,
	Crawler,
	Screamer
};
```

And a behaviour enum for the per-type special, so the character switches on an
explicit value rather than on the type id (a later type could reuse a
behaviour):

```cpp
UENUM(BlueprintType)
enum class ELTZombieBehaviour : uint8
{
	/** Walker. No special. */
	None,
	/** Sprinter. Fast, tighter gait band, optional short lunge on the attack wind-up. */
	Sprint,
	/** Brute. Front plate absorbs body shots until broken, then normal damage. */
	ArmourPlate,
	/** Crawler. Prone, low capsule, head only reachable by a crouched player. */
	LowProfile,
	/** Screamer. On sustained line of sight, calls an extra wave. */
	Scream
};
```

Fields on `ULTZombieTypeData`, grouped by category:

**Identity**

| Field | Type | Notes |
|---|---|---|
| `Type` | `ELTZombieType` | The id. Drives roster logging and the special-round rules. |
| `DisplayName` | `FText` | Original, generic. "Walker", "Sprinter" etc are fine, they are not CoD-specific. |
| `Behaviour` | `ELTZombieBehaviour` | Which special hook the character runs. |

**Stats (multipliers and absolutes against the `ALTZombieCharacter` base)**

| Field | Type | Applied as | Marker |
|---|---|---|---|
| `HealthMultiplier` | `float` | `Health` and `ApplyRoundScaling` both multiply `BaseHealth` by this | see 3.1 markers |
| `WalkSpeedMultiplier` | `float` | multiplies `BaseWalkSpeed` before the `SpeedStepRounds` steps are added | see 3.1 markers |
| `AttackDamageOverride` | `float` | if `> 0`, replaces `AttackDamage`; `0` keeps the base | see 3.1 markers |
| `AttackCooldownOverride` | `float` | if `> 0`, replaces `AttackCooldownSeconds` | see 3.1 markers |
| `AttackRangeOverride` | `float` | if `> 0`, replaces `AttackRange` | see 3.4 markers |

Use overrides that fall through to the base on `0` rather than multipliers for
damage, cooldown and range: the walker asset then carries `0` for all three and
stays identical to today's coded default, which is the stated safe baseline.
Health and speed are multipliers because every type moves off the same
round-scaled curve.

**Presentation (the one mesh, varied)**

| Field | Type | Notes |
|---|---|---|
| `MeshScale` | `float` | uniform scale applied to the character mesh component on spawn | see 3.1 markers |
| `ColourTint` | `FLinearColor` | passed to the mesh as a vector parameter named `TintColour`. See the crowd-cost marker below: this MUST go through a shared material with an instance-per-type, NOT a per-instance dynamic material |
| `AnimPlayRate` | `float` | multiplier stored on the character and read by the anim Blueprint (`GetAnimPlayRate()`), so the single locomotion set plays faster or slower per type | see 3.1 / 2.2 markers |

**Navigation and collision (per-type, tuned separately from the walker)**

| Field | Type | Notes | Marker |
|---|---|---|---|
| `RepathIntervalOverride` | `float` | if `> 0`, replaces `RepathIntervalSeconds` (sprinter wants a shorter cadence, crawler a longer one) | see 3.4 |
| `CapsuleHalfHeightOverride` | `float` | if `> 0`, resize the capsule on spawn. The constructor does NOT currently resize the capsule, so this is a new per-type step | see 3.4 |
| `CapsuleRadiusOverride` | `float` | if `> 0`, resize the capsule radius on spawn | see 3.4 |
| `AvoidanceConsiderationRadiusOverride` | `float` | if `> 0`, replaces the `45` set in the constructor (brute wants wider) | see 3.4 |
| `ContactRangeOverride` | `float` | if `> 0`, replaces `ContactRange` (a fast agent overshoots a `15` acceptance point on a short repath) | see 3.4 |

**Roster**

| Field | Type | Notes |
|---|---|---|
| `SpawnWeightNormalRound` | `float` | relative weight in a normal round's composition. `0` means never on a normal round (sprinter, brute) | see 3.1 markers |
| `FirstRoundAvailable` | `int32` | earliest round this type may spawn at all | see 3.1 markers |
| `HighHeatWeightMultiplier` | `float` | from heat 3+, multiply `SpawnWeightNormalRound` by this (5.1: sprinter, screamer, crawler weights roughly double at high heat) | see 5.1, default `2.0` for those three, `1.0` for walker and brute |
| `MaxAliveOfThisType` | `int32` | if `> 0`, never more than this many of this type alive at once (screamer is capped at 1). `0` means no per-type cap | see 3.1 markers |

**Behaviour parameters (only the ones the C++ hook reads; everything numeric is a marker)**

| Field | Type | Notes | Marker |
|---|---|---|---|
| `SprintLungeImpulse` | `float` | `Sprint` behaviour: forward impulse on the attack wind-up. `0` disables the lunge | see 3.1 (whether the sprinter lunges) |
| `ArmourBodyDamageToBreak` | `float` | `ArmourPlate` behaviour: accumulated body damage before the plate breaks and normal damage applies. Headshots and rear hits always bypass | see 3.1 (brute) |
| `ScreamLineOfSightSeconds` | `float` | `Scream` behaviour: continuous clear line of sight to the player camera before the scream fires | see 3.1 (screamer trigger) |
| `ScreamSummonCount` | `int32` | `Scream` behaviour: how many extra walkers the scream requests from the round manager | see 3.1 (screamer effect) |
| `ScreamCancelWindowSeconds` | `float` | `Scream` behaviour: if the screamer dies within this long of the scream starting, the summoned wave is cancelled | see 3.3 (screamer death) |

**Death**

| Field | Type | Notes | Marker |
|---|---|---|---|
| `CorpseLifetimeOverride` | `float` | if `> 0`, replaces `CorpseLifetime` (the brute is a landmark, wants longer) | see 3.3 |
| `bRagdollOnDeath` | `bool` | walker/sprinter snap to a settled pose, the crawler is already prone; a Blueprint reads this in `OnDeathPresentation` | see 3.3 |
| `DeathScreenShakeRadius` | `float` | `> 0` asks the death Blueprint for a small shake within this radius (the brute falling) | see 3.3 |

Add one helper, inline in the header like `ULTWeaponData::GetShotInterval`:

```cpp
/** True when this type is a special that only appears on its own special round
    or when explicitly requested, never in a normal round's weighted mix. */
UFUNCTION(BlueprintPure, Category = "Roster")
bool IsSpecialOnly() const { return SpawnWeightNormalRound <= 0.f; }
```

### 2. `ALTZombieCharacter::ApplyTypeData`

Changes to `Source/LastTrain/Public/Zombies/LTZombieCharacter.h` and its `.cpp`.
Keep the diff tight and keep every existing property and function exactly as it
is; `ApplyTypeData` layers on top.

Header additions:

- `#include "Zombies/LTZombieTypeData.h"` is not needed in the header; forward
  declare `class ULTZombieTypeData;` and `enum class ELTZombieType : uint8;` and
  include the data header in the `.cpp`. (`ELTZombieBehaviour` is only used in
  the `.cpp` switch.)
- A public accessor and the applied state:

```cpp
/** Applies every property of the type asset: stats, scale, tint, capsule,
    navigation overrides and the behaviour hook. Called on spawn straight after
    ApplyRoundScaling, so the round-scaled Health and MaxWalkSpeed are already
    set and this multiplies or replaces them. Null is a no-op, leaving a plain
    walker on the coded defaults. */
UFUNCTION(BlueprintCallable, Category = "Zombie")
void ApplyTypeData(const ULTZombieTypeData* Data);

UFUNCTION(BlueprintPure, Category = "Zombie")
ELTZombieType GetZombieType() const { return ZombieType; }

/** Anim play rate multiplier from the type asset. The anim Blueprint reads this
    to speed or slow the single shared locomotion set. */
UFUNCTION(BlueprintPure, Category = "Zombie")
float GetAnimPlayRate() const { return AnimPlayRate; }
```

- Private state, next to `RepathTimer`:

```cpp
UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Zombie", Transient,
	meta = (AllowPrivateAccess = "true"))
ELTZombieType ZombieType = ELTZombieType::Walker;

float AnimPlayRate = 1.f;

/** Kept so ApplyRoundScaling re-applies the type's health and speed multipliers
    on top of the round curve rather than losing them. */
float TypeHealthMultiplier = 1.f;
float TypeWalkSpeedMultiplier = 1.f;

/** ArmourPlate behaviour: body damage still to absorb before the plate breaks.
    Zero once broken or on a non-brute. */
float ArmourRemaining = 0.f;

/** Scream behaviour: seconds of continuous clear line of sight accumulated. */
float ScreamSightTimer = 0.f;
bool bHasScreamed = false;
```

- Two Blueprint hooks for the specials the C++ cannot present, next to
  `OnHitReaction` and `OnDeathPresentation`:

```cpp
/** Blueprint hook: the sprinter's short lunge, or any per-type attack tell. */
UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
void OnAttackWindUp();

/** Blueprint hook: the screamer's animation and audio. Fired once, when the
    line-of-sight timer completes. The summon request itself is done in C++. */
UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
void OnScream();
```

`.cpp` - `ApplyTypeData`:

1. Null-guard, early return.
2. `ZombieType = Data->Type;`
3. Store `TypeHealthMultiplier` and `TypeWalkSpeedMultiplier` from the asset.
4. `Health *= TypeHealthMultiplier;` (round scaling has already run, so this
   scales the round-correct value).
5. Movement: `MaxWalkSpeed *= TypeWalkSpeedMultiplier;` on the movement
   component. Apply `AvoidanceConsiderationRadiusOverride` if `> 0`.
6. Attack overrides: for each of `AttackDamageOverride`,
   `AttackCooldownOverride`, `AttackRangeOverride`, `RepathIntervalOverride`,
   `ContactRangeOverride`, replace the member only when the override is `> 0`.
7. Capsule: if `CapsuleHalfHeightOverride > 0` or `CapsuleRadiusOverride > 0`,
   call `GetCapsuleComponent()->SetCapsuleSize(...)`, keeping the un-overridden
   dimension at its current value. Re-seat the mesh so its feet stay on the
   capsule base (`GetMesh()->SetRelativeLocation` to `-HalfHeight` on Z, the
   Character default relationship).
8. Presentation: `GetMesh()->SetWorldScale3D(FVector(Data->MeshScale))`;
   `AnimPlayRate = Data->AnimPlayRate;`. For the tint, see the crowd-cost
   marker: set the vector parameter `TintColour` on the mesh's material, which
   MUST already be a per-type `UMaterialInstance` (or a `UMaterialInstanceDynamic`
   drawn from a pool). Do NOT call `CreateDynamicMaterialInstance` unconditionally
   here.
9. Behaviour: `switch (Data->Behaviour)` and seed the per-behaviour state:
   `ArmourPlate` sets `ArmourRemaining = Data->ArmourBodyDamageToBreak`;
   `Scream` resets `ScreamSightTimer` and `bHasScreamed`. `None`, `Sprint` and
   `LowProfile` need no seeding here (the sprint lunge is read on attack, the
   low profile is entirely capsule and pose).
10. `LT_LOG(Log, TEXT("Applied type %s: health %.0f, speed %.0f, scale %.2f"),
    *Data->DisplayName.ToString(), Health, MaxWalkSpeed, Data->MeshScale);`

`.cpp` - `ApplyRoundScaling` change: after it computes `Health` and
`MaxWalkSpeed`, multiply both by `TypeHealthMultiplier` and
`TypeWalkSpeedMultiplier`. These default to `1.f`, so a call before
`ApplyTypeData` is unaffected, and a re-scale later keeps the type's identity.

`.cpp` - `TryAttack` change: on the tick the range check passes and the cooldown
is set, call `OnAttackWindUp()`. If `ZombieType == ELTZombieType::Sprinter` and
the asset's `SprintLungeImpulse > 0`, add the forward impulse. Do NOT restructure
`TryAttack` into a wind-up state machine here: that is open-questions 2.5 and a
separate task. This spec only adds the hook call and the lunge. Coordinate with
the open Phase B "attack lands nothing" finding: fix that first.

`.cpp` - `ReceiveShot` change for `ArmourPlate`: before `Health -= Damage`, if
`ArmourRemaining > 0` and the hit is not a headshot and not a rear hit
(dot of shot direction and actor forward `> 0`, i.e. hitting the front),
subtract from `ArmourRemaining` instead of `Health`, clamp at `0`, fire
`OnHitReaction` with a "blocked" read left to the Blueprint, and return without
the impulse. Headshots and rear hits fall through to normal damage. Keep this
branch small; the exact `ArmourBodyDamageToBreak` is a marker.

`.cpp` - `Tick` change for `Scream`: if `ZombieType == ELTZombieType::Screamer`
and `!bHasScreamed`, run a line trace from the mesh head bone to the player
camera on `ECC_Visibility` (blocked by geometry, not by other zombies). While it
is clear, accumulate `ScreamSightTimer += DeltaSeconds`; while blocked, reset to
`0`. When it reaches `ScreamLineOfSightSeconds`, set `bHasScreamed = true`, call
`OnScream()`, and call a new delegate the round manager binds (below) to request
`ScreamSummonCount` extra walkers. No new trace channel: `ECC_Visibility` only.

`.cpp` - `Die` change: if the type asset set `CorpseLifetimeOverride > 0`, use
it for `SetLifeSpan`. If `ZombieType == ELTZombieType::Screamer` and
`bHasScreamed` was set less than `ScreamCancelWindowSeconds` ago, broadcast a
cancel on the summon delegate so the round manager drops the pending wave. Pass
`bRagdollOnDeath` and `DeathScreenShakeRadius` through to `OnDeathPresentation`
via two new params, or store them as members the Blueprint reads; keep whichever
matches the existing hook signature style with the least churn.

Add the summon delegate to the header, near `OnZombieDied`:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZombieScreamed, ALTZombieCharacter*, Screamer, int32, WalkerCount);

/** Fired when a screamer completes its scream. The round manager queues the
    extra wave. A second broadcast with WalkerCount 0 means cancel (screamer
    died inside the cancel window). */
UPROPERTY(BlueprintAssignable, Category = "Zombie")
FOnZombieScreamed OnZombieScreamed;
```

### 3. `ALTRoundManager` - the roster

Changes to `Source/LastTrain/Public/Rounds/LTRoundManager.h` and its `.cpp`.

Add, alongside `ZombieClass` (do not remove it):

```cpp
/** One spawnable zombie type. */
USTRUCT(BlueprintType)
struct FLTZombieRosterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	TObjectPtr<ULTZombieTypeData> TypeData;
};
```

```cpp
/** The station's zombie roster. When empty, the round manager falls back to
    spawning ZombieClass with no type data, exactly as before, so existing
    grey box maps keep working. */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rounds")
TArray<FLTZombieRosterEntry> Roster;

/** Rounds divisible by this are a sprinter round: the whole composition shifts
    to the Sprinter type and the count is scaled by SprinterRoundCountFraction. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
int32 SprinterRoundInterval = 5;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
float SprinterRoundCountFraction = 0.75f;

/** Rounds divisible by this get BruteRoundBrutePair brutes guaranteed, spawned
    partway through the normal count, on top of the normal composition. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
int32 BruteRoundInterval = 10;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rounds")
int32 BruteRoundBruteCount = 2;
```

Every one of `SprinterRoundInterval`, `SprinterRoundCountFraction`,
`BruteRoundInterval`, `BruteRoundBruteCount` is a `[DECISION NEEDED]` marker
(see 3.2). The values above are the open-questions 3.2 proposal.

`.cpp`:

- `BeginPlay`: log the roster. If `Roster.Num() == 0` and `ZombieClass` is null,
  the existing "no rounds will run" style warning path applies.
- New private helper `const ULTZombieTypeData* ChooseTypeForSpawn(int32 SpawnIndex, int32 Round) const;`

  Logic:
  1. Sprinter round (`SprinterRoundInterval > 0 && Round % SprinterRoundInterval == 0`):
     return the roster's `Sprinter` entry. Screamers and crawlers do not mix in
     on special rounds (3.2).
  2. Brute round guaranteed pair: `StartRound` computes two spawn indices at
     `~30%` and `~70%` of the round count (marker: the exact fractions, 3.2) and
     stores them; when `TrySpawnOne` hits one of those indices, force the
     `Brute` entry. This is on top of the normal composition, so the round total
     is `ComputeRoundCount(Round) + BruteRoundBruteCount` on a brute round.
  3. Normal round: weighted choice among roster entries where
     `Round >= FirstRoundAvailable` and `SpawnWeightNormalRound > 0` and the
     per-type `MaxAliveOfThisType` (if set) is not already met by the live
     count. Apply `HighHeatWeightMultiplier` when `Heat` reports a level `>= 3`
     (marker: the heat threshold, 5.1). Fall back to the first walker-behaviour
     entry, then to `ZombieClass`, if the weighted set is empty.

- `TrySpawnOne` change: after the spawn point is chosen and the actor spawned,
  call `Zombie->ApplyRoundScaling(CurrentRound)` as now, then
  `const ULTZombieTypeData* Chosen = ChooseTypeForSpawn(SpawnIndex, CurrentRound);`
  and `if (Chosen) { Zombie->ApplyTypeData(Chosen); }`. When `Chosen` is null
  (empty roster) the zombie stays a plain walker, unchanged from today. Track
  `SpawnIndex` as a per-round counter that increments here.
  - If `Roster.Num() > 0` but every entry's `TypeData` is null, or the chosen
    entry has no `TypeData`, skip type application and log a warning. Still spawn
    `ZombieClass` so the round can proceed.
  - The class actually spawned is still `ZombieClass` in all cases. The type
    data drives everything else. There is one mesh.

- `StartRound` change: reset `SpawnIndex` to `0`. On a brute round, compute and
  store the guaranteed-brute spawn indices. On a sprinter round, set
  `PendingSpawns = FMath::RoundToInt(ComputeRoundCount(Round) * SprinterRoundCountFraction)`.

- `MaximumAlive` / `GetEffectiveMaximumAlive` / heat: unchanged. The roster does
  not touch the concurrent cap. A brute round's extra pair still counts against
  `GetEffectiveMaximumAlive()` like any other live zombie, so the cap check in
  `Tick` needs no change.

- New handler bound to each spawned zombie's `OnZombieScreamed`:

```cpp
UFUNCTION()
void HandleZombieScreamed(ALTZombieCharacter* Screamer, int32 WalkerCount);
```

  On a positive `WalkerCount`, add that many to `PendingSpawns` and remember the
  screamer so a later cancel (`WalkerCount == 0`) can subtract any not-yet-spawned
  remainder. Bind it in `TrySpawnOne` next to the `OnZombieDied` bind; unbind it
  in `HandleZombieDied` and `EndPlay` the same way `OnZombieDied` is cleaned up.
  The summoned walkers are ordinary `PendingSpawns` and obey the cap and the
  spawn interval; they do not bypass anything.

### 4. Editor assets (not this task, note only)

The five `ULTZombieTypeData` assets, the shared tintable zombie material with a
`TintColour` vector parameter and its five instances, the anim Blueprint reading
`GetAnimPlayRate()`, and the roster wiring on the placed round managers in
`L_GreyboxTest` and `L_CanaryWharf_Greybox` are NeoStack editor work. Stub them
into `docs/tasks/neostack-build.md` once this C++ compiles. This task ends at
"the C++ compiles and, with assets present, behaves as the acceptance list
says".

## Decision markers

Every number below is unresolved. The value shown is the
`docs/design/open-questions.md` section 3 proposal, carried here so a build
session has a working default, NOT a signed-off design. Do not treat any of
these as final. Author the data assets with these values, flagged provisional.

- `[DECISION NEEDED: open-questions 3.1 walker - proposed: HP mult 1.0 (150), speed 130, dmg 24, cooldown 1.3s, scale 1.0, first round 1, normal-round weight 100, trait none]` - this row IS the current C++ default; the walker asset carries `0` overrides and multiplier `1.0` throughout.
- `[DECISION NEEDED: open-questions 3.1 sprinter - proposed: HP mult 0.55 (83), speed 500, dmg 18, cooldown 1.0s, scale 0.95, first round 5, normal-round weight 0, sprinter-round weight 100, trait "genuinely fast, between player walk 420 and sprint 640"]`
- `[DECISION NEEDED: open-questions 3.1 sprinter lunge - proposed: a short forward lunge (AddImpulse ~200) on the 0.35s attack wind-up; whether the sprinter lunges at all is unconfirmed]`
- `[DECISION NEEDED: open-questions 3.1 brute - proposed: HP mult 5.0 (750), speed 95, dmg 45, cooldown 2.2s, scale 1.5, first round 10, normal-round weight 0, pair on x10 rounds, trait "front plate absorbs body shots until ~200 accumulated body damage, then normal damage; headshots and rear hits always land; no knockback-on-hit, no blanket stagger immunity"]`
- `[DECISION NEEDED: open-questions 3.1 crawler - proposed: HP mult 0.35 (52), speed 150 (crawl), dmg 20, cooldown 1.1s, scale 0.5 (prone), first round 8, normal-round weight 12 from round 8, trait "low profile: head only reachable by a crouched player, body reachable standing; no death gas in the base design"]`
- `[DECISION NEEDED: open-questions 3.1 screamer - proposed: HP mult 0.8 (120), speed 110, dmg 10, cooldown 1.5s, scale 1.0, first round 12, normal-round weight 6 from round 12, max 1 alive, trait "on clear line of sight to the player for 2 continuous seconds, screams: summons one extra wave of 4 walkers; the scream is loud and directional and naturally masks other audio while it lasts; no submix effect, no post-death deafen; killing it fast is the counterplay"]`
- `[DECISION NEEDED: open-questions 3.1 screamer LoS test - proposed: line trace from the screamer head bone to the player camera on ECC_Visibility, blocked by geometry and by the flood water plane above 120cm, NOT blocked by other zombies; must hold clear 2 continuous seconds; darkness is not occlusion]`
- `[DECISION NEEDED: open-questions 3.1 / 18.4 per-type visual tell - proposed: types read primarily by silhouette (scale), speed and one strong tell each (crawler pose, brute mass, sprinter animation, screamer mouth), not by five bespoke characters; within a type use City Sample Crowds-style body/head/material variation; budget <= 6 unique skeletal meshes total, <= 12 head variants, <= 8 material instances]`
- `[DECISION NEEDED: open-questions 2.2 per-instance gait - proposed: on spawn multiply MaxWalkSpeed by FRandRange(0.88, 1.12) and match the anim play rate; sprinters get a tighter band (0.95 to 1.08); store the multiplier so ApplyRoundScaling re-applies it. This spec carries the type AnimPlayRate; the per-instance jitter on top is a separate open item]`
- `[DECISION NEEDED: open-questions 3.2 sprinter round composition - proposed: every 5th round (5, 15, 25...) the entire roster is sprinters, count reduced to 75% of the normal curve; crawlers and screamers never on special rounds]`
- `[DECISION NEEDED: open-questions 3.2 brute round composition - proposed: every 10th round (10, 20...) is a normal walker round PLUS 2 brutes spawned at 30% and 70% of the way through the count; round 20, 30 etc are both a sprinter round and a brute pair]`
- `[DECISION NEEDED: open-questions 3.2 sprinter-round count fraction - proposed: 75% (not 60%); contingent on sprinter speed being fixed at 500; tune in Phase G once sprinter speed is locked]`
- `[DECISION NEEDED: open-questions 3.3 death behaviours - proposed: walker/sprinter snap-to-settled-pose (no full ragdoll), 6s corpse; brute falls forward, small screen shake within 6m, 10s corpse; crawler does not ragdoll like the others (already prone); screamer mid-scream death cancels the summoned wave only if killed within the first 0.5s of the scream]`
- `[DECISION NEEDED: open-questions 3.3 crawler death gas - proposed: NOT in the base design; a 3m sodium gas cloud (4 HP/s for 4s) is new design with nothing behind it and interacts with flood water, the downed state and colourblind readability; if accepted, spec those interactions; if not, the crawler's threat is its low profile and pack numbers. This spec omits the gas; ArmourBodyDamageToBreak-style fields for it are not added]`
- `[DECISION NEEDED: open-questions 3.4 per-type navmesh/RVO - proposed: RepathIntervalSeconds sprinter 0.2, crawler 0.4, others 0.35; CapsuleHalfHeight crawler 45, brute 130, others 90 (the constructor does not currently resize the capsule, so this is new); AvoidanceConsiderationRadius brute 70, others 45; ContactRange sprinter 25 (not 15) so a 500-speed agent does not overshoot the acceptance point on a 0.2s repath]`
- `[DECISION NEEDED: open-questions 3.4 per-type animation sets - proposed: each type needs its own locomotion set (a brute at scale 1.5, a prone crawler at scale 0.5 and a 500-speed sprinter cannot share one walk cycle; the crawler needs a genuinely different rig pose); this is a Phase C/F animation blocker tracked in open-questions sections 14 and 17, not solved by the data asset]`
- `[DECISION NEEDED: open-questions 3.4 leash / de-spawn - proposed: a zombie that loses the player entirely keeps pathing to the last known player location for 20s, then walks to the nearest spawn point and de-spawns silently, decrementing LiveZombies, so LiveZombies.Num() == 0 stays reachable. Not built in this spec; noted as required alongside the roster]`
- `[DECISION NEEDED: open-questions 5.1 high-heat special weighting - proposed: from heat 3+, the sprinter, screamer and crawler normal-round weights roughly double (HighHeatWeightMultiplier 2.0 for those three, 1.0 for walker and brute); heat threshold for the doubling is 3]`
- `[DECISION NEEDED: open-questions 18.4 / 18.5 crowd cost of type variety - proposed: the ColourTint MUST be applied through a shared tintable material with one UMaterialInstance per type, or a pooled UMaterialInstanceDynamic; ApplyTypeData must NOT call CreateDynamicMaterialInstance per spawn. 24 to 40 mixed-type instances must hold 60fps on the section 18 budget (RTX 2060 class, 1080p, Medium). Corpse pool cap 10, blood decal pool 32, impact particle cap ~24 concurrent, all EditDefaultsOnly]`
- `[DECISION NEEDED: open-questions 19.3 sprinter as anti-kite tool - proposed: sprinter base 500, first appearing round 5, mixing into normal rounds from heat 3+ so a high-heat kiting player always has a few sprinters breaking the loop; a sprinter that loses line of sight for >3s drops to walker speed until it re-acquires (a punish for predictability, not an unshakeable heat-seeker). The heat-3 mix-in is covered by HighHeatWeightMultiplier; the "drop to walker speed on lost LoS" behaviour is a separate anti-kite item, not built here]`
- `[DECISION NEEDED: open-questions 19.1 / 19.2 flanking and ahead-of-player spawn bias - proposed: a fraction of the horde (scaling ~20% at round 5 to ~50% by round 15) paths to a predicted intercept point rather than the player; spawn-point weighting biases toward points ahead of the player's velocity. This is where type behaviour (the sprinter) and the anti-kite AI meet; the flanking/intercept system is open-questions section 19 and NOT part of this task, but the sprinter type exists partly to serve it]`

## Constraints

- New files: `Source/LastTrain/Public/Zombies/LTZombieTypeData.h` and, if it
  carries any function body, `Source/LastTrain/Private/Zombies/LTZombieTypeData.cpp`.
  Additions to `LTZombieCharacter.h/.cpp` (`ApplyTypeData`, `GetZombieType`,
  `GetAnimPlayRate`, the two enums via the data header, `OnAttackWindUp`,
  `OnScream`, `OnZombieScreamed`, the private applied state, the small
  `ArmourPlate` and `Scream` branches). Changes to `LTRoundManager.h/.cpp`
  (`FLTZombieRosterEntry`, `Roster`, the four special-round properties,
  `ChooseTypeForSpawn`, `HandleZombieScreamed`, `SpawnIndex`, the `StartRound`
  and `TrySpawnOne` edits). Nothing else.
- Compiles clean with the batch build, `-Werror` on.
- **No new trace or object channels.** The screamer line of sight uses
  `ECC_Visibility`. The weapon channel `ECC_GameTraceChannel1` is untouched.
- `ZombieClass` stays on `ALTRoundManager` and stays the class every zombie is
  spawned from. The roster drives type DATA, not the spawned class. When
  `Roster` is empty the behaviour is byte-for-byte the current single-`ZombieClass`
  path, so `L_GreyboxTest` and `L_CanaryWharf_Greybox` keep working with no map
  change until their round managers are given a roster.
- Do not restructure `TryAttack` into a wind-up state machine (open-questions
  2.5, separate task). Only add the `OnAttackWindUp()` call and the optional
  sprinter lunge impulse.
- Do not build the flanking / intercept AI (open-questions section 19), the
  leash / de-spawn (3.4), the crawler death gas (3.3), the sprinter
  lost-line-of-sight speed drop (19.3) or the per-instance gait jitter (2.2).
  They are marked and noted, not implemented.
- Do not modify `ULTWeaponComponent`, `ULTPointsComponent`, `ULTStationHeat`,
  `ALTSpawnPoint` or the player. If this task appears to need a change in one of
  those, stop and say so.
- British spelling everywhere including comments and `FText` (the checker rejects
  organiz*, color, behavior, customiz*). No em or en dashes. `#pragma once`
  first, `X.generated.h` last include. `TObjectPtr` for every `UObject` member
  and in every container, never a raw `UObject*`. `LT_LOG(Verbosity, TEXT("..."))`,
  never `UE_LOG`. Tab indent. No `TODO`, `FIXME`, `HACK`, `XXX` markers.
- Match `ULTWeaponData` for the data asset's shape and `ALTZombieCharacter` for
  the character conventions. Do not write idiomatic UE from memory where the
  surrounding file does it differently.
- Keep each of the four steps a compiling checkpoint. A small verified change
  beats a large unverified one.

## Acceptance

Runnable in PIE once the five `ULTZombieTypeData` assets, the tintable material
and its instances, and a roster on the `L_GreyboxTest` round manager exist.
Until then, step 1 is the bar.

1. All four steps compile clean with the batch build, `-Werror` on. With an
   empty `Roster`, `L_GreyboxTest` plays exactly as it did at the end of Phase
   B: one walker on the coded defaults, round loop intact, no behaviour change.
2. With the five assets and a roster wired: over one normal round, walkers,
   crawlers and (from round 12) screamers spawn in a visibly mixed crowd, each
   with a distinct mesh scale, colour tint and movement speed. No two types are
   visually indistinguishable at a glance.
3. The roster weighting produces a believable mix: a normal round is mostly
   walkers with a minority of crawlers, not an even split, and never a sprinter
   or a brute.
4. Round 5 (and 15) is a sprinter round: the whole crowd is sprinters, the count
   is about three-quarters of the normal round-5 count, and no crawlers or
   screamers appear.
5. Round 10 spawns exactly two brutes on top of the normal walker round, one
   partway in and one later, each visibly larger and slower, each taking many
   body shots before dying and dying fast to headshots or rear hits.
6. The screamer's effect is observable: hold line of sight to a screamer for two
   seconds without killing it and an extra wave of walkers is added to
   `PendingSpawns` (visible in the diagnostic log and as more zombies).
   Killing the screamer inside the cancel window drops the pending wave.
7. The crawler is low to the ground: shots that would hit a walker's head pass
   over a standing player's crosshair line to the crawler, and its capsule is
   shorter (verify with `show Collision`).
8. `GetEffectiveMaximumAlive()` is still respected on every round including the
   brute round and after a screamer summon: live count never exceeds the cap,
   summoned and guaranteed-pair zombies queue behind it.
9. Heat at level 3+ visibly raises the proportion of sprinters, screamers and
   crawlers in a normal round versus the same round at heat 0.
10. With 24 or more mixed-type zombies alive on the grey box, `stat unit` shows
    the frame rate holding at or near 60fps, no worse than the all-walker Phase
    B crowd check at the same count. No per-spawn dynamic material creation
    shows up in a quick `stat rhi` / material instance count.
11. A round still ends exactly once, `LiveZombies` reaches zero, and no type
    (crawler capsule, brute size, sprinter speed) leaves a zombie stuck such
    that the round cannot end.

## On pass

Update `docs/tasks/NEXT.md` and the Phase C row in `docs/tasks/README.md`. Fold
every resolved decision marker into `docs/design/gameplay-canon.md` (or
brief-v2 until that file exists) and strike it from `open-questions.md` section
3. Stub the five data assets, the tintable material and the roster wiring into
`docs/tasks/neostack-build.md` for a NeoStack editor pass. The next Phase C C++
is the train (`ALTTrainActor`): arrival on the 100s interval, 25s dwell,
boarding as an interactable during dwell.

# H2: finish the blood and gore decal system

Lane: CC-in-Unreal. The `Source/` half is done and is on this branch. What is
left is editor work: the decal materials, the one call that hands them to the
subsystem, the `MaxActiveDecals` number, and the read in PIE.

Depends on: nothing. `ULTGoreDecalSubsystem` is null safe with no materials
assigned, so the game runs exactly as it does today until step 1 lands.

Read `../art-direction.md` section 6 before tuning anything in here. It is four
sentences and it is the whole brief: restraint, high value moments over uniform
coverage, blood concentrated in the immediate combat area and on the zombies
themselves rather than sprayed over every surface, impact decals pooled and
capped with the oldest recycled. If a change makes the station messier, it is
wrong even if it looks impressive in a screenshot.

## What already exists in C++

`ULTGoreDecalSubsystem`, a `UWorldSubsystem`, in
`Source/LastTrain/Public/Combat/LTGoreDecalSubsystem.h`.

- `SpawnBloodDecal(const FVector& Location, const FVector& Normal, bool bHeadshot)`,
  `BlueprintCallable`. One impact spatter, or `HeadshotDecalCount` of them on a
  headshot.
- `SpawnDeathPool(const FVector& Location, bool bHeadshot)`, `BlueprintCallable`.
  One larger decal on the floor under a corpse.
- `SetDecalMaterials(UMaterialInterface* Impact, UMaterialInterface* DeathPool)`,
  `BlueprintCallable`. This is step 1 below.
- `ClearDecals()` and `GetActiveDecalCount()`, for a reset and for reading the
  pool in PIE.
- Static helpers for C++ callers that only hold a `UWorld`:
  `ULTGoreDecalSubsystem::SpawnBloodDecalForWorld(GetWorld(), Location, Normal, bHeadshot)`,
  `SpawnDeathPoolForWorld(GetWorld(), Location, bHeadshot)` and
  `Get(const UWorld*)`.

`ALTZombieCharacter` already calls it: `ReceiveShot` spawns the spatter next to
the existing `OnHitReaction` Blueprint event, and `Die` spawns the pool next to
`OnDeathPresentation`. Both Blueprint hooks still fire, unchanged. An armour
plated hit that the plate absorbs spawns nothing.

Spatters are placed on world geometry behind the target, found by a trace
against `WorldStatic` and `WorldDynamic` only, not on the zombie. A deferred
decal projected onto a skeletal mesh slides about with the mesh, and the blood
on the bodies is already the zombie material's own blood layer from S9 and F6a.
If nothing is within `SpatterTraceDistance` behind the target, the trace falls
back to the floor, and if that misses too, nothing is drawn.

**None of this has been compiled or run.** It was written in a remote session
with no engine available. Treat the first editor session on it as a compile and
smoke test, not as a tuning pass.

## 1. Assign the decal materials

The subsystem holds `ImpactDecalMaterial` and `DeathPoolDecalMaterial`, both
null until something assigns them, the same pattern as the Enhanced Input
actions and `ULTWeaponComponent::WeaponData`. A subsystem has no archetype you
can edit in the content browser, so the assignment is a call, not a property
edit in the details panel:

- In `BP_GameMode` (or `BP_PlayerCharacter`, if the game mode Blueprint is not
  the natural home) on `BeginPlay`, get the world subsystem
  `LTGoreDecalSubsystem` and call `SetDecalMaterials` with the two materials.
  The node is `Get Subsystem` with `LTGoreDecalSubsystem` as the class.

### The impact material: reuse `M_LT_ZombieWound`

`Content/LastTrain/Materials/M_LT_ZombieWound.uasset` already exists, already
compiles, and is attached to nothing. The S9 handover row describes it as a
valid deferred decal material, translucent blend, `SAMPLERTYPE_MASKS` sampler
on `T_SurfaceImperfections013_Opacity`, authored as a spare for hand placed
wound decals. That is exactly the material domain this subsystem needs, so the
impact spatter can ship on an asset that is already in the repository. Make a
material instance of it rather than editing the parent, so the character wound
use it was built for stays available.

Check two things before trusting it, both from `known-issues.md` 2.7: that its
material domain really is `MD_DeferredDecal`, and that
`unreal.MaterialEditingLibrary.get_statistics(mat).num_pixel_shader_instructions`
is not zero after a forced recompile. Zero means the shader does not exist
whatever the graph looks like, and this project has been caught by that twice.

### The textures: the F6a wound decals are genuinely reusable

`tools/zombie-surfaces/generate.py --kind wound` emits four
`T_ZombieWound_*` RGBA textures. Its README describes them as "not tiling, for
a deferred decal material such as the existing `M_LT_ZombieWound`", with R
depth, G rim, B wetness and **A the decal's own opacity**, imported `TA_Clamp`
with Compress Without Alpha off. They are world decal textures by construction,
not only character skin masks, and neither step of
`phase-f6a-zombie-surface-masks.md` wires them to anything: that spec ends by
saying the four wound decals are attached to nothing. So the answer to "is
anything reusable" is yes, and these are the exact assets.

They are not committed. There is no `git-lfs` in the remote container and
`*.png` is not on the LFS filter, so regenerate them here, deterministically,
with the command in `phase-f6a-zombie-surface-masks.md` section 1.1.

### The death pool needs one new texture

The four wound shapes read as a wound on a body: they are the right material
and the right channel packing, but the wrong silhouette for a pool settling on
a platform floor. Either author one broad, soft edged pool texture in the same
RGBA layout so it can share the same material, or add a `pool` kind to
`tools/zombie-surfaces/generate.py` alongside `body` and `wound` and keep the
whole set deterministic and regenerable. The generator route is the better one:
the repository's convention is that these textures are generated, not committed
as binaries, and adding a kind is a small change against an existing schema.

Until that exists, leave `DeathPoolDecalMaterial` null. The subsystem skips the
pool entirely when it is, and the impact spatters still work.

## 2. Tune `MaxActiveDecals`, which is a performance number

**This one is not arbitrary. Do not raise it on feel.**

`ULTRoundManager::MaximumAlive` is 24, and station heat widens it from there,
so the horde this has to survive is 24 to 40 alive. The S6 frame reading in
`handover.md` measured a packaged Mac Development build on an Apple M4 at
1280x720 running at roughly 26.5 fps with 24 to 36 zombies alive, **GPU bound
in every configuration**, with the game thread comfortably clear at about 5 ms.
The Phase B gate in `README.md` asks for 60 fps at 24 to 40 alive and that gate
currently fails on GPU cost alone.

Deferred decals are a GPU cost. They are fill rate and an extra pass over the
region they cover, which lands on precisely the side of the frame that is
already the bottleneck. A decal pool is therefore capable of making a measured,
already failing frame gate worse, and it is the only part of this system that
can.

The default is `MaxActiveDecals = 48`, chosen as two per zombie at the 24 alive
cap. Treat that as a starting point to measure, not as a value to accept:

- Measure with the S6 method, not with a PIE eyeball: a packaged Development
  build, CSV profiler, `t.MaxFPS 0`, a forced horde, and compare GPU frame time
  against the S6 numbers with the pool empty and with it full.
- `GetActiveDecalCount()` reports the live pool size, so a HUD debug readout or
  `playtest_read_state` can confirm the ring is capping rather than growing.
- The cap is honoured at placement time, so lowering it at runtime takes effect
  on the next hit. That makes an A and B comparison in one session cheap.
- If the pool costs measurable GPU time, the lever to reach for first is
  `DecalFadeScreenSize` (0.01 by default, raise it so distant decals drop out),
  then `DecalLifetimeSeconds` (45), then the cap.
- Record whatever you measure in the `handover.md` ledger. Phase F7 still has no
  frame rate baseline of its own (`known-issues.md` 2.8), so a number measured
  here is worth keeping.

## 3. Confirm the restrained read in PIE

Play a full round in `L_CanaryWharf_Greybox` and judge it against
`../art-direction.md` section 6, not against how visible the feature is.

- A single body hit leaves one small mark on the wall or floor behind the
  target, not on the zombie.
- A headshot reads as a noticeably bigger moment than a body hit, which is the
  "high value moments" half of the principle, and is still two marks, not a
  spray.
- After several rounds in one place, the platform reads as a fight happened
  here, not as a slaughterhouse. If it reads as excessive, the first number to
  cut is `HeadshotDecalCount`, then `DecalLifetimeSeconds`.
- Nothing floats in mid air, and nothing is stamped across a doorway or a sign.
- Boarding the train and travelling to the next station leaves no decals
  behind. The subsystem is world scoped precisely so that this is automatic,
  and it is worth confirming once.

## Notes for whoever merges the sibling branches

- The public entry point takes a plain world location, a surface normal and a
  headshot flag, so the melee branch calls
  `ULTGoreDecalSubsystem::SpawnBloodDecalForWorld(GetWorld(), Location, Normal, bCritical)`
  from its own hit resolution without touching `ULTWeaponComponent` and without
  needing an actor or a weapon side struct.
- The subsystem is a `UWorldSubsystem`. A sibling system picking a base class
  for consistency should match that only if it is likewise world scoped: a
  subtitle queue that must survive `OpenLevel` travel wants the game instance
  instead, and the two choices do not conflict.
- The zombie side of this branch touches only two places in
  `ALTZombieCharacter.cpp`, both additive and both next to an existing
  Blueprint event. A branch that also adds a call at those same two points
  should expect a small textual conflict and no semantic one.

## Accept

- `SetDecalMaterials` is called once on `BeginPlay` and
  `GetActiveDecalCount()` climbs from zero during a round and then stops at
  `MaxActiveDecals`.
- The impact material reports non zero pixel shader instructions after a forced
  recompile.
- A GPU frame time reading with the pool full, compared against S6, recorded in
  `handover.md`.
- Map Check 0 errors, 0 warnings, ignoring the stock lighting rebuild message.
- All five CI gates pass.

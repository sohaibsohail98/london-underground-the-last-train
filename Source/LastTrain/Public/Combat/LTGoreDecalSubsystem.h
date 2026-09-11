#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LTGoreDecalSubsystem.generated.h"

class UDecalComponent;
class UMaterialInterface;

/** Pooled blood decals for the immediate combat area.

	art-direction.md section 6 is the brief this implements: restraint, blood
	concentrated where the fighting happened rather than sprayed over every
	surface, impact decals pooled and capped with the oldest recycled. The pool
	is a fixed ring of UDecalComponent, so a long round reuses components rather
	than accumulating them, and the worst case cost is MaxActiveDecals whatever
	the horde does.

	This is a UWorldSubsystem, not a UGameInstanceSubsystem. Decal components
	belong to one UWorld and die with it, and boarding the train travels to
	NextStationMap through OpenLevel (see ALTGameMode and ULTGameInstance), so a
	game instance subsystem would outlive the world its pool points into and
	carry a ring of stale components across the transition. One instance per
	world, created with the world and torn down with it, is exactly the lifetime
	this needs. A sibling system choosing a subsystem base for consistency
	should match this only if it is likewise world scoped: anything that has to
	survive travel wants the game instance instead.

	Nothing here needs an actor, so any hit source can drive it: hitscan through
	ALTZombieCharacter::ReceiveShot, or a melee hit that never touches
	ULTWeaponComponent. Both call SpawnBloodDecal with a plain world location,
	a surface normal and a headshot flag. */
UCLASS()
class LASTTRAIN_API ULTGoreDecalSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The one line entry point for any hit source, hitscan or melee. Null safe
		on every step: no world, no subsystem or no assigned material is a quiet
		no-op rather than a crash, so a caller never has to check first. */
	static void
	SpawnBloodDecalForWorld(const UWorld* World, const FVector& Location, const FVector& Normal, bool bHeadshot);

	/** As above, for the death pool. */
	static void SpawnDeathPoolForWorld(const UWorld* World, const FVector& Location, bool bHeadshot);

	/** The subsystem for this world, or null. */
	static ULTGoreDecalSubsystem* Get(const UWorld* World);

	/** One impact spatter. Location and Normal are the hit point and surface
		normal on the target, as FHitResult reports them; the spatter itself is
		placed on whatever world geometry sits behind the target, since a decal
		projected onto a walking skeletal mesh slides with it. A headshot spawns
		HeadshotDecalCount rather than ImpactDecalCount and scales them by
		HeadshotSizeMultiplier. Does nothing if ImpactDecalMaterial is unset. */
	UFUNCTION(BlueprintCallable, Category = "Gore")
	void SpawnBloodDecal(const FVector& Location, const FVector& Normal, bool bHeadshot);

	/** The single settling pool under a corpse. Location is the dying actor's
		world location; the floor is found by tracing down from it. Does nothing
		if DeathPoolDecalMaterial is unset. */
	UFUNCTION(BlueprintCallable, Category = "Gore")
	void SpawnDeathPool(const FVector& Location, bool bHeadshot);

	/** Hides and releases every pooled decal. For a round reset or a test. */
	UFUNCTION(BlueprintCallable, Category = "Gore")
	void ClearDecals();

	/** Assigns the materials at runtime. A subsystem has no editable archetype
		in the editor, so this is the practical route: the game mode or a
		Blueprint calls it once on BeginPlay with the project's decal materials.
		Either argument may be null, which simply disables that decal kind. */
	UFUNCTION(BlueprintCallable, Category = "Gore")
	void SetDecalMaterials(UMaterialInterface* Impact, UMaterialInterface* DeathPool);

	UFUNCTION(BlueprintPure, Category = "Gore")
	int32 GetActiveDecalCount() const { return Decals.Num(); }

	/** Null until SetDecalMaterials or a subclass assigns one, following the
		same pattern as the input actions and the weapon data: the C++ carries
		the behaviour and the editor supplies the asset. A deferred decal
		material, in the family of the existing M_LT_ZombieWound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	TObjectPtr<UMaterialInterface> ImpactDecalMaterial;

	/** The larger, softer pool left under a corpse. Null disables it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	TObjectPtr<UMaterialInterface> DeathPoolDecalMaterial;

	/** Hard ceiling on live decal components. The oldest is recycled once this
		is reached, so the cost is bounded whatever the round does. Performance
		sensitive: read the note in docs/tasks/phase-h2-gore-system.md before
		raising it. 48 is two per zombie at the 24 alive cap. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	int32 MaxActiveDecals = 48;

	/** Spatters per body hit. One, per the restraint principle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	int32 ImpactDecalCount = 1;

	/** Spatters per headshot. The high value moment the style guide asks for,
		still only two. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	int32 HeadshotDecalCount = 2;

	/** Decal box half extents. X is the projection depth into the surface. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	FVector ImpactDecalSize = FVector(12.f, 16.f, 16.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	FVector DeathPoolDecalSize = FVector(16.f, 48.f, 48.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	float HeadshotSizeMultiplier = 1.35f;

	/** Random scale spread either side of the nominal size, so a wall of hits
		does not read as a row of identical stamps. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	float SizeJitterFraction = 0.25f;

	/** How far behind the target the spatter looks for a surface. Beyond this
		the hit happened in open space and nothing is drawn, which is the
		restrained answer rather than stamping the floor from a rooftop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	float SpatterTraceDistance = 260.f;

	/** How far down the death pool looks for a floor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	float GroundTraceDistance = 220.f;

	/** Seconds a decal stays at full strength before it starts to fade. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	float DecalLifetimeSeconds = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	float DecalFadeSeconds = 6.f;

	/** Screen size below which the renderer drops the decal. Cheap distance
		culling for a station full of them. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gore")
	float DecalFadeScreenSize = 0.01f;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

private:
	/** Places one decal, recycling the oldest pooled component once the ring is
		full. Returns null if the material is unset or the world has gone. */
	UDecalComponent*
	PlaceDecal(UMaterialInterface* Material, const FVector& Location, const FVector& Normal, const FVector& Size);

	/** Traces from a hit point through the target for the surface behind it,
		then straight down as a fallback. Returns false if neither finds one. */
	bool
	FindSpatterSurface(const FVector& Location, const FVector& Normal, FVector& OutLocation, FVector& OutNormal) const;

	/** Straight down from Origin for the floor. */
	bool FindGroundSurface(const FVector& Origin, FVector& OutLocation, FVector& OutNormal) const;

	FVector JitterSize(const FVector& Size, float Multiplier) const;

	/** The ring. Grows to MaxActiveDecals, then NextDecalIndex walks it, so the
		component reused is always the one placed longest ago. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDecalComponent>> Decals;

	int32 NextDecalIndex = 0;
};

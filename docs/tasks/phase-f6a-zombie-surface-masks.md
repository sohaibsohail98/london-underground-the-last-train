# F6a: wire the generated zombie surface masks

Lane: Opus in the editor (CC-in-Unreal). No `Source/` change. Depends on: nothing.
UE 5.8.

Two steps, with a **verification gate** between them rather than a session
break. Both can be done in one sitting. What must not be skipped is the check at
the end of 1.3: this subsystem has failed silently twice, in S9 and again in F2,
and both times it looked like success until somebody read a specific number. Do
not build step 2 on an uncompiled material.

Read `tools/zombie-surfaces/README.md` first, in particular the "Unreal import
settings" and the trap at the end of it. Read `docs/known-issues.md` 2.7 before
touching any material.

## Context

`tools/zombie-surfaces/` generates per-zombie mask sets. It exists because of
the defect the S9 handover row records: `M_Zombie_Tintable` tiles one generic
ambientCG mask (`T_SurfaceImperfections011_Opacity`) over every zombie in the
game, so the whole horde carries an identical blotch pattern.

The generator emits 8 tiling body masks and 4 wound decals. **The images are not
committed** (no `git-lfs` in the remote container, and `*.png` is not on the LFS
filter, see `../asset-pipeline-plan.md` section 1.5). They are deterministic, so
you regenerate them here and get byte identical files.

The body masks pack three separate masks into one texture, one per channel, so
the material samples once for all three. That matters at 24 to 40 alive, and it
means this change does **not** add a sampler: it replaces the existing single
blood mask sample with one that carries three masks.

| Channel | Mask | What it drives |
|---|---|---|
| R | blood | The role the existing `BloodMask` sample plays today |
| G | grime | New. Soot and dust, darkens the albedo |
| B | lividity | New. Dead flesh mottling, makes the skin uneven |

## Step 1: generate, import, and give each type its own mask

### 1.1 Generate

From the repo root, on this machine:

```sh
python3 -m venv .venv
.venv/bin/pip install -r tools/zombie-surfaces/requirements.txt
.venv/bin/python tools/zombie-surfaces/tests/run_assertions.py
.venv/bin/python tools/zombie-surfaces/generate.py --out build/zombie-surfaces --size 1024
```

The assertion run should report 33 tests OK. `build/` is gitignored.

Open `build/zombie-surfaces/contact-sheet.png` and look at it before importing
anything. If the masks look wrong, stop and say so: that is a generator problem
and it is fixed in `tools/zombie-surfaces/`, not in the editor.

### 1.2 Import

Import the 8 `T_ZombieSurface_*.png` and 4 `T_ZombieWound_*.png` to
`/Game/LastTrain/Zombies/Surfaces/`, using the same import path S9 used for the
ambientCG textures.

`build/zombie-surfaces/manifest.json` carries the settings per kind in its
`unreal` block, and they are the authority. In short:

- Both kinds: **Compression Settings `TC_Masks`, sRGB OFF**, Texture Group
  `TEXTUREGROUP_Character`, Mip Gen `TMGS_FromTextureGroup`.
- Body masks: Address X and Y `TA_Wrap`.
- Wound decals: Address X and Y `TA_Clamp`, and Compress Without Alpha **off**
  (the alpha channel is the decal's own opacity).

sRGB is the one to get right. A mask read as sRGB looks nearly correct and every
value in it is wrong.

Verify after import: 12 textures exist, every one reports `srgb = False` and
`TC_Masks`, and none is on `TC_EditorIcon` (the F5 pictogram trap).

### 1.3 Extend `M_Zombie_Tintable`

One new sampler parameter, replacing the existing blood mask sample rather than
adding to it.

1. Add a `TextureSampleParameter2D` named `SurfaceMask`. Assign
   `T_ZombieSurface_worn_01` as its default texture **before** setting
   `sampler_type`, then set `sampler_type = SAMPLERTYPE_MASKS`. That order is
   not optional: the S9 handover row records that reversing it makes the Metal
   shader compile fail silently and the engine Default Material render instead.
2. Route the **R** channel into wherever the existing `BloodMask` sample feeds.
   Keep `BloodTiling`, `BloodColour` and `BloodAmount`. **Bring `BloodContrast`
   down from 3.0 to about 1.2**: the generated mask already has its contrast
   curve baked in, so 3.0 will clip it to hard edges.
3. Route the **G** channel to lerp the albedo towards a new `GrimeColour`
   scalar-driven vector (default about `0.18, 0.17, 0.16`) by a new
   `GrimeAmount` (default 0.5), and add a small roughness rise where grime is
   heavy.
4. Route the **B** channel to multiply the dead flesh albedo by
   `0.82 + lividity * 0.36`, scaled by a new `LividityAmount` (default 1.0), so
   the skin reads uneven rather than flat.
5. Leave `DeadFleshColour`, `TintColour`, `TintStrength`, `RoughnessBoost` and
   `bUsedWithSkeletalMesh` alone. The five `MI_Zombie_*` inherit from this
   material and the C++ and Blueprint tint wiring must keep working untouched.

**Do not call `recompile_material` in the same `execute_script` payload that
builds the graph.** Build, save, then compile in a separate call. Then read
`unreal.MaterialEditingLibrary.get_statistics(mat).num_pixel_shader_instructions`
and confirm it is **not zero** before wiring the material to anything. Zero
means the shader does not exist whatever the graph looks like. A healthy
comparable is `M_LT_PBRSurface` at 379. This is `known-issues.md` 2.7 and it has
already cost one session.

### 1.4 Give each type its own mask

Set `SurfaceMask` per type on the five existing material instances. The
generator's `types` field says which variant suits which roster, and the
manifest records it per entry:

| Material instance | `SurfaceMask` |
|---|---|
| `MI_Zombie_Walker` | `T_ZombieSurface_worn_01` |
| `MI_Zombie_Sprinter` | `T_ZombieSurface_soaked_02` |
| `MI_Zombie_Brute` | `T_ZombieSurface_drenched_01` |
| `MI_Zombie_Crawler` | `T_ZombieSurface_filthy_01` |
| `MI_Zombie_Screamer` | `T_ZombieSurface_drenched_02` |

That gives five distinct looks with no Blueprint work, which is enough to prove
the whole path end to end. Getting the other three masks in play, so two walkers
differ, is step 2.

## Rules

- No gameplay actor default moves. No `Source/` change.
- `MAP CHECK` as a console exec is banned. Use `MAP CHECKDEP NOCLEARLOG`. See
  `editor-crash-endplaymap.md`.
- Never mutate an actor or asset while PIE is running.
- Commit the 12 `.uasset` textures and the changed materials via LFS. Verify each
  is a pointer: `git show <ref>:<path> | head -c 45` should start
  `version https://git-lfs...`.
- Do not commit anything from `build/`. It is gitignored on purpose.
- Update the F6a row in `handover.md` and the Phase F table in `README.md`.

## Accept

- 12 textures under `/Game/LastTrain/Zombies/Surfaces/`, all `srgb = False` and
  `TC_Masks`.
- `M_Zombie_Tintable` reports non zero `num_pixel_shader_instructions` after a
  forced recompile, and all five `MI_Zombie_*` still open with their tints intact.
- Play a round in `L_CanaryWharf_Greybox`: zombies render the new masks, not the
  engine Default Material and not a flat tint. A walker and a brute standing near
  each other are visibly differently marked.
- Spawn a sprinter round at 5 and the brute pair at 10 and confirm each type
  still carries its correct stats and accent tint.
- Map Check 0 errors, 0 warnings, ignoring the stock "Maps need lighting rebuilt".
- Note the frame cost with 24 to 40 alive for F7, if it is measurable.

## Step 2: variation inside a type

Only once the material statistic in 1.3 is non zero. After step 1 every walker
still shares one mask, because the mask lives on a material instance shared by
all walkers.

**Do this with more static material instances, not with a dynamic one.**
`ALTZombieCharacter` already swaps a material per type on spawn, so picking from
a wider set of pre made instances is the same single `SetMaterial` you already
pay for: no allocation per zombie, no `CreateDynamicMaterialInstance`, no new
runtime cost at all. A dynamic material instance per zombie would buy nothing
here and cost an allocation on every spawn.

1. Author three more instances of `M_Zombie_Tintable` so all eight masks are in
   play, each carrying its own `SurfaceMask` and inheriting the tint of the type
   it serves: `MI_Zombie_Walker_B` (`worn_02`), `MI_Zombie_Walker_C`
   (`soaked_01`), `MI_Zombie_Crawler_B` (`filthy_02`). Walker and crawler get
   the extra coverage because the walker is most of the horde and the crawler is
   the other type with two masks assigned to it in the data.
2. In `BP_Zombie`, hold a small array of instances per type and pick one on
   spawn, after the existing type swap so it is not overwritten.
3. Pick deterministically from the actor's own identity rather than from a raw
   random, so a replay looks the same.

Nothing here needs profiling, which is the other reason to prefer it: F7 has no
frame rate baseline at all (`known-issues.md` 2.8), so a change with a runtime
cost could not be measured against anything even if you wanted to.

The 4 wound decals are not wired by either step. `M_LT_ZombieWound` exists,
compiles, and is attached to nothing. Hand placed wound decals are their own
task and are worth doing only if the type reads need more help after F6.

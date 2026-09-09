# Zombie surface variation

Deterministic mask generation for the horde. Emits packed body detail masks and
wound decals as PNG, plus a manifest the editor session imports against.

The problem it solves is recorded in `docs/tasks/handover.md` under S9: every
zombie in the game currently shares one blotch pattern, because
`M_Zombie_Tintable` tiles a single generic ambientCG grime mask
(`T_SurfaceImperfections011_Opacity`) over all of them. The masks are fine; there
is only one of them. This track produces a set, biased per zombie type, so a
horde reads as a crowd rather than as one texture repeated.

## What it emits

**Body masks**, one RGB PNG per variant, tiling. Three separate masks packed one
per channel so the material samples once for all three, which matters at 24 to
40 zombies alive:

| Channel | Mask | What it is |
|---|---|---|
| R | blood | Fresh and dried fluid: ragged pools, rivulets running down from them, spatter, a dried crust on the boundary |
| G | grime | Soot, dust and tunnel fluff. Cloudy accumulation with fine cracking |
| B | lividity | Dead flesh mottling. Low frequency and low contrast on purpose: it should make skin uneven, not draw a shape on it |

**Wound decals**, one RGBA PNG each, not tiling, for a deferred decal material
such as the existing `M_LT_ZombieWound`:

| Channel | Mask |
|---|---|
| R | depth, how deep the wound reads |
| G | rim, the dried and crusted edge |
| B | wetness, where it still runs |
| A | shape, the decal's own opacity |

The committed batch is 8 body variants and 4 wounds at 1024 px, which is a
representative batch rather than the full volume. `--size 2048` regenerates at
double that; the parameters are fractions of the texture size, so a variant
looks the same at either and the resolution is a build option rather than a
look.

## Running it

```sh
python3 -m venv .venv
.venv/bin/pip install -r tools/zombie-surfaces/requirements.txt

# everything, at the committed size
.venv/bin/python tools/zombie-surfaces/generate.py --out build/zombie-surfaces --size 1024

# one variant, quickly, while tuning
.venv/bin/python tools/zombie-surfaces/generate.py --only drenched_01 --size 512

# what would be written, without writing it
.venv/bin/python tools/zombie-surfaces/generate.py --dry-run
```

`--kind body|wound|all`, `--only ID` (repeatable), `--seed N` to offset every
variant seed at once, `--no-contact-sheet`, `--stamp` to add build time and
commit to the manifest.

Assertions:

```sh
.venv/bin/python tools/zombie-surfaces/tests/run_assertions.py
```

33 assertions, stdlib `unittest`, no pytest. Takes about seven seconds.

## Reviewing it

`contact-sheet.png` is the point of the build for a human. Per variant it shows
each channel on its own, a composite that applies the masks the way
`M_Zombie_Tintable` does, and a two by two repeat so a tiling seam shows as a
cross through the middle of the block. The composite is not a render: there is
no lighting in it, so read it for distribution and contrast, not for final tone.

Two defects were found and fixed by looking at that sheet rather than at the
numbers, which is the reason it exists:

- The first blood masks were nearly empty. Single pixel origins blurred across a
  2048 texture carry almost no mass, and the contrast pivot then crushed what
  little survived. Discs with a real area fixed it.
- The first grime masks read as bubble wrap. The crust component was selecting
  the filled interiors of the cellular noise; it now selects a narrow band at
  the top of the distance field, which is the thin skeleton between cells and
  reads as cracking.

## Adding content

Edit data, never code.

`data/families.json` holds the default parameters for each of the four
families. `data/variants.json` holds one record per output file, and each record
overrides only the keys that differ from the defaults, so a new variant is a few
lines:

```json
{
  "id": "scorched_01",
  "seed": 10997,
  "types": ["brute"],
  "note": "Burnt rather than bloodied.",
  "overrides": {
    "blood": { "origins": 10, "pivot": 0.42 },
    "grime": { "pivot": 0.28, "crust_weight": 0.5 }
  }
}
```

`types` is advisory and names which of the five `ULTZombieTypeData` rosters a
variant suits, so the Blueprint that picks a variant on spawn has a documented
intent rather than a shuffle. Valid values are `walker`, `sprinter`, `brute`,
`crawler`, `screamer`.

The loader validates before anything renders, and names the fix in the error: an
unknown parameter lists the valid ones for that family, an unknown family lists
the valid families, a duplicate id says so, and a lattice period that would not
tile at the build size lists the periods that would.

**Periods must divide the texture size.** That is what makes the wrap exact. The
keys affected are `blood.breakup_period`, `blood.sheet_period`,
`blood.wander_period`, `blood.pool_edge_period`, `grime.period`,
`grime.detail_period`, `grime.crust_period`, `lividity.period` and
`wound.tear_period`. Powers of two are always safe.

## Determinism

Same data, size and seed offset produce byte identical PNGs and a byte identical
manifest. Asserted by building the whole set twice and comparing every byte, not
by trusting it.

Two decisions hold that up:

- **Noise comes from an integer hash, not from numpy's random generator.**
  PCG64's stream is stable in practice but it is not a documented file format,
  and this promises identical output across machines and numpy releases. An
  integer hash is arithmetic, so it cannot drift.
- **The manifest carries no timestamp by default.** A build time would either
  break the promise or force the manifest out of the check, and the manifest is
  the file most worth checking. `--stamp` adds provenance for a one off build.

## Manifest schema

`manifest.json`, schema version 1.

Top level: `schema_version`, `tool`, `size_px`, `texel_density_note`, `unreal`
(the shared import settings, keyed `body` and `wound`), and `entries`.

Each entry: `id`, `kind` (`body` or `wound`), `path` relative to the manifest,
`bytes`, `sha256`, `size_px`, `seed`, `note`, `types`, `asset_name`, and
`params`, which is the fully resolved parameter set that produced the file, so
the manifest is a complete provenance record and a variant can be reproduced
from it alone.

Treated as a contract: the assertion suite checks that every entry points at a
real file whose digest matches, and that every output file appears in the
manifest.

## Unreal import settings, UE 5.8

These are in the manifest per kind. Getting sRGB wrong on a mask is the classic
quiet bug, so it is asserted rather than documented alone.

**Body masks** (`T_ZombieSurface_*`), to `/Game/LastTrain/Zombies/Surfaces`:

| Setting | Value |
|---|---|
| Compression Settings | `TC_Masks` |
| sRGB | **off** |
| Texture Group | `TEXTUREGROUP_Character` |
| Address X and Y | `TA_Wrap` |
| Mip Gen Settings | `TMGS_FromTextureGroup` |

**Wound decals** (`T_ZombieWound_*`), same folder:

| Setting | Value |
|---|---|
| Compression Settings | `TC_Masks` |
| sRGB | **off** |
| Texture Group | `TEXTUREGROUP_Character` |
| Address X and Y | `TA_Clamp` (a decal must not wrap) |
| Compress Without Alpha | **off** (the shape channel is the decal's opacity) |

**One trap, carried forward from the S9 handover row.** When wiring these into a
material, the sampler type has to match the compression: assign the texture to
the `TextureSampleParameter2D` **before** setting `sampler_type`, and set it to
`SAMPLERTYPE_MASKS`. Mismatching it makes the Metal shader compile fail
silently and the Default Material renders instead. Related, from
`known-issues.md` 2.7: after authoring the material, read
`unreal.MaterialEditingLibrary.get_statistics(mat).num_pixel_shader_instructions`
and confirm it is not zero.

## Assumptions, and where they came from

- **The five zombie types** are walker, sprinter, brute, crawler and screamer,
  from `Source/LastTrain/Public/Zombies/LTZombieTypeData.h` and the five
  `DA_Zombie_*` assets. The `types` field validates against exactly that list.
- **Palette** is the four project colours from `docs/art-direction.md`. Used in
  the contact sheet only: the masks themselves are greyscale, and the colour is
  the material's job.
- **Restraint on gore** follows `docs/art-direction.md` section 6: blood
  concentrated on the zombies rather than sprayed uniformly. The variants span
  light to heavy so the material's own `BloodAmount` still has room to dial
  down, and the light variants are the ones assigned to the walker, which is
  most of the horde.
- **Tiling detail, not UV placed.** These are tiling masks because the material
  already tiles one. Placing blood deliberately on the chest and face would need
  the Quinn body UV layout, which is engine content and not reachable from the
  remote lane. That is a real limitation and it is the main thing a later pass
  could improve.
- **Test bands** are the measured range of the committed variants at 512 px with
  headroom, not round numbers chosen to pass. Changing the test size without
  re-measuring makes them meaningless, which the test module says in its
  docstring.

## What this does not do

- It does not touch `M_Zombie_Tintable`, `BP_Zombie` or any `.uasset`. Wiring a
  variant per spawn is an editor task: add the mask parameter to the material,
  then have the zombie pick a variant by its type on spawn.
- It does not vary the body mesh. That is F6 and it needs sourced assets.
- It does not produce hero gore. `docs/art-direction.md` asks for restraint and
  high value moments, which is the opposite of a generated set.

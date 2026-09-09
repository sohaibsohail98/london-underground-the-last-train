# F6: zombie body variation

Lane: Opus in the editor. Fable only if the anim blending misbehaves.
No `Source/` change expected (if one is needed, stop and write a spec).
Depends on: nothing, but do it after F1 so it is tested in the real space.

Read `docs/brief-v3-unreal.md` Part 0 "the honest ceiling" point 3 and
`docs/reference/reference-frame-notes.md`.

## Context

S9 (commit `782f39f`) was a minimal pass: it fixed `M_Zombie_Tintable`
(`used_with_skeletal_mesh` was false, so every zombie fell back to the engine
default mannequin) and added a dead-flesh tint plus baked blood. But every
zombie is still the same single Quinn mannequin mesh with a colour shift. The
handover row for S9 says "City Sample Crowds (varied clothed bodies) is still
the real fix."

## Goal

Varied clothed humanoid bodies for the horde, keeping the five-type system
(walker, sprinter, brute, armour, screamer) and its per-type tint and behaviour
intact.

## Route

1. **Preferred: City Sample Crowds.** The `/Content/CitySample/` pack is
   gitignored but available locally (see `.gitignore`). It ships a crowd system
   with clothed body variation, LODs and an animation set. Wire the zombie
   character to pull a random crowd body on spawn, apply the dead-flesh material
   treatment from `M_Zombie_Tintable` over it, and keep the per-type accent
   tint. Confirm the crowd meshes are Nanite-or-LOD'd enough for 24 to 40 alive.
2. **Fallback: MetaHuman bases.** A small set (4 to 6) of MetaHuman bodies at a
   mid LOD, clothed, with the flesh treatment. Less variety, simpler to wire.
3. Whatever the route, the five `ULTZombieTypeData` assets still drive stats,
   capsule, navigation and behaviour. The armour type still needs its visible
   plate, the brute its bulk, the screamer its silhouette read. The type tint
   (`TintColour` on each `MI_Zombie_*`) still applies as a light accent.

## Rules

- `ABP_Unarmed` (or whatever the current anim blueprint is) drives locomotion.
  `ULTZombieTypeData::AnimPlayRate` is plumbed in C++ but the stock ABP does not
  read it: if a real shamble/lunge anim set goes in as part of this, wire the
  play rate; if not, leave a note.
- No gameplay actor default moves. `MAP CHECK` console exec banned. No mutation
  during PIE.

## Accept

- Play a round in `L_CanaryWharf_Greybox`: the horde is visibly a crowd of
  different bodies, not one mesh repeated, all with the dead-flesh look.
- The five types still spawn with correct stats, silhouette and accent tint
  (spawn a sprinter round at 5 and a brute pair at 10 to check).
- Frame cost with 24 to 40 alive noted for F7.
- Committed via LFS. If `Source/` turned out to be needed, it is a separate
  spec, compiled locally, not slipped in here.

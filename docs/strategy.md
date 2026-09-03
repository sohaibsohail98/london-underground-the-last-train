# Last Train, build strategy

Live working document. Supersedes the credit split in `brief-v2.md` Part 3
where the two disagree, because the phase allocation there assumed Fable would
carry phases 1 to 5.

Note on external references: the `00` to `08` numbered documents and the
`docs/reference/` design notes referred to across the project's planning
material were external Project Knowledge uploads. They were never committed to
this repository and are not present here. The authoritative documents are the
ones in `docs/`. Much of this file also predates the move to Unreal Engine 5
and describes the browser render pipeline; `brief-v3-unreal.md` is the current
plan.

---

## 1. Where we are

| Phase | Planned model | Actually built by | State |
|---|---|---|---|
| 1 plan | Fable | Fable | Merged |
| 2 render pipeline | Fable | Opus | Merged, Gate B unjudged |
| 3 geometry and crowd | Fable | Opus | Merged, Gate C unjudged |
| 4 engine core | Fable | not started | |
| 5 schemas, rounds, stations | Fable | not started | |
| 6 content volume | Opus | not started | |
| 7 interface, audio, save | Opus | not started | |
| 8 balance and optimisation | Opus | not started | |

Both gates are outstanding. Nothing built so far has been seen running on a
GPU. That is the largest single risk in the project and no amount of further
code reduces it.

---

## 2. Model split from here

The honest position on Opus versus Fable for this work:

**Where Fable is likely to be worth the credits.** Anything where correctness
cannot be verified by a typecheck and the failure mode is subtle rather than
loud. Concretely: the volumetric raymarch, the VAT sampling and blend
mathematics, SSR quality at half resolution, and the optimisation pass in
Phase 8 where the job is reading a profile and picking the right three things
to fix. These are the tasks where deeper reasoning about numerical behaviour
pays for itself, and where a subtly wrong implementation still compiles, still
runs, and just looks slightly wrong forever.

**Where Opus is sufficient.** Everything that is volume, schema conformance, or
pattern application against an established convention. Phases 6 and 7 are
almost entirely this: 38 stations of data against a fixed schema, 12 weapons
against a fixed interface, HUD elements, menus, save serialisation, procedural
audio. The brief already allocated these to Opus and that judgement holds.

**The real constraint is not model choice.** It is that nobody has run the
build. A more capable model writing more unverified rendering code makes the
untested surface larger, not smaller. So the sequencing that actually helps is:
judge Gate B and Gate C on real hardware, collect the specific defects, and
then spend Fable credits on the defects rather than on new systems.

**Suggested use of remaining Fable credits, in priority order:**

1. Fix whatever Gate B and Gate C reveal, especially in the volumetric pass
   and the VAT blend, once there are real symptoms to describe.
2. Phase 4's collision and flow field at 60 zombies in a corridor. Crowd
   solver stability under pressure is genuinely hard and expensive to redo.
3. Phase 8's optimisation pass, with a real profile in hand.

Do not spend Fable credits on Phases 6 or 7. That would be paying a premium for
data entry.

---

## 3. Asset strategy, revised

The original brief assumed a sourced rigged humanoid, Mixamo clips, Poly Haven
textures and HDRIs. The build has since removed that dependency:

- **Zombie.** `render/crowd/Humanoid.ts` builds the skeleton, mesh and all six
  animation clips procedurally as pose keyframes. No external rig needed. It
  reads as an articulated mannequin, not a person. Sufficient at torch range;
  swapping in a real glTF later is a loader change, not a rewrite.
- **Textures.** `render/Materials.ts` loads KTX2 from `/assets` when present
  and otherwise generates the whole set procedurally: value-noise concrete, a
  grouted tile lattice, row-offset brick, derived normals and roughness.
- **HDRI.** `render/Lighting.ts` loads RGBE from `/assets` when present and
  otherwise generates a gradient environment biased so that light arrives from
  above, which is how a strip-lit tunnel behaves.

So the project has no blocking asset dependency. Sourcing real assets is now an
upgrade path rather than a prerequisite, and should happen after Gate B, when
there is a rendered frame to compare against.

**Constraint on sourcing.** The build container's network is restricted to
package registries and GitHub. Poly Haven, ambientCG, Mixamo and Sketchfab are
not reachable from it. CC0 assets mirrored in GitHub repositories are. Anything
else has to be downloaded by hand and committed. Record provenance and licence
in `assets/README.md` at download time, not later.

**Constraint on London Underground references.** Geometry and layout are fair
game: ticket barrier forms, escalator proportions, platform edge treatments,
tunnel ring spacing, wayfinding panel shapes. Trademarks are not: no roundel,
no Johnston or New Johnston, no official line diagram, no recorded
announcements, no operator livery. All in-world advertising and wayfinding must
be original artwork generated in code. This is a hard line, not a preference.

---

## 4. Lighting and readability

The brief's premise is that darkness is a budget multiplier and the torch is
the signature effect. That remains true for atmosphere and for cost. It is not
sufficient for playability: a horde you cannot see is not frightening, it is
just unfair.

The resolution now in the build is a three-layer scheme:

1. **Readability floor.** An ambient term plus a player fill light, both
   deliberately not physically motivated. Geometry, zombie silhouettes and the
   platform edge are always legible.
2. **Station lighting.** Emissive strips and the eight-light budget give the
   space its shape and carry the palette. This is what a blackout removes.
3. **Torch.** Contrast and directed attention on top of the other two, not the
   sole source of information.

A blackout now drops station lighting to nothing and the readability floor to
roughly a fifth, rather than to black. Master brightness is exposed as a
setting because acceptable levels vary enormously between panels.

---

## 5. Remaining work

### Phase 4, engine core

Fixed timestep is already in place. Remaining: input rebinding, capsule versus
capsule push-apart with sleeping for settled pairs, the 4Hz flow field with
water and barrier cost weighting, head hitboxes at 2.5x, hit markers, and the
health and regeneration model with the damage feedback already wired to the
post chain. Target: 60 zombies pressed into a corridor stays stable.

### Phase 5, schemas, rounds, stations

StationDef and WeaponDef are already written and validated. Remaining: the
round manager, the breather, the train timer with real arriving geometry and
door animation, station heat, travel between stations, the five zombie types
with their scaling, and the three reference stations at Canary Wharf,
Whitechapel and Paddington.

### Phase 6, content volume

38 stations as data. 10 further weapons. Three attachment slots per gun with
real stat effects. Four perks. Lost property office and upgrade bench. Oyster
Credit. No engine changes permitted in this phase, and no new mechanic ids or
tile characters.

### Phase 7, interface, audio, persistence

DOM HUD, menus with a slow orbit of a generated station, gunsmith restricted to
breathers and the train, stats, localStorage save checkpointed on arrival, and
fully procedural audio through Web Audio with SpeechSynthesis announcements
from each station's three original lines.

### Phase 8, balance and optimisation

Balance to round 12 to 15 on a first serious run, 30 as an achievement. Profile
first, then fix the top three costs. Verify the WebGL2 path. Android via
Capacitor is a stretch goal only.

---

## 6. Immediate next actions

1. Run the build. Judge Gate B: does the station read as a real place, does the
   torch read as a light in the air, do the pillar shadows on wet ground sell
   the space.
2. Judge Gate C: 46 zombies at 60fps on the largest station, no LOD popping, no
   VAT blend artefacts.
3. Write down specific defects. Then decide what goes to Fable.
4. Only then start Phase 4.

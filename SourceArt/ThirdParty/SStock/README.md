# S Stock carriage, CC-BY source mesh

One real sub-surface stock driving carriage, CC-BY-4.0, de-branded before it was
committed.

Read `LICENCE.txt` in this folder before doing anything with it. It carries the
credit the licence requires and the record of what was stripped.

## Read this first: it is not the train the game is building

`docs/reference/canary-wharf-research/rolling-stock.md` settles the question and
the answer is a Class 345 Aventra silhouette, main line loading gauge.
`docs/tasks/phase-f3-train-exterior.md` specs that vehicle. This carriage is a
different one, and the numbers say so:

| | This mesh, measured | F3 target |
|---|---|---|
| Car length | 17.92 m | 23.6 m driving, 22.5 m intermediate |
| Body width | 2.97 m | 2.772 m |
| Rail to roof | 3.61 m | 3.760 m |
| Doors per side | 4 double-leaf | 3 double-leaf plug |
| Cab front | Raked flat panes | Full width curved wraparound |

So it is **not** a drop-in for F3, and importing it and calling the train done
would ship the wrong vehicle. What it is good for, in descending order of value:

1. **Proportion and detail reference that exists in three dimensions.** Sub
   surface gauge is far closer to main line gauge than any deep tube stock, so
   the cross section, the door leaf proportions, the grab pole layout, the seat
   spacing and the underframe read across almost directly. Measuring off a mesh
   beats measuring off photographs.
2. **A kitbash base.** Stretching the body, replacing the cab front and
   redistributing the doors is less work than modelling a carriage from nothing,
   and the interior fittings transfer wholesale to F4.
3. **A blockout that fills the platform today.** Dropping it in gives F1 and F2
   a train-shaped wall to light against while F3 is still open.

Whichever route, the livery, the lettering and the station mark are original
work. Nothing about this mesh changes that.

## What is here

| File | Notes |
|---|---|
| `scene.gltf` | glTF 2.0, 143 meshes, 26 materials, one animation. Rewritten by the cleaning script, so it is indented and diffable. |
| `scene.bin` | 24.7 MB of vertex and index data. Plain binary, not LFS. |
| `textures/Electric_Warning_baseColor.png` | The one surviving texture. Generic BS EN ISO 7010 W012 hazard triangle, 1024 square. |
| `LICENCE.txt` | Attribution and the statement of changes. |

## Measurements taken from the file

One glTF unit is one metre, so the import scale into Unreal is 100.

| Axis | Extent | Real S Stock, for comparison |
|---|---|---|
| Width | 2.969 m | about 2.92 m |
| Height | 3.611 m | about 3.68 m |
| Length | 17.917 m | about 17.4 m per car |

Bounding box runs from `(-1.483, 0.138, -8.077)` to `(1.486, 3.749, 9.839)` in
glTF axes, which are Y up. The root node `Sketchfab_model` carries the Y up to Z
up correction matrix, so let the importer apply it rather than rotating by hand.
The lowest point is wheel tread, not carriage floor.

## The door animation

There is one animation, 7.92 seconds, linear, driving `translation` on six
nodes: `Doors_6`, `Doors.001_56`, `Empty_57`, `Doors.002_58`, `Doors.003_59`,
`Empty.001_60`. Between 83 and 86 keys each. It is a sliding open and shut
cycle, not a skinned rig, so it imports as a rigid-body animation on a
hierarchy rather than as a skeletal mesh.

That matters even though the mesh is not the final vehicle. The dwell logic in
`Train/LTTrain.cpp` already exposes presentation hooks with nothing attached to
them, and this clip is the first thing that can be attached: split it into an
open section and a shut section and drive them from the hooks. The timing and
the hook wiring then survive whatever mesh eventually replaces this one.

## Triangle budget, and why this is not drop-in

579,544 triangles for one carriage. The author states plainly that it is not
presented as game ready, and it is not:

- Nanite handles the count, but Nanite on Apple Silicon through Metal is not
  free. `docs/tasks/phase-f7-perf-pass.md` is where this gets measured. Until
  then treat the raw mesh as a blockout, not as the shipping asset.
- Most materials are flat colour with no maps at all. There is no PBR texture
  set to speak of. Re-authoring surfacing is Phase F3 work and the CC0 metal,
  glass and painted surfaces listed in `docs/reference/asset-sources-phase-f.md`
  are what it should be re-authored from.
- The interior is modelled but sparse. `docs/tasks/phase-f4-train-interior.md`
  owns filling it.

If the count becomes the bottleneck, the two levers in order are: decimate the
roof and underframe, which between them are 103k and 65k triangles and are
barely ever seen, then merge the many small parts into fewer sections.

## The two placeholder materials

`Station_Mark_Placeholder` (144 triangles) and `Door_Warning_Placeholder`
(24 triangles) are flat grey panels with no texture. They are where the
stripped operator branding used to be, and they are the mounting points for the
original station mark and original door wording that Phase F5 authors. See
`docs/tasks/phase-f5-signage-wayfinding.md`.

Leaving them grey in a build is acceptable. Putting anything resembling the
original back on them is not.

## Importing into Unreal 5.8

1. Import `scene.gltf` into `Content/LastTrain/Environment/Train/`, under a
   name that says it is a blockout. The
   Interchange glTF importer is on by default in 5.8.
2. Uniform import scale 100. Do not tick "Force front X axis"; the root node
   already carries the axis correction.
3. Import the animation. It lands as a level-sequence-friendly rigid hierarchy,
   so keep the node names intact.
4. Enable Nanite on the static meshes after import, then profile before
   assuming it is fine.
5. Add the row to `Content/ATTRIBUTION.md` if it is ever re-imported under a
   different name. The existing row covers this copy.

## Re-deriving this folder

```
python3 tools/asset-fetch/clean-sstock.py <unpacked-sketchfab-download> SourceArt/ThirdParty/SStock
```

The script is deterministic and refuses to run against a download that does not
contain the three files it exists to remove.

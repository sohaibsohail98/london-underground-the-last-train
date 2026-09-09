# SourceArt

Pre-import art in its original interchange format: glTF, FBX, OBJ, source PNG.
Unreal never reads this folder. `Content/` holds the imported `.uasset` result,
this holds what it was imported from.

It exists because the interesting third party assets are not `.uasset` files.
They are meshes off Sketchfab and textures off ambientCG, and once imported the
provenance disappears into a binary. Keeping the source beside the licence
means the licence survives.

## What may be committed here

Only assets whose licence permits redistribution of the raw file:

- **CC0** and public domain. No conditions.
- **CC-BY**, with the credit reproduced in a `LICENCE.txt` beside the files, and
  a statement of changes if it was modified.
- **MIT** and similar permissive terms.
- Our own work.

## What may never be committed here

- **Anything under a UE-Only Content or Fab Standard licence.** Almost all Epic
  and Fab content is one of those two. They let us ship the asset compiled into
  the game and sell that game. They do not let us re host the raw file in a
  public repository. Those packs are fetched per
  `docs/reference/free-assets.md` and land in gitignored folders under
  `Content/`.
- **Anything carrying an operator trademark.** No station mark, no Johnston or
  New Johnston lettering, no official line diagram, no operator livery. This
  applies to a texture inside an otherwise free mesh just as much as to a
  standalone file. Strip it before committing, script the strip so it is
  repeatable, and record it. `tools/asset-fetch/clean-sstock.py` is the worked
  example.
- **Anything licensed for personal, educational or research use only.** That
  includes the BBC RemArc sound archive. Fine to work with locally, not fine to
  commit and not fine to ship.
- **Anything whose licence you have not actually read.** "Free download" is not
  a licence.

## Layout

```
SourceArt/
  ThirdParty/
    SStock/     CC-BY-4.0 sub-surface stock carriage, de-branded
```

## The attribution chain

Three places, all of which must agree:

1. `LICENCE.txt` beside the files here. The full terms and the credit text.
2. `Content/ATTRIBUTION.md`. One row per asset, filled in at import time.
3. The in-game credits screen, once there is one. Phase F8 owns that.

A CC-BY asset that reaches a build without appearing in the credits is a licence
breach, not an oversight to fix later.

## Git and file size

Nothing here is tracked by Git LFS today. `scene.bin` is the largest file at
24.7 MB and compresses to about 8 MB in the object store, which is tolerable for
a one-off hero asset. If this folder grows past a handful of meshes, move it to
LFS before it gets worse. That needs the `git-lfs` binary installed, which is
why it has not been done from a remote session.

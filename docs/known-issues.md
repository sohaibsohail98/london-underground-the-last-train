# Known issues

Open problems, environment limits and unverified work. Kept so a cold session
does not rediscover them and so nothing quietly rots.

**Update this whenever an issue is closed or a new one is found.** An issue that
is fixed gets deleted from here, not ticked: git history is the record.

Last updated 2026-09-09.

---

## 1. What the remote session cannot do

These are hard limits of the remote Claude Code lane, not bugs. They shape what
that lane can be asked for. See `docs/tasks/who-does-what.md` for the split.

### 1.1 No network route to any asset host

The remote container's egress proxy allows GitHub, the Anthropic API and the
language package registries. **Everything else is refused at CONNECT with a
403:** ambientcg.com, polyhaven.com, quaternius.com, kenney.nl, freesound.org,
itch.io, wikimedia.org. Both `curl` and the agent's own page-fetch tool hit the
same wall.

Web search still works. So the remote lane can **research** assets and cannot
**download** them.

**Retested 2026-09-09, and the allowlist is narrower than "GitHub only" but
wider than nothing.** Reachable: `raw.githubusercontent.com` for individual
files from any public repository, `fonts.googleapis.com`, `fonts.gstatic.com`,
and the language package registries. Refused at CONNECT: polyhaven.com,
ambientcg.com, sketchfab.com, kenney.nl, quaternius.com, itch.io,
opengameart.org, commons.wikimedia.org, freesound.org. `api.github.com` answers
but rejects any repository not attached to the session, so directory listings
need `add_repo` first; raw file fetches do not.

The practical consequence is small but real: **anything living in a public
GitHub repository can be fetched remotely.** That covers the OFL font families,
which is how the Overpass variable fonts were checked. It does not cover a
single one of the texture, mesh, audio or HDRI sources, all of which host their
own downloads.

**Consequence for task specs:** never ask the remote session to fetch, stage or
download a file. Ask it for a manifest, and run the fetch here.

**One route around this, proven 2026-09-09.** Files attached to the session
prompt arrive inside the container without touching the network. The S Stock
carriage at `SourceArt/ThirdParty/SStock/` got in that way: downloaded on the
human's machine, attached to the request, processed and committed remotely. So
for anything small enough to attach and permissively enough licensed to commit,
the remote lane can do the whole job. It still cannot go and get it.

### 1.2 Nothing staged in the remote container can reach this machine

`_incoming_assets/` is gitignored (`.gitignore` line 80), committing assets is
forbidden, and the container is reclaimed when the session ends. A file the
remote session downloads has no route out **even if 1.1 were fixed.**

This is the more fundamental of the two, and it applies to any gitignored
output, not just assets.

**Unless the asset can be committed**, which is the exception the S Stock proved.
CC0, CC-BY and MIT assets go in `SourceArt/` and travel out on the branch like
any other file. The blocked case is specifically UE-Only and Fab Standard
content, which cannot be committed at all, and anything too large to be worth
carrying in git.

### 1.3 No compile

Unreal cannot be installed in the container, and the external Xcode mount is not
there. The remote session writes C++, and the human or the self-hosted runner
compiles it with `./tools/ci/compile.sh`. Already stated in `CLAUDE.md`;
repeated here because it is the same class of limit as the two above.

**Consequence:** the remote lane should not push C++ it cannot at least reason
its way to confidence about, and a small change beats a large one. Where a C++
addition would be nice but is not required, propose it in the spec rather than
writing it blind. `docs/tasks/phase-g2-hud.md` does this with
`GetReloadProgress()`.

### 1.4 The five CI gates are not a compile

`tools/ci/check_*.py` catch style, spelling, dashes, LFS pointer integrity and a
stand-in subset of reflection errors. **A green gate run says nothing about
whether the module builds.** `check_cpp_reflection.py` is explicitly the
compiler stand-in and covers only missing `GENERATED_BODY()`, a reflected header
not including its own `generated.h`, a `.cpp` not including its own header
first, and `LT_LOG` format specifier mismatches.

---

## 2. Open items from the 2026-09-09 remote session

### 2.1 `fetch-phase-f.sh` has never run against a live host

`tools/asset-fetch/fetch-phase-f.sh` was written in the remote container, which
cannot reach ambientcg.com or polyhaven.com. It is verified for **syntax, dry
run, and graceful failure against the blocked proxy** and nothing else.

Specifically unproven:

- The ambientCG zip URL shape `https://ambientcg.com/get?file=<ID>_2K-JPG.zip`.
  There is a fallback through the documented v2 CSV API, and that is unproven
  too.
- Whether all 36 ambientCG asset IDs resolve. `TactilePaving003`, `004` and
  `005` were **inferred from the family naming pattern**, not seen.
- The Poly Haven API response shape the script parses, and whether all 22 slugs
  exist.

**Owner: whoever runs it first.** It fails per-asset and reports what failed, so
a wrong URL costs a line in the failure list rather than the run. Expect to fix
something on the first go.

### 2.2 Every licence in the Phase F manifest is unverified

`docs/reference/asset-sources-phase-f.md` rests on web-search snippets. **No page
was opened and no file was looked at.** Only two per-file licences in the whole
document were confirmable from search evidence, both Wikimedia Commons files in
section 4.3.

Confirm the licence on the page before any file is used. Wikimedia Commons and
Freesound in particular are mixed-licence: the licence is a per-file fact, never
a per-site fact.

### 2.3 Four unresolved legal flags on Phase F assets

Listed in full in section 6 of `docs/reference/asset-sources-phase-f.md`. In
brief: the signboards and posters in the Modular Underground Metro pack; the bus
stop in City Environment Pack #2; photogrammetry scans that may carry painted
markings (ambientCG `Road001` named specifically); and the FOI re-use notice
attached to the Class 345 drawings, which nobody has read.

None is likely to be a problem. All four need eyes before the file is staged.

### 2.4 `docs/art-direction.md` section 7 is stale

It says Overpass "is not present in the project or the engine". The OFL faces
were imported in the S9 haul and are in `Content/LastTrain/UI/Fonts/`
(`Font_UI_Overpass`, `Font_UI_OverpassMono`, `Font_UI_Barlow`,
`Font_UI_BarlowCondensed`, `Font_UI_PublicSans`).

Fixing it is in the accept list of `docs/tasks/phase-g2-hud.md`, so it closes
with G2. Flagged here in case anything reads section 7 before then.

### 2.5 Two owner decisions are blocking nothing yet, but will

Both are stated in `docs/tasks/phase-g2-hud.md`:

- **The typeface split.** `WBP_MainMenu` uses Barlow plus Public Sans, the HUD
  spec calls for the Overpass pair. Three families across two screens is not
  defensible. The spec recommends one display face (Barlow Condensed, titles
  only) and one functional pair (Overpass, Overpass Mono).
- **The station schematic.** The Phase G plan lists it as the HUD's map
  substitute. The G2 spec argues it should be cut from the HUD entirely while
  v1 ships two stations, and built as diegetic platform signage in F5 instead,
  where it also does the legal job of replacing the official line diagram.

---

## 3. Repo hygiene

### 3.1 The discarded web build was removed

Done 2026-09-09. The Three.js tree that preceded the Unreal build, 66 files and
568K, is out of the working tree and preserved at the `web-threejs-final`
tag (`adeb32d`). The `phase-03` tag does not contain it.

**Recovering it:** `git ls-remote --tags origin`, not `git tag -l`. A fresh
clone may not fetch tags, which made the tag look missing during the survey.

`docs/reference/canary-wharf-grid.md` cited two files in that tree as the
provenance of its tile legend. It restates the legend and every constant in full
itself, so the citation was repointed at the tag and nothing was lost. `README.md`,
`CLAUDE.md`, `docs/brief-v3-unreal.md` and the now-dead skip in
`tools/ci/check_docs.py` were updated with it.

### 3.2 Dead references to retired task specs

Commit `bcd947a` retired 14 completed task files. Two live documents still point
readers at them as if they were readable:

- `docs/tasks/neostack.md`, the outstanding-editor-task list, cites
  `phase-c1-train.md` (twice), `phase-c-zombie-types.md`,
  `phase-b2-interaction.md` ("steps 3 to 7"), `phase-b3-feedback-widgets.md` and
  `phase-a4-editor-setup.md`. These are instructions to go and read a file that
  is not there.
- `README.md` pointed at `docs/tasks/phase-a4-editor-setup.md` as a how-to.

Both fixed 2026-09-09 by pointing at the retiring commit instead. Listed here so
the pattern is recognised: **when a task file is retired, grep for its name
first.**

### 3.3 Not dead, do not "fix"

Two families of reference look broken to a link checker and are deliberate:

- **The `00` to `08` series and `phase-1-plan.md`.** External Project Knowledge
  uploads, never committed. `docs/brief-v3-unreal.md` and
  `docs/art-direction.md` both say so in their own text.
- **`open-questions.md`, `open-questions-review.md`, `open-questions.json`.**
  Removed deliberately; `docs/design/gameplay-canon.md` replaced them and names
  commit `0b7e35f` as where the full sweep lives.

Also fine: `_incoming_assets/ASSET-RESEARCH.md` (gitignored by design), `.mcp.json`
(gitignored), and the engine's own `BaseGame.ini` / `MacGame.ini` / `IOSGame.ini`
cited in `handover.md`, which are engine-side and correctly qualified.

### 3.4 Images are committed raw, not through LFS

`.gitattributes` marks `*.png` as `binary` but **not** as an LFS filter, so PNGs
go into the pack as raw blobs. There is one such file left,
`docs/reference/reference-frame.png` at 2.3 MB, and it is the project's visual
target so it has to be somewhere.

`UI.png`, a byte-identical duplicate of it sitting at the repo root and
referenced by nothing, was removed 2026-09-09.

Worth deciding, low priority: move `*.png` onto the LFS filter like `*.tga` and
`*.exr`, or accept the 2.3 MB. Moving it rewrites nothing already committed, so
the existing blob stays in history either way.

### 3.5 SourceArt is committed raw, not through LFS

`SourceArt/ThirdParty/SStock/scene.bin` is 24.7 MB and goes into the pack as a
raw blob, as does the 271 KB PNG beside it. Neither `*.bin` nor `*.gltf` is on
an LFS filter, and `*.png` is not either, per 3.4.

It compresses to about 8 MB in the object store, so it is tolerable for one
hero asset and was committed rather than blocked on tooling: the remote
container has no `git-lfs` binary, so a session there cannot add a path to the
LFS filter and then commit through it without writing a broken pointer.

Worth doing on the machine that has `git-lfs`, before `SourceArt/` grows: put
`*.bin` and `*.gltf` under the LFS filter and migrate what is there. Low
priority at one asset, rising with each one added.

# Phase H asset sources: FP arms, weapon meshes, weapon animation

Researched 2026-09-10, per `docs/tasks/phase-h-weapon-presentation.md`. Same
licence discipline as `docs/reference/asset-sources-phase-f.md`: this is a
shopping list, not a haul. Every entry rests on search result evidence only.
No page was opened, no licence text was read on the page itself, no mesh or
animation was previewed. Entries the search did not settle are marked
UNVERIFIED. Confirm the licence and price on the page before staging or
importing anything, and treat the flags in the closing section as work to do,
not as findings.

**No download route exists from this research session.** As with Phase F, this
document is the transferable half of the job: which asset, from where, under
what licence, for which need. CC-in-Unreal (the session with editor access) is
the one that opens the page, confirms the price and licence line, downloads,
and imports.

## Why this is the actual blocker on Phase H

`phase-h-weapon-presentation.md` is explicit that the animation Blueprint,
montage and Blueprint wiring work is not the hard part: `ULTWeaponComponent`
already exposes `GetAimAlpha()`, `IsAiming()`, `IsReloading()`,
`OnAmmoChanged` and `OnHitConfirmed`, everything an anim Blueprint state
machine needs. What does not exist in `Content/` is anything to animate: no FP
arms skeletal mesh, no weapon mesh. This document exists to unblock that.

---

## 1. FP arms skeletal mesh

### 1.1 Requirements

- No operator likeness, no real name, no branding on the mesh or textures
  (gloves, sleeve patches, watches with invented logos are fine so long as
  they are original art).
- Compatible with UE 5.8: either rigged to the Epic UE4/UE5 mannequin skeleton
  (so it retargets cleanly against Manny/Quinn-based content already in
  `Content/`) or a Mixamo-derived rig that can be retargeted with an IK
  Retargeter pass.
- A skinned mesh that can drive at minimum idle, fire, reload and aim
  in/out on a state machine, so it needs to be a proper skeletal mesh with a
  working hand/finger rig, not a static prop glued to the camera.

### 1.2 Candidates

| Source | Asset | Licence | Verdict |
|---|---|---|---|
| **Fab** | **First Person Base Arms Vol. 1, Skinny Male** (`https://www.fab.com/listings/66df04fa-c347-489b-a635-5663135ac57c`, also listed on ArtStation Marketplace) | **Paid, USD 9.99 per search snippet, UNVERIFIED on the page.** Standard commercial licence implied, not confirmed. | Rigged to the UE4 default mannequin skeleton and designed as a drop in replacement for the mannequin arms, so it retargets cleanly. No animations included, it is a base mesh only, which matches this project's need to author its own montages. A small, low risk purchase if CC-in-Unreal has budget; otherwise treat as a fallback. |
| **Fab** | **FPS Arms Pack** (`https://www.fab.com/listings/1ae7dcc1-2e85-4967-8673-3030bb7a59d4`) | UNVERIFIED, price not established from search. Snippet describes it as a bare arms skeletal mesh rigged to the default Epic Skeleton with three glove variants, also on the Epic Skeleton. | Strong candidate on paper: Epic Skeleton compatibility means it should retarget against any Mixamo or Marketplace animation set without a bespoke retargeter chain. Open the page to confirm price before committing. |
| **Fab** | **MODULAR FPS ARMS PACK** (`https://www.fab.com/listings/da81324b-3470-4937-a049-1b62376eea40`) | UNVERIFIED. | Modular: separate gloves, sleeves (bare arm, shirt, rashguard), watches, patches. Useful if the project later wants a customisable look, but adds import complexity Phase H does not need. Lower priority than the two above unless modularity is wanted now. |
| **Fab** | **Lowpoly First Person Arms, FPS Arms, 3D Pixel Style** (`https://www.fab.com/listings/22de203e-a284-4801-a5c5-08e5a104cd28`) | UNVERIFIED. | Rigged to UE4 and UE5 skeleton, five skin colour textures. Stylised low poly look is off the project's visual target (the reference frame is not a pixel art game), so this is a grey box stand in only, not a Phase F/H shipping asset. |
| **Sketchfab** | Various, for example "First Person arms" by DJMaesen (`https://sketchfab.com/3d-models/first-person-arms-e3c42c05b22944e5839deb8e003f0987`), "First Person hands rigged" by David Fischer, "Low-poly FPS Arms (Rigged)" by INKXO, "Rigged FPS Arms" by RafaP | **Mixed, mostly CC-BY per Sketchfab's norm (see Phase F precedent, section 2.4 "Sketchfab generally").** DJMaesen's listing is confirmed CC Attribution from the search snippet; the rest are UNVERIFIED per file. | Free to download but each needs its own licence read and, if CC-BY, a credit line the project would have to carry. Sketchfab downloads are also frequently account gated. Treat as a last resort behind the two options below, and never take one without reading the page's exact licence block, the same rule Phase F applied to Sketchfab. |
| **Mixamo derived** | Extract arms from a full Mixamo character | **Free, Adobe's own terms: royalty free for commercial or non-commercial use, no attribution required.** The one real restriction is redistribution: Mixamo characters and animations cannot be repackaged and resold as standalone assets, they must be incorporated into a project. That restriction does not block using them inside this game. | The cheapest genuinely free route with no purchase and no attribution burden. The trade-off is manual work: pick a plain Mixamo character (avoid anything overtly branded or a licensed likeness, Mixamo's stock characters are original enough), isolate the forearms/hands in a DCC tool (Blender is the documented workflow, see the Medium walkthrough "Extracting First-Person Arms from Mixamo Characters in Unreal Engine 5"), then bring the isolated mesh into UE5 rigged to the Mixamo skeleton and retarget it with an IK Retargeter against Manny. More setup than a ready made FP arms pack, but zero cost and zero licence risk. |
| **Quaternius** | No dedicated FP arms mesh found. Quaternius's humanoid characters (CC0) could theoretically be used the same way as the Mixamo extraction route, but no first person specific rig was found in the sweep. | CC0 where it exists. | Not a direct hit. Worth a follow up search only if the two Fab options and the Mixamo route are all rejected. |

### 1.3 Recommendation for the arms mesh

**Start with the Mixamo extraction route if the editor session has Blender
available, otherwise buy Fab's "First Person Base Arms Vol. 1".** The Mixamo
route is genuinely free and licence clean, and the project already treats
Blender as available tooling elsewhere in the pipeline (see the Phase F
document's modelling notes). If Blender access or the extraction workflow is
not practical in the CC-in-Unreal session, the USD 9.99 Fab base arms mesh is
a small, low risk spend that gives a ready rigged asset with no animations
bundled, meaning it will not fight the project's own montage authoring. Either
choice is Epic/Mixamo skeleton compatible so it retargets against the fire and
reload animation candidates in section 3.

---

## 2. Weapon meshes: generic pistol and 1 to 2 other small arms silhouettes

### 2.1 Requirements, restated from `CLAUDE.md`

No Call of Duty weapon or attachment naming. No recognisable real firearm
replica: this is stricter than a licence question, it is the project's own
legal flag, the same instinct that excluded the Sketchfab R46 subway car in
Phase F for carrying a real operator's marks. A pistol mesh that is a faithful
dimensional copy of, say, a Glock 17 or a Beretta 92 is a legal flag even if
its licence is CC0, because trade dress and product configuration protection
exist independently of the 3D file's copyright licence. The brief for
`DA_Weapon_SMG` ("Stag Compact") already sets the precedent: an original name
on a plausible but not 1:1 replica silhouette.

### 2.2 Candidates

| Source | Asset | Licence | Verdict |
|---|---|---|---|
| **Quaternius / Poly Pizza** | **Scifi Pistol** (`https://poly.pizza/m/U0Q1BrKL1y`) | **CC0, verified pattern: Quaternius's whole catalogue is CC0, confirmed by multiple Phase F entries and repeated here from the same search evidence.** No account, no attribution. | **First pick for the starting pistol silhouette.** A sci-fi styled pistol is furthest from any real firearm's trade dress, which sidesteps the legal flag entirely rather than merely avoiding a name match. Low poly, so it will read as a placeholder against the reference frame's fidelity target, same caveat Phase F gave every Quaternius mesh: proportion and blockout reference, not a hero shipping asset without retouching. |
| **Quaternius / Poly Pizza** | **Revolver** (`https://poly.pizza/m/E7IaG9TptR`) | CC0, same basis as above. | A revolver silhouette is generically shaped enough across real world designs that it is lower legal risk than an automatic pistol copy, and it gives visual distinction from the Scifi Pistol if a second sidearm silhouette is wanted. |
| **Quaternius / Poly Pizza** | **Animated Pistol** (`https://poly.pizza/m/gmR2e0hWSF`) | CC0. | Listed because it ships with animation already; see section 3, it is unlikely the animation is FP arms rig compatible (Poly Pizza models are usually a static rig or a simple turntable), but worth opening to check before assuming it is unusable for animation. |
| **Quaternius / Poly Pizza** | **Rifle** (`https://poly.pizza/m/cCAgiMOQow`) and the **Ultimate Guns Pack** bundle (`https://poly.pizza/bundle/Ultimate-Guns-Pack-cpgUfI4t2F`, 25 models, FBX/OBJ/glTF, CC0, no login) | CC0. | The bundle is the efficient route to a second and third small arms silhouette (an SMG-shaped and a rifle-shaped mesh) in one download, covering the `phase-h1-starting-loadout.md` sidegrade/upgrade tiers without three separate sourcing trips. Same low poly caveat as above: verify visually before treating as a final asset rather than a grey box stand in. |
| **Fab** | Various paid "Animated FPS [X] Pistol Pack" listings (Animated FPS 5 Pistol Pack, Animated Austrian Pistol FPS Weapons Pack, FPS Animated Pistol Weapon Pack) | **Paid, roughly USD 150 value per the AAA-tier snippets found; price UNVERIFIED per exact listing.** | **Excluded from the recommendation on the legal flag, not the licence.** These are explicitly modelled on real, named firearms (one listing names an Austrian pistol design directly in its title, i.e. a Glock family clone). Even with a full commercial licence purchased, shipping a modelled-to-spec replica of a named real handgun risks the exact trade dress problem `CLAUDE.md` is written to avoid, and several list a specific real design by identifying nickname. Do not use these as the mesh source; they may still be worth having purely as reference for how a professional pistol rig's bone and socket layout is structured, nothing more. |
| **CGTrader / TurboSquid** | Not searched in detail, listed for completeness | Per Phase F precedent (section 2.4 exclusions): mixed, mostly royalty-free-with-terms, per-model EULAs that each need reading, and pricing generally higher than Fab equivalents. | **Excluded**, same reasoning as Phase F excluded them for meshes generally: no reason to re-open this category when Quaternius covers the need at CC0. |
| **Kenney** | No dedicated modern firearm pack found in this sweep; Kenney's `Blaster Kit` exists (sci-fi ray guns) but was not directly confirmed in this search. | CC0 if it exists, per Kenney's site-wide licence. | Worth a five minute follow up look if the Quaternius sci-fi pistol reads too toy-like: Kenney's blaster aesthetic is a plausible generic sidearm silhouette that is legally the same safe distance from a real firearm as the Scifi Pistol. Not confirmed present, flagged as UNVERIFIED rather than recommended outright. |

### 2.3 Recommendation for weapon meshes

**Use Quaternius's Scifi Pistol (CC0, via Poly Pizza) as the starting pistol
mesh, and pull one or two more silhouettes from the Ultimate Guns Pack bundle
for the sidegrade and upgrade tiers in `phase-h1-starting-loadout.md`.**
Everything is CC0, no account, no attribution, and a stylised sci-fi design is
the cleanest way to satisfy the "not a recognisable real firearm replica"
rule categorically rather than case by case. The low poly fidelity is a real
gap against the Phase F art target, so flag these as placeholder-quality in
`docs/known-issues.md` once imported, the same way Quaternius meshes are
flagged as reference-only everywhere else in the project's asset research.

---

## 3. Fire and reload animation sources

### 3.1 The honest assessment first

**Hand authoring in engine is the most realistic path for anything beyond a
first pass.** No free, licence clean, ready made animation set was found that
is confirmed to (a) target the same skeleton as a free or cheap FP arms mesh
from section 1, (b) cover idle/fire/reload/aim in/aim out specifically, and
(c) carry a licence that does not require either payment or a credit line.
The candidates below either cost money, are UNVERIFIED on licence, or require
a retargeting step that is realistically as much work as blocking out a
simple fire and reload montage by hand in the Sequencer using the existing
arms rig. Budget for hand authoring as the primary route, and treat any of
the below as a time saver only if it is confirmed cheap or free once opened.

### 3.2 Candidates

| Source | Asset | Licence | Verdict |
|---|---|---|---|
| **Quaternius** | **Universal Animation Library** (`https://quaternius.com/packs/universalanimationlibrary.html`, also on itch.io and OpenGameArt) and **Universal Animation Library 2** (`https://quaternius.com/packs/universalanimationlibrary2.html`) | **CC0, confirmed: "free to use in personal, educational and commercial projects", no attribution.** | **Best free lead found.** 120+ (library 1) and 130+ (library 2) animations including combat and gun actions, on Quaternius's own universal humanoid rig, documented as retargetable in Unreal via an Epic community tutorial ("Getting Started with Free Character Animations from Quaternius"). The catch: this is a full body humanoid rig, not an FP arms-only rig, so using it means either retargeting a full body gun animation down onto the FP arms skeleton (extra work, may not translate cleanly to a first person camera-relative pose) or using it as motion reference to hand key the FP arms montage. Confirm the exact animation list (does it actually include a fire and a reload cycle, not just idle/walk/run with a gun held) before relying on it. |
| **Mixamo** | Any Mixamo gun/rifle idle, aim, fire animations in the standard library, for example "Rifle Aiming Idle", "Firing Rifle" style clips (exact clip names UNVERIFIED, not individually searched) | **Free, same Adobe terms as section 1.3: royalty free, no attribution, redistribution of raw files as a standalone asset forbidden.** | Same trade-off as the Mixamo arms extraction: full body third person animations, so a fire/reload pose would need reworking into a first person camera-relative montage rather than a drop-in. Most useful as motion reference or a starting pose to hand key from, same as the Quaternius library. |
| **Fab** | **Ultimate FPS Animations Pack** (`https://www.fab.com/listings/10d385c9-7cff-41e2-9322-6f74f5ce0ec2`) | UNVERIFIED, likely paid given the "28 weapon models" AAA-tier scope described. | If genuinely FP arms rig animations (Idle, Draw, Equip, Fire, Fire ADS, Idle ADS, Reload, Reload ADS, Reload Empty, Reload Empty ADS per the snippet) this is an exact match for Phase H's state list, including the aim in/out and empty-reload variants the spec does not strictly require but would be nice to have. Open the page to confirm price and exactly which skeleton it targets before relying on it; a pack targeting a specific paid arms mesh (its own bundled arms, not a generic Epic Skeleton) would not retarget cleanly onto a different FP arms mesh from section 1. |
| **Fab** | **Tactical FPS Shooter Pack** (`https://www.fab.com/listings/76c5f2b0-ea55-461c-8b3c-811a9deb3279`) | UNVERIFIED, described as 7 unique animated weapon sets with models, sounds and VFX bundled. | Bundling weapon meshes with the animation is a plus (one source for mesh and animation, less retargeting risk) but a minus for reusing the CC0 pistol from section 2, since the meshes and animations are presumably paired. Only worth it if the bundled weapon silhouettes are themselves generic enough to pass the legal flag in section 2.1, unconfirmed from the snippet. |
| **UE Marketplace/Fab** | "Animated FPS M4A1 Assault Rifle Pack" and similar named-weapon packs | UNVERIFIED, but the name alone is disqualifying. | **Excluded on the same legal flag as section 2.2's named pistol packs.** An M4A1 branded pack is both a real weapon name (contrary to `CLAUDE.md`'s no-real-firearm-name rule, which the task spec extends from Call of Duty naming to the whole project) and almost certainly a modelled replica. Do not use, even for animation reference the naming alone should be avoided if a screenshot or asset browser entry could surface the string "M4A1" anywhere near this project. |
| **Reupload/aggregator sites** | "gameassetsfree.com", "ue3dfree.com", "assetfreaks.com", "unrealmix.com" and similar, surfaced repeatedly in the weapon and FPS template searches | **Not a licence question, an integrity question: these are unofficial reuploads of paid Marketplace/Fab content, not the original vendor.** | **Excluded outright.** Using a file from one of these sites means the actual licence terms are unknown and very likely violated (the original asset is paid, the reupload is not authorised redistribution). This is a harder exclusion than anything in Phase F's list: it is not a marginal licence, it is content obtained outside the rights holder's own distribution channel. Flagging explicitly so nobody mistakes a hit on one of these domains for a real option. |

### 3.3 Recommendation for animation

**Plan to hand author the fire and reload montages in the Unreal Sequencer or
AnimBlueprint tooling against whichever FP arms mesh is chosen from section
1, using the Quaternius Universal Animation Library's gun-related clips
purely as motion reference, not as a drop-in retarget.** This is the only
route in this table that is unambiguously free, unambiguously licence clean,
and does not gate Phase H's accept criteria behind opening and verifying a
paid Fab listing. `phase-h-weapon-presentation.md` already scopes the fire
montage as "a single shot loop triggered per `FireOnce()`" and the reload
montage as length matched to `ReloadSeconds`, both of which are short,
achievable hand keyed animations, not a large body of motion capture. If
CC-in-Unreal opens the Fab Ultimate FPS Animations Pack page and finds it is
free or cheap and confirmed Epic Skeleton compatible, it would be a
legitimate time saver and should be preferred at that point, but do not block
Phase H waiting for that page to be opened and verified.

---

## 4. Summary recommendation

**First choice, in order of what to acquire:**

1. **FP arms mesh:** Mixamo extraction (free, Blender required) as the first
   attempt; fall back to Fab's "First Person Base Arms Vol. 1" (USD 9.99,
   confirm on page) if Blender extraction is not practical in the editor
   session.
2. **Starting pistol mesh:** Quaternius Scifi Pistol via Poly Pizza (CC0, no
   account, no attribution). Pull a second and third silhouette from the
   Quaternius Ultimate Guns Pack bundle for the sidegrade/upgrade weapons in
   `phase-h1-starting-loadout.md`.
3. **Fire and reload animation:** hand author montages in engine against the
   chosen arms rig, using Quaternius's Universal Animation Library as motion
   reference only. Revisit the Fab Ultimate FPS Animations Pack only if
   opening its page confirms a free or low cost licence and genuine Epic
   Skeleton compatibility.

**Why this combination and not a single ready made pack:** every ready made
FP arms plus weapon plus animation bundle found in this sweep is either paid
at a scale (USD 150 class AAA packs) disproportionate to a placeholder Phase
H need, modelled on a real named firearm (the recurring legal flag this
project has already had to design around for the train livery and station
signage), or UNVERIFIED enough on licence and skeleton compatibility that
committing to it without opening the page would be premature. The CC0
Quaternius route plus a Mixamo extraction plus hand authored montages is
slower to build but has zero licence risk and zero legal flag risk, which
matches how Phase F resolved its own gaps (accept slower in-house work over a
convenient but compromised asset).

---

## 5. Inspect before staging: open legal and licence flags

1. **Fab's exact price and licence text for every Fab entry above.** Every
   Fab listing in this document is UNVERIFIED on price beyond what a search
   snippet stated; several described dollar values (USD 9.99, "$150 value")
   are marketing copy from the listing, not a confirmed current price. Open
   each page before committing spend.
2. **Fab's Standard Licence is engine-scoped.** Per Fab's own documentation
   (`https://www.fab.com/eula`), assets are licensed for use within
   Unreal-based products; this project is Unreal-based so this is not
   expected to be a problem, but it is worth reading once rather than
   assuming, since some Fab content (notably anything sourced from the old
   Epic MegaGrants or Lyra-adjacent listings) carries additional UE-only
   restrictions layered on top.
3. **Named-weapon Fab/Marketplace packs (the M4A1, Austrian pistol/G18 style
   listings) are excluded on the legal flag regardless of price or
   licence.** Restated because it is the single most important exclusion in
   this document: a cheap or even free named-replica pack is still the wrong
   choice for this project.
4. **Sketchfab licence per file.** As in Phase F, Sketchfab hosts mixed CC-BY
   and CC0 side by side and each model's page is the only authority. DJMaesen's
   "First Person arms" is noted as CC Attribution from the search snippet, so
   using it means carrying a credit line; confirm this before deciding whether
   that is acceptable for the project's stated preference for unencumbered
   assets.
5. **The Quaternius Universal Animation Library's actual clip list.** The
   search evidence confirms "combat and gun" animations exist in the pack but
   does not confirm the specific presence of a fire cycle and a reload cycle
   distinct from a held-weapon idle/walk/run set. Open the pack's asset list
   before assuming both actions are covered.
6. **Reupload aggregator sites are not a licence question to resolve, they
   are a source to avoid entirely.** Restated from section 3.2: do not treat
   a hit on gameassetsfree.com, ue3dfree.com or similar domains as a real
   candidate under any circumstances, even if a specific file looks
   convenient.

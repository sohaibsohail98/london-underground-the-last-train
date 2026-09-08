# Handover - the resume point

Last updated 2026-09-07 evening. One place for where the project stands and what
is left. **Update it whenever a task finishes**, so a cold session can pick up
without re-reading the history.

## ACTIVE SESSION LOG - MVP build + build out L_GreyboxTest (2026-09-07 night)

Session goal: work the "Exact next actions" list, build out `L_GreyboxTest` to
match Canary Wharf so both ends of the line are real arenas, and dress both maps
with free assets. Iterative. This block is updated as each step lands so a crash
mid-way loses nothing.

| Step | State |
|---|---|
| S0 Editor MCP connected, both maps inspected | DONE. `L_GreyboxTest` is a bare 27-actor box; `L_CanaryWharf_Greybox` is a full 235-actor greybox. |
| S1 Fill `StationDisplayNames` on `BP_GameMode` (handover action 3) | DONE. `L_GreyboxTest` -> "Greybox Test", `L_CanaryWharf_Greybox` -> "Canary Wharf". Compiled, saved, read back from a fresh script. |
| S2 Build out `L_GreyboxTest` geometry to arena standard | DONE. 33 greybox pieces added under folder `GreyboxTest/Station`: dark ceiling, platform lip + safety line, track bed under the train, west/east tunnel mouths (outer wall + lintel each), a row of 6 columns down the platform centre, a 6-piece ticket-barrier line at x=0 with two gaps, a south mezzanine deck + rail + cube ramp, 8 cover crates, 2 benches by the player start. Grid material `MI_PrototypeGrid_Gray` on most, `MI_PrototypeGrid_TopDark` on ceiling/tunnels/trackbed. All original gameplay actors (train, board, wall buy, round manager, 5 spawn points, player start) untouched and confirmed present. Map Check: 1 error ("Maps need lighting rebuilt" - stock dynamic-lighting message, Canary Wharf has it too), 0 warnings. Saved. Not yet play-tested in PIE. |
| S3 Fab packs | **DONE 2026-09-08.** Two packs are now in the project (both gitignored, see the `.gitignore` commit; NEVER `git add` a `.uasset` from either). Research from 2026-09-07 preserved below the results. **Results:** <br> **A. `/Game/SubwayTrain/`** (migrated from a UE 4.16 "Subway Train" pack, 378 asset registry entries). Walked the asset registry, `load_asset`-ed every entry to force the 4.16->5.8 upgrade, then `EditorAssetLibrary.save_directory("/Game/SubwayTrain")` (returned True). **All 378 loaded, 0 load failures, 0 exceptions.** Class counts: 143 Texture2D, **110 StaticMesh**, 77 MaterialInstanceConstant, **16 Material** (all compiled, 0 load errors), 9 MaterialFunction, 4 TextureLightProfile, 2 World, 2 TextureRenderTargetCube, 1 TextureCube, 1 MapBuildDataRegistry, 13 Blueprint (all loaded). **Maps: `/Game/SubwayTrain/Maps/Demonstration` and `/Game/SubwayTrain/Maps/Overview` both `load_level` = True, opened clean** (only new log warnings were unrelated Slate glyph warnings for editor-UI play/stop icons). **Minor issues (pre-existing in the source pack, NOT upgrade breakage, left as-is):** 2 meshes fall back to `WorldGridMaterial` on slot 0 - `SM_ext_floor_150_01`, `SM_chassis`; 2 material instances have a null texture param - `MI_GodRay_01` / `MI_GodRay_02` param "Cone Mask" (godray decals, cosmetic). Everything else has real materials. **Verdict: usable as-is.** <br> **B. `/Game/UrbanSubway/`** - imported the raw Urban Series FBX (`.../FabLibrary/Urban_Series___Subway_Section__Station_Platform-daab241b/fbx/low-poly-style-subway-st_extracted/source/lobby.fbx`, CC-BY) via `AssetImportTask`: combine meshes OFF, generate lightmap UVs ON, import materials + textures ON, static mesh, no skeletal, no anim, no physics asset. **61 assets created: 46 StaticMesh + 15 MaterialInstanceConstant** (`imported_object_paths` reported 61; folder is flat, no subfolders). All 61 paths are `/Game/UrbanSubway/<name>` (e.g. `Structure`, `Rails`, `Rails_structure`, `bench`, `bench_structure`, `board`, `number_destination`, `metro_traffic_light`, `Bin1`, `tube_light1`/`tube_light_2`, the `Wall_wire_*` / `Roof_wire_holder_*` / `metallic_wire_holder_*` / `Pipe_holder_*` sets, `underground_pipe1`/`2`, `wall_metal_gas_pipe`, `roof_wires`, `wall_wires`, `Net_1`/`net_2`, `Lamp_structure_1`, `light_structure_2`; MIs `concrete`, `grey_concrete`, `in_concrete`, `floor_tiles`, `metal`, `Plastic`, `wet`, `yellow`, `wires`, `light`/`light_001`, `green_light`, `bin`, `Material_001`/`Material_002`). Saved with `save_directory` (True). <br> **UrbanSubway caveats for S4:** (1) **NO textures - `Texture2D` count = 0.** The FBX carries none. All 15 MIs parent to `/InterchangeAssets/Materials/FBXLegacyPhongSurfaceMaterial` and are **flat colour only** (3-4 vector params each, 0 texture params). This is a low-poly flat-shaded style pack, so that is expected, but there is no detail/normal/roughness - dress accordingly or reskin with project MIs. (2) **Lightmap UVs were NOT generated** despite the import flag: every one of the 46 meshes has **1 UV channel** while `light_map_coordinate_index` = 1 (points at a channel that does not exist). `StaticMeshEditorSubsystem.set_generate_lightmap_uv` returned False for all 46 from script. Under Lumen/dynamic GI (what this project uses) this is harmless, but **baked static lighting on these meshes will be black/broken** and placed instances may throw Map Check "missing lightmap UV" warnings. If S4 needs baked light on them, fix in the Static Mesh editor (Build Settings -> Generate Lightmap UVs, or set Min Lightmap Resolution + reimport) - not attempted this session to stay within "import only, change nothing else". |
| S4 Dress both maps with imported / CC0 assets | **DONE 2026-09-08.** Both greybox maps dressed with `/Game/SubwayTrain/` assets into enclosed, sodium-lit underground platforms. First tried flooding the box with kit pieces (twice) - failed, rolled back both times: the greybox had no lighting design and an open top, so everything rendered flat white. What worked: an **enclose + atmosphere + retexture + props** pass, verified with screenshots each step. <br> **Shared committable assets** in `/Game/LastTrain/Materials/Dressing/`: `M_LT_BlackUnlit` (unlit near-black, for ceiling lids), `MI_LT_Wall` / `MI_LT_Floor` (children of pack `MI_Wall_01` / `MI_Floor_01`, tiling + charcoal tint), `MI_LT_Enclosure` (dark, child of `MM_Master_Material_01`), `MI_LT_EdgeTrim` (child of master, sodium tint for platform lips). <br> **`L_CanaryWharf_Greybox`** (`CanaryWharf/Dressing` folder, ~140 new actors): ceiling slab (`BasicShapes/Cube` scaled 115x72, `MI_LT_Enclosure`) closing the open top; `CW_SunLight` intensity 3.0 -> 0.05, `CW_SkyLight` 1.0 -> 0.18, `CW_HeightFog` density 0.02 -> 0.08 / falloff 0.35 / dark inscatter; an unbound `CW_D_PostProcess` volume (locked exposure 1.0/1.0, vignette 0.5, warm tint); ~34 `BP_Overheadlight_01` rigs in 3 rows (centre Y1100, track Y2300, north-wall Y450) with per-rig point ~900 + spot ~6000-8000, all Movable; ~26 `CW_D_Fill_` sodium point lights; retextured 140 `CW_Wall_*` + `CW_Floor` + 4 `CW_PlatformLip` + 28 tunnel/barrier pieces with the shared MIs (revert = set slot 0 back to `/Engine/BasicShapes/BasicShapeMaterial`); ~18 benches (`SM_chair_01a/b`, `MI_Chair_01`), ~9 wall posters, 4 speakers, 4 hanging `BP_Interior_ceiling_marquee_300`, ~6 bins (`SM_trash_can_01/02` at 11x), 20 loose trash props. **Original lighting for revert: sun intensity 3.0 / rot (-45,30,0); skylight 1.0; fog density 0.02 / height falloff 0.2.** <br> **`L_GreyboxTest`** (`GreyboxTest/Dressing` folder, ~55 new actors): already had a ceiling (`GBX_Ceiling`) so no slab - just reskinned it with `M_LT_BlackUnlit`; `GreyboxTest_SunLight` 3.0 -> 0 (small box, sky was flooding it), `GreyboxTest_SkyLight` -> 0.04 + source_type SPECIFIED_CUBEMAP (stop capturing bright sky), `GreyboxTest_SkyAtmosphere` hidden in editor, `GreyboxTest_HeightFog` density -> 0.08; `GBX_D_PostProcess` (same locked-exposure PPV); 8 `BP_Overheadlight_01` rigs (2 rows Y-300/Y+250, point ~150 / spot ~1100, Movable); 3 `GBX_D_Fill_` point lights; retextured 14 `GreyboxTest_Wall_*` + floor + 2 edge pieces with the shared MIs; 12 benches, 6 posters, 3 hanging marquees, 3 bins, 16 trash. **Original lighting for revert: sun intensity 3.0; skylight 1.0 (was SLS_CAPTURED_SCENE); fog density 0.02 / falloff 0.2; SkyAtmosphere visible.** <br> **Map Check both maps: 1 error ("WorldSettings Maps need lighting rebuilt"), 0 warnings** - the stock stationary-lighting message both maps already carried (S2 note), unchanged by this pass; no missing-lightmap-UV warnings from any imported prop. All dressing lights are Movable so nothing needs an actual bake under Lumen. <br> **Deferred / polish left for a human eye:** a few `BP_Overheadlight_01` fixtures hang ~1.5m below the ceiling and one or two poke sideways through a wall; one `L_GreyboxTest` hanging marquee spawned tilted (default pivot); some props render dark where no rig reaches; the wall MIs are cube-UV stretched on the engine-cube greybox (tiling param compensates but seams are visible). None block play or a packaged build. Every gameplay actor (train, board, wall buy, round manager, spawn points, player start, heat) was left untouched at its exact transform in both maps. |
| S5 Screamer scream verify (action 1) | **DONE - PASS.** Ran in Canary Wharf PIE. Temp values: `DA_Zombie_Screamer` FirstRoundAvailable 12->1, SpawnWeightNormalRound 6->40, MaxAliveOfThisType 1->2; `CW_RoundManager` OpeningRoundCounts ->(3,3,3,3,3). Round 1 spawned 2 screamers. Log: `BP_Zombie_C_1 screamed after 2.0s of sight. Calling 4 walkers.` and `Screamer called 4 extra walkers. Pending now 4.` So `UpdateScream` + the 2.0s LoS gate + `OnScream` + `OnZombieScreamed.Broadcast(this,4)` + `ALTRoundManager::HandleZombieScreamed` summon path all work end to end. `ScreamSummonCount` 4 respected, `MaxAliveOfThisType` 2 respected. **Cancel window (0.5s) NOT exercised** - too tight to hit by driving PIE from a script; the branch is a 3-line time guard on the proven path (`Broadcast(this,0)` -> `HandleZombieScreamed` drops `PendingScreamSpawns`), read and judged low-risk. PIE stopped cleanly. ALL temp values reverted and confirmed by fresh read-back (FRA=12, W=6, Max=1, counts=(6,8,10,12,14)). |
| S6 Frame-rate reading (action 2) | **DONE 2026-09-08 (crash-resume session).** Ran in `L_GreyboxTest` PIE, in-process bridge, editor tick NOT throttled this session (20x0.5s waits took 10.19s wall, so the reading is real). Temp `GreyboxTest_RoundManager.OpeningRoundCounts[0]` 6 -> 30; reverted after. Round 1 alone put **30 BP_Zombie alive** (cap is 24; the +6 are brutes in the round-1 mix and mid-spawn actors). **Result: whole-frame ~24.0 ms / ~40 fps, held flat across 6 consecutive 120-frame batches (24.7 to 26.0 ms).** `profile("frame_stats")`: `delta_time_ms` tracks `gpu_frame_time_ms` almost exactly (24.4 vs 24.0, 23.9 vs 23.7...) with `idle_time_ms` 0, so the frame is **GPU-bound, not game-thread-bound** - one sample where the GPU fit inside the frame showed `idle 8.41 ms`, i.e. the game+draw threads finished with 8 ms of headroom. One 54 ms spike (GC or spawn burst) in ~30 s. **Interpretation: the crowd itself is not a game-thread problem; the ~40 fps is the S9 zombie material + Lumen on this Mac's GPU in the editor.** B3's "60 fps" gate is not met in the editor at 24+ alive, but the bottleneck is render, and a packaged Development/Shipping build with a fixed resolution will read very differently. A packaged-build reading is still the honest number for the Phase B sign-off; the game thread is proven clear. Overlay note: `stat unit` draws on the editor debug canvas and is NOT captured by `playtest_observe` in either mode - use `profile("frame_stats")` for the split. |
| S7 Open acceptance: wall buy, HUD polish (action 3) | **wall buy PASS** (carried from the pre-crash session note: 500 -> 250 on the ammunition buy). Re-verified post-S11: `GreyboxTest_WallBuy_SMG` still at `X=-1200,Y=-740,Z=130`, `WeaponCost` 500 / `AmmunitionCost` 250, `bLockLocation` False - the S11 signs sit at X=-900/0 so nothing moved or overlapped it. **HUD polish REVIEWED 2026-09-08.** `WBP_HUD` = 34 widgets / 7 animations / 131 graph nodes, matches `phase-b3-feedback-widgets.md` element-for-element (Round label+number, PlayerName, Points row total+delta, Health SizeBox->Overlay->Track+Bar, Weapon name+magazine+divider+reserve, reticle dot+4 lines, 4 hit marks, prompt key+text, DamageVignette). Graph is healthy: all 5 delegates bound (`OnAmmoChanged`/`OnHealthChanged`/`OnHitConfirmed`/`OnPointsChanged`/`OnRoundStarted` + `OnInteractableChanged` assigned); crosshair spread wired (`GetCurrentSpreadDegrees` + `GetAimAlpha` -> `MapRangeClamped` x2 -> `SetRenderTranslation` x4 on Tick); 8 `PlayAnimation` (hit body 0.12s, hit headshot 0.16s, prompt in/out 0.15s, points delta 0.9s, damage flash 0.6s - all match spec durations). **Polish findings, none blocking:** (1) two leftover `PrintString` debug nodes in the EventGraph - delete them. (2) Fonts are engine stock `DroidSansMono` (numbers/labels) + `Roboto` (name/weapon/prompt), NOT the OFL project faces the S9 haul imported (Overpass / Barlow) that the S11 platform signs use. B3 explicitly wants the project typeface chosen here and recorded in `docs/art-direction.md` - this is the one real spec gap. (3) `PointsDelta`, `HitMarkTL..BR`, `DamageVignette` all sit at `Visibility=Visible` at rest and rely on empty text / `RenderOpacity=0` rather than defaulting Collapsed/Hidden - works, but a belt-and-braces pass would hide them and let the animations reveal. (4) `PlayerName` reads `PlayerState.GetPlayerName()` which in PIE is the machine ID ("Mac-BA60..."); fine until a real player state. (5) Palette compliant where it counts: HealthBar fill ~`#6C4C9C`, HealthTrack ~`#16161C`, PointsDelta ~`#E0A030`; grey/near-white body text is a spec-internal allowance. **Not driven live in PIE** (hit marker / prompt need keyboard aim per the spec) - the wiring proves they fire. |
| S8 Add both maps to packaging map list | **DONE 2026-09-08.** Written directly as `Config/DefaultGame.ini` rather than through the Project Settings UI: `[/Script/UnrealEd.ProjectPackagingSettings]` with `+MapsToCook=(FilePath="/Game/LastTrain/Maps/L_GreyboxTest")` and the same for `L_CanaryWharf_Greybox`. Committed in `f635ece`. So `L_CanaryWharf_Greybox` (only referenced by name in an `OpenLevel`) now cooks. |
| S11 Station signage + surface retint, both maps | **DONE 2026-09-08, committed `f635ece`.** Saved to disk at 12:30 before an editor crash on a `MAP CHECK` console exec at 12:31; state reconciled in the crash-resume session and confirmed complete (nothing lost), then committed as its own step. **New assets:** `M_LT_SignCharcoal` + `M_LT_SignViolet` (charcoal back panel, violet name bar) and `M_LT_Pictogram` (parent for the wayfinding instances); 14 `MI_Picto_*` under `Materials/Signage/Pictograms/` off the S9-haul ISO 7010 pictogram textures (all original wayfinding, no roundel, no operator livery). **Both maps:** Main / West / East platform signs, each a Back + Bar + Text triple, text in Overpass ("GREYBOX TEST" / "CANARY WHARF", sodium-on-charcoal); wayfinding pictogram actors on walls and by the tunnel mouths (9 in the greybox, 14 in Canary Wharf); surface retint of the S9 haul MIs - walls to `MI_Tiles133A` (greybox) / `MI_Tiles107` (Canary Wharf), floor to `MI_Concrete048` / `MI_Concrete046`, the 6 greybox platform columns (`GBX_Column_1..6`) and both Canary Wharf tunnel mouths to `MI_Concrete031`. 10 `MI_<Set>` under `Surfaces/` touched, all load clean with valid parents. **14 Canary Wharf actors deliberately left on the engine `BasicShapeMaterial`:** the 2 train carriages (the train's own visual), 4 escalator pieces and 8 gameplay Anchor markers (placeholders) - not a gap. Every gameplay actor left at its transform. Both maps pass the headless dependency Map Check (`MAP CHECKDEP`) with 0 errors, 0 warnings. |
| S9 Zombie appearance | **MINIMAL PASS DONE 2026-09-08. City Sample Crowds (varied clothed bodies) is still the real fix.** No `Source/` change. Root cause found in PIE: `M_Zombie_Tintable` (the parent of all five `MI_Zombie_*`) was a flat untextured lit material with only a `TintColour` vector AND `used_with_skeletal_mesh = false`, so every spawned zombie fell back to the engine Default Material and rendered as the clean bright Quinn mannequin. <br> **What changed:** <br> - **`M_Zombie_Tintable` rebuilt** (same asset path, so all five `MI_Zombie_*` inherit it unchanged, and the C++/BP tint-swap wiring is untouched): now `used_with_skeletal_mesh = true`; samples the Quinn D/N/MRA textures for real flesh detail; desaturates the albedo 85% and multiplies by `DeadFleshColour` (default `0.19,0.21,0.17` - muted cold grey-green, no bright green); applies the existing per-type `TintColour` as a light accent via `TintStrength` (0.35) so sprinter still reads faintly sodium / brute faintly crimson etc. without a paint job; `RoughnessBoost` (+0.15) on the MRA green channel. <br> - **Blood layer baked into the same material**: `BloodMask` samples `T_SurfaceImperfections011_Opacity` (ambientCG CC0, from the S-of-S4 haul), tiled `BloodTiling` (2.0), contrast `BloodContrast` (3.0) x `BloodAmount` (0.5), lerps the albedo toward `BloodColour` (`#B02030`-ish dark crimson) and drops roughness to 0.22 where blood is heavy (wet look). Every zombie gets the same blotch pattern - fine at gameplay distance in the dark station, costs nothing per spawn (no decal components, no dynamic MID). <br> - **`BP_Zombie`**: `CharacterMesh0` default `override_materials` set to `MI_Zombie_Walker` on both slots (so it looks right even before the BP's runtime type-swap), and `global_anim_rate_scale` 1.0 -> **0.8** as the shamble knob. NOTE: `ULTZombieTypeData::AnimPlayRate` is plumbed in C++ (`GetAnimPlayRate`) but the shared stock `ABP_Unarmed` AnimBP does not consume it, so per-type anim speed still is not differentiated - a real shamble/lunge anim set (or an AnimBP that reads `GetAnimPlayRate`) is still needed. DA `anim_play_rate` values left at their canon numbers. <br> - **`M_LT_ZombieWound`** (new, `/Game/LastTrain/Materials/`): a valid deferred-decal crimson wound material (translucent blend, MASKS sampler on `T_SurfaceImperfections013_Opacity`). Not wired to anything yet - spare, for hand-placed wound decals later. <br> **Also fixed in this commit: `M_LT_PBRSurface`** (the S4-haul surface master committed in `5e3e609`) failed to compile on `SF_METAL_SM6` - its `TextureSampleParameter2D` nodes had Normal/Grayscale sampler types with no assigned default texture, so it fell to the engine Default Material. Rebuilt: every sampler param now has a matching engine default texture (`WhiteSquareTexture` for colour slots, `DefaultNormal` for the normal slot) assigned before the sampler type is set; all colour-role samplers are `SAMPLERTYPE_COLOR`. The 17 `MI_<Set>` under `/Game/LastTrain/Surfaces/` inherit the fix. **Lesson for future material scripting on Mac: assign a compatible default texture to a `TextureSampleParameter2D` BEFORE setting `sampler_type`, and match `sampler_type` to the assigned texture's actual compression (`TC_MASKS` -> `SAMPLERTYPE_MASKS`, etc.) or the Metal compile silently fails and the Default Material renders.** <br> **Verified in PIE (read-only, Canary Wharf):** round 1 spawns, zombies render `MI_Zombie_Walker` -> base `M_Zombie_Tintable` (not the engine default), close-up shows desaturated dead-flesh skin with crimson blood streaks on chest/face/limbs. All four materials (`M_Zombie_Tintable`, `M_LT_ZombieWound`, `M_LT_PBRSurface`) pass a fresh forced recompile with 0 failures. PIE stopped cleanly; all in-PIE test mutations were on the throwaway PIE world only. |

Notes / anything done differently from spec will be appended here as it happens.

**2026-09-08 (post editor-crash resume session):** committed the 5 pre-crash
files + the `.gitignore` change as one commit (`feat(editor): MVP pass...`),
each `.uasset`/`.umap` verified as an LFS pointer in the index first. Then did
S3 (see the S3 row): re-saved and audited `/Game/SubwayTrain/` (378 assets, 0
failures) and imported the Urban Series FBX into `/Game/UrbanSubway/` (61
assets). `.gitignore` already carried `/Content/SubwayTrain/` and
`/Content/UrbanSubway/` from the pre-crash change - both dirs confirmed
git-ignored, nothing from them staged.

**2026-09-08 (continued, S4):** dressed both greybox maps into enclosed
sodium-lit underground platforms (see S4 row for the full manifest). Two naive
"flood the box with kit pieces" attempts on Canary Wharf were rolled back before
landing on the enclose/atmosphere/retexture/props approach. 5 shared MIs added
under `/Game/LastTrain/Materials/Dressing/`. Both maps Map Check at 1 stock
error / 0 warnings (unchanged from pre-dressing). Every gameplay actor left
untouched. NOTE: `.gitignore` still carries an uncommitted `/_incoming_assets/`
line that was already in the working tree at session start and is NOT from this
work - left unstaged for the user to triage. **(That line was committed
separately as `3b9427b` before this session's asset-import work.)**

**2026-09-08 (CC0 asset haul import + S9):** imported the safe CC0/OFL/public
-domain subset of `_incoming_assets/` under `Content/LastTrain/` and committed
it (`5e3e609`): 21 ambientCG PBR surface sets (121 Texture2D) + new master
`M_LT_PBRSurface` + 17 `MI_<Set>`; 12 FontFace + 5 `Font_UI_*` (Overpass /
Overpass Mono / Public Sans / Barlow / Barlow Condensed, OFL, each with its
OFL.txt); 26 ISO 7010 pictograms (SVG import is not scriptable in 5.8, so
rasterised to 512px PNG and imported as UI textures); 2 Poly Haven HDRIs
(metro_vijzelgracht, dresden_station_night) as TextureCube. Skipped per brief:
weapons/ (real firearm names, needs rename-on-import), characters/, props/,
audio/. Then S9 (see S9 row): rebuilt `M_Zombie_Tintable` into a dead-flesh +
per-type-tint + baked-blood material with the SkeletalMesh usage flag set, so
zombies stop rendering as the clean bright mannequin. **Found and fixed a Metal
compile failure in the just-committed `M_LT_PBRSurface`** (sampler-type vs
default-texture mismatch) in the same S9 commit.

**2026-09-09 (S11 crash-resume + S6/S7/S8/S11 review session):** editor
SIGSEGV'd at 12:31 on a `MAP CHECK` console exec during S11; relaunched.
`L_GreyboxTest` and `L_CanaryWharf_Greybox` were saved to disk at 12:30 before
the crash but nothing was committed. **Reconciled: nothing was lost.** Both maps
opened and audited via the MCP bridge - the 9 signage actors per map (Main /
West / East x Back / Bar / Text), the pictogram actors (9 greybox, 14 Canary
Wharf), the wall / floor / column / tunnel surface swaps and the 3 new signage
materials + 14 `MI_Picto_*` were all present and coherent, and both maps pass
`MAP CHECKDEP` (the non-dialog dependency check - **do not run `MAP CHECK` or
`MAP CHECK DONTDISPLAYDIALOG` again, that is what crashed**) at 0/0. Committed
S11 as its own step, `f635ece` (30 files, every `.uasset`/`.umap` verified as an
LFS pointer in the index first), together with `Config/DefaultGame.ini` (S8).
Then: **S8** done (see row - written as the ini, not the UI). **S6** done (see
row - frame gate ran, ~40 fps / ~24 ms GPU-bound at 30 zombies alive, game
thread proven clear; packaged-build reading still wanted for the Phase B
sign-off). **S7** wall buy re-verified post-S11 (unmoved, 500/250); **HUD
polish reviewed** (see row - `WBP_HUD` matches B3, findings: 2 debug
`PrintString` nodes to delete, fonts still engine-stock not the OFL project
faces, a few widgets `Visible` at rest instead of hidden). One editor-only
note: the S6 test did a temp `OpeningRoundCounts[0]` 6->30 write on
`GreyboxTest_RoundManager`, reverted to `(6,8,10,12,14)` and confirmed by
read-back before `save_all_levels()`; the save re-serialised
`L_GreyboxTest.umap` (same size, new hash - pure churn, gameplay values
identical) so it was `git checkout`'d back to the `f635ece` state. Working tree
clean. If the editor prompts to save `L_GreyboxTest` on close, the answer is
discard.

### S10 Damage feedback (player-hit indicator) - added on user request, DONE (Blueprint only, no C++)

User asked for a "getting hit" screen effect like Call of Duty. Added to `WBP_HUD`, no `Source/` change:

- New widget `DamageVignette`: full-screen `Image`, crimson `ColorAndOpacity` `(R=0.72,G=0.06,B=0.10,A=0.62)`, `HitTestInvisible`, ZOrder 1 (above the world layer, below the HUD text), `RenderOpacity` 0 at rest. Confirmed it renders in PIE (temporarily set to 0.85 -> full crimson wash, HUD readable on top, then reverted).
- New animation `DamageFlash`: keys on `DamageVignette.RenderOpacity` 0.0->1.0 (t=0.04) ->0.55 (t=0.16) ->0.0 (t=0.5). A ~half-second crimson screen pulse.
- New animation `LowHealthPulse` (authored, NOT yet wired - needs a tick/timer to breathe; left for later).
- New float var `PrevHealthFraction` (default 1.0).
- `WBP_HUD` EventGraph: extended `HandleHealthChanged` (which is bound to the player's `OnHealthChanged`). Was: `then -> HealthBar.SetPercent`. Now: `then -> Branch`; condition = `HealthFraction < PrevHealthFraction` (a `float <` node fed by the event's `Health Fraction` out and a `Get PrevHealthFraction`); `Branch.True -> PlayAnimation(DamageFlash, self, 1 loop) -> Set PrevHealthFraction`; `Branch.False -> Set PrevHealthFraction`; `Set PrevHealthFraction.then -> HealthBar.SetPercent` (bar still updates exactly as before). So any hit that lowers health plays the pulse; regen ticks (health rising) do not.
- Compiles clean (0 errors, 0 warnings). Verified graph topology by read-back.
- **Not captured on camera mid-hit**: the 0.5s pulse during a live zombie hit. Scripted PIE can't time a screenshot burst to the exact contact frame, and on `L_GreyboxTest` the player is swarmed and downed within ~6s (6 walkers, 10 dmg, 1.5s cooldown vs 100 HP + 50% auto-revive) so there is no clean single-hit window. The three things that prove it work were each verified independently: widget renders in PIE, `OnHealthChanged` fires on every damage/revive transition (health fraction 1->0->0.5->0 seen in logs), graph wired correctly. A human at the keyboard on Canary Wharf (more room) will see the pulse on the first zombie tag.
- Follow-ups worth doing: (a) wire `LowHealthPulse` to a repeating timer when health < ~0.3 for a sustained "critical" state; (b) if a *directional* indicator is wanted (wedge pointing at the attacker), that needs a small C++ change to pass `DamageCauser`/hit direction through the `OnDamageTaken` delegate - write it as a spec, do not edit C++ in an editor pass.

## The one line

Phase A is done. **Phase B: the screamer scream (S5) and the HUD polish review
(S7) are now done; the last open item is a packaged-build frame reading.** The
editor frame gate (S6) ran at ~40 fps / ~24 ms with 30 zombies alive and is
GPU-bound, not game-thread-bound, so the crowd logic is clear - but B3's 60 fps
number needs a packaged Development build to be the honest sign-off figure.
**Phase C is code complete, compiled, and verified in PIE for everything except
the screamer scream trigger... which S5 has now PASSED.** The C++ that was
written in remote sessions has been compiled locally and reviewed twice.
NeoStack's editor pass on 2026-09-07 finished all five milestones: reparented
`BP_GameMode`, built the five zombie type assets, the tintable material,
`BP_Train` and `BP_DepartureBoard`, wired the roster and `StationRoutes`, and
verified station travel both directions with exact carry-over. What is left is
small: a frame-rate reading in a packaged build, and the wall-buy walk-up in PIE
at the keyboard (the buy itself already passed via property read-back).

## State of play, `origin/main` at `2e0f59a`

| Layer | State |
|---|---|
| Phase C C++ | **Compiled clean under `-Werror`, all 5 CI gates green.** Train, departure board, station travel, five zombie types, special rounds, downed and revive. Reviewed in `phase-c-review-2026-09-07.md`. |
| Phase C editor assets | **Committed** (`2e0f59a`). `BP_GameMode` reparented to `ALTGameMode`, so the run state machine is now live in every level. Five `DA_Zombie_*` type assets, `M_Zombie_Tintable` plus five `MI_Zombie_*` instances, `BP_Train` boarding, `BP_DepartureBoard`, roster wired on both round managers, spawn points placed on both greybox maps. |
| Verified in PIE | Boarding end to end, downed then auto-revive, station heat 0 to 3 one step per departure, train full cycle with all ten hooks on schedule, types spawn with correct stats and silhouette, sprinter round at 5, brute pair at 10, armour plate absorbs 200 body damage. |
| Not verified | The screamer scream **cancel window** only (S5 PASSED the scream itself, summon and count - the 0.5s cancel branch is a read-and-judged 3-line time guard). The Phase B crowd frame gate at 60 fps **in a packaged build** (S6 ran it in the editor: ~40 fps / ~24 ms, GPU-bound, game thread clear - the editor number is render-limited, not logic-limited). |
| Station travel | **PASS both directions** 2026-09-07. `StationRoutes` wired on `BP_GameMode`, `CW_Train` and a heat component added to Canary Wharf. Boarded out of `L_GreyboxTest` with 1280 points / 240 reserve, arrived in Canary Wharf with the exact carry, rounds from 1, heat 0. Boarded back with 2060 / 240, same. |

## You can play the full loop right now

Open `L_GreyboxTest` or `L_CanaryWharf_Greybox` and press Play. Working: rounds,
five zombie types, special rounds at 5 and 10, the wall buy, points, the train
cycling with a live board prompt, boarding out to the other station carrying
points and weapon (rounds restart at 1, heat resets), downed plus auto-revive at
half health, station heat 0 to 3. The only thing not exercised in a run is the
screamer's scream (it does not spawn until round 12).

## Exact next actions, in order

Everything here is editor work or a packaged build. No `Source/` change is
expected; if one seems needed, stop and write it up as a spec rather than
editing C++ in this pass.

### 1. Verify the screamer scream (~20 min)

The screamer's data applies correctly (`Type SCREAMER`, `Behaviour SCREAM`, walk
speed 110.5) but the scream was never triggered because reaching round 12 by hand
was out of budget, and runtime spawning is not exposed to the editor Python API.

Force a natural spawn: on `DA_Zombie_Screamer`, temporarily set
`FirstRoundAvailable` from 12 to **1**, and drop `OpeningRoundCounts` on the
placed round manager to something small like `(3,3,3,3,3)`. Play, let a screamer
spawn, stand in its line of sight with a clear floor between you for more than
`ScreamLineOfSightSeconds` (2.0s). Expect:

- A `screamed after` log line, `OnScream` fires, `OnZombieScreamed` broadcasts.
- Four extra walkers spawn (`ScreamSummonCount` 4).
- If you kill the screamer within `ScreamCancelWindowSeconds` (0.5s) of the
  scream, the summon is cancelled (`OnZombieScreamed` broadcasts with count 0).

**Revert `FirstRoundAvailable` to 12 and `OpeningRoundCounts` to
`(6,8,10,12,14)` afterwards** and confirm by read-back. If the scream does not
fire with a natural spawn either, that is a `Source/` bug in
`ALTZombieCharacter::UpdateScream` / `TickScream` - write it up as a spec, do not
fix it in this pass. Per `CLAUDE.md` a bug that survives is Fable's.

### 2. Frame-rate reading for the Phase B gate

The gate is 60 fps with 24 to 40 zombies alive. It has never been measured
because every delta sample over the editor bridge returns exactly 0.33333s: the
editor throttles its tick to 3 fps while the window is unfocused, and every
bridge command runs unfocused.

Two ways to get a real number:

- **Focused editor**: play in a New Editor Window, click into it so it has
  focus, force a dense round (`OpeningRoundCounts` high, `BreatherSeconds` low),
  type `stat unit` in the console, read the ms on screen by eye. Revert the test
  values after.
- **Packaged build** (the honest number): `RunUAT BuildCookRun` for a Mac
  Development or Shipping build, both maps in the cook set. `Config/DefaultEngine.ini`
  pins `GameInstanceClass`, and `OpenLevel` by name means `L_CanaryWharf_Greybox`
  is referenced by no asset - **add both maps to the packaging map list** or the
  travel target will not cook. This is also the first real end-to-end test of a
  shipped artifact, which is the Sunday goal.

Record the number in `README.md` and here, then mark Phase B done if B2 and B3
also pass (below).

### 3. Run the acceptance lists that are still open

| What | Where | Status |
|---|---|---|
| Crowd frame gate, 24 to 40 at 60fps | `phase-b1-throttled-repath.md` | editor reading DONE (S6): ~40 fps / ~24 ms GPU-bound at 30 alive, game thread clear. Packaged-build number still wanted. |
| Wall buy prompt and purchase | `phase-b2-interaction.md` steps 3 to 7 | buy PASS via property read-back (500->250); walk-up/look/prompt-fade still needs a keyboard in PIE |
| HUD polish: hit marker, spread, prompt anchor | `phase-b3-feedback-widgets.md` | REVIEWED 2026-09-09 (S7). Wiring matches B3. Findings: 2 debug PrintString nodes, engine-stock fonts not the OFL project faces, a few widgets Visible at rest. None blocking. |
| Train, 12 points | `phase-c1-train.md` | PASS 2026-09-07 |
| Live countdown on a sign | `phase-c2-departure-board.md` | PASS 2026-09-07 |
| Board and arrive with the carry | `phase-c3-travel.md` | PASS both directions 2026-09-07 |
| Zombie types, 11 points | `phase-c-zombie-types.md` | PASS except screamer, action 1 |
| Sprinters on 5 and 15, brutes on 10 and 20 | `phase-c-special-rounds.md` | PASS at 5 and 10 |
| Down, bleed out, auto revive | `phase-e1-downed-revive.md` | PASS 2026-09-07 |
| Corridor stall no longer freezes a zombie | open code item 1 | fixed in code, unverified in PIE |

Also fill **`StationDisplayNames`** on `BP_GameMode` (two entries, key by map
asset name: `L_GreyboxTest` -> `Greybox Test`, `L_CanaryWharf_Greybox` ->
`Canary Wharf`) so the HUD and board have a station label. The C++ stamps it on
BeginPlay; blank until this is filled. Not blocking anything, but visible.

### 4. Full playthrough

Once 1 to 3 pass: play a complete session on each map, boarding between them at
least once, to round 12+ so a screamer and the special rounds all appear. Note
the top three feel-breakers (pacing, readability, a zombie doing something
visibly wrong) and write them up. That list is the next work session's input.

## What has landed

### Phase A, done 2026-09-04

`L_GreyboxTest` plays end to end. A5 acceptance passed on every testable check.
Records: `phase-a4-editor-setup.md`, `phase-a5-acceptance.md`.

### Phase B, code complete, one packaged-build measurement short

B1 throttled repath, B2 interaction and the first wall buy, B3 the HUD widget.
The zombie attack fix and the HUD health bar are verified in PIE. The HUD polish
review is done (S7, 2026-09-09) - wiring matches B3, five minor findings, none
blocking. The crowd frame gate ran in the editor (S6): ~40 fps / ~24 ms,
GPU-bound, game thread clear. **Open**: a packaged Development-build frame
reading for the honest 60 fps sign-off, and the wall-buy walk-up in PIE at the
keyboard (the buy passed via property read-back). The corridor stall edge case
has a verdict and a fix as of 2026-09-07, unverified in PIE: open code item 1.

### Phase C, code complete and compiled, editor pass done

- **C1 `ALTTrain`** (`phase-c1-train.md`): arrive, dwell, depart, away on the
  100s interval and 25s dwell, ten presentation hooks, the boarding interact,
  the heat increment on a not-boarded departure. `BP_Train` built and its
  12-point PIE list PASSED 2026-09-07.
- **C2 `ALTDepartureBoard`** (`phase-c2-departure-board.md`): finds the level's
  train or takes an instance override, polls the countdown getters, fires
  `OnCountdownChanged` only on a whole-second move, forwards `OnPhaseChanged`.
  `BP_DepartureBoard` built and the countdown tracked the train exactly in PIE.
- **C3 travel** (`phase-c3-travel.md`): `ULTGameInstance` carries points, weapon,
  reserve and the visited-station list across an `OpenLevel`;
  `NotifyPlayerBoarded` snapshots the carry and travels after
  `TravelDelaySeconds`; `RehydrateFromTravel` grants it on the far side. Uses
  `ULTPointsComponent::SetPoints` so the carry is not a HUD spend. Rounds restart
  at 1 on travel; reserve carries to full. Editor wiring done 2026-09-07 (StationRoutes).
- **Five zombie types** (`phase-c-zombie-types.md`): `ULTZombieTypeData` plus
  `ApplyTypeData`, the brute's front armour plate, the sprinter's lunge, the
  screamer's sight line and summon, the weighted roster with
  `FirstRoundAvailable`, per-type live caps and the heat-3 weight shift. Five
  `DA_Zombie_*` assets built from the `neostack.md` table, five `MI_Zombie_*`
  material instances, `BP_Zombie` swaps its instance on a 0.05s delay after
  BeginPlay off `GetZombieType()` (the delay is needed because `ApplyTypeData`
  runs after BeginPlay). PIE confirmed types, stats, silhouette, armour plate.
  **Screamer scream unverified: action 1.**
- **Special rounds** (`phase-c-special-rounds.md`): `FLTRoundPlan` decided once
  per round, a three-way branch in `TrySpawnOne`, `GetSpecialRoundTag` for a
  banner. Reviewed in `phase-c-review-2026-09-07.md`. Brute pair lands at ~30 and
  ~70 per cent through the round (canon section 6). Round 10 is a walker round
  plus the pair, not an all-sprinter horde, controlled by
  `bBruteRoundOverridesSprinterRound` (default set); clear it on
  `BP_RoundManager` to stack the two. Canon section 6 contradicts itself here and
  wants settling. PIE confirmed sprinters at 5, two brutes at 10.

### Phase E, started

**E1 downed and revive** (`phase-e1-downed-revive.md`): zero health calls
`Down()`, never the death path. Movement disabled, fire/aim/reload/interact/sprint
no-op while down, `Look` stays free, and the interaction sweep is disabled so no
stale prompt hangs on screen. `BleedOutSeconds` (30) runs in `Tick`; expiry fires
`OnBleedOutExpired` then `Die()`, now the only route to `NotifyPlayerDied`.
`bSoloAutoRevive` stands the player up at half health after 8s. Damage while down
is ignored. `Down()` is a no-op once the run state is `Boarded` or `Dead`. PIE
confirmed 2026-09-07. Seams open: `Revive()` for an item or co-op, `OnDowned`/
`OnRevived` for a last-stand weapon, the downed screen for the HUD and art pass.

Perks, the upgrade bench and lost property are untouched.

## Open code items

All eight from the 2026-09-07 review are closed in `main` and **compiled**.
Kept here for the record; only item 1 still needs a PIE check and item 5 is a
design call.

1. **Corridor stall edge case: fixed, unverified in PIE.** It was a gate-order
   bug: the repath in `Tick` re-issued the AI move 0.35s after
   `BeginStallRecovery` cancelled it, handing velocity back to path following.
   Plus `StallTimer` was wiped by any single jittery frame, and recovery exited
   on one jittery frame without the zombie moving. Now the repath is held while
   recovering, the timer decays, exit needs 40 units of ground closed, and
   `StallRecoverySeconds` (1.5) caps a shove. **Check in PIE**: a corridor queue
   should fan out, not freeze; no zombie sits still outside `AttackRange` for
   more than ~2s; "entering stall recovery" logs at most every couple of seconds
   per zombie, not every frame. If it survives, it goes to Fable.
2. `NavProjectionExtent` Z tightened 500 to 150, `NavProjectionWarnDistance`
   (200) logs which spawn point snapped and how far. Fixed.
3. Arrival no longer flashes the points HUD as a spend: `SetPoints` broadcasts a
   zero delta. Fixed.
4. Interaction prompts while downed: `SetInteractionEnabled(false)` on `Down`
   and `Die`, `true` on `Revive`. Fixed.
5. **Solo death is unreachable on the shipped defaults** (auto-revive at 8s vs a
   30s bleed-out, damage ignored while down, no down cap). Exactly what E1
   specified, but Phase E's own gate ("a full survival session start to death is
   possible") needs a revive item, a per-run cap, or `bSoloAutoRevive` cleared.
   Clearing it on `BP_PlayerCharacter` is the one-click version. **Design call.**
6. `ALTGameState::SetStationName` now has a caller: `ALTGameMode` stamps
   `StationDisplayNames` / `StationDisplayName` on BeginPlay. Fixed. **Editor
   follow-up: fill StationDisplayNames, action 3.**
7. `ALTTrain`'s class comment updated. Fixed.
8. `ALTPlayerCharacter::TakeDamage` subtracts what `Super::TakeDamage` returns,
   and a fully absorbed hit no longer restarts the regen delay. Fixed.

## Build, CI and toolchain

- **Compile:** `./tools/ci/compile.sh` (checks the external Xcode mount, then
  runs the engine `Build.sh` for `LastTrainEditor Mac Development`). Override the
  engine path with `LASTTRAIN_ENGINE_ROOT`. No hosted compile: Unreal is not on
  CI. An opt-in `compile` job runs the same script on a self-hosted macOS runner
  (labels `self-hosted, macOS, unreal`, repo variable `UNREAL_SELF_HOSTED` set
  to `true`); skipped until a runner is registered.
- **Five CI gates**, all run locally, on every push to `main`, `claude/**` and
  the phase branches, and on every PR: `check_hygiene.py`,
  `check_cpp_conventions.py` plus clang-format 20, `check_cpp_reflection.py`
  (the compiler stand-in: `GENERATED_BODY()`, `generated.h` include, TU includes
  its own header first, `LT_LOG`/`UE_LOG` format specifier vs argument
  agreement), `check_docs.py`, `check_content.py` (LFS pointer integrity).
- **External Xcode** on `/Volumes/DriveSohaib` must be mounted to compile.
  `xcode-select -p` must point inside it.
- **clang-format 20** governs `.h`/`.cpp`/`.cs`. If not on PATH:
  `python3 -m venv /tmp/cf-venv && /tmp/cf-venv/bin/pip install clang-format==20.1.0`.
- **Pending: move the engine to the external drive**, `drive-migration.md`. Do it
  with the editor fully closed. Frees ~43 GiB.
- **`LastTrain.uproject`** lists only `EnhancedInput`, `ModelingToolsEditorMode`
  and `CommonUI`. The editor rewrites the file on launch: it reformats it and
  re-adds plugin entries for whatever is installed locally (`NeoStackAI`,
  `ModelContextProtocol`, `AllToolsets`, `Terminal`, `EditorToolset`). **Check
  `git diff LastTrain.uproject` before committing after an editor session** and
  revert it if the only change is plugin re-additions. `CommonUI` must stay:
  NeoStack once enabled it as a dependency without its modules built, which
  SIGSEGV'd PIE.
- **Local tooling that is gitignored, not project code**: `Plugins/NeoStackAI/`,
  `.neostack/`, `.agents/`, `.mcp.json`, `.codex/`, `.cursor/`, `.gemini/`. The
  last four are MCP client configs the in-editor Claude Code terminal and the
  toolset plugins drop; they point at a local bridge.
- **`.uasset` and `.umap` are Git LFS.** Working-tree copies are raw binary
  locally (smudged); the committed blob must be a pointer. `check_content.py`
  enforces this. LFS objects upload on push.
- **Never mutate actors or assets while PIE is running.** Stop PIE with
  `playtest.stop` or Escape before any edit.
- **British spelling everywhere**, no em or en dashes in source or docs. The
  palette is `#16161C` charcoal, `#6C4C9C` violet, `#E0A030` sodium, `#B02030`
  crimson. No roundel, no Johnston, no official line diagram, no operator livery,
  no Call of Duty names.

## The docs a fresh session needs

- **This file** - the resume point.
- `neostack.md` - every editor task in detail, including the full zombie stat
  table and the `BP_GameMode` travel section.
- `phase-c-review-2026-09-07.md` - what the review pass changed and why.
- `neostack-run-2026-09-07.md` - NeoStack's own run logs for the day, milestone
  by milestone, with the PIE evidence.
- `docs/design/gameplay-canon.md` - the settled design, coded values marked
  authoritative.
- `CLAUDE.md` - conventions, module layout, legal constraints, the model split.

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
| S6 Frame-rate reading (action 2) | **DONE 2026-09-09. Both an editor reading and a packaged Development build reading. B3's 60 fps gate is NOT met on this Mac (Apple M4) at 1280x720 with Lumen on; the frame is GPU-bound in every configuration and the game thread is comfortably clear.** <br> **Editor PIE** (`L_GreyboxTest`, in-process bridge, tick NOT throttled - 20x0.5s waits took 10.19s wall): temp `OpeningRoundCounts[0]` 6->30 (reverted). 30 `BP_Zombie` alive. Whole-frame **~24 ms / ~40 fps**, flat over 6x120-frame batches. `profile("frame_stats")`: `delta_time_ms` == `gpu_frame_time_ms`, `idle_time_ms` 0 (one sample showed 8.41 ms idle when the GPU fit the frame, i.e. game+draw had 8 ms headroom). One 54 ms spike in ~30s. <br> **Packaged Mac Development build** (`RunUAT BuildCookRun`, cook clean 840 pkgs 0/0, staged to `Saved/StagedBuilds/Mac/LastTrain.app`, archived to `/Volumes/DriveSohaib/LastTrainBuild/Archive`; run `-game -windowed -ResX=1280 -ResY=720` on `L_CanaryWharf_Greybox`, CSV profiler, `t.MaxFPS 0`). Two runs: <br> - **Run A, natural spawns:** the unattended player idle-dies and auto-revives on round 1, so `Ticks/LTZombieCharacter` never exceeded **6**. 13,339 frames. Steady state **~31 ms / 32 fps**, GameThread **2.5 ms**, GPU **29 ms**. So even a near-empty Canary Wharf is ~32 fps - the base cost is the scene (Lumen + S9 zombie material + the S4 dressing), not the crowd. <br> - **Run B, forced horde:** temp-bumped the placed `CW_RoundManager.OpeningRoundCounts` to `(40,40,40,40,40)`, iterative re-cook (9 pkgs), reverted after. 5,583 frames. **24-36 zombies alive across 4,383 of them: ~37.8 ms / ~26.5 fps, GameThread ~5.2 ms, GPU ~36 ms.** Full-run mean (avg ~26 alive) ~36 ms / 27.5 fps. Peak 36 alive: 38.3 ms / 26 fps. <br> **Reading:** the horde adds ~7 ms GPU and ~2.7 ms game thread going 6 -> 36 zombies; the scene floor is ~29 ms GPU before any real crowd. Frame time tracks GPU time in every window (`GameThreadTime` 2.5-5.4 ms throughout), so the crowd's *logic* (B1 throttled repath, no behaviour tree) is not the bottleneck and B1's design goal holds - but the project does not hit 60 fps in a packaged build on this hardware at this resolution with the current lighting/material setup. That is a Phase F art/perf problem (Lumen settings, zombie material cost, a scalability pass), not a Phase B logic problem. **Phase B's frame gate as literally worded (60 fps, 24-40 alive, stat unit) FAILS; the underlying question it was protecting against - does the crowd tank the sim - is answered NO.** Note: `Ticks/LTZombieCharacter` in the CSV is the live zombie count per frame; `stat unit` overlay is not captured by `playtest_observe`. |
| S7 Open acceptance: wall buy, HUD polish (action 3) | **wall buy PASS** (carried from the pre-crash session note: 500 -> 250 on the ammunition buy). Re-verified post-S11: `GreyboxTest_WallBuy_SMG` still at `X=-1200,Y=-740,Z=130`, `WeaponCost` 500 / `AmmunitionCost` 250, `bLockLocation` False - the S11 signs sit at X=-900/0 so nothing moved or overlapped it. **HUD polish REVIEWED 2026-09-08.** `WBP_HUD` = 34 widgets / 7 animations / 131 graph nodes, matches `phase-b3-feedback-widgets.md` element-for-element (Round label+number, PlayerName, Points row total+delta, Health SizeBox->Overlay->Track+Bar, Weapon name+magazine+divider+reserve, reticle dot+4 lines, 4 hit marks, prompt key+text, DamageVignette). Graph is healthy: all 5 delegates bound (`OnAmmoChanged`/`OnHealthChanged`/`OnHitConfirmed`/`OnPointsChanged`/`OnRoundStarted` + `OnInteractableChanged` assigned); crosshair spread wired (`GetCurrentSpreadDegrees` + `GetAimAlpha` -> `MapRangeClamped` x2 -> `SetRenderTranslation` x4 on Tick); 8 `PlayAnimation` (hit body 0.12s, hit headshot 0.16s, prompt in/out 0.15s, points delta 0.9s, damage flash 0.6s - all match spec durations). **Polish findings, none blocking:** (1) two leftover `PrintString` debug nodes in the EventGraph - delete them. (2) Fonts are engine stock `DroidSansMono` (numbers/labels) + `Roboto` (name/weapon/prompt), NOT the OFL project faces the S9 haul imported (Overpass / Barlow) that the S11 platform signs use. B3 explicitly wants the project typeface chosen here and recorded in `docs/art-direction.md` - this is the one real spec gap. (3) `PointsDelta`, `HitMarkTL..BR`, `DamageVignette` all sit at `Visibility=Visible` at rest and rely on empty text / `RenderOpacity=0` rather than defaulting Collapsed/Hidden - works, but a belt-and-braces pass would hide them and let the animations reveal. (4) `PlayerName` reads `PlayerState.GetPlayerName()` which in PIE is the machine ID ("Mac-BA60..."); fine until a real player state. (5) Palette compliant where it counts: HealthBar fill ~`#6C4C9C`, HealthTrack ~`#16161C`, PointsDelta ~`#E0A030`; grey/near-white body text is a spec-internal allowance. **Not driven live in PIE** (hit marker / prompt need keyboard aim per the spec) - the wiring proves they fire. |
| S8 Add both maps to packaging map list | **DONE 2026-09-08.** Written directly as `Config/DefaultGame.ini` rather than through the Project Settings UI: `[/Script/UnrealEd.ProjectPackagingSettings]` with `+MapsToCook=(FilePath="/Game/LastTrain/Maps/L_GreyboxTest")` and the same for `L_CanaryWharf_Greybox`. Committed in `f635ece`. So `L_CanaryWharf_Greybox` (only referenced by name in an `OpenLevel`) now cooks. |
| S11 Station signage + surface retint, both maps | **SUPERSEDED by F5 (2026-09-09): the signage was rebuilt from scratch and is now readable in-game on both maps. See the F5 row.** The S11 assets themselves were always fine (`M_LT_SignCharcoal`, `M_LT_SignViolet`, `M_LT_Pictogram`, 14 `MI_Picto_*`, and the wall/floor/column/tunnel surface retint); only the actor placement was broken. Historical record of what S11 shipped and what S12 found is under the "## S12 Opus fresh-eyes review" heading and in git history at `f635ece`. |
| F5 Signage and wayfinding (fix + finish S11) | **DONE 2026-09-09. Editor + Blueprint only, no `Source/` change.** Rebuilt all six station-name sign triples on both maps rather than patching S11. **Root cause S12 missed:** the S11 signs were not merely mis-rotated, they were on the wrong wall entirely. In `L_CanaryWharf_Greybox` they sat at `Y~80`, against `CW_Wall_140` (the `Y=0..150` back-of-house wall), which is behind the solid `CW_Wall_130` row at `Y=600..750`. The wall the player actually faces from `CW_PlayerStart` is that `Y=600..750` row, room surface `Y=750`; the playable platform corridor is `Y 750..1350`. Greybox is simpler: one `GreyboxTest_Wall_South`, room surface `Y=-740`. So the S12 fix (rotate in place, nudge 2cm) would still have left every sign sealed inside a wall. **Facing, established empirically by 4-way test then deleted:** `/Engine/BasicShapes/Plane` at `Roll=90` stands vertical facing `+Y` (`scale.x = width/100`, `scale.y = height/100`); `TextRenderActor` at `Yaw=90` faces `+Y` and reads the correct way round (`Yaw=-90` renders mirrored). **Second root cause, the reason the text was invisible even once oriented:** `Font_UI_Overpass` and the other four `Font_UI_*` assets are `FontCacheType = Runtime` with **0 textures and 0 characters**. `TextRenderComponent` can only draw an **Offline** (texture-atlas) font, so assigning any `Font_UI_*` renders nothing at all. **New asset: `Font_Sign_Barlow`** (`Content/LastTrain/UI/Fonts/`, Offline, 1024px atlas, 256 ANSI chars, distance-field alpha), imported via `TrueTypeFontFactory` from Barlow SemiBold. Barlow is one of the two typefaces the F5 spec names, is OFL, and is not a Johnston clone. NOTE `TrueTypeFontFactory` rasterises from an **OS-installed** font by name, so `Barlow-SemiBold.ttf` / `Barlow-Bold.ttf` / `Overpass[wght].ttf` were copied from `_incoming_assets/fonts/` into `~/Library/Fonts/` to make the import possible; the variable-weight `Overpass[wght].ttf` failed to rasterise, Barlow succeeded. **Sign geometry (both maps, original, no roundel):** violet `#6C4C9C` header bar over a charcoal `#16161C` field, name centred in sodium `#E0A030` (`TextRenderColor (224,160,48)`). CW Main 700x130cm at `X=4500`, West/East 500x100cm at `X=3300`/`X=6750` (the S11 X positions 2200 and 9000 are gaps in the wall row, not wall). GBX Main 700x130 at `X=0`, West/East 500x100 at `X=-900`/`X=900`. Panels at `Roll=90`, text at `Yaw=90`, layered charcoal / bar / text at 3 / 5 / 8cm proud of the wall. **Pictograms:** all 26 `T_Picto_*` moved off `TC_EditorIcon` to `TC_Default` + `TEXTUREGROUP_World` (verified 0 non-conforming), so they cook. Greybox brought from 6 to **9** actors (added `fire_alarm_call_point`, `electricity_warning`, `general_warning`) and all 9 re-laid at `Roll=90` so they stand on the wall rather than lying flat. `MI_Picto_no_access_unauthorised` renders blank (source PNG appears near-empty) so `general_warning` was used in its place; the unused MI is left in place. **Departure board (F5 scope item 3):** `BP_DepartureBoard` was already fully wired (`OnCountdownChanged` -> format -> `Set Text` on its `BoardText` component) but **had never been placed in either map**. Placed as `CW_DepartureBoard` mid-platform at `(5625, 1057, 258)` with a charcoal face + violet header (`CW_DepartureBoard_Face` / `_Bar`). Blueprint component template updated with the offline font, sodium colour, centre alignment and `Yaw=90` facing, then compiled clean. **Verified live in PIE:** the board reads `30s inbound` -> `25s inbound` -> `19s inbound` over real time, driven by the actual `ALTTrain` state, and an in-game screenshot from the platform shows `CANARY WHARF` legible in sodium on charcoal. PIE stopped cleanly both times; no asset or actor was mutated while PIE was running. **Gameplay actors untouched:** CW `CW_PlayerStart (5625,1125,100)` and `CW_Train` unmoved, lights unchanged at 55, actor count 371 -> 374 (the 3 new board actors only). GBX `GreyboxTest_PlayerStart (-1300,0,100)` unmoved, 127 -> 130 (the 3 new pictograms only). Map Check on both maps: **0 errors / 0 warnings** immediately after save (the stock "needs lighting rebuilt" error reappears after a PIE cycle, as on every map since S2). **Remaining F5 work:** (1) the board's `WorldSize` is reset to 48 at runtime by the Blueprint despite the template saying 26, so the face is sized to the 48 that actually ships - worth tracking down. (2) `TextRenderComponent` uses the engine's **lit** `DefaultTextMaterialOpaque`, so sign text dims in unlit pockets (the West sign at CW `X=3300` is the visible case). Two attempts at an unlit variant both rendered solid quads instead of glyphs (`FontSample` samples fixed UVs, and duplicating the engine material and flipping it to Unlit breaks the opacity mask), so this needs a real fix in the F2 lighting pass or a purpose-built material. (3) `M_LT_SignTextUnlit` was created during that attempt and could not be deleted (held in memory); it is **not committed** and was removed from disk. (4) Wayfinding "Way out" / "To trains" / platform-number panels (F5 scope item 2) and the original network schematic wall map (item 4) are **not done**. |
| S9 Zombie appearance | **MINIMAL PASS DONE 2026-09-08. City Sample Crowds (varied clothed bodies) is still the real fix.** No `Source/` change. Root cause found in PIE: `M_Zombie_Tintable` (the parent of all five `MI_Zombie_*`) was a flat untextured lit material with only a `TintColour` vector AND `used_with_skeletal_mesh = false`, so every spawned zombie fell back to the engine Default Material and rendered as the clean bright Quinn mannequin. <br> **What changed:** <br> - **`M_Zombie_Tintable` rebuilt** (same asset path, so all five `MI_Zombie_*` inherit it unchanged, and the C++/BP tint-swap wiring is untouched): now `used_with_skeletal_mesh = true`; samples the Quinn D/N/MRA textures for real flesh detail; desaturates the albedo 85% and multiplies by `DeadFleshColour` (default `0.19,0.21,0.17` - muted cold grey-green, no bright green); applies the existing per-type `TintColour` as a light accent via `TintStrength` (0.35) so sprinter still reads faintly sodium / brute faintly crimson etc. without a paint job; `RoughnessBoost` (+0.15) on the MRA green channel. <br> - **Blood layer baked into the same material**: `BloodMask` samples `T_SurfaceImperfections011_Opacity` (ambientCG CC0, from the S-of-S4 haul), tiled `BloodTiling` (2.0), contrast `BloodContrast` (3.0) x `BloodAmount` (0.5), lerps the albedo toward `BloodColour` (`#B02030`-ish dark crimson) and drops roughness to 0.22 where blood is heavy (wet look). Every zombie gets the same blotch pattern - fine at gameplay distance in the dark station, costs nothing per spawn (no decal components, no dynamic MID). <br> - **`BP_Zombie`**: `CharacterMesh0` default `override_materials` set to `MI_Zombie_Walker` on both slots (so it looks right even before the BP's runtime type-swap), and `global_anim_rate_scale` 1.0 -> **0.8** as the shamble knob. NOTE: `ULTZombieTypeData::AnimPlayRate` is plumbed in C++ (`GetAnimPlayRate`) but the shared stock `ABP_Unarmed` AnimBP does not consume it, so per-type anim speed still is not differentiated - a real shamble/lunge anim set (or an AnimBP that reads `GetAnimPlayRate`) is still needed. DA `anim_play_rate` values left at their canon numbers. <br> - **`M_LT_ZombieWound`** (new, `/Game/LastTrain/Materials/`): a valid deferred-decal crimson wound material (translucent blend, MASKS sampler on `T_SurfaceImperfections013_Opacity`). Not wired to anything yet - spare, for hand-placed wound decals later. <br> **Also fixed in this commit: `M_LT_PBRSurface`** (the S4-haul surface master committed in `5e3e609`) failed to compile on `SF_METAL_SM6` - its `TextureSampleParameter2D` nodes had Normal/Grayscale sampler types with no assigned default texture, so it fell to the engine Default Material. Rebuilt: every sampler param now has a matching engine default texture (`WhiteSquareTexture` for colour slots, `DefaultNormal` for the normal slot) assigned before the sampler type is set; all colour-role samplers are `SAMPLERTYPE_COLOR`. The 17 `MI_<Set>` under `/Game/LastTrain/Surfaces/` inherit the fix. **Lesson for future material scripting on Mac: assign a compatible default texture to a `TextureSampleParameter2D` BEFORE setting `sampler_type`, and match `sampler_type` to the assigned texture's actual compression (`TC_MASKS` -> `SAMPLERTYPE_MASKS`, etc.) or the Metal compile silently fails and the Default Material renders.** <br> **Verified in PIE (read-only, Canary Wharf):** round 1 spawns, zombies render `MI_Zombie_Walker` -> base `M_Zombie_Tintable` (not the engine default), close-up shows desaturated dead-flesh skin with crimson blood streaks on chest/face/limbs. All four materials (`M_Zombie_Tintable`, `M_LT_ZombieWound`, `M_LT_PBRSurface`) pass a fresh forced recompile with 0 failures. PIE stopped cleanly; all in-PIE test mutations were on the throwaway PIE world only. |
| S13 Main menu (launch map -> start a run) | **DONE 2026-09-09. Editor + Blueprint only, no `Source/` change.** New files: `Content/LastTrain/Maps/L_MainMenu.umap`, `Content/LastTrain/Blueprints/BP_MenuGameMode.uasset`, `Content/LastTrain/UI/WBP_MainMenu.uasset`; plus `Config/DefaultEngine.ini` (`GameDefaultMap=/Game/LastTrain/Maps/L_MainMenu`) and `Config/DefaultGame.ini` (`+MapsToCook` for `L_MainMenu`, first in the list). <br> **Menu map:** cheap dark box under folder `MainMenu/` - a charcoal enclosure cube + floor (`MI_LT_Enclosure`), a back-wall panel (`MI_LT_Wall`), one violet spot (`MM_VioletAccent`, `#6C4C9C`) + one dim sodium point fill (`MM_SodiumFill`, `#E0A030`), both Movable, one `CameraActor` (`MM_MenuCamera`). No nav, no round manager, no spawns. Map Check: 1 error / 0 warnings - the same stock "Maps need lighting rebuilt" message both greybox maps carry (S2/S4 note), benign under Lumen. <br> **`BP_MenuGameMode`** (parent = engine `GameModeBase`, NOT `ALTGameMode` - no run lifecycle): `DefaultPawnClass = DefaultPawn`, HUDClass left at engine default `AHUD` (draws nothing without widgets). BeginPlay: Create Widget (`WBP_MainMenu`) -> store into `MenuWidget` var -> Add to Viewport -> Get Player Controller -> Set Show Mouse Cursor(true) -> Set Input Mode UI Only (focus = the menu widget). Set as `L_MainMenu` World Settings GameMode Override. <br> **`WBP_MainMenu`** (13 widgets): charcoal ground `#16161C`; centred column with a title group - violet name-bar (`#6C4C9C`, echo of the S11 sign geometry, not a roundel) with "LAST TRAIN" in `Font_UI_Barlow` Bold 76 off-white, "A STATION IS THE ARENA" subtitle in `Font_UI_OverpassMono` sodium under it; two stacked buttons in `Font_UI_PublicSans` SemiBold 22 - `StartRunButton` "START RUN" and `QuitButton` "QUIT", both `IsFocusable`, RoundedBox style with idle dark / hover violet fill / pressed brighter violet, and a sodium 2px outline on hover+press for a visible focus state; `DesiredFocus = StartRunButton`; "greybox build" string bottom-left in `Font_UI_OverpassMono` muted. START RUN -> `Open Level (by Name)` `L_CanaryWharf_Greybox`; QUIT -> `Quit Game`. <br> **Verified in scripted PIE (read-only):** launched `L_MainMenu` - `BP_MenuGameMode` builds the widget, it renders (title, bar, both buttons, build string), `bShowMouseCursor = true`. Fired `StartRunButton.OnClicked` (the Slate hit-test could not be driven from a normalised click through the bridge, so the delegate was broadcast directly - it runs the exact wired graph). World switched to `UEDPIE_0_L_CanaryWharf_Greybox`; HUD live (ROUND 1, 500 pts, "Stag Compact" 30\|90), zombies spawned and approaching, `LTGameState.RunState = "Active"`. PIE stopped clean. Menu + first-run-frame screenshots sent to the user. <br> **Follow-ups (noted, out of scope for S13):** (1) SELECT STATION sub-panel (Canary Wharf / Greybox Test) was skipped as not-quick - add later if wanted. (2) No "return to main menu" from the death/pause flow yet - wire it into `WBP_HUD` / the downed screen when that gets its art pass (`Open Level "L_MainMenu"` on a button, plus a pause menu). (3) L_MainMenu carries the same stock 1-error Map Check as the greybox maps; a real lighting/atmosphere pass on the menu box is Phase F polish, not a blocker. |

Notes / anything done differently from spec will be appended here as it happens.

**2026-09-09 (F5 signage and wayfinding):** rebuilt every station-name sign on both
maps, moved the 26 pictogram textures to a cookable group, brought greybox to 9
pictograms, and placed the departure board for the first time with its live
countdown verified in PIE. See the F5 row for the two root causes S12 did not
catch: the signs were on the wrong wall, and every `Font_UI_*` asset is a Runtime
font that `TextRenderComponent` cannot draw at all. Three Barlow/Overpass TTFs
were copied into `~/Library/Fonts/` because `TrueTypeFontFactory` imports by OS
font name, not from a file path; that is a change outside the repo and can be
undone by deleting those three files. Wayfinding panels and the network schematic
from the F5 spec are still outstanding.

**2026-09-09 (S13 main menu):** added `L_MainMenu` as the launch map with
`BP_MenuGameMode` + `WBP_MainMenu` (see the S13 row). The player now lands on a
menu at launch and START RUN opens `L_CanaryWharf_Greybox` with the run going
Active. Committed as its own step; only the three new `.uasset`/`.umap` files
plus the two `.ini` edits and this handover row. NOTE at session start the
working tree was NOT clean despite the git snapshot saying so: a `docs/tasks/`
reorganisation (phase-a/b/c/e specs staged for deletion, new `phase-f*.md`
specs, `asset-research-phase-f.md`, `editor-crash-endplaymap.md`) from an
earlier session/hand was already in progress and was left untouched - not part
of the S13 commit.

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

## S12 Opus fresh-eyes review

**RESOLVED 2026-09-09 by F5.** Defects 1, 2, 3 and 4 below are fixed (see the F5 row);
defect 1 was worse than described here, the signs were on the wrong wall entirely, not
just mis-rotated. Defect 5 (the mirrored pack marquee) is still open and still out of
scope. Kept below as the record of what the review found.

Cold review of `5e3e609`, `782f39f`, `f635ece`. Read-only: no commits, no asset edits, no `Source/` edits. Both maps loaded and PIE'd via the MCP bridge, both PIE sessions stopped with `playtest_stop`. Map Check run as `map_check({deprecated_only=true})` only, never bare `MAP CHECK`.

### Ranked defect list

**1. MAJOR - all six station name signs render their text edge-on, invisible from the platform**

*Where:* `GBX_Sign_Main_Text`, `GBX_Sign_West_Text`, `GBX_Sign_East_Text` (`L_GreyboxTest`); `CW_Sign_Main_Text`, `CW_Sign_West_Text`, `CW_Sign_East_Text` (`L_CanaryWharf_Greybox`).

Every sign is a Back + Bar + Text triple. The Back and Bar panels are `Pitch=0, Yaw=-90` giving a facing normal of `(0,-1,0)` - correctly turned into the room. All six Text actors are `Pitch=90, Yaw=0`, whose forward vector is `(0,0,-1)`, i.e. pointing straight down. A `TextRenderComponent` draws in the plane facing its local +X, so the glyphs are perpendicular to the panel they belong to and read as a hairline from any player-height viewpoint.

Compounding it, each Text actor sits 2cm *behind* its panel on the wall side, not in front:

| Sign | Panel Y | Text Y | Room side |
|---|---|---|---|
| GBX Main | -744 | **-746** | player is at Y > -744 |
| CW Main | 81 | **79** | player is at Y > 81 |

So even correctly rotated the text would be occluded by its own backing panel.

*Why it matters:* the station name is the single piece of original wayfinding S11 set out to add, and it is the identity of the arena. It is currently not readable in either map. The commit message claims "Text is Overpass, 'GREYBOX TEST' / 'CANARY WHARF', sodium-on-charcoal", and the handover row records S11 as DONE - neither is observably true in-game, so the docs overstate what shipped.

*Suggested fix:* set the Text actors to `Pitch=0, Yaw=-90` to match their panels, and move them to the room side of the panel (GBX `Y=-742`, CW `Y=83`, i.e. panel Y offset by +2 toward the player rather than -2). Verify by screenshot from the player start, not by property read-back - a read-back would have passed this defect.

**2. MAJOR - sign text colour is near-white, not the sodium the commit and docs claim**

*Where:* `TextRenderColor = (R=238, G=235, B=235, A=255)` on all six Text actors, both maps.

That is `#EEEBEB`, an off-white. The commit message and the S11 handover row both say "sodium-on-charcoal". Sodium in the project palette is `#E0A030` (`R=224, G=160, B=48`). The value shipped is not that colour and is not in the four-colour palette at all.

*Why it matters:* CLAUDE.md fixes the palette as non-negotiable, and off-white is the most "generic default" reading a sign can have - it is exactly the look the art direction is trying to avoid. It is also a documentation mismatch: the record says a thing was done that measurably was not.

*Suggested fix:* set `TextRenderColor` to `(R=224, G=160, B=48)`. Cheap, and it is the difference between the sign reading as sodium signage and reading as untouched default text. Correct in the same pass as defect 1, since both need the same six actors touched and one screenshot to verify.

**3. MINOR - 26 pictogram textures are `TC_EditorIcon`, an editor-only compression class**

*Where:* every `T_Picto_*` in `/Game/LastTrain/UI/Pictograms/` (all 26, verified individually; imported in `5e3e609`).

`TC_EditorIcon` (= uncompressed RGBA8) is intended for editor toolbar and slate icons, not for textures sampled by in-world geometry. Comparison: the surface haul imported in the same commit correctly uses `TC_Default` (checked `T_Tiles133A_BaseColor`).

*Why it matters:* two costs. Memory - 26 x 512x512 x 4 bytes ~= 27MB uncompressed versus roughly 1.7MB under BC7, on a project whose only measured frame reading is already GPU-bound (defect 6). And correctness - uncompressed editor-icon textures are the kind of thing that behaves differently or gets stripped in a cook, and this content is destined for a packaged build via the new `MapsToCook`.

*Suggested fix:* re-import or retarget the 26 to `TC_Default` (they are RGBA with a real alpha channel, which the pictogram material's Opacity pin consumes, so do not use `TC_DXT1`/`CompressionNoAlpha`). Only 14 are currently placed, but fix all 26 while the pass is open. Worth doing before the packaged-build frame reading, so that measurement is not taken against 27MB of avoidable VRAM.

**4. MINOR - greybox has 6 pictogram actors, the commit message and handover both say 9**

*Where:* `L_GreyboxTest`. Present: `no_smoking`, `emergency_exit_left`, `emergency_exit_right`, `first_aid`, `fire_extinguisher`, `slippery_floor_warning`. Canary Wharf's 14 are correct and match exactly.

*Why it matters:* purely a docs-versus-reality mismatch, no gameplay impact, but the count is stated twice (commit body and the S11 handover row) and is wrong in both. Given defects 1 and 2 are also "the record says it shipped, it did not", this is the third instance of the same pattern in one commit and is the reason to distrust the S11 row as a whole.

*Suggested fix:* either place the 3 missing pictograms or correct both the handover row and the count. Prefer placing them - the greybox box is visibly sparser than Canary Wharf.

**5. NIT - pre-existing pack marquee renders mirrored English station text**

*Where:* `GBX_D_Marquee_0/1/2`, component `wall_curvedmarquee_300_01` (SubwayTrain pack). Visible in PIE on the right-hand wall reading "...s at platform" reversed.

Not an S11 defect - these came in with the S4 dressing pass and are untouched by the three commits under review. Flagging because it is conspicuous in any greybox screenshot and because third-party pack signage carrying its own English transit wording is exactly the category CLAUDE.md wants replaced with original work. The same PIE frame also shows one marquee floating mid-air at a tilt clipping through a light fixture, which matches the known S4 "deferred / polish" note in the handover.

*Suggested fix:* out of scope here; fold into the Phase F signage replacement when the pack props get swapped for original assets.

### Categories that are clean

**Material assignments - CLEAN.** Swept every mesh, text and decal component on both maps: 70 material assignments in `L_GreyboxTest`, 234 in `L_CanaryWharf_Greybox`. **Zero `WorldGridMaterial`.** No checkerboard, no unintended fallback, confirmed both by property read and by PIE screenshot (floor concrete and wall tile both render correctly).

The 14 Canary Wharf actors on `BasicShapeMaterial` are **exactly** the documented intentional list, no more and no fewer: `CW_TrainCarriage_1/2`, `CW_Escalator_Left_Ramp`, `CW_Escalator_Left_Landing`, `CW_Escalator_Right_Ramp`, `CW_Escalator_Right_Landing`, and the 8 anchors `CW_Anchor_WallBuy_1..4`, `CW_Anchor_Perk_5/6`, `CW_Anchor_LostProperty_7`, `CW_Anchor_UpgradeBench_8`. Confirmed not a defect.

All 17 surface MIs parent to `M_LT_PBRSurface`; all 14 `MI_Picto_*` parent to `M_LT_Pictogram`, each with its own distinct texture (no crossed or reused assignments). All six new/rebuilt materials open and carry sensible settings, including the S9 fix `M_Zombie_Tintable bUsedWithSkeletalMesh=True` and `M_LT_ZombieWound MaterialDomain=MD_DeferredDecal`. The 32 `(None)` entries on `CW_D_Light_*` are empty override arrays - the mesh uses its own pack material, not a fallback.

**Signage against the legal rules - CLEAN.** No roundel, no Johnston or New Johnston, no line diagram, no operator livery, in any asset inspected.

Sign material colours are correct linear encodings of the palette, read straight off the graph constants:

- `M_LT_SignCharcoal`: `0.0185, 0.0185, 0.0224` -> sRGB `#16161C` charcoal, exact
- `M_LT_SignViolet`: `0.156, 0.076, 0.288` -> sRGB `#6C4C9C` violet, exact, x1.4 emissive on the bar

Both unlit opaque, which is right for signage. `M_LT_Pictogram` is unlit translucent with Emissive + Opacity wired and nothing else - correct for an alpha wayfinding decal.

Pictograms are genuine ISO 7010 safety symbols from Wikimedia, an international safety standard rather than operator IP, with `PICTOGRAM-SOURCES.txt` carrying per-file provenance and public-domain lines. Fonts are Overpass via `Font_UI_Overpass` (OFL 1.1), with `OFL.txt` present for all five families (Overpass, Overpass Mono, Public Sans, Barlow, Barlow Condensed). Nothing reads as TfL branding. Note the sign *colour* finding is defect 2 - a palette-compliance miss, not a trademark one.

**DefaultGame.ini - CLEAN, syntax and paths correct, no clobber risk.** Verified against UE 5.8 engine source rather than assumed:

- `MapsToCook` is declared `TArray<FFilePath>` in `Engine/Source/Developer/DeveloperToolSettings/Classes/Settings/ProjectPackagingSettings.h:561`, so `+MapsToCook=(FilePath="...")` is the correct struct-array form and `+` is the right append operator.
- The class is `UCLASS(config=Game, defaultconfig)`, so `DefaultGame.ini` is the correct file.
- `[/Script/UnrealEd.ProjectPackagingSettings]` is the engine's own canonical section name despite the class having moved modules - it appears verbatim in `BaseGame.ini:87`, `ConfigRedirects.ini:36` and the per-platform `MacGame.ini`/`IOSGame.ini`. Not stale.
- Both map paths resolve to real assets (both loaded successfully this session).
- No clobber risk: the file adds only `+MapsToCook` and sets no key that `BaseGame.ini` defines, so every engine packaging default (`UsePakFile`, `bUseIoStore`, compression settings, cultures) is inherited untouched.

Both maps will cook. One adjacent gap worth noting, strictly outside these three commits: `Config/DefaultEngine.ini` `[/Script/EngineSettings.GameMapsSettings]` sets only `GameInstanceClass` and defines **no `GameDefaultMap`**, so a packaged build has no startup map and will boot to the engine default. That will need setting before the packaged-build frame reading (S6 leftover) can be taken.

**LFS pointer integrity - CLEAN.** Every `.uasset` and `.umap` across all three commits scanned programmatically via `git show <sha>:<path> | head -1`; all print `version https://git-lfs.github.com/spec/v1`. Zero raw binaries. `check_content.py` passes: 212 tracked content assets.

**British spelling and docs - CLEAN.** `check_docs.py` passes (40 markdown, 0 JSON). No `organiz*`, `color`, `behavior`, `customiz*` in touched strings or docs. User-facing asset strings eyeballed: sign text is `"GREYBOX TEST"` / `"CANARY WHARF"` (place names, no spelling exposure); asset names use British spelling correctly, including `MI_Picto_no_access_unauthorised`. Note `docs/tasks/handover.md` is the only doc touched (in `782f39f`) and it passes.

**No `Source/` drift - CLEAN.** Confirmed for all three commits. `5e3e609` and `f635ece` touch nothing under `Source/`. The apparent grep hit on `782f39f` is the string "no `Source/` change" in the commit *message* body, not a file path - its five changed files are 4 assets plus `docs/tasks/handover.md`.

**Fresh PIE on both maps - RUN, read-only, no missing or pink materials.** Canary Wharf: PIE started, HUD live (Round 1, 500 points, "Stag Compact" 30|90), zombies spawned rendering the S9 dead-flesh material rather than the bright default Quinn mannequin, confirming the `782f39f` fix holds at runtime. Greybox: PIE started, geometry and retinted surfaces render correctly. Both sessions ended with `playtest_stop` and confirmed `playing=false, in_progress=false`. `map_check({deprecated_only=true})` on Canary Wharf: **0 errors, 0 warnings, passed=true, clean=true**. Message log empty - no accumulated asset warnings. No log errors reference any new asset.

One observation, not a defect of these commits: both maps read very dark, with a blown-out sodium pool near the lights and near-total black beyond. Playable and clearly intentional as atmosphere, but it is the reason defects 1 and 2 were invisible to property-level checking and only surfaced on screenshot - worth a lighting-readability look when the signs are fixed.

### Verification caveats

Two limits worth recording. The bridge's `read_log` caps at 100 lines regardless of the count requested, so "no log errors" rests on the Message Log being empty plus targeted searches, not a full-log sweep. And PIE console `setpos`/`ghost` did not take effect through the bridge, so the signage was assessed from the default player-start view plus exact transform/rotation maths rather than a walk-up screenshot of each of the six signs individually - the defect 1 mechanism is confirmed by the vector computation (text normal `(0,0,-1)` versus panel normal `(0,-1,0)`) and by the uniformity of the rotation across all six.

## The one line

Phase A is done. **Phase B: S5 (screamer scream), S6 (frame reading) and S7
(HUD polish review) are all done. The frame gate as literally worded FAILS -
~26.5 fps with a 24-36 zombie horde in a packaged Mac Development build on an
Apple M4 at 1280x720, GPU-bound. But the game thread is 5 ms flat at 36 alive,
so the crowd logic (B1) is not the problem; the frame cost is Lumen + the S9
zombie material + the S4 dressing, and hitting 60 is a Phase F art/perf pass
(scalability, Lumen tuning, material cost), not a Phase B blocker.** See the S6
row for the full editor + packaged numbers.
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
| Not verified | The screamer scream **cancel window** only (S5 PASSED the scream itself, summon and count - the 0.5s cancel branch is a read-and-judged 3-line time guard). <br> The Phase B 60 fps frame gate is **measured and FAILS**: packaged Mac Development build, Apple M4, 1280x720, ~26.5 fps at 24-36 zombies alive, GPU-bound (game thread ~5 ms). Not a logic regression - it is a lighting/material perf problem for Phase F. See the S6 row. |
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
| Crowd frame gate, 24 to 40 at 60fps | `phase-b1-throttled-repath.md` | MEASURED (S6, editor + packaged): packaged Mac Development, M4, 1280x720, **~26.5 fps at 24-36 alive, GPU-bound**, game thread ~5 ms. Gate as worded FAILS; crowd logic is not the cause. Phase F perf pass needed. |
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

### Phase B, code complete, frame gate measured and FAILING on perf

B1 throttled repath, B2 interaction and the first wall buy, B3 the HUD widget.
The zombie attack fix and the HUD health bar are verified in PIE. The HUD polish
review is done (S7, 2026-09-09) - wiring matches B3, five minor findings, none
blocking. **The crowd frame gate is measured (S6, 2026-09-09) and FAILS the
60 fps target:** packaged Mac Development build, Apple M4, 1280x720, ~26.5 fps
with 24-36 zombies alive, GPU-bound (game thread ~5 ms flat). The crowd's logic
is not the cause - the base scene is already ~29 ms GPU before any real horde -
so this is a Phase F lighting/material/scalability problem, not a Phase B logic
regression, and B1's throttled-repath design goal (crowd does not tank the sim)
holds. **Open, for a human at the keyboard:** the wall-buy walk-up in PIE (the
buy itself passed via property read-back). The corridor stall edge case has a
verdict and a fix as of 2026-09-07, unverified in PIE: open code item 1.
Whether the 60 fps number blocks Phase B sign-off or is deferred to Phase F is
a call for the project owner - the honest reading is now on record.

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

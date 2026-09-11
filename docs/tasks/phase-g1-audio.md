# G1: audio, the first sound in the game

Lane: Opus in the editor (CC-in-Unreal), plus one owner decision in section 7.
Depends on: the `Source/` hooks written on branch `claude/phase-g1-audio-prompt`
being compiled first. UE 5.8.

The game has no audio at all today. The `Source/` half of G1 is done: every
system that needed a trigger point now carries an optional `USoundBase`
property, null by default, played through a null check, so nothing below is
blocked on more C++. What is left is import, cue authoring, the performance
discipline a 40 strong horde needs, and assignment. That is all editor work.

Read `phase-f6a-zombie-surface-masks.md` for the shape of this task, and
`../known-issues.md` section 1 before trusting any file name in section 2.

## 0. Read this before importing anything

**The remote session that wrote this spec could not read
`_incoming_assets/audio/`.** That directory is gitignored (`.gitignore` line
80) and lives only on the Mac, so it is not in the remote container at all:
`../known-issues.md` 1.2 records the same limit in the other direction. Every
statement below about what is in the pool comes from
`../reference/free-asset-sweep-2026-09.md` sections 3.2 and 3.3 and from
`../reference/asset-sources-phase-f.md` section 5, **not** from the files.

Two consequences, and neither is optional:

1. **No file name in this spec is verified.** Section 2 names packs and the
   kind of sound wanted from each, never a specific `.wav`. Pick the actual
   file by listening.
2. **No attribution line in this spec is quoted from source.** Section 7 says
   what the repo's own research recorded about each licence. The
   `_incoming_assets/audio/SOURCES-*.txt` files are the authority and they win
   over anything here.

**Step 0, before any import:** open both `SOURCES-*.txt` files, and for each
pack you are about to use copy its exact licence and required credit line into
`Content/LastTrain/Audio/AUDIO-SOURCES.txt` (new file, same pattern as the
existing `Content/LastTrain/Lighting/HDRI-SOURCES.txt`) and add a row per pack
to `Content/ATTRIBUTION.md`. Then correct section 2 and section 7 of this file
in place, so the next session does not have to do this again. A pack whose
licence you cannot find in a `SOURCES` file does not get imported.

There is one known conflict to resolve while you are in there: the Phase G1
brief calls the footstep pack **CC-BY**, and
`../reference/free-asset-sweep-2026-09.md` section 3.2 lists OpenGameArt
"Footsteps on different surfaces" as **CC0 1.0 (confirm the block)**. One of
the two is wrong. The `SOURCES` file decides it.

## 1. What the C++ now offers

Every property is `EditDefaultsOnly, BlueprintReadOnly, TObjectPtr<USoundBase>`,
null by default, and every play call null checks first, exactly like the
Enhanced Input actions on `ALTPlayerCharacter`. Leaving one unassigned is
silence, never an error. Nothing here needs a Blueprint graph node: these are
class default values.

| Property | Class | Fires when | 2D or positioned | Assign on |
|---|---|---|---|---|
| `FireSound` | `ULTWeaponData` | every shot, in `FireOnce` | positioned, at the view point | `DA_Weapon_SMG` and every later weapon asset |
| `ReloadStartSound` | `ULTWeaponData` | `StartReload`, magazine out | positioned, at the view point | same |
| `ReloadCompleteSound` | `ULTWeaponData` | `FinishReload`, magazine in | positioned, at the view point | same |
| `DryFireSound` | `ULTWeaponData` | trigger pulled on an empty magazine with an empty reserve | positioned, at the view point | same |
| `IdleVocalSound` | `ALTZombieCharacter`, override on `ULTZombieTypeData` | every `IdleVocalIntervalSeconds`, jittered per instance | positioned, at the zombie | `BP_Zombie` for the fallback, `DA_Zombie_*` per type |
| `AggroVocalSound` | as above | once, on the zombie's first tick after spawn | positioned, at the zombie | as above |
| `AttackVocalSound` | as above | on the attack wind-up, with `OnAttackWindUp` | positioned, at the zombie | as above |
| `DeathVocalSound` | as above | in `Die`, with `OnDeathPresentation` | positioned, at the zombie | as above |
| `IdleVocalIntervalOverride` | `ULTZombieTypeData` | not a sound: above zero it replaces the 7s cadence | - | `DA_Zombie_*` per type |
| `ArrivalSound` | `ALTTrain` | with `OnArrivalStarted`, the inbound slide begins | positioned, at the train actor | `BP_Train` |
| `DoorOpenSound` | `ALTTrain` | with `OnDoorsOpen` | positioned, at the train actor | `BP_Train` |
| `DoorCloseSound` | `ALTTrain` | with `OnDoorsClose` | positioned, at the train actor | `BP_Train` |
| `DepartureSound` | `ALTTrain` | with `OnDepartureStarted` | positioned, at the train actor | `BP_Train` |
| `RoundStartSound` | `ALTRoundManager` | `StartRound`, before `OnRoundStarted` | 2D | `BP_RoundManager` |
| `RoundEndSound` | `ALTRoundManager` | `EndRound`, as the breather begins | 2D | `BP_RoundManager` |
| `HeatRiseSound` | `ULTStationHeat` | heat actually rises; the reset to 0 on travel is silent | 2D | the `ULTStationHeat` component on the placed round manager, in both maps |
| `InteractSound` | `ULTInteractionComponent` | a successful `TryInteract`, fired just before the interact | 2D | the `Interaction` component on `BP_PlayerCharacter` |

Three things the C++ deliberately does **not** carry, so do not go looking for
them:

- **No announcement sound properties.** The three announcement moments are
  still the `OnInboundAnnouncement`, `OnArrivalComplete` and
  `OnDepartureAnnouncement` Blueprint hooks on `ALTTrain`. Their wording is
  original work and a separate authoring task, and nothing in this project may
  transcribe or imitate a real operator recording or door chime.
- **No footstep hooks.** Footsteps belong on animation notifies, which need the
  locomotion set that Phase H is waiting on. The staged footstep pack is
  therefore imported by this task but wired by a later one.
- **No ambience or hum properties.** Those are placed `UAudioComponent` actors
  in the level, not one shots off a gameplay event. Section 6.

## 2. What to put on each hook

Pack names only. Pick the file by listening, and record what you picked.

| Hook | Pool | What to pick |
|---|---|---|
| `FireSound` | Kenney impact pack | The sharpest short transient in the pack, layered in the cue with a low thump if one shot alone reads thin. **This is a stand-in.** No real firearm layer is in the staged pool at all: see the note below. |
| `ReloadStartSound` / `ReloadCompleteSound` | Kenney impact and UI packs | Two different short mechanical clicks, the second lower and heavier than the first. |
| `DryFireSound` | Kenney UI pack | The driest, quietest click in the pack. It must not read as a shot. |
| zombie `IdleVocalSound` | 80 CC0 creature SFX, OpenGameArt | The low moans and breaths. Four to six variants into one cue with a random node, or every zombie on the platform is the same voice. |
| zombie `AggroVocalSound` | same | The short rising snarls. |
| zombie `AttackVocalSound` | same | The sharp barks. Shortest sounds in the set: this one fires on a 1.0s to 2.2s cooldown in contact. |
| zombie `DeathVocalSound` | same | The falling gurgles. |
| `ArrivalSound` | CC0 Freesound synth drones | A rising drone, pitched and faded into an approach. |
| `DoorOpenSound` / `DoorCloseSound` | Kenney sci-fi pack | Two mechanical servo movements, not a chime and not a bell. Read the legal section of `CLAUDE.md` before picking anything that sounds like an operator's door tone. |
| `DepartureSound` | CC0 Freesound synth drones | The arrival drone reversed and falling, so the two read as one system. |
| `RoundStartSound` | Kenney digital pack | One low hit. Restrained: this fires every round for the whole run. |
| `RoundEndSound` | Kenney digital pack | The same hit inverted or pitched down. The round ending is a relief, not a fanfare. |
| `HeatRiseSound` | Kenney sci-fi or digital pack | One unpleasant rising tone. This is the only feedback that staying has cost something. |
| `InteractSound` | Kenney UI pack | A soft confirm. It plays on every wall buy and every board, so it has to survive repetition. |
| ambience bed, section 6 | CC0 Freesound synth drones | The flattest, least eventful drone in the set. |
| menu and low tension bed, section 6 | MundoSound Dark Ambient Loop | **CC-BY, see section 7.** Instrumental, no voice, seamless, about 1:54. |
| footsteps, not wired by this task | footstep surface pack | Import and organise only. |

**The gunshot gap is real and worth flagging to the owner now.** The staged
pool is UI, impact, sci-fi, digital, creature, drone and footstep material. It
has no firearm recordings, because the sweep that produced it was not looking
for any. A Kenney impact stands in perfectly well for a grey box, and will not
survive Phase H's weapon presentation pass.
`../reference/free-asset-sweep-2026-09.md` section 3.2 already puts the Sonniss
GDC bundle in front of the owner as the answer, royalty free and commercial
safe but not CC0. That decision is theirs, and G1 does not need it.

## 3. Import settings

Import to `Content/LastTrain/Audio/`, in subfolders: `Weapons/`, `Zombies/`,
`Train/`, `Stingers/`, `Ambience/`, `Footsteps/`.

Per `USoundWave`. The engine spells its loading property the American way, so
it is referred to here by its values rather than its name:

- **Short one shots**, everything under about two seconds: loading set to
  `Force Inline`, `Streaming` off. These fire during combat and must not take a
  streaming hitch.
- **Beds and loops**, the drones and the MundoSound loop: loading set to
  `Retain On Load` off, streaming on, `Looping` on where the file is a true
  seamless loop.
- `Compression Quality` 40 is the engine default and is right for everything
  here except the beds, which can go to 30.
- Mono for everything positioned. A stereo asset on a positioned sound either
  collapses or refuses to spatialise depending on the attenuation settings, and
  either way it is not what you asked for. The two beds may be stereo.
- Nothing in this pool should need `Seek-able Streaming`.

## 4. Attenuation, concurrency and sound classes: the crowd discipline

This is the part that matters, and the reason the C++ plays positioned one
shots rather than spawning attached audio components. Do it before assigning
anything.

**The numbers to design against.** `ALTRoundManager::MaximumAlive` is 24, and
`GetEffectiveMaximumAlive` adds `LiveCapPerHeat` 6 per heat level, so heat 3 is
42 alive and the ceiling is higher still. The default idle cadence is 7s. Forty
alive on a 7s cadence is a vocal every 0.18 seconds, forever, before a single
attack or death. Without concurrency limits that is a wall of noise and a real
CPU cost, and `../known-issues.md` 2.8 records that there is no frame rate
baseline to measure the damage against.

Create these assets under `Content/LastTrain/Audio/Config/`:

| Asset | Kind | Settings |
|---|---|---|
| `ATT_Weapon` | Attenuation | Natural sound, inner radius 600, falloff 9000. The platform is about 10,950 uu long, so a shot should be audible end to end and clearly distant at the far end. |
| `ATT_ZombieVocal` | Attenuation | Natural sound, inner radius 300, falloff 2800. Spatialisation on. Air absorption on. Occlusion **off**: at 40 alive the trace cost is not worth it, and the station is one open hall. |
| `ATT_Train` | Attenuation | Natural sound, inner radius 1500, falloff 7000. The train is 9,420 uu of actor and the sound plays at its origin, so a wide inner radius stops it reading as a point source at the cab. |
| `CON_ZombieVocal` | Concurrency | Max count **5**, resolution `Stop Quietest`, volume scale 0.75 on duplicates, per sound owner off (the limit is global, that is the point). |
| `CON_WeaponFire` | Concurrency | Max count 8, resolution `Stop Oldest`. |
| `CON_Stinger` | Concurrency | Max count **1**, resolution `Stop Oldest`. A round stinger must never overlap a heat stinger. |

Sound classes under the same folder, all children of the engine master:
`SC_Weapon`, `SC_Creature`, `SC_Train`, `SC_Stinger`, `SC_Ambience`. Set them
on the cues as you build them. They exist so section 6's ducking has something
to duck, and so a mix pass in G3 has handles.

**The screamer ducks the mix.** `../design/gameplay-canon.md` section 6 says
the scream "is loud and directional and masks other audio while it lasts". Do
that with a `USoundMix` that pushes `SC_Creature` and `SC_Ambience` down by
about 12 dB for two seconds, pushed and popped from the existing `OnScream`
Blueprint hook on `BP_Zombie`. Not in C++, and not by turning the scream up.

## Verification gate

Do not assign a single sound to a gameplay asset until all of this passes. It
is quick, and it is here because an audio asset that is silently misconfigured
sounds exactly like an audio asset nobody assigned.

1. Every imported `USoundWave` plays in the content browser preview, is mono
   where section 3 says mono, and has the loading values section 3 asks for.
2. Every cue plays in the cue editor, and every cue with a random node is
   played five times and gives more than one result.
3. Every positioned cue has its attenuation asset set and every vocal cue has
   `CON_ZombieVocal`. A positioned cue with no attenuation plays at full volume
   everywhere in the level and is the single easiest mistake to make here.
4. `AUDIO-SOURCES.txt` and `Content/ATTRIBUTION.md` are filled in, per step 0,
   for every pack that has anything imported from it.
5. No `.wav` is committed. Only the imported `.uasset`, and each one is an LFS
   pointer: `git show <ref>:<path> | head -c 45` starts `version https://git-lfs`.

## 5. Assign

In this order, testing after each, so a mistake is attributable:

1. `DA_Weapon_SMG`: the four weapon sounds.
2. `BP_Zombie` class defaults: the four vocals as the fallback set, and leave
   `IdleVocalIntervalSeconds` at 7 for now.
3. `DA_Zombie_Walker` to `DA_Zombie_Screamer`: only the vocals that differ from
   the fallback. A null field on a type asset keeps the `BP_Zombie` value, so
   an asset that carries nothing is correct, not incomplete. The brute wants
   `IdleVocalIntervalOverride` around 10 and a lower, slower set; the sprinter
   around 4 and a faster, thinner one; the screamer's own scream stays on its
   `OnScream` hook, not on `AttackVocalSound`.
4. `BP_Train`: the four cycle sounds.
5. `BP_RoundManager`: the two stingers. Then the `ULTStationHeat` component on
   the placed round manager in **both** maps: `HeatRiseSound`.
6. `BP_PlayerCharacter`, `Interaction` component: `InteractSound`.

## 6. The beds, and silence

Three placed sounds, none of them driven by the C++ hooks:

1. **Station ambience**, an `AmbientSound` actor per map with the flat drone
   looping. Quiet. It should be noticed only when it stops.
2. **Train hum**, an `AmbientSound` child of the train, started and stopped
   from `OnArrivalStarted` and `OnTrainAway` on `BP_Train`, with a fade. The
   cycle is 100s between arrivals and a 25s dwell, so this is the loudest thing
   in the station for a quarter of every cycle and has to sit under the
   gunfire, not over it.
3. **Menu bed**, the MundoSound loop on `L_MainMenu`. CC-BY, section 7.

"Silence is a tool" is the brief's line and it is a real instruction, not a
mood. Concretely: the breather between rounds (`BreatherSeconds` 10) gets no
music and no stinger past `RoundEndSound`, so the empty platform is genuinely
empty; and the 15 seconds between the inbound announcement and the train
stopping should thin out, not build. If every system is playing something at
once, the accept test below cannot pass, because nothing is distinguishable.

## 7. Attribution

**Everything in this section is second hand. Step 0 replaces it with the real
text from `_incoming_assets/audio/SOURCES-*.txt`.**

What the repo's own research recorded:

- **MundoSound "Dark Ambient Loop" series**, from OpenGameArt. Dual licensed
  **CC-BY 3.0 and OGA-BY 3.0**. `../reference/free-asset-sweep-2026-09.md`
  section 3.3 records the requirement as "Attribution to Lucas Calvo /
  mundosound.com required". That is a paraphrase in our own docs, not a quoted
  credit line: copy the exact wording the `SOURCES` file carries.
- **The footstep surface pack.** Licence disputed, see step 0. If it is CC-BY,
  it needs a credit line and cannot be used without one. If it is CC0 it needs
  none. Settle this before it is imported, not after.
- **Kenney packs** (impact, UI, sci-fi, digital): CC0 1.0, no attribution
  required. Credit them anyway, it costs nothing.
- **80 CC0 creature SFX**, OpenGameArt: CC0 1.0, no attribution required.
- **CC0 Freesound synth drones**: CC0 1.0, no attribution required. Synthesised
  rather than recorded, which is why they were chosen:
  `../reference/asset-sources-phase-f.md` section 5.4 records the reasoning,
  that a synthesised drone provably cannot contain an announcement.

**Open question for the owner, do not guess it.** `Content/ATTRIBUTION.md` is a
repo file. It is not in the shipped game and it is not on a store page, so it
does not discharge a CC-BY obligation on its own. A CC-BY asset in the build
needs the credit somewhere a player or a customer can reach: an in-game credits
screen, a legal or attributions panel in the options menu, or the store page,
or all three. Nothing in the project has one today and G1 must not invent the
answer. Two assets are affected: the MundoSound bed, and the footstep pack if
step 0 finds it is CC-BY. Both have CC0 alternatives in the same pool, so a
third option is to use neither and carry no obligation at all.

## Rules

- Never mutate an asset or an actor while PIE is running.
- `MAP CHECK` as a console exec is banned. `MAP CHECKDEP NOCLEARLOG`. See
  `editor-crash-endplaymap.md`.
- Commit the imported `.uasset` files through LFS. Never a raw `.wav`.
- No Call of Duty, Treyarch or Activision naming anywhere, in a file name, a
  cue name or an asset description.
- No transcribed operator announcement, no operator door chime, no operator
  jingle. Original material only.
- British spelling in every asset description and every note. No em or en
  dashes.
- Update the G1 row in `handover.md` and the Phase G table in `README.md` when
  this lands, and correct sections 2 and 7 of this file in place.

## Accept

- **The brief's test, which is the real one:** play a round with your eyes
  closed and say what is happening. A zombie behind you, a zombie in front, the
  round ending, the train arriving, the doors opening, your magazine running
  out. If you cannot, the mix is wrong, not the asset list.
- A full train cycle at Canary Wharf: arrival, doors, close, departure, and the
  heat stinger on the departure you did not board.
- Round 5 and round 10, so a sprinter round and the brute pair both vocalise
  with their own sets.
- 24 alive, and the vocal layer is dense but individual sounds are still
  distinguishable. If it is mush, `CON_ZombieVocal` is too loose or the idle
  intervals are too short: fix it there, not by turning things down.
- Fire until the reserve is empty and confirm the dry fire click is the only
  thing the trigger does.
- Nothing regressed: rounds, boarding and travel behave exactly as before. Every
  sound property is optional and a mistake here can only be silence or noise,
  never a gameplay change.
- All five CI gates pass.
- Note the frame cost with 24 to 40 alive against F7, if it is measurable.

## Not in G1

The mix pass, the announcement recordings and their wording, footstep notifies,
weapon audio that survives Phase H, reverb volumes per station area, and any
decision about the Sonniss bundle. Each is named above where it touches this
task.

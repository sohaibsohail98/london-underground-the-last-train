# G6: subtitles

Lane: CC-in-Unreal for the widget and the caption table, plus one small owner
decision in section 6. The `Source/` half is written: see section 1. UE 5.8.

Depends on: `phase-g1-audio.md` (the sound hooks this hangs off, and the
assets that give a caption something to caption) and `phase-g5-save-settings.md`
(the `bSubtitlesEnabled` toggle, which is the only thing that turns any of this
on). Nothing here is blocked on `phase-g2-hud.md`, but the widget should be
built in the typeface G2 settles, so doing it after G2 saves a font swap.

Subtitles were an owner decision in 2026-09 and had no spec before this file.

## 0. The bar

The same bar as the rest of the HUD: **nothing on screen that is not telling
you something.** A caption line is a fifth screen region that the restrained
HUD does not otherwise have, so it earns its place only while it is carrying a
line, and it is `Collapsed` the rest of the time.

Two rules follow, and the C++ already enforces both.

1. **Speech is always captioned. A noise is captioned only when it carries
   something the player cannot get any other way.** The magazine count is on
   the HUD, so a dry fire does not need a caption. A train is visible, but a
   platform announcement is not, and neither is the heat rise that is the only
   feedback that letting the train go cost you something.
2. **One line at a time, and it must stay still long enough to read.** The live
   zombie cap is 24 before heat and 42 at heat 3. A per zombie cue with no
   suppression changes the caption several times a second, which is not a
   subtitle system, it is a strobe.

## 1. What the C++ now offers

Three files, written on branch `claude/subtitles-prompt`, uncompiled: there is
no engine in a remote session, so treat all of it as unverified until it builds.

`Source/LastTrain/Public/Audio/LTSubtitleLine.h`

- `FLTSubtitleLine`, a `USTRUCT` deriving from `FTableRowBase`: `DisplayText`,
  `DurationSeconds` (zero derives a reading time from the length),
  `ELTSubtitleCategory Category`, `int32 Priority` (zero takes the category
  default), and `HasText()`.
- `ELTSubtitleCategory`: `Announcement`, `Round`, `Train`, `Zombie`, `Weapon`,
  `Interaction`. Heat shares `Round`: it is a run level stinger, played the way
  a round stinger is.
- `namespace LTSubtitleKeys`, the row names, shared by the call sites and by
  whoever authors the table so the two cannot drift over a typo.

`Source/LastTrain/Public/Audio/LTSubtitleSubsystem.h`

| Member | What it does |
|---|---|
| `OnSubtitleChanged(const FText& Subtitle)` | `BlueprintAssignable`. The line to show, or empty text meaning cleared. This is what the widget binds. |
| `ShowSubtitleForWorld(World, Key, Category, FallbackText)` | `static`, null safe at every step. What every C++ call site uses. |
| `Get(World)` | `static`. The subsystem for a world, or null. |
| `ShowSubtitleByKey(Key, Category, FallbackText)` | `BlueprintCallable`. The table row, or the fallback if there is no such row. |
| `ShowSubtitle(const FLTSubtitleLine&)` | `BlueprintCallable`. A line built in Blueprint. |
| `ShowSubtitleText(Text, DurationSeconds, Category)` | `BlueprintCallable`. A one off with no row behind it. |
| `ClearSubtitle()` | `BlueprintCallable`. Takes the line down early. |
| `AreSubtitlesEnabled()` | `BlueprintPure`. Reads the saved setting, see section 2. |
| `GetCurrentSubtitle()` | `BlueprintPure`. For a widget created after the broadcast it missed. |
| `SetSubtitleTable(UDataTable*)` | `BlueprintCallable`. Assigns the caption script at runtime. |
| `MinimumDurationSeconds` 1.6, `MaximumDurationSeconds` 6, `SecondsPerCharacter` 0.055, `RepeatSuppressionSeconds` 4 | The tuning. 0.055s a character is about 200 words a minute, the usual captioning rate. |

It is a `UWorldSubsystem`, the same base class as `ULTGoreDecalSubsystem`, and
for the same reason rather than for consistency alone: what it holds is world
scoped. The caption describes something that happened in this station, the
widget bound to the delegate belongs to this world's player, and the expiry
timer runs on this world's timer manager. Boarding travels to `NextStationMap`
through `OpenLevel`, and a caption about doors closing at the station you left
has no business arriving at the next one.

Three things keep it readable: **priority** (a line reaches the screen only if
it is at least as important as the one already there, and equal replaces, so a
second announcement follows the first), **repeat suppression** (the same key
cannot come back within four seconds) and **duration** (the line's own, or a
reading time from its length, clamped either way).

## 2. The toggle, and why there is only one of it

`ULTSubtitleSubsystem::AreSubtitlesEnabled()` reads
`ULTGameInstance::GetGameSettings().bSubtitlesEnabled` every time it is asked.
It does not cache it and it does not own a second copy of the flag, so there is
nothing to keep in step and a settings panel needs no wiring to this subsystem
at all: the moment `ApplyAndSaveSettings` writes the bool, the next caption
obeys it.

**`bSubtitlesEnabled` defaults to false.** Until the player turns it on, every
`ShowSubtitle` call is a no-op and nothing reaches the screen. That is the
correct default for a subtitle system, but it does mean that a build with no
settings panel shows nothing at all, which looks exactly like a broken system.
So: the subtitles checkbox in `phase-g5-save-settings.md` section "The work"
step 1 is no longer optional. That spec says to leave it out or ship it
disabled until the subtitle system lands. It has landed. Ship the checkbox.

## 3. The work

1. **`WBP_Subtitle`**, a new widget, or a new block inside `WBP_HUD`. Prefer a
   separate widget added to the HUD, so the run-over card and the downed
   overlay in `phase-g2-hud.md` can collapse the main HUD without taking the
   caption with it.
   - Bottom centre, above the interaction prompt, inside the same 5 per cent
     safe margin. It is the one new screen region the HUD gains and it is
     shared with nothing.
   - **One line.** Wrap to a second only if the text genuinely does not fit at
     16:9, and never a third. A caption block that grows upward over the
     platform is exactly the busy furniture G2 refuses.
   - `Font_UI_Overpass`, the functional face from `phase-g2-hud.md`
     "Typography", off-white. Not the mono face: this is prose.
   - **No background box.** A thin drop shadow or a soft charcoal `#16161C`
     scrim at low opacity behind the text only, and only if the platform's
     sodium lighting makes it unreadable in practice. Test it against a lit
     platform before deciding you need one.
   - `Collapsed` at rest, per principle 3 in `phase-g2-hud.md`. A short fade in
     and out, about 0.15s, is enough; do not animate the text itself.
2. **Bind `OnSubtitleChanged`.** Get the subsystem with Get World Subsystem,
   class `LTSubtitleSubsystem`, on Construct, then bind the event. Empty text
   means clear: collapse on empty, set and reveal on anything else. Do not poll
   on Tick, and do not run a timer of your own, the subsystem owns the expiry.
   `GetCurrentSubtitle()` exists for the case where the widget is built after a
   line went up.
3. **`DT_Subtitles`**, a `UDataTable` of `FLTSubtitleLine` rows under
   `Content/LastTrain/Audio/`, row names from `LTSubtitleKeys`. Call
   `SetSubtitleTable` once on the game mode's or the HUD's BeginPlay. Without
   it every call site falls back to the placeholder text compiled into the C++,
   which is legible but is not the shipping script: see section 4.
4. **Write the captions.** Section 4 is the key list and what each one is for.
5. **Check it against a real mix.** Section 6.

## 4. The caption script

**Full subtitle copywriting is a follow-up, not this task and not the branch
that wrote the system.** The C++ carries one placeholder line per wired call
site so the system is testable and legible today. The final wording of the
three announcements in particular cannot be written until the announcements
themselves are, which `phase-g1-audio.md` section 1 lists as a separate
authoring task. When that lands, the captions must match the recorded line
word for word, and the rows below replace the placeholders.

Wired on this branch, with the placeholder text the C++ carries:

| Key | Where | Category | Placeholder |
|---|---|---|---|
| `Train.InboundAnnouncement` | `ALTTrain::TickAway`, with `OnInboundAnnouncement` | Announcement | Platform announcement: the next train is approaching. |
| `Train.Arrival` | with `OnArrivalStarted`, beside `ArrivalSound` | Train | [The train rumbles into the platform] |
| `Train.ArrivalAnnouncement` | with `OnArrivalComplete` | Announcement | Platform announcement: this train is ready to board. |
| `Train.DoorsOpen` | with `OnDoorsOpen`, beside `DoorOpenSound` | Train | [Carriage doors slide open] |
| `Train.DepartureAnnouncement` | with `OnDoorsClose` and `OnDepartureAnnouncement` | Announcement | Platform announcement: the doors are closing. |
| `Train.Departure` | with `OnDepartureStarted`, beside `DepartureSound` | Train | [The train pulls away into the tunnel] |
| `Round.Start` | `ALTRoundManager::StartRound`, beside `RoundStartSound` | Round | [Low tone: the next round begins] |
| `Round.End` | `ALTRoundManager::EndRound`, beside `RoundEndSound` | Round | [Falling tone: the platform is clear] |
| `Round.HeatRise` | `ULTStationHeat::SetHeat`, on a rise only, beside `HeatRiseSound` | Round | [Rising tone: the station is growing restless] |
| `Interaction.Confirm` | `ULTInteractionComponent::TryInteract`, before the interact, beside `InteractSound` | Interaction | [Confirm tone] |
| `Zombie.Scream` | `ALTZombieCharacter::UpdateScream`, with `OnScream` | Zombie | [A scream carries down the platform] |

Two judgements in that table worth knowing before you change them. The closing
doors and the departure announcement fire on the same frame, so they share one
caption: the announcement says everything the door close says, and captioning
both would flash a line the other immediately replaces. And the screamer is the
only zombie captioned, because it is rare, it is the loudest thing in the
station and it means a wave has been called, where an idle moan at the live cap
would be the strobe section 0 rules out.

`Interaction.Confirm` is the weakest line in the table and the first candidate
to cut. The HUD already shows the prompt the player just pressed, so the
caption tells them something they know. It is wired because the sound is, and
it sits at the bottom of the priority order so it can never displace anything.
Cut it if it reads as noise in play.

## 5. Pending the G1 audio merge

These call sites do not exist on the subtitle branch at all, because they
arrive with `claude/phase-g1-audio-prompt`. Their keys are already declared in
`LTSubtitleKeys` so the session that merges the two wires them rather than
inventing new names. Each one needs a `ShowSubtitleForWorld` call beside the
`PlayVocal` or `PlaySound` call, exactly as the wired sites do.

| Key | Call site once G1 is in | Category | Recommendation |
|---|---|---|---|
| `Zombie.Aggro` | `ALTZombieCharacter::UpdateVocals`, the first tick spawn vocal | Zombie | Wire it. It is once per zombie and it is the "something has noticed you" cue. Watch it at 24 alive; if it strobes, raise `RepeatSuppressionSeconds`. |
| `Zombie.Attack` | `ALTZombieCharacter::TryAttack`, with `OnAttackWindUp` | Zombie | Wire it. A swing you cannot hear coming is the most expensive thing to miss. |
| `Zombie.Death` | `ALTZombieCharacter::Die`, with `OnDeathPresentation` | Zombie | **Do not wire it.** Forty kills a round, each already confirmed by the hit marker and the points delta. Nothing to add. |
| `Zombie.Idle` | the jittered idle cadence in `UpdateVocals` | Zombie | **Do not wire it,** and no key is declared for it. A vocal every 0.18s at the cap, per `phase-g1-audio.md` section 4. |
| `Weapon.DryFire` | `ULTWeaponComponent`, trigger on an empty magazine and empty reserve | Weapon | Owner decision. The HUD already shows the magazine at zero in crimson, so this is a duplicate by rule 1 in section 0. Recommend not wiring it. |
| `Weapon.ReloadStart` / `Weapon.ReloadComplete` | `StartReload` and `FinishReload` | Weapon | Same, and the G2 reload indicator says it better. Recommend not wiring either. |

There is no per type caption override on `ULTZombieTypeData`, deliberately, even
though the sounds have one. A brute and a walker want different growls and the
same four words of caption.

## 6. Legal

Non negotiable, from the root `CLAUDE.md`. **No caption may transcribe or
paraphrase a real operator announcement.** A subtitle is not itself a recording
and reproduces nothing on its own, but a caption of an original in-fiction
announcement must be written in the project's own voice, from scratch. If a
line reads like something you have heard on a platform, it is wrong, whatever
the recording behind it says. The placeholders in section 4 are written to that
rule and the shipping script must be too.

**Owner decision, small.** Whether the caption line also carries a speaker
label for the announcements, for example `PLATFORM:` before the text. It helps
a player tell an announcement from a noise, and the square bracket convention
already does most of that job. Recommend no label: one line, no furniture.

## Accept

- The checkbox in `WBP_Settings` turns captions on and off with no restart, and
  the choice survives quitting the game.
- With subtitles off, nothing appears, and no widget is on screen at rest.
- A full train cycle at Canary Wharf: the inbound announcement, the arrival,
  the doors, the departure announcement and the departure each read, in order,
  and each clears itself.
- Let the train go: the heat caption reads, exactly once, and boarding a train
  produces no caption about heat at all.
- Round 1 to round 3: the round start and end captions read and do not stack.
- Round 10 with the brute pair, 24 alive: the caption line is still readable.
  If it flickers, raise `RepeatSuppressionSeconds`, do not remove the cue.
- Board the train: the caption from the old station does not appear at the new
  one. This is what the world subsystem lifetime buys and it is worth checking.
- Read a caption at arm's length on a 1080p screen without leaning in.
- All five CI gates pass.

## Not in G6

The announcement wording itself and the final caption script, both of which
wait on `phase-g1-audio.md`. Localisation: every string is an `FText` and every
row is a table row, so the work is done, but no culture beyond English is set
up. A caption history or log. Speaker labels, unless the owner decision in
section 6 goes the other way. Captioning anything the HUD already says.

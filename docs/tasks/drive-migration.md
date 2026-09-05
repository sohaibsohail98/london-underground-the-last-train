# Pending task: move the UE 5.8 engine to the external drive

Written 2026-09-05. Not started. Do this when NeoStack is idle and the editor is
fully closed, not mid run.

## Why

The internal SSD ran low on space (down to 11 GiB free on a 228 GiB disk),
which made NeoStack's PIE session recording fail with `LogAVCodecs: Error` spam
and risked a hard editor crash if the disk filled. On 2026-09-05 about 12 GiB
was reclaimed by deleting Docker container data, browser caches and Citro Labs,
bringing it to 23 GiB free. That is enough for now but not comfortable for a
long multi hour NeoStack run.

The permanent fix is to move the 43 GiB UE 5.8 engine install off the internal
SSD onto `/Volumes/DriveSohaib`, where Xcode already lives.

## The drive is suitable

- `/Volumes/DriveSohaib` is APFS, 3.6 TiB free. APFS supports symlinks, hard
  links and case sensitivity, which UE needs. A FAT or exFAT drive would not
  work.
- Mounted `noowners`, which is normal for an external APFS volume. Unix
  ownership is ignored there. Not a blocker for UE, but git may report spurious
  permission changes on files that live on it, so keep the project itself on the
  internal SSD (see below).

## What moves and what does not

| Item | Size | Action |
|---|---|---|
| UE 5.8 engine, `/Users/Shared/Epic Games/UE_5.8` | 43 GiB | **Move to the drive.** This is the whole win. |
| The project, `~/Projects/london-underground-the-last-train` | about 1 GiB | **Leave on the internal SSD.** Git and asset loading are faster on internal storage, and it is the thing you most want to survive a drive disconnect. |
| Xcode, `/Volumes/DriveSohaib/Applications/Xcode.app` | already on the drive | No change. |

## Procedure

1. Quit the Unreal Editor, the Epic Games Launcher, and any NeoStack session.
   Confirm nothing UE is running: `pgrep -fl UnrealEditor` returns nothing.
2. Copy, do not move, the engine to the drive. Copy first, verify, delete the
   original only after a test compile succeeds from the new location.
   ```
   rsync -a --info=progress2 "/Users/Shared/Epic Games/UE_5.8" "/Volumes/DriveSohaib/Epic Games/"
   ```
   Expect 30 to 60 minutes over USB.
3. Re-register the engine at its new path so the launcher and `.uproject` find
   it. Either:
   - Epic Games Launcher, Library, the engine slot, "Locate existing
     installation", point at `/Volumes/DriveSohaib/Epic Games/UE_5.8`, or
   - edit `~/Library/Application Support/Epic/UnrealEngine/Install.ini` and
     `~/Library/Application Support/Epic/UnrealEngineLauncher/LauncherInstalled.dat`
     to the new path.
4. Update the hardcoded build path everywhere it appears:
   - `CLAUDE.md`, the "Engine and build" section, the `Build.sh` command.
   - `docs/tasks/NEXT.md` and any handover doc that repeats it.
   - New path:
     `/Volumes/DriveSohaib/Epic Games/UE_5.8/Engine/Build/BatchFiles/Mac/Build.sh`
5. Test compile from the new location:
   ```
   "/Volumes/DriveSohaib/Epic Games/UE_5.8"/Engine/Build/BatchFiles/Mac/Build.sh LastTrainEditor Mac Development -Project="$PWD/LastTrain.uproject"
   ```
6. Open the editor, confirm the project loads and PIE runs.
7. Only then delete the original `/Users/Shared/Epic Games/UE_5.8`. That frees
   the 43 GiB.

## Consequence to accept

After this, the drive must be mounted to compile C++ or shaders **and** to open
the editor at all, not just for compilation. This is the same dependency Xcode
already imposes. If the drive is unplugged, nothing UE works. The project itself
still opens in an editor or IDE for reading and editing code, and git still
works, because the project stays on the internal SSD.

## Also worth doing while clearing space

- Docker Desktop is kept (needed for platform engineering work). Its 7.6 GiB
  container data was cleared on 2026-09-05 and will grow back as it is used.
  When space is tight again, `docker system prune -a --volumes` reclaims unused
  images and volumes without uninstalling anything.
- `brew cleanup -s` and `pip cache purge`, small but free.
- The UE engine DDC at
  `~/Library/Application Support/Epic/UnrealEngine/Common/DerivedDataCache`
  regenerates and can be cleared any time the editor is closed.

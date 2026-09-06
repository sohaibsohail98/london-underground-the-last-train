# Editor crash: SIGSEGV in EndPlayMap on PIE end

Written 2026-09-06. Investigation of a crash that has hit the editor at least
three times during NeoStack sessions.

## Signature

```
Assertion failed: false [File: PlayLevel.cpp] [Line: 553]
Object 'GameInstance /Engine/Transient.UnrealEdEngine_0:GameInstance_1' from PIE
level still referenced. Shortest path from root:
  (root) UnrealEdEngine /Engine/Transient.UnrealEdEngine_0
   -> UObject* UUnrealEdEngine::Trans = TransBuffer /Engine/Transient.TransBuffer_0
    -> TransBuffer::AddReferencedObjects( (Garbage) GameInstance ... )
       ^ This reference is preventing the old GameInstance from being GC'd ^

=== Critical error: ===
SIGSEGV: invalid attempt to access memory at address 0x3
  FDebug::CheckVerifyFailedImpl2
  UEditorEngine::EndPlayMap()
  UEditorEngine::Tick(float, bool)
  UUnrealEdEngine::Tick(float, bool)
  FEngineLoop::Tick()
```

## What is happening

On PIE end, `UEditorEngine::EndPlayMap()` garbage collects the PIE world and its
`GameInstance`. It cannot, because the editor's **transaction buffer**, the undo
history (`UUnrealEdEngine::Trans`, a `UTransBuffer`), still holds a reference to
an object that lives in the PIE world.

`EndPlayMap` hits an `ensure` (`PlayLevel.cpp:553`). The ensure handler then
runs `FReferenceChainSearch::FindAndPrintStaleReferencesToObjects` to log the
offending chain. That logging path dereferences a pointer that is already
garbage, `0x3`, and the process takes a `SIGSEGV`. So the visible crash is a
secondary failure inside the diagnostic for the real problem.

## Root cause

Something creates an **undo transaction on a PIE world object while PIE is
running**. PIE actors are not normally transacted. This happens when an
editor-context mutation is applied to a live PIE actor: spawning, deleting, or
`Modify()`-ing an actor, opening a level, or saving an asset, all while a Play
In Editor session is active.

In these sessions the correlation is exact: NeoStack `execute_script` calls that
mutate actors or assets, running during or immediately before a PIE session, and
sessions ended by closing the PIE window rather than stopping cleanly. An
earlier run logged an `execute_script` timeout at 60 seconds immediately before
one of these crashes.

## This is not a LAST TRAIN C++ bug

The game module does nothing with the transaction system, `Modify()`, or editor
transactions. `LTZombieCharacter`, `LTRoundManager`, `LTPlayerCharacter` and the
components cannot put a reference into `UUnrealEdEngine::Trans`. Only editor
code, or a tool driving editor code, can.

## Fixes applied

### Process rule, the real fix

Added to `docs/tasks/neostack-build.md` ground rules 10 and 11:

- Never mutate actors, assets or levels while PIE is running. Order of work is
  always: stop PIE cleanly, then mutate, then start PIE again to observe. During
  PIE, read only.
- Stop PIE with `playtest.stop` or Escape, never by closing the PIE window.

### Defensive C++, good hygiene regardless

`ALTRoundManager` gained an `EndPlay` override that drops every world reference
the actor holds before teardown: it calls `StopRounds()`, unbinds
`OnZombieDied` from every live zombie, then resets `LiveZombies`, `SpawnPoints`
and the `Heat` pointer. See the current `LTRoundManager.cpp` for the exact body.
`HandleZombieDied` now unbinds the delegate as well as removing the entry.

This drops every world reference the round manager holds before teardown. It
does not touch the transaction buffer, so it is not a direct fix for the crash,
but it removes our actors from the teardown reference graph and is correct on
its own merits: the `OnZombieDied` bindings and the `LiveZombies` array would
otherwise keep dead PIE actors alive through GC.

## If it recurs

- Confirm NeoStack is obeying ground rules 10 and 11. Check the run log for any
  `execute_script` mutation timestamped inside a `playtest` session window.
- As a manual mitigation, in the editor: Edit menu, or the console,
  `TRANSACTION RESET` clears the undo buffer. Doing this before ending PIE
  removes the stale reference. Not automatable from the game module.
- Longer term this is a candidate to raise with NeoStack: their `execute_script`
  should refuse or defer actor and asset mutations while PIE is active.

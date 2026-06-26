# Task 2 Report: Movement engine changes (src/act.movement.c)

## Status: COMPLETE — clean compile

## Changes made to `src/act.movement.c`

### 2a. Terrain bypass for AFF_FLYING players

**Water gate (lines ~178-183):** No change made. Confirmed `has_boat()` returns 1 when
`AFF_FLAGGED(ch, AFF_FLYING)` is true (lines 47-48 of the function). However, note that
the actual water gate check does NOT call `has_boat()` -- it only tests `!IS_NPC` and
`!PRF_NOHASSLE`. This means AFF_FLYING players would still be blocked at the water gate
by the current code. Left as-is per the explicit task brief instruction.

**Mountain gate (lines ~186-191):** Added `&& !AFF_FLAGGED(ch, AFF_FLYING)` to the inner
condition. Flying players now bypass the "terrain is too steep" block alongside NPCs and
NOHASSLE players.

Before:
```c
if (!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
```

After:
```c
if (!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_NOHASSLE) && !AFF_FLAGGED(ch, AFF_FLYING)) {
```

### 2b. Look suppression for PLR_AUTOFLIGHT players

The `look_at_room(ch, 0)` call after landing in the destination room (the one guarded by
`ch->desc != NULL`) now also checks `!PLR_FLAGGED(ch, PLR_AUTOFLIGHT)`. The second
`look_at_room` call in the greet-trigger-failed rollback path was left untouched -- that
one must still fire so the player sees the room they were rolled back to.

Before:
```c
if (ch->desc != NULL)
  look_at_room(ch, 0);
```

After:
```c
if (ch->desc != NULL && !PLR_FLAGGED(ch, PLR_AUTOFLIGHT))
  look_at_room(ch, 0);
```

### 2c. Manual movement rejection for PLR_AUTOFLIGHT players

Inserted an early-return guard in `perform_move()` after the basic direction/fight
validation and before the exit/door check chain. Players with PLR_AUTOFLIGHT receive
"You are flying -- you cannot change direction manually." and the call returns 0.

The original code was a single long `if (...) return; else if ... else if ... else { }`.
Breaking the chain at the PLR_AUTOFLIGHT guard required converting the remaining
`else if` start into a standalone `if` -- safe because both preceding paths (NULL/dir
check and AUTOFLIGHT check) return early.

```c
/* Block manual movement during auto-flight */
if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_AUTOFLIGHT)) {
  send_to_char(ch, "You are flying -- you cannot change direction manually.\r\n");
  return (0);
}
```

## Compile result

Clean compile, no warnings or errors.

```
gcc -w   -c -o act.movement.o act.movement.c
gcc -o ../bin/circle  act.comm.o ... act.movement.o ...   -lcrypt
make[1]: Leaving directory '.../src'
```

## Files changed

- `src/act.movement.c` -- all three changes (2a mountain gate, 2b look suppression, 2c manual-move lock)

# Task 6 Report: Quit/crash safety — clear flight flags on exit/login

## Status: DONE

## Compile
Clean. `make circle CFLAGS=-w` succeeded with no errors.

## What was changed

Single file edited: `src/players.c`

### 6a — save_char() (quit/linkdead path)

Added `int autoflight_was_set;` to the local variable declarations at the top of `save_char()`.

Just before the `PLR_FLAGS` / `AFF_FLAGS` bitvectors are serialised to disk (before the `sprintascii` calls for the `Act :` and `Aff :` lines), the function now:
1. Records whether `PLR_AUTOFLIGHT` is currently set.
2. If so, clears both `PLR_AUTOFLIGHT` and `AFF_FLYING` from the in-memory struct.

After `fclose(fl)` (but before spells and equipment are re-applied), if the flag was set it is restored to the in-memory struct.

**Why snapshot-and-restore instead of a permanent clear?** `save_char` is called from many sites besides quit/linkdead — periodic ticks (`limits.c`), item use, level-up, OLC, etc. Permanently stripping `AFF_FLYING` during a mid-session save would immediately ground a flying player. Restoring after the write means the file is clean while the live session is unaffected.

### 6b — load_char() (login path)

After `affect_total(ch)` (once all saved data is fully parsed and totalled), two `REMOVE_BIT_AR` calls clear `PLR_AUTOFLIGHT` and `AFF_FLYING`. This is a defensive cleanup: if a player file was written before this fix existed (or if a bug ever sets the flags again), login still starts fresh.

## Call-site rationale

`save_char()` / `load_char()` in `players.c` are the single choke points for all ASCII player-file I/O. Every quit and every linkdead extraction eventually calls `save_char`; every login calls `load_char`. This covers both paths with two small, local edits and no changes to the quit/extract/linkdead call chains.

## Files changed

- `src/players.c` — 6a and 6b fixes
- `changelog` — entry added

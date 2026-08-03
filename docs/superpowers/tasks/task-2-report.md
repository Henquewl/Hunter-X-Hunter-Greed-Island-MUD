# Task 2 Report: Modify MapArea visibility logic

**Goal:** Allow room mapping even when it's dark and the player lacks infravision.

**Changes:**
- Modified `MapArea` in `src/asciimap.c` to remove the forced setting of `SECT_DARK` when it is dark and the player lacks infravision.
- The `AFF_FLAGGED(ch, AFF_BLIND)` check remains to handle cases where the player is blind.
- This allows rooms to display their actual sector type even in darkness, provided the player is not blind. The circular vision restriction remains handled by the recursion depth.

**Verification:**
- Compiled the code successfully using `make circle CFLAGS=-w`.
- The logic change directly addresses the requirement to show actual sector types instead of `SECT_DARK` for non-blind players in dark rooms.

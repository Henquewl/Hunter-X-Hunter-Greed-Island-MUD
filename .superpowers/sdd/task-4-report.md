# Task 4 Report: Auto-flight event engine

## Status

DONE. Clean compile, no warnings.

## Files changed

- `src/mud_event.h` — added `eAUTOFLIGHT` to `event_id` enum; forward-declared `EVENTFUNC(event_autoflight)`
- `src/mud_event.c` — added `{ "AutoFlight", event_autoflight, EVENT_CHAR }` row to `mud_event_index[]` at position matching enum
- `src/act.movement.c` — added `#include "mud_event.h"`; added flight engine block (helpers, config table, structs, EVENTFUNC, start_flight)
- `src/dg_objcmd.c` — added `extern int start_flight(...)` declaration (ready for Task 5)

## Key design decision: flight_data storage

`new_mud_event()` calls `strdup(sVariables)` when sVariables is not NULL. Passing a `struct flight_data *` cast to `char *` through that path would corrupt the struct (strdup stops at the first null byte inside the struct).

Fix: call `new_mud_event(eAUTOFLIGHT, ch, NULL)` then set `pMudEvent->sVariables = (char *) data` directly, bypassing strdup. `free_mud_event` then calls `free(sVariables)` which correctly frees the flight_data struct when the event ends. This is the only deviation from the `NEW_EVENT` macro pattern — the reason is documented in comments.

## EVENTFUNC(event_autoflight) behaviour

- Retrieves `ch` from `pMudEvent->pStruct`, `data` from `pMudEvent->sVariables`.
- Safety-exits if `ch == NULL`, `IS_NPC(ch)`, `IN_ROOM(ch) == NOWHERE`, or `PLR_AUTOFLIGHT` no longer set.
- Each tick: steps up to 5 tiles via `flight_direction()` + `perform_move()`, emitting a trail act per step.
- Issues `look_at_room` after stepping.
- Reschedules at `1 * PASSES_PER_SEC` while en route; on arrival dispatches the `FLY_ARRIVE_*` mode block, clears both `PLR_AUTOFLIGHT` and `AFF_FLYING`, and returns 0 (event ends, `free_mud_event` frees data).

## Destination table

All `map_tile` vnums are `NOWHERE` (stubbed). Interior vnums set: antokiba=12064, masadora=3053; rabicuta and start/leave left NOWHERE until P1 city placement.

## Post-implementation fixes (commit 5b61868)

Two small correctness issues patched after initial Task 4 implementation:

1. **sVariables warning comment** — added a block comment directly above `pMudEvent->sVariables = (char *) data` in `start_flight()` warning that `change_event_duration()` must never be called on `eAUTOFLIGHT` events (it would strdup the struct pointer, truncating at the first null byte).

2. **AFF_FLYING cleared on safety exit** — the `if (!PLR_FLAGGED(ch, PLR_AUTOFLIGHT)) return 0;` early-exit in `event_autoflight` now also calls `REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_FLYING)` before returning, preventing a stuck flying state if `PLR_AUTOFLIGHT` is cleared externally (e.g. admin flag wipe) without the event running to its normal completion.

## For Task 5 implementer

- Call `start_flight(ch, dest->map_tile, dest->arrive_mode, 0, FALSE)` from `%ofly%` handler in `dg_objcmd.c`.
- `extern` declaration is already in place in `dg_objcmd.c`.
- `FLY_ARRIVE_*` constants and `find_fly_dest()` are file-static in `act.movement.c`; if Task 5 needs to look up destinations by name from outside that file, expose `find_fly_dest` or add a wrapper.
- `ELENA_ROOM_VNUM` (1406) is file-static — verify the vnum is correct before Task 5 wires up the Leave Point.

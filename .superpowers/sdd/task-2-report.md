# Task 2 Report: asciimap.c + utils.h — colors, legend, render override

## Status: DONE

## Changes Applied

### Change A — Invert Field/Forest colors
- `map_info[]`: SECT_FIELD `\tg` → `\tG`, SECT_FOREST `\tG` → `\tg`. Verified.
- `world_map_info[]`: SECT_FIELD `\tg` → `\tG`, SECT_FOREST `\tG` → `\tg`. Verified.

### Change B — Point sector glyphs (both arrays)
- `map_info[]`: PORT `\tD@` → `\tYP`, CITYENT `\tg@` → `\tYC`, MYSTERY `\ty|` → `\tg?`,
  START `""` → `\tc[\tYS\tc]\tn`, LEAVE `""` → `\tc[\tYL\tc]\tn`. Verified.
- `world_map_info[]`: PORT `\tD@` → `\tYP`, CITYENT `\tg@` → `\tYC`, MYSTERY `\ty|` → `\tg?`,
  START `""` → `\tYS`, LEAVE `""` → `\tYL`. Verified.

### Change C — perform_map() legend
- Worldmap block: SECT_CITY label `"City"` → `"Road"`. Verified.
- Worldmap block: Added Start and Leave entries after Entrance (19 → 21 entries; under 24 cap). Verified.
- Normal block: Added Start and Leave entries after Entrance (22 → 24 entries; at 24 cap). Verified.

### Change D — Remove dead ROOM_CRATER branch in MapArea
- Removed the `else if (ROOM_FLAGGED(room, ROOM_CRATER)) map[x][y] = SECT_PORT;` branch.
  ROOM_CRATER define in structs.h retained. Verified.

### Change E — IS_ENTRY_POINT_SECT macro (utils.h)
- Added after IS_CARD macro at line ~754 in utils.h:
  ```c
  #define IS_ENTRY_POINT_SECT(s) \
    ((s)==SECT_PORT||(s)==SECT_CITYENT||(s)==SECT_MYSTERY|| \
     (s)==SECT_START||(s)==SECT_LEAVE)
  ```
  Covers all five point sectors. Verified.

### Change F — Peaceful render override exemption
- Applied after Change D to the resulting single-line `ROOM_PEACEFUL` branch:
  `ROOM_FLAGGED(room, ROOM_PEACEFUL)` → `ROOM_FLAGGED(room, ROOM_PEACEFUL) && !IS_ENTRY_POINT_SECT(SECT(room))`.
  Verified.

## Compile Output

Full recompile (all .c files recompiled due to utils.h change), no errors, no warnings:

```
gcc -w   -c -o asciimap.o asciimap.c
...
gcc -o ../bin/circle  act.comm.o ... asciimap.o ... utils.o ...   -lcrypt
make[1]: Leaving directory '.../src'
```

Exit: success (no error messages, no warnings).

## Deviations

None. All changes match the brief verbatim. Legend entry count (24 max): worldmap=21, normal=24 — both within bounds.

## Files Changed

- `src/asciimap.c` — Changes A, B, C, D, F
- `src/utils.h` — Change E
- `changelog` — entry added
- `lib/text/news` — gameplay-facing entry added

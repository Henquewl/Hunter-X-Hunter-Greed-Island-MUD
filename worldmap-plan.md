# Plan: Greed Island World Map

**Research date:** Jun 23 2026
**Status:** Ready for future implementation

---

## Context

The v1.00 milestone requires the world to be reworked to canonically reflect Greed Island (Antokiba, Masadora, Rabicuta, Limeiro Castle, etc.), replacing the generic TbaMUD content. The centerpiece is a navigable island world map requiring hundreds of rooms connected in a grid — far beyond the conventional 100-rooms-per-zone limit.

---

## Design decision: 256x256 with 4 quadrants (inspired by FE MUD DragonBall Z)

The target model is the same as FE MUD: coordinates from 1,1 to 128,128 and from -128,-128 to -1,-1 (4 quadrants, no origin at 0,0). Total: 256x256 = **65,536 rooms**.

### Why this requires changing the vnum type

`room_vnum = ush_int` (unsigned short, 2 bytes, max 65,535). `NOWHERE = 65,535` is the reserved nil sentinel. Maximum usable vnums in the entire game: **65,534**. 256x256 = 65,536 — impossible without widening the type.

### Real cost of the IDXTYPE change

Audit performed on the codebase (`grep %hd %hu (ush_int)` in `src/`):
- **5 occurrences in 2 files**: `src/db.c` (4) and `src/shop.c` (1).
- Player files, object saves, and world files are **all ASCII format** — not affected by a binary type change.
- The `ush_int` typedef stays in the code for other structs; only `IDXTYPE` changes.

**Conclusion: the IDXTYPE change is the simplest possible prerequisite** — 5 lines to fix manually after editing `structs.h`.

---

## Research findings

### Zone system (no engine change required for zones themselves)

- The 100-rooms-per-zone limit is a **convention, not an engine rule**. The only validation in `db.c` is `bot > top`. A zone with 900 rooms (bot=42000, top=42899) loads without any code change.
- `real_room()` uses binary search on the `world[]` array — no arithmetic that assumes zone size.
- Vnum ceiling: **65,534** (65,535 = `NOWHERE` sentinel).
- **Confirmed by tbamud.com forum**: "you can create a zone of any size you want."

### Existing worldmap infrastructure (`src/asciimap.c`)

A full ASCII map rendering engine already exists with two modes:

| Mode | Tiles | Exit connectors | When active |
|---|---|---|---|
| Normal | `[X]` 3-char with color | Yes (`|`, `-`) | Default |
| Worldmap | 1-char bare (`·`, `~`, etc.) | No | XOR of flags (see below) |

**XOR logic in `show_worldmap()` (`asciimap.c:774`):**
- `ZONE_WORLDMAP` set, room WITHOUT individual `ROOM_WORLDMAP` → **worldmap mode** ✓
- `ZONE_WORLDMAP` set, room WITH individual `ROOM_WORLDMAP` → **normal mode** (useful for city entrances)
- Neither or both → normal mode

**Existing GI worldmap zones (do not touch):**
- Zone 400 (40000-40099): Greed Island Start — `ZONE_WORLDMAP`
- Zone 401 (40100-40199): Path to G.I. southern — `ZONE_WORLDMAP`
- Zone 402 (40200-40299): Road to Ai Ai — individual `ROOM_WORLDMAP` rooms
- Zone 410 (41000-41099): Dorias/Landing Platform — individual `ROOM_WORLDMAP` rooms

**Map canvas**: `MAX_MAP_SIZE = 12` (max visible radius), canvas `51x51`. On a 256x256 grid, a player at center sees up to 12 rooms in every direction.

### tbamud.com forum research

- [Luminari MUD](https://luminarimud.com) implemented a **1024x1024** wilderness using a dynamic vnum pool. Discarded for this project: dynamic pools cause persistence bugs (player disconnects and reconnects in the wrong room because the vnum was reassigned). Our island is static — no such problem.
- Building large grids manually with buildwalk/OLC is described as "tedious and error-prone". **Forum consensus: use a text file generation script** (.wld/.zon), not interactive OLC.

---

## Note on 255x255

255x255 = 65,025 rooms — still doesn't fit. The largest contiguous free vnum block in the current layout is ~24,000 (41100-65299). 65,025 doesn't fit in it. And even if all zones were reorganized: 12,713 (existing rooms) + 65,025 (255x255) = 77,738 vnums needed total, against a ceiling of 65,534. Mathematically impossible without the type change.

---

## Recommended approach: IDXTYPE -> uint32_t + Zone 1000 (256x256)

### Step 0 (prerequisite): Widen IDXTYPE to uint32_t

**Why it's necessary:** Any grid above ~155x155 requires a contiguous vnum block that doesn't exist in the current ush_int space.

**Why it's safe:**
- All world files, player files, and object saves use **ASCII format** (not 2-byte binary) — existing saves are not broken.
- The `ush_int` typedef stays in the code for other structs; only `IDXTYPE` changes.
- Only **5 occurrences** of `%hd`/`%hu`/`(ush_int)` in `src/db.c` (4) and `src/shop.c` (1) — the only lines to fix after the change.
- With `uint32_t`, `NOWHERE = 0xFFFFFFFF`. All existing logic continues to work.

**Change in `structs.h`** (1 line):
```c
// Before:
#define IDXTYPE  ush_int
// After:
#define IDXTYPE  uint32_t
```

Then compile and fix the 5 `%hd` -> `%u` occurrences.

### World zone specification

- **Zone 1000**, vnums **100000-165535** (65,536 rooms = 256x256)
- Zone flag: `g` (bit 6 = `ZONE_WORLDMAP`) in the .zon header line
- Vnum formula: `vnum = 100000 + (row * 256) + col`, where row in [0-255], col in [0-255]
- **Player-facing coordinates** (displayed in prompt/look, FE MUD style):
  - `x = col - 128` — range [-128, 127] (negative = west, positive = east)
  - `y = 128 - row` — range [-128, 127] (negative = south, positive = north)
  - Quadrant I: x in [1,128], y in [1,128] / Quadrant III: x in [-128,-1], y in [-128,-1]
- **Ocean border**: rows 0-1 and 254-255 + cols 0-1 and 254-255 -> `SECT_WATER_NOSWIM`, no exits outside the grid
- **City entrances**: use BOTH flags -> XOR turns off worldmap mode (shows as normal room), valid as teleport targets

### City locations on the grid (approximate — to be finalized in the layout)

| City | x,y (player) | row, col | Approx vnum |
|---|---|---|---|
| Antokiba (starting city) | 0, 0 | 128, 128 | 132,896 |
| Masadora (spell cards) | 50, 40 | 88, 178 | 122,706 |
| Rabicuta | -40, -30 | 158, 88 | 140,568 |
| Limeiro Castle | 0, 60 | 68, 128 | 117,504 |
| Dorias port | -10, -60 | 188, 118 | 148,086 |

*(exact coordinates to be determined during implementation based on the canonical island map)*

---

## Conflicts and how to avoid them

| Risk | Location | Resolution |
|---|---|---|
| Existing zones (0-654) | All | Zone 1000 starts at vnum 100,000 — above everything that exists. Zero overlap. |
| Card teleport (`act.item.c:1416`) | Scans city zone range, not zone 1000 | Grid rooms have no individual `ROOM_WORLDMAP` -> not teleport targets. City entrances have both flags -> automatically valid. No change needed. |
| XOR worldmap logic (`asciimap.c:774`) | Zones 400/401/402/410 | Those zones are untouched; zone 1000 is an independent new `ZONE_WORLDMAP`. |
| Zone 410, room 41001 exit | `lib/world/wld/410.wld` | **One exit to change**: `D1 (north)` changes from `40060` to the port grid room vnum (~148,086). Only edit to an existing file. |
| `%hd` format strings | `src/db.c` (4x), `src/shop.c` (1x) | Change to `%u` in 5 places after updating `IDXTYPE`. Trivial. |
| `city_met[8]` overflow (`structs.h:1018`) | `act.item.c:1487` | Do not flag GI city zones with `ZONE_CITY` yet — separate task. |
| `genzon.c` limit 655 | `genzon.c:67` | OLC will reject zone 1000 via interactive interface. Do not use OLC — edit text files directly only. |
| World file indexes | `lib/world/*/index` | Add `1000` entry in numeric order in all 7 indexes. |

---

## Files to create/modify

### Engine (minimal type change)
```
src/structs.h             <- 1 line: IDXTYPE ush_int -> uint32_t
src/db.c                  <- 4 occurrences of %hd -> %u (vnum printf)
src/shop.c                <- 1 occurrence of %hd -> %u (vnum printf)
```

### New (all created by generation script)
```
lib/world/zon/1000.zon    <- zone header: bot=100000, top=165535, flag=g (ZONE_WORLDMAP)
lib/world/wld/1000.wld    <- 65,536 rooms with N/S/E/W exits wired
lib/world/mob/1000.mob    <- empty ($)
lib/world/obj/1000.obj    <- empty ($)
lib/world/shp/1000.shp    <- empty ($~)
lib/world/qst/1000.qst    <- empty ($~)
lib/world/trg/1000.trg    <- empty ($~)
lib/world/map/greed_island.txt  <- canonical map source (256 lines x 256 chars)
```

### Manually edited (minimum)
```
lib/world/wld/410.wld     <- change 1 exit (room 41001 D1: ~148086)
lib/world/*/index         <- add "1000" to each of the 7 indexes
```

### Untouched
```
src/asciimap.c            <- do not touch (worldmap renderer already works)
src/act.item.c            <- do not touch (teleport works via flags)
```

---

## Player coordinate display

To reproduce the FE MUD experience (player sees "You are at (42, -15)" instead of "Room 135970"):

Add to `look` (in `act.informative.c`) a coordinate line when the player is in a zone with `ZONE_WORLDMAP`:

```c
if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(ch)), ZONE_WORLDMAP)) {
    room_vnum v = GET_ROOM_VNUM(IN_ROOM(ch));
    int col = (v - 100000) % 256;
    int row = (v - 100000) / 256;
    int x = col - 128;
    int y = 128 - row;
    send_to_char(ch, "[ Location: %d, %d ]\r\n", x, y);
}
```

This is the only optional engine addition — not required for the map to function, but improves UX.

---

## Generation script (`tools/gen_worldmap.py`)

The script reads `lib/world/map/greed_island.txt` (256 lines x 256 chars) and generates `1000.zon` and `1000.wld`. Requirements:

1. **Source**: `greed_island.txt` — 256 lines of 256 chars. Each char = sector:
   ```
   . = SECT_FIELD (2)      ^ = SECT_MOUNTAIN (5)    ~ = SECT_WATER_NOSWIM (7)
   f = SECT_FOREST (3)     h = SECT_HILLS (4)        = = SECT_WATER_SWIM (6)
   # = SECT_CITY (1)       @ = city entrance (generates ROOM_WORLDMAP flag)
   ```
2. **Vnum formula**: `vnum = 100000 + row * 256 + col`
3. **Exits**: each room connects N/S/E/W to adjacent rooms, except border (`~`) rooms have no external exits
4. **`@` rooms (city entrances)**: flags include `65536` (`ROOM_WORLDMAP`), plus an extra exit to the city zone vnum (configurable in the script)
5. **Zone header**: `100000 165535 30 2 g 0 0 0 -1 -1`
6. **Validation**: 65,536 rooms generated, no duplicate vnums, full `~` border, city exits pointing to valid vnums
7. **Performance**: the generated .wld will be ~6-8 MB — normal for this size

---

## Sector types (suggestions for the island layout)

| # | TbaMUD constant | Worldmap tile | Use |
|---|---|---|---|
| 0 | `SECT_INSIDE` | brown | interior caves |
| 1 | `SECT_CITY` | gray | city areas |
| 2 | `SECT_FIELD` | light green | open fields |
| 3 | `SECT_FOREST` | dark green | forests |
| 4 | `SECT_HILLS` | yellow | hills |
| 5 | `SECT_MOUNTAIN` | white | mountains |
| 6 | `SECT_WATER_SWIM` | light blue | rivers/lakes |
| 7 | `SECT_WATER_NOSWIM` | dark blue | ocean/border |

---

## Implementation order (for the future session)

### Phase 1 — Engine (prerequisite, ~30min)
1. `structs.h`: change `IDXTYPE` from `ush_int` to `uint32_t`
2. Fix 5 printf calls: `%hd` -> `%u` in `db.c` (4x) and `shop.c` (1x)
3. Compile: `cd src && make circle CFLAGS=-w` — must compile without errors
4. Quick boot and verify existing zones still work

### Phase 2 — Map layout (design, outside the code)
5. Draw `greed_island.txt` (256x256) based on the canonical island map
   - Use a plain text editor; each char = one tile
   - Place 5+ main cities (char `@`)
   - Border: first/last 2 rows and columns = `~`

### Phase 3 — Generation script
6. Write `tools/gen_worldmap.py` to read the .txt and generate `1000.zon` + `1000.wld`
7. Run it, validate output (65,536 rooms, correct exits)

### Phase 4 — Integration
8. Copy generated files to `lib/world/zon/` and `lib/world/wld/`
9. Create empty files: `1000.mob`, `1000.obj`, `1000.shp`, `1000.qst`, `1000.trg`
10. Add `1000` to all 7 `lib/world/*/index` files (in numeric order — at the end)
11. Edit `410.wld` room 41001: exit D1 -> port grid room vnum
12. Boot and test: `goto 132896` (Antokiba on the grid), run `map` and `map world`, walk to border, confirm ocean
13. Verify card teleport in zone 410 still lands on 41000 (not on the grid)

---

## References

- tbamud.com forum — [Virtual Wilderness, Room Pools and coordinate confusion](https://www.tbamud.com/forums/4-development/3601-virtual-wilderness-room-pools-and-coordinate-confusion)
- tbamud.com forum — [Creating large grid zones](https://www.tbamud.com/kunena/3-building/3984-creating-large-grid-zones)
- `src/asciimap.c` — existing renderer: `show_worldmap()` (XOR logic), `MapArea()`, `WorldMap()`
- `src/act.item.c:1416,1676` — card teleport logic (conflict reference)
- `src/structs.h:38-44` — `IDXTYPE`, `CIRCLE_UNSIGNED_INDEX`, `NOWHERE` definitions
- `src/db.c` and `src/shop.c` — only 5 occurrences of `%hd`/`%hu` to fix

---

> **Once this plan is fully implemented and tested, delete this file.**

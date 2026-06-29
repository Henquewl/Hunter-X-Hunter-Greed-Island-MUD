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

### Real cost of the IDXTYPE change (deep audit, Jun 23 2026)

A full audit revised the original "1 line + 5 cosmetic prints" estimate. The real picture:

**The actual blocker the first draft missed — a silent 99999 record cap (`db.c:1104`):**
```c
if (nr >= 99999)   /* inside discrete_load(), runs for WLD/MOB/OBJ/TRG/QST */
    return;
```
`discrete_load()` reads each `#NNNNN` record fine (`int` + `%d`), but the moment it sees a record number `>= 99999` it **returns and abandons the rest of the file, with no SYSERR**. Every world-map room (100000–165535) is `>= 99999`, so the first `#100000` makes the parser bail and **not a single grid room loads**. This must be raised (e.g. `if (nr >= 999999)` for headroom, or key it off a named max-vnum constant; note the boundary is `>=`, so the top vnum itself must be below the cap). **This is the single most important engine edit.** Also present in the offline tool `util/wld2html.c:269`.

**The 5 `%hd` occurrences are reads, not prints — and they are must-fix, not cosmetic.** They are `sscanf` calls writing into fields that become 32-bit:
- `db.c:2086, 2089, 2094` — `Z.bot`/`Z.top` (`room_vnum`). `%hd` truncates `100000→34464`, `165535→34463`, then the `bot > top` check (`db.c:2117`) **aborts boot**. Change first two `%hd`→`%d` on each line.
- `db.c:2063` — `Z.number` (`zone_vnum`). Zone 1000 fits in 16 bits so it loads today, but `%hd` into a 32-bit field leaves the upper 2 bytes uninitialized (UB). Fix anyway.
- `shop.c:1210` (via `read_line`/`sscanf "%hd"` at `shop.c:1057`) — `SHOP_KEEPER` (`mob_rnum`). Uninitialized-upper-bytes UB on **every shop load** after widening, regardless of vnum size. Fix to `%d`.

This is the **complete, confirmed-exhaustive** set of `%hd`/`%hu` in `src/*.c`/`*.h`. (Three `(sh_int)asciiflag_conv(...)` casts in `dg_*cmd.c` assign to door flags, **not vnums — leave alone**.)

**Save-file safety — the original "all ASCII" claim, corrected:**
- Main player files (`players.c`), player object/rent files (`objsave.c`, `fread_string`/tag-based), mail (`mail.c`): **ASCII — safe.** ✓
- The only binary structs holding `IDXTYPE` vnum fields are **house-related**: `house_control_rec` (`house.h:21-22`, `vnum`+`atrium`) and `obj_file_elem` (`structs.h:784`, `item_number`), written raw via `fwrite` (`house.c:222/688`). Widening changes their byte layout → pre-existing house binary files would be misread.
- **Resolved:** the housing system is **not used** in this MUD (confirmed by the project owner). There is no house data to corrupt, and the type change is self-consistent (writes/reads use the same new size). So in practice **no existing save breaks.** Housing is independent of the world-map work and is **not** being removed (see note at end).
- `last_entry` (`handler.h:120`) and `board_msginfo` (`boards.h:24`) are binary but contain **no vnum fields — safe.** ✓

**Conclusion:** the prerequisite is **~6 engine edits** (`structs.h` typedef + the 99999 cap + 5 `%hd` reads), not a one-liner — but every one is small and surgical, and nothing in the live save format breaks on this server.

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

### Step 0 (prerequisite): Widen IDXTYPE + lift the 99999 cap + fix the 5 `%hd` reads

**Why it's necessary:** Two independent ceilings sit below our target. The `ush_int` type maxes at 65,535, and `discrete_load()` separately refuses any record `>= 99999`. A 256x256 grid (65,536 rooms) needs a contiguous block, and the only space that big lives above both ceilings — so **both** must be lifted.

**The complete engine change set (see "Real cost" above for full detail):**

1. **`structs.h`** — widen the index type (1 line):
   ```c
   // Before:  #define IDXTYPE  ush_int
   // After:   #define IDXTYPE  uint32_t
   ```
   `NOWHERE`/`NOTHING`/`NOBODY` become `0xFFFFFFFF`; harmless since the max real vnum is 165,535. The `ush_int` typedef itself stays for non-index uses.
2. **`db.c:1104`** — raise the `if (nr >= 99999) return;` cap (the silent blocker; without it no grid room loads).
3. **`db.c:2086, 2089, 2094`** — `Z.bot`/`Z.top`: first two `%hd` -> `%d` on each line (else boot aborts on truncation).
4. **`db.c:2063`** — `Z.number`: `%hd` -> `%d` (latent UB).
5. **`shop.c:1210`** (reads via `read_line`, `shop.c:1057`) — `SHOP_KEEPER`: `%hd` -> `%d` (UB on every shop load).

**Verified safe — no change needed** (audited Jun 23 2026):
- Boot has **no zone-number cap** (`load_zones` only checks `bot > top`) — zone **1000 loads fine**. The `655` cap in `genzon.c:59` is **OLC-only**; it blocks editing zone 1000 in `zedit`, not booting a hand-written file.
- Exit-target parsing (`setup_dir`, `int t[5]`+`%d`), zone reset commands (`reset_com.arg*` are `int`+`%d`), and `real_room`/`real_zone` binary searches all widen cleanly — 100000+ vnums handled.
- No runtime struct stores a room/obj/mob vnum in a fixed narrow type; all use `IDXTYPE` (widens) or `int` (already wide). `GET_LOADROOM`, recall/goto/teleport, autoquest, shops, guilds all verified.
- Card-teleport `*100` loops (`act.item.c`) are numerically safe at zone 1000. (They only ever scan a zone's first 100 rooms — irrelevant since zone 1000 is **not** `ZONE_CITY`. Keep it that way; see conflicts table.)

**Minor / conditional:**
- Door-key sentinel `t[1]==65535 -> NOTHING` (`db.c:1363`) is a magic number; harmless for the grid (no key 65535 used), but note it no longer equals the new `NOTHING` (`0xFFFFFFFF`).
- Offline tool `util/wld2html.c` has BOTH the `99999` cap (line 269) and `typedef sh_int room_num/obj_num` (lines 48-49) — fix only if you run it on the new world. Not linked into `bin/circle`.
- `genzon.c:59 max_zone` — raise only if you later want to edit zone 1000 in `zedit`.

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
| **99999 record cap (SILENT — no grid room loads)** | `src/db.c:1104` | **Must raise** the `if (nr >= 99999) return;` in `discrete_load()`. Without this, the first `#100000` aborts the file with no error. The #1 critical edit. Also in `util/wld2html.c:269` (offline tool). |
| `%hd` reads truncating zone bot/top (BOOT ABORT) | `src/db.c:2086, 2089, 2094` | `%hd`->`%d` for `Z.bot`/`Z.top`. Without it, `100000`->`34464` triggers the `bot>top` abort at `db.c:2117`. |
| `%hd` reads → uninitialized upper bytes (UB) | `src/db.c:2063` (`Z.number`), `src/shop.c:1210` (`SHOP_KEEPER`) | `%hd`->`%d`. UB on every zone/shop load once IDXTYPE is 32-bit, regardless of vnum size. |
| Existing zones (0-654) | All | Zone 1000 starts at vnum 100,000 — above everything that exists. Zero overlap. Boot has **no zone-number cap** (verified). |
| Card teleport `*100` loops | `act.item.c` (Return/Drift/Accompany) | Numerically safe at zone 1000. Keep zone 1000 **out of `ZONE_CITY`** — the `*100` scheme only scans a zone's first 100 rooms, so a worldmap flagged `ZONE_CITY` would land teleports only in its top-left corner. Worldmaps shouldn't be `ZONE_CITY` anyway. |
| XOR worldmap logic (`asciimap.c:774`) | Zones 400/401/402/410 | Those zones are untouched; zone 1000 is an independent new `ZONE_WORLDMAP`. |
| Zone 410, room 41001 exit | `lib/world/wld/410.wld` | **One exit to change**: `D1 (north)` changes from `40060` to the port grid room vnum (~148,086). Only edit to an existing file. |
| House binary files (`house_control_rec`, `obj_file_elem`) | `house.c:222/688` | Contain `IDXTYPE` vnum fields → layout shifts on widening. **Moot: housing is unused** (no data to corrupt; format is self-consistent going forward). Not removed (see end note). |
| `city_met[8]` overflow (`structs.h:1018`) | `act.item.c:1487` | Do not flag GI city zones with `ZONE_CITY` yet — separate task. |
| `genzon.c` limit 655 | `genzon.c:59` | OLC-only — blocks editing zone 1000 in `zedit`, NOT booting it. Hand-write the zone file. Raise `max_zone` only if you later want OLC on it. |
| World file indexes | `lib/world/*/index` | Add `1000` entry in numeric order in all 7 indexes. |

---

## Files to create/modify

### Engine (~6 surgical edits — see "Step 0" for detail)
```
src/structs.h   <- 1 line: IDXTYPE ush_int -> uint32_t
src/db.c:1104   <- raise the `if (nr >= 99999) return;` cap   [CRITICAL — silent blocker]
src/db.c:2086,2089,2094  <- Z.bot/Z.top: %hd -> %d  [else boot aborts]
src/db.c:2063   <- Z.number: %hd -> %d
src/shop.c:1210 <- SHOP_KEEPER read (shop.c:1057): %hd -> %d
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
2. `db.c:1104`: raise the `if (nr >= 99999) return;` cap **(critical — without this no grid room loads, silently)**
3. `db.c:2086,2089,2094`: `Z.bot`/`Z.top` `%hd`->`%d`; `db.c:2063`: `Z.number` `%hd`->`%d`; `shop.c:1210`: `SHOP_KEEPER` `%hd`->`%d`
4. Compile: `cd src && make circle CFLAGS=-w` — must compile without errors
5. Boot and verify existing zones still work (load a player, `goto` a few zones, enter a shop, check zone resets) — this proves the widening + `%hd` fixes didn't regress anything **before** building the 65k-room zone
6. Sanity check: `grep -rn 99999 src/` to confirm no other hidden record cap besides `db.c:1104` (and `util/wld2html.c:269`, offline)

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
- `src/db.c:1104` — the `99999` record cap; `db.c:2063/2086/2089/2094`, `shop.c:1057/1210` — the `%hd` reads
- `src/house.h:20`, `src/structs.h:782` — binary structs with vnum fields (moot, housing unused)

---

## Note: housing system

The housing system is **not used** in this MUD. It surfaced in the audit only because its binary save structs (`house_control_rec`, `obj_file_elem`) contain `IDXTYPE` vnum fields whose layout shifts when the type widens — but with no house data on disk, nothing breaks, and housing is fully independent of the world-map work.

**Recommendation: do not remove it as part of this work.** It is not in the way. A clean removal would touch ~10 files (`house.c`/`house.h` + call sites in `comm.c` boot/save, `db.c`, `interpreter.c` commands, `objsave.c`/`handler.c` hooks, `constants.c`, `config.c`, `Makefile`) — pure churn with no benefit to the map and real regression risk. If desired purely for cleanliness, do it as a separate, deliberate task with its own compile/test cycle.

---

> **Once this plan is fully implemented and tested, delete this file.**

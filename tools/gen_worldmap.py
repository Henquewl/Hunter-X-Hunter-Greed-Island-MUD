#!/usr/bin/env python3
"""
gen_worldmap.py — Generate zone 1000 world files from a 256x256 island map.

Usage:
    python tools/gen_worldmap.py [--dry-run] [--validate-only]

Reads:   lib/world/map/greed_island.txt  (256 lines × 256 chars)
Writes:  lib/world/wld/1000.wld
         lib/world/zon/1000.zon

Sector legend (one char per tile):
    .  FIELD (2)          ^ MOUNTAIN (5)
    f  FOREST (3)         ~ WATER_NOSWIM (7) — ocean/border
    h  HILLS (4)          = WATER_SWIM (6)   — rivers/lakes
    S  START (13)         — Start Point
    L  LEAVE (14)         — Leave Point
    P  PORT (10)          — Port
    C  CITYENT (11)       — City
    ?  MYSTERY (12)       — Mysterious Entrance

Access point links (L/P/C/? tile → destination room vnum):
    Edit ENTRY_LINKS below. Key = (row, col) of the tile on the map,
    value = dest_vnum of the room to warp to.
    NOTE: S (Start Point) does NOT need an entry — it is a landing zone only
    (players arrive here; `enter` is not available on S tiles).

Vnum formula:  vnum = 100000 + row*256 + col   (row,col ∈ [0,255])
Coordinates:   x = col - 128  (negative=west, positive=east)
               y = 128 - row  (negative=south, positive=north)

Requirements:
    - Exactly 256 lines, each exactly 256 chars (no trailing spaces needed, but line length
      must be >= 256; extra chars are ignored).
    - First/last 2 rows and cols must be ~ (ocean border).
    - Every char must be in the legend.
    - Each L/P/C/? tile must have a matching entry in ENTRY_LINKS (S is exempt).

Run --validate-only to check the source map without writing any files.
"""

import argparse
import os
import sys

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.join(os.path.dirname(__file__), "..")
MAP_SRC   = os.path.join(REPO_ROOT, "lib", "world", "map", "greed_island.txt")
WLD_OUT   = os.path.join(REPO_ROOT, "lib", "world", "wld", "1000.wld")
ZON_OUT   = os.path.join(REPO_ROOT, "lib", "world", "zon", "1000.zon")

GRID = 256          # side length of the grid
BASE_VNUM = 100000  # vnum of room at row=0, col=0

# Zone 1000 header fields
ZONE_BOT      = BASE_VNUM
ZONE_TOP      = BASE_VNUM + GRID * GRID - 1   # 165535
ZONE_LIFESPAN = 30
ZONE_RESET    = 2   # reset only if no players present
ZONE_FLAGS    = "g" # ZONE_WORLDMAP

ROOM_PEACEFUL_FLAG = 16    # ROOM_PEACEFUL bit (index 4 → 1<<4)

# Sector type numbers
SECT = {
    '.': 2,   # SECT_FIELD
    'f': 3,   # SECT_FOREST
    'h': 4,   # SECT_HILLS
    '^': 5,   # SECT_MOUNTAIN
    '=': 6,   # SECT_WATER_SWIM
    '~': 7,   # SECT_WATER_NOSWIM
    'P': 10,  # SECT_PORT
    'C': 11,  # SECT_CITYENT
    '?': 12,  # SECT_MYSTERY
    'S': 13,  # SECT_START
    'L': 14,  # SECT_LEAVE
    'b': 15,  # SECT_BEACH
    'r': 1,   # SECT_CITY  (Road)
}

# Sector names for room descriptions
SECT_NAME = {
    '.': "Open Field",
    'f': "Forest",
    'h': "Rolling Hills",
    '^': "Mountain",
    '=': "River",
    '~': "Ocean",
    'P': "Port",
    'C': "City",
    '?': "Mysterious Entrance",
    'S': "Start Point",
    'L': "Leave Point",
    'b': "Beach",
    'r': "Road",
}

# Direction indices used in TbaMUD world files
# D0=north D1=east D2=south D3=west  (TbaMUD uses N=0 S=2 E=1 W=3)
DIR_NORTH = 0
DIR_EAST  = 1
DIR_SOUTH = 2
DIR_WEST  = 3

# row/col deltas for each direction
DELTA = {
    DIR_NORTH: (-1,  0),
    DIR_EAST:  ( 0, +1),
    DIR_SOUTH: (+1,  0),
    DIR_WEST:  ( 0, -1),
}

# ---------------------------------------------------------------------------
# Access point entry links
# Edit this table: key = (row, col) of the point tile on the map,
#                  value = dest_vnum (the room to warp to on `enter`).
# Every S/L/P/C/? tile in greed_island.txt MUST have an entry here.
# Example (uncomment and fill in real values):
#   (128, 128): 41001,   # Start Point at Antokiba → zone 400 entry room
# ---------------------------------------------------------------------------
ENTRY_LINKS = {
    # (row, col): dest_vnum,
}

POINT_CHARS = set("SLPC?")   # all access-point chars (rendered as special glyphs)
PORTAL_CHARS = set("LPC?")   # subset that emit a D5 portal exit and require ENTRY_LINKS
                              # S is excluded: it is a landing zone only (no `enter`)

# Keyword emitted on the D5 portal exit for each portal type.
# Lets `enter <keyword>` work via the existing do_enter keyword loop.
POINT_KEYWORD = {
    'L': 'leave',
    'P': 'port',
    'C': 'city',
    '?': 'entrance',
}

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def vnum(row, col):
    return BASE_VNUM + row * GRID + col

def coord_display(row, col):
    x = col - 128
    y = 128 - row
    return x, y

def room_name(tile, row, col):
    x, y = coord_display(row, col)
    return "{} ({}, {})".format(SECT_NAME[tile], x, y)

def room_desc(tile, row, col):
    x, y = coord_display(row, col)
    if tile == '~':
        return "   Vast ocean surrounds you in every direction.\n"
    elif tile == '.':
        return "   Open grassland stretches out in all directions.\n"
    elif tile == 'f':
        return "   Dense forest closes in around you.\n"
    elif tile == 'h':
        return "   Rolling hills rise and fall across the landscape.\n"
    elif tile == '^':
        return "   Steep mountain terrain makes travel difficult.\n"
    elif tile == '=':
        return "   A river flows quietly past.\n"
    elif tile == 'S':
        return "   A magical starting point shimmers here.\n"
    elif tile == 'L':
        return "   This is where Greed Island meets the outside world.\n"
    elif tile == 'P':
        return "   The port bustles with activity.\n"
    elif tile == 'C':
        return "   The lights of a nearby city are visible.\n"
    elif tile == '?':
        return "   A strange mysterious entrance lies before you.\n"
    elif tile == 'b':
        return "   Sandy beach stretches along the shore.\n"
    elif tile == 'r':
        return "   A dusty road winds through the landscape.\n"
    return "   Greed Island stretches around you.\n"

def is_border(row, col):
    return row < 2 or row >= GRID - 2 or col < 2 or col >= GRID - 2

# ---------------------------------------------------------------------------
# Map loading and validation
# ---------------------------------------------------------------------------

def load_map(path):
    """Load and validate the source map. Returns list of 256 strings (256 chars each)."""
    errors = []
    if not os.path.exists(path):
        print("ERROR: map source not found: {}".format(path), file=sys.stderr)
        print("       Create lib/world/map/greed_island.txt (256 lines × 256 chars).", file=sys.stderr)
        sys.exit(1)

    with open(path, "r", encoding="ascii", errors="replace") as f:
        raw = f.readlines()

    if len(raw) != GRID:
        errors.append("Expected {} lines, got {}".format(GRID, len(raw)))

    valid_chars = set(SECT.keys())
    rows = []
    point_tiles = []
    for i, line in enumerate(raw):
        line = line.rstrip("\n\r")
        if len(line) < GRID:
            errors.append("Row {} too short: {} chars (need {})".format(i, len(line), GRID))
            line = line.ljust(GRID, '~')
        line = line[:GRID]  # ignore extra chars

        bad = set(line) - valid_chars
        if bad:
            errors.append("Row {} contains unknown chars: {}".format(i, sorted(bad)))

        rows.append(line)

        for j, ch in enumerate(line):
            if ch in POINT_CHARS:
                point_tiles.append((i, j))

    if len(raw) == GRID:
        # Check ocean border: first/last 2 rows
        for r in list(range(2)) + list(range(GRID - 2, GRID)):
            row_str = rows[r] if r < len(rows) else ""
            non_ocean = [c for c in row_str if c != '~']
            if non_ocean:
                errors.append("Border row {} must be all '~', found: {}".format(r, non_ocean[:5]))
        # Check ocean border: first/last 2 cols
        for r, row_str in enumerate(rows):
            for c in list(range(2)) + list(range(GRID - 2, GRID)):
                if c < len(row_str) and row_str[c] != '~':
                    errors.append("Border col {} row {} must be '~', got '{}'".format(c, r, row_str[c]))

    # Verify that every portal tile (L/P/C/?) has an ENTRY_LINKS entry.
    # S tiles are exempt — they are landing zones with no D5 exit.
    missing_links = []
    for rc in point_tiles:
        ch = rows[rc[0]][rc[1]]
        if ch in PORTAL_CHARS and rc not in ENTRY_LINKS:
            missing_links.append("  ({}, {}) char={} x={} y={}".format(
                rc[0], rc[1], ch, rc[1]-128, 128-rc[0]))
    if missing_links:
        errors.append(
            "Portal tiles with no entry in ENTRY_LINKS (edit gen_worldmap.py):\n" +
            "\n".join(missing_links))

    if errors:
        print("Map validation FAILED ({} error(s)):".format(len(errors)), file=sys.stderr)
        for e in errors:
            print("  - " + e, file=sys.stderr)
        sys.exit(1)

    print("Map loaded: {} rooms, {} access points".format(
        GRID * GRID, len(point_tiles)))
    return rows

# ---------------------------------------------------------------------------
# World file generation
# ---------------------------------------------------------------------------

def write_zon(path, dry_run):
    """Write the zone header file for zone 1000."""
    lines = []
    lines.append("#1000")
    lines.append("Henque~")
    lines.append("Greed Island~")
    lines.append("{} {} {} {} {} 0 0 0 -1 -1".format(
        ZONE_BOT, ZONE_TOP, ZONE_LIFESPAN, ZONE_RESET, ZONE_FLAGS))
    lines.append("S")
    lines.append("$~")
    content = "\n".join(lines) + "\n"

    if dry_run:
        print("[DRY RUN] Would write {} ({} bytes)".format(path, len(content)))
        return
    with open(path, "w", encoding="ascii") as f:
        f.write(content)
    print("Wrote: {}".format(path))


def write_wld(path, rows, dry_run):
    """Write the 65,536-room world file for zone 1000."""
    buf = []
    count = 0

    for row in range(GRID):
        for col in range(GRID):
            tile  = rows[row][col]
            v     = vnum(row, col)
            sect  = SECT[tile]

            # ROOM_PEACEFUL (16) for safe points; 0 for mystery and terrain
            if tile in ('S', 'L', 'P', 'C'):
                flags = ROOM_PEACEFUL_FLAG
            else:
                flags = 0

            buf.append("#{}".format(v))
            buf.append("{}~".format(room_name(tile, row, col)))
            buf.append(room_desc(tile, row, col).rstrip("\n"))
            buf.append("~")
            # Room flag line: <ignored> <flags0> <flags1> <flags2> <flags3> <sector>
            # (parse_room reads 4 flag bitvectors then the sector type; field 1 is ignored)
            buf.append("1000 {} 0 0 0 {}".format(flags, sect))

            # Exits: only wire exits that stay on the grid and don't leave ocean border
            for direction, (dr, dc) in DELTA.items():
                nr, nc = row + dr, col + dc
                if 0 <= nr < GRID and 0 <= nc < GRID:
                    # Ocean border tiles have no exits out of the grid (already handled by
                    # the bounds check above), but also don't get exits INTO the ocean from
                    # the second ring — let the worldmap logic handle that naturally via
                    # sector type. Only suppress exits that would go OFF the array.
                    buf.append("D{}".format(direction))
                    buf.append("~")
                    buf.append("~")
                    buf.append("0 0 {}".format(vnum(nr, nc)))

            # D5 portal exit for portal tiles (L/P/C/?) — not S (landing zone only)
            if tile in PORTAL_CHARS:
                dest_vnum = ENTRY_LINKS[(row, col)]
                buf.append("D5")
                buf.append(POINT_KEYWORD[tile] + "~")  # keyword for `enter <keyword>`
                buf.append("~")
                buf.append("0 0 {}".format(dest_vnum))

            buf.append("S")
            count += 1

    buf.append("$~")
    content = "\n".join(buf) + "\n"

    if dry_run:
        print("[DRY RUN] Would write {} ({} rooms, ~{:.1f} MB)".format(
            path, count, len(content) / 1_000_000))
        return

    with open(path, "w", encoding="ascii") as f:
        f.write(content)
    print("Wrote: {} ({} rooms, {:.1f} MB)".format(path, count, len(content) / 1_000_000))

# ---------------------------------------------------------------------------
# Validation of generated content
# ---------------------------------------------------------------------------

def validate_output(rows):
    """Quick sanity checks after generation."""
    errors = []
    seen = set()
    for row in range(GRID):
        for col in range(GRID):
            v = vnum(row, col)
            if v in seen:
                errors.append("Duplicate vnum {}".format(v))
            seen.add(v)

    if len(seen) != GRID * GRID:
        errors.append("Expected {} rooms, got {}".format(GRID * GRID, len(seen)))

    top = BASE_VNUM + GRID * GRID - 1
    if top != ZONE_TOP:
        errors.append("Top vnum {} != ZONE_TOP {}".format(top, ZONE_TOP))

    if errors:
        print("Post-generation validation FAILED:")
        for e in errors:
            print("  - " + e)
        sys.exit(1)

    print("Validation OK: {} unique rooms, vnum range {}-{}".format(
        len(seen), BASE_VNUM, ZONE_TOP))

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Generate zone 1000 world files from greed_island.txt")
    parser.add_argument("--dry-run", action="store_true",
        help="Parse and validate without writing output files")
    parser.add_argument("--validate-only", action="store_true",
        help="Validate map source only, do not generate")
    args = parser.parse_args()

    dry_run = args.dry_run or args.validate_only

    print("Reading map: {}".format(MAP_SRC))
    rows = load_map(MAP_SRC)

    validate_output(rows)

    if args.validate_only:
        print("Validation complete. No files written (--validate-only).")
        return

    # Ensure output dirs exist
    for d in [os.path.dirname(WLD_OUT), os.path.dirname(ZON_OUT)]:
        if not dry_run:
            os.makedirs(d, exist_ok=True)

    write_zon(ZON_OUT, dry_run)
    write_wld(WLD_OUT, rows, dry_run)

    if not dry_run:
        print()
        print("Done. Next steps:")
        print("  1. Add '1000.wld' to lib/world/wld/index  (before the $ line) — if not already there")
        print("  2. Add '1000.zon' to lib/world/zon/index  (before the $ line) — if not already there")
        print("  3. Populate ENTRY_LINKS in tools/gen_worldmap.py with point tile coordinates and dest vnums")
        print("  4. Place point chars (S/L/P/C/?) in lib/world/map/greed_island.txt")
        print("  5. Boot: bin/circle 4000")


if __name__ == "__main__":
    main()

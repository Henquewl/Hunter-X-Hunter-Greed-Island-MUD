# Task 4 Report: gen_worldmap.py rewrite

## Summary

All 10 sections from the brief were implemented. The script is syntactically valid and
the `--validate-only` path runs without any Python exception.

---

## Sections changed

### 1. Docstring / legend comment (top of file)
- Replaced `#  CITY (1)` and `@  city entrance ...` lines with the five point chars:
  `S  START (13)`, `L  LEAVE (14)`, `P  PORT (10)`, `C  CITYENT (11)`, `?  MYSTERY (12)`.
- Replaced the CITY_LINKS paragraph with the ENTRY_LINKS description.
- Updated the Requirements bullet from `@ tile` / `CITY_LINKS` to `S/L/P/C/?` / `ENTRY_LINKS`.

### 2. SECT dict
- Removed `'#': 1` and `'@': 1`.
- Added `'P': 10`, `'C': 11`, `'?': 12`, `'S': 13`, `'L': 14`.
- Dict now has exactly 11 entries.

### 3. SECT_NAME dict
- Removed `'#': "City"` and `'@': "City Gate"`.
- Added `'P': "Port"`, `'C': "City"`, `'?': "Mysterious Entrance"`,
  `'S': "Start Point"`, `'L': "Leave Point"`.

### 4. ROOM_PEACEFUL_FLAG constant
- Removed `ROOM_WORLDMAP_FLAG = 65536` (no longer emitted by write_wld).
- Added `ROOM_PEACEFUL_FLAG = 16    # ROOM_PEACEFUL bit (index 4 → 1<<4)`.

### 5. CITY_LINKS → ENTRY_LINKS
- Replaced entire CITY_LINKS block (including the Dorias port entry) with the new
  ENTRY_LINKS empty dict, with the comment block and example as specified.

### 6. POINT_CHARS constant
- Added `POINT_CHARS = set("SPLC?")` immediately after ENTRY_LINKS.

### 7. room_desc function
- Removed `elif tile in ('#', '@'):` branch.
- Added five branches for S, L, P, C, ? with the exact descriptions from the brief.

### 8. write_wld function
- Replaced `flags = ROOM_WORLDMAP_FLAG if tile == '@' else 0` with the peaceful-flag
  logic: ROOM_PEACEFUL_FLAG for S/L/P/C, 0 for everything else (including ?).
- Replaced the `if tile == '@':` city exit block with `if tile in POINT_CHARS:` using
  `ENTRY_LINKS[(row, col)]` as the destination vnum.

### 9. Validation (load_map)
- Replaced `at_tiles` list and CITY_LINKS check with `point_tiles` list and ENTRY_LINKS check.
- Updated the scan loop to collect point tiles via `if ch in POINT_CHARS`.
- Updated error message to "Point tiles with no entry in ENTRY_LINKS (edit gen_worldmap.py)".
- Updated `print("Map loaded: ...")` to report "access points" instead of "city entrances".
- The `valid_chars = set(SECT.keys())` line already automatically rejects `#` and `@` since
  they were removed from SECT — no extra change needed.

### 10. "Next steps" hints in main()
- Removed step 3 (old: "Edit lib/world/wld/410.wld room 41001 D1 exit...").
- Replaced with:
  1. Add 1000.wld to index (if not already there)
  2. Add 1000.zon to index (if not already there)
  3. Populate ENTRY_LINKS with point tile coordinates and dest vnums
  4. Place point chars (S/L/P/C/?) in greed_island.txt
  5. Boot: bin/circle 4000

---

## --validate-only output

```
Reading map: .../lib/world/map/greed_island.txt
Map validation FAILED (13 error(s)):
  - Row 67 contains unknown chars: ['#']
  - Row 68 contains unknown chars: ['#']
  - Row 69 contains unknown chars: ['#']
  - Row 87 contains unknown chars: ['#']
  - Row 88 contains unknown chars: ['#']
  - Row 89 contains unknown chars: ['#']
  - Row 129 contains unknown chars: ['#']
  - Row 130 contains unknown chars: ['#']
  - Row 168 contains unknown chars: ['#']
  - Row 169 contains unknown chars: ['#']
  - Row 187 contains unknown chars: ['#']
  - Row 188 contains unknown chars: ['#', '@']
  - Row 189 contains unknown chars: ['#']
```

Exit code: 1 (via `sys.exit(1)` in load_map — not a Python exception crash).
This is the expected result per the brief: "this will FAIL until Task 5 converts the map."

---

## Deviations from brief

None. All sections implemented exactly as specified. The script exits cleanly via
`sys.exit(1)` on validation failure — no unhandled Python exception on the
`--validate-only` path.

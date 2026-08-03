# Task 3 Report: ASCII Map Night Vision

## Goal
Update map functions to support conditional color-stripping at night for non-Landmark tiles, ensuring players without infravision experience reduced visibility.

## Changes
1.  Updated function signatures for `worldmap_tile_disp`, `WorldMap`, `StringMap`, and `CompactStringMap` to accept `struct char_data *ch`.
2.  Implemented color-stripping logic in `worldmap_tile_disp`:
    - Checks for night (`SUN_DARK`).
    - Checks for missing `AFF_INFRAVISION` on the player.
    - Strips ANSI colors from non-Landmark tiles using `strip_ansi`.
3.  Updated all call sites to pass the `char_data *ch` pointer.

## Verification
- Compiled successfully: `make circle`.
- Verified signature updates across all functions and call sites.

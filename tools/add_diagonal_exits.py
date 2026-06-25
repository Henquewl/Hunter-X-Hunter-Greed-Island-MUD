#!/usr/bin/env python3
"""Add diagonal exits (D6=NW, D7=NE, D8=SE, D9=SW) to lib/world/wld/1000.wld.

The worldmap is a 256x256 grid. Room VNUM = 100000 + row*256 + col.
Diagonal neighbor VNUMs:
  NW (D6): row-1, col-1
  NE (D7): row-1, col+1
  SE (D8): row+1, col+1
  SW (D9): row+1, col-1
"""

import sys
import os

BASE = 100000
COLS = 256
ROWS = 256

WLD_FILE = os.path.join(os.path.dirname(__file__), '..', 'lib', 'world', 'wld', '1000.wld')


def diag_exit(direction_code, to_vnum):
    return f"D{direction_code}\n~\n~\n0 -1 {to_vnum}\n"


def process(path):
    with open(path, 'r', encoding='latin-1') as f:
        content = f.read()

    lines = content.split('\n')
    out = []
    i = 0
    rooms_patched = 0

    while i < len(lines):
        line = lines[i]

        # Room header: line starting with # followed by a number
        if line.startswith('#') and line[1:].strip().isdigit():
            vnum = int(line[1:].strip())
            if BASE <= vnum < BASE + ROWS * COLS:
                offset = vnum - BASE
                row = offset // COLS
                col = offset % COLS

                # Collect all lines until 'S' (room terminator)
                room_lines = [line]
                i += 1
                while i < len(lines) and lines[i].strip() != 'S':
                    room_lines.append(lines[i])
                    i += 1
                # lines[i] is 'S'

                # Build diagonal exits to insert before S
                diagonals = []
                # NW = D6
                if row > 0 and col > 0:
                    diagonals.append(diag_exit(6, BASE + (row - 1) * COLS + (col - 1)))
                # NE = D7
                if row > 0 and col < COLS - 1:
                    diagonals.append(diag_exit(7, BASE + (row - 1) * COLS + (col + 1)))
                # SE = D8
                if row < ROWS - 1 and col < COLS - 1:
                    diagonals.append(diag_exit(8, BASE + (row + 1) * COLS + (col + 1)))
                # SW = D9
                if row < ROWS - 1 and col > 0:
                    diagonals.append(diag_exit(9, BASE + (row + 1) * COLS + (col - 1)))

                out.extend(room_lines)
                for d in diagonals:
                    # diag_exit returns a string with embedded newlines
                    out.extend(d.rstrip('\n').split('\n'))
                out.append('S')
                rooms_patched += 1
                i += 1  # skip the 'S' we just handled
                continue

        out.append(line)
        i += 1

    print(f"Patched {rooms_patched} rooms with diagonal exits.", file=sys.stderr)

    with open(path, 'w', encoding='latin-1') as f:
        f.write('\n'.join(out))


if __name__ == '__main__':
    path = sys.argv[1] if len(sys.argv) > 1 else WLD_FILE
    process(path)
    print("Done.", file=sys.stderr)

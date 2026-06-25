#!/usr/bin/env python3
"""
extract_biome_mask.py — Build tools/biome_mask.txt from a hand-marked sample.

The sample (Greed_Island_Map_29_hillvsmountainsample.png) is the full map with
mountain areas circled in RED and hills areas circled in PURPLE. This script
fills the interior of each circle and downsamples the result to the same 256x256
grid geometry used by img_to_map.py, writing a mask file of one char per tile:

    M = inside a mountain circle
    H = inside a hills circle
    . = neither

img_to_map.py reads this mask to decide where brown terrain becomes mountain vs
hills (and to suppress spurious brown outside any circle).

Usage:
    python tools/extract_biome_mask.py [--sample PATH] [--out tools/biome_mask.txt]
"""
import argparse
import os
import sys
from collections import deque

REPO_ROOT = os.path.join(os.path.dirname(__file__), "..")
SAMPLE    = os.path.join(REPO_ROOT, "..",
                         "Greed_Island_Map_29_hillvsmountainsample.png")
OUT       = os.path.join(os.path.dirname(__file__), "biome_mask.txt")

# Grid geometry — MUST match img_to_map.py
GRID      = 256
ROW_START = 2
ROW_END   = 254
COL_START = 39
COL_END   = 217

DILATE    = 3     # thicken circle outlines this many px to close small gaps


def is_red(r, g, b):
    return r > 150 and g < 90 and b < 90

def is_purple(r, g, b):
    return r > 110 and b > 110 and g < 110 and abs(r - b) < 90


def outline_mask(px, W, H, test):
    m = [[False] * W for _ in range(H)]
    for y in range(H):
        for x in range(W):
            r, g, b, a = px[x, y]
            if a >= 128 and test(r, g, b):
                m[y][x] = True
    return m


def dilate(m, W, H, d):
    out = [[False] * W for _ in range(H)]
    for y in range(H):
        for x in range(W):
            if not m[y][x]:
                continue
            for dy in range(-d, d + 1):
                for dx in range(-d, d + 1):
                    ny, nx = y + dy, x + dx
                    if 0 <= ny < H and 0 <= nx < W:
                        out[ny][nx] = True
    return out


def fill_interior(outline, W, H, d):
    """Fill the inside of closed outlines: dilate, flood from border, invert, erode back."""
    thick = dilate(outline, W, H, d)
    reached = [[False] * W for _ in range(H)]
    q = deque()
    for x in range(W):
        for y in (0, H - 1):
            if not thick[y][x] and not reached[y][x]:
                reached[y][x] = True
                q.append((y, x))
    for y in range(H):
        for x in (0, W - 1):
            if not thick[y][x] and not reached[y][x]:
                reached[y][x] = True
                q.append((y, x))
    while q:
        y, x = q.popleft()
        for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            ny, nx = y + dy, x + dx
            if 0 <= ny < H and 0 <= nx < W and not thick[ny][nx] and not reached[ny][nx]:
                reached[ny][nx] = True
                q.append((ny, nx))
    interior = [[(not reached[y][x]) for x in range(W)] for y in range(H)]
    # erode by d to undo the dilation so the region tracks the drawn circle
    er = [[False] * W for _ in range(H)]
    for y in range(H):
        for x in range(W):
            if not interior[y][x]:
                continue
            ok = True
            for dy in range(-d, d + 1):
                for dx in range(-d, d + 1):
                    ny, nx = y + dy, x + dx
                    if not (0 <= ny < H and 0 <= nx < W) or not interior[ny][nx]:
                        ok = False
                        break
                if not ok:
                    break
            er[y][x] = ok
    return er


def downsample(fill, W, H):
    g = [[False] * GRID for _ in range(GRID)]
    dr_total = ROW_END - ROW_START
    dc_total = COL_END - COL_START
    for row in range(ROW_START, ROW_END):
        for col in range(COL_START, COL_END):
            dr = row - ROW_START
            dc = col - COL_START
            x0 = int(dc * W / dc_total); x1 = max(x0 + 1, int((dc + 1) * W / dc_total))
            y0 = int(dr * H / dr_total); y1 = max(y0 + 1, int((dr + 1) * H / dr_total))
            cnt = tot = 0
            for yy in range(y0, min(y1, H)):
                for xx in range(x0, min(x1, W)):
                    tot += 1
                    if fill[yy][xx]:
                        cnt += 1
            if tot and cnt / tot >= 0.5:
                g[row][col] = True
    return g


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sample", default=SAMPLE)
    ap.add_argument("--out", default=OUT)
    args = ap.parse_args()

    try:
        from PIL import Image
    except ImportError:
        print("ERROR: Pillow not installed.", file=sys.stderr)
        sys.exit(1)

    if not os.path.exists(args.sample):
        print("ERROR: sample not found: {}".format(args.sample), file=sys.stderr)
        sys.exit(1)

    im = Image.open(args.sample).convert("RGBA")
    W, H = im.size
    px = im.load()
    print("Sample: {}x{}".format(W, H))

    red_fill = fill_interior(outline_mask(px, W, H, is_red),    W, H, DILATE)
    pur_fill = fill_interior(outline_mask(px, W, H, is_purple), W, H, DILATE)

    mnt = downsample(red_fill, W, H)
    hil = downsample(pur_fill, W, H)

    n_m = sum(sum(r) for r in mnt)
    n_h = sum(sum(r) for r in hil)
    print("Mountain tiles: {}   Hills tiles: {}".format(n_m, n_h))

    with open(args.out, "w", encoding="ascii") as f:
        for row in range(GRID):
            line = []
            for col in range(GRID):
                if hil[row][col]:
                    line.append('H')      # hills wins where they overlap
                elif mnt[row][col]:
                    line.append('M')
                else:
                    line.append('.')
            f.write(''.join(line) + "\n")
    print("Wrote: {}".format(args.out))


if __name__ == "__main__":
    main()

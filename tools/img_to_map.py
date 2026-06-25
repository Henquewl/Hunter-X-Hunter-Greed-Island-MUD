#!/usr/bin/env python3
"""
img_to_map.py — Convert Greed_Island_Map_29.png to lib/world/map/greed_island.txt

Usage:
    python tools/img_to_map.py [--preview PREVIEW.png]
    python tools/img_to_map.py --png /path/to/map.png --out /path/to/greed_island.txt

The source PNG is 453x640 RGBA; transparency = ocean.
Output is 256x256 ASCII grid (lines x cols), ocean border of 2 tiles on all sides.

Layout (preserving the 0.707 portrait aspect):
    Island occupies rows 2..253 (252 data rows), cols 39..216 (178 data cols).
    Everything outside that rectangle is ocean (~).
    Border rows/cols 0,1 and 254,255 are all ocean -- satisfying gen_worldmap.py's requirement.

Tile legend:
    ~  ocean (transparent pixel or outside island rect)
    .  field        (light/bright green, open meadow)
    f  forest       (dark/saturated green -- tree clumps)
    h  hills        (light brown)
    ^  mountain     (dark brown)
    =  water        (blue -- rivers/lakes)
    b  beach        (whitish/sandy -- low saturation, high brightness, warm)
    r  road         (grey/desaturated -- city dots + connecting paths)

Classification strategy:
  1. For rare/thin features (water, beach, road), per-pixel priority voting is used
     so they survive downsampling.
  2. For terrain (field, forest, hills, mountain), the block-average color is classified:
     - R > G by >12 => brown family (mountain if dark, hills if medium, field if very bright)
     - R ~ G => yellow-green family (forest if dark/saturated, field if bright/less-saturated)
     - G > R by >12 => green family (forest if dark, field if bright)
"""

import argparse
import os
import sys

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
REPO_ROOT  = os.path.join(os.path.dirname(__file__), "..")
PNG_SRC    = os.path.join(REPO_ROOT, "..", "Downloads", "Greed_Island_Map_29.png")
MAP_OUT    = os.path.join(REPO_ROOT, "lib", "world", "map", "greed_island.txt")

# ---------------------------------------------------------------------------
# Grid geometry
# ---------------------------------------------------------------------------
GRID       = 256       # side length of the output grid
IMG_W      = 453       # source PNG width
IMG_H      = 640       # source PNG height

# Island occupies rows [ROW_START..ROW_END) and cols [COL_START..COL_END)
ROW_START  = 2
ROW_END    = 254       # 252 data rows
COL_START  = 39
COL_END    = 217       # 178 data cols  (approx 252 * 453/640 = 178.4)

DATA_ROWS  = ROW_END  - ROW_START   # 252
DATA_COLS  = COL_END  - COL_START   # 178

# ---------------------------------------------------------------------------
# Priority weights for per-pixel voting
# Rare/thin features get extra weight so they survive downsampling.
# ---------------------------------------------------------------------------
PRIORITY = {
    '=': 6,   # rivers are 1-2px wide
    'b': 5,   # beaches are a few px wide
    'r': 4,   # roads / city dots are thin
    '^': 2,
    'f': 2,
    'h': 1,
    '.': 1,
    '~': 1,
}

# ---------------------------------------------------------------------------
# Classifiers
# ---------------------------------------------------------------------------

def classify_pixel_rare(r, g, b, a):
    """
    Classify a single pixel, returning ONLY rare/thin features.
    Returns None for terrain (let block-average handle it).
    """
    if a < 128:
        return '~'
    bri = (r + g + b) / 3.0
    if bri < 30:
        return None   # near-black outline / anti-alias

    # --- Water: strongly blue ---
    if b > r + 40 and b > g + 20 and b > 100:
        return '='

    # --- Road/city: grey (low absolute saturation, medium brightness) ---
    max_c = max(r, g, b)
    min_c = min(r, g, b)
    if (max_c - min_c) < 35 and 50 < bri < 175:
        return 'r'

    # --- Beach: high brightness, low relative saturation, warm (R >= B) ---
    if bri > 160 and (max_c - min_c) / max(max_c, 1) < 0.42 and b > 110 and r >= b:
        return 'b'

    # --- Teal/cyan water (lakes may have this cast) ---
    import colorsys
    h, s, v = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
    hd = h * 360.0
    if 150 < hd < 270 and s > 0.25 and v > 0.3:
        return '='

    return None   # terrain -- classify by block average


def classify_block(r, g, b, opaque_frac):
    """
    Classify a block given its average (r, g, b) of non-transparent, non-black pixels.
    opaque_frac = fraction of the block that was opaque (alpha >= 128).
    """
    import colorsys

    if opaque_frac < 0.25:
        return '~'

    bri = (r + g + b) / 3.0

    # --- Water (blue) ---
    if b > r + 40 and b > g + 20 and b > 100:
        return '='

    # --- Beach: high brightness, low relative saturation, warm ---
    max_c = max(r, g, b)
    min_c = min(r, g, b)
    if bri > 165 and (max_c - min_c) / max(max_c, 1) < 0.42 and b > 110 and r >= b:
        return 'b'

    # --- Road/city: grey ---
    if (max_c - min_c) < 35 and 50 < bri < 175:
        return 'r'

    # --- Teal/cyan water ---
    h, s, v = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
    hd = h * 360.0
    if 150 < hd < 270 and s > 0.25 and v > 0.3:
        return '='

    # ---- Terrain: use R-G as brown/green discriminator ----
    rdiff = r - g   # positive = brownish, negative = greenish

    # Brown/earth family (R clearly > G)
    if rdiff > 12:
        if v < 0.50:
            return '^'   # dark brown = mountain
        elif v < 0.72:
            return 'h'   # light brown = hills
        else:
            return '.'   # very bright warm = open field / dry grass

    # Yellow-green family (R approx == G)
    if abs(rdiff) <= 12 and s > 0.10:
        if v < 0.50:
            return 'f'   # dark = forest shadow
        elif v > 0.68 and s < 0.72:
            return '.'   # bright, moderate saturation = open field
        elif v > 0.74:
            return '.'   # very bright = field
        else:
            return 'f'   # default green = forest

    # Green family (G clearly > R)
    if (g - r) > 12 and s > 0.10:
        return 'f' if v < 0.55 else '.'

    return '.'   # fallback = field


# ---------------------------------------------------------------------------
# Grid conversion
# ---------------------------------------------------------------------------

def img_to_grid(im):
    """
    Convert a PIL RGBA image to a 256x256 list-of-strings grid.
    Returns list of 256 strings, each 256 chars.
    """
    px = im.load()
    grid = []

    for row in range(GRID):
        line = []
        for col in range(GRID):

            # Tiles outside island rect or in ocean border -> ocean
            if row < ROW_START or row >= ROW_END or col < COL_START or col >= COL_END:
                line.append('~')
                continue

            # Map grid cell to source-image rectangle
            dr = row - ROW_START
            dc = col - COL_START

            x0 = int(dc       * IMG_W / DATA_COLS)
            x1 = max(x0 + 1, int((dc + 1) * IMG_W / DATA_COLS))
            y0 = int(dr       * IMG_H / DATA_ROWS)
            y1 = max(y0 + 1, int((dr + 1) * IMG_H / DATA_ROWS))
            x1 = min(x1, IMG_W)
            y1 = min(y1, IMG_H)

            # --- Per-pixel pass: priority voting for rare features ---
            prio_votes = {}
            tr = tg = tb = tn = 0
            transp = 0
            total_px = 0

            for py in range(y0, y1):
                for px_ in range(x0, x1):
                    R, G, B, A = px[px_, py]
                    total_px += 1
                    if A < 128:
                        transp += 1
                        continue
                    if (R + G + B) / 3 < 30:
                        continue  # skip near-black
                    t2 = classify_pixel_rare(R, G, B, A)
                    if t2 and t2 != '~':
                        prio_votes[t2] = prio_votes.get(t2, 0) + PRIORITY.get(t2, 1)
                    tr += R
                    tg += G
                    tb += B
                    tn += 1

            opaque_frac = (total_px - transp) / max(total_px, 1)

            # Mostly transparent -> ocean
            if opaque_frac < 0.25:
                line.append('~')
                continue

            # If rare features won the priority vote -> use them
            if prio_votes:
                winner = max(prio_votes, key=lambda t: prio_votes[t])
                line.append(winner)
                continue

            # --- Block-average pass for terrain ---
            if tn == 0:
                line.append('~')
                continue

            tile = classify_block(tr / tn, tg / tn, tb / tn, opaque_frac)
            line.append(tile)

        grid.append(''.join(line))

    return grid


# ---------------------------------------------------------------------------
# Preview PNG renderer (optional, for visual calibration)
# ---------------------------------------------------------------------------
TILE_COLORS_RGB = {
    '~': (10,   20,  80),   # dark blue (ocean)
    '.': (140, 200,  80),   # light green (field)
    'f': ( 30, 100,  20),   # dark green (forest)
    'h': (180, 140,  80),   # light brown (hills)
    '^': (100,  70,  30),   # dark brown (mountain)
    '=': ( 60, 160, 200),   # blue (water/lake)
    'b': (240, 220, 150),   # sandy yellow (beach)
    'r': (200, 185, 155),   # light grey-brown (road)
}

def render_preview(grid, out_path):
    try:
        from PIL import Image as PILImage
    except ImportError:
        print("PIL not available; skipping preview.", file=sys.stderr)
        return
    scale = 3
    img = PILImage.new("RGB", (GRID * scale, GRID * scale), (0, 0, 0))
    pix = img.load()
    for row in range(GRID):
        for col in range(GRID):
            tile = grid[row][col]
            color = TILE_COLORS_RGB.get(tile, (128, 0, 128))
            for dy in range(scale):
                for dx in range(scale):
                    pix[col * scale + dx, row * scale + dy] = color
    img.save(out_path)
    print("Preview written: {}".format(out_path))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(description="Convert PNG map to greed_island.txt")
    parser.add_argument("--png",     default=None,
        help="Source PNG path (default: ../Downloads/Greed_Island_Map_29.png)")
    parser.add_argument("--out",     default=None,
        help="Output .txt path (default: lib/world/map/greed_island.txt)")
    parser.add_argument("--preview", default=None, metavar="PREVIEW.png",
        help="Also write a colour-coded preview PNG to this path")
    args = parser.parse_args()

    png_path = args.png or PNG_SRC
    out_path = args.out or MAP_OUT

    try:
        from PIL import Image
    except ImportError:
        print("ERROR: Pillow (PIL) not installed. Run: pip install Pillow", file=sys.stderr)
        sys.exit(1)

    if not os.path.exists(png_path):
        print("ERROR: PNG not found: {}".format(png_path), file=sys.stderr)
        sys.exit(1)

    print("Loading: {}".format(png_path))
    im = Image.open(png_path).convert("RGBA")
    actual_w, actual_h = im.size
    if actual_w != IMG_W or actual_h != IMG_H:
        print("WARNING: expected {}x{} but got {}x{}".format(
            IMG_W, IMG_H, actual_w, actual_h))
        print("         Geometry constants may need adjustment.")

    print("Converting to {}x{} grid...".format(GRID, GRID))
    grid = img_to_grid(im)

    from collections import Counter
    counts = Counter(ch for row in grid for ch in row)
    total  = GRID * GRID
    print("Tile counts:")
    for tile, cnt in sorted(counts.items(), key=lambda kv: -kv[1]):
        print("  {:2s}  {:6d}  ({:5.1f}%)".format(tile, cnt, 100 * cnt / total))

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="ascii") as f:
        for row in grid:
            f.write(row + "\n")
    print("Written: {}".format(out_path))

    if args.preview:
        render_preview(grid, args.preview)

    print("Done. Next: python tools/gen_worldmap.py --validate-only")


if __name__ == "__main__":
    main()

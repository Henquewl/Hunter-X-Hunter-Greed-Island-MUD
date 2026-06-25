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
    .  field        (light/bright green, open meadow -- the default land tile)
    f  forest       (dense tree clumps -- only where canopy shadows are clear)
    h  hills        (light/bright brown)
    ^  mountain     (dark brown, ridge shadows)
    =  water        (blue -- rivers/lakes)
    b  beach        (whitish/sandy -- low saturation, high brightness, warm)

Classification strategy (calibrated against user-marked reference samples):
  1. Rare/thin features (water, beach) win via per-pixel priority voting so they
     survive downsampling.
  2. Terrain is decided from the block-average color, biased toward FIELD when in
     doubt (per user direction):
     - Real brown (R-G > BROWN_RG): hills if bright (V > HILLS_V), else mountain.
       The brightness V -- not R-G -- separates hills (light/rust) from mountain
       (dark ridge shadows). Weak browns (R-G <= BROWN_RG, e.g. coastal/among-field
       tints) fall through to field.
     - Green: forest ONLY when the block has a clear fraction of dark canopy-shadow
       pixels (>= FOREST_DARK); otherwise field. Field and forest are nearly
       identical in average color in this art, so the bias is toward field.
  3. Spatial denoise: small isolated clusters of brown ({h,^}) or forest ({f})
     are flipped to field, removing speckle while keeping real ranges/forests.
  4. Coastal cleanup: interior "beach" patches (no water nearby) revert to field;
     real beaches get their holes filled; detached islets are dropped; 1-tile
     coastal spikes/threads are eroded so the shoreline isn't ragged.
  5. Roads are NOT generated here -- cities and roads will be placed by hand later.
"""

import argparse
import os
import sys

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
REPO_ROOT  = os.path.join(os.path.dirname(__file__), "..")
PNG_SRC    = os.path.join(REPO_ROOT, "..", "Greed_Island_Map_29.webp")
MAP_OUT    = os.path.join(REPO_ROOT, "lib", "world", "map", "greed_island.txt")
MASK_SRC   = os.path.join(os.path.dirname(__file__), "biome_mask.txt")

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
# Classification tunables (calibrated against the marked reference image)
# ---------------------------------------------------------------------------
BROWN_RG    = 15     # R-G above this = real brown (hills/mountain); below -> field
HILLS_V     = 0.60   # within brown: V above -> hills (light), else mountain (dark)
FOREST_DARK = 0.18   # min fraction of dark canopy-shadow pixels in a block -> forest
MIN_BROWN   = 14     # connected {h,^} clusters smaller than this -> field
MIN_FOREST  = 8      # connected {f}  clusters smaller than this -> field

# Coastal cleanup (post-processing) tunables
INLAND_BEACH_RADIUS = 3   # beach with no ocean/water within this Chebyshev radius -> field
FILL_BEACH_ITERS    = 2   # passes of: field/forest mostly surrounded by beach -> beach
FILL_BEACH_THRESH   = 4   # min beach neighbours (of 8) to absorb a field/forest tile
ISLET_MIN           = 30  # land components smaller than this (not the main island) -> ocean
ERODE_MAXLAND       = 1   # land tile with <= this many land neighbours (of 4) -> ocean (tips/threads)

# ---------------------------------------------------------------------------
# Priority weights for per-pixel voting
# Rare/thin features get extra weight so they survive downsampling.
# ---------------------------------------------------------------------------
PRIORITY = {
    '=': 6,   # rivers are 1-2px wide
    'b': 5,   # beaches are a few px wide
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

    # --- Beach: high brightness, low relative saturation, warm (R >= B) ---
    max_c = max(r, g, b)
    min_c = min(r, g, b)
    if bri > 160 and (max_c - min_c) / max(max_c, 1) < 0.42 and b > 110 and r >= b:
        return 'b'

    # --- Teal/cyan water (lakes may have this cast) ---
    import colorsys
    h, s, v = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
    hd = h * 360.0
    if 150 < hd < 270 and s > 0.25 and v > 0.3:
        return '='

    return None   # terrain -- classify by block average


def classify_block(r, g, b, opaque_frac, dark_green_frac):
    """
    Classify a block given its average (r, g, b) of non-transparent, non-black pixels.
    opaque_frac     = fraction of the block that was opaque (alpha >= 128).
    dark_green_frac = fraction of the block's pixels that look like canopy shadow
                      (greenish AND dark AND saturated) -- the only usable forest signal.

    Calibrated from samples taken directly off the map:
      Hills  (upper "lip" / rust): R-G ~ 40, V ~ 0.66  -> brown & bright
      Mountain (lower "lip"/ridge): R-G ~ 28, V ~ 0.56  -> brown & dark
      Mountain (real range):        R-G ~ 19, V ~ 0.50  -> brown & dark
      Spurious coastal brown:       R-G ~ 12            -> falls through to field
      Field / Forest:               R-G ~ 0, V ~ 0.57-0.59 (nearly identical)
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

    # --- Teal/cyan water ---
    h, s, v = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
    hd = h * 360.0
    if 150 < hd < 270 and s > 0.25 and v > 0.3:
        return '='

    # ---- Brown family: R clearly warmer than G ----
    # Brightness V separates hills (light/rust) from mountain (dark ridge shadow).
    if (r - g) > BROWN_RG:
        return 'h' if v > HILLS_V else '^'

    # ---- Green family: forest only with clear canopy-shadow texture, else field ----
    if dark_green_frac >= FOREST_DARK:
        return 'f'

    return '.'           # default: field (bias toward field when in doubt)


# ---------------------------------------------------------------------------
# Spatial denoise: flip small isolated clusters to field
# ---------------------------------------------------------------------------

def denoise_clusters(grid, members, min_size, replace='.'):
    """
    4-connected flood fill over tiles whose char is in `members`.
    Any connected component smaller than `min_size` is rewritten to `replace`.
    Mutates `grid` (a list of lists) in place.
    """
    from collections import deque
    seen = [[False] * GRID for _ in range(GRID)]
    for i in range(GRID):
        for j in range(GRID):
            if seen[i][j] or grid[i][j] not in members:
                continue
            comp = []
            q = deque([(i, j)])
            seen[i][j] = True
            while q:
                y, x = q.popleft()
                comp.append((y, x))
                for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                    ny, nx = y + dy, x + dx
                    if (0 <= ny < GRID and 0 <= nx < GRID
                            and not seen[ny][nx] and grid[ny][nx] in members):
                        seen[ny][nx] = True
                        q.append((ny, nx))
            if len(comp) < min_size:
                for (y, x) in comp:
                    grid[y][x] = replace


# ---------------------------------------------------------------------------
# Coastal cleanup helpers (operate on a list-of-lists grid, in place)
# ---------------------------------------------------------------------------
LAND_CHARS  = set('.fh^b')   # everything that is dry land (incl. beach)
WATER_CHARS = set('~=')      # ocean + lakes/rivers
N8 = ((-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1))
N4 = ((-1, 0), (1, 0), (0, -1), (0, 1))


def _count_neighbours(grid, r, c, members, offsets):
    n = 0
    for dr, dc in offsets:
        nr, nc = r + dr, c + dc
        if 0 <= nr < GRID and 0 <= nc < GRID and grid[nr][nc] in members:
            n += 1
    return n


def remove_inland_beach(grid, radius):
    """Beach tiles with no ocean/water within `radius` (Chebyshev) -> field.
    Removes bright/dry interior patches mis-read as beach."""
    chg = []
    for r in range(GRID):
        for c in range(GRID):
            if grid[r][c] != 'b':
                continue
            near_water = False
            for dr in range(-radius, radius + 1):
                for dc in range(-radius, radius + 1):
                    nr, nc = r + dr, c + dc
                    if 0 <= nr < GRID and 0 <= nc < GRID and grid[nr][nc] in WATER_CHARS:
                        near_water = True
                        break
                if near_water:
                    break
            if not near_water:
                chg.append((r, c))
    for r, c in chg:
        grid[r][c] = '.'


def fill_beach_holes(grid, iters, thresh):
    """field/forest tiles surrounded by enough beach -> beach (closes holes in beaches)."""
    for _ in range(iters):
        chg = [(r, c) for r in range(GRID) for c in range(GRID)
               if grid[r][c] in '.f'
               and _count_neighbours(grid, r, c, {'b'}, N8) >= thresh]
        for r, c in chg:
            grid[r][c] = 'b'


def remove_small_islands(grid, min_size):
    """All land components except the largest, if smaller than min_size, -> ocean."""
    from collections import deque
    seen = [[False] * GRID for _ in range(GRID)]
    comps = []
    for i in range(GRID):
        for j in range(GRID):
            if seen[i][j] or grid[i][j] not in LAND_CHARS:
                continue
            comp = []
            q = deque([(i, j)])
            seen[i][j] = True
            while q:
                y, x = q.popleft()
                comp.append((y, x))
                for dr, dc in N8:
                    ny, nx = y + dr, x + dc
                    if (0 <= ny < GRID and 0 <= nx < GRID
                            and not seen[ny][nx] and grid[ny][nx] in LAND_CHARS):
                        seen[ny][nx] = True
                        q.append((ny, nx))
            comps.append(comp)
    comps.sort(key=len, reverse=True)
    for comp in comps[1:]:            # keep the largest (the main island)
        if len(comp) < min_size:
            for y, x in comp:
                grid[y][x] = '~'


def erode_tips(grid, max_land):
    """One pass: land tiles with <= max_land land neighbours (of 4) -> ocean.
    Trims 1-tile spikes and threads without shrinking solid masses."""
    chg = [(r, c) for r in range(GRID) for c in range(GRID)
           if grid[r][c] in LAND_CHARS
           and _count_neighbours(grid, r, c, LAND_CHARS, N4) <= max_land]
    for r, c in chg:
        grid[r][c] = '~'


# ---------------------------------------------------------------------------
# Biome mask (mountain/hills location authority, from the hand-marked sample)
# ---------------------------------------------------------------------------

def load_biome_mask(path):
    """Load tools/biome_mask.txt (256x256 of 'M'/'H'/'.'). Returns None if absent."""
    if not os.path.exists(path):
        return None
    with open(path, "r", encoding="ascii") as f:
        rows = [line.rstrip("\n\r") for line in f.readlines()]
    if len(rows) < GRID:
        return None
    return [r.ljust(GRID, '.')[:GRID] for r in rows[:GRID]]


def apply_biome_mask(grid, mask):
    """
    Mountain/hills are only valid inside the marked circles:
      - brown tile (h/^) inside an 'H' region  -> hills
      - brown tile (h/^) inside an 'M' region  -> mountain
      - brown tile outside any region          -> field (spurious brown)
    """
    for r in range(GRID):
        for c in range(GRID):
            if grid[r][c] in 'h^':
                m = mask[r][c]
                grid[r][c] = 'h' if m == 'H' else '^' if m == 'M' else '.'


def solidify_in_mask(grid, mask, region_char, biome_char):
    """
    Fill interior holes of a biome inside its mask region so it reads as a solid
    cluster (no field/forest specks in the middle). A field/forest tile inside the
    region becomes the biome unless it connects (4-way) to the region's outer edge
    -- i.e. only fully-enclosed holes are filled; the green rim of the circle stays.
    """
    from collections import deque
    fillable = [[(grid[r][c] in '.f' and mask[r][c] == region_char)
                 for c in range(GRID)] for r in range(GRID)]
    reached = [[False] * GRID for _ in range(GRID)]
    q = deque()
    # Seed flood from fillable tiles on the region boundary (adjacent to a non-region tile)
    for r in range(GRID):
        for c in range(GRID):
            if not fillable[r][c]:
                continue
            on_border = False
            for dr, dc in N4:
                nr, nc = r + dr, c + dc
                if not (0 <= nr < GRID and 0 <= nc < GRID) or mask[nr][nc] != region_char:
                    on_border = True
                    break
            if on_border:
                reached[r][c] = True
                q.append((r, c))
    while q:
        r, c = q.popleft()
        for dr, dc in N4:
            nr, nc = r + dr, c + dc
            if 0 <= nr < GRID and 0 <= nc < GRID and fillable[nr][nc] and not reached[nr][nc]:
                reached[nr][nc] = True
                q.append((nr, nc))
    # Unreached fillable tiles are enclosed holes -> biome
    for r in range(GRID):
        for c in range(GRID):
            if fillable[r][c] and not reached[r][c]:
                grid[r][c] = biome_char


# ---------------------------------------------------------------------------
# Grid conversion
# ---------------------------------------------------------------------------

def img_to_grid(im):
    """
    Convert a PIL RGBA image to a 256x256 grid.
    Returns list of 256 strings, each 256 chars.
    """
    import colorsys
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

            # --- Per-pixel pass: priority voting + canopy-shadow counting ---
            prio_votes = {}
            tr = tg = tb = tn = 0
            transp = 0
            total_px = 0
            dark_green = 0

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
                    # canopy-shadow signal: greenish, dark, saturated
                    h, s, v = colorsys.rgb_to_hsv(R / 255.0, G / 255.0, B / 255.0)
                    if G >= R and v < 0.45 and s > 0.45:
                        dark_green += 1
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

            dgf = dark_green / tn
            tile = classify_block(tr / tn, tg / tn, tb / tn, opaque_frac, dgf)
            line.append(tile)

        grid.append(line)

    # --- Biome mask: mountain/hills only where the sample marks them ---
    mask = load_biome_mask(MASK_SRC)
    if mask is not None:
        apply_biome_mask(grid, mask)              # brown outside circles -> field
        solidify_in_mask(grid, mask, 'M', '^')    # fill enclosed holes in mountains
        solidify_in_mask(grid, mask, 'H', 'h')    # fill enclosed holes in hills
    else:
        print("WARNING: {} not found; falling back to colour-only brown denoise."
              .format(MASK_SRC), file=sys.stderr)
        denoise_clusters(grid, {'h', '^'}, MIN_BROWN)

    # --- Spatial denoise: small forest specks -> field (forest may be sparse) ---
    denoise_clusters(grid, {'f'}, MIN_FOREST)

    # --- Coastal cleanup ---
    remove_inland_beach(grid, INLAND_BEACH_RADIUS)   # interior "beach" patches -> field
    fill_beach_holes(grid, FILL_BEACH_ITERS, FILL_BEACH_THRESH)  # close holes in real beaches
    remove_small_islands(grid, ISLET_MIN)            # drop detached islets
    erode_tips(grid, ERODE_MAXLAND)                  # trim 1-tile spikes / threads
    remove_small_islands(grid, ISLET_MIN)            # re-drop islets erosion may have split off

    return [''.join(row) for row in grid]


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

#!/usr/bin/env python3
"""
migrate_vnums.py — Task 1: Migrate world data files from zone 653/654 vnums
into new card zones 0-3.

Mapping rules:
  Objects (653.obj → 0.obj):    vnum 65300+N → N (range 0-99)
  Objects (654.obj → 1.obj/3.obj):
    Exceptions to 1.obj: 65400→110, 65401→111, 65534→133, 65535→102
    All others to 3.obj: 65400+N → N+300
  Mobs (654.mob → 3.mob):       vnum 65400+N → N+300
  Triggers (653.trg → 0.trg):   vnum 65300+N → N
  Triggers (654.trg → 3.trg):   vnum 65400+N → N+300
  Trigger bodies: all 65300-65516 occurrences remapped
"""

import re
import os
import sys

PROJECT = "/mnt/c/Users/henqu/source/repos/Hunter-X-Hunter-Greed-Island-MUD"

# ---------------------------------------------------------------------------
# VNUM MAP for trigger body substitution
# ---------------------------------------------------------------------------

VNUM_MAP = {}
# 653 cards: 65300+N → N  (0-99)
for n in range(100):
    VNUM_MAP[65300 + n] = n
# 654 items/mobs: 65400+N → N+300
# but exceptions: 65400→110, 65401→111, 65534→133, 65535→102
for n in range(200):  # 65400-65599 (covers 65516 and beyond safely)
    VNUM_MAP[65400 + n] = n + 300
# Override exceptions
VNUM_MAP[65400] = 110
VNUM_MAP[65401] = 111
VNUM_MAP[65534] = 133
VNUM_MAP[65535] = 102

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def remap_vnum_in_bodies(text):
    """Replace all 65300-65516 range vnums in trigger bodies using VNUM_MAP."""
    pattern = re.compile(r'\b(653\d{2}|654\d{2}|6550\d|6551[0-6])\b')
    def replacer(m):
        v = int(m.group(0))
        if v in VNUM_MAP:
            return str(VNUM_MAP[v])
        return m.group(0)  # not in map, leave unchanged
    return pattern.sub(replacer, text)

def split_obj_entries(content_bytes):
    """
    Split binary .obj content into a list of (vnum_int, entry_bytes) tuples.
    The entry_bytes include the '#VNUM\n' header line through the trailing '~\n'.
    The final '$\n' or '$~\n' terminator is NOT included.
    """
    text = content_bytes.decode('latin-1')
    entries = []
    # Each entry starts with a line like '#65300\n'
    # Split on lines starting with '#' followed by digits
    parts = re.split(r'(?m)^(?=#\d)', text)
    for part in parts:
        part = part.strip('\n')
        if not part:
            continue
        # Skip the terminator
        if part.startswith('$'):
            continue
        m = re.match(r'^#(\d+)', part)
        if m:
            vnum = int(m.group(1))
            entries.append((vnum, part))
        else:
            print(f"  WARNING: skipping unrecognized block: {repr(part[:40])}")
    return entries

def split_mob_entries(content_bytes):
    """Split mob file. Returns list of (vnum, text_block)."""
    return split_obj_entries(content_bytes)  # same structure

def split_trg_entries(content_bytes):
    """Split trigger file. Returns list of (vnum, text_block)."""
    return split_obj_entries(content_bytes)

def remap_obj_entry(entry_text, old_vnum, new_vnum, trig_remap_fn=None):
    """Remap vnum header and T-lines in an obj entry."""
    # Replace the #OLDVNUM header
    result = re.sub(r'^#' + str(old_vnum), '#' + str(new_vnum), entry_text, count=1, flags=re.MULTILINE)
    # Remap T-lines: 'T 65XXX' -> 'T NEW'
    if trig_remap_fn:
        def t_replacer(m):
            old_t = int(m.group(1))
            new_t = trig_remap_fn(old_t)
            return 'T ' + str(new_t)
        result = re.sub(r'\bT (\d{5,})\b', t_replacer, result)
    return result

def remap_mob_entry(entry_text, old_vnum, new_vnum, trig_remap_fn=None):
    """Remap vnum header and T-lines in a mob entry."""
    return remap_obj_entry(entry_text, old_vnum, new_vnum, trig_remap_fn)

def remap_trg_header(entry_text, old_vnum, new_vnum):
    """Remap just the #VNUM header of a trigger entry; body is handled separately."""
    return re.sub(r'^#' + str(old_vnum), '#' + str(new_vnum), entry_text, count=1, flags=re.MULTILINE)

def entries_to_bytes(entries, terminator=b'$\n'):
    """
    Reassemble entries into file bytes.
    entries: list of text strings (already remapped).
    """
    result = b''
    for entry in entries:
        # Ensure entry ends with exactly one newline before next entry
        text = entry.rstrip('\n') + '\n'
        result += text.encode('latin-1')
    result += terminator
    return result

def write_file(path, content_bytes):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(content_bytes)
    print(f"  Written: {path} ({len(content_bytes)} bytes)")

def write_text_file(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', newline='\n') as f:
        f.write(content)
    print(f"  Written: {path} ({len(content)} bytes)")

# ---------------------------------------------------------------------------
# Trig remap function: maps old trigger vnum to new
# ---------------------------------------------------------------------------

def trig_remap_from_653(old_t):
    """653.trg triggers: 65300+N -> N"""
    if 65300 <= old_t <= 65399:
        return old_t - 65300
    # If out of range, remap via VNUM_MAP if possible
    if old_t in VNUM_MAP:
        return VNUM_MAP[old_t]
    return old_t

def trig_remap_from_654(old_t):
    """654.trg triggers: 65400+N -> N+300 (with exceptions)"""
    if old_t in VNUM_MAP:
        return VNUM_MAP[old_t]
    return old_t

# ---------------------------------------------------------------------------
# STEP 1: Generate 0.obj (cards from 653.obj, vnums 65300→0..65399→99)
# ---------------------------------------------------------------------------

print("=== Step 1: Generating 0.obj from 653.obj ===")
src_653_obj = os.path.join(PROJECT, "lib/world/obj/653.obj")
with open(src_653_obj, 'rb') as f:
    raw = f.read()

entries_653 = split_obj_entries(raw)
print(f"  Found {len(entries_653)} entries in 653.obj")

remapped_0 = []
for old_vnum, entry_text in entries_653:
    if 65300 <= old_vnum <= 65399:
        new_vnum = old_vnum - 65300
        remapped = remap_obj_entry(entry_text, old_vnum, new_vnum, trig_remap_fn=trig_remap_from_653)
        remapped_0.append((new_vnum, remapped))
    else:
        print(f"  WARNING: unexpected vnum {old_vnum} in 653.obj, skipping")

# Sort by new vnum (should already be in order, but enforce)
remapped_0.sort(key=lambda x: x[0])
print(f"  Mapped {len(remapped_0)} entries -> 0.obj vnums {remapped_0[0][0]}-{remapped_0[-1][0]}")

out_0_obj = entries_to_bytes([e for _, e in remapped_0], terminator=b'$\n')
write_file(os.path.join(PROJECT, "lib/world/obj/0.obj"), out_0_obj)

# ---------------------------------------------------------------------------
# STEP 2: Generate 1.obj (exceptions from 654.obj) and 3.obj (remainder)
# ---------------------------------------------------------------------------

print("\n=== Step 2: Generating 1.obj and 3.obj from 654.obj ===")
src_654_obj = os.path.join(PROJECT, "lib/world/obj/654.obj")
with open(src_654_obj, 'rb') as f:
    raw = f.read()

entries_654 = split_obj_entries(raw)
print(f"  Found {len(entries_654)} entries in 654.obj")

# Exception mapping: old vnum -> new vnum for 1.obj
EXCEPTIONS_1 = {
    65400: 110,
    65401: 111,
    65534: 133,
    65535: 102,
}

entries_1 = []  # for 1.obj
entries_3 = []  # for 3.obj

for old_vnum, entry_text in entries_654:
    if old_vnum in EXCEPTIONS_1:
        new_vnum = EXCEPTIONS_1[old_vnum]
        remapped = remap_obj_entry(entry_text, old_vnum, new_vnum, trig_remap_fn=trig_remap_from_654)
        entries_1.append((new_vnum, remapped))
    elif 65400 <= old_vnum <= 65499:
        new_vnum = (old_vnum - 65400) + 300
        remapped = remap_obj_entry(entry_text, old_vnum, new_vnum, trig_remap_fn=trig_remap_from_654)
        entries_3.append((new_vnum, remapped))
    else:
        print(f"  WARNING: unexpected vnum {old_vnum} in 654.obj, skipping")

# Sort by new vnum
entries_1.sort(key=lambda x: x[0])
entries_3.sort(key=lambda x: x[0])

print(f"  1.obj: {len(entries_1)} entries (vnums: {[v for v,_ in entries_1]})")
print(f"  3.obj: {len(entries_3)} entries")
if entries_3:
    print(f"         vnums {entries_3[0][0]}-{entries_3[-1][0]}")

out_1_obj = entries_to_bytes([e for _, e in entries_1], terminator=b'$\n')
write_file(os.path.join(PROJECT, "lib/world/obj/1.obj"), out_1_obj)

out_3_obj = entries_to_bytes([e for _, e in entries_3], terminator=b'$\n')
write_file(os.path.join(PROJECT, "lib/world/obj/3.obj"), out_3_obj)

# ---------------------------------------------------------------------------
# STEP 3: Generate 2.obj (placeholder)
# ---------------------------------------------------------------------------

print("\n=== Step 3: Generating 2.obj (placeholder) ===")
write_file(os.path.join(PROJECT, "lib/world/obj/2.obj"), b'$~\n')

# ---------------------------------------------------------------------------
# STEP 4: Generate 3.mob from 654.mob
# ---------------------------------------------------------------------------

print("\n=== Step 4: Generating 3.mob from 654.mob ===")
src_654_mob = os.path.join(PROJECT, "lib/world/mob/654.mob")
with open(src_654_mob, 'rb') as f:
    raw = f.read()

entries_654_mob = split_mob_entries(raw)
print(f"  Found {len(entries_654_mob)} entries in 654.mob")

remapped_3mob = []
for old_vnum, entry_text in entries_654_mob:
    if 65400 <= old_vnum <= 65499:
        new_vnum = (old_vnum - 65400) + 300
        remapped = remap_mob_entry(entry_text, old_vnum, new_vnum, trig_remap_fn=trig_remap_from_654)
        remapped_3mob.append((new_vnum, remapped))
    else:
        print(f"  WARNING: unexpected mob vnum {old_vnum} in 654.mob")

remapped_3mob.sort(key=lambda x: x[0])
print(f"  Mapped {len(remapped_3mob)} mobs -> vnums: {[v for v,_ in remapped_3mob]}")

out_3_mob = entries_to_bytes([e for _, e in remapped_3mob], terminator=b'$\n')
write_file(os.path.join(PROJECT, "lib/world/mob/3.mob"), out_3_mob)

# ---------------------------------------------------------------------------
# STEP 5: Generate empty mob files (0.mob, 1.mob, 2.mob)
# ---------------------------------------------------------------------------

print("\n=== Step 5: Generating empty mob files 0.mob, 1.mob, 2.mob ===")
for n in [0, 1, 2]:
    path = os.path.join(PROJECT, f"lib/world/mob/{n}.mob")
    write_file(path, b'$~\n')

# ---------------------------------------------------------------------------
# STEP 6: Generate 0.trg from 653.trg
# ---------------------------------------------------------------------------

print("\n=== Step 6: Generating 0.trg from 653.trg ===")
src_653_trg = os.path.join(PROJECT, "lib/world/trg/653.trg")
with open(src_653_trg, 'rb') as f:
    raw = f.read()

entries_653_trg = split_trg_entries(raw)
print(f"  Found {len(entries_653_trg)} entries in 653.trg")

remapped_0trg = []
for old_vnum, entry_text in entries_653_trg:
    if 65300 <= old_vnum <= 65399:
        new_vnum = old_vnum - 65300
        # First remap header
        remapped = remap_trg_header(entry_text, old_vnum, new_vnum)
        # Then remap all vnums in body
        remapped = remap_vnum_in_bodies(remapped)
        remapped_0trg.append((new_vnum, remapped))
    else:
        print(f"  WARNING: unexpected trigger vnum {old_vnum} in 653.trg")

remapped_0trg.sort(key=lambda x: x[0])
print(f"  Mapped {len(remapped_0trg)} triggers -> vnums: {[v for v,_ in remapped_0trg]}")

out_0_trg = entries_to_bytes([e for _, e in remapped_0trg], terminator=b'$~\n')
write_file(os.path.join(PROJECT, "lib/world/trg/0.trg"), out_0_trg)

# ---------------------------------------------------------------------------
# STEP 7: Generate 1.trg (empty placeholder)
# ---------------------------------------------------------------------------

print("\n=== Step 7: Generating 1.trg (empty placeholder) ===")
write_file(os.path.join(PROJECT, "lib/world/trg/1.trg"), b'$~\n')

# ---------------------------------------------------------------------------
# STEP 8: Generate 3.trg from 654.trg
# ---------------------------------------------------------------------------

print("\n=== Step 8: Generating 3.trg from 654.trg ===")
src_654_trg = os.path.join(PROJECT, "lib/world/trg/654.trg")
with open(src_654_trg, 'rb') as f:
    raw = f.read()

entries_654_trg = split_trg_entries(raw)
print(f"  Found {len(entries_654_trg)} entries in 654.trg")

remapped_3trg = []
for old_vnum, entry_text in entries_654_trg:
    if 65400 <= old_vnum <= 65599:
        new_vnum = (old_vnum - 65400) + 300
        remapped = remap_trg_header(entry_text, old_vnum, new_vnum)
        remapped = remap_vnum_in_bodies(remapped)
        remapped_3trg.append((new_vnum, remapped))
    else:
        print(f"  WARNING: unexpected trigger vnum {old_vnum} in 654.trg")

remapped_3trg.sort(key=lambda x: x[0])
print(f"  Mapped {len(remapped_3trg)} triggers -> vnums {remapped_3trg[0][0]}-{remapped_3trg[-1][0]}")

out_3_trg = entries_to_bytes([e for _, e in remapped_3trg], terminator=b'$~\n')
write_file(os.path.join(PROJECT, "lib/world/trg/3.trg"), out_3_trg)

# ---------------------------------------------------------------------------
# STEP 9: Generate zone files 0.zon - 3.zon
# ---------------------------------------------------------------------------

print("\n=== Step 9: Generating zone files 0.zon - 3.zon ===")

zone_contents = {
    "0.zon": "#0\nHenque~\nRestricted Cards~\n0 99 20 0 f 0 0 0 -1 -1\nS\n$~\n",
    "1.zon": "#1\nHenque~\nFree Cards Special~\n100 199 20 0 f 0 0 0 -1 -1\nS\n$~\n",
    "2.zon": "#2\nHenque~\nFree Cards~\n200 299 20 0 f 0 0 0 -1 -1\nS\n$~\n",
    "3.zon": "#3\nHenque~\nCard Items and Mobs~\n300 416 20 0 f 0 0 0 -1 -1\nS\n$~\n",
}

for fname, content in zone_contents.items():
    path = os.path.join(PROJECT, f"lib/world/zon/{fname}")
    write_text_file(path, content)

# ---------------------------------------------------------------------------
# VERIFICATION
# ---------------------------------------------------------------------------

print("\n=== Verification ===")
import subprocess

checks = [
    ("0.obj entry count (expect 100)",
     f"grep -c '^#' {PROJECT}/lib/world/obj/0.obj"),
    ("0.obj first 3 vnums (expect #0 #1 #2)",
     f"grep '^#' {PROJECT}/lib/world/obj/0.obj | head -3"),
    ("0.obj last 3 vnums (expect #97 #98 #99)",
     f"grep '^#' {PROJECT}/lib/world/obj/0.obj | tail -3"),
    ("1.obj vnums (expect #102 #110 #111 #133 sorted)",
     f"grep '^#' {PROJECT}/lib/world/obj/1.obj"),
    ("3.obj entry count",
     f"grep -c '^#' {PROJECT}/lib/world/obj/3.obj"),
    ("3.obj first and last vnums",
     f"grep '^#' {PROJECT}/lib/world/obj/3.obj | head -3 && grep '^#' {PROJECT}/lib/world/obj/3.obj | tail -3"),
    ("0.trg: no old 6530x vnums (expect empty)",
     f"grep '6530' {PROJECT}/lib/world/trg/0.trg || echo 'PASS: no 6530x found'"),
    ("3.trg: no old 6530x or 6540x vnums (expect empty)",
     f"grep '6530\\|6540' {PROJECT}/lib/world/trg/3.trg || echo 'PASS: no old vnums found'"),
    ("3.mob mobs (expect 317 346 347 348 396)",
     f"grep '^#' {PROJECT}/lib/world/mob/3.mob"),
    ("0.trg trigger vnums",
     f"grep '^#' {PROJECT}/lib/world/trg/0.trg"),
    ("3.trg trigger vnums",
     f"grep '^#' {PROJECT}/lib/world/trg/3.trg"),
    ("0.zon content",
     f"cat {PROJECT}/lib/world/zon/0.zon"),
    ("3.zon content",
     f"cat {PROJECT}/lib/world/zon/3.zon"),
    ("attach 65515 in 3.trg (expect none)",
     f"grep 'attach 65515\\|detach 65515' {PROJECT}/lib/world/trg/3.trg || echo 'PASS: no attach/detach 65515'"),
    ("attach 415 in 3.trg (expect present)",
     f"grep 'attach 415\\|detach 415' {PROJECT}/lib/world/trg/3.trg | head -5"),
]

print()
results = []
for label, cmd in checks:
    try:
        out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT).decode('utf-8', errors='replace').strip()
    except subprocess.CalledProcessError as e:
        out = e.output.decode('utf-8', errors='replace').strip() if e.output else "(no output)"
    print(f"  [{label}]")
    print(f"    {out}")
    results.append((label, out))

print("\n=== Done ===")

# AGENTS.md

Condensed operational guidance for lightweight/local models working in this repo. `CLAUDE.md`
has the full detail (boot sequence, password storage, the 99999/`%hd` vnum audit, copyover vs
reboot) — read it only if a task specifically needs that depth.

## What this is

A text-based multiplayer RPG (MUD) set in the **Hunter x Hunter** universe, themed around
**Greed Island**. Heavily-modified fork of **TbaMUD** (CircleMUD derivative). C (~80 `.c` files
in `src/`), targeting Linux/WSL. A prior plan to rewrite in C# was **cancelled** — all work stays
in this C codebase, do not propose a migration.

## Build & verify

Compiles under **WSL/Linux** only (gcc + glibc `-lcrypt`), not native Windows.

```bash
cd src && make circle CFLAGS=-w   # binary lands in bin/circle
bin/circle 4000                  # run on default port from project root
```

**There is no test suite.** Verification = compile cleanly (`make circle CFLAGS=-w` succeeds);
in-game behavior is verified manually by the maintainer. Do not write socket/telnet test
scripts, and never commit credentials.

## Pre-commit checklist (project convention)

1. **Compile** (skip only if no `.c`/`.h` files changed): `cd src && make circle CFLAGS=-w`. From
   PowerShell: `wsl -e bash -c "make -C src circle CFLAGS=-w 2>&1"`.
2. Update **`changelog`** (`[Mmm DD YYYY] - Henque` entries at the top) describing what changed.
3. Update **`lib/text/news`** *only* if the change affects gameplay (new commands, mechanics,
   balance, in-game bug fixes) — never for admin-only changes (version bumps, client fixes, text
   corrections). `news` tone: describe *what* changed, no exact numbers (damage, %, timers,
   costs) — those go in `changelog` only.
4. Then commit.

## Architecture: `src/` (engine) vs `lib/` (game world & state)

`src/` is data-driven. Content — rooms, monsters, items, shops, quests, scripts — lives as
plain-text files in `lib/world/`, loaded at boot by `db.c`. **Editing world content rarely
requires recompiling.** Changing rules/mechanics means editing `src/` and recompiling.

`lib/world/` subdirs, one file per zone number: `wld/` (rooms), `mob/` (monsters), `obj/`
(items), `zon/` (zone reset tables), `shp/` (shops), `qst/` (quests), `trg/` (DG Script
triggers).

## Critical invariant: vnum ordering

Within each world file, entries **must be sorted by ascending vnum** — TbaMUD binary-searches
(`real_room`, `real_object`, `real_mobile`, `real_trigger`) to map a vnum to an array index.
Out-of-order vnums silently break lookups.

**`.trg` files are the dangerous case**: an out-of-order vnum makes `real_trigger()` return
NOTHING with **no SYSERR logged**. Symptoms: `stat obj <name>` shows `Triggers: None` despite a
`T <vnum>` line in the `.obj` file; `tstat <vnum>` says "That vnum does not exist." Check with
`grep '^#[0-9]' lib/world/trg/<zone>.trg` — if any vnum is lower than a preceding one, that's the
bug. When adding items/mobs/triggers to a world file, always insert in vnum order.

## Working with content files

- Keep entries vnum-sorted; ensure each record is properly `~`-terminated — a missing `~` or a
  stray `E`/`A`-keyword line is a common source of boot `SYSERR`s.
- **`.obj` files with ANSI colors (`@m`, `@g`, `@n`, etc.)**: the `Read` tool treats these as
  binary. Use `Grep` to read them. To write/replace, use WSL Python in **binary mode**
  (`open(path, 'wb')` / `open(path, 'rb')`) and keep all strings pure ASCII — opening in text
  mode truncates the file before raising an encoding error.
- Never hand-edit `lib/plrfiles`/`lib/plrobjs` — binary/ascii saves managed by the engine,
  gitignored.
- **MXP safety in `lib/text/`** (news, motd, greetings, info, etc.): avoid bare `<`/`>` — web MXP
  clients interpret them as tags. Use `==`, `[]`, or `""` instead.
- **Copyover vs reboot**: a copyover re-reads all `lib/` files from disk — sufficient for
  `lib/text/`, `lib/world/`, `lib/misc/` changes. A full shutdown/reboot is only needed after
  recompiling `src/`.

## Item type numbers (first field in the `.obj` property line)

| # | Constant | Notes |
|---|---|---|
| 3 | `ITEM_WAND` | charges in val1/val2 |
| 11 | `ITEM_WORN` | cosmetic slot item |
| 12 | `ITEM_OTHER` | generic, no special behavior |
| 15 | `ITEM_CONTAINER` | val0=capacity, val1=flags, val2=key vnum (-1 = none) |
| 16 | `ITEM_NOTE` | writable scroll |
| 19 | `ITEM_FOOD` | val0=nutrition (0=candy/no effect on hunger); fires `OTRIG_CONSUME` |
| 24 | `ITEM_CARD` | free/unrestricted card |
| 25 | `ITEM_SPELLCARD` | cast on `gain`, consumed |
| 26 | `ITEM_RESTRICTED` | numbered 000–099 collectible cards |

Wear-flag `a` = `ITEM_WEAR_TAKE` (can be picked up). Extra flags `ao` = NODROP + ANTI_OTHER.

## HxH / Greed Island customizations

- **Nen system**: 7 classes in `class.c` (Enhancer, Emitter, Conjurer, Transmuter, Manipulator,
  Specialist, Hunter), each with class-specific skills/progression.
- **Jajanken** (charged rock-paper-scissors attack) — `act.offensive.c`, gated by player flags
  (`PLR_JAJANKEN`, `AFF_ENHANCE`, `AFF_BOOST`).
- **Greed Island cards** — `ITEM_CARD`/`ITEM_SPELLCARD`/`ITEM_RESTRICTED` item types. The 100-card
  restricted set lives in `653.obj`/`654.obj`; some drive DG Script triggers.

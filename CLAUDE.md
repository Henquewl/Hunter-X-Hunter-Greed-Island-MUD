# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A text-based multiplayer RPG (MUD) set in the **Hunter x Hunter** universe, themed around the **Greed Island** arc. It is a heavily-modified fork of **TbaMUD 3.67** (a CircleMUD derivative). The codebase is C (~80 `.c` files in `src/`), targeting Linux/WSL. The current project version is in `VERSION`.

> A previous plan to rewrite the engine in C# was **cancelled**. All work happens in this C codebase — make targeted fixes and additions here, do not propose a migration.

## Build & run

This compiles and runs under **WSL/Linux** (gcc + glibc `-lcrypt`), not native Windows.

```bash
# Compile (binary lands in bin/circle). CFLAGS=-w silences the many legacy warnings.
cd src && make circle CFLAGS=-w

# Run on default port 4000 from the project root
bin/circle 4000

# Convenience launcher (auto-compiles if bin/circle is missing, then runs)
./start.sh            # from WSL
run.bat               # from Windows — shells into WSL and calls start.sh
```

- `make clean` removes `*.o` and `depend`. `make depend` regenerates header dependencies.
- The server is ready when the log prints `Boot db -- DONE`.
- Stop with `Ctrl+C`, or `pkill circle`.
- `autorun` / `autorun.sh` is the production loop that auto-reboots on crash and rotates logs; control it with sentinel files in the root (`.fastboot`, `.killscript`, `pause`).
- Connect with any MUD/telnet client to `localhost:4000` (MUSHclient/Mudlet recommended for ANSI color).

There is no unit-test framework. `test_quit.py` is an ad-hoc connection script, not a test suite. Verification = compile cleanly, boot the server, and confirm in-game behavior via a client.

## Pre-commit checklist (project convention)

Before every commit in this repo:
1. **Compile** (`cd src && make circle CFLAGS=-w`) and confirm it succeeds — there is no CI.
2. Update **`changelog`** and **`lib/text/news`** describing what changed (the changelog uses `[Mon DD YYYY] - Henque` entries at the top of the recent section).
3. Then commit.

## Architecture

### The two halves: `src/` (engine) and `lib/` (game world & state)

The C engine in `src/` is data-driven. Almost all *content* — rooms, monsters, items, shops, quests, scripts — lives as plain-text data files in `lib/world/`, loaded at boot by `db.c`. **Editing world content rarely requires recompiling.** Conversely, changing rules/mechanics means editing `src/` and recompiling.

`lib/world/` subdirectories, each keyed by zone number with one file per zone:
- `wld/` — rooms (`.wld`)
- `mob/` — monsters/NPCs (`.mob`)
- `obj/` — items (`.obj`)
- `zon/` — **zone reset tables** (`.zon`): what mobs/objects spawn where on each reset cycle
- `shp/` — shops (`.shp`)
- `qst/` — quests (`.qst`)
- `trg/` — **DG Script triggers** (`.trg`): the in-world scripting system

Other `lib/` state: `etc/` (runtime state — `last`, `time`, `plrmail`; gitignored), `plrfiles/` + `plrobjs/` (player saves — gitignored, server starts fresh), `text/` (login screens, MOTD, `news`, help files), `misc/`.

### Boot sequence

`comm.c` holds `main()` and the network/game loop. At startup it calls into `db.c`, which parses every `lib/world/*` file into in-memory arrays. **Critical invariant:** within each world file, entries must be sorted by ascending vnum — TbaMUD uses binary search (`real_room`, `real_object`, `real_mobile`) to map a *virtual* number (vnum, the stable content ID) to a *real* array index. Out-of-order vnums silently break lookups and produce `SYSERR: invalid vnum` on zone reset (see recent changelog fixes in zones 653/654). When adding items/mobs to a world file, insert them in vnum order.

### `src/` layout conventions

- `act.*.c` — player command handlers grouped by domain: `act.offensive.c` (combat commands incl. **Jajanken**), `act.movement.c`, `act.informative.c` (look/score/who), `act.item.c`, `act.comm.c`, `act.social.c`, `act.wizard.c` (immortal commands), `act.other.c`.
- `fight.c` — combat resolution; `magic.c` / `spell_parser.c` / `spells.c` — spell/skill casting; `class.c` — class definitions and per-class progression tables.
- `db.c` — world/player loading & saving; `handler.c` — object/char manipulation primitives; `utils.c`/`utils.h` — macros (the `GET_*` accessors live here).
- `structs.h` — core data structures and the bitvector flag `#define`s; `spells.h` — spell/skill numeric IDs; `constants.h`/`constants.c` — lookup tables.
- `*edit.c` (`redit`, `medit`, `oedit`, `zedit`, `aedit`, `cedit`, ...) — the in-game **OLC** (OnLine Creation) editors that read/write the `lib/world/*` files.
- `dg_*.c` — the **DG Scripts** engine that interprets `lib/world/trg/` triggers.
- `src/util/` — standalone maintenance utilities (player-file conversion, index rebuilders, `scheck` sanity checker), built via `src/util/Makefile`.

### HxH / Greed Island customizations layered on TbaMUD

These are the bespoke mechanics — when touching gameplay, expect logic spread across the generic engine files plus these hooks:
- **Nen system**: 7 classes defined in `class.c` (Enhancer, Emitter, Conjurer, Transmuter, Manipulator, Specialist, plus Hunter). Each has an ANSI color and class-specific skills/progression tables.
- **Jajanken** (rock-paper-scissors charged attack) — `act.offensive.c`, gated by player flags in `structs.h` (e.g. `PLR_JAJANKEN`, `AFF_ENHANCE`, `AFF_BOOST`).
- **Greed Island cards** — custom item types in `structs.h`: `ITEM_CARD` (24, unrestricted), `ITEM_SPELLCARD`, `ITEM_RESTRICTED` (26). `SPELL_LOCATE_CARD` (59) in `spells.h`. The 100-card restricted set lives in zone files `653.obj`/`654.obj`; some cards drive DG Script triggers (e.g. Corruption #1022, Compromise #1023). See `TODO-restricted-cards-world-spawn.md` for the outstanding world-spawn design.

## Working with content files

- Adding/editing rooms, mobs, items, shops, or scripts: prefer editing the `lib/world/*` text files (or use the in-game OLC editors), keep entries vnum-sorted, and ensure each record is properly `~`-terminated — a missing `~` or a stray `E`/`A`-keyword line is a common source of boot `SYSERR`s.
- Never hand-edit player files in `lib/plrfiles`/`lib/plrobjs`; they are binary/ascii saves managed by the engine and are gitignored.

## The Greed Island card system (detailed)

The card mechanic is the heart of the game and its logic is spread across a few hooks. Reference map:

**Item types** (`structs.h`): `ITEM_CARD` 24 (free/unrestricted card), `ITEM_SPELLCARD` 25 (cast on use, consumed), `ITEM_RESTRICTED` 26 (the numbered 000–099 collectible cards). `IS_CARD(obj)` (`utils.h`) = any of those three.

**vnum mappings**
- Restricted: **card vnum ∈ 65300–65399 ↔ physical item = card + 100 (65400–65499)**. The restricted card's `GET_OBJ_RENT` holds the *global copy limit* (the "-N" in `SS-1`, `A-17`).
- Free cards (type 24): the physical-item vnum is stored in the card's **`GET_OBJ_RENT`** (e.g. free cards in `lib/world/obj/400.obj`, their items in `401.obj`). A freshly-created free card stamps the source item's vnum into `GET_OBJ_RENT`.
- Voucher: a type-24 card at vnum 65535 flagged `ITEM_QUEST`; appears when a restricted card's global limit is reached.

**The conversion engine**: `make_card(ch, obj, show)` in `act.item.c` is a single **bidirectional** function — it decides direction from the object's type/vnum (item→card creates/loads a card; card→item reads the stored target). `show` only controls the room message. Returns 1 on success, 0 if nothing happened. `ITEM_NOGAIN` (extra flag) marks "already converted, cannot convert again" and is never cleared.

**Player commands**
- `gain` (`do_gain`): bidirectional convenience verb — card→item, item→card, and **casting spell cards** (needs the book/`PLR_BOOK` active via the `book` command).
- `change` (`do_change`): **item→card only**, for ordinary items. This is the manual path for common loot.
- `book` (`do_book`, `act.movement.c`): summons the binder container (vnum **3203**) via the ring (3202); 45 free slots + 100 restricted (deduped by vnum).

**Auto-conversion rule (current)**: on pickup/give-from-NPC/steal, an item auto-converts to a card **only if it becomes a restricted card** — gated by `GET_OBJ_VNUM(obj) > 65300 && != 65535` at the call sites in `act.item.c` (`perform_get_from_room`, `perform_get_from_container`, `perform_give`) and `act.other.c` (`do_steal`). Ordinary items never auto-convert; the player uses `change`.

**Timed auto-reversion**: `second_update` (`limits.c`) decrements each loose card's `GET_OBJ_TIMER` (set to 62 on creation / on binder put-get); at 0 a card outside binder 3203 reverts to its item (and is flagged `ITEM_NOGAIN`). Cards inside the binder are exempt — that's the incentive to store them.

**Player-facing text to keep in sync** when card behavior changes: NPC **Eta** (mob 1401) triggers `1419`/`1496` in `lib/world/trg/14.trg`; the notice sign obj **3298** in `lib/world/obj/32.obj`; help entries `GAIN`, `CHANGE`, `CARDS`, `BOOK` in `lib/text/help/help.hlp`; and `lib/text/info`.

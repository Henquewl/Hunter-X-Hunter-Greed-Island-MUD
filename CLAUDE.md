# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A text-based multiplayer RPG (MUD) set in the **Hunter x Hunter** universe, themed around the **Greed Island** arc. It is a heavily-modified fork of **TbaMUD** (a CircleMUD derivative), with upstream bug fixes synced through the **2025** TbaMUD release. The codebase is C (~80 `.c` files in `src/`), targeting Linux/WSL. The current project version is in `VERSION`.

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

There is no unit-test framework or test scripts. Verification = compile cleanly; in-game behavior is verified manually by the maintainer (do not write socket/telnet test scripts, and never commit credentials).

## Pre-commit checklist (project convention)

Before every commit in this repo:
1. **Compile** (`cd src && make circle CFLAGS=-w`) and confirm it succeeds — there is no CI. From PowerShell/Windows use: `wsl -e bash -c "cd '/mnt/c/Users/henqu/source/repos/Hunter-X-Hunter-Greed-Island-MUD/src' && make circle CFLAGS=-w 2>&1"`. Skip this step only when no `.c`/`.h` files were changed.
2. Update **`changelog`** and **`lib/text/news`** describing what changed (the changelog uses `[Mon DD YYYY] - Henque` entries at the top of the recent section).
   - **`lib/text/news`** — only if the change affects gameplay (new commands, mechanics, balance, in-game bug fixes). Administrative changes (version bumps, client fixes, text corrections) go in `changelog` only, never in `news`.
   - **`lib/text/news` tone** — describe *what* changed or that something was rebalanced; no exact numbers (damage values, percentages, timers, stamina costs, etc.). Exact numbers belong in `changelog`.
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

### Password storage

Passwords are SHA-512 crypt hashes (`$6$...`) via `hash_password()` / `password_matches()` / `password_needs_rehash()` in `players.c` (prototypes in `db.h`). Legacy plaintext pfiles still authenticate and are rehashed+saved on first successful login. The stored hash lives in `passwd[MAX_PWD_HASH_LENGTH+1]` (`structs.h`); `MAX_PWD_LENGTH` (30) remains the limit for the *typed* password. Gotchas: `src/conf.h` is autoconf-generated — re-running `./configure` would revert `CIRCLE_CRYPT`/`HAVE_CRYPT_H` to undefined and silently bring plaintext storage back; never re-run configure without re-checking those two defines. Never print/echo passwords (see `act.wizard.c` set password). Player files and `lib/plrfiles/index` are gitignored; a fresh clone boots with an empty player table and the first character created becomes an Implementor (`db.c:3483`).

### Boot sequence

`comm.c` holds `main()` and the network/game loop. At startup it calls into `db.c`, which parses every `lib/world/*` file into in-memory arrays. **Critical invariant:** within each world file, entries must be sorted by ascending vnum — TbaMUD uses binary search (`real_room`, `real_object`, `real_mobile`, `real_trigger`) to map a *virtual* number (vnum, the stable content ID) to a *real* array index. Out-of-order vnums silently break lookups and produce `SYSERR: invalid vnum` on zone reset (see recent changelog fixes in zones 653/654). When adding items/mobs/triggers to a world file, insert them in vnum order.

**Trigger ordering failure mode is silent:** for `.trg` files specifically, an out-of-order vnum causes `real_trigger()` to return NOTHING with no SYSERR logged. Symptoms: `stat obj <name>` shows `Triggers: None` despite `T <vnum>` being in the `.obj` file; `tstat <vnum>` says "That vnum does not exist." Diagnosis: `grep '^#[0-9]' lib/world/trg/<zone>.trg` — if any vnum is lower than a preceding one, the binary search's early-exit check (`trig_index[top]->vnum < target`) silently discards all lookups for vnums above the out-of-order entry.

**The 99999 record cap (`db.c:1104`) and `%hd` vnum reads:** vnums are capped at 65534 today (`IDXTYPE` is `ush_int`; 65535 = `NOWHERE`). Two silent ceilings sit just below any plan to use vnums above ~65k (e.g. the 256x256 world-map zone). (1) `discrete_load()` has `if (nr >= 99999) return;` — the moment a `#NNNNN` record number reaches 99999 it abandons the rest of the file **with no SYSERR**, so no record at/above that loads. (2) Five `sscanf`/`read_line` calls parse vnums (or vnum-typed fields) with `%hd`, which writes only 16 bits: `db.c:2063` (`Z.number`), `db.c:2086/2089/2094` (`Z.bot`/`Z.top` — truncation here trips the `bot > top` boot abort at `db.c:2117`), and `shop.c:1210` (`SHOP_KEEPER`, via `read_line`'s `%hd` at `shop.c:1057`). After widening `IDXTYPE` to 32-bit, all five are UB (uninitialized upper bytes) regardless of vnum magnitude. The rest of the boot/runtime path is vnum-width-clean: record reads use `int`+`%d`, exit targets (`setup_dir`, `int t[5]`) and zone reset args (`reset_com.arg*` are `int`) are wide, there is **no zone-number cap on the boot path** (only `genzon.c:59`'s 655 cap, which is OLC-only), and every live struct holds vnums as `IDXTYPE`/`int`. See `worldmap-plan.md` for the full audit.

### HxH / Greed Island customizations layered on TbaMUD

These are the bespoke mechanics — when touching gameplay, expect logic spread across the generic engine files plus these hooks:
- **Nen system**: 7 classes defined in `class.c` (Enhancer, Emitter, Conjurer, Transmuter, Manipulator, Specialist, plus Hunter). Each has an ANSI color and class-specific skills/progression tables.
- **Jajanken** (rock-paper-scissors charged attack) — `act.offensive.c`, gated by player flags in `structs.h` (e.g. `PLR_JAJANKEN`, `AFF_ENHANCE`, `AFF_BOOST`).
- **Greed Island cards** — custom item types in `structs.h`: `ITEM_CARD` (24, unrestricted), `ITEM_SPELLCARD`, `ITEM_RESTRICTED` (26). `SPELL_LOCATE_CARD` (59) in `spells.h`. The 100-card restricted set lives in zone files `653.obj`/`654.obj`; some cards drive DG Script triggers (e.g. Corruption #1022, Compromise #1023). See `TODO-restricted-cards-world-spawn.md` for the outstanding world-spawn design.

## Working with content files

- Adding/editing rooms, mobs, items, shops, or scripts: prefer editing the `lib/world/*` text files (or use the in-game OLC editors), keep entries vnum-sorted, and ensure each record is properly `~`-terminated — a missing `~` or a stray `E`/`A`-keyword line is a common source of boot `SYSERR`s.
- Never hand-edit player files in `lib/plrfiles`/`lib/plrobjs`; they are binary/ascii saves managed by the engine and are gitignored.
- **MXP safety in `lib/text/`**: web MXP clients (e.g. mudportal.com) interpret `<` as the start of a tag. In files served through the pager (`news`, `motd`, `greetings`, `info`, etc.), avoid bare `<` and `>`. Use `==`, `[]`, or `""` instead. The prompt's `<100%>` is safe because it's sent on a locked MXP line; paged text files are not.
- **Copyover vs reboot**: a copyover re-execs the binary and re-reads all `lib/` files from disk — sufficient for changes to `lib/text/`, `lib/world/`, `lib/misc/`. A full shutdown/reboot is only needed after recompiling `src/`.

### Item type numbers (first field in the `.obj` property line)

| # | Constant | Notes |
|---|---|---|
| 3 | `ITEM_WAND` | charges in val1/val2 |
| 11 | `ITEM_WORN` | cosmetic slot item |
| 12 | `ITEM_OTHER` | generic, no special behavior |
| 15 | `ITEM_CONTAINER` | val0=capacity, val1=flags, val2=key vnum (-1 = none) |
| 16 | `ITEM_NOTE` | writable scroll |
| 19 | `ITEM_FOOD` | val0=nutrition (0=candy/no effect on hunger); fires `OTRIG_CONSUME` trigger |
| 24 | `ITEM_CARD` | free/unrestricted card |
| 25 | `ITEM_SPELLCARD` | cast on `gain`, consumed |
| 26 | `ITEM_RESTRICTED` | numbered 000–099 collectible cards |

Wear-flag `a` in the sixth field = `ITEM_WEAR_TAKE` (can be picked up). Extra flags `ao` = NODROP + ANTI_OTHER.

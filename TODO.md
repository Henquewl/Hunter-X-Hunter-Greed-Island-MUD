# TODO — Greed Island MUD

## Status summary

| System | Status |
|---|---|
| All 100 restricted cards (canon names, effects) | Done |
| Fickle card pattern (50+ cards auto-store in binder) | Done |
| NPC death → dynamic card drop | Done |
| Ruler's Invitation end-game flow (99 cards → owl delivery) | Done |
| Worldmap — 256×256 grid, zone 1000, `gen_worldmap.py` | Done |
| Map display — level-scaled, tabular legend, auto-regen on boot | Done |

---

## Next milestone: v1.00 — world content

The engine and card systems are complete. What remains before v1.00 is **populating the world**:

### Priority 1 — World areas & cities
- Replace generic TbaMUD starter zones with canonical Greed Island locations
- **Antokiba** (already has zone 410 stub — expand with shops, NPCs, quests)
- **Masadora** — magic shops, spell card vendors, arena
- **Rabicuta** — forest area, creature cards
- **Limeiro Castle** — end-game dungeon; NPC that receives the Ruler's Invitation letter and grants Ruler's Blessing (#0, card 65300)
- Other canon locations from the manga arc (port towns, wilderness outposts, etc.)
- Update `greed_island.txt` for each new area and run `gen_worldmap.py`; link `@` city entrances via `CITY_LINKS` in the script

### Priority 2 — Mobs & NPCs
- Populate each city zone with canonical NPCs (shopkeepers, questgivers, guards)
- Create encounter mobs appropriate to each terrain type (field, forest, hills, mountain)
- Assign mob drops that cover the remaining restricted cards not yet sourced from any mob
- Ensure all 99 cards (65301–65399) have at least one dedicated in-world source

### Priority 3 — Pending live tests
The following cards have untested runtime logic — verify in-game before v1.00:
- **#87 Shield of Faith** (65487) — WEAR/REMOVE triggers protecting from AS cards
- **#89 Tax Collector's Gauntlet** (65489) — `levy` trigger + `same_group` DG field
- **#82 Staff of Judgment** (65482) — `SPELL_JUDGMENT` alignment-based rebound
- **#96 Clairvoyant Snake** (65496) — `feed` trigger + `cardcount` DG field
- **#88 Eternal Hammer** and **#94 Bandit's Blade** — both need a live PvP test
- **#4 Hot Springs, #6 Liquor Spring, #8 Mystery Pond, #9 Tree of Plenty** — DG timer triggers

### Priority 4 — Deferred / low priority
- **Vnum migration** — renumber restricted card vnums (653xx/654xx) into zones 0/1; not blocking v1.00
- **Limeiro Castle dungeon** interior rooms and boss encounter
- NPC mob spawns for #16 Night Shift Dwarves and #26 Fairy King's Advice (currently fickle)

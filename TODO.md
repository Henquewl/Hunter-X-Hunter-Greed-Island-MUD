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
| Worldmap access points (S/L/P/C/?) — `enter` command, safezones, per-type colors | Done |
| Fly mechanic — auto-flight engine, `%ofly%`, PLR_AUTOFLIGHT, transport card triggers | Done |
| Vnum migration — restricted cards renumbered from 653xx/654xx into zones 0-3 | Done |

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
- For each new city/area: place the access-point char (`S`/`L`/`P`/`C`/`?`) in `greed_island.txt` at the desired map coordinate, then add the matching entry to `ENTRY_LINKS` in `tools/gen_worldmap.py` — the generator auto-reruns on each reboot
- **After each city is placed on the worldmap**: fill in the `map_tile` vnum for that city in `fly_destinations[]` in `src/act.movement.c` so transport cards fly there correctly

### Priority 2 — Mobs & NPCs
- Populate each city zone with canonical NPCs (shopkeepers, questgivers, guards)
- Create encounter mobs appropriate to each terrain type (field, forest, hills, mountain)
- Assign mob drops that cover the remaining restricted cards not yet sourced from any mob
- Ensure all 99 cards (65301–65399) have at least one dedicated in-world source
- See `TODO-restricted-cards-world-spawn.md` for the full card-by-card placement plan

### Priority 3 — Pending live tests
The following cards have untested runtime logic — verify in-game before v1.00:
- **#87 Shield of Faith** (65487) — WEAR/REMOVE triggers protecting from AS cards
- **#89 Tax Collector's Gauntlet** (65489) — `levy` trigger + `same_group` DG field
- **#82 Staff of Judgment** (65482) — `SPELL_JUDGMENT` alignment-based rebound
- **#96 Clairvoyant Snake** (65496) — `feed` trigger + `cardcount` DG field
- **#88 Eternal Hammer** and **#94 Bandit's Blade** — both need a live PvP test
- **#4 Hot Springs, #6 Liquor Spring, #8 Mystery Pond, #9 Tree of Plenty** — DG timer triggers

### Priority 4 — Deferred / low priority (conceptual only, not yet implemented)
- **Limeiro Castle** — Limeiro is a citadel with an inner castle at its center. Concept (no boss encounter):
  - NPCs across the citadel get a trigger that celebrates the arrival of the Greed Island champion
  - At the inner castle's gate, a guard NPC only allows entry to the player holding the Ruler's Invitation
  - The room beyond the gate leads into a zone restricted to Immortals only

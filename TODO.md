# TODO — Restricted-card item compliance

Audit (Jun 13 2026) of the 100 restricted cards (`lib/world/obj/653.obj`, vnums 65300–65399)
vs. their physical items (`654.obj`, item = card+100) and triggers (`654.trg`, `653.trg`,
`32.trg`).

Card↔item mapping: restricted card `653xx` ↔ physical item `654xx` (card vnum + 100).
`make_card` (`src/act.item.c`) loads `card_vnum+100`; a missing item makes the transform a
safe no-op (no SYSERR).

---

## DONE — implemented Jun 13 2026 (data-only batch + Rebirth)

**Passive worn affects** (obj affect-bitvector, last 4 tokens of the flag line; letter =
`'a'+AFF_position`):
- Aqua Guard (65438) → `s` AFF_SCUBA (breathe underwater)
- Glowing Wings (65454) → `r` AFF_FLYING
- Invisible Cloak (65460) → `c` AFF_INVISIBLE
- Dragon's Eye (65479) → `eg` AFF_DETECT_INVIS + AFF_SENSE_LIFE (held)
- Levitation Stone (65480) → **already had `hqr`** (FLY+WATERWALK+NOTRACK); audit error fixed.

**DG-trigger effects** (new triggers in `654.trg`, `T` lines in `654.obj`):
- Cane of Healing (65419): `tap` → heals wielder.
- Fairy's Breath (65448): `inhale` → AFF_FLYING 60t, consumed.
- Explosive Marbles (65420): `throw marbles` → AoE damage to others, consumed.
- Flame Guitar (65451): `play guitar` → AoE fire damage to others.
- Sleep Clock (65455): `ring clock` → AoE SLEEP on others.
- Flute of Confusion (65492): `play flute` → AoE CURSE on others.
- Haze Smoke Bomb (65493): `throw bomb` → AoE BLIND on others, consumed.
- Witch's Wisdom Potion (65472): retyped to FOOD; eat → INT +5 / 240t.
- Elixir of Life (65478): retyped to FOOD; eat → full heal.

**New B2 consumable/NPC items** (created in `654.obj` + triggers):
- Witch's Love Potion (65464), Rejuvenation (65465), Diet (65466); Doyen's Growth Pills
  (65467), Hair Restorer (65469) — FOOD with consume effects.
- Mad Scientist's Pheromones (65471): `spray` → CHA +3 / 60t.
- Gold Dust Girl (65446): card-item spawns existing mob 65446 on gain (trigger 65350).

**Fledgling "magic egg" series (canon #37-45: cards 65337-65345 / items 65437-65445)** —
Athlete, Artist, Politician, Musician, Pilot, Novelist, Gambler, Actor, CEO. Card → egg item;
HOLD in hand 3 continuous game hours → hatches → +1 permanent random attribute from a
profession pool. C in `limits.c` (`egg_incubation_update`/`hatch_fledgling_egg`, table-driven).
**Architectural note:** preferred future form is a trigger that evokes a C primitive rather than
the standalone point_update() scan (see memory `trigger-evokes-code-pattern`). *Needs a live
hold-to-hatch test.*

**#35 Chameleon Cat (S-6) / #36 Recycling Room (S-10)** — added as canon plain collectibles
(cards 65335/65336, items 65435/65436). Their canon effects (transform-into-animal; repair-in-
24h) are NOT implemented yet — TODO.

**Rebirth — REMOVED** (non-canon invention); fight.c die() hook deleted. Face Lift Machine also
removed. Their slots #44/#45 are now Fledgling Actor/CEO.

---

## Milestone v1.00 — release criteria

Version 1.00 requires that **all 99 restricted cards** (except #0 Ruler's Blessing, which is
the end-game completion reward) be **obtainable in the world** — through a dedicated mob drop,
room placement, trigger, or any other intentional source. In addition, the game world must be
**reworked to reflect canonical Greed Island**: zones, cities, and NPCs consistent with the
manga arc (Antokiba, Masadora, Rabicuta, Limeiro castle, etc.), replacing the generic TbaMUD
content inherited from CircleMUD.

Specific prerequisites before declaring v1.00:
1. All 99 cards (65301–65399) have at least one dedicated in-world source of acquisition
2. World rework — main Greed Island zones with canonical names, themes, and NPCs
3. Ruler's Blessing (#0) obtainable only as a reward for completing the full 99-card set
4. All items marked *Needs a live test* above verified in live gameplay

---

## DEFERRED — still open (need new mobs, new rooms, or more C)

**Needs new NPC mobs** (card has no item; spawn a creature on gain — Archangel/65350 pattern):
Night Shift Dwarves (65326), Fickle Genie (65315), Fairy King's Advice (65316).
(Gold Dust Girl already done — its mob existed. Fledgling Politician is now an egg card.)

**Needs new rooms / a world-build** — terrain/location cards (card-only today):
Ruler's Blessing castle (65300), Patch of Forest (65301), Patch of Shore / Poseidon's Cavern
(65302), Skin Care Hot Springs (65304), Spirited Away Hollow (65305), Liquor Spring (65306),
Mystery Pond (65308), Tree of Plenty (65309). Decision was to defer; they need destination
rooms/zones (653/654.wld are empty placeholders).

**Needs more C / complex design:**
- Fickle Genie 3-wishes (65315) — bespoke wish mechanic, no design yet.

**Flavor-only collectibles** (effect not meaningfully modelable; acceptable as-is): Pregnancy
Stones, Dress of Memory, Paper Doll, Book of VIP Parties, Master Mime, Echo Recorder,
Face-Lift Machine, Miracle Seed, Sand Ship, Turtle Mansion, Mr. Billionaire, Returned
Postcard, Loving Slave, Pretty Little Devil, Crystal Ball, etc.

---

## DONE — all 100 cards now canon (Jun 13 2026)
The 45 invented slots were replaced with their canon card (name + rank + effect text from the
saved fandom page); cards are type-26 restricted, items are takeable collectibles. The old
invented triggers/affects were removed (triggers 65419/65420/65448/65451/65455/65472/65478/
65492/65493 dropped from 654.trg; fly/invis/detect affects cleared). Two canon effects were
implemented with existing mechanics:
- **#95 Secret Cape** — "Blackout Curtain" concealment → passive AFF_INVISIBLE (obj affect `c`).
- **#70 Mad Scientist's Steroids** — FOOD; consume trigger 65470 grants a temporary +2 STR.
- **#14 Connection Severing Scissors** — ITEM_WAND (item 65414, 99 charges, SPELL_TELEPORT).
- **#34 Universal Survey** — ITEM_NOTE (item 65434); writable roleplay survey booklet.
- **#36 Recycling Room** — `gain` on card 65336 repairs all worn/carried equipment instantly via `repair_all_player_items()` in act.item.c; card is consumed.
- **#56 Perfect Memory Studio** — ITEM_NOTE (item 65456); player's name injected into photo description at creation time via make_card() hook in act.item.c.
- **#87 Shield of Faith** — WEAR/REMOVE triggers (#65499/#65500 in 654.trg) set/clear a `shield_faith` remote variable; guard added to Return (#1009), Railguide (#1012), Drift (#1016), Collision (#1017) triggers in 10.trg. Protects wearer and followers. *Needs a live test.*
- **#89 Tax Collector's Gauntlet** — `levy` command trigger (#65501 in 654.trg): consumes one random restricted card from wielder's own binder as tribute, then steals a card from every non-allied PC in the room bypassing defensive spells. `same_group` DG field added to dg_variables.c. *Needs a live test.*

### Remaining EFFECT work (cards are canon, but their effect isn't mechanically modeled yet)
Most canon cards are life-sim/social/flavor with no existing mechanic, so they sit as canon
collectibles (acceptable). Candidates that COULD be done later with existing patterns:
- **Staff of Judgment (#82)** — DONE (custom spell + wand): item 65482 is now an ITEM_WAND
  (99 charges) that casts the new spell **SPELL_JUDGMENT (61)** (`spell_judgment` in spells.c,
  registered in spell_parser.c). Effect: a Nen (mana) calamity equal to the |alignment gap|,
  striking the worse-aligned party — the target if it is more EVIL than the wielder, or the
  wielder if the target is more righteous (rebound). Works on players and non-protected NPCs.
  `use staff <target>`. *Needs a live test.*

- **Spell-card-driven weapons — DEFERRED (need a C primitive).** IMPORTANT: in restricted-card
  text, "spell" = a Greed Island **spell card** (`ITEM_SPELLCARD`, the 1000–1040 set in
  `lib/world/obj/10.obj`: Leave=1014, Clairvoyance=1015, Blackout Curtain=1025, Magnetic
  Force=1005, Collision=1017, Rob=1021, Levy=1018, …), NOT a player magic spell. Spell-card
  effects live in C (the big `switch` in `do_gain`, `act.item.c`) plus some DG triggers, so
  another item cannot invoke "spell-card X on target Y" from a trigger today. To do these
  faithfully, add a small C primitive (e.g. apply_spellcard_effect(ch, victim, card_vnum)) and
  call it from the weapon's trigger (trigger-evokes-code):
  - **#86 Quiver of Frustration — DONE** (pure DG): item 65486 holds 10 arrows (val0);
    `loose` (trigger 65486) spends one arrow and loads a Leave card (1014) — works regardless
    of binder Leaves. Empty when arrows run out.
  - **#96 Clairvoyant Snake — DONE**: card-item 65496 summons a follower snake **mob 65496**
    (1 hp; spawn trigger 65496 + follow trigger 65497 using `mfollow`). `feed <snake>` (trigger
    65498) once per game day (remote `lastfed` vs `%time.day%`) spits up a Clairvoyance card
    (1015), unless the global limit is reached — checked with a NEW read-only DG field
    `%actor.cardcount(<vnum>)%` (returns `obj_index[].number`) added in dg_variables.c
    (trigger-evokes-code). *Needs live test (runtime-only logic).*
  - **#88 Eternal Hammer — DONE** (Jun 18 2026). `eternal_hammer_proc()` in fight.c: on a
    successful PvP hit while wielding 65488 with a free hold-hand and an AS card in the binder,
    draws a random AS card, equips it, ensures PLR_BOOK, calls `gain <victim>` so the card's
    own DG trigger fires and consumes it. AS card pool: 1006/1007/1008/1021/1022/1023/1027/
    1028/1029/1033. Paladin's Necklace (65484) makes victim immune. **NEEDS A PvP LIVE TEST.**
  - **#94 Bandit's Blade — DONE** (Jun 18 2026). `bandit_blade_proc()` in fight.c: 20% chance
    on each successful PvP hit to spawn a random AS card from thin air (no binder consumed)
    and cast it at the victim via `gain <victim>`. Stats: 1d4 pierce, weight 5, −2 hitroll,
    −1 DEX. Paladin's Necklace (65484) grants immunity. **NEEDS A PvP LIVE TEST.**
  - Also relevant: **#95 Secret Cape** approximates "Blackout Curtain" (1025) with
    AFF_INVISIBLE; revisit if the real Blackout Curtain effect differs.
- **NPC/pet cards** (need new mobs): #47 Sleeping Girl, #48 Aromatherapy Girl, #49 Miniature
  Mermaid, #50 Miniature Dino, #51 Miniature Dragon, #99 Panda Maid, etc.
- **Consumable flavor with a stat angle** could be added if desired (e.g. #33 Hormone Cookies).

Below is the historical invention→canon mapping that this batch applied:

| # | current (invention) | canon → | rank |
|---|---|---|---|
| 18 | Invisible Stalker | Imp's Wink | A-18 |
| 19 | Cane of Healing | Poltergeist Pillow | A-13 |
| 20 | Explosive Marbles | Mood Clock | B-30 |
| 21 | Cutter | X-Ray Goggles | B-27 |
| 22 | Dress of Memory | Toraemon | A-22 |
| 23 | Paper Doll | Tome of a Thousand Tales | B-30 |
| 24 | Book of V.I.P. Parties | Hypothetical T.V. | A-20 |
| 27 | Dino Basket | Book of V.I.P Passes | B-25 |
| 28 | Lottery Rose | Capricious Remote | B-27 |
| 29 | Loving Slave | Pre-Order Vouchers | A-20 |
| 30 | Pretty Little Devil | Favor Cushion | B-21 |
| 32 | Diamond Sword | Parrot Candy | B-30 |
| 33 | Dragon's Jaw | Hormone Cookies | S-13 |
| 34 | Time-Stopping Watch | Universal Survey | B-30 |
| 47 | Wind Chime | Sleeping Girl | A-11 |
| 48 | Fairy's Breath | Aromatherapy Girl | A-15 |
| 49 | Aura Amplifier | Miniature Mermaid | A-23 |
| 50 | Miracle Seed | Miniature Dino | A-11 |
| 51 | Flame Guitar | Miniature Dragon | S-10 |
| 52 | Sand Ship | Pearl Locusts | B-30 |
| 53 | Crystal Ball | King White Stag Beetle | A-30 |
| 54 | Glowing Wings | Millennium Butterfly | A-25 |
| 55 | Sleep Clock | Revenge Shop | A-20 |
| 57 | Worm Snake | Hideout Realtor | A-11 |
| 58 | Mr. Billionaire | Secrets Video Rental | A-13 |
| 59 | Secret Stash | Instant Foreign Language School | A-20 |
| 60 | Invisible Cloak | Long Lost Delivery | B-30 |
| 63 | Binding Snake | Virtual Restaurant | B-30 |
| 70 | Rainbow Diamond | Mad Scientist's Steroids | A-16 |
| 72 | Witch's Wisdom Potion | Mad Scientist's Plastic Surgery | A-15 |
| 76 | Transmutation Stone | Roaming Ruby | B-30 |
| 77 | Philosopher's Stone | Beauty Magnet Emerald | S-10 |
| 78 | Elixir of Life | Lonely Sapphire | B-30 |
| 79 | Dragon's Eye | Rainbow Diamond | A-20 |
| 82 | Comet Stone | Staff of Judgment | A-15 |
| 86 | Heretic's Axe | Quiver of Frustration | A-11 |
| 88 | Iron Boots | Eternal Hammer | A-15 |
| 91 | Sniper Rifle | Plastic King | A-20 |
| 92 | Flute of Confusion | Swap Ticket | S-7 |
| 93 | Haze Smoke Bomb | Book of Life | B-28 |
| 95 | Armor of Zeno | Secret Cape | A-20 |
| 96 | Thunderbolt Disk | Clairvoyant Snake | A-12 |
| 97 | Razor Wind Turban | 3-D Camera | A-20 |
| 98 | Turtle Claw Gloves | Silver Dog | S-8 |
| 99 | Three-Star Hunter License | Panda Maid | S-6 |

Already-canon (no change): #0-17, 25, 26, 31, 35-46, 56, 61, 62, 64-69, 71, 73, 75, 80, 81,
83, 84, 85, 87, 89, 90, 94. Safe data fixes done Jun 13: #66 renamed to "Witch's Diet Pills",
#9 rank S-20→S-10, #74 rank A-15→A-11.

## New TODOs — future mechanics (Jun 2026)

**PvP Alignment Penalty System**: PvP actions (attacking, stealing, using AS cards) should
reduce the aggressor's alignment. Players with very evil alignment should face progressive
restrictions (e.g., barred from certain zones, hostile NPCs, stat penalties). Implement in
`fight.c` / `act.offensive.c` with an alignment penalty table per action type.

**NPCs defeated → cards**: when a mob is killed, there is a chance to transform its corpse
(or drop directly) into a Greed Island card related to that mob type. Requires a mob_vnum →
card_vnum mapping table (in `limits.c` or a new hook in `fight.c` die() handler).

---

## Reference — functional cards that predated this work
Breath of Archangel (65317→65417, trg 3265 spawns healing mob), Risky Dice (65425), Vending
Check-Up (65461 scanner), Sword of Truth (65483), Paladin's Necklace (65484), Doyen's Virility
Pills (65468), Pitcher of Eternal Water (65403), containers (Secret Stash 65459, Dino Basket
65427), and well-statted weapons/armor (Diamond Sword, Dragon's Jaw, Armor of Zeno, …).

# TODO: Restricted Cards — World Spawning

Cards and their physical items need a way to reach players.
The possible methods are:

- **Mob drop** — mob carries the card/item (`G` line in `.zon`); drops on kill
- **Mob equip** — mob wears/holds the item (`E`/`P` line in `.zon`); drops on kill
- **Room object** — item placed directly in a room (`O` line in `.zon`)
- **Trigger load** — `%load% obj VNUM` inside a DG Script
- **Mob transformation** — a special mob turns into the card/item on greet or death

> **Note on the existing random mechanism**
> Zone 32 (Greed Island loot boxes) has a 1-in-100 chance to give any card (1–99)
> as a rare reward. Zone 120 has a lottery that gives a card from the 3–17 range.
> These exist but are too rare to be the primary source for most cards.

---

## Cards That Already Have a World Presence

These are covered and do NOT need action:

| Card | Name | How obtained |
|------|------|-------------|
| #7  | Pregnancy Stones S-10          | Physical item given by mob in zone 65 |
| #25 | Risky Dice B-30                | Card dropped from loot boxes (zone 32) |
| #26 | Night Shift Dwarves A-20       | Trigger: near-death mob drops card |
| #31 | Returned Postcard from the Dead S-13 | Physical item placed in room (zone 31) |
| #39 | Fledgling Politician B-30      | Mob drop, zone 31 |
| #46 | Gold Dust Girl A-13            | Special mob (zone 9) transforms into card on greet |
| #56 | Perfect Memory Studio B-25     | Mob drop, zone 279 |
| #61 | Vending Check-Up A-20          | Physical item placed in room (zone 65) |
| #62 | Club "You Rule" B-20           | Mob drop, zone 279 |
| #64 | Witch's Love Potion B-30       | Mob drop, zone 279 |
| #68 | Doyen's Virility Pills A-20    | Physical item placed in room (zone 65) |
| #71 | Mad Scientist's Pheromones A-20 | Mob drop, zone 279 |
| #73 | Night Jade A-15                | Physical item placed in room (zone 36) |
| #74 | Sage's Aquamarine A-15         | Physical item placed in room (zone 36) |
| #75 | Wild Luck Alexandrite A-20     | Card and item loaded by trigger (zone 401) |
| #80 | Levitation Stone S-7           | Physical item on mob (zone 79) |
| #81 | Blue Planet SS-5               | Physical item given by mob (zone 410) |
| #83 | Sword of Truth B-22            | Physical item on mob (zone 79) |
| #84 | Paladin's Necklace D-60        | Physical item on mob (zone 186) |
| #87 | Shield of Faith S-15           | Physical item on mob (zone 9) |
| #89 | Tax Collector's Gauntlet A-20  | Physical item on mob (zone 31) |
| #90 | Memory Helmet A-20             | Physical item on mob (zone 79) |
| #94 | Bandit's Blade S-10            | Physical item on mobs (zones 79, 279) |

**Also covered via special mechanics (not needing regular placement):**
- #0 Ruler's Blessing SS-1 — completion reward (trigger says "bring all cards to Limeiro castle gate"); intentionally has no spawn
- #1 Patch of Forest SS-3 — trigger marks it "too valuable to use"; needs clarification on intended obtain path
- #2 Patch of Shore SS-3 — housing/elevator key; obtained through the housing system

---

## Cards That Need World Spawning

### ORIGINAL CARDS — missing placement

These existed before the card expansion but have no dedicated mob or room that provides them.
They can only reach players through the rare random event in zone 32 (1% chance).

| Card | Name | Grade | Suggested placement |
|------|------|-------|---------------------|
| #4  | Skin Care Hot Springs    | A-15  | Spa/inn mob in a relaxation area; drop from NPC healer |
| #5  | Spirited Away Hollow     | S-8   | Hidden forest mob; rare drop from forest guardian |
| #6  | Liquor Spring            | A-15  | Tavern mob or bartender NPC |
| #8  | Mystery Pond             | S-10  | Rare drop from aquatic mob; or hidden room object near water area |
| #9  | Tree of Plenty           | S-20  | Boss mob in a garden/nature zone; very rare (high-grade) |
| #10 | Golden Guidebook         | A-20  | Library mob, shop keeper, or scholar NPC |
| #11 | Golden Scales            | B-30  | Merchant or judge NPC mob drop |
| #12 | Golden Dictionary        | S-10  | Rare drop from an elder/sage mob |
| #13 | Luck Bankbook            | A-20  | Casino or gambling area mob; or lottery trigger |
| #14 | Connection Severing Scissors | B-22 | Tailor or barber mob drop |
| #15 | Fickle Genie             | S-10  | Rare event trigger: genie mob appears and drops this |
| #16 | Fairy King's Advice      | S-6   | Has "target example" trigger — needs a source mob (fairy-type boss?) |
| #65 | Witch's Rejuvenation Potion | S-10 | Witch mob drop (zone 279 already has witch-themed mobs) |
| #66 | Witch's Diet Potion      | B-28  | Witch mob drop (zone 279) |
| #67 | Doyen's Growth Pills     | B-30  | Doyen NPC or apothecary mob drop |
| #69 | Doyen's Hair Restorer    | B-30  | Doyen NPC or apothecary mob drop |
| #85 | Sacrifice Armor          | S-8   | Rare drop from a paladin or guardian boss mob |

---

### NEW CARDS — all 55 need placement

These were added to complete the 100-card set. None have a dedicated spawn point yet.
Each card transforms into its physical item when gained; the item does NOT need separate placement
unless you want players to find the item directly (bypassing the card).

#### SS Grade (most rare)
| Card | Name | Suggested placement |
|------|------|---------------------|
| #99 | Three-Star Hunter License SS-1 | Quest reward from the Hunter Association Chairman NPC, or rare boss drop in the endgame zone |

#### S Grade (very rare)
| Card | Name | Suggested placement |
|------|------|---------------------|
| #18 | Invisible Stalker S-7       | Rare drop from a stealthy assassin-type mob |
| #34 | Time-Stopping Watch S-8     | Rare drop from a clockmaker or time-mage mob |
| #40 | Tornado Stand S-9           | Drop from wind-element boss mob |
| #41 | Magnetic Rod S-10           | Drop from a mechanical or golem-type mob |
| #43 | Paladin's Prayer Beads S-7  | Drop from a high-rank paladin or cleric boss |
| #44 | Rebirth S-10                | Very rare drop from a phoenix or resurrection-themed mob |
| #49 | Aura Amplifier S-6          | Rare drop from a Nen master NPC |
| #59 | Secret Stash S-7            | Drop from a thief or treasure hunter mob |
| #63 | Binding Snake S-8           | Drop from a snake charmer or summoner mob |
| #72 | Witch's Wisdom Potion S-8   | Witch mob drop (zone 279 or new witch area) |
| #76 | Transmutation Stone S-8     | Drop from an alchemist mob |
| #78 | Elixir of Life S-6          | Very rare drop from an alchemist boss or ancient sage |
| #95 | Armor of Zeno S-6           | Drop from a Zoldyck-family themed mob (assassin boss) |

#### A Grade (uncommon)
| Card | Name | Suggested placement |
|------|------|---------------------|
| #19 | Cane of Healing A-13        | Drop from a healer or nurse NPC mob |
| #22 | Dress of Memory A-17        | Drop from a dressmaker or ghost-type mob |
| #24 | Book of V.I.P. Parties A-20 | Drop from an aristocrat or party host NPC |
| #27 | Dino Basket A-18            | Drop from a jungle area mob or dinosaur herder |
| #32 | Diamond Sword A-18          | Drop from a swordsmith or gem merchant mob |
| #33 | Dragon's Jaw A-18           | Drop from a dragon-type mob or dragon slayer NPC |
| #37 | Echo Recorder A-20          | Drop from a journalist or inventor NPC |
| #38 | Aqua Guard A-17             | Drop from a sea guard or aquatic area mob |
| #42 | Water Divination Staff A-17 | Drop from a diviner or oracle NPC |
| #47 | Wind Chime A-17             | Drop from a shrine keeper or wind-element mob |
| #48 | Fairy's Breath A-15         | Drop from a fairy-type mob or nature spirit |
| #50 | Miracle Seed A-18           | Drop from a botanist or garden area mob |
| #52 | Sand Ship A-18              | Drop from a desert explorer or sailor mob |
| #53 | Crystal Ball A-13           | Drop from a fortune teller or oracle mob |
| #54 | Glowing Wings A-15          | Drop from a winged mob (angel/fairy type) |
| #57 | Worm Snake A-20             | Drop from a serpent/snake mob in a dungeon area |
| #58 | Mr. Billionaire A-20        | Drop from a wealthy NPC/merchant boss |
| #60 | Invisible Cloak A-18        | Drop from a spy or shadow-type mob |
| #70 | Rainbow Diamond A-13        | Drop from a gemstone golem or jewel thief mob |
| #77 | Philosopher's Stone A-15    | Drop from an alchemist or scholar mob |
| #79 | Dragon's Eye A-15           | Drop from a dragon-type mob or gem collector |
| #82 | Comet Stone A-15            | Drop from a meteor crater area mob or astronomer |
| #86 | Heretic's Axe A-20          | Drop from an outlaw or blasphemer-themed mob |
| #88 | Iron Boots A-20             | Drop from a blacksmith or heavy-armored mob |
| #91 | Sniper Rifle A-20           | Drop from a marksman or bounty hunter mob |
| #92 | Flute of Confusion A-17     | Drop from a musician or bard NPC mob |
| #98 | Turtle Claw Gloves A-20     | Drop from a turtle-type mob or martial artist |

#### B Grade (moderate)
| Card | Name | Suggested placement |
|------|------|---------------------|
| #21 | Cutter B-25                 | Drop from a knife fighter or rogue-type mob |
| #36 | Master Mime B-30            | Drop from a theater performer mob or entertainer |
| #45 | Face Lift Machine B-30      | Drop from a doctor or beauty salon NPC |
| #55 | Sleep Clock B-25            | Drop from a night guard or sandman-type mob |
| #93 | Haze Smoke Bomb B-30        | Drop from a ninja or shadow ops mob |
| #96 | Thunderbolt Disk B-22       | Drop from an electric-element mob or disk fighter |
| #97 | Razor Wind Turban B-22      | Drop from a wind-element mob or turban fighter |

#### C Grade (common)
| Card | Name | Suggested placement |
|------|------|---------------------|
| #20 | Explosive Marbles C-50      | Common drop; found in room near a toy shop or mine |
| #23 | Paper Doll C-45             | Common drop; drop from a shaman or paper crafter mob |

#### D Grade (very common, low value)
| Card | Name | Suggested placement |
|------|------|---------------------|
| #28 | Lottery Rose D-70           | Common room object; place in a garden or flower shop area |
| #29 | Loving Slave D-65           | Common drop from any low-level mob (low grade) |
| #30 | Pretty Little Devil D-70    | Common drop or room object in a souvenir shop |
| #35 | Turtle Mansion D-65         | Common room object in a toy store or noble quarter |
| #51 | Flame Guitar D-70           | Common room object or drop from a street musician mob |

---

## Implementation Notes

### Quick approach (little content work)
Add the new cards as mob drops on existing mobs in already populated zones.
For example, zone 279 (Dorias — the gambling city) already has 5 mob drops;
several new cards fit thematically and could be added there.

### Proper approach (recommended)
Create new mobs or use existing under-utilized mobs in fitting zones.
Each card should feel like it belongs in the zone that drops it.
Zones with available mob slots: 31 (castle), 36 (gem area), 79 (forest),
186 (palace), 279 (Dorias), 410 (special area).

### Note on physical items
Physical items (zone 654) do NOT need separate placement if the card can be obtained.
When a player uses `gain` on a card, it transforms into the item automatically.
Only add direct item placement if you want the item to be findable WITHOUT having the card first
(as is done for the scanner, night jade, postcard, etc.).

### Special cards to clarify before implementing
- **#0 Ruler's Blessing SS-1**: The trigger text says "Stay at front of castle gate in Limeiro
  if you got all restricted cards." This should be the final reward for completing the full set —
  implement as a quest NPC that gives card 0 when player has all 99 other cards.
- **#1 Patch of Forest SS-3**: Marked "too valuable to use" — decide if this is a display-only
  card or needs its own obtain path and mechanics.
- **#2 Patch of Shore SS-3**: Tied to the housing system — already has elevator mechanics.

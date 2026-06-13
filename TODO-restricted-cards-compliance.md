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
- Time-Stopping Watch (65334) — stop-time; not readily modelable.
- Shield of Faith (65387) — nullify Railguide/Return/Drift/Collision in the room.
- Tax Collector's Gauntlet (65389) — `levy` command that destroys a random binder card.
- Connection Severing Scissors (65314), Fickle Genie 3-wishes (65315),
  Perfect Memory Studio (65356) — bespoke/flavor mechanics.
- Bandit's Blade (65394) — cast Rob/Pickpocket/Thief on hit (negative hit/dam **intentional**).

**Flavor-only collectibles** (effect not meaningfully modelable; acceptable as-is): Pregnancy
Stones, Dress of Memory, Paper Doll, Book of VIP Parties, Master Mime, Echo Recorder,
Face-Lift Machine, Miracle Seed, Sand Ship, Turtle Mansion, Mr. Billionaire, Returned
Postcard, Loving Slave, Pretty Little Devil, Crystal Ball, etc.

---

## BIG BACKLOG — full canon alignment of invented cards (deferred)
Audited against the official fandom list (saved at `C:\Users\henqu\Downloads\Greed Island Card
Lists _ Hunterpedia _ Fandom.html`). Many front-half cards are inventions to be replaced with
canon (names + ranks + effects from the saved page), e.g.: #18 Imp's Wink, #19 Poltergeist
Pillow, #20 Mood Clock, #21 X-Ray Goggles, #22 Toraemon, #23 Tome of a Thousand Tales, #24
Hypothetical T.V., #27 Book of V.I.P Passes, #28 Capricious Remote, #29 Pre-Order Vouchers,
#30 Favor Cushion, #32 Parrot Candy, #33 Hormone Cookies, #34 Universal Survey, #47 Sleeping
Girl, #48 Aromatherapy Girl, #49 Miniature Mermaid, plus the #50-60 cluster and scattered
back-half items. The back-half "Witch's/Doyen's/jewel/weapon" cluster is mostly canon-NAMED but
may be off by ~1 in numbering — reconcile against the saved page. Do in batches; keep card
vnums contiguous and trigger vnums ascending.

## Reference — functional cards that predated this work
Breath of Archangel (65317→65417, trg 3265 spawns healing mob), Risky Dice (65425), Vending
Check-Up (65461 scanner), Sword of Truth (65483), Paladin's Necklace (65484), Doyen's Virility
Pills (65468), Pitcher of Eternal Water (65403), containers (Secret Stash 65459, Dino Basket
65427), and well-statted weapons/armor (Diamond Sword, Dragon's Jaw, Armor of Zeno, …).

# SDD Progress Ledger — vnum-migration branch
# Base commit: bf1dd0b

## Tasks
- [x] Task 1: World data files (Python migration → 0.obj, 1.obj, 2.obj, 3.mob, 0.trg, 3.trg, zone files) — verified OK
- [x] Task 2: C source — act.item.c — verified OK
- [x] Task 3: C source — remaining files (act.informative.c, act.other.c, fight.c, limits.c, utils.c) — completed in b680d04
- [x] Task 4: External triggers (10.trg, 32.trg, 120.trg, 401.trg) + quests (14.qst, 186.qst, 400.qst) — completed
- [x] Task 5: Cleanup — empty old 653/654 zon stubs, verify index — completed

## Scheme (final)
| Content | vnum | File |
|---|---|---|
| Restricted cards 000-099 | obj 0-99 | 0.obj |
| Special items (rations/apple/backpack) | obj 110/111/133 | 1.obj |
| Card-items (physical form) | obj 200-299 | 2.obj |
| Stock CircleMUD objects (restored) | obj 300-399 | 3.obj |
| Summon NPCs + clairvoyant snake | mob 317/346/347/348/396 | 3.mob |
| Wildlife (unchanged) | mob 400-416 | 4.mob |

## Notes
- offset: card N ↔ item N+200 (was N+300, caused collision with stock lib 300-399)
- NOTHING sentinel (65535) must never be remapped to a real vnum; use the NOTHING constant in C
- mob 396 (clairvoyant snake) added to 3.mob; wildlife 400-416 removed (were duplicates of 4.mob)
- 653.zon/654.zon are stubs (no reset commands); obj/mob/trg 653/654 are all stubs

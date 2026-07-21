---
name: greed-island-cards
description: Reference for the Greed Island card system - vnum mappings between restricted cards and their physical items, the make_card conversion engine, player commands (gain/change/book), auto-conversion and timed-reversion rules, and where player-facing text must stay in sync. Use when working on card mechanics, ITEM_CARD/ITEM_SPELLCARD/ITEM_RESTRICTED items, or the 653/654 zone card files.
---

# The Greed Island card system (detailed)

The card mechanic is the heart of the game and its logic is spread across a few hooks. Reference map:

**Item types** (`structs.h`): `ITEM_CARD` 24 (free/unrestricted card), `ITEM_SPELLCARD` 25 (cast on use, consumed), `ITEM_RESTRICTED` 26 (the numbered 000–099 collectible cards). `IS_CARD(obj)` (`utils.h`) = any of those three.

**vnum mappings**
- Restricted: **card vnum ∈ 65300–65399 ↔ physical item = card + 100 (65400–65499)**. The restricted card's `GET_OBJ_RENT` holds the *global copy limit* (the "-N" in `SS-1`, `A-17`).
- Free cards (type 24): the physical-item vnum is stored in the card's **`GET_OBJ_RENT`** (e.g. free cards in `lib/world/obj/400.obj`, their items in `401.obj`). A freshly-created free card stamps the source item's vnum into `GET_OBJ_RENT`.
- Voucher: a type-24 card at vnum 65535 flagged `ITEM_QUEST`; appears when a restricted card's global limit is reached.

**The conversion engine**: `make_card(ch, obj, show)` in `act.item.c` is a single **bidirectional** function — it decides direction from the object's type/vnum (item→card creates/loads a card; card→item reads the stored target). `show` only controls the room message. Returns 1 on success, 0 if nothing happened. `ITEM_NOGAIN` (extra flag) marks "already converted, cannot convert again" and is never cleared.

Special cases are inserted at `act.item.c` ~line 1919 (after the Recycling Room block and the Perfect Memory Studio block, both inside the `ITEM_RESTRICTED` branch). The newly-created item is in the local variable `card`. Use `obj_to_obj(child, card)` to pre-load objects into a container before it reaches the player (see Hormone Cookies #33, vnum 65333 → loads 20 cookies vnum 65534 into box 65433).

**Player commands**
- `gain` (`do_gain`): bidirectional convenience verb — card→item, item→card, and **casting spell cards** (needs the book/`PLR_BOOK` active via the `book` command).
- `change` (`do_change`): **item→card only**, for ordinary items. This is the manual path for common loot.
- `book` (`do_book`, `act.movement.c`): summons the binder container (vnum **3203**) via the ring (3202); 45 free slots + 100 restricted (deduped by vnum).

**Auto-conversion rule (current)**: on pickup/give-from-NPC/steal, an item auto-converts to a card **only if it becomes a restricted card** — gated by `GET_OBJ_VNUM(obj) > 65300 && != 65535` at the call sites in `act.item.c` (`perform_get_from_room`, `perform_get_from_container`, `perform_give`) and `act.other.c` (`do_steal`). Ordinary items never auto-convert; the player uses `change`.

**Timed auto-reversion**: `second_update` (`limits.c`) decrements each loose card's `GET_OBJ_TIMER` (set to 62 on creation / on binder put-get); at 0 a card outside binder 3203 reverts to its item (and is flagged `ITEM_NOGAIN`). Cards inside the binder are exempt — that's the incentive to store them.

**Player-facing text to keep in sync** when card behavior changes: NPC **Eta** (mob 1401) triggers `1419`/`1496` in `lib/world/trg/14.trg`; the notice sign obj **3298** in `lib/world/obj/32.obj`; help entries `GAIN`, `CHANGE`, `CARDS`, `BOOK` in `lib/text/help/help.hlp`; and `lib/text/info`.

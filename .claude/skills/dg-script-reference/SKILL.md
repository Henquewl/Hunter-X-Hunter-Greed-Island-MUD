---
name: dg-script-reference
description: Reference for DG Script triggers - tick/second timing conversions, trigger type codes (first line after the name~), the elseif-vs-"else if" parsing gotcha, halt vs return 0 in consume triggers, the timed player-effect pattern (remote variable + attach), and writable %char.field% subfields. Use when writing or debugging lib/world/trg/ trigger files.
---

# DG Script reference

## Timing

`affect_update()` fires **once per MUD hour** (every 75 real seconds). That is 1 "tick."

| Unit | Real time |
|---|---|
| 1 tick (1 MUD hour) | 75 seconds |
| 24 ticks (24 MUD hours) | ~30 minutes |
| 240 ticks | ~5 hours |

- **`dg_affect %actor% STAT mod duration`** — duration is in **ticks** (e.g. `dg_affect %actor% STR 2 240` = +2 STR for ~5 hours).
- **`wait N sec`** — duration is in **real seconds** (e.g. `wait 1800 sec` = 24 MUD hours).

## Trigger type codes (first line after the name~)

`<entity-type> <event-flags> <chance>`

| entity-type | 0 = mob | 1 = obj | 2 = room |
|---|---|---|---|
| event-flag `s` | — | OTRIG_CONSUME (eat/drink) | — |
| event-flag `n` | — | OTRIG_LOAD | — |
| event-flag `b` | MOB_TRIGGER_BRIBE | background (mob, runs on attach) | — |
| event-flag `j` | — | OTRIG_WEAR | — |
| event-flag `l` | — | OTRIG_REMOVE | — |

- `1 s 0` — object consume trigger, always fires.
- `0 b 100` — mob background trigger, 100% chance; standard for timers attached to a player with `attach`.
- `1 n 100` — object load trigger; used for setup loops (like Hot Spring, Tree of Plenty).

## `elseif` vs `else if` in DG Scripts

DG Script does **not** support `else if` (two separate words). The interpreter checks for `elseif ` (7 chars, one word + space) before falling back to `else` (4 chars). A line written as `else if <condition>` matches the `else` branch — the condition is **ignored** and the block always executes as a plain `else`. This causes Neutral-sex actors to receive the Female branch, boolean guards to be skipped, etc. Always write `elseif <condition>` as one word.

## `halt` vs `return 0` in consume triggers

- **`halt`** — stops the trigger; `consume_otrigger` returns 1 → the item **is consumed** normally. Use when the effect simply doesn't apply but the item should still be eaten.
- **`return 0`** — `consume_otrigger` returns 0 → the item is **not consumed** and nothing happens. Use only when the action should be fully blocked.

## Timed player effect pattern (remote variable + attach)

Standard pattern for a temporary effect on a player that must auto-revert:

```
* --- consume trigger (1 s 0) on the item ---
eval myeffect_active 1
remote myeffect_active %actor.id%       * flag readable as %actor.myeffect_active%
attach <TIMER_TRIG_VNUM> %actor.id%     * binds a mob trigger to the player

* --- timer trigger (0 b 100) attached to the player ---
wait <N> sec
* apply revert here
rdelete myeffect_active %self.id%       * clear the flag
detach <TIMER_TRIG_VNUM> %self.id%
```

- **Check if effect is active**: `if %actor.myeffect_active%` (empty string = falsy when var absent).
- **Remote variables are volatile**: lost on full server reboot (not on copyover) **and on player quit/reconnect**. The underlying C change (e.g. `GET_SEX`) is saved to the player file, but the attached timer trigger and remote vars are not. Consequence: a player who quits mid-effect reconnects with the stat change permanent and no reversion timer. Fix requires porting the effect to the C `affect` system (duration-based, saved to player file). Accepted as a known limitation for effects where the exploit is harmless (e.g. Hormone Cookies sex swap).
- **`%actor.is_npc%`** — reliable guard to skip NPCs in any trigger.

## Writable DG fields (subfield syntax)

Some `%char.field%` support a write path via parenthesised subfield: `nop %actor.field(value)%`.

| Field | Write subfield | Notes |
|---|---|---|
| `sex` | `0`/`1`/`2` (SEX_NEUTRAL/MALE/FEMALE) | Patched in `dg_variables.c` ~line 1038 |
| `saving_spell` | integer delta | Adds delta to current value |
| `skillset` | `<skillname> <value>` | Sets skill % |

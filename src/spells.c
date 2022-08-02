/**************************************************************************
*  File: spells.c                                          Part of tbaMUD *
*  Usage: Implementation of "manual spells."                              *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "interpreter.h"
#include "dg_scripts.h"
#include "act.h"
#include "fight.h"




/* Special spells appear below. */
/*ASPELL(spell_energy_drain)
{
  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch) {
    send_to_char(ch, "This can cause a headache..\r\n");
    return;
  }
	
  if (mag_savingthrow(victim, SAVING_SPELL, 0)) {
    return;
  }
//	
  if (GET_EXP(victim) <= 1000 + (GET_LEVEL(ch) * 1300)) {
    send_to_char(ch, "\tDYour nen was increased by \tG%d\tD.\tn", (GET_EXP(victim) / 4));
	GET_EXP(ch) += GET_EXP(victim) / 4;
	  if (IS_NPC(victim))
	    GET_EXP(victim) -= GET_EXP(victim) / 4;
      else
        GET_EXP(victim) = 1;
  } else {
	send_to_char(ch, "\tDYour nen was increased by \tG%d\tD.\tn", ((1000 + (GET_LEVEL(ch) * 1300)) / 4));
      if (IS_NPC(victim))
	    GET_EXP(victim) -= ((1000 + (GET_LEVEL(ch) * 1300)) / 4);
      else
		GET_EXP(victim) -= 1000 + (GET_LEVEL(ch) * 1300);
    GET_EXP(ch) += ((1000 + (GET_LEVEL(ch) * 1300)) / 4);
  }
//
  GET_MANA(victim) = (GET_MANA(victim) - 5);
  GET_HIT(ch) += (GET_HIT(victim) / 20);
}
*/
ASPELL(spell_create_water)
{
  int water;

  if (ch == NULL || obj == NULL)
    return;
  /* level = MAX(MIN(level, LVL_IMPL), 1);	 - not used */

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
	if (GET_OBJ_VAL(obj, 0) > 0 && GET_OBJ_VAL(obj, 1) < GET_OBJ_VAL(obj, 0)) {
	  water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
	  GET_OBJ_VAL(obj, 1) += water;
	  weight_change_object(obj, water);
	  act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);	  
	} else {
      if (GET_OBJ_VAL(obj, 0) == -1 || GET_OBJ_VAL(obj, 0) == GET_OBJ_VAL(obj, 1))
	    send_to_char(ch, "But %s is already full!", obj->short_description);
	  else	
	    send_to_char(ch, "You tried to fill %s but it can't sustain a single drop.", obj->short_description);
	  return;
	}
  } else
	send_to_char(ch, "Is it a mirage or do you think %s is a liquid container?", obj->short_description); 
 
	 
/*    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      name_from_drinkcon(obj);
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
      if (water > 0) {
	if (GET_OBJ_VAL(obj, 1) >= 0)
	  name_from_drinkcon(obj);
	GET_OBJ_VAL(obj, 2) = LIQ_WATER;
	GET_OBJ_VAL(obj, 1) += water;
	name_to_drinkcon(obj, LIQ_WATER);
	weight_change_object(obj, water);
	act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
*/
}

ASPELL(spell_recall)
{
  room_rnum to_room;	
	
  if (victim == NULL || IS_NPC(victim))
    return;

  if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(victim)), ZONE_NOASTRAL)) {
    send_to_char(ch, "A bright flash prevents your spell from working!");
    return;
  }
  
  to_room = real_room(40000);

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);
}

ASPELL(spell_teleport)
{
	struct follow_type *k, *next;
	bool found;
    room_rnum to_room;    


  if (victim == NULL)
	return;
  
  if (IS_NPC(victim) && !AFF_FLAGGED(victim, AFF_CHARM)) {
    act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	return;   
  }  
  
  if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(victim)), ZONE_NOASTRAL)) {
    send_to_char(ch, "A bright flash prevents your spell from working!");
    return;
  }  
  
  if (!IS_NPC(victim))
	found = TRUE;
  
  if (IS_NPC(victim) && AFF_FLAGGED(victim, AFF_CHARM) && victim->followers)
	for (k = victim->followers; k; k = next) {
	  next = k->next;
	  if (k == ch)
		found = TRUE;	
	  }  
	if (!found)
	  return;
//  to_room = GET_LOADROOM(ch);
  if (victim == ch)
    send_to_char(ch, "You close your eyes trying to sense the teleport aura marking...\r\n");
  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  if (GET_LOADROOM(ch) == NOWHERE) {
	to_room = real_room(40000);
	char_to_room(victim, to_room);
  } else
    char_to_room(victim, GET_LOADROOM(ch));
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);

/*  do {
    to_room = rand_number(0, top_of_world);
  } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE) || ROOM_FLAGGED(to_room, ROOM_DEATH) ||
           ROOM_FLAGGED(to_room, ROOM_GODROOM) || ZONE_FLAGGED(GET_ROOM_ZONE(to_room), ZONE_CLOSED) ||
           ZONE_FLAGGED(GET_ROOM_ZONE(to_room), ZONE_NOASTRAL));

  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);
*/
}

#define SUMMON_FAIL "You failed.\r\n"
ASPELL(spell_summon)
{
  if (ch == NULL || victim == NULL)
    return;

  if (GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, level + 1)) {
    send_to_char(ch, "%s", SUMMON_FAIL);
    return;
  }

  if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(victim)), ZONE_NOASTRAL) ||
      ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(ch)), ZONE_NOASTRAL)) {
    send_to_char(ch, "A bright flash prevents your spell from working!");
    return;
  }

/*  if (!CONFIG_PK_ALLOWED) {
    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
      act("As the words escape your lips and $N travels\r\n"
	  "through time and space towards you, you realize that $E is\r\n"
	  "aggressive and might harm you, so you wisely send $M back.",
	  FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
  if ((!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE)) || 
  (!IS_NPC(victim) && !PLR_FLAGGED(victim, PLR_KILLER))) {
    send_to_char(victim, "%s just tried to summon you to: %s.\r\n"
	    "This failed because you have summon protection on.\r\n"
	    "Type NOSUMMON to allow other players to summon you.\r\n",
	    GET_NAME(ch), world[IN_ROOM(ch)].name);

    send_to_char(ch, "You failed because %s has summon protection on.\r\n", GET_NAME(victim));
    mudlog(BRF, LVL_IMMORT, TRUE, "%s failed summoning %s to %s.", GET_NAME(ch), GET_NAME(victim), world[IN_ROOM(ch)].name);
    return;
  }
  } */

  if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE) && ((!PLR_FLAGGED(victim, PLR_KILLER) && mag_savingthrow(victim, SAVING_SPELL, 0)))) {
	send_to_char(ch, "%s", SUMMON_FAIL);
    return;	 
  }		 
  
  if (MOB_FLAGGED(victim, MOB_NOSUMMON) ||
      (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL, 0))) {
    send_to_char(ch, "%s", SUMMON_FAIL);
    return;
  }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, IN_ROOM(ch));

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);
}

/* Used by the locate object spell to check the alias list on objects */
int isname_obj(char *search, char *list)
{
  char *found_in_list; /* But could be something like 'ring' in 'shimmering.' */
  char searchname[128];
  char namelist[MAX_STRING_LENGTH];
  int found_pos = -1;
  int found_name=0; /* found the name we're looking for */
  int match = 1;
  int i;

  /* Force to lowercase for string comparisons */
  sprintf(searchname, "%s", search);
  for (i = 0; searchname[i]; i++)
    searchname[i] = LOWER(searchname[i]);

  sprintf(namelist, "%s", list);
  for (i = 0; namelist[i]; i++)
    namelist[i] = LOWER(namelist[i]);

  /* see if searchname exists any place within namelist */
  found_in_list = strstr(namelist, searchname);
  if (!found_in_list) {
    return 0;
  }

  /* Found the name in the list, now see if it's a valid hit. The following
   * avoids substrings (like ring in shimmering) is it at beginning of
   * namelist? */
  for (i = 0; searchname[i]; i++)
    if (searchname[i] != namelist[i])
      match = 0;

  if (match) /* It was found at the start of the namelist string. */
    found_name = 1;
  else { /* It is embedded inside namelist. Is it preceded by a space? */
    found_pos = found_in_list - namelist;
    if (namelist[found_pos-1] == ' ')
      found_name = 1;
  }

  if (found_name)
    return 1;
  else
    return 0;
}

ASPELL(spell_locate_card)
{
  struct obj_data *i;
  char name[MAX_INPUT_LENGTH];
  int j;  
  
  /*  added a global var to catch 2nd arg. */  
  sprintf(name, "%s", cast_arg2); 

  if (!obj) {
    send_to_char(ch, "No card with this name in database.\r\n");
    return;
  } 

  j = 31;
  
  for (i = object_list; i && (j > 0); i = i->next) {		
    if (!isname_obj(name, i->name))
      continue;
	else if (!IS_CARD(i))
	  continue; 
    else if (i->carried_by && GET_MOB_SPEC(i->carried_by))
	  continue;
    else if (IN_ROOM(i) != NOWHERE && (ROOM_FLAGGED(IN_ROOM(i), ROOM_GODROOM) || ROOM_FLAGGED(IN_ROOM(i), ROOM_PRIVATE)))
	  continue;

    send_to_char(ch, "%c%s", UPPER(*i->short_description), i->short_description + 1);

    if (i->carried_by)	  
      send_to_char(ch, " is being carried by %s.\r\n", PERS(i->carried_by, ch));
    else if (IN_ROOM(i) != NOWHERE)
      send_to_char(ch, " is in %s.\r\n", world[IN_ROOM(i)].name);    
	else if (i->in_obj) {
	  if (i->in_obj->carried_by)
		send_to_char(ch, " is in %s being carried by %s.\r\n", i->in_obj->short_description, PERS(i->in_obj->carried_by, ch));  
	  else if (i->in_obj->worn_by)
	    send_to_char(ch, " is in %s being worn by %s.\r\n", i->in_obj->short_description, PERS(i->in_obj->carried_by, ch));
	  else if (IN_ROOM(i->in_obj) != NOWHERE)
		send_to_char(ch, " is inside %s in %s.\r\n", i->in_obj->short_description, world[IN_ROOM(i->in_obj)].name);
	  else
        send_to_char(ch, " is in %s.\r\n", i->in_obj->short_description);
    } else if (i->worn_by)
      send_to_char(ch, " is being worn by %s.\r\n", PERS(i->worn_by, ch));
    else
      send_to_char(ch, "'s location is uncertain.\r\n");

    j--;
  }  
}

ASPELL(spell_locate_object)
{
  struct obj_data *i;
  char name[MAX_INPUT_LENGTH];
  int j; 

  if (!obj) {
    send_to_char(ch, "You sense nothing.\r\n");
    return;
  }

  sprintf(name, "%s", cast_arg2);   

  /*  added a global var to catch 2nd arg. */  

  j = GET_LEVEL(ch) / 3;  /* # items to show = twice char's level */

  for (i = object_list; i && (j > 0); i = i->next) {
	if (!isname_obj(name, i->name))
      continue;	
    else if (IS_CARD(i))
	  continue;

  send_to_char(ch, "%c%s", UPPER(*i->short_description), i->short_description + 1);

    if (i->carried_by)
      send_to_char(ch, " is being carried by %s.\r\n", PERS(i->carried_by, ch));
    else if (IN_ROOM(i) != NOWHERE)
      send_to_char(ch, " is in %s.\r\n", world[IN_ROOM(i)].name);
    else if (i->in_obj) {
	  if (i->in_obj->carried_by)
		send_to_char(ch, " is in %s being carried by %s.\r\n", i->in_obj->short_description, PERS(i->in_obj->carried_by, ch));  
	  else if (i->in_obj->worn_by)
	    send_to_char(ch, " is in %s being worn by %s.\r\n", i->in_obj->short_description, PERS(i->in_obj->carried_by, ch));
	  else if (IN_ROOM(i->in_obj) != NOWHERE)
		send_to_char(ch, " is inside %s in %s.\r\n", i->in_obj->short_description, world[IN_ROOM(i->in_obj)].name);
	  else
        send_to_char(ch, " is in %s.\r\n", i->in_obj->short_description);
    } else if (i->worn_by)
      send_to_char(ch, " is being worn by %s.\r\n", PERS(i->worn_by, ch));
    else
      send_to_char(ch, "'s location is uncertain.\r\n");

    j--;
  }
}

ASPELL(spell_charm)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch)
    send_to_char(ch, "You like yourself even better!\r\n");
  /*else if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE))
    send_to_char(ch, "You fail because SUMMON protection is on!\r\n");*/
  else if (AFF_FLAGGED(victim, AFF_SANCTUARY))
    send_to_char(ch, "Your victim is protected by fortify!\r\n");
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char(ch, "Your victim resists!\r\n");
  else if (AFF_FLAGGED(ch, AFF_CHARM))
    send_to_char(ch, "You can't have any followers of your own!\r\n");
  else if (AFF_FLAGGED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
    send_to_char(ch, "You fail.\r\n");
  /* player charming another player - no legal reason for this */
  /*else if (!CONFIG_PK_ALLOWED && !IS_NPC(victim))
    send_to_char(ch, "You fail - shouldn't be doing it anyway.\r\n");*/
  else if (circle_follow(victim, ch))
    send_to_char(ch, "Sorry, following in circles is not allowed.\r\n");
  else if (mag_savingthrow(victim, SAVING_PARA, 0))
    send_to_char(ch, "Your victim resists!\r\n");
  else if (GET_CHA(ch) <= GET_INT(victim))
	send_to_char(ch, "Your victim resists!\r\n");
  else {
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    new_affect(&af);
    af.spell = SPELL_CHARM;
    af.duration = GET_LEVEL(ch) + GET_CHA(ch);
    if (GET_CHA(ch))
      af.duration *= GET_CHA(ch);
    if (GET_INT(victim))
      af.duration /= GET_INT(victim);
	SET_BIT_AR(af.bitvector, AFF_CHARM);
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim))
      REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_SPEC);
  }
}

ASPELL(spell_identify)
{
  int i, found;
  size_t len;

  if (obj) {
    char bitbuf[MAX_STRING_LENGTH];

    sprinttype(GET_OBJ_TYPE(obj), item_types, bitbuf, sizeof(bitbuf));
    send_to_char(ch, "You feel informed:\r\nObject '%s', Item type: %s\r\n", obj->short_description, bitbuf);

    if (GET_OBJ_AFFECT(obj)) {
      sprintbitarray(GET_OBJ_AFFECT(obj), affected_bits, AF_ARRAY_MAX, bitbuf);
      send_to_char(ch, "Item will give you following abilities:  %s\r\n", bitbuf);
    }

    sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, bitbuf);
    send_to_char(ch, "Item is: %s\r\n", bitbuf);

    send_to_char(ch, "Weight: %d, Value: %d, Min. level: %d\r\n",
                     GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_LEVEL(obj));

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      len = i = 0;

      if (GET_OBJ_VAL(obj, 1) >= 1) {
	i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s", skill_name(GET_OBJ_VAL(obj, 1)));
        if (i >= 0)
          len += i;
      }

      if (GET_OBJ_VAL(obj, 2) >= 1 && len < sizeof(bitbuf)) {
	i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s", skill_name(GET_OBJ_VAL(obj, 2)));
        if (i >= 0)
          len += i;
      }

      if (GET_OBJ_VAL(obj, 3) >= 1 && len < sizeof(bitbuf)) {
	i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s", skill_name(GET_OBJ_VAL(obj, 3)));
        if (i >= 0)
          len += i;
      }

      send_to_char(ch, "This %s casts: %s\r\n", item_types[(int) GET_OBJ_TYPE(obj)], bitbuf);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      send_to_char(ch, "This %s casts: %s\r\nIt has %d maximum charge%s and %d remaining.\r\n",
		item_types[(int) GET_OBJ_TYPE(obj)], skill_name(GET_OBJ_VAL(obj, 3)),
		GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s", GET_OBJ_VAL(obj, 2));
      break;
    case ITEM_WEAPON:
      send_to_char(ch, "Damage Dice is '%dD%d' for an average per-round damage of %.1f.\r\n",
		GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2), ((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1));
      break;
    case ITEM_ARMOR:
      send_to_char(ch, "AC-apply is %d\r\n", GET_OBJ_VAL(obj, 0));
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	    if (!found) {
	      send_to_char(ch, "Can affect you as :\r\n");
	      found = TRUE;
	    }
	    if (obj->affected[i].location == APPLY_AC || obj->affected[i].location == APPLY_SAVING_SPELL) {
	      sprinttype(obj->affected[i].location, apply_types, bitbuf, sizeof(bitbuf));
	      send_to_char(ch, "   Affects: %s By %d\r\n", bitbuf, (obj->affected[i].modifier * -1));		
		} else {
	      sprinttype(obj->affected[i].location, apply_types, bitbuf, sizeof(bitbuf));
	      send_to_char(ch, "   Affects: %s By %d\r\n", bitbuf, obj->affected[i].modifier);
	    }		
      }
    }
  } else if (victim) {		/* victim */
    send_to_char(ch, "Name: %s\r\n", GET_NAME(victim));
    if (!IS_NPC(victim))
      send_to_char(ch, "%s is %d years, %d months, %d days and %d hours old.\r\n",
	      GET_NAME(victim), age(victim)->year, age(victim)->month,
	      age(victim)->day, age(victim)->hours);
    send_to_char(ch, "Height %d cm, Weight %d pounds\r\n", GET_HEIGHT(victim), GET_WEIGHT(victim));
    send_to_char(ch, "Level: %d, Hits: %d/%d, Nen: %d/%d, Move: %d/%d\r\n", GET_LEVEL(victim), GET_HIT(victim), GET_TOTAL_HIT(victim),
	    GET_MANA(victim), GET_MAX_MANA(victim), GET_MOVE(victim), GET_MAX_MOVE(victim));
    send_to_char(ch, "AR: %d, Hitroll: %d, Damroll: %d, Skill Affection: %d\r\n",
    	((compute_armor_class(victim) - 100) * -1), GET_HITROLL(victim), GET_DAMROLL(victim), GET_SAVE(victim, SAVING_SPELL));
    send_to_char(ch, "Str: %d/%d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
	GET_STR(victim), GET_ADD(victim), GET_INT(victim),
	GET_WIS(victim), GET_DEX(victim), GET_CON(victim), GET_CHA(victim));
  }
}

/* Cannot use this spell on an equipped object or it will mess up the wielding
 * character's hit/dam totals. */
ASPELL(spell_enchant_weapon)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  /* Either already enchanted. */
  if (OBJ_FLAGGED(obj, ITEM_MAGIC)){
    send_to_char(ch, "%c%s already enfold and nothing happens.\r\n", UPPER(*obj->short_description), obj->short_description + 1);    
  }
  else {

    /* Make sure no other affections. */
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE) {
	    send_to_char(ch, "This item already has special properties and rejects receive enfold.\r\n");
        return;
	  }
  
    SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
	SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ENFOLDED);

    if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
      obj->affected[0].location = APPLY_HITROLL;
      obj->affected[0].modifier = 1 + (level / 10);

      obj->affected[1].location = APPLY_DAMROLL;
      obj->affected[1].modifier = 1 + (level / 10);
    } else if (GET_OBJ_TYPE(obj) == ITEM_ARMOR) {
	  obj->affected[0].location = APPLY_AC;
      obj->affected[0].modifier = -2 - (level / 5);
	
	  obj->affected[1].location = APPLY_CHA;
	  obj->affected[1].modifier = 1;
	
	  obj->affected[2].location = APPLY_CON;
	  obj->affected[2].modifier = 1;
	
	  obj->affected[3].location = APPLY_DEX;
	  obj->affected[3].modifier = (level / 20);
	
	  obj->affected[4].location = APPLY_STR;
	  obj->affected[4].modifier = (level / 30);
    } else {
      obj->affected[0].location = APPLY_AC;
      obj->affected[0].modifier = -1 - (level / 10);
	
	  obj->affected[1].location = APPLY_WIS;
	  obj->affected[1].modifier = 1;
	
	  obj->affected[2].location = APPLY_INT;
	  obj->affected[2].modifier = 1;

      obj->affected[3].location = APPLY_HITROLL;
      obj->affected[3].modifier = (level / 20);

      obj->affected[4].location = APPLY_DAMROLL;
      obj->affected[4].modifier = (level / 30);	
    }	

    if (IS_GOOD(ch)) {
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows purple.", FALSE, ch, obj, 0, TO_CHAR);
    } else
      act("$p glows white.", FALSE, ch, obj, 0, TO_CHAR);
  }

}

ASPELL(spell_repair)
{
  if (ch == NULL || obj == NULL)
    return;

  switch(GET_OBJ_TYPE(obj)){
	case ITEM_LIGHT:
	  if (GET_OBJ_VAL(obj, 2) == 0) {
		GET_OBJ_VAL(obj, 2) = (GET_LEVEL(ch) * 2);
        act("$p lights again!", FALSE, ch, obj, 0, TO_CHAR);
	  } else
		act("$p needs no repairs.", FALSE, ch, obj, 0, TO_CHAR);
      break;
    case ITEM_STAFF:
	  GET_HIT(ch) -= ((GET_TOTAL_HIT(ch) * GET_OBJ_VAL(obj, 1)) / 20);
	case ITEM_WAND:
	  GET_HIT(ch) -= ((GET_TOTAL_HIT(ch) * GET_OBJ_VAL(obj, 1)) / 20);
	  if (GET_OBJ_VAL(obj, 2) == 0) {
		GET_OBJ_VAL(obj, 2) = GET_OBJ_VAL(obj, 1);
        act("$p was recharged!", FALSE, ch, obj, 0, TO_CHAR);
	  } else
		act("$p needs no repairs.", FALSE, ch, obj, 0, TO_CHAR);
      break;
	default:
	  if (GET_OBJ_DURABILITY(obj) == 100 || GET_OBJ_DURABILITY(obj) <= 0)
	    act("$p needs no repairs.", FALSE, ch, obj, 0, TO_CHAR);
      else { 
        GET_OBJ_DURABILITY(obj) = 100;
        act("$p was fully repaired!", FALSE, ch, obj, 0, TO_CHAR);	  
	  }
	  break;
  }    
}

/*ASPELL(spell_detect_poison)
{
  if (victim) {
    if (victim == ch) {
      if (AFF_FLAGGED(victim, AFF_POISON))
        send_to_char(ch, "You can sense poison in your blood.\r\n");
      else
        send_to_char(ch, "You feel healthy.\r\n");
    } else {
      if (AFF_FLAGGED(victim, AFF_POISON))
        act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
        act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      if (GET_OBJ_VAL(obj, 3))
	act("You sense that $p has been contaminated.",FALSE,ch,obj,0,TO_CHAR);
      else
	act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
	    TO_CHAR);
      break;
    default:
      send_to_char(ch, "You sense that it should not be consumed.\r\n");
    }
  }
} */

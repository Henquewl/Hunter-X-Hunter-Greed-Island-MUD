/**************************************************************************
*  File: limits.c                                          Part of tbaMUD *
*  Usage: Limits & gain funcs for HMV, exp, hunger/thirst, idle time.     *
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
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "dg_scripts.h"
#include "act.h"
#include "class.h"
#include "fight.h"
#include "screen.h"
#include "mud_event.h"
#include "constants.h" /* for nen and stamina plus recovery */

/* local file scope function prototypes */
static int graf(int grafage, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
static void check_idling(struct char_data *ch);


/* When age < 15 return the value p0
   When age is 15..29 calculate the line between p1 & p2
   When age is 30..44 calculate the line between p2 & p3
   When age is 45..59 calculate the line between p3 & p4
   When age is 60..79 calculate the line between p4 & p5
   When age >= 80 return the value p6 */
static int graf(int grafage, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (grafage < 15)
    return (p0);					/* < 15   */
  else if (grafage <= 29)
    return (p1 + (((grafage - 15) * (p2 - p1)) / 15));	/* 15..29 */
  else if (grafage <= 44)
    return (p2 + (((grafage - 30) * (p3 - p2)) / 15));	/* 30..44 */
  else if (grafage <= 59)
    return (p3 + (((grafage - 45) * (p4 - p3)) / 15));	/* 45..59 */
  else if (grafage <= 79)
    return (p4 + (((grafage - 60) * (p5 - p4)) / 20));	/* 60..79 */
  else
    return (p6);					/* >= 80 */
}

/* The hit_limit, mana_limit, and move_limit functions are gone.  They added an
 * unnecessary level of complexity to the internal structure, weren't
 * particularly useful, and led to some annoying bugs.  From the players' point
 * of view, the only difference the removal of these functions will make is
 * that a character's age will now only affect the HMV gain per tick, and _not_
 * the HMV maximums. */
/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
  int gain = 0;

  if (IS_NPC(ch)) {
    /* Neat and fast */
	if (!FIGHTING(ch))
      gain = 6;
    else
	  gain = 1;
  } else if (!IS_NPC(ch) || GET_MOB_VNUM(ch) == 10) {
//    gain = graf(age(ch)->year, 1, 2, 3, 4, 3, 2, 1);

    /* Class calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain = ((8 * con_app[GET_CON(ch)].hitp) / 100);
      break;
    case POS_RESTING:
      gain = ((4 * con_app[GET_CON(ch)].hitp) / 100);
      break;
    case POS_SITTING:
      gain = ((2 * con_app[GET_CON(ch)].hitp) / 100);
      break;
	case POS_FIGHTING:	  
	case POS_STANDING:
	  gain = ((1 * con_app[GET_CON(ch)].hitp) / 100);
	  break;
    }
  if (IS_WARRIOR(ch))
	gain += 1;
  
  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 2;

  if (gain <= 0)
	gain = 1;
  
  }  
  return (gain);
}
/* Hitpoint gain pr. game hour */
int hit_gain(struct char_data *ch)
{
  int gain;  
  
  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_VNUM(ch) == 10)) {
	gain = GET_TOTAL_HIT(ch) / 20;
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
        gain = ((gain * con_app[GET_CON(ch)].hitp) / 100);
        break;
      case POS_RESTING:
        gain = ((gain * con_app[GET_CON(ch)].hitp) / 150);
        break;
	  case POS_SITTING:
	  case POS_FIGHTING:
	  case POS_STANDING:
	    if (IS_NPC(ch) && ch->master && (!IS_NPC(ch->master) && IS_WARRIOR(ch->master)))
		  gain = ((gain * con_app[GET_CON(ch->master)].hitp) / 200);
	    else if (!IS_NPC(ch) && IS_WARRIOR(ch))
		  gain = 0;
	    else
		  return (0);
	    break;
	  default:
	      return (0);
	    break;
    }
	
	if (!IS_NPC(ch)) {
	  if (IS_WARRIOR(ch) && !PLR_FLAGGED(ch, PLR_POWERDOWN) && GET_HIT(ch) > 5)
		gain += ((gain * con_app[GET_CON(ch)].hitp) / 200);		
	  if ((GET_COND(ch, THIRST) == 0) && GET_LEVEL(ch) > 1)
        gain /= 2;  
      if ((GET_COND(ch, HUNGER) == 0) && GET_LEVEL(ch) > 1)
	    gain /= 2;	  
    }
	if (AFF_FLAGGED(ch, AFF_POISON))
        gain /= 2;
  } else
	  return (0);    

  return (gain);
}

/* move gain pr. game hour */
int move_gain(struct char_data *ch)
{
  int gain;

  if (GET_MAX_MOVE(ch) > GET_MOVE(ch))
    gain = GET_MAX_MOVE(ch) - GET_MOVE(ch);
  else
	gain = 0;
	
  return (gain);
}

void set_title(struct char_data *ch, char *title)
{
  if (GET_TITLE(ch) != NULL)
    free(GET_TITLE(ch));

  if (title == NULL) {
    GET_TITLE(ch) = title_hunter(GET_LEVEL(ch));
  } else {
    if (strlen(title) > MAX_TITLE_LENGTH)
      title[MAX_TITLE_LENGTH] = '\0';

    GET_TITLE(ch) = strdup(title);
  }
}

void run_autowiz(void)
{
#if defined(CIRCLE_UNIX) || defined(CIRCLE_WINDOWS)
  if (CONFIG_USE_AUTOWIZ) {
    size_t res;
    char buf[256];
    int i;

#if defined(CIRCLE_UNIX)
    res = snprintf(buf, sizeof(buf), "nice ../bin/autowiz %d %s %d %s %d &",
	CONFIG_MIN_WIZLIST_LEV, WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
#elif defined(CIRCLE_WINDOWS)
    res = snprintf(buf, sizeof(buf), "autowiz %d %s %d %s",
	CONFIG_MIN_WIZLIST_LEV, WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE);
#endif /* CIRCLE_WINDOWS */

    /* Abusing signed -> unsigned conversion to avoid '-1' check. */
    if (res < sizeof(buf)) {
      mudlog(CMP, LVL_IMMORT, FALSE, "Initiating autowiz.");
      reboot_wizlists();
      i = system(buf);
    } else
      log("Cannot run autowiz: command-line doesn't fit in buffer.");
  }
#endif /* CIRCLE_UNIX || CIRCLE_WINDOWS */
}

void gain_exp(struct char_data *ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;
  
  if (GET_MAX_HIT(ch) == 1000000000)
	return;
  
  if (IS_NPC(ch)) {
    GET_MAX_HIT(ch) += gain;
    return;
  }
  
  if (gain > 0) {
	
	if (gain < 10)
      gain = 9;
    else if (gain > (GET_TOTAL_HIT(ch) / 5))
	  gain = (GET_TOTAL_HIT(ch) / 5);
    else if (GET_EXP(ch) > 1)
	  gain += ((gain - 1) * (GET_EXP(ch) / 100));

    if ((IS_HAPPYHOUR) && (IS_HAPPYEXP))
      gain += (gain * ((float)HAPPY_EXP / (float)(100)));
  
    if ((gain + GET_MAX_HIT(ch)) > 1000000000) {
	  GET_MAX_HIT(ch) = 1000000000;
	  return;
	}

//    gain = MIN((GET_MAX_HIT(ch) / 4), gain);	/* put a cap on the max gain per kill */
    GET_MAX_HIT(ch) += gain;
	affect_total(ch);
	send_to_char(ch, "\tDYour nen was increased by \tG%d\tD.\tn\r\n", gain);
    while (GET_LEVEL(ch) < LVL_IMMORT - CONFIG_NO_MORT_TO_IMMORT &&
	GET_MAX_HIT(ch) >= level_exp(GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if (is_altered) {
      mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s advanced %d level%s to level %d.",
		GET_NAME(ch), num_levels, num_levels == 1 ? "" : "s", GET_LEVEL(ch));
      if (num_levels == 1)
        send_to_char(ch, "\tYYou rise a level!\tn\r\n");
      else
	send_to_char(ch, "\tYYou rise %d levels!\tn\r\n", num_levels);
//      set_title(ch, NULL);
      if (GET_LEVEL(ch) >= LVL_IMMORT && !PLR_FLAGGED(ch, PLR_NOWIZLIST))
        run_autowiz();
    }
  } else if (gain < 0) {
    GET_MAX_HIT(ch) += gain;
	affect_total(ch);	
	send_to_char(ch, "\tRYour nen was decreased by \tG%d\tR!\tn\r\n", gain);
	
  }  
  if (GET_LEVEL(ch) >= LVL_IMMORT && !PLR_FLAGGED(ch, PLR_NOWIZLIST))
    run_autowiz();
  }

void gain_exp_regardless(struct char_data *ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;

  if ((IS_HAPPYHOUR) && (IS_HAPPYEXP))
    gain += (gain * ((float)HAPPY_EXP / (float)(100)));

  GET_MAX_HIT(ch) += gain;
  if (GET_MAX_HIT(ch) < 0)
    GET_MAX_HIT(ch) = 0;

  if (!IS_NPC(ch)) {
    while (GET_LEVEL(ch) < LVL_IMPL &&
	GET_MAX_HIT(ch) >= level_exp(GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if (is_altered) {
      mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s advanced %d level%s to level %d.",
		GET_NAME(ch), num_levels, num_levels == 1 ? "" : "s", GET_LEVEL(ch));
      if (num_levels == 1)
        send_to_char(ch, "\tYYou rise a level!\tn\r\n");
      else
	send_to_char(ch, "\tYYou rise %d levels!\tn\r\n", num_levels);
//      set_title(ch, NULL);
    }
  }
  affect_total(ch);
  if (GET_LEVEL(ch) >= LVL_IMMORT && !PLR_FLAGGED(ch, PLR_NOWIZLIST))
    run_autowiz();
}

void gain_condition(struct char_data *ch, int condition, int value)
{
  bool intoxicated;

  if (IS_NPC(ch) || GET_COND(ch, condition) == -1 || GET_POS(ch) == POS_SLEEPING) /* No change */
    return;
  
  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  if (GET_COND(ch, condition) == GET_COND(ch, DRUNK))
	GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));
  else
    GET_COND(ch, condition) = MIN(100, GET_COND(ch, condition));

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING))
    return;
  
  switch (condition) {
  case HUNGER:
    send_to_char(ch, "You are hungry.\r\n");
    break;
  case THIRST:
    send_to_char(ch, "You are thirsty.\r\n");
    break;
  case DRUNK:
    if (intoxicated)
      send_to_char(ch, "You are now sober.\r\n");
    break;
  default:
    break;
  }
}

static void check_idling(struct char_data *ch)
{
  if (ch->char_specials.timer > CONFIG_IDLE_VOID) {
    if (GET_WAS_IN(ch) == NOWHERE && IN_ROOM(ch) != NOWHERE) {
      GET_WAS_IN(ch) = IN_ROOM(ch);
      if (FIGHTING(ch)) {
	stop_fighting(FIGHTING(ch));
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char(ch, "You have been idle, and are pulled into a void.\r\n");
      save_char(ch);
      Crash_crashsave(ch);
      char_from_room(ch);
      char_to_room(ch, 1);
	  SET_BIT_AR(PRF_FLAGS(ch), PRF_AFK);	    
    } else if (ch->char_specials.timer > CONFIG_IDLE_RENT_TIME) {
      if (IN_ROOM(ch) != NOWHERE)
	char_from_room(ch);
      char_to_room(ch, 3);
      if (ch->desc) {
	STATE(ch->desc) = CON_DISCONNECT;
	/*
	 * For the 'if (d->character)' test in close_socket().
	 * -gg 3/1/98 (Happy anniversary.)
	 */
	ch->desc->character = NULL;
	ch->desc = NULL;
      }
      if (CONFIG_FREE_RENT)
	Crash_rentsave(ch, 0);
      else
	Crash_idlesave(ch);
      mudlog(CMP, LVL_GOD, TRUE, "%s force-rented and extracted (idle).", GET_NAME(ch));
      add_llog_entry(ch, LAST_IDLEOUT);
      extract_char(ch);
    }
  }
}

void recover_update(void)
{
  struct char_data *i, *next_char;
  struct follow_type *k, *next;
  int cost = 0;
  int powerlevel;
  bool found = FALSE;
  
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
	
	if (GET_ADD_HIT(i) < 0 && !IS_NPC(i)) /* restore bugged negative add nen */
      GET_ADD_HIT(i) = 0;
	if (GET_HIT(i) < 5 && !IS_NPC(i)) /* restore bugged nen */
	  GET_HIT(i) = 5;
	if (GET_MANA(i) < 0 && !IS_NPC(i)) /* restore bugged stamina */
	  GET_MANA(i) = 0;
	if (GET_POS(i) == POS_STUNNED) {	  
	  GET_MANA(i) += 1;      
	} else if (GET_POS(i) > POS_STUNNED) {
      if (i->followers) { /* check for nen beasts and clone for costing */
	  for (k = i->followers; k; k = next) {
		next = k->next;
		switch (GET_MOB_VNUM(k->follower)) {
		  case 10:
		    if ((GET_HIT(i) - 4) < (GET_TOTAL_HIT(i) / 50) || GET_MANA(i) < 2) {			
			  act("$n disappears suddenly.", TRUE, k->follower, 0, 0, TO_ROOM);			
			  extract_char(k->follower);			
		    } else {
			  found = TRUE;
			  if (IS_CLERIC(i)) {
		        GET_HIT(i) -= (GET_TOTAL_HIT(i) / 50);			
		        cost += 2;
			  } else {
				GET_HIT(i) -= (GET_TOTAL_HIT(i) / 25);			
		        cost += 4;
		      }
		    }
			break;
		  case 11:
		    if ((GET_HIT(i) - 4) < (GET_TOTAL_HIT(i) / 100) || GET_MANA(i) < 1) {			
			  act("$n falls and decomposes.", TRUE, k->follower, 0, 0, TO_ROOM);			
			  extract_char(k->follower);			
		    } else {
			  found = TRUE;
		      if (IS_MANIPULATOR(i))
		        cost += 1;
			  else
			    cost += 2;		    
		    }
			break;
		  case 35:
		    if ((GET_HIT(i) - 4) < (GET_TOTAL_HIT(i) / 100) || GET_MANA(i) < 1) {			
			  act("$n disappears suddenly.", TRUE, k->follower, 0, 0, TO_ROOM);			
			  extract_char(k->follower);			
		    } else {
			  found = TRUE;
		      if (IS_THIEF(i)) {
		        GET_HIT(i) -= (GET_TOTAL_HIT(i) / 100);			
		        cost += 1;
			  } else {
			  	GET_HIT(i) -= (GET_TOTAL_HIT(i) / 50);			
		        cost += 2;
		      }
		    }
			break;
		  default:
		    if (!AFF_FLAGGED(k->follower, AFF_CHARM))
			  break;
		    if ((GET_HIT(i) - 4) < (GET_TOTAL_HIT(i) / 100) || GET_MANA(i) < 1) {			
			  act("$n regained control!", TRUE, k->follower, 0, 0, TO_ROOM);	
			  stop_follower(k->follower);
		    } else {
			  found = TRUE;
		      if (IS_MANIPULATOR(i) && IS_NPC(k->follower))
		        cost += 1;
			  else
			    cost += 2; 
			}
			break;
	    }
	  }
        if (found){	  
	      GET_MANA(i) -= cost;	
	      if (cost >= GET_MANA(i)) {
		    if (!IS_NPC(i))
			  GET_POS(i) = POS_STUNNED;
            else
			  GET_POS(i) = POS_DEAD;
	      }
        }
	  }
	  if (AFF_FLAGGED(i, AFF_ENHANCE)) {
		if (GET_MANA(i) < 3 || GET_POS(i) == POS_STUNNED) {
		  affect_from_char(i, SKILL_ENHANCE);
		  send_to_char(i, "Your body can no longer sustain the enhanced form and returns to normal.\r\n");
		  act("$n body can no longer sustain the enhanced form and returns to normal.", TRUE, i, 0, 0, TO_ROOM);
	    } else
		  GET_MANA(i) -= 1;
	  }
	  GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_TOTAL_HIT(i));
      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));	  
	if (GET_POS(i) != POS_SLEEPING) {
	  if (GET_MANA(i) <= 15 && GET_POS(i) != POS_RESTING) {
		GET_HIT(i) -= GET_TOTAL_HIT(i) / 20;
		if (GET_HIT(i) >= 5) {		
		act("$n is dizzy.", TRUE, i, 0, 0, TO_ROOM);   
	    send_to_char(i, "You feel dizzy.\r\n");
		} else {			
            if (!IS_NPC(i)) {
			  update_pos(i);
              act("$n lays unconscious.", TRUE, i, 0, 0, TO_ROOM);
              send_to_char(i, "You lays unconscious.\r\n");	
	          GET_HIT(i) = 5;
	          GET_MANA(i) = 0;			  
              continue;
			} else {
			  GET_POS(i) = POS_DEAD;
			  update_pos(i);
			  continue;
			}
		}
	  } else if (GET_MANA(i) <= 25 && rand_number(0, 1) == 1) {
	    act("$n are panting.", TRUE, i, 0, 0, TO_ROOM);   
	    send_to_char(i, "You are panting, damn near out of breath.\r\n");
	  } else if (GET_MANA(i) <= 35 && rand_number(0, 2) == 2) {
	    act("$n is soaked in sweat..", TRUE, i, 0, 0, TO_ROOM);   
	    send_to_char(i, "You're soaked in sweat.\r\n");
	  } else if (GET_MANA(i) <= 50 && rand_number(0, 3) == 3) {
	    act("A drop of sweat runs down of $n's face.", TRUE, i, 0, 0, TO_ROOM);   
	    send_to_char(i, "A drop of sweat runs down from your face.\r\n");
	  }
	}
      if (AFF_FLAGGED(i, AFF_POISON))
	    if (damage(i, i, GET_HIT(i) / 100, SPELL_POISON) == -1)
	      continue;	/* Oops, they died. -gg 6/24/98 */
    } else if (GET_POS(i) == POS_INCAP) {
      if (damage(i, i, 1, TYPE_SUFFERING) == -1)
	continue;
    } else if (GET_POS(i) == POS_MORTALLYW) {
      if (damage(i, i, 2, TYPE_SUFFERING) == -1)
	continue;    
    }
	if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_JAJANKEN)) {
	  if (GET_MANA(i) > 10 && GET_MAX_MOVE(i) < 370) {
	    GET_MANA(i) -= 10;
		GET_MAX_MOVE(i) += 100;		
		if (GET_MAX_MOVE(i) > 370)
	      send_to_char(i, "Your fist now has 4x times more aura!!!\r\n");
	    else if (GET_MAX_MOVE(i) > 270)
	      send_to_char(i, "Your fist now has 3x times more aura!!\r\n");
	    else if (GET_MAX_MOVE(i) > 170)	
	      send_to_char(i, "Your fist now has 2x times more aura!\r\n");
		act("$n concentrates some amount of aura over the fist.", TRUE, i, 0, 0, TO_ROOM);
		pracskill(i, SKILL_JAJANKEN, 15);
	  } else
		  do_jajanken(i, 0, 0, 0);
	}
	if (IS_NPC(i) && GET_MOB_VNUM(i) > 100 && GET_HIT(i) < GET_MAX_HIT(i)) {
	  powerlevel = ((GET_MAX_HIT(i) * GET_LEVEL(i)) / 100);
	  if (!FIGHTING(i))
        do_power(i, "up", 0, 0);	
	}
    update_pos(i);
  }
}
void power_update(void)
{
  struct char_data *i, *next_char; 
  int percent, prob;
  int power;
  
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    power = (GET_TOTAL_HIT(i) / 10);	

  if (PLR_FLAGGED(i, PLR_POWERUP) || PLR_FLAGGED(i, PLR_POWERDOWN) || MOB_FLAGGED(i, MOB_POWERUP)) {
	if (IS_NPC(i)) {
	  percent = 1;
	  prob = 101;
    } else {
      percent = rand_number(1, 101);
      prob = GET_SKILL(i, SKILL_POWER);
	  pracskill(i, SKILL_POWER, 20);
    }
	if ((PLR_FLAGGED(i, PLR_POWERUP) || MOB_FLAGGED(i, MOB_POWERUP)) && (GET_POS(i) < POS_FIGHTING)){
	  if (IS_NPC(i))
	    REMOVE_BIT_AR(MOB_FLAGS(i), MOB_POWERUP);
	  else
		REMOVE_BIT_AR(PLR_FLAGS(i), PLR_POWERUP);
	  send_to_char(i, "You stop to power up.\r\n"); 
	} else if (PLR_FLAGGED(i, PLR_POWERDOWN) && GET_POS(i) < POS_SITTING) {
		REMOVE_BIT_AR(PLR_FLAGS(i), PLR_POWERDOWN);
		send_to_char(i, "You stop to power down.\r\n"); 
	}
  }
  
  if (PLR_FLAGGED(i, PLR_POWERUP) || MOB_FLAGGED(i, MOB_POWERUP)) {  
    if (percent > prob){
	  send_to_char(i, "You lost your concentration!\r\n");	
    } else {  
    if ((GET_HIT(i) + power) >= GET_TOTAL_HIT(i)) {
	  GET_HIT(i) = GET_TOTAL_HIT(i);
      send_to_char(i, "You reached your maximum power!\r\n");
	  act("$n reached the maximum power!", TRUE, i, 0, 0, TO_ROOM);
	  if (IS_NPC(i))
	    REMOVE_BIT_AR(MOB_FLAGS(i), MOB_POWERUP);
	  else
	    REMOVE_BIT_AR(PLR_FLAGS(i), PLR_POWERUP);
    } else {  
      GET_HIT(i) += power;  
    send_to_char(i, "A strong aura flows all over your body.\r\n");
	act("A strong aura flows all over $n body.", TRUE, i, 0, 0, TO_ROOM);
    }  
    if (!IS_NPC(i) || (IS_NPC(i) && GET_POS(i) == POS_FIGHTING))  
      GET_MANA(i) = (GET_MANA(i) - 5);
    }
  }  
   else if (PLR_FLAGGED(i, PLR_POWERDOWN)) {
	if (percent > prob){
	send_to_char(i, "You lost your concentration!\r\n");	
    } else {
	if (GET_HIT(i) > 5)
	    GET_HIT(i) -= power;	  
      if (GET_HIT(i) <= 5) {
	    GET_HIT(i) = 5;
		send_to_char(i, "You reached your minimum power.\r\n");
		act("$n reached the minimum power.", TRUE, i, 0, 0, TO_ROOM);
		REMOVE_BIT_AR(PLR_FLAGS(i), PLR_POWERDOWN);
	  } else {
      send_to_char(i, "An aura leaks calmly from your body.\r\n");
	  act("An aura leaks calmly from $n body.", TRUE, i, 0, 0, TO_ROOM);
	  }
	  if (((rand_number(0, 100) + wis_app[GET_WIS(i)].bonus) >= 90) && GET_LEVEL(i) < 7) {
	    gain_exp(i, (GET_TOTAL_HIT(i) / 100));
      }
    }	  
   }	 	    
  } // end of for	
}

void second_update(void)
{
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing;
  
    /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    
	if (i->char_specials.cooldown >= 0) {
	  if (i->char_specials.cooldown <= 1)
		REMOVE_BIT_AR(AFF_FLAGS(i), AFF_HEALED);  
	  i->char_specials.cooldown -= 1;
	}    
  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;	/* Next in object list */
	
	if (j->carried_by && GET_MOB_SPEC(j->carried_by))
	  continue;
	
	if (IS_CARD(j)) {
	  int mcard = 0;
	  if (GET_OBJ_TIMER(j) > 0)
	    GET_OBJ_TIMER(j)--;
	  if (GET_OBJ_TIMER(j) == 0) {		  
		if (j->carried_by) {
	      mcard += make_card(j->carried_by, j, TRUE);
		  if (mcard == 0 || GET_OBJ_TYPE(j) == ITEM_SPELLCARD) {
			send_to_char(j->carried_by, "NOOOOO! %s you were holding expires and vanishes in a puff of smoke.\r\n", j->short_description);
			extract_obj(j);
		  }
	    } else if (j->worn_by) {
		  mcard += make_card(j->worn_by, j, TRUE);
		  if (mcard == 0 || GET_OBJ_TYPE(j) == ITEM_SPELLCARD) {
			send_to_char(j->worn_by, "NOOOOO! Your %s expires and vanishes in a puff of smoke.\r\n", j->short_description);
			extract_obj(j);
		  }
	    } else if (j->in_obj && GET_OBJ_VNUM(j->in_obj) != 3203) {
			if (GET_OBJ_TYPE(j) == ITEM_SPELLCARD)
			  extract_obj(j);
		    else
		      make_card_cnt(j);		  
	    } else if (IN_ROOM(j) != NOWHERE) {
			if (GET_OBJ_TYPE(j) == ITEM_SPELLCARD) {
			  send_to_room(IN_ROOM(j), "NOOOOO! %s expires and vanishes in a puff of smoke on the ground.\r\n", j->short_description);
			  extract_obj(j);
			} else
			  make_card_room(j);			
		}
	  }
	}		
  }
}

/* Update PCs, NPCs, and objects */
void point_update(void)
{
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
	
	if (!IS_NPC(i) && !PRF_FLAGGED(i, PRF_AFK)) {
    gain_condition(i, HUNGER, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
	}

    if (!IS_NPC(i)) {
      update_char_objects(i);
      (i->char_specials.timer)++;
      if (GET_LEVEL(i) < CONFIG_IDLE_MAX_LEVEL)
	check_idling(i);
    }
	update_pos(i);
  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;	/* Next in object list */
	
	if (j->carried_by && GET_MOB_SPEC(j->carried_by))
	  continue;
	
    /* If this is a corpse */
    if (IS_CORPSE(j) ) {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
	GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j)) {

	if (j->carried_by)
	  act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((IN_ROOM(j) != NOWHERE) && (world[IN_ROOM(j)].people)) {
	  act("$p vanishes in a poof of smoke.",
	      TRUE, world[IN_ROOM(j)].people, j, 0, TO_ROOM);
	  act("$p vanishes in a poof of smoke.",
	      TRUE, world[IN_ROOM(j)].people, j, 0, TO_CHAR);
	}
	for (jj = j->contains; jj; jj = next_thing2) {
	  next_thing2 = jj->next_content;	/* Next in inventory */
	  obj_from_obj(jj);

	  if (j->in_obj)
	    obj_to_obj(jj, j->in_obj);
	  else if (j->carried_by)
	    obj_to_room(jj, IN_ROOM(j->carried_by));
	  else if (IN_ROOM(j) != NOWHERE)
	    obj_to_room(jj, IN_ROOM(j));
	  else
	    core_dump();
	}
	extract_obj(j);
      }
    }
    /* If the timer is set, count it down and at 0, try the trigger
     * note to .rej hand-patchers: make this last in your point-update() */
    else if (GET_OBJ_TIMER(j)>0) {
      GET_OBJ_TIMER(j)--;
      if (!GET_OBJ_TIMER(j))
        timer_otrigger(j);
    }
  }

  /* Take 1 from the happy-hour tick counter, and end happy-hour if zero */
       if (HAPPY_TIME > 1)  HAPPY_TIME--;
  else if (HAPPY_TIME == 1)   /* Last tick - set everything back to zero */
  {
    HAPPY_QP = 0;
    HAPPY_EXP = 0;
    HAPPY_GOLD = 0;
    HAPPY_TIME = 0;
   game_info("Happy hour has ended!");
  }
}

/* Note: amt may be negative */
int increase_gold(struct char_data *ch, int amt)
{
  int curr_gold;

  curr_gold = GET_GOLD(ch);

  if (amt < 0) {
    GET_GOLD(ch) = MAX(0, curr_gold+amt);
    /* Validate to prevent overflow */
    if (GET_GOLD(ch) > curr_gold) GET_GOLD(ch) = 0;
  } else {
    GET_GOLD(ch) = MIN(MAX_GOLD, curr_gold+amt);
    /* Validate to prevent overflow */
    if (GET_GOLD(ch) < curr_gold) GET_GOLD(ch) = MAX_GOLD;
  }
  if (GET_GOLD(ch) == MAX_GOLD)
    send_to_char(ch, "%sYou have reached the maximum gold!\r\n%sYou must spend it or bank it before you can gain any more.\r\n", QBRED, QNRM);

  return (GET_GOLD(ch));
}

int decrease_gold(struct char_data *ch, int deduction)
{
  int amt;
  amt = (deduction * -1);
  increase_gold(ch, amt);
  return (GET_GOLD(ch));
}

int increase_bank(struct char_data *ch, int amt)
{
  int curr_bank;

  if (IS_NPC(ch)) return 0;

  curr_bank = GET_BANK_GOLD(ch);

  if (amt < 0) {
    GET_BANK_GOLD(ch) = MAX(0, curr_bank+amt);
    /* Validate to prevent overflow */
    if (GET_BANK_GOLD(ch) > curr_bank) GET_BANK_GOLD(ch) = 0;
  } else {
    GET_BANK_GOLD(ch) = MIN(MAX_BANK, curr_bank+amt);
    /* Validate to prevent overflow */
    if (GET_BANK_GOLD(ch) < curr_bank) GET_BANK_GOLD(ch) = MAX_BANK;
  }
  if (GET_BANK_GOLD(ch) == MAX_BANK)
    send_to_char(ch, "%sYou have reached the maximum bank balance!\r\n%sYou cannot put more into your account unless you withdraw some first.\r\n", QBRED, QNRM);
  return (GET_BANK_GOLD(ch));
}

int decrease_bank(struct char_data *ch, int deduction)
{
  int amt;
  amt = (deduction * -1);
  increase_bank(ch, amt);
  return (GET_BANK_GOLD(ch));
}

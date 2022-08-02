/**************************************************************************
*  File: act.offensive.c                                   Part of tbaMUD *
*  Usage: Player-level commands of an offensive nature.                   *
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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "act.h"
#include "fight.h"
#include "mud_event.h"
#include "constants.h"

ACMD(do_assist)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char(ch, "You're already fighting!  How can you assist someone else?\r\n");
    return;
  }
  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Whom do you wish to assist?\r\n");
  else if (!(helpee = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (helpee == ch)
    send_to_char(ch, "You can't help yourself any more than this!\r\n");
  else {
    /*
     * Hit the same enemy the person you're helping is.
     */
    if (FIGHTING(helpee))
      opponent = FIGHTING(helpee);
    else
      for (opponent = world[IN_ROOM(ch)].people;
	   opponent && (FIGHTING(opponent) != helpee);
	   opponent = opponent->next_in_room);

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
         /* prevent accidental pkill */
    else if (!CONFIG_PK_ALLOWED && !IS_NPC(opponent))
      send_to_char(ch, "You cannot kill other players.\r\n");
    else {
      send_to_char(ch, "You join the fight!\r\n");
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}

ACMD(do_punch)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  
  one_argument(argument, arg);
  
  if (!*arg)
    send_to_char(ch, "Hit who?\r\n");
  else if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "That target is not here.\r\n");
  else if (vict == ch) {
    send_to_char(ch, "You hit yourself...OUCH!.\r\n");
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {	
	send_to_char(ch, "\tBYou retracts your left arm...\tn\r\n");
    send_to_char(vict, "\tR%s retracts %s left arm...\tn\r\n", GET_NAME(ch), HSSH(ch));
	act("$n retracts $s left arm...", FALSE, ch, 0, vict, TO_NOTVICT);
    GET_HTIMER(ch) = 0;
	GET_HTIMER(ch) = (3 * PASSES_PER_SEC);	
	if (GET_POS(ch) > POS_STUNNED && (FIGHTING(ch) == NULL))
      set_fighting(ch, vict);
    if (GET_POS(vict) > POS_STUNNED && (FIGHTING(vict) == NULL))
      set_fighting(vict, ch);
  }
}

ACMD(do_hit)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;  

 one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Hit who?\r\n");
  else if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "That target is not here.\r\n");
  else if (vict == ch) {
    send_to_char(ch, "You hit yourself...OUCH!.\r\n");
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {
//    if (!CONFIG_PK_ALLOWED && !IS_NPC(vict) && !IS_NPC(ch)) 
//      check_killer(ch, vict);
    if (!IS_NPC(ch) && !IS_NPC(vict)) {
      if (IS_GOOD(ch) && (IS_NEUTRAL(vict) || IS_GOOD(vict))) {
	    send_to_char(ch, "Your mom never tells you that good people don't kill each other?!\r\n");
	    send_to_char(ch, "\tBGOOD \tcalign only can attack \tREVIL\tc players.\tn\r\n");
	    return;
	  } else if (IS_NEUTRAL(ch) && IS_GOOD(vict)) {
	    send_to_char(ch, "You are not a bad person, why start now?\r\n");
	    send_to_char(ch, "NEUTRAL \tcalign only can attack other \tnNEUTRAL \tcor \tREVIL\tc players.\tn\r\n");
	    return;
      } else if ((IS_AFFECTED(ch, AFF_NOPK) || IS_AFFECTED(vict, AFF_NOPK) || GET_TOTAL_HIT(vict) < (GET_TOTAL_HIT(ch) / 1.5) || GET_LEVEL(vict) == 1)) {
	    send_to_char(ch, "Pick on someone your own size!\r\n");	    
	    return;
	  }
	}

    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      if (GET_DEX(ch) > GET_DEX(vict) || (GET_DEX(ch) == GET_DEX(vict) && rand_number(1, 2) == 1))  /* if faster */
        hit(ch, vict, TYPE_UNDEFINED);  /* first */
      else hit(vict, ch, TYPE_UNDEFINED);  /* or the victim is first */
        WAIT_STATE(ch, PULSE_VIOLENCE + 2); 
    } else 
      send_to_char(ch, "You're fighting the best you can!\r\n"); 
  } 
}

ACMD(do_kill)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;  

  if (GET_LEVEL(ch) < LVL_GRGOD || IS_NPC(ch) || !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Kill who?\r\n");
  } else {
    if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
      send_to_char(ch, "That target is not here.\r\n");
    else if (ch == vict)
      send_to_char(ch, "Your mother would be so sad.. :(\r\n");
    else {
      act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}

ACMD(do_snapneck)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;  

  if (IS_NPC(ch))    
    return;
  
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Snap whose neck?\r\n");
  } else {
    if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
      send_to_char(ch, "That target is not here.\r\n");
    else if (ch == vict)
      send_to_char(ch, "Your mother would be so sad.. :(\r\n");
    else if (GET_LEVEL(vict) == 1 || IS_AFFECTED(ch, AFF_NOPK))
	  send_to_char(ch, "P'takh! You should be ashamed when trying to kill a protected player!\r\n");
    else if (GET_POS(vict) > POS_STUNNED)
	  send_to_char(ch, "You can't finish %s right now!\r\n", GET_NAME(vict));
    else {
      act("You snap $M's neck! *CRACK*", FALSE, ch, 0, vict, TO_CHAR);
      act("$N snaps your neck!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n brutally snap $N's neck!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}

ACMD(do_backstab)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob, i;  
  
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_BACKSTAB)) {
    send_to_char(ch, "Unpractised you are, a master you must seek, hum.\r\n");
    return;
  }

  one_argument(argument, buf);
  
  if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
	  send_to_char(ch, "Flurry of blows on who?\r\n");
      return;
	}
  }
  if (vict == ch) {
    send_to_char(ch, "Want a free massage, eh?\r\n");
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL) || (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_NOKILL))) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }
  
  percent = rand_number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BACKSTAB);
  

  if (AWAKE(vict) && (percent > prob)) {	  
	damage(ch, vict, 0, TYPE_UNDEFINED);
	damage(ch, vict, 0, TYPE_UNDEFINED);
  } else {
	if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
	hit(vict, ch, TYPE_UNDEFINED);
	hit(ch, vict, TYPE_UNDEFINED);    
	WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
    }
	hit(ch, vict, TYPE_UNDEFINED);
	if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_SECOND_ATTACK) > 0) {
	  for (i = 142; i < 145; i++) {		
        if (GET_SKILL(ch, i) > 0 && (GET_SKILL(ch, i) + rand_number(0, 100)) > 100) {
	      hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
		  pracskill(ch, i, 20); 
		}
	  }
	}	  
  }
	  
/*	for (i = 0; i > u; i++)
	  send_to_char(ch, "attack damage: %d\r\n", i);
      damage(ch, vict, 0, SKILL_BACKSTAB);    
  } else {
	for (i = 0; i > u; i++)
	  send_to_char(ch, "attack hit: %d\r\n", i);
      hit(ch, vict, SKILL_BACKSTAB); 
  }
*/
  pracskill(ch, SKILL_BACKSTAB, 10);
  GET_MANA(ch) -= 4;
  WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
}

ACMD(do_order)
{
  char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
  bool found = FALSE;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char(ch, "Order who to do what?\r\n");
  else if (!(vict = get_char_vis(ch, name, NULL, FIND_CHAR_ROOM)) && !is_abbrev(name, "followers"))
    send_to_char(ch, "That person isn't here.\r\n");
  else if (ch == vict)
    send_to_char(ch, "You obviously suffer from skitzofrenia.\r\n");
  else {
    if (AFF_FLAGGED(ch, AFF_CHARM)) {
      send_to_char(ch, "Your superior would not aprove of you giving orders.\r\n");
      return;
    }
    if (vict) {
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !AFF_FLAGGED(vict, AFF_CHARM))
        act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
        send_to_char(ch, "%s", CONFIG_OK);
        command_interpreter(vict, message);
      }
    } else {			/* This is order "followers" */
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);

      for (k = ch->followers; k; k = k->next) {
        if (IN_ROOM(ch) == IN_ROOM(k->follower))
          if (AFF_FLAGGED(k->follower, AFF_CHARM)) {
            found = TRUE;
            command_interpreter(k->follower, message);
          }
      }
      if (found)
        send_to_char(ch, "%s", CONFIG_OK);
      else
        send_to_char(ch, "Nobody here is a loyal subject of yours!\r\n");
    }
  }
}

ACMD(do_flee)
{
  int i, attempt, loss;
  struct char_data *was_fighting;

  if (GET_POS(ch) < POS_FIGHTING) {
    send_to_char(ch, "You are in pretty bad shape, unable to flee!\r\n");
    return;
  }
  if (AFF_FLAGGED(ch, AFF_BUNGEE_GUM)) {
	send_to_char(ch, "You are unable to flee while gummed!\r\n");
	act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
	return;
  }
  for (i = 0; i < 6; i++) {
    attempt = rand_number(0, DIR_COUNT - 1); /* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH)) {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      was_fighting = FIGHTING(ch);
      if (do_simple_move(ch, attempt, TRUE)) {
	send_to_char(ch, "You flee head over heels.\r\n");
        if (was_fighting && !IS_NPC(ch)) {
	  loss = GET_MAX_HIT(was_fighting) - GET_HIT(was_fighting);
	  loss *= (GET_LEVEL(was_fighting)) * 2;
      GET_MANA(ch) = (GET_MANA(ch) - 3);
//	  gain_exp(ch, -loss);
//	  send_to_char(ch, "You lose %d points of experience.\r\n", (loss / 2));
        }
      if (FIGHTING(ch)) 
        stop_fighting(ch); 
      if (was_fighting && ch == FIGHTING(was_fighting))
        stop_fighting(was_fighting); 
      } else {
	act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  }
  send_to_char(ch, "PANIC!  You couldn't escape!\r\n");
}

ACMD(do_bash)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_BASH)) {
    send_to_char(ch, "Unpractised you are, a master you must seek, hum.\r\n");
    return;
  }  
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char(ch, "Bash who?\r\n");
      return;
    }
  }
  if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL) || (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_NOKILL))) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict)) {
    if (IS_GOOD(ch) && (IS_NEUTRAL(vict) || IS_GOOD(vict))) {
	  send_to_char(ch, "Your mom never tells you that good people don't kill each other?!\r\n");
	  send_to_char(ch, "\tBGOOD \tcalign only can attack \tREVIL\tc players.\tn\r\n");
	  return;
    } else if (IS_NEUTRAL(ch) && IS_GOOD(vict)) {
	  send_to_char(ch, "You are not a bad person, why start now?\r\n");
	  send_to_char(ch, "NEUTRAL \tcalign only can attack other \tnNEUTRAL \tcor \tREVIL\tc players.\tn\r\n");
	  return;
    } else if ((IS_AFFECTED(ch, AFF_NOPK) || IS_AFFECTED(vict, AFF_NOPK) || GET_TOTAL_HIT(vict) < (GET_TOTAL_HIT(ch) / 1.5) || GET_LEVEL(vict) == 1)) {
	  send_to_char(ch, "Pick on someone your own size!\r\n");	    
	  return;
    }
  }
  if (MOB_FLAGGED(vict, MOB_NOKILL)) {
    send_to_char(ch, "Your attack has suddenly blocked!\r\n");
    return;
  }
  if (IS_AFFECTED(vict, AFF_NOPK)) {
	send_to_char(ch, "The target player is protected by NO_PK flag.\r\n");
	return;
  }
  if (GET_MANA(ch) <= 4) {
	send_to_char(ch, "You are too tired to do this.\r\n");
	return;
  }
  
  percent = rand_number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BASH);
  
  pracskill(ch, SKILL_BASH, 10);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BASH);
    GET_POS(ch) = POS_SITTING;
  } else {
    /*
     * If we bash a player and they wimp out, they will move to the previous
     * room before we set them sitting.  If we try to set the victim sitting
     * first to make sure they don't flee, then we can't bash them!  So now
     * we only set them sitting if they didn't flee. -gg 9/21/98
     */
    if (damage(ch, vict, GET_HIT(ch) / rand_number(10, 12), SKILL_BASH) > 0) {	/* -1 = dead, 0 = miss */
      WAIT_STATE(vict, PULSE_VIOLENCE);
      if (IN_ROOM(ch) == IN_ROOM(vict))
        GET_POS(vict) = POS_SITTING;
    }
  }
  if (IS_WARRIOR(ch))
    GET_MANA(ch) = (GET_MANA(ch) - 2);
  else
	GET_MANA(ch) = (GET_MANA(ch) - 4);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_rescue)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_RESCUE)) {
    send_to_char(ch, "Unpractised you are, a master you must seek, hum.\r\n");
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    send_to_char(ch, "Whom do you want to rescue?\r\n");
    return;
  }
  if (vict == ch) {
    send_to_char(ch, "What about fleeing instead?\r\n");
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char(ch, "How can you rescue someone you are trying to kill?\r\n");
    return;
  }
  for (tmp_ch = world[IN_ROOM(ch)].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if ((FIGHTING(vict) != NULL) && (FIGHTING(ch) == FIGHTING(vict)) && (tmp_ch == NULL)) {
     tmp_ch = FIGHTING(vict);
     if (FIGHTING(tmp_ch) == ch) {
     send_to_char(ch, "You have already rescued %s from %s.\r\n", GET_NAME(vict), GET_NAME(FIGHTING(ch)));
     return;
  }
  }

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (GET_MANA(ch) <= 3) {
	send_to_char(ch, "You are too tired to do this.\r\n");
	return;
  }
  percent = rand_number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_RESCUE);
  
  pracskill(ch, SKILL_RESCUE, 10);

  if (percent > prob) {
    send_to_char(ch, "You fail the rescue!\r\n");
    return;
  }
  send_to_char(ch, "Banzai!  To the rescue...\r\n");
  act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
  act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

  if (FIGHTING(vict) == tmp_ch)
    stop_fighting(vict);
  if (FIGHTING(tmp_ch))
    stop_fighting(tmp_ch);
  if (FIGHTING(ch))
    stop_fighting(ch);

  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);
  
  GET_MANA(ch) = (GET_MANA(ch) - 3);
  WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
}

EVENTFUNC(event_whirlwind)
{
  struct char_data *ch, *tch;
  struct mud_event_data *pMudEvent;
  struct list_data *room_list;
  int count;
	
  /* This is just a dummy check, but we'll do it anyway */
  if (event_obj == NULL)
    return 0;
	  
  /* For the sake of simplicity, we will place the event data in easily
   * referenced pointers */  
  pMudEvent = (struct mud_event_data *) event_obj;
  ch = (struct char_data *) pMudEvent->pStruct;    
  
  /* When using a list, we have to make sure to allocate the list as it
   * uses dynamic memory */
  room_list = create_list();
  
  /* We search through the "next_in_room", and grab all NPCs and add them
   * to our list */
  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)  
    if (IS_NPC(tch))
      add_to_list(tch, room_list);
      
  /* If our list is empty or has "0" entries, we free it from memory and
   * close off our event */    
  if (room_list->iSize == 0) {
    free_list(room_list);
    send_to_char(ch, "There is no one in the room to whirlwind!\r\n");
    return 0;
  }
  
  /* We spit out some ugly colour, making use of the new colour options,
   * to let the player know they are performing their whirlwind strike */
  send_to_char(ch, "\t[f313]You deliver a vicious \t[f014]\t[b451]WHIRLWIND!!!\tn\r\n");
  
  /* Lets grab some a random NPC from the list, and hit() them up */
  for (count = dice(1, 4); count > 0; count--) {
    tch = random_from_list(room_list);
    hit(ch, tch, TYPE_UNDEFINED);
  }
  
  /* Now that our attack is done, let's free out list */
  free_list(room_list);
  
  /* The "return" of the event function is the time until the event is called
   * again. If we return 0, then the event is freed and removed from the list, but
   * any other numerical response will be the delay until the next call */
  if (GET_SKILL(ch, SKILL_WHIRLWIND) < rand_number(1, 101)) {
    send_to_char(ch, "You stop spinning.\r\n");
    return 0;
  } else
    return 1.5 * PASSES_PER_SEC;
}

/* The "WHIRLWIND" skill is designed to provide a basic understanding of the
 * mud event and list systems. */
ACMD(do_whirlwind)
{
  
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_WHIRLWIND)) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }
  
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }

  if (GET_POS(ch) < POS_FIGHTING) {
    send_to_char(ch, "You must be on your feet to perform a whirlwind.\r\n");
    return;    
  }

  /* First thing we do is check to make sure the character is not in the middle
   * of a whirl wind attack.
   * 
   * "char_had_mud_event() will sift through the character's event list to see if
   * an event of type "eWHIRLWIND" currently exists. */
  if (char_has_mud_event(ch, eWHIRLWIND)) {
    send_to_char(ch, "You are already attempting that!\r\n");
    return;   
  }

  send_to_char(ch, "You begin to spin rapidly in circles.\r\n");
  act("$n begins to rapidly spin in a circle!", FALSE, ch, 0, 0, TO_ROOM);
  
  /* NEW_EVENT() will add a new mud event to the event list of the character.
   * This function below adds a new event of "eWHIRLWIND", to "ch", and passes "NULL" as
   * additional data. The event will be called in "3 * PASSES_PER_SEC" or 3 seconds */
  NEW_EVENT(eWHIRLWIND, ch, NULL, 3 * PASSES_PER_SEC);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_remote_punch)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int i, u, percent, prob;
  
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_REMOTE_PUNCH)) {
    send_to_char(ch, "Unpractised you are, a master you must seek, hum.\r\n");
    return;
  }  
  
  one_argument(argument, arg);    
  
  if (!*arg && FIGHTING(ch)) {
	vict = FIGHTING(ch);
	goto end;
  } else if (!*arg) {
	send_to_char(ch, "Remote Punch who?\r\n");
	return;
  }

  /* Check if the vict is already in fight or in room */    
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
	  goto end;
	}
  } else if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL) || ROOM_FLAGGED(IN_ROOM(vict), ROOM_PEACEFUL) || (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_NOKILL))) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;  
  } else if (vict) {
	goto end;
  }
  /* vict not in room */    
	room_vnum vroom;
	room_rnum rroom;
	u = (zone_table[world[IN_ROOM(ch)].zone].number * 100);	
	for (i = (u + 100); u < i; u++) {
	  vroom = u;
	  rroom = real_room(vroom);
	  if (rroom == NOWHERE || ROOM_FLAGGED(rroom, ROOM_PEACEFUL) || ROOM_FLAGGED(rroom, ROOM_NOMAGIC))
		continue;
      if (world[rroom].people) {	
	    for (vict = world[rroom].people; vict; vict = vict->next_in_room)
          if (isname(arg, vict->player.name))
            if (CAN_SEE(ch, vict))
			  break;
	  }
	  if (vict)
		break;
    }
  
  if (!vict) {
	send_to_char(ch, "You failed to lock on a target.\r\n");	
	return;
  } 
  
  rroom = IN_ROOM(ch);
  char_from_room(ch);
  char_to_room(ch, IN_ROOM(vict));
  look_at_room(ch, 0);
  send_to_room(rroom, "%s punches the ground and %s body is sucked in nen wormhole-like!\r\n", GET_NAME(ch), HSHR(ch));
  
  end:  
  
  percent = GET_SAVE(vict, SAVING_SPELL) + rand_number(1, 100);
  prob = GET_SKILL(ch, SKILL_REMOTE_PUNCH);  

  if (percent < prob) {
    hit(ch, vict, SKILL_REMOTE_PUNCH);   
  } else
    damage(ch, vict, 0, SKILL_REMOTE_PUNCH);
  GET_MANA(ch) = (GET_MANA(ch) - 4);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_kick)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_KICK)) {
    send_to_char(ch, "Unpractised you are, a master you must seek, hum.\r\n");
    return;
  }  

  one_argument(argument, arg);

  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char(ch, "Kick who?\r\n");
      return;
    }
  }
  if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL) || (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_NOKILL))) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }
  if (!IS_NPC(ch) && !IS_NPC(vict)) {
    if (IS_GOOD(ch) && (IS_NEUTRAL(vict) || IS_GOOD(vict))) {
	  send_to_char(ch, "Your mom never tells you that good people don't kill each other?!\r\n");
	  send_to_char(ch, "\tBGOOD \tcalign only can attack \tREVIL\tc players.\tn\r\n");
	  return;
    } else if (IS_NEUTRAL(ch) && IS_GOOD(vict)) {
	  send_to_char(ch, "You are not a bad person, why start now?\r\n");
	  send_to_char(ch, "NEUTRAL \tcalign only can attack other \tnNEUTRAL \tcor \tREVIL\tc players.\tn\r\n");
	  return;
    } else if ((IS_AFFECTED(ch, AFF_NOPK) || IS_AFFECTED(vict, AFF_NOPK) || GET_TOTAL_HIT(vict) < (GET_TOTAL_HIT(ch) / 1.5) || GET_LEVEL(vict) == 1)) {
	  send_to_char(ch, "Pick on someone your own size!\r\n");
	  return;
    }
  }
  if (GET_MANA(ch) <= 2) {
	send_to_char(ch, "You are too tired to do this.\r\n");
	return;
  }
  /* 101% is a complete failure */
  percent = ((10 - (compute_armor_class(vict) / 10)) * 2) + rand_number(1, 101);
  prob = GET_SKILL(ch, SKILL_KICK);

  if (percent < prob) {
    if (!IS_WARRIOR(ch))
      damage(ch, vict, GET_HIT(ch) / rand_number((31 - GET_STR(ch)), (33 - GET_STR(ch))), SKILL_KICK);
    else
	  damage(ch, vict, GET_HIT(ch) / rand_number((29 - GET_STR(ch)), (31 - GET_STR(ch))), SKILL_KICK);   
  } else
    damage(ch, vict, 0, SKILL_KICK);	

  pracskill(ch, SKILL_KICK, 10);
  if (IS_WARRIOR(ch))
    GET_MANA(ch) = (GET_MANA(ch) - 3);
  else
	GET_MANA(ch) = (GET_MANA(ch) - 4);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_jajanken)
{
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_JAJANKEN)) {
    send_to_char(ch, "Unpractised you are, a master you must seek, hum.\r\n");
    return;
  }  
  
  if (PLR_FLAGGED(ch, PLR_JAJANKEN) || (!PLR_FLAGGED(ch, PLR_JAJANKEN) && GET_MAX_MOVE(ch) > 170)) {
	send_to_char(ch, "You stop charging.\r\n");
	REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_JAJANKEN);	
	if (GET_MAX_MOVE(ch) > 370)
	  GET_MAX_MOVE(ch) -= 300;
	else if (GET_MAX_MOVE(ch) > 270)
	  GET_MAX_MOVE(ch) -= 200;
	else if (GET_MAX_MOVE(ch) > 170)	
	  GET_MAX_MOVE(ch) -= 100;
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	return;	
  }
  
  if (GET_POS(ch) == POS_FIGHTING) {
	send_to_char(ch, "You can't do this in middle of a fight!\r\n");
	return;
  }
  
  if (GET_MANA(ch) <= 10) {
	send_to_char(ch, "You are too tired to do this.\r\n");
	return;
  }
  do_say(ch, "Saisho wa guu...", 0, 0);
  GET_MANA(ch) = (GET_MANA(ch) - 10);
  send_to_char(ch, "You concentrates an amount of aura over your fist.\r\n");
  SET_BIT_AR(PLR_FLAGS(ch), PLR_JAJANKEN);
  pracskill(ch, SKILL_JAJANKEN, 15);
}

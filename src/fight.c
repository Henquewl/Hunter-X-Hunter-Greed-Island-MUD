/**************************************************************************
*  File: fight.c                                           Part of tbaMUD *
*  Usage: Combat system.                                                  *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#define __FIGHT_C__

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "constants.h"
#include "dg_scripts.h"
#include "act.h"
#include "class.h"
#include "fight.h"
#include "shop.h"
#include "quest.h"


/* locally defined global variables, used externally */
/* head of l-list of fighting chars */
struct char_data *combat_list = NULL;
/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},    /* 0 */
  {"sting", "stings"},
  {"whip", "whips"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},  /* 5 */
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"}, /* 10 */
  {"pierce", "pierces"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"stab", "stabs"}
};

/* local (file scope only) variables */
static struct char_data *next_combat_list = NULL;

/* local file scope utility functions */
static void perform_group_gain(struct char_data *ch, int base, struct char_data *victim);
static void dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type);
static void make_corpse(struct char_data *ch);
static void change_alignment(struct char_data *ch, struct char_data *victim);
static void group_gain(struct char_data *ch, struct char_data *victim);
static void solo_gain(struct char_data *ch, struct char_data *victim);
/** @todo refactor this function name */
static char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural);
static int compute_thaco(struct char_data *ch, struct char_data *vict);


#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))
#define DAM_ADD(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].todam + GET_DAMROLL(ch))
#define DAM_ADD_PLUS(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].todam + GET_DAMROLL(ch) + GET_LEVEL(ch))
/* The Fight related routines */
void appear(struct char_data *ch)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INVISIBLE);
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

  if (GET_LEVEL(ch) < LVL_IMMORT)
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  else
    act("You feel a strange presence as $n appears, seemingly from nowhere.",
	FALSE, ch, 0, 0, TO_ROOM);
}

int compute_armor_class(struct char_data *ch)
{
  int armorclass = GET_AC(ch);

  if (AWAKE(ch))
    armorclass += dex_app[GET_DEX(ch)].defensive * 10;

  return (MAX(-1000, armorclass));      /* -100 is lowest */
}

void update_pos(struct char_data *victim)
{
  if ((GET_HIT(victim) >= 5 && GET_MANA(victim) > 0) && (GET_POS(victim) > POS_STUNNED)) {
	if ((!IS_NPC(victim)) && ((GET_HIT(victim) >= GET_TOTAL_HIT(victim)) && (GET_MANA(victim) >= GET_MAX_MANA(victim)))) {
	  if ((GET_POS(victim) == POS_SLEEPING) && (!AFF_FLAGGED(victim, AFF_SLEEP))) {
		send_to_char(victim, "You feel well rested and wake up.\r\n");
		act("$n awakens.", TRUE, victim, 0, 0, TO_ROOM);
		GET_POS(victim) = POS_STANDING;
      } else if (GET_POS(victim) == POS_RESTING)
        do_stand(victim, 0, 0, 0);
	  else
        return;
	} else {
	  return;
	}
  } else if (GET_HIT(victim) < 5 || GET_MANA(victim) <= 0) {
	if (IS_NPC(victim))
	  GET_POS(victim) = POS_DEAD;
    else {      			  
	  GET_POS(victim) = POS_STUNNED;
	  if (PLR_FLAGGED(victim, PLR_POWERUP)){
		REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_POWERUP);
		send_to_char(victim, "You stop to power up.\r\n"); 
	  }	else if (PLR_FLAGGED(victim, PLR_POWERDOWN)){
		REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_POWERDOWN);
		send_to_char(victim, "You stop to power down.\r\n"); 
	  }	
      if (FIGHTING(victim)) {
		stop_fighting(FIGHTING(victim));
		stop_fighting(victim);
      }	  
	}
  } else if (GET_MANA(victim) > 15) {
    GET_POS(victim) = POS_STANDING;
	act("$n regain consciousness.", TRUE, victim, 0, 0, TO_ROOM);    
	send_to_char(victim, "You regain consciousness.\r\n");
  }
}

void check_killer(struct char_data *ch, struct char_data *vict)
{
  if (PLR_FLAGGED(vict, PLR_KILLER) || PLR_FLAGGED(vict, PLR_THIEF))
    return;
  if (PLR_FLAGGED(ch, PLR_KILLER))
    return;
  if (!IS_NPC(vict))
    stop_fighting(ch);
    return;
  
  SET_BIT_AR(PLR_FLAGS(ch), PLR_KILLER);
  send_to_char(ch, "Now you are a PLAYER KILLER...\r\n");
  mudlog(BRF, LVL_IMMORT, TRUE, "PC Killer bit set on %s for kill %s at %s.",
    GET_NAME(ch), GET_NAME(vict), world[IN_ROOM(vict)].name);	 
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
  if (ch == vict)
    return;

  if (FIGHTING(ch)) {
    core_dump();
    return;
  }

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (AFF_FLAGGED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP);

  FIGHTING(ch) = vict;
  GET_POS(ch) = POS_FIGHTING;
  
  if (PLR_FLAGGED(vict, PLR_KILLER) || PLR_FLAGGED(ch, PLR_KILLER))
	return;

//  if (!CONFIG_PK_ALLOWED)
	//send_to_char(ch, "Kill a player will turn you a PLAYER KILLER...\r\n");
    
}

/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
  struct char_data *temp;

  if (ch == next_combat_list)
    next_combat_list = ch->next_fighting;
  
  REMOVE_FROM_LIST(ch, combat_list, next_fighting);
  ch->next_fighting = NULL;
  FIGHTING(ch) = NULL;
  GET_POS(ch) = POS_STANDING;
  update_pos(ch);  
  
}

static void make_corpse(struct char_data *ch)
{
  char buf2[MAX_NAME_LENGTH + 64];
  struct obj_data *corpse, *o;
  struct obj_data *money;
  struct obj_data *booster;
  int i, x, y;

  corpse = create_obj();

  corpse->item_number = NOTHING;
  IN_ROOM(corpse) = NOWHERE;
  corpse->name = strdup("corpse");

  snprintf(buf2, sizeof(buf2), "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = strdup(buf2);

  snprintf(buf2, sizeof(buf2), "the corpse of %s", GET_NAME(ch));
  corpse->short_description = strdup(buf2);

  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
  for(x = y = 0; x < EF_ARRAY_MAX || y < TW_ARRAY_MAX; x++, y++) {
    if (x < EF_ARRAY_MAX)
      GET_OBJ_EXTRA_AR(corpse, x) = 0;
    if (y < TW_ARRAY_MAX)
      corpse->obj_flags.wear_flags[y] = 0;
  }
  SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE);
  SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);
  GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
  GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */
  GET_OBJ_WEIGHT(corpse) = ((GET_WEIGHT(ch) + IS_CARRYING_W(ch)) + 800);
  GET_OBJ_RENT(corpse) = 100000;
  if (IS_NPC(ch))
    GET_OBJ_TIMER(corpse) = CONFIG_MAX_NPC_CORPSE_TIME;
  else
    GET_OBJ_TIMER(corpse) = CONFIG_MAX_PC_CORPSE_TIME;

  /* transfer character's inventory to the corpse */
  corpse->contains = ch->carrying;
  for (o = corpse->contains; o != NULL; o = o->next_content)
    o->in_obj = corpse;
  object_list_new_owner(corpse, NULL);

  /* transfer character's equipment to the corpse */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i)) {
      remove_otrigger(GET_EQ(ch, i), ch);
      obj_to_obj(unequip_char(ch, i), corpse);
    }
	
  /* transfer gold */
  if (GET_GOLD(ch) > 0) {
    /* following 'if' clause added to fix gold duplication loophole. The above
     * line apparently refers to the old "partially log in, kill the game
     * character, then finish login sequence" duping bug. The duplication has
     * been fixed (knock on wood) but the test below shall live on, for a
     * while. -gg 3/3/2002 */
    if (IS_NPC(ch) || ch->desc) {
      money = create_money(GET_GOLD(ch));
      obj_to_obj(money, corpse);
    }
    GET_GOLD(ch) = 0;
  }
  
  /* booster pack prize */
  if ((rand_number(1, 100) + (GET_LEVEL(ch) / 10) + HAPPY_QP) >= 100) {
	booster = read_object(3250, VIRTUAL);
    obj_to_obj(booster, corpse);
	act("$n's body has $p!!!", TRUE, ch, booster, 0, TO_ROOM);
  } 
   
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  obj_to_room(corpse, IN_ROOM(ch));
}

/* When ch kills victim */
static void change_alignment(struct char_data *ch, struct char_data *victim)
{
  /* new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast. */
  GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) / 16;
}

void death_cry(struct char_data *ch)
{
  int door;
  
  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);

  for (door = 0; door < DIR_COUNT; door++)
    if (CAN_GO(ch, door))
      send_to_room(world[IN_ROOM(ch)].dir_option[door]->to_room, "Your blood freezes as you hear someone's death cry.\r\n");
}

void raw_kill(struct char_data * ch, struct char_data * killer)
{
  struct obj_data *obj, *next_obj, *card, *next_card;
  struct affected_type af;

  if (FIGHTING(ch))
    stop_fighting(ch);

  while (ch->affected)
    affect_remove(ch, ch->affected);

  /* To make ordinary commands work in scripts.  welcor*/
  GET_POS(ch) = POS_STANDING;

  if (killer) {
    if (death_mtrigger(ch, killer))
      death_cry(ch);
  } else
    death_cry(ch);

  if (killer)
    autoquest_trigger_check(killer, ch, NULL, AQ_MOB_KILL);

  /* Alert Group if Applicable */
  if (GROUP(ch))
    send_to_group(ch, GROUP(ch), "%s has died.\r\n", GET_NAME(ch));

  if (!IS_NPC(ch) && !IS_NPC(killer)) {
    gain_exp(killer, (GET_MAX_HIT(ch) / 10));
    gain_exp(ch, -(GET_MAX_HIT(ch) / 10));
	if (GET_EQ(ch, WEAR_HOLD)) {
	  if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_RESTRICTED || GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_SPELLCARD)
		extract_obj(GET_EQ(ch, WEAR_HOLD));		
	}
	for (obj = ch->carrying; obj; obj = next_obj) { // in inventory
	  next_obj = obj->next_content;
	  if (GET_OBJ_VNUM(obj) == 3203) { // is this book?
		for (card = obj->contains; card; card = next_card) {
		  next_card = card->next_content;
		  if (GET_OBJ_TYPE(card) == ITEM_RESTRICTED || GET_OBJ_TYPE(card) == ITEM_SPELLCARD)
		    extract_obj(card); // junk all good stuff
		}		  
      }
      if (GET_OBJ_TYPE(obj) == ITEM_RESTRICTED || GET_OBJ_TYPE(obj) == ITEM_SPELLCARD || 
		  (GET_OBJ_TYPE(obj) == ITEM_CARD && SCRIPT(obj))) // is this a restricted or spell card?
		extract_obj(obj);	  
	}
	Crash_rentsave(ch, 0);
	change_alignment(killer, ch);
	check_killer(killer, ch);
	new_affect(&af);
    af.spell = SKILL_NOPK;
    af.duration = 47;
    SET_BIT_AR(af.bitvector, AFF_NOPK);
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  
  update_pos(ch);
  if (IS_NPC(ch) && GET_MOB_VNUM(ch) > 100)
    make_corpse(ch);
  extract_char(ch);	

  if (killer) {
    autoquest_trigger_check(killer, NULL, NULL, AQ_MOB_SAVE);
    autoquest_trigger_check(killer, NULL, NULL, AQ_ROOM_CLEAR);
  }
}

void die(struct char_data * ch, struct char_data * killer)
{
  gain_exp(ch, -(GET_MAX_HIT(ch) / 10));
  if (!IS_NPC(ch)) {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_KILLER);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_THIEF);
  }
  raw_kill(ch, killer);
}

static void perform_group_gain(struct char_data *ch, int base,
			     struct char_data *victim)
{
  int share, hap_share;

  share = MIN(CONFIG_MAX_EXP_GAIN, MAX(1, base));

  if ((IS_HAPPYHOUR) && (IS_HAPPYEXP))
  {
    /* This only reports the correct amount - the calc is done in gain_exp */
    hap_share = share + (share * ((float)HAPPY_EXP / (float)(100)));
    share = MIN(CONFIG_MAX_EXP_GAIN, MAX(1, hap_share));
  }
  if (share < 9) {
    share = 9;
    if ((rand_number(1, 100) + int_app[GET_INT(ch)].learn) >= 100) {
      GET_PRACTICES(ch)++;
	  send_to_char(ch, "\tDYour upgrade points was increased by \tG1\tD!\tn\r\n");
	}
  } else if ((rand_number(1, 100) + int_app[GET_INT(ch)].learn) >= 95) {
    GET_PRACTICES(ch)++;
    send_to_char(ch, "\tDYour upgrade points was increased by \tG1\tD!\tn\r\n");
  }  
  
  gain_exp(ch, share);
  change_alignment(ch, victim);
}

static void group_gain(struct char_data *ch, struct char_data *victim)
{
  int tot_members = 0, base, tot_gain;
  struct char_data *k;
  
  while ((k = (struct char_data *) simple_list(GROUP(ch)->members)) != NULL)
    if (IN_ROOM(ch) == IN_ROOM(k))
      tot_members++;

  /* round up to the nearest tot_members */
  tot_gain = (GET_EXP(victim) / 3) + tot_members - 1;

  /* prevent illegal xp creation when killing players */
  if (!IS_NPC(victim))
    tot_gain = MIN(CONFIG_MAX_EXP_LOSS * 2 / 3, tot_gain);

  if (tot_members >= 1)
    base = MAX(1, tot_gain / tot_members);
  else
    base = 0;

  while ((k = (struct char_data *) simple_list(GROUP(ch)->members)) != NULL)
    if (IN_ROOM(k) == IN_ROOM(ch))
      perform_group_gain(k, base, victim);
}

static void solo_gain(struct char_data *ch, struct char_data *victim)
{
  int exp;  

  /* Calculate level-difference bonus */
/*  if (IS_NPC(ch))
    exp += MAX(0, (exp * MIN(4, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
  else
    exp += MAX(0, (exp * MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
*/
  
  if (GET_MAX_HIT(victim) < (GET_TOTAL_HIT(ch) / 2))
	exp = 1;
  else
    exp = (GET_MAX_HIT(victim) / 20);  

  exp = MAX(exp, 1);

/*  if (IS_HAPPYHOUR && IS_HAPPYEXP) {
    happy_exp = exp + (exp * ((float)HAPPY_EXP / (float)(100)));
    exp = MAX(happy_exp, 1);
  }
*/
  if (exp < 9)
	exp = 9;
  else {
    if (GET_LEVEL(victim) > GET_LEVEL(ch)) {
	  if ((rand_number(1, 100) + int_app[GET_INT(ch)].learn) > 90) {
	    GET_PRACTICES(ch)++;
	    send_to_char(ch, "\tDYour upgrade points was increased by \tG1\tD!\tn\r\n");
	  }
	} else if (GET_LEVEL(victim) == (GET_LEVEL(ch) - 1)) {
	  if ((rand_number(1, 100) + int_app[GET_INT(ch)].learn) >= 100) {
	    GET_PRACTICES(ch)++;
	    send_to_char(ch, "\tDYour upgrade points was increased by \tG1\tD!\tn\r\n");
	  }
	} else if (GET_LEVEL(victim) < (GET_LEVEL(ch) - 1)) {
	  if ((rand_number(1, 100) + int_app[GET_INT(ch)].learn) >= 105) {
	    GET_PRACTICES(ch)++;
	    send_to_char(ch, "\tDYour upgrade points was increased by \tG1\tD!\tn\r\n");
	  }
	} else {
	  if ((rand_number(1, 100) + int_app[GET_INT(ch)].learn) > 95) {	
	    GET_PRACTICES(ch)++;
	    send_to_char(ch, "\tDYour upgrade points was increased by \tG1\tD!\tn\r\n");
	  }
	}    
  }  
    if (!IS_NPC(victim) || (IS_NPC(victim) && GET_MOB_VNUM(victim) > 100))
      gain_exp(ch, exp);
  change_alignment(ch, victim);
}

static char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural)
{
  static char buf[256];
  char *cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
	for (; *weapon_plural; *(cp++) = *(weapon_plural++));
	break;
      case 'w':
	for (; *weapon_singular; *(cp++) = *(weapon_singular++));
	break;
      default:
	*(cp++) = '#';
	break;
      }
    } else
      *(cp++) = *str;

    *cp = 0;
  }				/* For */ 

  return (buf);
}

/* message for doing damage with a weapon */
static void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		      int w_type)
{
  char *buf;
  char hit[MAX_STRING_LENGTH];
  int msgnum;

  static struct dam_weapon_type {
    const char *to_room;
    const char *to_char;
    const char *to_victim;
  } dam_weapons[] = {

    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

    {
      "$n tries to #w $N, but misses.",	/* 0: 0     */
      "You try to #w $N, but miss.",
      "$n tries to #w you, but misses."
    },

    {
      "$n tickles $N as $e #W $M.",	/* 1: 1..2  */
      "You tickle $N as you #w $M.",
      "$n tickles you as $e #W you."
    },

    {
      "$n barely #W $N.",		/* 2: 3..4  */
      "You barely #w $N.",
      "$n barely #W you."
    },

    {
      "$n #W $N.",			/* 3: 5..6  */
      "You #w $N.",
      "$n #W you."
    },

    {
      "$n #W $N hard.",			/* 4: 7..10  */
      "You #w $N hard.",
      "$n #W you hard."
    },

    {
      "$n #W $N very hard.",		/* 5: 11..14  */
      "You #w $N very hard.",
      "$n #W you very hard."
    },

    {
      "$n #W $N \tYextremely\tD hard.",	/* 6: 15..19  */
      "You #w $N \tYextremely\tB hard.",
      "$n #W you \tYextremely\tR hard."
    },

    {
      "$n \tWmassacres\tD $N to small fragments with $s #w.",	/* 7: 19..23 */
      "You \tWmassacres\tB $N to small fragments with your #w.",
      "$n \tWmassacres\tR you to small fragments with $s #w."
    },

    {
      "$n \ty**\tWOBLITERATES\ty**\tD $N with $s deadly #w!!",	/* 8: > 23   */
      "You \ty**\tWOBLITERATES\ty**\tB $N with your deadly #w!!",
      "$n \ty**\tWOBLITERATES\ty**\tR you with $s deadly #w!!"
    }
  };

  w_type -= TYPE_HIT;		/* Change to base of table with text */

  if (dam >= GET_HIT(victim))				msgnum = 8;
  else if (dam >= (GET_HIT(victim) / 2))	msgnum = 7;
  else if (dam >= (GET_HIT(victim) / 3.25))	msgnum = 6;
  else if (dam >= (GET_HIT(victim) / 4.5))	msgnum = 5;
  else if (dam >= (GET_HIT(victim) / 5.75))	msgnum = 4;
  else if (dam >= (GET_HIT(victim) / 7))	msgnum = 3;
  else if (dam >= (GET_HIT(victim) / 8.5))	msgnum = 2;
  else if (dam > 0)							msgnum = 1;
  else										msgnum = 0;

  /* damage message to onlookers */
  sprintf(hit, " \tG=\tC[\tW%d\tC]\tG=\tn", dam);
  buf = replace_string(dam_weapons[msgnum].to_room,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);  
  strcat(buf, hit);
  send_to_room(IN_ROOM(ch), "\tD");
  act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
  send_to_char(ch, CCNRM(ch, C_CMP));
  send_to_char(ch, CCNRM(victim, C_CMP));

  /* damage message to damager */
  sprintf(hit, " \tG=\tC[\tW%d\tC]\tG=\tn", dam);
  buf = replace_string(dam_weapons[msgnum].to_char,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  strcat(buf, hit);
  send_to_char(ch, "\tB");
  act(buf, FALSE, ch, NULL, victim, TO_CHAR);  
  send_to_char(ch, CCNRM(ch, C_CMP));

  /* damage message to damagee */
  sprintf(hit, " \tG=\tC[\tW%d\tC]\tG=\tn", dam);
  buf = replace_string(dam_weapons[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);  
  strcat(buf, hit);
  send_to_char(victim, "\tR");
  act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
  send_to_char(victim, CCNRM(victim, C_CMP));
}

/*  message for doing damage with a spell or skill. Also used for weapon
 *  damage on miss and death blows. */
int skill_message(int dam, struct char_data *ch, struct char_data *vict,
		      int attacktype)
{
  int i, j, nr;
  struct message_type *msg;
  
  char buf[MAX_STRING_LENGTH], hit[MAX_STRING_LENGTH];

  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);  
  
  snprintf(hit, sizeof(hit), " \tG=\tC[\tW%d\tC]\tG=\tn", dam);

  /* @todo restructure the messages library to a pointer based system as
   * opposed to the current cyclic location system. */
  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
        msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMPL)) {
        act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
        act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
        act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      } else if (dam != 0) {
        /*
         * Don't send redundant color codes for TYPE_SUFFERING & other types
         * of damage without attacker_msg.
         */		
        if (GET_POS(vict) == POS_DEAD) {		  
          if (msg->die_msg.attacker_msg) {
			strcpy(buf, msg->die_msg.attacker_msg);
            strcat(buf, hit);
            send_to_char(ch, CBBLU(ch, C_CMP));
            act(buf, FALSE, ch, weap, vict, TO_CHAR);
            send_to_char(ch, CCNRM(ch, C_CMP));
          }

		  strcpy(buf, msg->die_msg.victim_msg);
          strcat(buf, hit);
          send_to_char(vict, CBRED(vict, C_CMP));
          act(buf, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
          send_to_char(vict, CCNRM(vict, C_CMP));
		  
		  strcpy(buf, msg->die_msg.room_msg);
          strcat(buf, hit);
          send_to_char(ch, CBBLK(ch, C_CMP));
		  send_to_char(vict, CBBLK(vict, C_CMP));
          act(buf, FALSE, ch, weap, vict, TO_NOTVICT);
		  send_to_char(ch, CCNRM(ch, C_CMP));
		  send_to_char(vict, CCNRM(vict, C_CMP));
        } else {		  
          if (msg->hit_msg.attacker_msg) {            
            strcpy(buf, msg->hit_msg.attacker_msg);
			strcat(buf, hit);
            send_to_char(ch, CBBLU(ch, C_CMP));
			act(buf, FALSE, ch, weap, vict, TO_CHAR);
            send_to_char(ch, CCNRM(ch, C_CMP));				
          }
          
		  strcpy(buf, msg->hit_msg.victim_msg);
          strcat(buf, hit);
          send_to_char(vict, CBRED(vict, C_CMP));
          act(buf, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
          send_to_char(vict, CCNRM(vict, C_CMP));
		  
		  strcpy(buf, msg->hit_msg.room_msg);
          strcat(buf, hit);
		  send_to_char(ch, CBBLK(ch, C_CMP));
		  send_to_char(vict, CBBLK(vict, C_CMP));
          act(buf, FALSE, ch, weap, vict, TO_NOTVICT);
		  send_to_char(ch, CCNRM(ch, C_CMP));
		  send_to_char(vict, CCNRM(vict, C_CMP));
        }
      } else if (ch != vict) {	/* Dam == 0 */	    
        if (msg->miss_msg.attacker_msg) {
          send_to_char(ch, CCYEL(ch, C_CMP));
          act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
          send_to_char(ch, CCNRM(ch, C_CMP));
        }

        send_to_char(vict, CCRED(vict, C_CMP));
        act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
        send_to_char(vict, CCNRM(vict, C_CMP));
		
		send_to_char(ch, CCCYN(ch, C_CMP));
		send_to_char(vict, CCCYN(vict, C_CMP));
        act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
		send_to_char(ch, CCNRM(ch, C_CMP));
		send_to_char(vict, CCNRM(vict, C_CMP));
      }	  
      return (1);
    }
  }
  return (0);
}

/* This function returns the following codes:
 *	< 0	Victim died.
 *	= 0	No damage.
 *	> 0	How much damage done. */
int damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype)
{
  long local_gold = 0, happy_gold = 0;
  int percent, prob, pos = rand_number(1, 15);
  int exp;
  char local_buf[256];
  struct char_data *tmp_char;
  struct obj_data *corpse_obj;
//  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);
//  struct obj_data *equip = GET_EQ(victim, pos);   

  if (GET_POS(victim) <= POS_DEAD) {
    /* This is "normal"-ish now with delayed extraction. -gg 3/15/2001 */
    if (PLR_FLAGGED(victim, PLR_NOTDEADYET) || MOB_FLAGGED(victim, MOB_NOTDEADYET))
      return (-1);

    log("SYSERR: Attempt to damage corpse '%s' in room #%d by '%s'.",
        GET_NAME(victim), GET_ROOM_VNUM(IN_ROOM(victim)), GET_NAME(ch));
    die(victim, ch);
    return (-1);			/* -je, 7/7/92 */
  }

  /* peaceful rooms */
  if (ch->nr != real_mobile(DG_CASTER_PROXY) &&
      ch != victim && ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
	send_to_char(victim, "This room just has such a peaceful, easy feeling...\r\n");
    return (0);
  }

  /* shopkeeper and MOB_NOKILL protection */
  if (!ok_damage_shopkeeper(ch, victim) || MOB_FLAGGED(victim, MOB_NOKILL)) {
    send_to_char(ch, "Your attack has suddenly blocked!\r\n");
    return (0);
  }

  /* You can't damage an immortal! */
  if (!IS_NPC(victim) && ((GET_LEVEL(victim) >= LVL_IMMORT) && PRF_FLAGGED(victim, PRF_NOHASSLE)))
    dam = 0;

  if (victim != ch) {
    /* Start the attacker fighting the victim */
    if (GET_POS(ch) > POS_STUNNED && (FIGHTING(ch) == NULL))
      set_fighting(ch, victim);

    /* Start the victim fighting the attacker */
    if (GET_POS(victim) > POS_STUNNED && (FIGHTING(victim) == NULL)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch))
	remember(victim, ch);
    }
  }

  /* If you attack a pet, it hates your guts */
  if (victim->master == ch)
    stop_follower(victim);

  /* If the attacker is invisible, he becomes visible */
  if (AFF_FLAGGED(ch, AFF_INVISIBLE) || AFF_FLAGGED(ch, AFF_HIDE))
    appear(ch);

  if (PLR_FLAGGED(ch, PLR_JAJANKEN)) {	
	if (attacktype == SPELL_FIREBALL || attacktype == SPELL_MAGIC_MISSILE)
	  do_say(ch, "JAN KEN PAA!!!", 0, 0);
    else if (attacktype < NUM_SPELLS  && (attacktype != SPELL_FIREBALL || attacktype != SPELL_MAGIC_MISSILE))
	  do_say(ch, "JAN KEN CHI!!!", 0, 0);	
    else
	  do_say(ch, "JAN KEN GUU!!!", 0, 0);
    dam = (dam * GET_MAX_MOVE(ch)) / (200 - GET_SKILL(ch, SKILL_JAJANKEN));
	REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_JAJANKEN);
	if (GET_MAX_MOVE(ch) > 370)
	  GET_MAX_MOVE(ch) -= 300;
	else if (GET_MAX_MOVE(ch) > 270)
	  GET_MAX_MOVE(ch) -= 200;
	else if (GET_MAX_MOVE(ch) > 170)	
	  GET_MAX_MOVE(ch) -= 100;
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  }

  /* Cut damage in half if victim has sanct, to a minimum 1 */
  if (AFF_FLAGGED(victim, AFF_SANCTUARY) && dam >= 2)
    dam /= 2;  

  /* When PC hit: 20% chance to damage the weapon or 10% if it enchanted */  
/*  if (!IS_NPC(ch) && weap) {
	if (dam > 0 && (GET_OBJ_TYPE(weap) == ITEM_WEAPON && 
		GET_OBJ_DURABILITY(weap) > 0) && rand_number(1, 5) == 5) {
	  if (OBJ_FLAGGED(weap, ITEM_ENFOLDED) && rand_number(0, 1) == 1)
	    GET_OBJ_DURABILITY(weap)--;	    
	  else if (!OBJ_FLAGGED(weap, ITEM_ENFOLDED))
	    GET_OBJ_DURABILITY(weap)--;
      if (GET_OBJ_DURABILITY(weap) <= 0) {
	    act("$n's wielded $p broke!", TRUE, ch, weap, 0, TO_ROOM);
        send_to_char(ch, "\tR%s was \x1B[5mDESTROYED!\tn\r\n", weap->short_description);
	    extract_obj(weap);
	    affect_total(ch);
	  }	
    }
  }
  if (!IS_NPC(victim) && equip) {
*/  /* 10% chance to damage 1 random equip, even if the attack was missed */
/*    if (GET_OBJ_DURABILITY(equip) > 0 && rand_number(1, 10) == 10) {
	  if (!OBJ_FLAGGED(equip, ITEM_ENFOLDED) && OBJ_FLAGGED(weap, ITEM_ENFOLDED))
	    GET_OBJ_DURABILITY(equip) -= 3;
	  else if ((!OBJ_FLAGGED(equip, ITEM_ENFOLDED) && !OBJ_FLAGGED(weap, ITEM_ENFOLDED)) || 
	  	  (OBJ_FLAGGED(equip, ITEM_ENFOLDED) && OBJ_FLAGGED(weap, ITEM_ENFOLDED)))
	    GET_OBJ_DURABILITY(equip) -= 2;
      else
	    GET_OBJ_DURABILITY(equip)--;
	  if (GET_OBJ_DURABILITY(equip) <= 0) {
	    act("$n's equipped $p broke!", TRUE, victim, equip, 0, TO_ROOM);
        send_to_char(victim, "\tR%s was \x1B[5mDESTROYED!\tn\r\n", equip->short_description);
	    extract_obj(equip);
	    affect_total(victim);
	  }	  
    }  
  }
*/if (dam > 0) {
    if (!IS_NPC(victim) && GET_SKILL(victim, SKILL_INSTANT_FORTIFY) > 0) {	  
	    percent = rand_number(1, 200);
        prob = GET_SKILL(victim, SKILL_INSTANT_FORTIFY);
	    pracskill(victim, SKILL_INSTANT_FORTIFY, 15);
        if (percent > prob)
	      GET_MANA(victim) -= 1;
	} else
	    GET_MANA(victim) -= 1;
  }
  /* Set the maximum damage per round and subtract the hit points */
  
  GET_HIT(victim) -= dam;    

  update_pos(victim);

  /* skill_message sends a message from the messages file in lib/misc.
   * dam_message just sends a generic "You hit $n extremely hard.".
   * skill_message is preferable to dam_message because it is more
   * descriptive.
   *
   * If we are _not_ attacking with a weapon (i.e. a spell), always use
   * skill_message. If we are attacking with a weapon: If this is a miss or a
   * death blow, send a skill_message if one exists; if not, default to a
   * dam_message. Otherwise, always send a dam_message. */
  if (!IS_WEAPON(attacktype))
    skill_message(dam, ch, victim, attacktype);
  else {
    if (GET_POS(victim) == POS_DEAD || dam == 0) {
      if (!skill_message(dam, ch, victim, attacktype))
	dam_message(dam, ch, victim, attacktype);
    } else {
      dam_message(dam, ch, victim, attacktype);
    }
  }
  
    /* Gain exp for the hit */
  if ((rand_number(0, 100) + wis_app[GET_WIS(ch)].bonus) >= 100 && ch != victim){
	exp = (GET_MAX_HIT(victim) / 1000);
	if (exp < 9)
	  exp = 9;	
	gain_exp(ch, exp);	
  }
  
  if (IS_NPC(victim) && GET_POS(victim) <= POS_STUNNED)
	GET_POS(victim) = POS_DEAD;

  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char(victim, "You are mortally wounded, and will die soon, if not aided.\r\n");
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char(victim, "You are incapacitated and will slowly die, if not aided.\r\n");
    break;
  case POS_STUNNED:
      act("$n lays unconscious.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char(victim, "You lays unconscious.\r\n");
	  forget(ch, victim);
	  GET_HIT(victim) = 5;
	  GET_MANA(victim) = 0;
	  exp = (GET_MAX_HIT(victim) / 10);
	  if (GET_LEVEL(victim) > 1)
	    gain_exp(victim, -exp);
	  if (FIGHTING(victim)) {
	    stop_fighting(FIGHTING(victim));
	    stop_fighting(victim);
      }      
	break;
  case POS_DEAD:
    act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char(victim, "You are dead!  Sorry...\r\n");
    break;

  default:			/* >= POSITION SLEEPING */
    if (dam > (GET_MAX_HIT(victim) / 4))
      send_to_char(victim, "That really did HURT!\r\n");
    	
    if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 9)) {
//      send_to_char(victim, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n", CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
	  if (ch != victim && MOB_FLAGGED(victim, MOB_WIMPY))		
	do_flee(victim, NULL, 0, 0);
    }
    if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
	GET_HIT(victim) < GET_WIMP_LEV(victim) && GET_HIT(victim) > 0) {
      send_to_char(victim, "You wimp out, and attempt to flee!\r\n");
      do_flee(victim, NULL, 0, 0);
    }
    break;
  }

  /* Help out poor linkless people who are attacked */
  if (!IS_NPC(victim) && !(victim->desc) && GET_POS(victim) > POS_STUNNED) {
    do_flee(victim, NULL, 0, 0);
    if (!FIGHTING(victim)) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      GET_WAS_IN(victim) = IN_ROOM(victim);
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }

  /* prevent PK someone from fighting if they're stunned or worse */
  if (!IS_NPC(victim) && GET_POS(victim) <= POS_STUNNED && FIGHTING(victim) != NULL)
    stop_fighting(victim);
  
  if (!IS_NPC(victim)) {
	if (GET_POS(victim) == POS_STUNNED || GET_POS(victim) == POS_INCAP || GET_POS(victim) == POS_MORTALLYW)
	check_killer(ch, victim);  
  }
  /* Uh oh.  Victim died. */
  if (GET_POS(victim) == POS_DEAD) {
    if (ch != victim && (IS_NPC(victim) || victim->desc)) {
      if (GROUP(ch))
	group_gain(ch, victim);
      else
        solo_gain(ch, victim);
    }

    if (!IS_NPC(victim)) {
	  check_killer(ch, victim);
      mudlog(BRF, LVL_IMMORT, TRUE, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch), world[IN_ROOM(victim)].name);
      if (MOB_FLAGGED(ch, MOB_MEMORY))
	forget(ch, victim);
    }
    /* Cant determine GET_GOLD on corpse, so do now and store */
    if (IS_NPC(victim)) {
      if ((IS_HAPPYHOUR) && (IS_HAPPYGOLD))
      {
        happy_gold = (long)(GET_GOLD(victim) * (((float)(HAPPY_GOLD))/(float)100));
        happy_gold = MAX(0, happy_gold);
        increase_gold(victim, happy_gold);
      }
      local_gold = GET_GOLD(victim);
      sprintf(local_buf,"%ld", (long)local_gold);
    }

    die(victim, ch);
    if (GROUP(ch) && (local_gold > 0) && PRF_FLAGGED(ch, PRF_AUTOSPLIT) ) {
      generic_find("corpse", FIND_OBJ_ROOM, ch, &tmp_char, &corpse_obj);
      if (corpse_obj) {
        do_get(ch, "all.coin corpse", 0, 0);
        do_split(ch, local_buf, 0, 0);
      }
      /* need to remove the gold from the corpse */
    } else if (!IS_NPC(ch) && (ch != victim) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
      do_get(ch, "all.coin corpse", 0, 0);
    }
    if (!IS_NPC(ch) && (ch != victim) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
      do_get(ch, "all corpse", 0, 0);
    } else
	  do_look(ch, "in corpse", 0, 0);
    if (IS_NPC(victim) && !IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOSAC)) {
      do_sac(ch,"corpse",0,0);
    }
    return (-1);
  }
  return (dam);
}

/* Calculate the THAC0 of the attacker. 'victim' currently isn't used but you
 * could use it for special cases like weapons that hit evil creatures easier
 * or a weapon that always misses attacking an animal. */
static int compute_thaco(struct char_data *ch, struct char_data *victim)
{
  int calc_thaco;

  if (!IS_NPC(ch))
    calc_thaco = thaco(GET_CLASS(ch), GET_LEVEL(ch));
  else		/* THAC0 for monsters is set in the HitRoll */
    calc_thaco = 20;
  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= GET_HITROLL(ch);
  calc_thaco -= (int) ((GET_DEX(ch) - 11) / 2);	    /* Dex accuracy */
  calc_thaco -= (int) ((GET_INT(ch) - 11) / 3.5);	/* Intelligence helps! */
  calc_thaco -= (int) ((GET_WIS(ch) - 11) / 3.5);	/* So does wisdom */

  return calc_thaco;
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  int w_type, victim_ac, calc_thaco, diceroll, prob, percent, armor;
  int dam;
//  bool second = FALSE, third = FALSE, fourth = FALSE;

  /* Check that the attacker and victim exist */
  if (!ch || !victim) return;

  /* check if the character has a fight trigger */
  fight_mtrigger(ch);

  /* Do some sanity checking, in case someone flees, etc. */
  if (IN_ROOM(ch) != IN_ROOM(victim)) {
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }
  
  if (GET_POS(victim) <= POS_STUNNED && IS_NPC(ch)) {
	update_pos(victim);
	stop_fighting(ch);
    return;	
  }  
     
  /* Find the weapon type (for display purposes only) */
  if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else {
    if (IS_NPC(ch) && ch->mob_specials.attack_type != 0)
      w_type = ch->mob_specials.attack_type + TYPE_HIT;
    else
      w_type = TYPE_HIT;
  } 

  /* Calculate chance of hit. Lower THAC0 is better for attacker. */
  if (AFF_FLAGGED(ch, AFF_LUCK)) {
	calc_thaco = -21;
	affect_from_char(ch, SPELL_LUCK);
	send_to_char(ch, "You feel less lucky."); 
  } else
  calc_thaco = compute_thaco(ch, victim);

  /* Calculate the raw armor including magic armor.  Lower AC is better for defender. */
  victim_ac = compute_armor_class(victim) / 10;
  
  /* Calculate the % armor to lower ch damage output. */
  armor = (((compute_armor_class(victim) - 100) * -1) / 10);  
  
/*  if (IS_NPC(ch)){
    if (GET_LEVEL(ch) >= 30)
      u = 3;
    else if (GET_LEVEL(ch) >= 20)
	  u = 2;
	else if (GET_LEVEL(ch) >= 10)
	  u = 1;
  } else {
  if (GET_SKILL(ch, SKILL_SECOND_ATTACK) > 0)
	u++;
  if (GET_SKILL(ch, SKILL_THIRD_ATTACK) > 0)
	u++;  
  if (GET_SKILL(ch, SKILL_FOURTH_ATTACK) > 0)
	u++;
  }  
  
  while (i <= u) {
*/	  
  if (GET_POS(ch) == POS_SITTING)
	return;

  if (GET_POS(ch) <= POS_STUNNED || GET_POS(victim) <= POS_STUNNED) {
	stop_fighting(ch);
	stop_fighting(victim);
	return;
  }	

  /* roll the die and take your chances... */
/*  if (u >= 3){
	if (IS_NPC(ch))
	  diceroll = rand_number(1, 20);
    else {
    diceroll = (rand_number(1, 20) + ((GET_SKILL(ch, SKILL_FOURTH_ATTACK) - 100) / 5));	
	pracskill(ch, SKILL_FOURTH_ATTACK, 20);
	}
	fourth = TRUE;
  } else if (u == 2) {
	if (IS_NPC(ch))
	  diceroll = rand_number(1, 20);
    else {
	diceroll = (rand_number(1, 20) + ((GET_SKILL(ch, SKILL_THIRD_ATTACK) - 100) / 5));    
	pracskill(ch, SKILL_THIRD_ATTACK, 20);
	}
	third = TRUE;
  } else if (u == 1) {
	if (IS_NPC(ch))
	  diceroll = rand_number(1, 20);
    else {
	diceroll = (rand_number(1, 20) + ((GET_SKILL(ch, SKILL_SECOND_ATTACK) - 100) / 5));
    pracskill(ch, SKILL_SECOND_ATTACK, 20);
	}
	second = TRUE;
  } else
*/	  diceroll = rand_number(1, 20);
  

  /* report for debugging if necessary */
  if (CONFIG_DEBUG_MODE >= NRM)
    send_to_char(ch, "\t1Debug:\r\n   \t2Thaco: \t3%d\r\n   \t2AC: \t3%d\r\n   \t2Diceroll: \t3%d\tn\r\n", 
      calc_thaco, victim_ac, diceroll);

  /* Decide whether this is a hit or a miss.
   *  Victim asleep = hit, otherwise:
   *     1   = Automatic miss.
   *   2..19 = Checked vs. AC.
   *    20   = Automatic hit. */
  if (diceroll == 20 || !AWAKE(victim))
    dam = TRUE;
  else if (diceroll == 1)
    dam = FALSE;
  else
    dam = (calc_thaco - diceroll <= victim_ac);

  if (!dam) {
    /* the attacker missed the victim */
    damage(ch, victim, 0, w_type);
  } else if ((!IS_NPC(victim) && GET_SKILL(victim, SKILL_PARRY) > 0) && (rand_number(1, 10) == 10)) {
    if ((rand_number(1, 100) + dex_app_skill[GET_DEX(victim)].sneak + GET_SKILL(victim, SKILL_PARRY)) > 125)
	  damage(ch, victim, 0, TYPE_PARRY);
	if (GET_POS(ch) != POS_SITTING) {
      send_to_char(victim, "\tC[TECH BLOCK BONUS - %s is stunned!]\tn\r\n", GET_NAME(ch));	
	  WAIT_STATE(ch, PULSE_VIOLENCE);
	  if (IN_ROOM(victim) == IN_ROOM(ch))
        GET_POS(ch) = POS_SITTING;
	}
    pracskill(ch, SKILL_PARRY, 10);			
  } else if ((!IS_NPC(victim) && GET_SKILL(victim, SKILL_DODGE) > 0) && (rand_number(1, 5) == 5)) {
	  if ((rand_number(1, 100) + dex_app_skill[GET_DEX(victim)].hide + GET_SKILL(victim, SKILL_DODGE)) > 125)
		damage(ch, victim, 0, TYPE_DODGE);	
	  pracskill(ch, SKILL_DODGE, 20);        
  } else {
	  
	/* damage based on hitpoints */  
	dam = GET_HIT(ch);	
	  
/*    if (fourth == TRUE)
	  dam = dam / rand_number(13, 15);
    else if (third == TRUE)
	  dam = dam / rand_number(12, 14);
	else if (second == TRUE)
	  dam = dam / rand_number(11, 13);
    else
*/	  dam = dam / rand_number(10, 12);

    /* Maybe holding arrow? */
    if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON) {
      /* Add weapon-based damage if a weapon is being wielded */
      dam += ((dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) * 100) / (101 - GET_LEVEL(ch)));
    } else {
      /* If no weapon, add bare hand damage instead */
        if (IS_NPC(ch)) /* damage += damage * (weapon dice + level) / 100 = x% extra damage */
          dam += ((dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice) * 100) / (101 - GET_LEVEL(ch)));
//        else
//          dam += ((rand_number(1, 2) * 100) / (101 - GET_LEVEL(ch)));	/* Max 2 bare hand damage for players */
    }

    /* Include a damage multiplier if victim isn't ready to fight:
     * Position sitting  1.33 x normal
     * Position resting  1.66 x normal
     * Position sleeping 2.00 x normal
     * Position stunned  2.33 x normal
     * Position incap    2.66 x normal
     * Position mortally 3.00 x normal
     * Note, this is a hack because it depends on the particular
     * values of the POSITION_XXX constants. */
    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
  
    /* add % if has bonuses and victim is not a transmuter */
	if (IS_NPC(victim) || (!IS_NPC(victim) && GET_CLASS(victim) != CLASS_MAGIC_USER)) {
	  if (IS_NPC(ch) && DAM_ADD_PLUS(ch) > 0)
	    dam += ((dam * DAM_ADD_PLUS(ch)) / 100);
	  else if (!IS_NPC(ch) && DAM_ADD(ch) > 0)
        dam += ((dam * DAM_ADD(ch)) / 100);
    }
  
    /* reduce damage by % unless you are u have armor piercing */
	if (IS_NPC(ch) || (!IS_NPC(ch) && GET_CLASS(ch) != CLASS_THIEF))
	  if (armor > 0 && armor < 100)
	    dam -= ((dam * armor) / 100);

    /* at least 9 hp damage min per hit */
    dam = MAX(9, dam);	
/*
    if (type == SKILL_BACKSTAB)  
	  damage(ch, victim, dam, w_type);	 
    else if (type == SKILL_SECOND_ATTACK) {
      damage(ch, victim, dam, w_type);
	  damage(ch, victim, dam, w_type);
	} else if (type == SKILL_THIRD_ATTACK) {
      damage(ch, victim, dam, w_type);
	  damage(ch, victim, dam, w_type);
	  damage(ch, victim, dam, w_type);
	} else if (type == SKILL_FOURTH_ATTACK) {
      damage(ch, victim, dam, w_type);
	  damage(ch, victim, dam, w_type);
	  damage(ch, victim, dam, w_type);
	  damage(ch, victim, dam, w_type);
	} else */
    damage(ch, victim, dam, w_type);    
  }
   if (!IS_NPC(ch)) {
	if (!GET_SKILL(ch, SKILL_BAREHANDED_EXPERT) || (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON))
	  GET_MANA(ch) -= 1;
    else {
	  pracskill(ch, SKILL_BAREHANDED_EXPERT, 20);  
	  percent = rand_number(1, 200);
      prob = GET_SKILL(ch, SKILL_BAREHANDED_EXPERT);
      if (percent > prob)
	    GET_MANA(ch) -= 1;
	}
   }  
   if (GET_MANA(ch) <= 0 && IS_NPC(victim)) {
	 GET_HIT(ch) = 5;  
	 act("$n lays unconscious.", TRUE, ch, 0, 0, TO_ROOM);
     send_to_char(ch, "You lays unconscious.\r\n");	
	 gain_exp(ch, -(GET_MAX_HIT(ch) / 10));	          
   }	 
   update_pos(ch);

  /* check if the victim has a hitprcnt trigger */
  hitprcnt_mtrigger(victim);
}
/*
void power_up(void)
{
  struct char_data *ch;
  int i, limit;
  
  if (*arg || *arg > GET_MAX_HIT(ch))
    limit = GET_MAX_HIT(ch);  
  else
	limit = *arg;
  
  if (PLR_FLAGGED(ch, PLR_POWERUP)) {
    for (i = GET_HIT(ch); i >= limit; i++) {	  
	GET_MANA(ch) = (GET_MANA(ch) - 5);	  
	GET_HIT(ch) = ((GET_MAX_HIT(ch) * 10) / 100);
	send_to_char(ch, "A strongly aura flows all over your body.\r\n");	
    }  
    GET_HIT(ch) = limit;
    send_to_char(ch, "You have reached your desired enhancement!\r\n");
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_POWERUP);
  } else if (GET_MANA(ch) <= 0 && PLR_FLAGGED(ch, PLR_POWERUP) {
	send_to_char(ch, "Your body not support such effort and collapses!\r\n");
	update_pos(ch);
  } else {
	send_to_char(ch, "Your stop to enhance up.\r\n");  
  }
  
}
*/

void pracskill(struct char_data *ch, int skill_num, int chance)
{
  struct obj_data *booster;
  int i, skilladd, dice, found;
  
  if (!ch || IS_NPC(ch) || GET_SKILL(ch, skill_num) >= 100)
	return;

  if (!skill_num) {
	log("ERROR: pracskill called without skill_num");
	return;
  }	
  
  dice = rand_number(1, 20);
  
  for (i = 0; i < NUM_CLASSES; i++)
	if (GET_LEVEL(ch) >= spell_info[skill_num].min_level[i])
      found = i;    
  
  if ((GET_SKILL(ch, skill_num) < (prac_params[found][(int) GET_CLASS(ch)])) && (dice + (wis_app[GET_WIS(ch)].bonus) >= chance)){ 
    skilladd = GET_SKILL(ch, skill_num);
    skilladd += MIN(15, MAX(1, int_app[GET_INT(ch)].learn));
    SET_SKILL(ch, skill_num, MIN((prac_params[found][(int) GET_CLASS(ch)]), skilladd));
	if (GET_SKILL(ch, skill_num) >= (prac_params[found][(int) GET_CLASS(ch)])) {
      send_to_char(ch, "\tDYou mastered \tG%s\tD!\tn\r\n", spell_info[skill_num].name);
	  booster = read_object(3250, VIRTUAL);
	  obj_to_char(booster, ch);
	  send_to_char(ch, "Congratulations! You have been rewarded by the Game Masters with %s!\r\n", booster->short_description);
    } else
	  send_to_char(ch, "\tcYou get better with \tC%s\tc...\tn\r\n", spell_info[skill_num].name);
  }
  return;
}

/* control the fights going on.  Called every 2 seconds from comm.c. */
void perform_violence(void)
{
  struct char_data *ch, *tch;
  int i;
  
  for (ch = combat_list; ch; ch = next_combat_list) {
    next_combat_list = ch->next_fighting;

    if (FIGHTING(ch) == NULL || IN_ROOM(ch) != IN_ROOM(FIGHTING(ch))) {
      stop_fighting(ch);
      continue;
    }

    if (IS_NPC(ch)) {
      if (GET_MOB_WAIT(ch) > 0) {
        GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;		
        continue;
      }
      GET_MOB_WAIT(ch) = 0;
      if (GET_POS(ch) < POS_FIGHTING) {
        GET_POS(ch) = POS_FIGHTING;
        act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
      }
    }

    if (GET_POS(ch) < POS_FIGHTING) {
      send_to_char(ch, "You can't fight while sitting!!\r\n");
      continue;
    }
/*	
	if (GET_HTIMER(ch) > 0) {
	  GET_HTIMER(ch) -= 1;
      continue;
	}
*/
    if (GROUP(ch)) {
      while ((tch = (struct char_data *) simple_list(GROUP(ch)->members)) != NULL) {
        if (tch == ch)
          continue;
        if (!IS_NPC(tch) && !PRF_FLAGGED(tch, PRF_AUTOASSIST))
          continue;
        if (IN_ROOM(ch) != IN_ROOM(tch))
          continue;
        if (FIGHTING(tch))
          continue;
        if (GET_POS(tch) != POS_STANDING)
          continue;
        if (!CAN_SEE(tch, ch))
          continue;
      
        do_assist(tch, GET_NAME(ch), 0, 0);				  
      }
    }
	hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
	if (IS_NPC(ch) && GET_LEVEL(ch) > 8) {
	  if (GET_LEVEL(ch) >= 29 && FIGHTING(ch) && GET_POS(FIGHTING(ch)) > POS_STUNNED)
		hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
	  if (GET_LEVEL(ch) >= 19 && FIGHTING(ch) && GET_POS(FIGHTING(ch)) > POS_STUNNED)
		hit(ch, FIGHTING(ch), TYPE_UNDEFINED);  
	  if (GET_LEVEL(ch) >= 9 && FIGHTING(ch) && GET_POS(FIGHTING(ch)) > POS_STUNNED)
		hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
	} else if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_SECOND_ATTACK) > 0) {
	  for (i = 142; i < 145; i++) {		
        if (GET_SKILL(ch, i) > 0 && FIGHTING(ch) && GET_POS(FIGHTING(ch)) > POS_STUNNED && (GET_SKILL(ch, i) + rand_number(0, 100)) > 100) {
	      hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
		  pracskill(ch, i, 20); 
		}
	  }
	}

//    if (GET_HTIMER(ch) == 0) {
//	  GET_HTIMER(ch) = -1;
//	}	
    if (MOB_FLAGGED(ch, MOB_SPEC) && GET_MOB_SPEC(ch) && !MOB_FLAGGED(ch, MOB_NOTDEADYET)) {
      char actbuf[MAX_INPUT_LENGTH] = "";
      (GET_MOB_SPEC(ch)) (ch, ch, 0, actbuf);
    }
  }
}

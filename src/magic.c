/**************************************************************************
*  File: magic.c                                           Part of tbaMUD *
*  Usage: Low-level functions for magic; spell template code.             *
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
#include "interpreter.h"
#include "constants.h"
#include "dg_scripts.h"
#include "class.h"
#include "fight.h"
#include "mud_event.h"


/* local file scope function prototypes */
static int mag_materials(struct char_data *ch, IDXTYPE item0, IDXTYPE item1, IDXTYPE item2, int extract, int verbose);
static void perform_mag_groups(int level, struct char_data *ch, struct char_data *tch, int spellnum, int savetype);


/* Negative apply_saving_throw[] values make saving throws better! So do
 * negative modifiers.  Though people may be used to the reverse of that.
 * It's due to the code modifying the target saving throw instead of the
 * random number of the character as in some other systems. */
int mag_savingthrow(struct char_data *ch, int type, int modifier)
{
  /* NPCs use warrior tables according to some book */
  int class_sav = CLASS_WARRIOR;
  int save;

  if (!IS_NPC(ch))
    class_sav = GET_CLASS(ch);

  save = saving_throws(class_sav, type, GET_LEVEL(ch));
  save += GET_SAVE(ch, type);
  save += modifier;

  /* Throwing a 0 is always a failure. */
  if (MAX(1, save) < rand_number(0, 99))
    return (TRUE);

  /* Oops, failed. Sorry. */
  return (FALSE);
}

/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  struct affected_type *af, *next;
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next) {
      next = af->next;
      if (af->duration >= 1)
	af->duration--;
      else if (af->duration == -1)	/* No action */
	;
      else {
	if ((af->spell > 0) && (af->spell <= MAX_SPELLS))
	  if (!af->next || (af->next->spell != af->spell) ||
	      (af->next->duration > 0))
	    if (spell_info[af->spell].wear_off_msg)
	      send_to_char(i, "%s\r\n", spell_info[af->spell].wear_off_msg);
	affect_remove(i, af);
      }
    }	
}

/* Checks for up to 3 vnums (spell reagents) in the player's inventory. If
 * multiple vnums are passed in, the function ANDs the items together as
 * requirements (ie. if one or more are missing, the spell will not fail).
 * @param ch The caster of the spell.
 * @param item0 The first required item of the spell, NOTHING if not required.
 * @param item1 The second required item of the spell, NOTHING if not required.
 * @param item2 The third required item of the spell, NOTHING if not required.
 * @param extract TRUE if mag_materials should consume (destroy) the items in
 * the players inventory, FALSE if not. Items will only be removed on a
 * successful cast.
 * @param verbose TRUE to provide some generic failure or success messages,
 * FALSE to send no in game messages from this function.
 * @retval int TRUE if ch has all materials to cast the spell, FALSE if not.
 */
static int mag_materials(struct char_data *ch, IDXTYPE item0,
    IDXTYPE item1, IDXTYPE item2, int extract, int verbose)
{
  /* Begin Local variable definitions. */
  /*------------------------------------------------------------------------*/
  /* Used for object searches. */
  struct obj_data *tobj = NULL;
  /* Points to found reagents. */
  struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;
  /*------------------------------------------------------------------------*/
  /* End Local variable definitions. */

  /* Begin success checks. Checks must pass to signal a success. */
  /*------------------------------------------------------------------------*/
  /* Check for the objects in the players inventory. */
  for (tobj = ch->carrying; tobj; tobj = tobj->next_content)
  {
    if ((item0 != NOTHING) && (GET_OBJ_VNUM(tobj) == item0))
    {
      obj0 = tobj;
      item0 = NOTHING;
    }
    else if ((item1 != NOTHING) && (GET_OBJ_VNUM(tobj) == item1))
    {
      obj1 = tobj;
      item1 = NOTHING;
    }
    else if ((item2 != NOTHING) && (GET_OBJ_VNUM(tobj) == item2))
    {
      obj2 = tobj;
      item2 = NOTHING;
    }
  }

  /* If we needed items, but didn't find all of them, then the spell is a
   * failure. */
  if ((item0 != NOTHING) || (item1 != NOTHING) || (item2 != NOTHING))
  {
    /* Generic spell failure messages. */
    if (verbose)
    {
      switch (rand_number(0, 2))
      {
      case 0:
        send_to_char(ch, "A wart sprouts on your nose.\r\n");
        break;
      case 1:
        send_to_char(ch, "Your hair falls out in clumps.\r\n");
        break;
      case 2:
        send_to_char(ch, "A huge corn develops on your big toe.\r\n");
        break;
      }
    }
    /* Return fales, the material check has failed. */
    return (FALSE);
  }
  /*------------------------------------------------------------------------*/
  /* End success checks. */

  /* From here on, ch has all required materials in their inventory and the
   * material check will return a success. */

  /* Begin Material Processing. */
  /*------------------------------------------------------------------------*/
  /* Extract (destroy) the materials, if so called for. */
  if (extract)
  {
    if (obj0 != NULL)
      extract_obj(obj0);
    if (obj1 != NULL)
      extract_obj(obj1);
    if (obj2 != NULL)
      extract_obj(obj2);
    /* Generic success messages that signals extracted objects. */
    if (verbose)
    {
      send_to_char(ch, "A puff of smoke rises from your pack.\r\n");
      act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
    }
  }

  /* Don't extract the objects, but signal materials successfully found. */
  if(!extract && verbose)
  {
    send_to_char(ch, "Your pack rumbles.\r\n");
    act("Something rumbles in $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
  }
  /*------------------------------------------------------------------------*/
  /* End Material Processing. */

  /* Signal to calling function that the materials were successfully found
   * and processed. */
  return (TRUE);
}


/* Every spell that does damage comes through here.  This calculates the amount
 * of damage, adds in any modifiers, determines what the saves are, tests for
 * save and calls damage(). -1 = dead, otherwise the amount of damage done. */
int mag_damage(int level, struct char_data *ch, struct char_data *victim,
		     int spellnum, int savetype)
{
  int dam = 0;

  if (victim == NULL || ch == NULL)
    return (0);

  switch (spellnum) {
    /* Mostly mages */
  case SPELL_MAGIC_MISSILE:
    if (IS_THIEF(ch))
	  dam = GET_HIT(ch) / rand_number(8, 10);
    else
      dam = GET_HIT(ch) / rand_number(9, 11);
  case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
    if (IS_MANIPULATOR(ch))
      dam = GET_HIT(ch) / rand_number(9, 11);
    else
      dam = GET_HIT(ch) / rand_number(10, 12);
    break;
  case SPELL_BURNING_HANDS:
    if (IS_MAGIC_USER(ch))
      dam = GET_HIT(ch) / rand_number(7, 11);
    else
      dam = GET_HIT(ch) / rand_number(8, 12);
    break;
  case SPELL_SHOCKING_GRASP:
    if (IS_MAGIC_USER(ch))
      dam = GET_HIT(ch) / rand_number(6, 9);
    else
      dam = GET_HIT(ch) / rand_number(7, 10);
    break;
  case SPELL_LIGHTNING_BOLT:
    if (IS_MAGIC_USER(ch))
      dam = GET_HIT(ch) / rand_number(5, 7);
    else
      dam = GET_HIT(ch) / rand_number(6, 8);
  case SPELL_COLOR_SPRAY:
    if (IS_MAGIC_USER(ch))
      dam = GET_HIT(ch) / 6;
    else
      dam = GET_HIT(ch) / rand_number(7, 9);
    break;
  case SPELL_FIREBALL:
    if (IS_THIEF(ch))
      dam = GET_HIT(ch) / rand_number(4, 6);
    else
      dam = GET_HIT(ch) / rand_number(5, 7);
    break;

    /* Mostly clerics */
  case SPELL_DISPEL_EVIL:
  
    dam = GET_HIT(ch) / rand_number(5, 9);
    if (IS_EVIL(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (IS_GOOD(victim)) {
      act("A good nen protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      return (0);
    }
    break;
  case SPELL_DISPEL_GOOD:
    dam = GET_HIT(ch) / rand_number(5, 9);
    if (IS_GOOD(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (IS_EVIL(victim)) {
      act("An evil nen protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      return (0);
    }
    break;


  case SPELL_CALL_LIGHTNING:
    if (IS_CLERIC(ch))
	  dam = GET_HIT(ch) / rand_number(5, 8);
    else
      dam = GET_HIT(ch) / rand_number(6, 9);
    break;

  case SPELL_HARM:
    if (IS_MAGIC_USER(ch))
	  dam = GET_HIT(ch) / rand_number(2, 12);
    else
	  dam = GET_HIT(ch) / rand_number(3, 15);
    break;

  case SPELL_ENERGY_DRAIN:
    dam = GET_HIT(victim) / rand_number(4, 8);	
    GET_HIT(ch) += (dam / 2);
	break;

    /* Area spells */
  case SPELL_EARTHQUAKE:
    if (IS_WARRIOR(ch))
      dam = GET_HIT(ch) / rand_number(14, 18);
	else
      dam = GET_HIT(ch) / rand_number(15, 20);
    break;
	
  case SPELL_TIGHTEN_CHAINS:
    if (IS_CLERIC(ch))
      dam = GET_HIT(ch) / rand_number(9, 11);
    else
      dam = GET_HIT(ch) / rand_number(10, 12);
    GET_MANA(victim) -= 4;
    break;

  } /* switch(spellnum) */


  /* divide damage by two if victim makes his saving throw */
  if (mag_savingthrow(victim, savetype, 0))
    dam = 0;

  /* and finally, inflict the damage */
  return (damage(ch, victim, dam, spellnum));
}


/* Every spell that does an affect comes through here.  This determines the
 * effect, whether it is added or replacement, whether it is legal or not, etc.
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod) */
#define MAX_SPELL_AFFECTS 5	/* change if more needed */

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
		      int spellnum, int savetype)
{
  struct affected_type af[MAX_SPELL_AFFECTS];
  bool accum_affect = FALSE, accum_duration = FALSE;
  const char *to_vict = NULL, *to_room = NULL;
  int i, j;


  if (victim == NULL || ch == NULL)
    return;

  for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
    new_affect(&(af[i]));
    af[i].spell = spellnum;
  }

  switch (spellnum) {
  case SPELL_CALL_LIGHTNING:
    if (MOB_FLAGGED(victim, MOB_NOBASH))
      break;
	if (mag_savingthrow(victim, savetype, 0))
      WAIT_STATE(victim, PULSE_VIOLENCE);
    else
	  WAIT_STATE(victim, PULSE_VIOLENCE);
	  GET_POS(victim) = POS_SITTING;
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	to_vict = "You feel chained!";
	break;
	
  case SPELL_COLOR_SPRAY:
    if (mag_savingthrow(victim, savetype, 0))
      af[0].duration = 1;
    else
      af[0].duration = (GET_LEVEL(ch) / 6);
    SET_BIT_AR(af[0].bitvector, AFF_BUNGEE_GUM);
    accum_duration = TRUE;
    to_vict = "You feel gummed!";
    break;
  
  case SPELL_CHILL_TOUCH:
    af[0].location = APPLY_STR;
    if (mag_savingthrow(victim, savetype, 0))
      af[0].duration = 1;
    else
      af[0].duration = 4;
    if (GET_WIS(ch) > 11)
      af[0].modifier = -1 - ((GET_WIS(ch) / 2) - 5);
    else
	  af[0].modifier = -1;
    accum_duration = TRUE;
	accum_affect = FALSE;
    to_vict = "You feel your strength wither!";
    break;

  case SPELL_ARMOR:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].location = APPLY_AC;
    af[0].modifier = -25 - (GET_LEVEL(ch) * 1.5);
    af[0].duration = 10 + (GET_LEVEL(ch));
  //accum_duration = TRUE;
    to_vict = "You feel an aura protecting you.";
    break;
	
  case SPELL_CHAIN_PROTECTION:
    af[0].location = APPLY_AC;
    af[0].modifier = -20 - (GET_LEVEL(ch));
    af[0].duration = 18 + (GET_LEVEL(ch));
  //accum_duration = TRUE;
    to_vict = "Some chains covered with aura protects you.";
    break;

  case SPELL_BLESS:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
  
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 4 + (GET_LEVEL(ch) / 8);
    af[0].duration = 8 + (GET_LEVEL(ch) / 4);

    af[1].location = APPLY_SAVING_SPELL;
    af[1].modifier = -2 - (GET_LEVEL(ch) / 16);
    af[1].duration = 8 + (GET_LEVEL(ch) / 4);

    //accum_duration = TRUE;
    to_vict = "You feel focused.";
    break;

  case SPELL_BLINDNESS:
    if (MOB_FLAGGED(victim, MOB_NOBLIND) || GET_LEVEL(victim) >= LVL_IMMORT || mag_savingthrow(victim, savetype, 0)) {
      send_to_char(ch, "You fail.\r\n");
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].modifier = -4 - (GET_LEVEL(ch) / 7);
    af[0].duration = 2 + (GET_LEVEL(ch) / 7);
    SET_BIT_AR(af[0].bitvector, AFF_BLIND);

    af[1].location = APPLY_AC;
    af[1].modifier = 40 + (GET_LEVEL(ch) / 1.5);
    af[1].duration = 2 + (GET_LEVEL(ch) / 7);
    SET_BIT_AR(af[1].bitvector, AFF_BLIND);

    to_room = "$n seems to be blinded!";
    to_vict = "You have been blinded!";
    break;

  case SPELL_CURSE:
    if (mag_savingthrow(victim, savetype, 0)) {
      send_to_char(ch, "%s", CONFIG_NOEFFECT);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].duration = 1 + (GET_LEVEL(ch) / 8);
    af[0].modifier = -1 - (GET_LEVEL(ch) / 16);
    SET_BIT_AR(af[0].bitvector, AFF_CURSE);

    af[1].location = APPLY_DAMROLL;
    af[1].duration = 1 + (GET_LEVEL(ch) / 8);
    af[1].modifier = -1 - (GET_LEVEL(ch) / 16);
    SET_BIT_AR(af[1].bitvector, AFF_CURSE);

  //accum_duration = TRUE;
    accum_affect = TRUE;
    to_room = "$n briefly glows black!";
    to_vict = "You feel very uncomfortable.";
    break;

  case SPELL_DETECT_ALIGN:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 12 + level;
    SET_BIT_AR(af[0].bitvector, AFF_DETECT_ALIGN);
    accum_duration = TRUE;
    to_vict = "You feel people's intentions.";
    break;

  case SPELL_DETECT_INVIS:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 12 + level;
    SET_BIT_AR(af[0].bitvector, AFF_DETECT_INVIS);
    accum_duration = TRUE;
    to_vict = "You put some aura over your eyes.";
	to_room = "$n's put some aura over the eyes.";
    break;

  case SPELL_DETECT_MAGIC:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 12 + level;
    SET_BIT_AR(af[0].bitvector, AFF_DETECT_MAGIC);
    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;
	
  case SPELL_DETECT_POISON:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 12 + level;
    SET_BIT_AR(af[0].bitvector, AFF_DETECT_MAGIC);
    accum_duration = TRUE;
    to_vict = "Now you can detect diseases.";
    break;

  case SPELL_FLY:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 24;
    SET_BIT_AR(af[0].bitvector, AFF_FLYING);
    accum_duration = TRUE;
    to_vict = "You float above the ground.";
    break;

  case SPELL_INFRAVISION:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 12 + level;
    SET_BIT_AR(af[0].bitvector, AFF_INFRAVISION);
    accum_duration = TRUE;
    to_vict = "Your eyes glow red.";
    to_room = "$n's eyes glow red.";
    break;

  case SPELL_INVISIBLE:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    if (!victim)
      victim = ch;

    af[0].duration = 12 + (GET_LEVEL(ch) / 4);
    af[0].modifier = -40;
    af[0].location = APPLY_AC;
    SET_BIT_AR(af[0].bitvector, AFF_INVISIBLE);
    accum_duration = TRUE;
    to_vict = "You vanish.";
    to_room = "$n slowly fades out of existence.";
    break;
	
  case SPELL_POISON:
    if (mag_savingthrow(victim, savetype, 0)) {
      send_to_char(ch, "%s", CONFIG_NOEFFECT);
      return;
    }

    af[0].location = APPLY_STR;
    af[0].duration = GET_LEVEL(ch) / 16;
    af[0].modifier = -2 - (GET_LEVEL(ch) / 16);
    SET_BIT_AR(af[0].bitvector, AFF_POISON);
    to_vict = "You feel very sick.";
    to_room = "$n gets violently ill!";
    break;  

  case SPELL_SANCTUARY:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 4 + (GET_LEVEL(ch) / 8);
    SET_BIT_AR(af[0].bitvector, AFF_SANCTUARY);

    accum_duration = TRUE;
    to_vict = "A white aura momentarily surrounds you.";
    to_room = "$n is surrounded by a white aura.";
    break;

  case SPELL_SLEEP:
    if (!CONFIG_PK_ALLOWED && !IS_NPC(ch) && !IS_NPC(victim))
      return;
    if (MOB_FLAGGED(victim, MOB_NOSLEEP))
      return;
    if (mag_savingthrow(victim, savetype, 0))
      return;

    af[0].duration = 1 + (GET_LEVEL(ch) / 4);
    SET_BIT_AR(af[0].bitvector, AFF_SLEEP);

    if (GET_POS(victim) > POS_SLEEPING) {
      send_to_char(victim, "You feel very sleepy...  Zzzz......\r\n");
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_STRENGTH:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
	
/*	SET_BIT_AR(af[0].bitvector, AFF_ENHANCE);
    af[0].location = APPLY_STR;    
	if (IS_WARRIOR(ch) || IS_WARRIOR(victim)) {
	  af[0].duration = GET_LEVEL(ch) + 4;
      af[0].modifier = 25;
	  af[0].location = APPLY_HIT;
	  af[0].duration = GET_LEVEL(ch) + 4;
      af[0].modifier = 25;
	  to_vict = "Your body grows in height and muscles!";
    } else {
*/	  af[0].location = APPLY_DAMROLL;    
	  af[0].duration = 12 + (GET_LEVEL(ch) / 4);
      af[0].modifier = (level / 5);
	  SET_BIT_AR(af[0].bitvector, AFF_BOOST);
	  to_vict = "\tRYour aura color changes!\tn";		
    break;

  case SPELL_SENSE_LIFE:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    to_vict = "Your feel your awareness improve.";
    af[0].duration = GET_LEVEL(ch);
    SET_BIT_AR(af[0].bitvector, AFF_SENSE_LIFE);
    accum_duration = TRUE;
    break;

  case SPELL_WATERWALK:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 24;
    SET_BIT_AR(af[0].bitvector, AFF_WATERWALK);
    accum_duration = TRUE;
    to_vict = "You feel webbing between your toes.";
    break;
	
  case SPELL_LUCK:
    if (victim != ch && !IS_MANIPULATOR(ch)) {
      send_to_char(ch, "Only manipulators can perform these skills on others.\r\n");
      return;
    }
    af[0].duration = 1;
    SET_BIT_AR(af[0].bitvector, AFF_LUCK);
    accum_duration = FALSE;
    to_vict = "You feel luckier.";
    break;
  }  

  /* If this is a mob that has this affect set in its mob file, do not perform
   * the affect.  This prevents people from un-sancting mobs by sancting them
   * and waiting for it to fade, for example. */
  if (IS_NPC(victim) && !affected_by_spell(victim, spellnum)) {
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
      for (j=0; j<NUM_AFF_FLAGS; j++) {
        if (IS_SET_AR(af[i].bitvector, j) && AFF_FLAGGED(victim, j)) {
          send_to_char(ch, "%s", CONFIG_NOEFFECT);
          return;
        }
      }
    }
  }

  /* If the victim is already affected by this spell, and the spell does not
   * have an accumulative effect, then fail the spell. */
  if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect) && af[0].duration < 99) {
    send_to_char(ch, "%s", CONFIG_NOEFFECT);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector[0] || af[i].bitvector[1] ||
        af[i].bitvector[2] || af[i].bitvector[3] ||
        (af[i].location != APPLY_NONE))		
			affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE);		

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}

/* This function is used to provide services to mag_groups.  This function is
 * the one you should change to add new group spells. */
static void perform_mag_groups(int level, struct char_data *ch,
			struct char_data *tch, int spellnum, int savetype)
{
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
    mag_points(level, ch, tch, SPELL_HEAL, savetype);
    break;
  case SPELL_GROUP_ARMOR:
    mag_affects(level, ch, tch, SPELL_CHAIN_PROTECTION, savetype);
    break;
  case SPELL_GROUP_RECALL:
    spell_teleport(level, ch, tch, NULL);
    break;
  }
}

/* Every spell that affects the group should run through here perform_mag_groups
 * contains the switch statement to send us to the right magic. Group spells
 * affect everyone grouped with the caster who is in the room, caster last. To
 * add new group spells, you shouldn't have to change anything in mag_groups.
 * Just add a new case to perform_mag_groups. */
void mag_groups(int level, struct char_data *ch, int spellnum, int savetype)
{  
  struct follow_type *k, *next;

  if (ch == NULL)
    return;

  perform_mag_groups(level, ch, ch, spellnum, savetype);

  if (ch->followers) {
	for (k = ch->followers; k; k = next) {
	  next = k->next;			
	  if (IN_ROOM(k->follower) != IN_ROOM(ch))
		continue;
	  perform_mag_groups(level, ch, k->follower, spellnum, savetype);
	}
  }

/*  if (!GROUP(ch))
    return;
    
  while ((tch = (struct char_data *) simple_list(GROUP(ch)->members)) != NULL) {
    if (IN_ROOM(tch) != IN_ROOM(ch))
      continue;
    perform_mag_groups(level, ch, tch, spellnum, savetype);
  }
*/
}


/* Mass spells affect every creature in the room except the caster. No spells
 * of this class currently implemented. */
void mag_masses(int level, struct char_data *ch, int spellnum, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;

    switch (spellnum) {
    }
  }
}

/* Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual damage.
 * All spells listed here must also have a case in mag_damage() in order for
 * them to work. Area spells have limited targets within the room. */
void mag_areas(int level, struct char_data *ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  const char *to_char = NULL, *to_room = NULL;

  if (ch == NULL)
    return;

  /* to add spells just add the message here plus an entry in mag_damage for
   * the damaging part of the spell.   */
  switch (spellnum) {
  case SPELL_EARTHQUAKE:
    to_char = "You hit the ground and the earth begins to shake all around you!";
    to_room ="$n hit the ground and the earth begins to shake violently!";
    break;
  }
  
  SET_BIT_AR(ROOM_FLAGS(IN_ROOM(ch)), ROOM_CRATER);

  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);


  for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;

    /* The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     *            5: other players in the same group (if the spell is 'violent') 
     *            6: Flying people if earthquake is the spell                         */
    if (tch == ch)
      continue;
    if (!IS_NPC(tch))
      continue;
   /*if (!CONFIG_PK_ALLOWED && !IS_NPC(ch) && !IS_NPC(tch))
      continue;*/
    if (!IS_NPC(ch) && IS_NPC(tch) && AFF_FLAGGED(tch, AFF_CHARM))
      continue;
    if (!IS_NPC(tch) && spell_info[spellnum].violent && GROUP(tch) && GROUP(ch) && GROUP(ch) == GROUP(tch))
      continue;
	if ((spellnum == SPELL_EARTHQUAKE) && (SECT(IN_ROOM(ch)) == SECT_WATER_SWIM || SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM || AFF_FLAGGED(tch, AFF_FLYING)))
	  continue;
    /* Doesn't matter if they die here so we don't check. -gg 6/24/98 */
    mag_damage(level, ch, tch, spellnum, 1);
  }
}

/*----------------------------------------------------------------------------*/
/* Begin Magic Summoning - Generic Routines and Local Globals */
/*----------------------------------------------------------------------------*/

/* Every spell which summons/gates/conjours a mob comes through here. */
/* These use act(), don't put the \r\n. */
static const char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!",
  "$n animates a corpse!",
  "$N appears from a cloud of thick blue smoke!",
  "$N appears from a cloud of thick green smoke!",
  "$N appears from a cloud of thick red smoke!",
  "$N disappears in a thick black cloud!"
  "As $n makes a strange magical gesture, you feel a strong breeze.",
  "As $n makes a strange magical gesture, you feel a searing heat.",
  "As $n makes a strange magical gesture, you feel a sudden chill.",
  "As $n makes a strange magical gesture, you feel the dust swirl.",
  "$n magically divides!",
  "$n animates a corpse!"
};

/* Keep the \r\n because these use send_to_char. */
static const char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Gosh durnit!\r\n",
  "The elements resist!\r\n",
  "You failed.\r\n",
  "There is no corpse!\r\n"
};

/* Defines for Mag_Summons */
#define MOB_CLONE            10   /**< vnum for the clone mob. */
#define OBJ_CLONE            161  /**< vnum for clone material. */
#define MOB_ZOMBIE           11   /**< vnum for the zombie mob. */
#define MOB_BEAST            35   /**< vnum for the nen beast mob. */
#define MOB_COOKIE			 36   /**< vnum for cookie, the nen massagist. */

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
		      int spellnum, int savetype)
{
  struct char_data *mob = NULL;
  struct obj_data *tobj, *next_obj;
  char buf[256];
  int pfail = 0, msg = 0, fmsg = 0, num = 1, handle_corpse = FALSE, i;
  mob_vnum mob_num;

  if (ch == NULL)
    return;

  switch (spellnum) {
  case SPELL_CLONE:
    msg = 10;
    fmsg = rand_number(2, 6);	/* Random fail message. */
    mob_num = MOB_CLONE;
    /*
     * We have designated the clone spell as the example for how to use the
     * mag_materials function.
     * In stock tbaMUD it checks to see if the character has item with
     * vnum 161 which is a set of sacrificial entrails. If we have the entrails
     * the spell will succeed,  and if not, the spell will fail 102% of the time
     * (prevents random success... see below).
     * The object is extracted and the generic cast messages are displayed.
     */
/*    if( !mag_materials(ch, OBJ_CLONE, NOTHING, NOTHING, TRUE, TRUE) )
      pfail = 102; No materials, spell fails.
    else
      pfail = 0;	We have the entrails, spell is successfully cast. */
	pfail = 10;
    break;

  case SPELL_ANIMATE_DEAD:
    if (obj == NULL || !IS_CORPSE(obj)) {
      act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    handle_corpse = TRUE;
    msg = 11;
    fmsg = rand_number(2, 6);	/* Random fail message. */
    mob_num = MOB_ZOMBIE;
    pfail = 10;	/* 10% failure, should vary in the future. */
    break;
	
  case SPELL_NEN_BEAST:
    msg = 10;
    fmsg = rand_number(2, 6);	/* Random fail message. */
    mob_num = MOB_BEAST;    
	pfail = 10;
    break;
	
  case SPELL_MASSAGIST:
    msg = 10;
    fmsg = rand_number(2, 6);	/* Random fail message. */
    mob_num = MOB_COOKIE;    
	pfail = 10;
    break;

  default:
    return;
  }

  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    send_to_char(ch, "You are too giddy to have any followers!\r\n");
    return;
  }
  if (rand_number(0, 101) < pfail) {
    send_to_char(ch, "%s", mag_summon_fail_msgs[fmsg]);
    return;
  }
  for (i = 0; i < num; i++) {
    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
      send_to_char(ch, "You don't quite remember how to make that creature.\r\n");
      return;
    }	
    char_to_room(mob, IN_ROOM(ch));
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
	if (GET_MOB_VNUM(mob) != 36)
      SET_BIT_AR(AFF_FLAGS(mob), AFF_CHARM);
  
    if (spellnum == SPELL_CLONE) {
      /* Don't mess up the prototype; use new string copies. */
	  sprintf(buf, "%s%s%s", GET_NAME(ch), *GET_TITLE(ch) ? " " : "", GET_TITLE(ch));
      mob->player.name = strdup(GET_NAME(ch));
      mob->player.short_descr = strdup(buf);
	  mob->player.long_descr = ch->player.long_descr;	  
	  if (ch->player.description)
	    mob->player.description = strdup(ch->player.description);	  
	  mob->player.level = ch->player.level;
	  mob->player.sex = ch->player.sex;
	  mob->points.max_hit = GET_TOTAL_HIT(ch);
	  mob->points.hit = ch->points.hit;
	  mob->points.mana = (ch->points.mana - 20);
	  mob->aff_abils.str = ch->aff_abils.str;
	  mob->aff_abils.str_add = ch->aff_abils.str_add;
	  mob->aff_abils.dex = ch->aff_abils.dex;
	  mob->aff_abils.con = ch->aff_abils.con;
	  mob->aff_abils.intel = ch->aff_abils.intel;
	  mob->aff_abils.wis = ch->aff_abils.wis;
	  mob->aff_abils.cha = ch->aff_abils.cha;
	  mob->char_specials.saved.alignment = ch->char_specials.saved.alignment;
	  mob->char_specials.saved.apply_saving_throw[SAVING_PARA] = ch->char_specials.saved.apply_saving_throw[SAVING_PARA];
	  mob->char_specials.saved.apply_saving_throw[SAVING_ROD] = ch->char_specials.saved.apply_saving_throw[SAVING_ROD];
	  mob->char_specials.saved.apply_saving_throw[SAVING_PETRI] = ch->char_specials.saved.apply_saving_throw[SAVING_PETRI];
	  mob->char_specials.saved.apply_saving_throw[SAVING_BREATH] = ch->char_specials.saved.apply_saving_throw[SAVING_BREATH];
    } else if (spellnum == SPELL_NEN_BEAST) {
		mob->player.level = ch->player.level;
		mob->player.sex = ch->player.sex;
		mob->points.max_hit = (GET_TOTAL_HIT(ch) / rand_number(2, 4));
		mob->points.hit = mob->points.max_hit;
		mob->char_specials.saved.alignment = ch->char_specials.saved.alignment;
	}
	  
    act(mag_summon_msgs[msg], FALSE, ch, 0, mob, TO_ROOM);
    load_mtrigger(mob);
	if (GET_MOB_VNUM(mob) != 36)
      add_follower(mob, ch);
    
    if (GROUP(ch) && GROUP_LEADER(GROUP(ch)) == ch && GET_MOB_VNUM(mob) != 36)
      join_group(mob, GROUP(ch));    
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}

/* Clean up the defines used for mag_summons. */
#undef MOB_CLONE
#undef OBJ_CLONE
#undef MOB_ZOMBIE

/*----------------------------------------------------------------------------*/
/* End Magic Summoning - Generic Routines and Local Globals */
/*----------------------------------------------------------------------------*/


void mag_points(int level, struct char_data *ch, struct char_data *victim,
		     int spellnum, int savetype)
{

  if (victim == NULL)
    return;

  if (AFF_FLAGGED(victim, AFF_HEALED)) {
	send_to_char(ch, "%s can't bear this skill so often! (Cooldown timer: \tR%d\tn seconds)\r\n", (victim == ch) ? "You" : GET_NAME(victim), victim->char_specials.cooldown - 1);
	return;
  }

  switch (spellnum) {
  case SPELL_CURE_LIGHT:    
	GET_MANA(victim) = MIN(GET_MAX_MANA(victim), GET_MANA(victim) + 15);	
    send_to_char(victim, "You feel better.\r\n");
    break;
  case SPELL_CURE_CRITIC:
    GET_HIT(victim) = MIN(GET_TOTAL_HIT(victim), (GET_HIT(victim) + (GET_TOTAL_HIT(victim) / 5)));
    send_to_char(victim, "You feel energized!\r\n");
    break;
  case SPELL_HEAL:
    GET_HIT(victim) = MIN(GET_TOTAL_HIT(victim), (GET_HIT(victim) + (GET_TOTAL_HIT(victim) / 10)));
	GET_MANA(victim) = MIN(GET_MAX_MANA(victim), (GET_MANA(victim) + 10));	
    send_to_char(victim, "A warm feeling floods your body.\r\n");
    break;
  }
  SET_BIT_AR(AFF_FLAGS(ch), AFF_HEALED);
  ch->char_specials.cooldown = 122;
  update_pos(victim);
}

void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
		        int spellnum, int type)
{
  int spell = 0, msg_not_affected = TRUE;
  const char *to_vict = NULL, *to_room = NULL;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_HEAL:
    /* Heal also restores health, so don't give the "no effect" message if the
     * target isn't afflicted by the 'blindness' spell. */
    msg_not_affected = FALSE;
    /* fall-through */
  case SPELL_CURE_BLIND:
    spell = SPELL_BLINDNESS;
    to_vict = "Your vision returns!";
    to_room = "There's a momentary gleam in $n's eyes.";
    break;
  case SPELL_REMOVE_POISON:
    spell = SPELL_POISON;
    to_vict = "A warm feeling runs through your body!";
    to_room = "$n looks better.";
    break;
  case SPELL_REMOVE_CURSE:
    spell = SPELL_CURSE;
    to_vict = "A cursed aura was removed from you!";
    break;
  default:
    log("SYSERR: unknown spellnum %d passed to mag_unaffects.", spellnum);
    return;
  }

  if (!affected_by_spell(victim, spell)) {
    if (msg_not_affected)
      send_to_char(ch, "%s", CONFIG_NOEFFECT);
    return;
  }

  affect_from_char(victim, spell);
  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
		         int spellnum, int savetype)
{
  const char *to_char = NULL, *to_room = NULL;

  if (obj == NULL)
    return;

  switch (spellnum) {
    case SPELL_BLESS:
      if (!OBJ_FLAGGED(obj, ITEM_BLESS) && (GET_CLASS(ch) == 1 || GET_CLASS(ch) == 5)) {
	SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BLESS);
	GET_OBJ_WEIGHT(obj) = 1;
	to_char = "Some symbols appears over $p.";
      }
      break;
    case SPELL_CURSE:
      if (!OBJ_FLAGGED(obj, ITEM_NODROP)) {
	SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NODROP);
	if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
	  GET_OBJ_VAL(obj, 2)--;
	to_char = "$p briefly glows red.";
      }
      break;
    case SPELL_INVISIBLE:
      if (!OBJ_FLAGGED(obj, ITEM_NOINVIS) && !OBJ_FLAGGED(obj, ITEM_INVISIBLE)) {
        SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_INVISIBLE);
		SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_CONCEALED);
        to_char = "$p vanishes.";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
      GET_OBJ_VAL(obj, 3) = 1;
      to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (OBJ_FLAGGED(obj, ITEM_NODROP)) {
        REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)++;
        to_char = "The cursed aura was removed from $p.";
		break;
      }
	  if (OBJ_FLAGGED(obj, ITEM_INVISIBLE)) {
        if (OBJ_FLAGGED(obj, ITEM_NOSELL))
		  REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NOSELL);
	    else
		  SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NOSELL);
		REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_INVISIBLE);
        to_char = "The concealment aura was removed from $p.";
		break;
      }
	  if (!(obj->worn_by) && (OBJ_FLAGGED(obj, ITEM_MAGIC) || obj->affected[0].location != APPLY_NONE)) {
		if (OBJ_FLAGGED(obj, ITEM_MAGIC))
		  REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
	    if (OBJ_FLAGGED(obj, ITEM_ENFOLDED))
	      REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ENFOLDED);
		
	    obj->affected[0].location = APPLY_NONE;
		obj->affected[0].modifier = 0;
		obj->affected[1].location = APPLY_NONE;
		obj->affected[1].modifier = 0;
		obj->affected[2].location = APPLY_NONE;
		obj->affected[2].modifier = 0;
		obj->affected[3].location = APPLY_NONE;
		obj->affected[3].modifier = 0;
		obj->affected[4].location = APPLY_NONE;
		obj->affected[4].modifier = 0;
		obj->affected[5].location = APPLY_NONE;
		obj->affected[5].modifier = 0;

		to_char = "The enfoldment aura was removed from $p.";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 0;
        to_char = "$p steams briefly.";
      }
      break;
  }

  if (to_char == NULL)
    send_to_char(ch, "%s", CONFIG_NOEFFECT);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);
}

void mag_creations(int level, struct char_data *ch, int spellnum)
{
  struct obj_data *tobj;
  obj_vnum z;

  if (ch == NULL)
    return;
  /* level = MAX(MIN(level, LVL_IMPL), 1); - Hm, not used. */

  switch (spellnum) {
  case SPELL_CREATE_FOOD:
    z = 10;
    break;
  default:
    send_to_char(ch, "Spell unimplemented, it would seem.\r\n");
    return;
  }

  if (!(tobj = read_object(z, VIRTUAL))) {
    send_to_char(ch, "I seem to have goofed.\r\n");
    log("SYSERR: spell_creations, spell %d, obj %d: obj not found",
	    spellnum, z);
    return;
  }
  obj_to_char(tobj, ch);
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
  load_otrigger(tobj);
}

void mag_rooms(int level, struct char_data *ch, int spellnum)
{
  room_rnum rnum;
  int duration;
  bool failure = FALSE;
  event_id IdNum = eNULL;
  const char *msg = NULL;
  const char *room = NULL;
  
  rnum = IN_ROOM(ch);
  
  if (ROOM_FLAGGED(rnum, ROOM_NOMAGIC))
    failure = TRUE;
  
  switch (spellnum) {
    case SPELL_DARKNESS:
      IdNum = eSPL_DARKNESS;
      if (ROOM_FLAGGED(rnum, ROOM_DARK))
        failure = TRUE;
        
      duration = 5;
      SET_BIT_AR(ROOM_FLAGS(rnum), ROOM_DARK);
        
      msg = "You cast a shroud of darkness upon the area.";
      room = "$n casts a shroud of darkness upon this area.";
    break;
  
  }
  
  if (failure || IdNum == eNULL) {
    send_to_char(ch, "You failed!\r\n");
    return;
  }
  
  send_to_char(ch, "%s\r\n", msg);
  act(room, FALSE, ch, 0, 0, TO_ROOM);
  
  NEW_EVENT(eSPL_DARKNESS, &world[rnum], NULL, duration * PASSES_PER_SEC);
}

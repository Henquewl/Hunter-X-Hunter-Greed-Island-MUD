/**************************************************************************
*  File: spell_parser.c                                    Part of tbaMUD *
*  Usage: Top-level magic routines; outside points of entry to magic sys. *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#define __SPELL_PARSER_C__

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "dg_scripts.h"
#include "fight.h"  /* for hit() */
#include "constants.h"

#define SINFO spell_info[spellnum]

/* Global Variables definitions, used elsewhere */
struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];
char cast_arg2[MAX_INPUT_LENGTH];
const char *unused_spellname = "!UNUSED!"; /* So we can get &unused_spellname */

/* Local (File Scope) Function Prototypes */
static void say_spell(struct char_data *ch, int spellnum, struct char_data *tch, struct obj_data *tobj);
static void spello(int spl, const char *name, int max_mana, int min_mana, int mana_change, int minpos, int targets, int violent, int routines, const char *wearoff);
static int mag_manacost(struct char_data *ch, int spellnum);

/* Local (File Scope) Variables */
struct syllable {
  const char *org;
  const char *news;
};
static struct syllable syls[] = {
  {" ", " "},
  {"align", "shinjitsu"},
  {"armor", "ren"},
  {"aura", "ryu"},
  {"ball", "boru"},
  {"beast", "kemono"},
  {"blast", "ryu ha"},
  {"blind", "shitsumei"},
  {"blindness", "shitsumei"},
  {"burninghands", "moeru te"},
  {"card", "kado"},
  {"circleofen", "en"},
  {"chain", "kusari"},
  {"chilltouch", "chirutatchi"},
  {"create", "sakusei suru"},
  {"clone", "bunshin"},
  {"conceal", "in"},
  {"cure", "iyashimasu"},
  {"curse", "noroi"},
  {"darkness", "yami"},
  {"detectconcealed", "gyo"},
  {"detect", "kenshutsu"},
  {"drain", "dorein"},
  {"dowzing", "daujingu"},
  {"earthquake", "jishin"},
  {"energy", "enerugi"},
  {"enfold", "shu"},
  {"boost", "ko"},
  {"exorcise", "kiyomemasu"},  
  {"focus", "ten"},
  {"fortify", "ken"},
  {"food", "shokumotsu"},
  {"heal", "iyasu"},
  {"identify", "shikibetsu"},
  {"jail", "keimusho"},
  {"levitate", "fuyo suru"},
  {"littleflower", "little flower"},
  {"locate", "mitsukeru"},
  {"luck", "koun"},
  {"infravision", "kurayami no naka de miru"},
  {"mass", "tairyo"},
  {"massagist", "majikaru esute"},
  {"nen", "nen"},
  {"object", "taisho"},
  {"poison", "doku"},  
  {"repair", "shufuku"},  
  {"slumber", "suimin"},
  {"stitches", "sutetchi"},
  {"shockinggrasp", "erekuto haaku"},  
  {"water", "mizu"},
  {"weapon", "buki"},
/*{"a", "a"}, {"b", "b"}, {"c", "c"}, {"d", "d"}, {"e", "e"}, {"f", "f"}, {"g", "g"},
  {"h", "h"}, {"i", "i"}, {"j", "j"}, {"k", "k"}, {"l", "l"}, {"m", "m"}, {"n", "n"},
  {"o", "o"}, {"p", "p"}, {"q", "q"}, {"r", "r"}, {"s", "s"}, {"t", "t"}, {"u", "u"},
  {"v", "v"}, {"w", "w"}, {"x", "x"}, {"y", "y"}, {"z", "z"},*/ {"", ""}
};



static int mag_manacost(struct char_data *ch, int spellnum)
{
  return MAX(SINFO.mana_max - (SINFO.mana_change *
		    (GET_LEVEL(ch) - SINFO.min_level[(int) GET_CLASS(ch)])),
	     SINFO.mana_min);
}

static void say_spell(struct char_data *ch, int spellnum, struct char_data *tch,
	            struct obj_data *tobj)
{
  char lbuf[256], buf[256], buf1[256], buf2[256];	/* FIXME */
  const char *format;

  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strlcpy(lbuf, skill_name(spellnum), sizeof(lbuf));

  while (lbuf[ofs]) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].news);	/* strcat: BAD */
	ofs += strlen(syls[j].org);
        break;
      }
    }
    /* i.e., we didn't find a match in syls[] */
    if (!*syls[j].org) {
      log("No entry in syllable table for substring of '%s'", lbuf);
      ofs++;
    }
  }

  if (tch != NULL && IN_ROOM(tch) == IN_ROOM(ch)) {
    if (tch == ch)
      format = "$n concentrate $s nen and utters the words, '%s'!";
    else
      format = "$n stares at $N and utters the words, '%s'!";
  } else if (tobj != NULL &&
	     ((IN_ROOM(tobj) == IN_ROOM(ch)) || (tobj->carried_by == ch)))
    format = "$n stares at $p and utters the words, '%s'!";
  else
    format = "$n utters the words, '%s'!";

  snprintf(buf1, sizeof(buf1), format, skill_name(spellnum));
  snprintf(buf2, sizeof(buf2), format, buf);

  for (i = world[IN_ROOM(ch)].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_CLASS(ch) == GET_CLASS(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && IN_ROOM(tch) == IN_ROOM(ch)) {
    snprintf(buf1, sizeof(buf1), "$n stares at you and utters the words, '%s'!",
	    GET_CLASS(ch) == GET_CLASS(tch) ? skill_name(spellnum) : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}

/* This function should be used anytime you are not 100% sure that you have
 * a valid spell/skill number.  A typical for() loop would not need to use
 * this because you can guarantee > 0 and <= TOP_SPELL_DEFINE. */
const char *skill_name(int num)
{
  if (num > 0 && num <= TOP_SPELL_DEFINE)
    return (spell_info[num].name);
  else if (num == -1)
    return ("UNUSED");
  else
    return ("UNDEFINED");
}

int find_skill_num(char *name)
{
  int skindex, ok;
  char *temp, *temp2;
  char first[256], first2[256], tempbuf[256];

  for (skindex = 1; skindex <= TOP_SPELL_DEFINE; skindex++) {
    if (is_abbrev(name, spell_info[skindex].name))
      return (skindex);

    ok = TRUE;
    strlcpy(tempbuf, spell_info[skindex].name, sizeof(tempbuf));	/* strlcpy: OK */
    temp = any_one_arg(tempbuf, first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
	ok = FALSE;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return (skindex);
  }

  return (-1);
}

/* This function is the very heart of the entire magic system.  All invocations
 * of all types of magic -- objects, spoken and unspoken PC and NPC spells, the
 * works -- all come through this function eventually. This is also the entry
 * point for non-spoken or unrestricted spells. Spellnum 0 is legal but silently
 * ignored here, to make callers simpler. */
int call_magic(struct char_data *caster, struct char_data *cvict,
	     struct obj_data *ovict, int spellnum, int level, int casttype)
{
  int savetype;

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return (0);

  if (!cast_wtrigger(caster, cvict, ovict, spellnum))
    return 0;
  if (!cast_otrigger(caster, ovict, spellnum))
    return 0;
  if (!cast_mtrigger(caster, cvict, spellnum))
    return 0;

  if (ROOM_FLAGGED(IN_ROOM(caster), ROOM_NOMAGIC)) {
    send_to_char(caster, "Your skills not work in this place!\r\n");
    act("$n's tried use a skill but it was prevented!", FALSE, caster, 0, 0, TO_ROOM);
    return (0);
  }
  if (ROOM_FLAGGED(IN_ROOM(caster), ROOM_PEACEFUL) &&
      (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
    send_to_char(caster, "The room suck your violent aura preventing using the skill!\r\n");
    act("White light from no particular source suddenly fills the room, then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
    return (0);
  }
  if (cvict && MOB_FLAGGED(cvict, MOB_NOKILL)) {
    send_to_char(caster, "Your attack has suddenly blocked!\r\n");
    return (0);
  }
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }

  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    if (mag_damage(level, caster, cvict, spellnum, savetype) == -1)
      return (-1);	/* Successful and target died, don't cast again. */

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_ROOMS))
    mag_rooms(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_CHARM:		MANUAL_SPELL(spell_charm); break;
    case SPELL_CREATE_WATER:	MANUAL_SPELL(spell_create_water); break;
//    case SPELL_DETECT_POISON:	MANUAL_SPELL(spell_detect_poison); break;
    case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_REPAIR:		MANUAL_SPELL(spell_repair); break;
	case SPELL_IDENTIFY:	MANUAL_SPELL(spell_identify); break;
	case SPELL_LOCATE_CARD:   MANUAL_SPELL(spell_locate_card); break;
    case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object); break;
    case SPELL_SUMMON:		MANUAL_SPELL(spell_summon); break;
    case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); break;
    case SPELL_TELEPORT:	MANUAL_SPELL(spell_teleport); break;	
    }

  return (1);
}

/* mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 * Staves and wands will default to level 14 if the level is not specified; the
 * DikuMUD format did not specify staff and wand levels in the world files */
void mag_objectmagic(struct char_data *ch, struct obj_data *obj,
		          char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      send_to_char(ch, "It seems powerless.\r\n");
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      /* Level to cast spell at. */
      k = GET_OBJ_VAL(obj, 0) ? GET_OBJ_VAL(obj, 0) : DEFAULT_STAFF_LVL;

      /* Area/mass spells on staves can cause crashes. So we use special cases
       * for those spells spells here. */
      if (HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, 3), MAG_MASSES | MAG_AREAS)) {
        for (i = 0, tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
	  i++;
	while (i-- > 0)
	  call_magic(ch, NULL, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
      } else {
	for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
	  next_tch = tch->next_in_room;
	  if (ch != tch)
	    call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
	}
      }
    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	if (obj->action_description)
	  act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	else
	  act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description)
	act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
	act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else if (IS_SET(spell_info[GET_OBJ_VAL(obj, 3)].routines, MAG_AREAS | MAG_MASSES)) {
      /* Wands with area spells don't need to be pointed. */
      act("You point $p outward.", FALSE, ch, obj, NULL, TO_CHAR);
      act("$n points $p outward.", TRUE, ch, obj, NULL, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      send_to_char(ch, "It seems powerless.\r\n");
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 GET_OBJ_VAL(obj, 0), CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 DEFAULT_WAND_LVL, CAST_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;

    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i <= 3; i++)
      if (call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_SCROLL) <= 0)
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;

  if (!consume_otrigger(obj, ch, OCMD_QUAFF))  /* check trigger */
    return;

    act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i <= 3; i++)
      if (call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_POTION) <= 0)
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type %d in mag_objectmagic.",
	GET_OBJ_TYPE(obj));
    break;
  }
}

/* cast_spell is used generically to cast any spoken spell, assuming we already
 * have the target char/obj and spell number.  It checks all restrictions,
 * prints the words, etc. Entry point for NPC casts.  Recommended entry point
 * for spells cast by NPCs via specprocs. */
int cast_spell(struct char_data *ch, struct char_data *tch,
	           struct obj_data *tobj, int spellnum)
{
  if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE) {
    log("SYSERR: cast_spell trying to call spellnum %d/%d.", spellnum,
	TOP_SPELL_DEFINE);
    return (0);
  }

  if (GET_POS(ch) < SINFO.min_position) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char(ch, "You dream about great magical powers.\r\n");
      break;
    case POS_RESTING:
      send_to_char(ch, "You cannot concentrate while resting.\r\n");
      break;
    case POS_SITTING:
      send_to_char(ch, "You can't do this sitting!\r\n");
      break;
    case POS_FIGHTING:
      send_to_char(ch, "Impossible!  You can't concentrate enough!\r\n");
      break;
    default:
      send_to_char(ch, "You can't do much of anything like this!\r\n");
      break;
    }
    return (0);
  }
  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char(ch, "You are afraid you might hurt your master!\r\n");
    return (0);
  }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
    send_to_char(ch, "You can only cast this spell upon yourself!\r\n");
    return (0);
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char(ch, "You cannot cast this spell upon yourself!\r\n");
    return (0);
  }
/*  if (IS_SET(SINFO.routines, MAG_GROUPS) && !GROUP(ch)) {
    send_to_char(ch, "You can't cast this spell if you're not in a group!\r\n");
    return (0);
  }
*/  send_to_char(ch, "%s", CONFIG_OK);
  say_spell(ch, spellnum, tch, tobj);

  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL));
}

/* do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell(). */
ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int number, mana, spellnum, i, found = 0, target = 0;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  if (subcmd)
    spellnum = subcmd;
  if (!spellnum) {
  /* get: blank, spell name, target name */
  s = strtok(argument, "'");
  
  if (s == NULL) {
    send_to_char(ch, "Cast what where?\r\n");
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char(ch, "Usage: cast 'skill name' self | target | object\r\n");
    return;
  }
  t = strtok(NULL, "\0");  

  skip_spaces(&s);

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_skill_num(s);
  
  if (!spellnum || (spellnum < 1) || (spellnum > MAX_SPELLS) || !*s) {
    send_to_char(ch, "Cast what?!?\r\n");
    return;  
  }
  if (GET_LEVEL(ch) < SINFO.min_level[(int) GET_CLASS(ch)]) {
    send_to_char(ch, "You do not know that skill!\r\n");
    return;
  }
  if (GET_SKILL(ch, spellnum) == 0) {
    send_to_char(ch, "You are unfamiliar with that skill.\r\n");
    return;
  }
  } else {
	if (!subcmd)
	  return;
	t = buf;
	if (argument)
	  one_argument(argument, buf);	
    

  for (i = 0; i < NUM_CLASSES; i++)
    if (GET_LEVEL(ch) < SINFO.min_level[i])
      found++;
	
  if (!found) {
	send_to_char(ch, "You do not know that skill!\r\n");
    return;
  }
  
  if (GET_SKILL(ch, spellnum) == 0) {
    send_to_char(ch, "Unpractised you are, a master you must seek, hum.\r\n");
    return;
  }  

  if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOMAGIC)) && SINFO.violent) {
	send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
	return;
  }

  if (spellnum == SPELL_TELEPORT && is_abbrev(t, "mark")) {
	if (SECT(IN_ROOM(ch)) > 5)
	  send_to_char(ch, "Have you thought about going to dry land first?\r\n");	  
	else if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(ch)), ZONE_NOASTRAL))
      send_to_char(ch, "This place has some special property that prevents your marking.");     
    else {
	  GET_LOADROOM(ch) = IN_ROOM(ch);
	  send_to_char(ch, "You put some aura on your fingertip and make a mark on the ground.\r\n");
	  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	}
	return;
  }
  }
  /* Find the target */
  if (t != NULL) {
    char arg[MAX_INPUT_LENGTH];	
	
	strlcpy(arg, t, sizeof(arg));
    one_argument(arg, t);
	skip_spaces(&t);
    
    /* Copy target to global cast_arg2, for use in spells like locate object */
    strcpy(cast_arg2, t);	
  }	

  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {	
      number = get_number(&t);
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_vis(ch, t, &number, FIND_CHAR_ROOM)) != NULL)
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t, &number, FIND_CHAR_WORLD)) != NULL)
	    target = TRUE;	
	
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, &number, ch->carrying)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	    if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
	      tobj = GET_EQ(ch, i);
	      target = TRUE;
	    }
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, &number, world[IN_ROOM(ch)].contents)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t, &number)) != NULL)
	  target = TRUE;

  } else {			/* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	!SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
	  send_to_char(ch, "\tcSyntax: \tC%s \tc[\tY%s\tc]\tn\r\n", spell_info[spellnum].name,
	    IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD | TAR_OBJ_EQUIP) ? "item" : "target");      
      return;
    }
  }  

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char(ch, "You shouldn't cast that on yourself -- could be bad for your health!\r\n");
    return;
  } else if (target && SINFO.violent && tch && (!IS_NPC(tch) && IS_AFFECTED(tch, AFF_NOPK))) {
	send_to_char(ch, "The target player is protected by NO_PK flag.\r\n");
	return;
  }  
  if (!target) {
    send_to_char(ch, "\tcSyntax: \tC%s \tc[\tY%s\tc]\tn\r\n", spell_info[spellnum].name,
	    IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD | TAR_OBJ_EQUIP) ? "item" : "target");
	return;
  }
  
  mana = mag_manacost(ch, spellnum);
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char(ch, "You haven't the energy to cast that skill!\r\n");
    return;
  }  

  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (rand_number(0, 101) > GET_SKILL(ch, spellnum)) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, spellnum))
      send_to_char(ch, "You failed to concentrate your aura.\r\n");
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana / 2)));
    if (SINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, spellnum)) {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
      if (mana > 0)
	    GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
    }
  }  
  pracskill(ch, spellnum, 18); 
}

void spell_level(int spell, int chclass, int level)
{
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
    return;
  }

  if (chclass < 0 || chclass >= NUM_CLASSES) {
    log("SYSERR: assigning '%s' to illegal class %d/%d.", skill_name(spell),
		chclass, NUM_CLASSES - 1);
    bad = 1;
  }

  if (level < 1 || level > LVL_IMPL) {
    log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),
		level, LVL_IMPL);
    bad = 1;
  }

  if (!bad)
	spell_info[spell].min_level[chclass] = level;

}


/* Assign the spells on boot up */
static void spello(int spl, const char *name, int max_mana, int min_mana,
	int mana_change, int minpos, int targets, int violent, int routines, const char *wearoff)
{
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = max_mana;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
  spell_info[spl].name = name;
  spell_info[spl].wear_off_msg = wearoff;
}

void unused_spell(int spl)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_IMPL + 1;
  spell_info[spl].mana_max = 0;
  spell_info[spl].mana_min = 0;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = 0;
  spell_info[spl].targets = 0;
  spell_info[spl].violent = 0;
  spell_info[spl].routines = 0;
  spell_info[spl].name = unused_spellname;
}

#define skillo(skill, name) spello(skill, name, 0, 0, 0, 0, 0, 0, 0, NULL);
/* Arguments for spello calls:
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 *  spells.h (such as SPELL_HEAL).
 * spellname: The name of the spell.
 * maxmana :  The maximum mana this spell will take (i.e., the mana it
 *  will take when the player first gets the spell).
 * minmana :  The minimum mana this spell will take, no matter how high
 *  level the caster is.
 * manachng:  The change in mana for the spell from level to level.  This
 *  number should be positive, but represents the reduction in mana cost as
 *  the caster's level increases.
 * minpos  :  Minimum position the caster must be in for the spell to work
 *  (usually fighting or standing). targets :  A "list" of the valid targets
 *  for the spell, joined with bitwise OR ('|').
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 *  spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 *  set on any spell that inflicts damage, is considered aggressive (i.e.
 *  charm, curse), or is otherwise nasty.
 * routines:  A list of magic routines which are associated with this spell
 *  if the spell uses spell templates.  Also joined with bitwise OR ('|').
 * See the documentation for a more detailed description of these fields. You
 * only need a spello() call to define a new spell; to decide who gets to use
 * a spell or skill, look in class.c.  -JE */
void mag_assign_spells(void)
{
  int i;

  /* Do not change the loop below. */
  for (i = 0; i <= TOP_SPELL_DEFINE; i++)
    unused_spell(i);
  /* Do not change the loop above. */

  spello(SPELL_ANIMATE_DEAD, "animatedead", 10, 10, 3, POS_STANDING,
	TAR_OBJ_ROOM, FALSE, MAG_SUMMONS,
	NULL);

  spello(SPELL_ARMOR, "armor", 10, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You feel less protected.");

  spello(SPELL_BLESS, "focus", 10, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS,
	"You feel less focused.");

  spello(SPELL_BLINDNESS, "blindness", 10, 25, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS,
	"You feel a cloak of blindness dissolve.");

  spello(SPELL_BURNING_HANDS, "burninghands", 2, 10, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_CALL_LIGHTNING, "chainjail", 5, 50, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS,
	NULL);

  spello(SPELL_CHAIN_PROTECTION, "chainprotected", 10, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"The chains around you get loose and degrades.");
	
  spello(SPELL_CHARM, "charmperson", 20, 50, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL,
	"You feel more self-confident.");

  spello(SPELL_CHILL_TOUCH, "chilltouch", 2, 10, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS,
	"You feel your strength return.");

  spello(SPELL_CLONE, "clone", 20, 65, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_SUMMONS,
	NULL);
	
  spello(SPELL_NEN_BEAST, "nenbeast", 15, 65, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_SUMMONS,
	NULL);
	
  spello(SPELL_MASSAGIST, "massagist", 15, 10, 3, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_SUMMONS,
	NULL);

  spello(SPELL_COLOR_SPRAY, "bungeegum", 5, 20, 4, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS,
	"You feel detached from bungee gum.");

  spello(SPELL_CONTROL_WEATHER, "controlweather", 40, 25, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_CREATE_FOOD, "createfood", 10, 5, 4, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS,
	NULL);

  spello(SPELL_CREATE_WATER, "refill", 10, 5, 4, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_CURE_BLIND, "cureblind", 10, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS,
	NULL);

  spello(SPELL_CURE_CRITIC, "nencure", 0, 20, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS,
	NULL);

  spello(SPELL_CURE_LIGHT, "nenstitches", 0, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS,
	NULL);

  spello(SPELL_CURSE, "curse", 10, 50, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS,
	"The cursed aura leaves your body.");

  spello(SPELL_DARKNESS, "darkness", 4, 5, 4, POS_STANDING,
	TAR_IGNORE, TRUE, MAG_ROOMS,
	NULL);

  spello(SPELL_DETECT_ALIGN, "detectalignment", 5, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You feel less aware.");

  spello(SPELL_DETECT_INVIS, "detectconcealed", 6, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"Your eyes stop tingling.");

  spello(SPELL_DETECT_MAGIC, "detectenfold", 10, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"The detect enfold wears off.");

  spello(SPELL_DETECT_POISON, "detectpoison", 4, 4, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"The detect poison wears off.");

  spello(SPELL_DISPEL_EVIL, "dispelevil", 5, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_DISPEL_GOOD, "dispelgood", 5, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_EARTHQUAKE, "earthquake", 8, 25, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS,
	NULL);

  spello(SPELL_ENCHANT_WEAPON, "enfold", 40, 100, 10, POS_STANDING,
	TAR_OBJ_INV, FALSE, MAG_MANUAL,
	NULL);
	
  spello(SPELL_REPAIR, "repair", 15, 100, 10, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, FALSE, MAG_MANUAL,
	NULL); 

  spello(SPELL_ENERGY_DRAIN, "energydrain", 8, 25, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_GROUP_ARMOR, "massarmor", 15, 30, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS,
	NULL);

  spello(SPELL_FIREBALL, "auraball", 8, 30, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_FLY, "levitate", 10, 20, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You drift slowly to the ground.");
	
  spello(SPELL_LUCK, "luck", 20, 10, 6, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You feel less lucky\r\n.");

  spello(SPELL_GROUP_HEAL, "massheal", 20, 60, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS,
	NULL);

  spello(SPELL_HARM, "littleflower", 8, 45, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_HEAL, "heal", 0, 40, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_UNAFFECTS,
	NULL);

  spello(SPELL_INFRAVISION, "infravision", 10, 10, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"Your night vision seems to fade.");

  spello(SPELL_INVISIBLE, "conceal", 10, 25, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS,
	"You feel yourself exposed.");

  spello(SPELL_LIGHTNING_BOLT, "lightningbolt", 4, 15, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);
	
  spello(SPELL_LOCATE_CARD, "locatecard", 10, 20, 1, POS_STANDING,
	TAR_OBJ_WORLD, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_LOCATE_OBJECT, "locateobject", 10, 20, 1, POS_STANDING,
	TAR_OBJ_WORLD, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_MAGIC_MISSILE, "blast", 2, 10, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_POISON, "poison", 6, 20, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE,
	MAG_AFFECTS | MAG_ALTER_OBJS,
	"You feel less sick.");  

  spello(SPELL_REMOVE_CURSE, "exorcise", 30, 35, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE,
	MAG_UNAFFECTS | MAG_ALTER_OBJS,
	NULL);

  spello(SPELL_REMOVE_POISON, "curepoison", 10, 8, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS,
	NULL);

  spello(SPELL_SANCTUARY, "fortify", 30, 85, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"The white aura around your body fades.");

  spello(SPELL_SENSE_LIFE, "circleofen", 15, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You feel less aware of your surroundings.");

  spello(SPELL_SHOCKING_GRASP, "shockinggrasp", 3, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_SLEEP, "slumber", 15, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS,
	"You feel less tired.");

  spello(SPELL_STRENGTH, "boost", 20, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"Your aura return to whitish color.");

  spello(SPELL_SUMMON, "summonritual", 30, 100, 3, POS_STANDING,
	TAR_CHAR_WORLD | TAR_NOT_SELF, TRUE, MAG_MANUAL,
	NULL);

  spello(SPELL_TELEPORT, "teleport", 20, 50, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_WATERWALK, "waterwalk", 10, 20, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"Your feet seem less buoyant.");

  spello(SPELL_WORD_OF_RECALL, "return", 20, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
	NULL);
	
  spello(SPELL_GROUP_RECALL, "massescape", 15, 60, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS,
	NULL);

  spello(SPELL_IDENTIFY, "identify", 10, 25, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL,
        NULL);
		
  spello(SPELL_TIGHTEN_CHAINS, "dowzingchain", 2, 25, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL); 

  /* NON-castable spells should appear below here. */
/*  spello(SPELL_IDENTIFY, "identify", 0, 0, 0, 0,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL,
	NULL);
*/
  /* you might want to name this one something more fitting to your theme -Welcor*/
  spello(SPELL_DG_AFFECT, "Script-inflicted", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0,
	NULL);

  /* Declaration of skills - this actually doesn't do anything except set it up
   * so that immortals can use these skills by default.  The min level to use
   * the skill for other classes is set up in class.c. */
  skillo(SKILL_BACKSTAB, "flurryofblows");
  skillo(SKILL_BASH, "bash");
  skillo(SKILL_HIDE, "enhance");
  skillo(SKILL_HIDE, "hide");
  skillo(SKILL_KICK, "kick");
  skillo(SKILL_PICK_LOCK, "picklock");
  skillo(SKILL_RESCUE, "rescue");  
  skillo(SKILL_SNEAK, "sneak");
  skillo(SKILL_STEAL, "steal");
  skillo(SKILL_TRACK, "track");
  skillo(SKILL_WHIRLWIND, "whirlwind");
  skillo(SKILL_POWER, "power");
  skillo(SKILL_SECOND_ATTACK, "second attack");
  skillo(SKILL_THIRD_ATTACK, "third attack");
  skillo(SKILL_FOURTH_ATTACK, "fourth attack");
  skillo(SKILL_ANALYSIS, "analysis");
  skillo(SKILL_DODGE, "dodge");
  skillo(SKILL_PARRY, "parry");
  skillo(SKILL_INSTANT_FORTIFY, "instant fortify");
  skillo(SKILL_BAREHANDED_EXPERT, "barehanded expert");
  skillo(SKILL_JAJANKEN, "jajanken");
  skillo(SKILL_RECALL, "recall");
  skillo(SKILL_CHANGE, "change");
  skillo(SKILL_NOPK, "NO_PK");
  skillo(SKILL_ENHANCE, "enhance");
  skillo(SKILL_SENSE, "sense");
  skillo(SKILL_REMOTE_PUNCH, "remotepunch");
}


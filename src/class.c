/**************************************************************************
*  File: class.c                                           Part of tbaMUD *
*  Usage: Source file for class-specific code.                            *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

/** Help buffer the global variable definitions */
#define __CLASS_C__

/* This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class, you
 * should go through this entire file from beginning to end and add the
 * appropriate new special cases for your new class. */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "spells.h"
#include "interpreter.h"
#include "constants.h"
#include "act.h"
#include "comm.h"

/* Names first */
const char *class_abbrevs[] = {
  "Tr",
  "Co",
  "Em",
  "En",
  "Ma",
  "Sp",
  "Hu",
  "\n"
};

const char *class_colors[] = {
  "\x1B[0;35m", /* 0 Transmuter Magenta */
  "\x1B[0;32m", /* 1 Conjurer Green */
  "\x1B[0;33m", /* 2 Emissor Yellow  */
  "\x1B[0;36m", /* 3 Enhancer Cyan */
  "\x1B[1;30m", /* 4 Manipulator Gray */
  "\x1B[0;31m", /* 5 Specialist Red */
  "\x1B[0;0m",  /* 6 Hunter Normal */
  "\n"
};

const char *show_color[] = {
  "",
  "\x1B[0;37m",
  "\x1B[1;37m",
  "\x1B[0;33m",
  "\x1B[1;30m",
  "\x1B[1;33m",
  "\x1B[0;34m",
  "\x1B[1;34m",
  "\x1B[0;31m",
  "\x1B[1;31m",
  "\x1B[0;32m",
  "\x1B[1;32m",
  "\x1B[0;35m",
  "\x1B[1;35m",
  "\x1B[0;36m",
  "\x1B[1;36m",
  "\n"
};

const char *pc_class_types[] = {
  "\x1B[0;35mTransmuter\x1B[0;0m",
  "\x1B[0;32mConjurer\x1B[0;0m",
  "\x1B[0;33mEmitter\x1B[0;0m",
  "\x1B[0;36mEnhancer\x1B[0;0m",
  "\x1B[1;30mManipulator\x1B[0;0m",
  "\x1B[0;31mSpecialist\x1B[0;0m",
  "\x1B[0;0mHunter",
  "\n"
};

const char *pc_special_abs[] = {
  "Menacing Presence",
  "Double Item Affect",
  "Armor Piercing",
  "Regenaration",
  "Headhunter",
  "Scarlet Eyes",
  "None",
  "\n"
};

const char *cosmetic_color[] = {
  "None",
  "White",
  "Pale",
  "Tan",
  "Black",
  "Yellow",
  "Azure",
  "Blue",
  "Crimson",
  "Red",
  "Green",
  "Emerald",
  "Magenta",
  "Pink",
  "Teal",
  "Cyan",
  "\n"
};

const char *eye_color[] = {
  "None",
  "Clouded",
  "White",
  "Hazel",
  "Black",
  "Yellow",
  "Azure",
  "Blue",
  "Scarlet",
  "Red",
  "Green",
  "Emerald",
  "Magenta",
  "Pink",
  "Teal",
  "Cyan",
  "\n"
};

const char *hair_style[] = {
  "None",
  "Bald",
  "Buzz cut",
  "Bowl cut",
  "Short",
  "Long",
  "Parted",
  "Flattop",
  "Afro",
  "Cornrows",
  "Dreadlocks",
  "Spiked",
  "Mohawk",
  "Bun",
  "Pigtails",
  "Ponytail",
  "\n"
};

/* The menu for choosing a class in interpreter.c: */
const char *class_menu =
"\r\n"
"Select a class number:\r\n\r\n"
"1. \x1B[0;36m\t<send href=\"1\">Enhancer\t</send>\x1B[0;0m\r\n"
"2. \x1B[0;33m\t<send href=\"2\">Emitter\t</send>\x1B[0;0m\r\n"
"3. \x1B[0;32m\t<send href=\"3\">Conjurer\t</send>\x1B[0;0m\r\n"
"4. \x1B[0;35m\t<send href=\"4\">Transmuter\t</send>\x1B[0;0m\r\n"
"5. \x1B[1;30m\t<send href=\"5\">Manipulator\t</send>\x1B[0;0m\r\n"
"6. \x1B[0;31m\t<send href=\"6\">Specialist\t</send>\tn\x1B[0;0m\r\n";

const char *skin_menu =
"\r\nSelect a skin style:\r\n\r\n"
" 1 = \tc[\twwhite\tc]\tn\r\n"
" 2 = \tc[\tWpale\tc]\tn\r\n"
" 3 = \tc[\tytan\tc]\tn\r\n"
" 4 = \tc[\tDblack\tc]\tn\r\n"
" 5 = \tc[\tYyellow\tc]\tn\r\n"
" 6 = \tc[\tbazure\tc]\tn\r\n"
" 7 = \tc[\tBblue\tc]\tn\r\n"
" 8 = \tc[\trcrimson\tc]\tn\r\n"
" 9 = \tc[\tRred\tc]\tn\r\n"
"10 = \tc[\tggreen\tc]\tn\r\n"
"11 = \tc[\tGemerald\tc]\tn\r\n"
"12 = \tc[\tmmagenta\tc]\tn\r\n"
"13 = \tc[\tMpink\tc]\tn\r\n"
"14 = \tc[\tcteal\tc]\tn\r\n"
"15 = \tc[\tCcyan\tc]\tn\r\n";

const char *eye_menu =
"\r\nSelect eye color:\r\n\r\n"
" 1 = \tc[\twclouded\tc]\tn\r\n"
" 2 = \tc[\tWwhite\tc]\tn\r\n"
" 3 = \tc[\tyhazel\tc]\tn\r\n"
" 4 = \tc[\tDblack\tc]\tn\r\n"
" 5 = \tc[\tYyellow\tc]\tn\r\n"
" 6 = \tc[\tbazure\tc]\tn\r\n"
" 7 = \tc[\tBblue\tc]\tn\r\n"
" 8 = \tc[\trscarlet\tc]\tn\r\n"
" 9 = \tc[\tRred\tc]\tn\r\n"
"10 = \tc[\tggreen\tc]\tn\r\n"
"11 = \tc[\tGemerald\tc]\tn\r\n"
"12 = \tc[\tmmagenta\tc]\tn\r\n"
"13 = \tc[\tMpink\tc]\tn\r\n"
"14 = \tc[\tcteal\tc]\tn\r\n"
"15 = \tc[\tCcyan\tc]\tn\r\n";

const char *hair_menu =
"\r\nSelect a hair style:\r\n\r\n"
" 1 = \tc[\tCbald\tc]\tn\r\n"
" 2 = \tc[\tCbuzz cut\tc]\tn\r\n"
" 3 = \tc[\tCbowl cut\tc]\tn\r\n"
" 4 = \tc[\tCshort\tc]\tn\r\n"
" 5 = \tc[\tClong\tc]\tn\r\n"
" 6 = \tc[\tCparted\tc]\tn\r\n"
" 7 = \tc[\tCflattop\tc]\tn\r\n"
" 8 = \tc[\tCafro\tc]\tn\r\n"
" 9 = \tc[\tCcornrows\tc]\tn\r\n"
"10 = \tc[\tCdreadlocks\tc]\tn\r\n"
"11 = \tc[\tCspiked\tc]\tn\r\n"
"12 = \tc[\tCmohawk\tc]\tn\r\n"
"13 = \tc[\tCbun\tc]\tn\r\n"
"14 = \tc[\tCpigtails\tc]\tn\r\n"
"15 = \tc[\tCponytail\tc]\tn\r\n";

const char *hcolor_menu =
"\r\nSelect a hair color:\r\n\r\n"
" 1 = \tc[\twsilver\tc]\tn\r\n"
" 2 = \tc[\tWwhite\tc]\tn\r\n"
" 3 = \tc[\tybrown\tc]\tn\r\n"
" 4 = \tc[\tDblack\tc]\tn\r\n"
" 5 = \tc[\tYyellow\tc]\tn\r\n"
" 6 = \tc[\tbazure\tc]\tn\r\n"
" 7 = \tc[\tBblue\tc]\tn\r\n"
" 8 = \tc[\trcrimson\tc]\tn\r\n"
" 9 = \tc[\tRred\tc]\tn\r\n"
"10 = \tc[\tggreen\tc]\tn\r\n"
"11 = \tc[\tGemerald\tc]\tn\r\n"
"12 = \tc[\tmmagenta\tc]\tn\r\n"
"13 = \tc[\tMpink\tc]\tn\r\n"
"14 = \tc[\tcteal\tc]\tn\r\n"
"15 = \tc[\tCcyan\tc]\tn\r\n";

/* The code to interpret a class letter -- used in interpreter.c when a new
 * character is selecting a class and by 'set class' in act.wizard.c. */
int parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case '4': return CLASS_MAGIC_USER;
  case '3': return CLASS_CLERIC;
  case '1': return CLASS_WARRIOR;
  case '2': return CLASS_THIEF;
  case '5': return CLASS_MANIPULATOR;
  case '6': return CLASS_SPECIALIST;
  default:  return CLASS_UNDEFINED;
  }
}

/* bitvectors (i.e., powers of two) for each class, mainly for use in do_who
 * and do_users.  Add new classes at the end so that all classes use sequential
 * powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, etc.) up to
 * the limit of your bitvector_t, typically 0-31. */
bitvector_t find_class_bitvector(const char *arg)
{
  size_t rpos, ret = 0;

  for (rpos = 0; rpos < strlen(arg); rpos++)
    ret |= (1 << parse_class(arg[rpos]));

  return (ret);
}

/* These are definitions which control the guildmasters for each class.
 * The  first field (top line) controls the highest percentage skill level a
 * character of the class is allowed to attain in any skill.  (After this
 * level, attempts to practice will say "You are already learned in this area."
 *
 * The second line controls the maximum percent gain in learnedness a character
 * is allowed per practice -- in other words, if the random die throw comes out
 * higher than this number, the gain will only be this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a character
 * is allowed per practice -- in other words, if the random die throw comes
 * out below this number, the gain will be set up to this number.
 *
 * The fourth line simply sets whether the character knows 'spells' or 'skills'.
 * This does not affect anything except the message given to the character when
 * trying to practice (i.e. "You know of the following spells" vs. "You know of
 * the following skills" */

#define SPELL	0
#define SKILL	1

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		2  min percent gain in skill per practice */
/* #define PRAC_TYPE		3  should it say 'spell' or 'skill'?	*/

int prac_params[8][NUM_CLASSES] = {
  /* MAG	CLE	THE	WAR MAN SPC HUN */
  { 100,	80,	60,	80, 40, 100, 0	},	/* learned level TRANSMUTER */
  { 80,		100, 40, 60, 60, 100, 1	},	/* learned level CONJURER */
  { 60,		40,	100,	80, 80, 100, 0	},	/* learned level EMITTER */
  { 80,		60,	80,	100, 60, 100, 0	},	/* learned level ENHANCER */
  { 40,		60,	80,	60, 100, 100, 1	},	/* learned level MANIPULATOR */
  { 0,		1,	0,	0, 1, 100, 0	},	/* learned level HUNTER */
  { 100,	100,	100,	100,	100, 100, 100	},	/* learned level SPECIALIST */
  { SPELL,	SPELL,	SPELL,	SKILL,	SPELL, SPELL	},	/* prac name */
};

/* The appropriate rooms for each guildmaster/guildguard; controls which types
 * of people the various guildguards let through.  i.e., the first line shows
 * that from room 3017, only MAGIC_USERS are allowed to go south. Don't forget
 * to visit spec_assign.c if you create any new mobiles that should be a guild
 * master or guard so they can act appropriately. If you "recycle" the
 * existing mobs that are used in other guilds for your new guild, then you
 * don't have to change that file, only here. Guildguards are now implemented
 * via triggers. This code remains as an example. */
struct guild_info_type guild_info[] = {

/* Midgaard
 { CLASS_MAGIC_USER,    3017,    SOUTH   },
 { CLASS_CLERIC,        3004,    NORTH   },
 { CLASS_THIEF,         3027,    EAST   },
 { CLASS_WARRIOR,       3021,    EAST   },
*/
/* Brass Dragon */
  { -999 /* all */ ,	5065,	WEST	},

/* this must go last -- add new guards above! */
  { -1, NOWHERE, -1}
};

/* Saving throws for : MCTW : PARA, ROD, PETRI, BREATH, SPELL. Levels 0-40. Do
 * not forget to change extern declaration in magic.c if you add to this. */
byte saving_throws(int class_num, int type, int level)
{
  switch (class_num) {
  case CLASS_MAGIC_USER:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 67;
      case  5: return 66;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 54;
      case 14: return 53;
      case 15: return 53;
      case 16: return 52;
      case 17: return 51;
      case 18: return 50;
      case 19: return 48;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 42;
      case 24: return 40;
      case 25: return 38;
      case 26: return 36;
      case 27: return 34;
      case 28: return 32;
      case 29: return 30;
      case 30: return 28;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 55;
      case  2: return 53;
      case  3: return 51;
      case  4: return 49;
      case  5: return 47;
      case  6: return 45;
      case  7: return 43;
      case  8: return 41;
      case  9: return 40;
      case 10: return 39;
      case 11: return 37;
      case 12: return 35;
      case 13: return 33;
      case 14: return 31;
      case 15: return 30;
      case 16: return 29;
      case 17: return 27;
      case 18: return 25;
      case 19: return 23;
      case 20: return 21;
      case 21: return 20;
      case 22: return 19;
      case 23: return 17;
      case 24: return 15;
      case 25: return 14;
      case 26: return 13;
      case 27: return 12;
      case 28: return 11;
      case 29: return 10;
      case 30: return  9;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage rod saving throw.");
	break;
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 63;
      case  3: return 61;
      case  4: return 59;
      case  5: return 57;
      case  6: return 55;
      case  7: return 53;
      case  8: return 51;
      case  9: return 50;
      case 10: return 49;
      case 11: return 47;
      case 12: return 45;
      case 13: return 43;
      case 14: return 41;
      case 15: return 40;
      case 16: return 39;
      case 17: return 37;
      case 18: return 35;
      case 19: return 33;
      case 20: return 31;
      case 21: return 30;
      case 22: return 29;
      case 23: return 27;
      case 24: return 25;
      case 25: return 23;
      case 26: return 21;
      case 27: return 19;
      case 28: return 17;
      case 29: return 15;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 53;
      case 14: return 51;
      case 15: return 50;
      case 16: return 49;
      case 17: return 47;
      case 18: return 45;
      case 19: return 43;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 37;
      case 24: return 35;
      case 25: return 33;
      case 26: return 31;
      case 27: return 29;
      case 28: return 27;
      case 29: return 25;
      case 30: return 23;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage breath saving throw.");
	break;
      }
    case SAVING_SPELL:	/* Generic spells */
      return (100 - level);
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
    break;
  case CLASS_CLERIC:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 60;
      case  2: return 59;
      case  3: return 48;
      case  4: return 46;
      case  5: return 45;
      case  6: return 43;
      case  7: return 40;
      case  8: return 37;
      case  9: return 35;
      case 10: return 34;
      case 11: return 33;
      case 12: return 31;
      case 13: return 30;
      case 14: return 29;
      case 15: return 27;
      case 16: return 26;
      case 17: return 25;
      case 18: return 24;
      case 19: return 23;
      case 20: return 22;
      case 21: return 21;
      case 22: return 20;
      case 23: return 18;
      case 24: return 15;
      case 25: return 14;
      case 26: return 12;
      case 27: return 10;
      case 28: return  9;
      case 29: return  8;
      case 30: return  7;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for cleric paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 66;
      case  5: return 65;
      case  6: return 63;
      case  7: return 60;
      case  8: return 57;
      case  9: return 55;
      case 10: return 54;
      case 11: return 53;
      case 12: return 51;
      case 13: return 50;
      case 14: return 49;
      case 15: return 47;
      case 16: return 46;
      case 17: return 45;
      case 18: return 44;
      case 19: return 43;
      case 20: return 42;
      case 21: return 41;
      case 22: return 40;
      case 23: return 38;
      case 24: return 35;
      case 25: return 34;
      case 26: return 32;
      case 27: return 30;
      case 28: return 29;
      case 29: return 28;
      case 30: return 27;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for cleric rod saving throw.");
	break;
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 64;
      case  3: return 63;
      case  4: return 61;
      case  5: return 60;
      case  6: return 58;
      case  7: return 55;
      case  8: return 53;
      case  9: return 50;
      case 10: return 49;
      case 11: return 48;
      case 12: return 46;
      case 13: return 45;
      case 14: return 44;
      case 15: return 43;
      case 16: return 41;
      case 17: return 40;
      case 18: return 39;
      case 19: return 38;
      case 20: return 37;
      case 21: return 36;
      case 22: return 35;
      case 23: return 33;
      case 24: return 31;
      case 25: return 29;
      case 26: return 27;
      case 27: return 25;
      case 28: return 24;
      case 29: return 23;
      case 30: return 22;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for cleric petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 79;
      case  3: return 78;
      case  4: return 76;
      case  5: return 75;
      case  6: return 73;
      case  7: return 70;
      case  8: return 67;
      case  9: return 65;
      case 10: return 64;
      case 11: return 63;
      case 12: return 61;
      case 13: return 60;
      case 14: return 59;
      case 15: return 57;
      case 16: return 56;
      case 17: return 55;
      case 18: return 54;
      case 19: return 53;
      case 20: return 52;
      case 21: return 51;
      case 22: return 50;
      case 23: return 48;
      case 24: return 45;
      case 25: return 44;
      case 26: return 42;
      case 27: return 40;
      case 28: return 39;
      case 29: return 38;
      case 30: return 37;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for cleric breath saving throw.");
	break;
      }
    case SAVING_SPELL:	/* Generic spells */
      return (100 - level);
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
    break;
  case CLASS_THIEF:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 64;
      case  3: return 63;
      case  4: return 62;
      case  5: return 61;
      case  6: return 60;
      case  7: return 59;
      case  8: return 58;
      case  9: return 57;
      case 10: return 56;
      case 11: return 55;
      case 12: return 54;
      case 13: return 53;
      case 14: return 52;
      case 15: return 51;
      case 16: return 50;
      case 17: return 49;
      case 18: return 48;
      case 19: return 47;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 43;
      case 24: return 42;
      case 25: return 41;
      case 26: return 40;
      case 27: return 39;
      case 28: return 38;
      case 29: return 37;
      case 30: return 36;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 68;
      case  3: return 66;
      case  4: return 64;
      case  5: return 62;
      case  6: return 60;
      case  7: return 58;
      case  8: return 56;
      case  9: return 54;
      case 10: return 52;
      case 11: return 50;
      case 12: return 48;
      case 13: return 46;
      case 14: return 44;
      case 15: return 42;
      case 16: return 40;
      case 17: return 38;
      case 18: return 36;
      case 19: return 34;
      case 20: return 32;
      case 21: return 30;
      case 22: return 28;
      case 23: return 26;
      case 24: return 24;
      case 25: return 22;
      case 26: return 20;
      case 27: return 18;
      case 28: return 16;
      case 29: return 14;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief rod saving throw.");
	break;
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 60;
      case  2: return 59;
      case  3: return 58;
      case  4: return 58;
      case  5: return 56;
      case  6: return 55;
      case  7: return 54;
      case  8: return 53;
      case  9: return 52;
      case 10: return 51;
      case 11: return 50;
      case 12: return 49;
      case 13: return 48;
      case 14: return 47;
      case 15: return 46;
      case 16: return 45;
      case 17: return 44;
      case 18: return 43;
      case 19: return 42;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 38;
      case 24: return 37;
      case 25: return 36;
      case 26: return 35;
      case 27: return 34;
      case 28: return 33;
      case 29: return 32;
      case 30: return 31;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 79;
      case  3: return 78;
      case  4: return 77;
      case  5: return 76;
      case  6: return 75;
      case  7: return 74;
      case  8: return 73;
      case  9: return 72;
      case 10: return 71;
      case 11: return 70;
      case 12: return 69;
      case 13: return 68;
      case 14: return 67;
      case 15: return 66;
      case 16: return 65;
      case 17: return 64;
      case 18: return 63;
      case 19: return 62;
      case 20: return 61;
      case 21: return 60;
      case 22: return 59;
      case 23: return 58;
      case 24: return 57;
      case 25: return 56;
      case 26: return 55;
      case 27: return 54;
      case 28: return 53;
      case 29: return 52;
      case 30: return 51;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief breath saving throw.");
	break;
      }
    case SAVING_SPELL:	/* Generic spells */
      return (100 - level);
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
    break;
  case CLASS_WARRIOR:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 68;
      case  3: return 67;
      case  4: return 65;
      case  5: return 62;
      case  6: return 58;
      case  7: return 55;
      case  8: return 53;
      case  9: return 52;
      case 10: return 50;
      case 11: return 47;
      case 12: return 43;
      case 13: return 40;
      case 14: return 38;
      case 15: return 37;
      case 16: return 35;
      case 17: return 32;
      case 18: return 28;
      case 19: return 25;
      case 20: return 24;
      case 21: return 23;
      case 22: return 22;
      case 23: return 20;
      case 24: return 19;
      case 25: return 17;
      case 26: return 16;
      case 27: return 15;
      case 28: return 14;
      case 29: return 13;
      case 30: return 12;
      case 31: return 11;
      case 32: return 10;
      case 33: return  9;
      case 34: return  8;
      case 35: return  7;
      case 36: return  6;
      case 37: return  5;
      case 38: return  4;
      case 39: return  3;
      case 40: return  2;
      case 41: return  1;	/* Some mobiles. */
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 78;
      case  3: return 77;
      case  4: return 75;
      case  5: return 72;
      case  6: return 68;
      case  7: return 65;
      case  8: return 63;
      case  9: return 62;
      case 10: return 60;
      case 11: return 57;
      case 12: return 53;
      case 13: return 50;
      case 14: return 48;
      case 15: return 47;
      case 16: return 45;
      case 17: return 42;
      case 18: return 38;
      case 19: return 35;
      case 20: return 34;
      case 21: return 33;
      case 22: return 32;
      case 23: return 30;
      case 24: return 29;
      case 25: return 27;
      case 26: return 26;
      case 27: return 25;
      case 28: return 24;
      case 29: return 23;
      case 30: return 22;
      case 31: return 20;
      case 32: return 18;
      case 33: return 16;
      case 34: return 14;
      case 35: return 12;
      case 36: return 10;
      case 37: return  8;
      case 38: return  6;
      case 39: return  5;
      case 40: return  4;
      case 41: return  3;
      case 42: return  2;
      case 43: return  1;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior rod saving throw.");
	break;
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 72;
      case  4: return 70;
      case  5: return 67;
      case  6: return 63;
      case  7: return 60;
      case  8: return 58;
      case  9: return 57;
      case 10: return 55;
      case 11: return 52;
      case 12: return 48;
      case 13: return 45;
      case 14: return 43;
      case 15: return 42;
      case 16: return 40;
      case 17: return 37;
      case 18: return 33;
      case 19: return 30;
      case 20: return 29;
      case 21: return 28;
      case 22: return 26;
      case 23: return 25;
      case 24: return 24;
      case 25: return 23;
      case 26: return 21;
      case 27: return 20;
      case 28: return 19;
      case 29: return 18;
      case 30: return 17;
      case 31: return 16;
      case 32: return 15;
      case 33: return 14;
      case 34: return 13;
      case 35: return 12;
      case 36: return 11;
      case 37: return 10;
      case 38: return  9;
      case 39: return  8;
      case 40: return  7;
      case 41: return  6;
      case 42: return  5;
      case 43: return  4;
      case 44: return  3;
      case 45: return  2;
      case 46: return  1;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 85;
      case  2: return 83;
      case  3: return 82;
      case  4: return 80;
      case  5: return 75;
      case  6: return 70;
      case  7: return 65;
      case  8: return 63;
      case  9: return 62;
      case 10: return 60;
      case 11: return 55;
      case 12: return 50;
      case 13: return 45;
      case 14: return 43;
      case 15: return 42;
      case 16: return 40;
      case 17: return 37;
      case 18: return 33;
      case 19: return 30;
      case 20: return 29;
      case 21: return 28;
      case 22: return 26;
      case 23: return 25;
      case 24: return 24;
      case 25: return 23;
      case 26: return 21;
      case 27: return 20;
      case 28: return 19;
      case 29: return 18;
      case 30: return 17;
      case 31: return 16;
      case 32: return 15;
      case 33: return 14;
      case 34: return 13;
      case 35: return 12;
      case 36: return 11;
      case 37: return 10;
      case 38: return  9;
      case 39: return  8;
      case 40: return  7;
      case 41: return  6;
      case 42: return  5;
      case 43: return  4;
      case 44: return  3;
      case 45: return  2;
      case 46: return  1;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior breath saving throw.");
	break;
      }
    case SAVING_SPELL:	/* Generic spells */
      return (100 - level);
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
	  case CLASS_MANIPULATOR:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 67;
      case  5: return 66;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 54;
      case 14: return 53;
      case 15: return 53;
      case 16: return 52;
      case 17: return 51;
      case 18: return 50;
      case 19: return 48;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 42;
      case 24: return 40;
      case 25: return 38;
      case 26: return 36;
      case 27: return 34;
      case 28: return 32;
      case 29: return 30;
      case 30: return 28;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for manipulator paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 55;
      case  2: return 53;
      case  3: return 51;
      case  4: return 49;
      case  5: return 47;
      case  6: return 45;
      case  7: return 43;
      case  8: return 41;
      case  9: return 40;
      case 10: return 39;
      case 11: return 37;
      case 12: return 35;
      case 13: return 33;
      case 14: return 31;
      case 15: return 30;
      case 16: return 29;
      case 17: return 27;
      case 18: return 25;
      case 19: return 23;
      case 20: return 21;
      case 21: return 20;
      case 22: return 19;
      case 23: return 17;
      case 24: return 15;
      case 25: return 14;
      case 26: return 13;
      case 27: return 12;
      case 28: return 11;
      case 29: return 10;
      case 30: return  9;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for manipulator rod saving throw.");
	break;
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 63;
      case  3: return 61;
      case  4: return 59;
      case  5: return 57;
      case  6: return 55;
      case  7: return 53;
      case  8: return 51;
      case  9: return 50;
      case 10: return 49;
      case 11: return 47;
      case 12: return 45;
      case 13: return 43;
      case 14: return 41;
      case 15: return 40;
      case 16: return 39;
      case 17: return 37;
      case 18: return 35;
      case 19: return 33;
      case 20: return 31;
      case 21: return 30;
      case 22: return 29;
      case 23: return 27;
      case 24: return 25;
      case 25: return 23;
      case 26: return 21;
      case 27: return 19;
      case 28: return 17;
      case 29: return 15;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for manipulator petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 53;
      case 14: return 51;
      case 15: return 50;
      case 16: return 49;
      case 17: return 47;
      case 18: return 45;
      case 19: return 43;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 37;
      case 24: return 35;
      case 25: return 33;
      case 26: return 31;
      case 27: return 29;
      case 28: return 27;
      case 29: return 25;
      case 30: return 23;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for manipulator breath saving throw.");
	break;
      }
    case SAVING_SPELL:	/* Generic spells */
      return (100 - level);
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
	}
	case CLASS_SPECIALIST:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 67;
      case  5: return 66;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 54;
      case 14: return 53;
      case 15: return 53;
      case 16: return 52;
      case 17: return 51;
      case 18: return 50;
      case 19: return 48;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 42;
      case 24: return 40;
      case 25: return 38;
      case 26: return 36;
      case 27: return 34;
      case 28: return 32;
      case 29: return 30;
      case 30: return 28;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 55;
      case  2: return 53;
      case  3: return 51;
      case  4: return 49;
      case  5: return 47;
      case  6: return 45;
      case  7: return 43;
      case  8: return 41;
      case  9: return 40;
      case 10: return 39;
      case 11: return 37;
      case 12: return 35;
      case 13: return 33;
      case 14: return 31;
      case 15: return 30;
      case 16: return 29;
      case 17: return 27;
      case 18: return 25;
      case 19: return 23;
      case 20: return 21;
      case 21: return 20;
      case 22: return 19;
      case 23: return 17;
      case 24: return 15;
      case 25: return 14;
      case 26: return 13;
      case 27: return 12;
      case 28: return 11;
      case 29: return 10;
      case 30: return  9;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage rod saving throw.");
	break;
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 63;
      case  3: return 61;
      case  4: return 59;
      case  5: return 57;
      case  6: return 55;
      case  7: return 53;
      case  8: return 51;
      case  9: return 50;
      case 10: return 49;
      case 11: return 47;
      case 12: return 45;
      case 13: return 43;
      case 14: return 41;
      case 15: return 40;
      case 16: return 39;
      case 17: return 37;
      case 18: return 35;
      case 19: return 33;
      case 20: return 31;
      case 21: return 30;
      case 22: return 29;
      case 23: return 27;
      case 24: return 25;
      case 25: return 23;
      case 26: return 21;
      case 27: return 19;
      case 28: return 17;
      case 29: return 15;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 53;
      case 14: return 51;
      case 15: return 50;
      case 16: return 49;
      case 17: return 47;
      case 18: return 45;
      case 19: return 43;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 37;
      case 24: return 35;
      case 25: return 33;
      case 26: return 31;
      case 27: return 29;
      case 28: return 27;
      case 29: return 25;
      case 30: return 23;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage breath saving throw.");
	break;
      }
    case SAVING_SPELL:	/* Generic spells */
      return (100 - level);
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
	case CLASS_HUNTER:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 67;
      case  5: return 66;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 54;
      case 14: return 53;
      case 15: return 53;
      case 16: return 52;
      case 17: return 51;
      case 18: return 50;
      case 19: return 48;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 42;
      case 24: return 40;
      case 25: return 38;
      case 26: return 36;
      case 27: return 34;
      case 28: return 32;
      case 29: return 30;
      case 30: return 28;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 55;
      case  2: return 53;
      case  3: return 51;
      case  4: return 49;
      case  5: return 47;
      case  6: return 45;
      case  7: return 43;
      case  8: return 41;
      case  9: return 40;
      case 10: return 39;
      case 11: return 37;
      case 12: return 35;
      case 13: return 33;
      case 14: return 31;
      case 15: return 30;
      case 16: return 29;
      case 17: return 27;
      case 18: return 25;
      case 19: return 23;
      case 20: return 21;
      case 21: return 20;
      case 22: return 19;
      case 23: return 17;
      case 24: return 15;
      case 25: return 14;
      case 26: return 13;
      case 27: return 12;
      case 28: return 11;
      case 29: return 10;
      case 30: return  9;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage rod saving throw.");
	break;
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 63;
      case  3: return 61;
      case  4: return 59;
      case  5: return 57;
      case  6: return 55;
      case  7: return 53;
      case  8: return 51;
      case  9: return 50;
      case 10: return 49;
      case 11: return 47;
      case 12: return 45;
      case 13: return 43;
      case 14: return 41;
      case 15: return 40;
      case 16: return 39;
      case 17: return 37;
      case 18: return 35;
      case 19: return 33;
      case 20: return 31;
      case 21: return 30;
      case 22: return 29;
      case 23: return 27;
      case 24: return 25;
      case 25: return 23;
      case 26: return 21;
      case 27: return 19;
      case 28: return 17;
      case 29: return 15;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 53;
      case 14: return 51;
      case 15: return 50;
      case 16: return 49;
      case 17: return 47;
      case 18: return 45;
      case 19: return 43;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 37;
      case 24: return 35;
      case 25: return 33;
      case 26: return 31;
      case 27: return 29;
      case 28: return 27;
      case 29: return 25;
      case 30: return 23;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage breath saving throw.");
	break;
      }
    case SAVING_SPELL:	/* Generic spells */
      return (100 - level);
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
  default:
    log("SYSERR: Invalid class saving throw.");
    break;
  }

  /* Should not get here unless something is wrong. */
  return 100;
}

/* THAC0 for classes and levels.  (To Hit Armor Class 0) */
int thaco(int class_num, int level)
{
  if (level <= 0)
	return 100;
  else if (level < 31)
    return ((32 - level) / 1.5);
  else
	return 1;  
}


/* Roll the 6 stats for a character... each stat is made of the sum of the best
 * 3 out of 4 rolls of a 6-sided die.  Each class then decides which priority
 * will be given for the best to worst stats. */
void roll_real_abils(struct char_data *ch)
{
  int i, j, k, temp;
  ubyte table[6];
  ubyte rolls[4];

  for (i = 0; i < 6; i++)
    table[i] = 0;

  for (i = 0; i < 6; i++) {

    for (j = 0; j < 4; j++)
      rolls[j] = rand_number(1, 6);

    temp = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
      MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));

    for (k = 0; k < 6; k++)
      if (table[k] < temp) {
	temp ^= table[k];
	table[k] ^= temp;
	temp ^= table[k];
      }
  }

  ch->real_abils.str_add = 0;

  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
    ch->real_abils.intel = table[0];
    ch->real_abils.wis = table[1];
    ch->real_abils.dex = table[2];
    ch->real_abils.str = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_CLERIC:
    ch->real_abils.wis = table[0];
    ch->real_abils.intel = table[1];
    ch->real_abils.str = table[2];
    ch->real_abils.dex = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_THIEF:
    ch->real_abils.dex = table[0];
    ch->real_abils.str = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_WARRIOR:
    ch->real_abils.str = table[0];
    ch->real_abils.con = table[1];
    ch->real_abils.dex = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = rand_number(0, 100);
    break;
  case CLASS_MANIPULATOR:
    ch->real_abils.cha = table[0];
	ch->real_abils.wis = table[1];
	ch->real_abils.intel = table[2];
	ch->real_abils.dex = table[3];
	ch->real_abils.str = table[4];
    ch->real_abils.con = table[5];  
    break;
  case CLASS_SPECIALIST:
    ch->real_abils.wis = 6 + (rand_number(1, 6) + rand_number(1, 6));
	ch->real_abils.intel = 6 + (rand_number(1, 6) + rand_number(1, 6));
	ch->real_abils.cha = 6 + (rand_number(1, 6) + rand_number(1, 6));
    ch->real_abils.dex = (rand_number(1, 6) + rand_number(1, 6) + rand_number(1, 6));
    ch->real_abils.str = (rand_number(1, 6) + rand_number(1, 6) + rand_number(1, 6));
    ch->real_abils.con = (rand_number(1, 6) + rand_number(1, 6) + rand_number(1, 6));
    
    break;
  case CLASS_HUNTER:
    ch->real_abils.wis = table[0];
	ch->real_abils.intel = table[1];
    ch->real_abils.dex = table[2];
    ch->real_abils.str = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  }
  ch->aff_abils = ch->real_abils;
}

/* Some initializations for characters, including initial skills */
void do_start(struct char_data *ch)
{
  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;  
  
  GET_MAX_HIT(ch)  = 250;  
  GET_MAX_MANA(ch) = 100;
  GET_MAX_MOVE(ch) = 100;  
  
  GET_PAGE_LENGTH(ch) = 40;
  
  SET_SKILL(ch, SKILL_POWER, (GET_WIS(ch) + 1));    
  SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOGOLD);
  SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTODOOR);
  SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOMAP); 

  advance_level(ch);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  
  GET_COND(ch, THIRST) = 100;
  GET_COND(ch, HUNGER) = 100;
  GET_COND(ch, DRUNK) = 0;
  GET_ADD_HIT(ch)  = 0; 
  
  if (CONFIG_SITEOK_ALL)
    SET_BIT_AR(PLR_FLAGS(ch), PLR_SITEOK);
}

/* This function controls the change to maxmove, maxmana, and maxhp for each
 * class every time they gain a level. */
void advance_level(struct char_data *ch)
{
  int i, pract = 3;
/*  int add_hp, add_mana = 0, add_move = 0, i;
  add_hp = con_app[GET_CON(ch)].hitp;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    add_hp += rand_number(((GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8), (int)(2 * (GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8));
    add_mana = rand_number(((GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5), (int)(2 * (GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5));
    add_mana = MIN(add_mana, 10);
    add_move = rand_number(((GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10));
    break;

  case CLASS_CLERIC:
    add_hp += rand_number(((GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8), (int)(2 * (GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8));
    add_mana = rand_number(((GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5), (int)(2 * (GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5));
    add_mana = MIN(add_mana, 10);
    add_move = rand_number(((GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10));
    break;

  case CLASS_THIEF:
    add_hp += rand_number(((GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8), (int)(2 * (GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8));
    add_mana = rand_number(((GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5), (int)(2 * (GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5));
	add_mana = MIN(add_mana, 10);
    add_move = rand_number(((GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10));
    break;

  case CLASS_WARRIOR:
    add_hp += rand_number(((GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 6), (int)(2 * (GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 6));
    add_mana = rand_number(((GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 10));
    add_move = rand_number(((GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10));
    break;
	
  case CLASS_MANIPULATOR:
    add_hp += rand_number(((GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8), (int)(2 * (GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8));
    add_mana = rand_number(((GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5), (int)(2 * (GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5));
    add_mana = MIN(add_mana, 10);
    add_move = rand_number(((GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10));
    break;
	
  case CLASS_SPECIALIST:
    add_hp += rand_number(((GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8), (int)(2 * (GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8));
    add_mana = rand_number(((GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5), (int)(2 * (GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5));
    add_mana = MIN(add_mana, 10);
    add_move = rand_number(((GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10));
    break;
	
  case CLASS_HUNTER:
    add_hp += rand_number(((GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8), (int)(2 * (GET_LEVEL(ch) + (GET_CON(ch) * 2)) / 8));
    add_mana = rand_number(((GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5), (int)(2 * (GET_LEVEL(ch) + (GET_INT(ch) * 2)) / 5));
    add_mana = MIN(add_mana, 10);
    add_move = rand_number(((GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10), (int)(2 * (GET_LEVEL(ch) + (GET_DEX(ch) * 2)) / 10));
    break;
	
  }

  ch->points.max_hit += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);

  if (GET_LEVEL(ch) > 1)
    ch->points.max_mana += add_mana;

  if (IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_THIEF(ch) || IS_MANIPULATOR(ch) || IS_SPECIALIST(ch))
    GET_PRACTICES(ch) += MAX(3, wis_app[GET_WIS(ch)].bonus);
  else
    GET_PRACTICES(ch) += MIN(3, MAX(2, wis_app[GET_WIS(ch)].bonus));


  if (GET_LEVEL(ch) % 2)
    pract = 2;
  else
	pract = 3;
*/
  GET_PRACTICES(ch) += pract;
  send_to_char(ch, "\tDYour upgrade points was increased by \tG%d\tD!\tn\r\n", pract);
  
  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (char) -1;
    SET_BIT_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT);
  }

  snoop_check(ch);
  save_char(ch);
}

/* This simply calculates the backstab multiplier based on a character's level.
 * This used to be an array, but was changed to be a function so that it would
 * be easier to add more levels to your MUD.  This doesn't really create a big
 * performance hit because it's not used very often. */
int backstab_mult(int level)
{
  if (level <= 7)
    return 2;	  /* level 1 - 7 */
  else if (level <= 13)
    return 3;	  /* level 8 - 13 */
  else if (level <= 20)
    return 4;	  /* level 14 - 20 */
  else if (level <= 28)
    return 5;	  /* level 21 - 28 */
  else if (level < LVL_IMMORT)
    return 6;	  /* all remaining mortal levels */
  else
    return 20;	  /* immortals */
}

/* invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors. */
int invalid_class(struct char_data *ch, struct obj_data *obj)
{
  if (OBJ_FLAGGED(obj, ITEM_ANTI_MAGIC_USER) && IS_MAGIC_USER(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_CLERIC) && IS_CLERIC(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_WARRIOR) && IS_WARRIOR(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch))
    return TRUE;

  return FALSE;
}

/* SPELLS AND SKILLS.  This area defines which spells are assigned to which
 * classes, and the minimum level the character must be to use the spell or
 * skill. */
void init_spell_levels(void)
{
	
  /* TRANSMUTER */
  spell_level(SPELL_BURNING_HANDS, CLASS_MAGIC_USER, 1);
  spell_level(SPELL_INFRAVISION, CLASS_MAGIC_USER, 2);
  spell_level(SPELL_CURE_LIGHT, CLASS_MAGIC_USER, 3);
  spell_level(SPELL_MASSAGIST, CLASS_MAGIC_USER, 5);
  spell_level(SPELL_SHOCKING_GRASP, CLASS_MAGIC_USER, 7);
  spell_level(SPELL_CURE_CRITIC, CLASS_MAGIC_USER, 10);
  spell_level(SPELL_ENCHANT_WEAPON, CLASS_MAGIC_USER, 12);
  spell_level(SPELL_LIGHTNING_BOLT, CLASS_MAGIC_USER, 14);
  spell_level(SPELL_COLOR_SPRAY, CLASS_MAGIC_USER, 17);
  spell_level(SPELL_HARM, CLASS_MAGIC_USER, 23);
  

  /* CONJURER */
  spell_level(SPELL_TIGHTEN_CHAINS, CLASS_CLERIC, 1);
  spell_level(SPELL_CREATE_FOOD, CLASS_CLERIC, 2);
  spell_level(SPELL_CREATE_WATER, CLASS_CLERIC, 3);  
  spell_level(SPELL_IDENTIFY, CLASS_CLERIC, 4);
  spell_level(SPELL_REPAIR, CLASS_CLERIC, 5);
  spell_level(SPELL_INVISIBLE, CLASS_CLERIC, 7);  
  spell_level(SPELL_REMOVE_CURSE, CLASS_CLERIC, 10);  
  spell_level(SPELL_CALL_LIGHTNING, CLASS_CLERIC, 12);
  spell_level(SPELL_CLONE, CLASS_CLERIC, 14);
  spell_level(SPELL_HEAL, CLASS_CLERIC, 16);
  spell_level(SPELL_GROUP_ARMOR, CLASS_CLERIC,18);  
  spell_level(SPELL_GROUP_HEAL, CLASS_CLERIC, 20);
  

  /* EMISSOR */
  spell_level(SPELL_MAGIC_MISSILE, CLASS_THIEF, 1);  
  spell_level(SPELL_DARKNESS, CLASS_THIEF, 2);
  spell_level(SPELL_WATERWALK, CLASS_THIEF, 3);
  spell_level(SKILL_DODGE, CLASS_THIEF, 5);
  spell_level(SPELL_TELEPORT, CLASS_THIEF, 7);  
  spell_level(SPELL_NEN_BEAST, CLASS_THIEF, 8);
  spell_level(SKILL_RECALL, CLASS_THIEF, 9);
  spell_level(SKILL_SENSE, CLASS_THIEF, 10);
  spell_level(SPELL_FLY, CLASS_THIEF, 12);
  spell_level(SKILL_REMOTE_PUNCH, CLASS_THIEF, 15);  
  spell_level(SPELL_SENSE_LIFE, CLASS_THIEF, 18);
  spell_level(SPELL_FIREBALL, CLASS_THIEF, 22);
  


  /* ENHANCER */
  spell_level(SKILL_KICK, CLASS_WARRIOR, 1);
  spell_level(SPELL_REMOVE_POISON, CLASS_WARRIOR, 3);
  spell_level(SPELL_CURE_BLIND, CLASS_WARRIOR, 4);
  spell_level(SKILL_JAJANKEN, CLASS_WARRIOR, 5);  
  spell_level(SKILL_PARRY, CLASS_WARRIOR, 7);
  spell_level(SKILL_BASH, CLASS_WARRIOR, 10);
  spell_level(SKILL_BACKSTAB, CLASS_WARRIOR, 12);  
  spell_level(SPELL_STRENGTH, CLASS_WARRIOR, 16);
  spell_level(SPELL_EARTHQUAKE, CLASS_WARRIOR, 20);
  spell_level(SKILL_BAREHANDED_EXPERT, CLASS_WARRIOR, 21);
  spell_level(SKILL_ENHANCE, CLASS_WARRIOR, 23);
  spell_level(SPELL_SANCTUARY, CLASS_WARRIOR, 25);
    
  
  /* MANIPULATOR */
  spell_level(SPELL_CHILL_TOUCH, CLASS_MANIPULATOR, 1);
  spell_level(SPELL_DETECT_POISON, CLASS_MANIPULATOR, 2);
  spell_level(SPELL_DETECT_MAGIC, CLASS_MANIPULATOR, 3);
  spell_level(SPELL_DETECT_ALIGN, CLASS_MANIPULATOR, 4);
  spell_level(SPELL_BLESS, CLASS_MANIPULATOR, 5);  
  spell_level(SPELL_ANIMATE_DEAD, CLASS_MANIPULATOR, 6);
  spell_level(SPELL_POISON, CLASS_MANIPULATOR, 7);
  spell_level(SPELL_BLINDNESS, CLASS_MANIPULATOR, 8);
  spell_level(SPELL_CURSE, CLASS_MANIPULATOR, 9);
  spell_level(SPELL_DISPEL_EVIL, CLASS_MANIPULATOR, 10);
  spell_level(SPELL_DISPEL_GOOD, CLASS_MANIPULATOR, 12);  
  spell_level(SPELL_SLEEP, CLASS_MANIPULATOR, 15);  
  spell_level(SPELL_ENERGY_DRAIN, CLASS_MANIPULATOR, 20);
  spell_level(SPELL_CHARM, CLASS_MANIPULATOR, 25);

  /* SPECIALIST */  
  spell_level(SPELL_GROUP_RECALL, CLASS_SPECIALIST, 10);  
  spell_level(SPELL_LOCATE_OBJECT, CLASS_SPECIALIST, 20);   
  spell_level(SPELL_SUMMON, CLASS_SPECIALIST, 30);
  spell_level(SPELL_LUCK, CLASS_SPECIALIST, 40);
  
  /* HUNTER */
  spell_level(SKILL_POWER, CLASS_HUNTER, 1);
  spell_level(SPELL_DETECT_INVIS, CLASS_HUNTER, 1);
  spell_level(SKILL_PICK_LOCK, CLASS_HUNTER, 2);
  spell_level(SKILL_ANALYSIS, CLASS_HUNTER, 3);
  spell_level(SKILL_HIDE, CLASS_HUNTER, 4);
  spell_level(SKILL_RESCUE, CLASS_HUNTER, 5);
  spell_level(SKILL_SNEAK, CLASS_HUNTER, 6);  
  spell_level(SKILL_TRACK, CLASS_HUNTER, 7);  
  spell_level(SKILL_STEAL, CLASS_HUNTER, 8);
  spell_level(SPELL_ARMOR, CLASS_HUNTER, 9);
  spell_level(SKILL_SECOND_ATTACK, CLASS_HUNTER, 10);
  spell_level(SKILL_INSTANT_FORTIFY, CLASS_HUNTER, 15);
  spell_level(SKILL_THIRD_ATTACK, CLASS_HUNTER, 20);  
  spell_level(SKILL_FOURTH_ATTACK, CLASS_HUNTER, 30);    
  
}

/* This is the exp given to implementors -- it must always be greater than the
 * exp required for immortality, plus at least 20,000 or so. */
#define EXP_MAX  1000000000

/* Function to return the exp required for each class/level */
int level_exp(int level)
{  
  
  if (level > LVL_IMPL || level <= 0) {
    log("SYSERR: Requesting exp for invalid level %d!", level);
    return 0;
  }

  /* Game Masters have exp close to EXP_MAX.  This statement should never have to
   * changed, regardless of how many mortal or immortal levels exist. */
   if (level > LVL_IMMORT) {
     return EXP_MAX - ((LVL_IMPL - level) * 1000);
   }
   
    switch (level) {
	  case 1: return 250;
	  case 2: return 1000;
	  case 3: return 2250;
	  case 4: return 3750;
	  case 5: return 5500;
	  case 6: return 7500;
	  case 7: return 10000;
	  case 8: return 13000;
	  case 9: return 16500;
      case 10: return 20500;
	  case 11: return 26000;
	  case 12: return 32000;
	  case 13: return 39000;
	  case 14: return 47000;
	  case 15: return 57000;
	  case 16: return 69000;
	  case 17: return 83000;
	  case 18: return 99000;
	  case 19: return 119000;
      case 20: return 143000;
	  case 21: return 175000;
	  case 22: return 210000;
	  case 23: return 255000;
	  case 24: return 310000;
	  case 25: return 375000;
	  case 26: return 450000;
	  case 27: return 550000;
	  case 28: return 675000;
	  case 29: return 825000;
      case 30: return 1000000;
	  case 31: return 2250000;
	  case 32: return 3750000;
	  case 33: return 5500000;
	  case 34: return 7500000;
	  case 35: return 10000000;
	  case 36: return 13000000;
	  case 37: return 16500000;
	  case 38: return 20500000;
	  case 39: return 26000000;
	  case 40: return 32000000;
	  case 41: return 39000000;
	  case 42: return 47000000;
	  case 43: return 57000000;
	  case 44: return 69000000;
	  case 45: return 83000000;
	  case 46: return 99000000;
	  case 47: return 119000000;
	  case 48: return 143000000;
	  case 49: return 175000000;
	  case 50: return 210000000;
	  default: return EXP_MAX;
	}

  /* This statement should never be reached if the exp tables in this function
   * are set up properly.  If you see exp of 123456 then the tables above are
   * incomplete. */
  log("SYSERR: XP tables not set up correctly in class.c!");
  return 123456;
}

/* Default titles of male characters. */
const char *title_hunter(int level)
{
  static char buf[MAX_STRING_LENGTH];
  int rank;  
  
  if (level <= 0 || level > LVL_IMPL)
    return "the Hunter";
  if (level == LVL_IMPL)
    return "the Chairman";

  switch (level) {
	  case 1: return "a H rank Hunter";
	  case 2: return "a G rank Hunter";
	  case 3: return "a F rank Hunter";
	  case 4: return "a E rank Hunter";
	  case 5: return "a D rank Hunter";
	  case 6: return "a C rank Hunter";
	  case 7: return "a B rank Hunter";
	  case 8: return "a A rank Hunter";
	  case 9: return "a S rank Hunter";
      case 10: return "a 1-Star Hunter";
	  case 11: return "a 1S H rank Hunter";
	  case 12: return "a 1S G rank Hunter";
	  case 13: return "a 1S F rank Hunter";
	  case 14: return "a 1S E rank Hunter";
	  case 15: return "a 1S D rank Hunter";
	  case 16: return "a 1S C rank Hunter";
	  case 17: return "a 1S B rank Hunter";
	  case 18: return "a 1S A rank Hunter";
	  case 19: return "a 1S S rank Hunter";
      case 20: return "a 2-Star Hunter";
	  case 21: return "a 2S H rank Hunter";
	  case 22: return "a 2S G rank Hunter";
	  case 23: return "a 2S F rank Hunter";
	  case 24: return "a 2S E rank Hunter";
	  case 25: return "a 2S D rank Hunter";
	  case 26: return "a 2S C rank Hunter";
	  case 27: return "a 2S B rank Hunter";
	  case 28: return "a 2S A rank Hunter";
	  case 29: return "a 2S S rank Hunter";
      case 30: return "the 3-Star Hunter";
	  case 31: return "the 3S H rank Hunter";
	  case 32: return "the 3S G rank Hunter";
	  case 33: return "the 3S F rank Hunter";
	  case 34: return "the 3S E rank Hunter";
	  case 35: return "the 3S D rank Hunter";
	  case 36: return "the 3S C rank Hunter";
	  case 37: return "the 3S B rank Hunter";
	  case 38: return "the 3S A rank Hunter";
	  case 39: return "the 3S S rank Hunter";
	  case 40: return "the 3S SS rank Hunter";
	  default:
	    rank = 51 - level;
		sprintf(buf, "the SS-%d rank Hunter", rank);
		return (buf);
  }
  

  /* Default title for classes which do not have titles defined */
  return "the Classless";
}

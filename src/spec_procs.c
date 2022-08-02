/**************************************************************************
*  File: spec_procs.c                                      Part of tbaMUD *
*  Usage: Implementation of special procedures for mobiles/objects/rooms. *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

/* For more examples: 
 * ftp://ftp.circlemud.org/pub/CircleMUD/contrib/snippets/specials */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "act.h"
#include "spec_procs.h"
#include "class.h"
#include "fight.h"
#include "modify.h"


/* locally defined functions of local (file) scope */
static int compare_spells(const void *x, const void *y);
static void npc_steal(struct char_data *ch, struct char_data *victim);

/* Special procedures for mobiles. */
static int spell_sort_info[MAX_SKILLS + 1];



static int compare_spells(const void *x, const void *y)
{
  int	a = *(const int *)x,
	b = *(const int *)y;

  return strcmp(spell_info[a].name, spell_info[b].name);
}

void sort_spells(void)
{
  int a;

  /* initialize array, avoiding reserved. */
  for (a = 1; a <= MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  qsort(&spell_sort_info[1], MAX_SKILLS, sizeof(int), compare_spells);
}

const char *prac_types[] = {
  "skill",
  "skill"
};

#define LEARNED_TRA	0	/* % known which is considered "learned" */
#define LEARNED_CON	1	/* % known which is considered "learned" */
#define LEARNED_EMT	2	/* % known which is considered "learned" */
#define LEARNED_ENH	3	/* % known which is considered "learned" */
#define LEARNED_MAN	4	/* % known which is considered "learned" */
#define LEARNED_SPE	5	/* % known which is considered "learned" */
#define LEARNED_SKL	6	/* % known which is considered "learned" */
#define PRAC_TYPE	7	/* should it say 'spell' or 'skill'?	 */

#define LEARNED0(ch) (prac_params[LEARNED_TRA][(int)GET_CLASS(ch)])
#define LEARNED1(ch) (prac_params[LEARNED_CON][(int)GET_CLASS(ch)])
#define LEARNED2(ch) (prac_params[LEARNED_EMT][(int)GET_CLASS(ch)])
#define LEARNED3(ch) (prac_params[LEARNED_ENH][(int)GET_CLASS(ch)])
#define LEARNED4(ch) (prac_params[LEARNED_MAN][(int)GET_CLASS(ch)])
#define LEARNED5(ch) (prac_params[LEARNED_SPE][(int)GET_CLASS(ch)])
#define LEARNEDSKL(ch) (prac_params[LEARNED_SKL][(int)GET_CLASS(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])

/** Practice chance for all skills when used. */
/*void pracskill(struct char_data *ch, int skill_num, int chance)
{
  struct obj_data *booster;
  int i, skilladd, dice, found = 0;
	
  skill_num = find_skill_num(skill_num);
  dice = rand_number(1, 20);  
  
  if (!ch || IS_NPC(ch) || dice == 1)
	return;
  
  for (i = 0; i < NUM_CLASSES; i++){    
	if (GET_LEVEL(ch) >= spell_info[skill_num].min_level[i])
      found = i;	  
  }
 
  if ((GET_SKILL(ch, skill_num) < (prac_params[found][(int) GET_CLASS(ch)])) && ((dice + (((wis_app[GET_WIS(ch)].bonus) / 2) + 1) >= chance) || dice == 20)){ 
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
*/
void list_of_skills(struct char_data *ch, int lvl)
{
  int i, skl, skclass = 0, sortpos, line = 0;
/*  size_t len = 0;
  char buf2[MAX_STRING_LENGTH];*/
  const char *skill_area[] = {	
	"Transmuter Skill",
	"Conjurer Skill",
	"Emitter Skill",
	"Enhancer Skills",
	"Manipulator Skill",
	"Specialist Skill",
	"Hunter Skills"
  };

//	len = snprintf(buf2, sizeof(buf2), "\r\n[ ::::::::::: \tc%-17.17s (max %3d%%)\tn :::::::::::: ]\r\n\r\n", skill_area[6], LEARNED5(ch));
  
  send_to_char(ch, "[ ::::::::::: %s%-17.17s (max %3d%%)\tn :::::::::::: ]\r\n", class_colors[6], skill_area[6], LEARNEDSKL(ch));
  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {    
	i = spell_sort_info[sortpos];
	skl = spell_info[i].min_level[6];	
    if (lvl >= skl) {
      if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(L%3d\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(L%3d\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
		line = 1;
	  }
    }
	sortpos++;
	i = spell_sort_info[sortpos];
	skl = spell_info[i].min_level[6];	
	if (lvl >= skl) {
	  if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(L%3d\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(L%3d\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
		line = 1;
	  }

    }
  }
  while (skclass < 6) {
  line = 0; 
  send_to_char(ch, "\r\n\r\n[ ::::::::::: %s%-17.17s (max %3d%%)\tn :::::::::::: ]\r\n", class_colors[skclass], skill_area[skclass], prac_params[skclass][(int)GET_CLASS(ch)]);  
  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {    
	i = spell_sort_info[sortpos];
	skl = spell_info[i].min_level[skclass];
    if (lvl >= skl) {
      if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(L%3d\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(L%3d\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
		line = 1;
	  }

    }
	sortpos++;
	i = spell_sort_info[sortpos];
	skl = spell_info[i].min_level[skclass];
	if (lvl >= skl) {
	  if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(L%3d\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(L%3d\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, skl);
		line = 1;
	  }
    }
  }
  skclass++;
  }
  send_to_char(ch, "\r\n");
  return;
}

void list_skills(struct char_data *ch)
{
//  const char *overflow = "\r\n**OVERFLOW**\r\n";
  int i, skclass = 0, sortpos, line = 0;
/*  size_t len = 0;
  char buf2[MAX_STRING_LENGTH];*/
  const char *skill_area[] = {	
	"Transmuter Skill",
	"Conjurer Skill",
	"Emitter Skill",
	"Enhancer Skills",
	"Manipulator Skill",
	"Specialist Skill",
	"Hunter Skills"
  };

//	len = snprintf(buf2, sizeof(buf2), "\r\n[ ::::::::::: \tc%-17.17s (max %3d%%)\tn :::::::::::: ]\r\n\r\n", skill_area[6], LEARNED5(ch));
  
  send_to_char(ch, "[ ::::::::::: %s%-17.17s (max %3d%%)\tn :::::::::::: ]\r\n", class_colors[6], skill_area[6], LEARNEDSKL(ch));
  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {    
	i = spell_sort_info[sortpos];
    if (GET_LEVEL(ch) >= spell_info[i].min_level[6]) {
      if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
		line = 1;
	  }

    }
	sortpos++;
	i = spell_sort_info[sortpos];
	if (GET_LEVEL(ch) >= spell_info[i].min_level[6]) {
	  if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
		line = 1;
	  }

    }
  }
  while (skclass < 5) {
  line = 0; 
  send_to_char(ch, "\r\n\r\n[ ::::::::::: %s%-17.17s (max %3d%%)\tn :::::::::::: ]\r\n", class_colors[skclass], skill_area[skclass], prac_params[skclass][(int)GET_CLASS(ch)]);  
  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {    
	i = spell_sort_info[sortpos];
    if (GET_LEVEL(ch) >= spell_info[i].min_level[skclass]) {
      if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
		line = 1;
	  }

    }
	sortpos++;
	i = spell_sort_info[sortpos];
	if (GET_LEVEL(ch) >= spell_info[i].min_level[skclass]) {
	  if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
		line = 1;
	  }

    }
  }
  skclass++;
  }  
  line = 0;
  send_to_char(ch, "\r\n\r\n[ ::::::::::: %s%-17.17s (max %3d%%)\tn :::::::::::: ]\r\n", class_colors[5], skill_area[5], LEARNED5(ch));    
  if ((GET_CLASS(ch) == 1 || GET_CLASS(ch) >= 4)){
    for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {    
	i = spell_sort_info[sortpos];
    if (GET_LEVEL(ch) >= spell_info[i].min_level[5]) {
      if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
		line = 1;
	  }

    }
	sortpos++;
	i = spell_sort_info[sortpos];
	if (GET_LEVEL(ch) >= spell_info[i].min_level[5]) {
	  if (line == 1) {	
        send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn)\r\n", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    send_to_char(ch, "%s%-20s \tn(\tC%3d%%\tn) | ", GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", spell_info[i].name, GET_SKILL(ch, i));
		line = 1;
	  }

    }
  }
	}
/*
  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {    
	i = spell_sort_info[sortpos];
    if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
      if (line == 1) {	
        ret = snprintf(buf2 + len, sizeof(buf2) - len, "\tc%-20s \tn(%s%3d%%\tn)\r\n", spell_info[i].name, GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    ret = snprintf(buf2 + len, sizeof(buf2) - len, "\tc%-20s \tn(%s%3d%%\tn) | ", spell_info[i].name, GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", GET_SKILL(ch, i));
		line = 1;
	  }

    }
	sortpos++;
	i = spell_sort_info[sortpos];
	if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
	  if (line == 1) {	
        ret = snprintf(buf2 + len, sizeof(buf2) - len, "\tc%-20s \tn(%s%3d%%\tn)\r\n", spell_info[i].name, GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", GET_SKILL(ch, i));
	    line = 0;
	  } else {
	    ret = snprintf(buf2 + len, sizeof(buf2) - len, "\tc%-20s \tn(%s%3d%%\tn) | ", spell_info[i].name, GET_SKILL(ch, i) <= 0 ? "\tD" : "\tc", GET_SKILL(ch, i));
		line = 1;
	  }

    }
  }
  if (len >= sizeof(buf2))
    strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow);
  
  page_string(ch->desc, buf2, TRUE);*/
  send_to_char(ch, "\r\n\tDUpgrade/Practice points remaining: \tG%d\tn\r\n", GET_PRACTICES(ch));
}

void list_trains(struct char_data *ch)
{
   int str = 2, dex = 2, con = 2, intl = 2, wis = 2, cha = 2;
   
   if (ch->real_abils.str >= 24)
	  str += 2;
    else if (ch->real_abils.str > 18)
	  str++;
    else if (ch->real_abils.str < 11)
      str--;
  
    if (ch->real_abils.dex >= 24)
	  dex += 2;
    else if (ch->real_abils.dex > 18)
	  dex++;
    else if (ch->real_abils.str < 11)
      dex--;
  
    if (ch->real_abils.con >= 24)
	  con += 2;
    else if (ch->real_abils.con > 18)
	  con++;
    else if (ch->real_abils.str < 11)
      con--;
  
    if (ch->real_abils.intel >= 24)
	  intl += 2;
    else if (ch->real_abils.intel > 18)
	  intl++;
    else if (ch->real_abils.str < 11)
      intl--;
    
	if (ch->real_abils.wis >= 24)
	  wis += 2;
    else if (ch->real_abils.wis > 18)
	  wis++;
    else if (ch->real_abils.str < 11)
      wis--;
  
    if (ch->real_abils.cha >= 24)
	  cha += 2;
    else if (ch->real_abils.cha > 18)
	  cha++;
    else if (ch->real_abils.str < 11)
      cha--;
  
   send_to_char(ch, "\r\n[ Real Attributes ]\r\n");   
   send_to_char(ch, " \tcSTR: \tG%2d\tn | \tcDEX: \tG%2d\tn\r\n", ch->real_abils.str, ch->real_abils.dex);   
   send_to_char(ch, " \tcINT: \tG%2d\tn | \tcWIS: \tG%2d\tn\r\n", ch->real_abils.intel, ch->real_abils.wis);   
   send_to_char(ch, " \tcCON: \tG%2d\tn | \tcCHA: \tG%2d\tn\r\n", ch->real_abils.con , ch->real_abils.cha);   
   send_to_char(ch, "\r\n[  Upgrade cost   ]\r\n");
   send_to_char(ch, " \tcNEN: \tG1\tn /|\\ \tcSTA: \tG1\tn\r\n");
   send_to_char(ch, " \tcSTR: \tG%d\tn \\|/ \tcDEX: \tG%d\tn\r\n", str, dex);   
   send_to_char(ch, " \tcINT: \tG%d\tn /|\\ \tcWIS: \tG%d\tn\r\n", intl, wis);   
   send_to_char(ch, " \tcCON: \tG%d\tn \\|/ \tcCHA: \tG%d\tn\r\n", con , cha);   
   send_to_char(ch, "[ ::::::::::::::: ]\r\n");   
   send_to_char(ch, " \tDUpgrade points: \tG%d\tn\r\n", GET_PRACTICES(ch));
}

SPECIAL(guild)
{
  int skill_num, percent, i, found = 0;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || (!CMD_IS("practice") && !CMD_IS("train") && !CMD_IS("upgrade")))
    return (FALSE);

  skip_spaces(&argument);
  
  if (CMD_IS("train") || CMD_IS("upgrade")) {
	
    if (!*argument) {
      list_trains(ch);
      return (TRUE);
	}
	
	if (GET_PRACTICES(ch) <= 0) {
      send_to_char(ch, "You do not seem to be able to train now.\r\n");
      return (TRUE);
    }
	
	found = 2;	
	
	
	
	if (is_abbrev(argument, "nen")) {
	  if (GET_MAX_HIT(ch) == 1000000000) {
		send_to_char(ch, "Your nen is already at its maximum!\r\n");
	    return (TRUE);
	  }
	  gain_exp(ch, (GET_MAX_HIT(ch) / 40));
	  GET_PRACTICES(ch)--;	  
	} else if (is_abbrev(argument, "stamina")) {	  
	  GET_PRACTICES(ch)--;
	  GET_MAX_MANA(ch) += 2;
	  sprintf(buf, "stamina");
	} else if (is_abbrev(argument, "strenght")) {      
	  if (ch->real_abils.str < 25) {
		if (ch->real_abils.str >= 24)
	      found += 2;
        else if (ch->real_abils.str > 18)
	      found++;	    
	    else if (ch->real_abils.str < 11)
		  found--;		
		GET_PRACTICES(ch) -= found;
		if (GET_PRACTICES(ch) < 0) {
		  GET_PRACTICES(ch) += found;
		  send_to_char(ch, "You don't have enough upgrade points.\r\n");
		  return (TRUE); 	
		}
		ch->real_abils.str += 1;
	    sprintf(buf, "strenght");
	  } else {
		send_to_char(ch, "Strenght already at maximum!\r\n");
		return (TRUE);
	  }
	} else if (is_abbrev(argument, "dexterity")) {
	  if (ch->real_abils.dex < 25) {
		if (ch->real_abils.dex >= 24)
	      found += 2;
        else if (ch->real_abils.dex > 18)
	      found++;
	    else if (ch->real_abils.dex < 11)
		  found--;
		GET_PRACTICES(ch) -= found;
		if (GET_PRACTICES(ch) < 0) {
		  GET_PRACTICES(ch) += found;
		  send_to_char(ch, "You don't have enough upgrade points.\r\n");
		  return (TRUE); 	
		}
		ch->real_abils.dex += 1;
		sprintf(buf, "dexterity");
	  } else {
		send_to_char(ch, "Dexterity already at maximum!\r\n");
		return (TRUE);
	  }
	} else if (is_abbrev(argument, "intelligence")) {
	  if (ch->real_abils.intel < 25) {
		if (ch->real_abils.intel >= 24)
		  found += 2;
		else if (ch->real_abils.intel > 18)
		  found++;
	    else if (ch->real_abils.intel < 11)
		  found--;
		GET_PRACTICES(ch) -= found;
		if (GET_PRACTICES(ch) < 0) {
		  GET_PRACTICES(ch) += found;
		  send_to_char(ch, "You don't have enough upgrade points.\r\n");
		  return (TRUE); 	
		}
		ch->real_abils.intel += 1;
		sprintf(buf, "intelligence");
	  } else {
		send_to_char(ch, "Intelligence already at maximum!\r\n");
		return (TRUE);
	  }
	} else if (is_abbrev(argument, "wisdom")) {
	  if (ch->real_abils.wis < 25) {
		if (ch->real_abils.wis >= 24)
		  found += 2;
		else if (ch->real_abils.wis > 18)
		  found++;
	    else if (ch->real_abils.wis < 11)
		  found--;
		GET_PRACTICES(ch) -= found;
		if (GET_PRACTICES(ch) < 0) {
		  GET_PRACTICES(ch) += found;
		  send_to_char(ch, "You don't have enough upgrade points.\r\n");
		  return (TRUE); 	
		}
		ch->real_abils.wis += 1;
		sprintf(buf, "wisdom");
	  } else {
		send_to_char(ch, "Wisdom already at maximum!\r\n");
		return (TRUE);
	  }
	} else if (is_abbrev(argument, "constituition")) {
	  if (ch->real_abils.con < 25) {
		if (ch->real_abils.con >= 24)
		  found += 2;
		else if (ch->real_abils.con > 18)
		  found++;
	    else if (ch->real_abils.con < 11)
		  found--;
		GET_PRACTICES(ch) -= found;
		if (GET_PRACTICES(ch) < 0) {
		  GET_PRACTICES(ch) += found;
		  send_to_char(ch, "You don't have enough upgrade points.\r\n");
		  return (TRUE); 	
		}
		ch->real_abils.con += 1;
		sprintf(buf, "constituition");
	  } else {
		send_to_char(ch, "Constituition already at maximum!\r\n");
		return (TRUE);
	  }
	} else if (is_abbrev(argument, "charisma")) {
	  if (ch->real_abils.cha < 25) {
		if (ch->real_abils.cha >= 24)
	      found += 2;
        else if (ch->real_abils.cha > 18)
	      found++; 
		else if (ch->real_abils.cha < 11)
		  found--;
		GET_PRACTICES(ch) -= found;
		if (GET_PRACTICES(ch) < 0) {
		  GET_PRACTICES(ch) += found;
		  send_to_char(ch, "You don't have enough upgrade points.\r\n");
		  return (TRUE); 	
		}
		ch->real_abils.cha += 1;
		sprintf(buf, "charisma");
	  } else {
		send_to_char(ch, "Charisma already at maximum!\r\n");
		return (TRUE);
	  }
	} else {
      send_to_char(ch, "Train what?\r\n");
	  return (TRUE);
	}	
	if (*buf && is_abbrev(argument, "stamina"))
	  send_to_char(ch, "\tDYour %s was increased by \tG2\tD!\tn\r\n", buf);
	else if (*buf && !is_abbrev(argument, "nen"))
	  send_to_char(ch, "\tDYour %s was increased by \tG1\tD!\tn\r\n", buf);
    affect_total(ch);
	save_char(ch);
    Crash_crashsave(ch);
	return (TRUE);	
	
  } else {

  if (!*argument) {
    list_skills(ch);
    return (TRUE);
  }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char(ch, "You do not seem to be able to practice now.\r\n");
    return (TRUE);
  }

  skill_num = find_skill_num(argument);  

  for (i = 0; i < NUM_CLASSES; i++) {	
	if ((skill_num <= NUM_VALID_SKILLS) && GET_LEVEL(ch) >= spell_info[skill_num].min_level[i])
      found++;
  }
  
  if (!found || skill_num <= 0) {
    send_to_char(ch, "You do not know of that %s.\r\n", SPLSKL(ch));
	if (GET_LEVEL(ch) == LVL_IMPL)
	  send_to_char(ch, "Found: %s | skill_num: %d.\r\n", found ? "TRUE" : "FALSE", skill_num ? skill_num : 0);
    return (TRUE);
    }    
  
  if (GET_SKILL(ch, skill_num) > 0) {
    send_to_char(ch, "You are already learned in that area.\r\n");
    return (TRUE);
  }
  
  if (LEARNED5(ch) == 0 && (GET_SKILL(ch, skill_num) == 31 || GET_SKILL(ch, skill_num) == 40)) {
    send_to_char(ch, "You can't learn Specialist %s.\r\n", SPLSKL(ch));
    return (TRUE);
  }
  
  send_to_char(ch, "\r\n\tDYou learned \tG%s\tD!\tn\r\n", spell_info[skill_num].name);
  GET_PRACTICES(ch)--;    
  
  //percent = ((GET_WIS(ch) * (prac_params[i][(int) GET_CLASS(ch)])) / 100);
  percent = GET_WIS(ch);
  if (LEARNED5(ch) == 1 && (skill_num == 31 || skill_num == 40))
    SET_SKILL(ch, skill_num, 1);
  else
    SET_SKILL(ch, skill_num, MIN(LEARNEDSKL(ch), percent));

  return (TRUE);
  }
}

SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  for (k = world[IN_ROOM(ch)].contents; k; k = world[IN_ROOM(ch)].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return (FALSE);

  do_drop(ch, argument, cmd, SCMD_DROP);

  for (k = world[IN_ROOM(ch)].contents; k; k = world[IN_ROOM(ch)].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    send_to_char(ch, "You are awarded for outstanding performance.\r\n");
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      increase_gold(ch, value);
  }
  return (TRUE);
}

SPECIAL(mayor)
{
  char actbuf[MAX_INPUT_LENGTH];

  const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
  const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path = NULL;
  static int path_index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      path_index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      path_index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return (FALSE);

  switch (path[path_index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[path_index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_UNLOCK);	/* strcpy: OK */
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_OPEN);	/* strcpy: OK */
    break;

  case 'C':
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_CLOSE);	/* strcpy: OK */
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_LOCK);	/* strcpy: OK */
    break;

  case '.':
    move = FALSE;
    break;

  }

  path_index++;
  return (FALSE);
}

/* General special procedures for mobiles. */

static void npc_steal(struct char_data *ch, struct char_data *victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;
  if (!CAN_SEE(ch, victim))
    return;

  if (AWAKE(victim) && (rand_number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some Jenny */
    gold = (GET_GOLD(victim) * rand_number(1, 10)) / 100;
    if (gold > 0) {
      increase_gold(ch, gold);
	  decrease_gold(victim, gold);
    }
  }
}

/* Quite lethal to low-level characters. */
SPECIAL(snake)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING || !FIGHTING(ch))
    return (FALSE);

  if (IN_ROOM(FIGHTING(ch)) != IN_ROOM(ch) || rand_number(0, GET_LEVEL(ch)) != 0)
    return (FALSE);

  act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
  act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
  call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
  return (TRUE);
}

SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd || GET_POS(ch) != POS_STANDING)
    return (FALSE);

  for (cons = world[IN_ROOM(ch)].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && GET_LEVEL(cons) < LVL_IMMORT && !rand_number(0, 4)) {
      npc_steal(ch, cons);
      return (TRUE);
    }

  return (FALSE);
}

SPECIAL(magic_user)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (FALSE);

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !rand_number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
    vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
    return (TRUE);

  if (GET_LEVEL(ch) > 13 && rand_number(0, 10) == 0)
    cast_spell(ch, vict, NULL, SPELL_POISON);

  if (GET_LEVEL(ch) > 7 && rand_number(0, 8) == 0)
    cast_spell(ch, vict, NULL, SPELL_BLINDNESS);

  if (GET_LEVEL(ch) > 12 && rand_number(0, 12) == 0) {
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_GOOD);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }

  if (rand_number(0, 4))
    return (TRUE);

  switch (GET_LEVEL(ch)) {
    case 4:
    case 5:
      cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
      break;
    case 6:
    case 7:
      cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
      break;
    case 8:
    case 9:
      cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
      break;
    case 10:
    case 11:
      cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
      break;
    case 12:
    case 13:
      cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
      break;
    case 14:
    case 15:
    case 16:
    case 17:
      cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
      break;
    default:
      cast_spell(ch, vict, NULL, SPELL_FIREBALL);
      break;
  }
  return (TRUE);
}

/* Special procedures for mobiles. */
SPECIAL(guild_guard) 
{ 
  int i, direction; 
  struct char_data *guard = (struct char_data *)me; 
  const char *buf = "The guard humiliates you, and blocks your way.\r\n"; 
  const char *buf2 = "The guard humiliates $n, and blocks $s way."; 

  if (!IS_MOVE(cmd) || AFF_FLAGGED(guard, AFF_BLIND)) 
    return (FALSE); 
     
  if (GET_LEVEL(ch) >= LVL_IMMORT) 
    return (FALSE); 
   
  /* find out what direction they are trying to go */ 
  for (direction = 0; direction < NUM_OF_DIRS; direction++)
    if (!strcmp(cmd_info[cmd].command, dirs[direction]))
      for (direction = 0; direction < DIR_COUNT; direction++)
		if (!strcmp(cmd_info[cmd].command, dirs[direction]) ||
			!strcmp(cmd_info[cmd].command, autoexits[direction]))
	      break; 

  for (i = 0; guild_info[i].guild_room != NOWHERE; i++) { 
    /* Wrong guild. */ 
    if (GET_ROOM_VNUM(IN_ROOM(ch)) != guild_info[i].guild_room) 
      continue; 

    /* Wrong direction. */ 
    if (direction != guild_info[i].direction) 
      continue; 

    /* Allow the people of the guild through. */ 
    if (!IS_NPC(ch) && GET_CLASS(ch) == guild_info[i].pc_class) 
      continue; 

    send_to_char(ch, "%s", buf); 
    act(buf2, FALSE, ch, 0, 0, TO_ROOM); 
    return (TRUE); 
  } 
  return (FALSE); 
} 

SPECIAL(puff)
{
  char actbuf[MAX_INPUT_LENGTH];

  if (cmd)
    return (FALSE);

  switch (rand_number(0, 60)) {
    case 0:
      do_say(ch, strcpy(actbuf, "My god!  It's full of stars!"), 0, 0);	/* strcpy: OK */
      return (TRUE);
    case 1:
      do_say(ch, strcpy(actbuf, "How'd all those fish get up here?"), 0, 0);	/* strcpy: OK */
      return (TRUE);
    case 2:
      do_say(ch, strcpy(actbuf, "I'm a very female dragon."), 0, 0);	/* strcpy: OK */
      return (TRUE);
    case 3:
      do_say(ch, strcpy(actbuf, "I've got a peaceful, easy feeling."), 0, 0);	/* strcpy: OK */
      return (TRUE);
    default:
      return (FALSE);
  }
}

SPECIAL(fido)
{
  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content) {
    if (!IS_CORPSE(i))
      continue;

    act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
    for (temp = i->contains; temp; temp = next_obj) {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_room(temp, IN_ROOM(ch));
    }
    extract_obj(i);
    return (TRUE);
  }
  return (FALSE);
}

SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return (TRUE);
  }
  return (FALSE);
}

SPECIAL(cityguard)
{
  struct char_data *tch, *evil, *spittle;
  int max_evil, min_cha;

  if (cmd || !AWAKE(ch) || FIGHTING(ch))
    return (FALSE);

  max_evil = 1000;
  min_cha = 6;
  spittle = evil = NULL;

  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
    if (!CAN_SEE(ch, tch))
      continue;
    if (!IS_NPC(tch) && PLR_FLAGGED(tch, PLR_KILLER)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }

    if (!IS_NPC(tch) && PLR_FLAGGED(tch, PLR_THIEF)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }

    if (FIGHTING(tch) && GET_ALIGNMENT(tch) < max_evil && (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
      max_evil = GET_ALIGNMENT(tch);
      evil = tch;
    }

    if (GET_CHA(tch) < min_cha) {
      spittle = tch;
      min_cha = GET_CHA(tch);
    }
  }

  if (evil && GET_ALIGNMENT(FIGHTING(evil)) >= 0) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }

  /* Reward the socially inept. */
  if (spittle && !rand_number(0, 9)) {
    static int spit_social;

    if (!spit_social)
      spit_social = find_command("spit");

    if (spit_social > 0) {
      char spitbuf[MAX_NAME_LENGTH + 1];
      strncpy(spitbuf, GET_NAME(spittle), sizeof(spitbuf));	/* strncpy: OK */
      spitbuf[sizeof(spitbuf) - 1] = '\0';
      do_action(ch, spitbuf, spit_social, 0);
      return (TRUE);
    }
  }
  return (FALSE);
}

#define PET_PRICE(pet) (GET_LEVEL(pet) * 300)
SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  room_rnum pet_room;
  struct char_data *pet;

  /* Gross. */
  pet_room = IN_ROOM(ch) + 1;

  if (CMD_IS("list")) {
    send_to_char(ch, "Available pets are:\r\n");
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      /* No, you can't have the Implementor as a pet if he's in there. */
      if (!IS_NPC(pet))
        continue;
      send_to_char(ch, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    two_arguments(argument, buf, pet_name);

    if (!(pet = get_char_room(buf, NULL, pet_room)) || !IS_NPC(pet)) {
      send_to_char(ch, "There is no such pet!\r\n");
      return (TRUE);
    }
    if (GET_GOLD(ch) < PET_PRICE(pet)) {
      send_to_char(ch, "You don't have enough gold!\r\n");
      return (TRUE);
    }
    decrease_gold(ch, PET_PRICE(pet));

    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT_AR(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      snprintf(buf, sizeof(buf), "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = strdup(buf);

      snprintf(buf, sizeof(buf), "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = strdup(buf);
    }
    char_to_room(pet, IN_ROOM(ch));
    add_follower(pet, ch);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char(ch, "May you enjoy your pet.\r\n");
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return (TRUE);
  }

  /* All commands except list and buy */
  return (FALSE);
}

/* Special procedures for objects. */
SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      send_to_char(ch, "Your current balance is %d coins.\r\n", GET_BANK_GOLD(ch));
    else
      send_to_char(ch, "You currently have no money deposited.\r\n");
    return (TRUE);
  } else if (CMD_IS("deposit")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char(ch, "How much do you want to deposit?\r\n");
      return (TRUE);
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char(ch, "You don't have that many coins!\r\n");
      return (TRUE);
    }
    decrease_gold(ch, amount);
	increase_bank(ch, amount);
    send_to_char(ch, "You deposit %d coins.\r\n", amount);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return (TRUE);
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char(ch, "How much do you want to withdraw?\r\n");
      return (TRUE);
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char(ch, "You don't have that many coins deposited!\r\n");
      return (TRUE);
    }
    increase_gold(ch, amount);
	decrease_bank(ch, amount);
    send_to_char(ch, "You withdraw %d coins.\r\n", amount);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return (TRUE);
  } else
    return (FALSE);
}


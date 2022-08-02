/**************************************************************************
*  File: act.item.c                                        Part of tbaMUD *
*  Usage: Object handling routines -- get/drop and container handling.    *
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
#include "constants.h"
#include "dg_scripts.h"
#include "oasis.h"
#include "act.h"
#include "quest.h"
#include "screen.h"

/* local function prototypes */
/* do_get utility functions */
static int can_take_obj(struct char_data *ch, struct obj_data *obj);
static void get_check_money(struct char_data *ch, struct obj_data *obj);
static void get_from_container(struct char_data *ch, struct obj_data *cont, char *arg, int mode, int amount);
static void get_from_room(struct char_data *ch, char *arg, int amount);
static bool has_luck(struct char_data *ch);
int perform_unpack(struct char_data *ch, struct obj_data *obj);
static void perform_get_from_container(struct char_data *ch, struct obj_data *obj, struct obj_data *cont, int mode);
static int perform_get_from_room(struct char_data *ch, struct obj_data *obj);
/* do_give utility functions */
static struct char_data *give_find_vict(struct char_data *ch, char *arg);
static void perform_give(struct char_data *ch, struct char_data *vict, struct obj_data *obj);
static void perform_give_gold(struct char_data *ch, struct char_data *vict, int amount);
/* do_drop utility functions */
static int perform_drop(struct char_data *ch, struct obj_data *obj, byte mode, const char *sname, room_rnum RDR);
static void perform_drop_gold(struct char_data *ch, int amount, byte mode, room_rnum RDR);
/* do_put utility functions */
static void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont);
/* do_remove utility functions */
static void perform_remove(struct char_data *ch, int pos);
/* do_wear utility functions */
static void perform_wear(struct char_data *ch, struct obj_data *obj, int where);
static void wear_message(struct char_data *ch, struct obj_data *obj, int where);
static void fly_to_char (struct char_data *ch, struct char_data *target);
static void fly_to_room (struct char_data *ch, room_rnum target);


static void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont)
{
  struct obj_data *next_obj;
  int limit = 0, busy = FALSE;

  if (!drop_otrigger(obj, ch))
    return;

  if (!obj) /* object might be extracted by drop_otrigger */
    return;
	
  for (next_obj = cont->contains; next_obj; next_obj = next_obj->next_content){
    if (GET_OBJ_VNUM(cont) == 3203) {
	  if (GET_OBJ_TYPE(next_obj) == ITEM_CARD || GET_OBJ_TYPE(next_obj) == ITEM_SPELLCARD)
	    limit++;
//      else if (GET_OBJ_TYPE(next_obj) == ITEM_SPELLCARD)
//	    limitp++;
      else {
	    if (GET_OBJ_VNUM(next_obj) == GET_OBJ_VNUM(obj))
		  busy = TRUE;
	  }
	}
  }  
  
    if (busy == TRUE)
      act("The specified slot for $p is already occupied into your $P.", FALSE, ch, obj, cont, TO_CHAR);	  
    else if ((GET_OBJ_TYPE(obj) == ITEM_CARD || GET_OBJ_TYPE(obj) == ITEM_SPELLCARD) && (limit > 44))
	  act("Has no free slot for your $p fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
//    else if ((GET_OBJ_TYPE(obj) == ITEM_SPELLCARD) && (limitp > 39))
//	  act("Has no free slot for your $p fit in $P.", FALSE, ch, obj, cont, TO_CHAR);    
    else if (((GET_OBJ_VNUM(cont) == 3203 && !IS_CARD(obj)) || (GET_OBJ_VNUM(cont) != 3203 && IS_CARD(obj))) || (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj)) > GET_OBJ_VAL(cont, 0))
      act("$p won't fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
    else if ((GET_OBJ_VNUM(cont) == 3203 && OBJ_FLAGGED(obj, ITEM_NODROP)) && IN_ROOM(cont) != NOWHERE)
      act("The $p seems to be rejected when you insert it inside the $P.", FALSE, ch, obj, cont, TO_CHAR);    
	else {
	  if (GET_OBJ_VNUM(cont) == 3203)
		GET_OBJ_TIMER(obj) = 62;
      obj_from_char(obj);
      obj_to_obj(obj, cont);

      act("$n puts $p in $P.", TRUE, ch, obj, cont, TO_ROOM);

      /* Yes, I realize this is strange until we have auto-equip on rent. -gg */
      if (OBJ_FLAGGED(obj, ITEM_NODROP) && !OBJ_FLAGGED(cont, ITEM_NODROP)) {
        SET_BIT_AR(GET_OBJ_EXTRA(cont), ITEM_NODROP);
        act("You get a strange feeling as you put $p in $P.", FALSE,
                ch, obj, cont, TO_CHAR);
      } else
        act("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
    }
}

/* The following put modes are supported:
     1) put <object> <container>
     2) put all.<object> <container>
     3) put all <container>
   The <container> must be in inventory or on ground. All objects to be put
   into container must be in inventory. */
ACMD(do_put)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj, *cont;
  struct char_data *tmp_char;
  int obj_dotmode, cont_dotmode, found = 0, howmany = 1;
  char *theobj, *thecont;

  one_argument(two_arguments(argument, arg1, arg2), arg3);	/* three_arguments */

  if (*arg3 && is_number(arg1)) {
    howmany = atoi(arg1);
    theobj = arg2;
    thecont = arg3;
  } else {
    theobj = arg1;
    thecont = arg2;
  }
  obj_dotmode = find_all_dots(theobj);
  cont_dotmode = find_all_dots(thecont);

  if (!*theobj)
    send_to_char(ch, "Put what in what?\r\n");
  else if (cont_dotmode != FIND_INDIV)
    send_to_char(ch, "You can only put things into one container at a time.\r\n");
  else if (!*thecont) {
    send_to_char(ch, "What do you want to put %s in?\r\n", obj_dotmode == FIND_INDIV ? "it" : "them");
  } else {
    generic_find(thecont, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
    if (!cont)
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(thecont), thecont);
    else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
      act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
    else if (OBJVAL_FLAGGED(cont, CONT_CLOSED) && (GET_LEVEL(ch) < LVL_IMMORT || !PRF_FLAGGED(ch, PRF_NOHASSLE)))
      send_to_char(ch, "You'd better open it first!\r\n");
    else {
      if (obj_dotmode == FIND_INDIV) {	/* put <obj> <container> */
	if (!(obj = get_obj_in_list_vis(ch, theobj, NULL, ch->carrying)))
	  send_to_char(ch, "You aren't carrying %s %s.\r\n", AN(theobj), theobj);
	else if (obj == cont && howmany == 1)
	  send_to_char(ch, "You attempt to fold it into itself, but fail.\r\n");
	else {
	  while (obj && howmany) {
	    next_obj = obj->next_content;
            if (obj != cont) {
              howmany--;
	      perform_put(ch, obj, cont);
            }
	    obj = get_obj_in_list_vis(ch, theobj, NULL, next_obj);
	  }
	}
      } else {
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
	      (obj_dotmode == FIND_ALL || isname(theobj, obj->name))) {
	    found = 1;
	    perform_put(ch, obj, cont);
	  }
	}
	if (!found) {
	  if (obj_dotmode == FIND_ALL)
	    send_to_char(ch, "You don't seem to have anything to put in it.\r\n");
	  else
	    send_to_char(ch, "You don't seem to have any %ss.\r\n", theobj);
	}
      }
    }
  }
}

ACMD(do_compare)
{
  struct obj_data *obj1, *obj2;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  char *item1, *item2;
  int i;
  bool same = FALSE;
  
  two_arguments(argument, buf, buf2);
  
  item1 = buf;
  item2 = buf2;
  
  if (!*item1)
	send_to_char(ch, "Compare what with what?\r\n");
  else if (!*item2)
	send_to_char(ch, "Compare %s with what?\r\n", item1);
  else {
	obj1 = get_obj_in_list_vis(ch, item1, NULL, ch->carrying);
    obj2 = get_obj_in_list_vis(ch, item2, NULL, ch->carrying);
	if (!obj1)
	  send_to_char(ch, "You aren't carrying %s %s.\r\n", AN(item1), item1);	
	else if (!obj2)
      send_to_char(ch, "You aren't carrying %s %s.\r\n", AN(item2), item2);
    else if (obj1 == obj2)
	  send_to_char(ch, "\"Never be afraid to mislabel a product.\" - Rule of Aquisition Number 239.\r\n");
    else if (GET_OBJ_TYPE(obj1) != GET_OBJ_TYPE(obj2))
	  send_to_char(ch, "You only can compare objects of the same type.\r\n");
    else {
	  switch (GET_OBJ_TYPE(obj1)) {
	  case ITEM_WEAPON:	  
		if ((((GET_OBJ_VAL(obj1, 2) + 1) / 2.0) * GET_OBJ_VAL(obj1, 1)) > (((GET_OBJ_VAL(obj2, 2) + 1) / 2.0) * GET_OBJ_VAL(obj2, 1)))
		  send_to_char(ch, "It seems %s is better than %s.\r\n", obj1->short_description, obj2->short_description);
	    else if ((((GET_OBJ_VAL(obj1, 2) + 1) / 2.0) * GET_OBJ_VAL(obj1, 1)) < (((GET_OBJ_VAL(obj2, 2) + 1) / 2.0) * GET_OBJ_VAL(obj2, 1)))
		  send_to_char(ch, "It seems %s is better than %s.\r\n", obj2->short_description, obj1->short_description);
	    else {
		  send_to_char(ch, "It seems %s is equal than %s.", obj1->short_description, obj2->short_description);
		  if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
            send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj1->short_description, obj2->short_description);
	      else if (GET_OBJ_COST(obj1) < GET_OBJ_COST(obj2))
		    send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj2->short_description, obj1->short_description);
	      else
		    send_to_char(ch, "\r\n");		    
		}
	  break;
	  case ITEM_ARMOR:
	    for (i = 0; i < 13; i++)
		  if (CAN_WEAR(obj1, i) == CAN_WEAR(obj2, i))
			same = TRUE;
	  case ITEM_FOOD:
	  case ITEM_CONTAINER:
	  case ITEM_DRINKCON:
	    if (same || GET_OBJ_TYPE(obj1) != ITEM_ARMOR) {
		  if (GET_OBJ_VAL(obj1, 0) > GET_OBJ_VAL(obj2, 0))
		    send_to_char(ch, "It seems %s is better than %s.\r\n", obj1->short_description, obj2->short_description);
	      else if (GET_OBJ_VAL(obj1, 0) < GET_OBJ_VAL(obj2, 0))
		    send_to_char(ch, "It seems %s is better than %s.\r\n", obj2->short_description, obj1->short_description);
	      else {
		    send_to_char(ch, "It seems %s is equal than %s.", obj1->short_description, obj2->short_description);
		    if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
              send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj1->short_description, obj2->short_description);
	        else if (GET_OBJ_COST(obj1) < GET_OBJ_COST(obj2))
		      send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj2->short_description, obj1->short_description);
	        else
		      send_to_char(ch, "\r\n");		    
		  } 
		} else
		    send_to_char(ch, "It's like comparing apples and oranges...\r\n");
	  break;
	  case ITEM_LIGHT:
	    if (GET_OBJ_VAL(obj1, 2) > GET_OBJ_VAL(obj2, 2) || GET_OBJ_VAL(obj1, 2) == -1)
		  send_to_char(ch, "It seems %s is better than %s.\r\n", obj1->short_description, obj2->short_description);
		else if (GET_OBJ_VAL(obj1, 2) < GET_OBJ_VAL(obj2, 2) || GET_OBJ_VAL(obj2, 2) == -1)
		  send_to_char(ch, "It seems %s is better than %s.\r\n", obj2->short_description, obj1->short_description);
		else {
		  send_to_char(ch, "It seems %s is equal than %s.", obj1->short_description, obj2->short_description);
		  if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
            send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj1->short_description, obj2->short_description);
	      else if (GET_OBJ_COST(obj1) < GET_OBJ_COST(obj2))
		    send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj2->short_description, obj1->short_description);
	      else
		    send_to_char(ch, "\r\n");		    
		}
      break;
      case ITEM_BOAT:
        if (GET_OBJ_WEIGHT(obj1) > GET_OBJ_WEIGHT(obj2))
          send_to_char(ch, "It seems %s is lighter than %s.\r\n", obj1->short_description, obj2->short_description);
	    else if (GET_OBJ_WEIGHT(obj1) < GET_OBJ_WEIGHT(obj2))
		  send_to_char(ch, "It seems %s is lighter than %s.\r\n", obj2->short_description, obj1->short_description);
	    else {
		  send_to_char(ch, "It seems %s is equal than %s.", obj1->short_description, obj2->short_description);
		  if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
            send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj1->short_description, obj2->short_description);
	      else if (GET_OBJ_COST(obj1) < GET_OBJ_COST(obj2))
		    send_to_char(ch, ".. but %s seems more valuable than %s.\r\n", obj2->short_description, obj1->short_description);
	      else
		    send_to_char(ch, "\r\n");		    
		}
	  break;	  
	  case ITEM_TRASH:
      case ITEM_NOTE:
	  case ITEM_KEY:
	  case ITEM_PEN:
	  case ITEM_WORN:
      case ITEM_FURNITURE:
	  case ITEM_FOUNTAIN:
	    send_to_char(ch, "\"Dignity and an empty sack is worth the sack.\" - Rule of Aquisition Number 109.\r\n");
	  break;
	  case ITEM_CARD:
	  case ITEM_SPELLCARD:
	  case ITEM_RESTRICTED:
	    if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
          send_to_char(ch, "It seems %s is more valuable than %s.\r\n", obj1->short_description, obj2->short_description);
	    else if (GET_OBJ_COST(obj1) < GET_OBJ_COST(obj2))
		  send_to_char(ch, "It seems %s is more valuable than %s.\r\n", obj2->short_description, obj1->short_description);
	    else
		  send_to_char(ch, "It seems %s has the same valuable than %s.\r\n", obj1->short_description, obj2->short_description);
	  break;
      default:
	    if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
          send_to_char(ch, "It is hard to know which one is better... but %s seems more valuable than %s.\r\n", obj1->short_description, obj2->short_description);
	    else if (GET_OBJ_COST(obj1) < GET_OBJ_COST(obj2))
		  send_to_char(ch, "It is hard to know which one is better... but %s seems more valuable than %s.\r\n", obj2->short_description, obj1->short_description);
	    else
		  send_to_char(ch, "It is hard to know which one is better...\r\n");
	  break;
	  }
	}
  }
}

static int can_take_obj(struct char_data *ch, struct obj_data *obj)
{
if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE)) && (IS_NPC(ch) || !PRF_FLAGGED(ch, PRF_NOHASSLE))) {
  act("$p: you can't take that!", FALSE, ch, obj, 0, TO_CHAR);
  return (0);
  }

if (!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
    act("$p: you can't carry that many items.", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  } else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    act("$p: you can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }
}
  
  if (OBJ_SAT_IN_BY(obj)){
    act("It appears someone is sitting on $p..", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }
  
  return (1);
}

static void get_check_money(struct char_data *ch, struct obj_data *obj)
{
  int value = GET_OBJ_VAL(obj, 0);

  if (GET_OBJ_TYPE(obj) != ITEM_MONEY || value <= 0)
    return;

  extract_obj(obj);

  increase_gold(ch, value);

  if (value == 1)
    send_to_char(ch, "There was 1 coin of Jenny.\r\n");
  else
    send_to_char(ch, "There were %d Jenny.\r\n", value);
}

static void perform_get_from_container(struct char_data *ch, struct obj_data *obj,
				     struct obj_data *cont, int mode)
{
  if (mode == FIND_OBJ_INV || can_take_obj(ch, obj)) {
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      act("$p: you can't hold any more items.", FALSE, ch, obj, 0, TO_CHAR);
    else if (get_otrigger(obj, ch)) {
      obj_from_obj(obj);
      obj_to_char(obj, ch);
      act("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
      act("$n gets $p from $P.", TRUE, ch, obj, cont, TO_ROOM);
      get_check_money(ch, obj);
	  if (GET_OBJ_VNUM(cont) == 3203)
		GET_OBJ_TIMER(obj) = 62;	  
	  else if (obj && !IS_NPC(ch) && !IS_CARD(obj))
	    make_card(ch, obj, TRUE);      
    }
  }
}

void get_from_container(struct char_data *ch, struct obj_data *cont,
			     char *arg, int mode, int howmany)
{
  struct obj_data *obj, *next_obj;
  int obj_dotmode, found = 0;

  obj_dotmode = find_all_dots(arg);

  if (OBJVAL_FLAGGED(cont, CONT_CLOSED) && (GET_LEVEL(ch) < LVL_IMMORT || !PRF_FLAGGED(ch, PRF_NOHASSLE)))
    act("$p is closed.", FALSE, ch, cont, 0, TO_CHAR);
  else if (obj_dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, NULL, cont->contains))) {
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "There doesn't seem to be %s %s in $p.", AN(arg), arg);
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
    } else {
      struct obj_data *obj_next;
      while (obj && howmany--) {
        obj_next = obj->next_content;
        perform_get_from_container(ch, obj, cont, mode);
        obj = get_obj_in_list_vis(ch, arg, NULL, obj_next);
      }
    }
  } else {
    if (obj_dotmode == FIND_ALLDOT && !*arg) {
      send_to_char(ch, "Get all of what?\r\n");
      return;
    }
    for (obj = cont->contains; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (obj_dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;
	perform_get_from_container(ch, obj, cont, mode);
      }
    }
    if (!found) {
      if (obj_dotmode == FIND_ALL)
	act("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
      else {
        char buf[MAX_STRING_LENGTH];

	snprintf(buf, sizeof(buf), "You can't seem to find any %ss in $p.", arg);
	act(buf, FALSE, ch, cont, 0, TO_CHAR);
      }
    }
  }
}

static int perform_get_from_room(struct char_data *ch, struct obj_data *obj)
{
  if (can_take_obj(ch, obj) && get_otrigger(obj, ch)) {
    obj_from_room(obj);
    obj_to_char(obj, ch);
    act("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
    get_check_money(ch, obj);
	if (obj && !IS_NPC(ch) && !IS_CARD(obj) && GET_OBJ_VNUM(obj) != 65535)
	    make_card(ch, obj, TRUE);
    return (1);
  }
  return (0);
}

static void get_from_room(struct char_data *ch, char *arg, int howmany)
{
  struct obj_data *obj, *next_obj;
  int dotmode, found = 0;

  dotmode = find_all_dots(arg);

  if (dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents)))
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(arg), arg);
    else {
      struct obj_data *obj_next;
      while(obj && howmany--) {
	obj_next = obj->next_content;
	    if (GET_OBJ_VNUM(obj) != 3203 || GET_LEVEL(ch) >= LVL_IMMORT){
          perform_get_from_room(ch, obj);
          obj = get_obj_in_list_vis(ch, arg, NULL, obj_next);
		} else
			send_to_char(ch, "You don't see %s %s here.\r\n", AN(arg), arg);
      }
    }
  } else {
    if (dotmode == FIND_ALLDOT && !*arg) {
      send_to_char(ch, "Get all of what?\r\n");
      return;
    }
    for (obj = world[IN_ROOM(ch)].contents; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if ((CAN_SEE_OBJ(ch, obj) &&
	  (dotmode == FIND_ALL || isname(arg, obj->name))) && (GET_OBJ_VNUM(obj) != 3203 || GET_LEVEL(ch) >= LVL_IMMORT)) {
	found = 1;
	perform_get_from_room(ch, obj);
      }
    }
    if (!found) {
      if (dotmode == FIND_ALL)
	send_to_char(ch, "There doesn't seem to be anything here.\r\n");
      else
	send_to_char(ch, "You don't see any %ss here.\r\n", arg);
    }
  }
}

ACMD(do_get)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];

  int cont_dotmode, found = 0, mode;
  struct obj_data *cont;
  struct char_data *tmp_char;

  one_argument(two_arguments(argument, arg1, arg2), arg3);	/* three_arguments */

  if (!*arg1)
    send_to_char(ch, "Get what?\r\n");
  else if (!*arg2)
    get_from_room(ch, arg1, 1);
  else if (is_number(arg1) && !*arg3)
    get_from_room(ch, arg2, atoi(arg1));
  else {
    int amount = 1;
    if (is_number(arg1)) {
      amount = atoi(arg1);
      strcpy(arg1, arg2); /* strcpy: OK (sizeof: arg1 == arg2) */
      strcpy(arg2, arg3); /* strcpy: OK (sizeof: arg2 == arg3) */
    }
    cont_dotmode = find_all_dots(arg2);
    if (cont_dotmode == FIND_INDIV) {
      mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
      if (!cont)
	send_to_char(ch, "You don't have %s %s.\r\n", AN(arg2), arg2);
      else if (GET_OBJ_VNUM(cont) == 3203 && !PLR_FLAGGED(ch, PLR_BOOK))
	send_to_char(ch, "You don't have %s %s.\r\n", AN(arg2), arg2);	
      else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
	act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);      
      else
	get_from_container(ch, cont, arg1, mode, amount);
    } else {
      if (cont_dotmode == FIND_ALLDOT && !*arg2) {
	send_to_char(ch, "Get from all of what?\r\n");
	return;
      }
      for (cont = ch->carrying; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
		if (GET_OBJ_VNUM(cont) != 3203 || (GET_OBJ_VNUM(cont) == 3203 && PLR_FLAGGED(ch, PLR_BOOK))) {
	      found = 1;
	      get_from_container(ch, cont, arg1, FIND_OBJ_INV, amount);
		}
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    found = 1;
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	  }
	}
      for (cont = world[IN_ROOM(ch)].contents; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    if (GET_OBJ_VNUM(cont) != 3203 || GET_LEVEL(ch) >= LVL_IMMORT) {
	      get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, amount);
	      found = 1;
		} 
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	    found = 1;
	  }
	}
      if (!found) {
	if (cont_dotmode == FIND_ALL)
	  send_to_char(ch, "You can't seem to find any containers.\r\n");
	else
	  send_to_char(ch, "You can't seem to find any %ss here.\r\n", arg2);
      }
    }
  }
}

static bool has_luck(struct char_data *ch)
{
	if (!ch)
	  return (FALSE);
	
	if (AFF_FLAGGED(ch, AFF_LUCK)) {
	  affect_from_char(ch, SPELL_LUCK);
	  send_to_char(ch, "You feel less lucky.\r\n");
	  return (TRUE);
	}
	return (FALSE);
}

int perform_unpack(struct char_data *ch, struct obj_data *obj)
{
  struct obj_data *cardr, *cardp, *card;
  int chance, check = 0, qnt = 1;
  bool lck = FALSE;
  obj_rnum crnum;
  obj_vnum cvnum;

  if (IS_NPC(ch) || GET_OBJ_TYPE(obj) != ITEM_BOOSTER)
    return (0);

  lck = has_luck(ch);

  if (GET_OBJ_VNUM(obj) == 3251) {
	qnt = 3;	
	while (qnt) {
	  cvnum = (1000 + rand_number(1, 40));
	  if ((card = read_object(cvnum, VIRTUAL)) != NULL) {
		if (obj_index[GET_OBJ_RNUM(card)].number > GET_OBJ_RENT(card))
		  extract_obj(card);		  
	    else {
		  obj_to_char(card, ch);
          send_to_char(ch, "You unpacked a %s!\r\n", card->short_description);
		  qnt--;
		}
	  }
	} 
	return (1);
  }
  /* 5% chance drop a random restricted card, miss = normal drop */
  chance = rand_number(1, 20);
  if (lck)
	chance = 20;
  if (chance == 20) {
	for ( ; ; ) {
	  cvnum = (65300 + rand_number(3, 99));
	  if (!(cardr = read_object(cvnum, VIRTUAL)))
		continue;	  
	  else if ((obj_index[GET_OBJ_RNUM(cardr)].number > GET_OBJ_RENT(cardr)) || (lck && GET_OBJ_COST(cardr) > 100000)) {
		extract_obj(cardr);
		continue;
	  } else
		break;
	}
    obj_to_char(cardr, ch);
    send_to_char(ch, "You unpacked a %s!!!\r\n", cardr->short_description);	
  } else
	qnt++; 

  /* 10% chance drop a random spell card, miss = normal drop */
  chance = rand_number(1, 10);
  if (lck)
	chance = 10;
  if (chance == 10) {
    for ( ; ; ) {
	  cvnum = (1000 + rand_number(1, 40));
	  if (!(cardp = read_object(cvnum, VIRTUAL)))
		continue;
	  else if (obj_index[GET_OBJ_RNUM(cardp)].number > GET_OBJ_RENT(cardp)) {
		extract_obj(cardp);
		continue;
	  } else
		break;
	}
    obj_to_char(cardp, ch);
    send_to_char(ch, "You unpacked a %s!\r\n", cardp->short_description);
  } else
	qnt++;

  /* Gives between 1 and 3 normal cards */
  while (qnt) {	
    crnum = rand_number(1, top_of_objt);
	if ((card = read_object(crnum, REAL)) != NULL) {
	  if ((CAN_WEAR(card, ITEM_WEAR_TAKE)) && !(GET_OBJ_TYPE(card) == ITEM_BOOSTER  && IS_CARD(card)) &&
	      !OBJ_FLAGGED(card, ITEM_NOGAIN) && (GET_OBJ_VNUM(card) > 100 && GET_OBJ_VNUM(card) < 65300)) {		
		obj_to_char(card, ch);
		check = make_card(ch, card, FALSE);
		if (!check) {
		  extract_obj(card);
		  continue;
		}
	    send_to_char(ch, "You unpacked a %s.\r\n", ch->carrying->short_description);	
	    qnt--;
	  } else extract_obj(card);
    }
  }
  return (1);
}

ACMD(do_unpack)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  int dotmode;
  
  if (IS_NPC(ch))
	return;
  
  argument = one_argument(argument, arg);
  
  if (!*arg) {
    send_to_char(ch, "Unpack what?\r\n\tcSyntax: unpack \tYitem\tn\r\n");
	return;
  } else {
   dotmode = find_all_dots(arg);    
   if (dotmode == FIND_ALL) {    
	  send_to_char(ch, "Where is the pleasure of unpack all at same time?\r\n");	    
  } else if (dotmode == FIND_ALLDOT) { 
	  send_to_char(ch, "Where is the pleasure of unpack all at same time?\r\n");
    } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      else {		
		if (GET_OBJ_TYPE(obj) == ITEM_BOOSTER) {		  
		  act("$n eagerly start opening $p.", TRUE, ch, obj, 0, TO_ROOM);
	      send_to_char(ch, "You eagerly start opening %s...\r\n", obj->short_description);
		  perform_unpack(ch, obj);
		  extract_obj(obj);
		  save_char(ch);
          Crash_crashsave(ch);
	    } else
	      send_to_char(ch, "Baka! %s is not a booster pack.\r\n", obj->short_description);
	  }
    }
  }
}

static void perform_drop_gold(struct char_data *ch, int amount, byte mode, room_rnum RDR)
{
  struct obj_data *obj;

  if (amount <= 0)
    send_to_char(ch, "Heh heh heh.. we are jolly funny today, eh?\r\n");
  else if (GET_GOLD(ch) < amount)
    send_to_char(ch, "You don't have that many coins!\r\n");
  else {
    if (mode != SCMD_JUNK) {
      WAIT_STATE(ch, PULSE_VIOLENCE); /* to prevent coin-bombing */
      obj = create_money(amount);
      if (mode == SCMD_DONATE) {
	send_to_char(ch, "You throw some gold into the air where it disappears in a puff of smoke!\r\n");
	act("$n throws some gold into the air where it disappears in a puff of smoke!",
	    FALSE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, RDR);
	act("$p suddenly appears in a puff of orange smoke!", 0, 0, obj, 0, TO_ROOM);
      } else {
        char buf[MAX_STRING_LENGTH];

        if (!drop_wtrigger(obj, ch)) {
          extract_obj(obj);
          return;
        }

	snprintf(buf, sizeof(buf), "$n drops %s.", money_desc(amount));
	act(buf, TRUE, ch, 0, 0, TO_ROOM);

	send_to_char(ch, "You drop some gold.\r\n");
	obj_to_room(obj, IN_ROOM(ch));
      }
    } else {
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "$n drops %s which disappears in a puff of smoke!", money_desc(amount));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);

      send_to_char(ch, "You drop some gold which disappears in a puff of smoke!\r\n");
    }
    decrease_gold(ch, amount);
  }
}

#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")
static int perform_drop(struct char_data *ch, struct obj_data *obj,
		     byte mode, const char *sname, room_rnum RDR)
{
  char buf[MAX_STRING_LENGTH];
  int value;

  if (!drop_otrigger(obj, ch))
    return 0;

  if ((mode == SCMD_DROP) && !drop_wtrigger(obj, ch))
    return 0;

  if (GET_OBJ_VNUM(obj) == 3203 && GET_LEVEL(ch) < LVL_IMMORT)
	return 0;

  if (OBJ_FLAGGED(obj, ITEM_NODROP) && !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
    snprintf(buf, sizeof(buf), "You can't %s $p, it must be CURSED!", sname);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }
  
  if ((mode == SCMD_JUNK) && GET_OBJ_TYPE(obj) == ITEM_CONTAINER && (obj->contains)) {
    snprintf(buf, sizeof(buf), "You can't %s $p, has something inside it!", sname);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }
	

  snprintf(buf, sizeof(buf), "You %s $p.%s", sname, VANISH(mode));
  act(buf, FALSE, ch, obj, 0, TO_CHAR);

  snprintf(buf, sizeof(buf), "$n %ss $p.%s", sname, VANISH(mode));
  act(buf, TRUE, ch, obj, 0, TO_ROOM);

  obj_from_char(obj);

  if ((mode == SCMD_DONATE) && OBJ_FLAGGED(obj, ITEM_NODONATE))
    mode = SCMD_JUNK;

  switch (mode) {
  case SCMD_DROP:
    obj_to_room(obj, IN_ROOM(ch));
    return (0);
  case SCMD_DONATE:
    obj_to_room(obj, RDR);
    act("$p suddenly appears in a puff a smoke!", FALSE, 0, obj, 0, TO_ROOM);
    return (0);
  case SCMD_JUNK:
    value = MAX(1, MIN(200, GET_OBJ_COST(obj) / 16));
    extract_obj(obj);
    return (value);
  default:
    log("SYSERR: Incorrect argument %d passed to perform_drop.", mode);
    /* SYSERR_DESC: This error comes from perform_drop() and is output when
     * perform_drop() is called with an illegal 'mode' argument. */
    break;
  }

  return (0);
}

ACMD(do_drop)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj;
  struct obj_data *booster;
  room_rnum RDR = 0;
  byte mode = SCMD_DROP;
  int dotmode, amount = 0, multi, num_don_rooms;
  const char *sname;

  switch (subcmd) {
  case SCMD_JUNK:
    sname = "junk";
    mode = SCMD_JUNK;
    break;
  case SCMD_DONATE:
    sname = "donate";
    mode = SCMD_DONATE;
    /* fail + int chance for room 1   */
    num_don_rooms = (CONFIG_DON_ROOM_1 != NOWHERE) * 2 +
                    (CONFIG_DON_ROOM_2 != NOWHERE)     +
                    (CONFIG_DON_ROOM_3 != NOWHERE)     + 1 ;
    switch (rand_number(0, num_don_rooms)) {
    case 0:
      mode = SCMD_JUNK;
      break;
    case 1:
    case 2:
      RDR = real_room(CONFIG_DON_ROOM_1);
      break;
    case 3: RDR = real_room(CONFIG_DON_ROOM_2); break;
    case 4: RDR = real_room(CONFIG_DON_ROOM_3); break;

    }
    if (RDR == NOWHERE) {
      send_to_char(ch, "Sorry, you can't donate anything right now.\r\n");
      return;
    }
    break;  
  default:
    sname = "drop";
    break;
  }

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "What do you want to %s?\r\n", sname);
    return;
  } else if (is_number(arg)) {
    multi = atoi(arg);
    one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg))
      perform_drop_gold(ch, multi, mode, RDR);
    else if (multi <= 0)
      send_to_char(ch, "Yeah, that makes sense.\r\n");
    else if (!*arg)
      send_to_char(ch, "What do you want to %s %d of?\r\n", sname, multi);
    else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
      send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);
    else {
      do {
        next_obj = get_obj_in_list_vis(ch, arg, NULL, obj->next_content);
        amount += perform_drop(ch, obj, mode, sname, RDR);
        obj = next_obj;
      } while (obj && --multi);
    }
  } else {
    dotmode = find_all_dots(arg);

    /* Can't junk or donate all */
    if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
      if (subcmd == SCMD_JUNK)
	send_to_char(ch, "Go to the dump if you want to junk EVERYTHING!\r\n");      
      else
	send_to_char(ch, "Go do the donation room if you want to donate EVERYTHING!\r\n");
      return;
    } else {	
    if (dotmode == FIND_ALL) {
      if (!ch->carrying)
	send_to_char(ch, "You don't seem to be carrying anything.\r\n");
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  amount += perform_drop(ch, obj, mode, sname, RDR);
	}
    } else if (dotmode == FIND_ALLDOT) {
      if (!*arg) {
	send_to_char(ch, "What do you want to %s all of?\r\n", sname);
	return;
      }
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);

      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg, NULL, obj->next_content);
	amount += perform_drop(ch, obj, mode, sname, RDR);
	obj = next_obj;
      }
    } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      else {
		if (GET_OBJ_VNUM(obj) == 3203)
		  send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	    else
	amount += perform_drop(ch, obj, mode, sname, RDR);
	  }
    }
  }

  if (amount && (subcmd == SCMD_JUNK)) {
	if ((rand_number(1, 101) + (amount / 100) >= 101)){
	  booster = read_object(3250, VIRTUAL);
	  obj_to_char(booster, ch);
	  send_to_char(ch, "You have been rewarded by the Game Masters with a booster pack!\r\n");
	} else {
	GET_GOLD(ch) += amount;
    send_to_char(ch, "You have been rewarded by the Game Masters with %d Jenny!\r\n", amount);
    act("$n has been rewarded by the Game Masters!", TRUE, ch, 0, 0, TO_ROOM);
    }
   }
 }
}

static void fly_to_char (struct char_data *ch, struct char_data *target)
{
	if (!ch || !target || ch == target || IN_ROOM(ch) == NOWHERE || IN_ROOM(target) == NOWHERE)
	  return;
	
	room_rnum prevroom = IN_ROOM(ch);
	
	send_to_char(ch, "A bolt of energy envelops you!\r\n");
	act("A bolt of energy envelope $n and sent $S to far away.", TRUE, ch, 0, 0, TO_ROOM);
	if (world[IN_ROOM(target)].zone != world[prevroom].zone) {
	  send_to_zone("You see a bolt of energy flying through the skies.\r\n", world[IN_ROOM(target)].zone);
	}	
	send_to_room(IN_ROOM(target), "A bolt of energy flying toward your direction hits the ground!\r\n");
	char_from_room(ch);	
	char_to_room(ch, IN_ROOM(target));	  
	if (world[IN_ROOM(target)].zone != world[prevroom].zone) {
	  send_to_zone("You see a bolt of energy flying through the skies.\r\n", world[prevroom].zone);
	}
	look_at_room(ch, 0);
	return;
}

static void fly_to_room (struct char_data *ch, room_vnum room)
{	
	if (!ch || !room || IN_ROOM(ch) == NOWHERE)
	  return;
  
    room_rnum target = real_room(room);
	room_rnum prevroom = IN_ROOM(ch);
	
	if (target == NOWHERE)
	  return;
  
	send_to_char(ch, "\tYA bolt of energy envelops you!\tn\r\n");
	act("A bolt of energy envelope $n and sent $S to far away.", TRUE, ch, 0, 0, TO_ROOM);
	if (world[target].zone != world[prevroom].zone) {
	  send_to_zone("You see a bolt of energy flying through the skies.\r\n", world[target].zone);
	}
	send_to_room(target, "\tYA bolt of energy flying toward your direction hits the ground!\tn\r\n");	
	char_from_room(ch);	
	char_to_room(ch, target);
	if (world[target].zone != world[prevroom].zone) {
	  send_to_zone("You see a bolt of energy flying through the skies.\r\n", world[prevroom].zone);
	}
	look_at_room(ch, 0);	
	return;
}

ACMD(do_gain) 
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH], *desc;  
  struct char_data *tmp_char, *target = NULL;
  struct obj_data *obj, *next_obj, *held, *cont;
  struct follow_type *k, *next;
  int dotmode, i, dice = 0, city = 0, check = 0, msg = 0;
  bool found = FALSE;
  trig_data *trig;
  
  argument = one_argument(argument, arg);
  
  if (!(held = GET_EQ(ch, WEAR_HOLD)) && !*arg) {
	send_to_char(ch, "You must hold or specify the items before gain it.\r\n\tcSyntax: gain \tYitem\tc OR gain all.\tYitem\tn\r\n");
	return;
  }

  if (held && GET_OBJ_TYPE(held) == ITEM_SPELLCARD) {
	if (!PLR_FLAGGED(ch, PLR_BOOK)) {
	  send_to_char(ch, "\tcYou need call your \tCbook\tc first.\tn\r\n");
	  return;
	}  
  
	if (!(GET_OBJ_VNUM(held) == 1009 || GET_OBJ_VNUM(held) == 1016 || GET_OBJ_VNUM(held) == 1017 || GET_OBJ_VNUM(held) == 1031 || GET_OBJ_VNUM(held) == 1032 || GET_OBJ_VNUM(held) == 1038)) {
	  if ((target = get_player_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
	    found = TRUE;
		if (ch != target)
	      check += pmet_check(target, ch);
		else
		  check = -1;
	    if (check >= 0)		
		  goto end;	  
	  }
	  if (target && PRF_FLAGGED(target, PRF_NOHASSLE)) {
	    send_to_char(ch, "Unable to comply, target is a Game Master with 'no hassle' activated.");
	    send_to_char(target, "Player \tW%s\tn tried to use spellcard %s at you.", GET_NAME(ch), held->short_description);
	    return;
	  }
	}
	if (!*arg && found == TRUE)
	  found = FALSE;    	
	switch (GET_OBJ_VNUM(held)) {
	  case 1001: /* OK */	
	    if (target) {
		  sprintf(buf, "Peek ON! %s!", GET_NAME(target));
		  do_say(ch, buf, cmd, 0);
	      if (generic_find("binder", FIND_OBJ_INV, target, &tmp_char, &cont)) {
	        send_to_char(ch, "%s (book):\r\n", GET_NAME(target));
	        list_obj_to_char(cont->contains, ch, 1, 4);			
	      }
	    } else
		  goto end;
	    break;
	  case 1002: /* OK */
	    if (target) {
		  sprintf(buf, "Fluroscopy ON! %s!", GET_NAME(target));
		  do_say(ch, buf, cmd, 0);
	      if (generic_find("binder", FIND_OBJ_INV, target, &tmp_char, &cont)) {
	        send_to_char(ch, "%s (book):\r\n", GET_NAME(target));
	        list_obj_to_char(cont->contains, ch, 1, 3);			
	      }
	    } else
		  goto end;
	    break;
	  case 1005: /* OK */	    
	    if (target) { 
		  sprintf(buf, "Magnetic Force ON! %s!", GET_NAME(target));
		  do_say(ch, buf, cmd, 0);
		  fly_to_char(ch, target);		  
		} else
		  goto end;
	  break;
	  case 1009: /* OK */
	    if (!*arg || !GET_CITY_MET(ch))
		  goto end;
	  
	    zone_rnum znum;
		room_vnum vroom;
		room_rnum rroom;
		
		for (i = 0; i < GET_CITY_MET(ch); i++) {
		  znum = real_zone(ch->player_specials->saved.city_met[i]);
		  if (is_abbrev(arg, zone_table[znum].name)) {
			city += cmet_check(zone_table[znum].number, ch);
		    if (city < 0) {
			  sprintf(buf, "Return ON! %s!", zone_table[znum].name);  
			  do_say(ch, buf, cmd, 0);
			  dice = (zone_table[znum].number * 100);
		    } else
			  goto end;
		    break;
		  }
		}
		if (!dice)
		  goto end;
		for (i = (dice + 100); dice < i; dice++) {		  
		  vroom = dice;		  
		  rroom = real_room(vroom);
		  if (rroom == NOWHERE || !ROOM_FLAGGED(rroom, ROOM_WORLDMAP))
			continue;
		  else {
			fly_to_room(ch, GET_ROOM_VNUM(rroom));			
			break;
		  }
		}
	    break;
	  case 1012: /* OK */
	    if (target && IN_ROOM(ch) == IN_ROOM(target) && 
			!ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(ch)), ZONE_NOASTRAL) &&
			!ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
	      room_rnum rroom;

		  sprintf(buf, "Relegate ON! %s!", GET_NAME(target));
		  do_say(ch, buf, cmd, 0);
		do {
			rroom = rand_number(0, top_of_world);
		} while (ROOM_FLAGGED(rroom, ROOM_PRIVATE) || ROOM_FLAGGED(rroom, ROOM_DEATH) ||
				ROOM_FLAGGED(rroom, ROOM_GODROOM) || ZONE_FLAGGED(GET_ROOM_ZONE(rroom), ZONE_CLOSED) ||
				ZONE_FLAGGED(GET_ROOM_ZONE(rroom), ZONE_NOASTRAL));				
		
		fly_to_room(target, GET_ROOM_VNUM(rroom));
		} else
		  goto end;
	    break;
	  case 1015: /* OK */
	    if (target) {
		  sprintf(buf, "Sightvision ON! %s!", GET_NAME(target));
		  do_say(ch, buf, cmd, 0);
	      if (generic_find("binder", FIND_OBJ_INV, target, &tmp_char, &cont)) {
	        send_to_char(ch, "%s (book):\r\n", GET_NAME(target));
	        list_obj_to_char(cont->contains, ch, 1, 2);			
	      }
	    } else
		  goto end;
	    break;
	  case 1016: /* OK */
	    found = TRUE;		
		do_say(ch, "Drift ON!", cmd, 0);
		for (i = 0; i <= top_of_zone_table; i++) {		  
		  if (ZONE_FLAGGED(i, ZONE_CITY)) {			
			check = 0;
			if (GET_CITY_MET(ch))
			  check += cmet_check(ZONE_NUMBER(i), ch);
			if (check >= 0)
			  dice++;
		  }
		}
		if (!dice) {
		  send_to_char(ch, "%s vanishes in a puff of smoke and nothing happens!\r\n", held->short_description);
		  extract_obj(held);
		  return;
		}
		while (found) {
		  for (i = 0; i <= top_of_zone_table; i++) {			
		    if (ZONE_FLAGGED(i, ZONE_CITY)) {			  
			  check = 0;
			  if (GET_CITY_MET(ch))
				check += cmet_check(ZONE_NUMBER(i), ch);
			  if (check >= 0 && rand_number(1, dice) == 1) {
			    found = FALSE;
			    room_rnum prevroom = IN_ROOM(ch);
			    while (prevroom == IN_ROOM(ch)) {			    
			      dice = ((ZONE_NUMBER(i) * 100) + rand_number(1, 99));
				  zone_vnum fly = dice;
			      fly_to_room(ch, fly);
			    }
			    GET_CITY_MET(ch)++;
			    ch->player_specials->saved.city_met[check] = zone_table[i].number;
			    send_to_char(ch, "\r\n\tDNew city met: \tG%s\tD!\tn\r\n", zone_table[i].name);
				save_char(ch);
				Crash_crashsave(ch);
		        break;
			  } 
		    }
		  }
		}
	    break;
	  case 1017: /* OK */		
		found = TRUE;				
		do_say(ch, "Collision ON!", cmd, 0);
		for (i = 0; i <= top_of_p_table; i++) {
		  if (player_table[i].id == GET_IDNUM(ch))
			continue;
		  if ((target = get_player_vis(ch, player_table[i].name, NULL, FIND_CHAR_WORLD))) {	
	      check = 0;
	      check += pmet_check(target, ch);
		  if (check > 0)
			  dice++;
		  }
		}
		if (!dice) {
		  send_to_char(ch, "%s vanishes in a puff of smoke and nothing happens!\r\n", held->short_description);
		  extract_obj(held);
		  return;
		}
		dice = rand_number(1, dice);
		for (i = 0; i <= top_of_p_table; i++) {
		  if (player_table[i].id == GET_IDNUM(ch))
			continue;
		  if ((target = get_player_vis(ch, player_table[i].name, NULL, FIND_CHAR_WORLD))) {
			check = 0;
	        check += pmet_check(target, ch);
			if (check > 0 && dice == 1) {
			  fly_to_char(ch, target);
			  save_char(ch);
			  Crash_crashsave(ch);
			  break;
			} else if (check > 0)
			  dice--;
		    }
		}
		break;
	  case 1030:
	    if (!*arg)
		  goto end;
		send_to_char(ch, "You insert the card in a special slot inside binder's back cover...\r\nThe screen inside the back cover starts to show some information.\r\n\r\n");
		dice = atoi(arg);		 
		if (dice < 1 || dice > 99) {
		  send_to_char(ch, "'ERROR: Invalid number, must be between 1 and 99.'\r\n\r\nYour book spits out the spell card from casting slot.\r\n");
		  return;
		}
		send_to_char(ch, "'Location of restricted card number %d found... On screen:'\r\n\r\nYour book spits out the spell card from casting slot.\r\n", dice);
		switch (dice) {
			
		  default:
		    send_to_char(ch, "'ERROR: Card number %d not implemented yet.'\r\n\r\nYour book spits out the spell card from casting slot.\r\n", dice);
		    break;
		}
	    break;
	  case 1031:	    
		if (!*arg)
		  goto end;
		send_to_char(ch, "You insert the card in a special slot inside binder's back cover...\r\nThe screen inside the back cover starts to show some information.\r\n\r\n");
		obj_vnum onum = atoi(arg);		 
		if (onum < 1 || onum > 99) {
		  send_to_char(ch, "'ERROR: Invalid number, must be between 1 and 99.'\r\n\r\nYour book spits out the spell card from casting slot.\r\n");
		  return;
		}		  
		onum = onum + 65300;		
	    if ((obj = read_object(onum, VIRTUAL)) != NULL) {
		  if ((desc = find_exdesc(obj->name, obj->ex_description)) != NULL)
		    send_to_char(ch, "%s\r\n%s", obj->short_description, desc);
		  else {
			send_to_char(ch, "'ERROR: Card number %d has no description yet.'\r\n\r\nYour book spits out the spell card from casting slot.\r\n", atoi(arg));
			log("SYSERR: Missing description of restricted %s.", obj->short_description);
			extract_obj(obj);
			return;
		  }
		  extract_obj(obj);
		} else {
		  send_to_char(ch, "'ERROR: Card number %d not implemented yet.'\r\n\r\nYour book spits out the spell card from casting slot.\r\n", atoi(arg));
		  return; 
		}		    
	    break;
	  case 1032:
		do_say(ch, "Lottery ON!", cmd, 0);
	    do {
		  obj_rnum crnum;
		  if (dice || has_luck(ch)) {
			if (!dice)
			  dice = rand_number(1, 2);
			if (dice == 1)
			  crnum = real_object(rand_number(65303, 65397));
			else
			  crnum = rand_number(1, top_of_objt);
		  } else if (found || rand_number(1, 40) == 1) {
			found = TRUE;
			crnum = real_object(rand_number(65301, 65399));
		  } else
	        crnum = rand_number(1, top_of_objt);
		  if ((obj = read_object(crnum, REAL)) != NULL) {
			if (dice == 1 && GET_OBJ_COST(obj) < 100000) {
			  obj_to_char(obj, ch);
			  send_to_room(IN_ROOM(ch), "%s's %s turned into a %s.\r\n", GET_NAME(ch), held->short_description, ch->carrying->short_description);			  
			  check = 1;
			} else if (GET_OBJ_TYPE(obj) != ITEM_SPELLCARD && !OBJ_FLAGGED(obj, ITEM_NOGAIN) && GET_OBJ_VNUM(obj) > 100) {
			  if (dice == 2 && (GET_OBJ_TYPE(obj) == ITEM_RESTRICTED || (GET_OBJ_COST(obj) == 0 && !OBJ_FLAGGED(obj, ITEM_MAGIC)) || GET_LEVEL(ch) < GET_OBJ_LEVEL(obj) || obj->affected[0].modifier <= 0))
				continue;
		      obj_to_char(obj, ch);
			  check = 0;
			  if (!IS_CARD(obj))		      
	            check += make_card(ch, obj, FALSE);
			  else
				check = 1;
			  send_to_room(IN_ROOM(ch), "%s's %s turned into a %s.\r\n", GET_NAME(ch), held->short_description, ch->carrying->short_description);	
	        } else extract_obj(obj);
          } 
		} while (!check);		
	    break;
	  case 1037:
	    if (!*arg || !(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
		  goto end;	    
		else {
		  if (GET_OBJ_TYPE(obj) == ITEM_BOOSTER || GET_OBJ_TYPE(obj) == ITEM_MONEY || IS_CARD(obj) || IS_CORPSE(obj) || GET_OBJ_VNUM(obj) == 65535) {
			found = TRUE;
			send_to_char(ch, "'ERROR: Invalid target.' - ");
			goto end;
		  }
		  do_say(ch, "Recycle ON!", cmd, 0);
		  next_obj = read_object(GET_OBJ_VNUM(obj), VIRTUAL);		  
		  obj_to_char(next_obj, ch);
		  dice = make_card(ch, next_obj, TRUE);
		  if (!dice)
		    extract_obj(next_obj);
		  else
			extract_obj(obj);
		}
	    break;
	  case 1038:	    
	    if (!*arg)
		  goto end;
		dice = real_trigger(1038);
		if ((dice == NOTHING) || !(trig = read_trigger(dice)))
		  goto end;
		send_to_char(ch, "You insert the card in a special slot inside binder's back cover...\r\nThe screen inside the back cover starts to show some information.\r\n");		
		sprintf(buf, "Search for %s", arg);		
	    if (!SCRIPT(ch))
          CREATE(SCRIPT(ch), struct script_data, 1);
        add_trigger(SCRIPT(ch), trig, -1);
		do_say(ch, buf, cmd, 0);
	    break;
	  case 1039:
		if (!*arg)
		  goto end;
		room_rnum prevroom = IN_ROOM(ch);
	    if (found) {
		  sprintf(buf, "Accompany ON! %s!", GET_NAME(target));
		  do_say(ch, buf, cmd, 0);
		  fly_to_char(ch, target);
		} else {
		  if (!GET_CITY_MET(ch))
		    goto end;
	  
	      zone_rnum znum;
		  room_vnum vroom;
		  room_rnum rroom;
		
		  for (i = 0; i < GET_CITY_MET(ch); i++) {
		    znum = real_zone(ch->player_specials->saved.city_met[i]);
		    if (is_abbrev(arg, zone_table[znum].name)) {
		  	  city += cmet_check(zone_table[znum].number, ch);
		    if (city < 0) {
			  sprintf(buf, "Accompany ON! %s!", zone_table[znum].name);  
			  do_say(ch, buf, cmd, 0);		
			  dice = (ch->player_specials->saved.city_met[i] * 100);
		    } else
			  goto end;
		    break;
		    }
		  }
		  for (i = (dice + 100); dice < i; dice++) {
		    vroom = dice;
		    rroom = real_room(vroom);
		    if (rroom == NOWHERE || !ROOM_FLAGGED(rroom, ROOM_WORLDMAP))
			  continue;
			else {
			  fly_to_room(ch, GET_ROOM_VNUM(rroom));			
			break;
		    }			
		  }
		}
		if (ch->followers) {		  
		  for (k = ch->followers; k; k = next) {
		    next = k->next;			
		    if (IN_ROOM(k->follower) == prevroom)
			  fly_to_room(k->follower, GET_ROOM_VNUM(IN_ROOM(ch)));		    
		  }
		}
		break;
	  default:
	    send_to_char(ch, "You insert the card in a special slot inside binder's back cover...\r\nThe screen inside the back cover starts to show some information.\r\n");
		send_to_char(ch, "%s was not implemented yet, please wait for the next updates.\r\n", held->short_description);
	    log("SYSERR: %s called not implemented yet.", held->short_description);
		return;
	  }	
	extract_obj(held);
	return;
	end:
	if (!found)
	  send_to_char(ch, "You must specify the target of %s.\r\n", held->short_description);
    else
	  send_to_char(ch, "Nothing happens with %s.\r\n", held->short_description);
	return;
  
  }
  
  if (!*arg){
    if (held) {
	  do_say(ch, "GAIN!", cmd, 0);
	  if (OBJ_FLAGGED(held, ITEM_NOGAIN) && !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
	    send_to_char(ch, "Nothing happens with %s.\r\n", held->short_description);
		return;
	  }  
      switch (GET_OBJ_TYPE(held)) {
	    case 3:
        case 4:
		  if (GET_OBJ_VAL(held, 2) == GET_OBJ_VAL(held, 1) || GET_LEVEL(ch) == LVL_IMPL)
			make_card(ch, held, TRUE);
		  else
			send_to_char(ch, "A puff of smoke comes out of %s and nothing happens... low battery perhaps? \r\n", held->short_description);  
		break;
//		case 8:
	    case 13:
        case 15:
		  if (!held->contains)
			make_card(ch, held, TRUE);
		  else
			send_to_char(ch, "Nothing happens with %s.\r\n", held->short_description);
		  break;
        case 16:
        case 17:
	    case 18:
	    case 20:
        case 23:
		  if (!PRF_FLAGGED(ch, PRF_NOHASSLE)) {
            send_to_char(ch, "Nothing happens with %s.\r\n", held->short_description);
            break;
		  }
        default:
          make_card(ch, held, TRUE);
          break;		
      }
    } else {
        send_to_char(ch, "You must hold or specify the items before gain it.\r\n\tcSyntax: gain \tYitem\tc OR gain all.\tYitem\tn\r\n");
    }
  } else {
	dotmode = find_all_dots(arg);

    if (dotmode == FIND_ALL)
      send_to_char(ch, "\tcSyntax: gain all.\tYitem\tn\r\n"); 
    else if (dotmode == FIND_ALLDOT) {
      if (!*arg) {
	send_to_char(ch, "What do you want to gain all of?\r\n");
	return;
      }
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	    send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);
	  else
		do_say(ch, "GAIN!", cmd, 0);
      while (obj) {
	    next_obj = get_obj_in_list_vis(ch, arg, NULL, obj->next_content);
	    msg = 0;
		if (GET_OBJ_TYPE(obj) == ITEM_CARD)
		  msg += make_card(ch, obj, TRUE);
	  
	    if (!msg && GET_OBJ_TYPE(obj) == ITEM_CARD)
	      send_to_char(ch, "Nothing happens with %s.\r\n", obj->short_description);
	    obj = next_obj;
      }
    } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      else {
		if (GET_OBJ_VNUM(obj) == 3203)
		  send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	    else if (GET_OBJ_TYPE(obj) == ITEM_SPELLCARD)
		  send_to_char(ch, "You must hold the %s in order to gain it.\r\n", obj->short_description);
	    else {
		  do_say(ch, "GAIN!", cmd, 0);
		  msg = 0;
		  msg += make_card(ch, obj, TRUE);
		  if (!msg)
	        send_to_char(ch, "Nothing happens with %s.\r\n", obj->short_description);
		}
	  }
    }
  }
  
/*  } else if (dotmode == FIND_ALL || dotmode == FIND_ALLDOT) {
      send_to_char(ch, "You can not just gain EVERYTHING...\r\n"); 
  } else {
    if ((i = get_obj_pos_in_equip_vis(ch, arg, NULL, ch->equipment)) < 0) {
      send_to_char(ch, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
      return;
    }
	do_say(ch, "GAIN!", cmd, 0);
    if (OBJ_FLAGGED(GET_EQ(ch, i), ITEM_NOGAIN) && !PRF_FLAGGED(ch, PRF_NOHASSLE)) 
	  send_to_char(ch, "Nothing happens with %s.\r\n", arg);
    else if (GET_OBJ_DURABILITY(GET_EQ(ch, i)) < 100 && !PRF_FLAGGED(ch, PRF_NOHASSLE))
	  send_to_char(ch, "Huuum... %s appears to be damaged.\r\n", arg);
    else
      make_card(ch, GET_EQ(ch, i), TRUE);
  }*/
}

int make_card(struct char_data *ch, struct obj_data *obj, bool show)
{  
  struct obj_data *card, *rst;
  
  int number, limit = 0;
  
  /* Return if ch or obj is NULL | corpse | booster pack | spell card | generic money | container with obj */  
  if (!ch || !obj || IS_CORPSE(obj) || GET_OBJ_TYPE(obj) == ITEM_BOOSTER || GET_OBJ_TYPE(obj) == ITEM_SPELLCARD || 
       (GET_OBJ_TYPE(obj) == ITEM_MONEY && GET_OBJ_VNUM(obj) == 65535) || (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && (obj->contains)))
	return (0);
	
  /* GM can override no_gain */
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOHASSLE) && show)
	goto skip;
	
  /* Return npc and no_gain cards */
  if (IS_NPC(ch) || OBJ_FLAGGED(obj, ITEM_NOGAIN))
	return (0);
  
  if (show && OBJ_FLAGGED(obj, ITEM_NODROP) && (IS_NPC(ch) || !PRF_FLAGGED(ch, PRF_NOHASSLE))){
    act("You can't gain $p, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
    return (0);  
  } else if (GET_OBJ_TYPE(obj) == ITEM_WAND || GET_OBJ_TYPE(obj) == ITEM_STAFF) {
	if (GET_OBJ_VAL(obj, 2) < GET_OBJ_VAL(obj, 1)) {
	  if (show)
        send_to_char(ch, "A puff of smoke comes out of %s and nothing happens... low battery perhaps? \r\n", obj->short_description);
	  return (0);
	}
  }
  skip:

  number = GET_OBJ_VNUM(obj);
  
  /* number == 65535 && OBJ_FLAGGED(obj, ITEM_QUEST) == VOUCHER */  
  if ((number == 65535 && OBJ_FLAGGED(obj, ITEM_QUEST)) || (number > 65400 && number != 65535)) {
	if (number == 65535 && OBJ_FLAGGED(obj, ITEM_QUEST))
	  rst = read_object((GET_OBJ_RENT(obj) - 100), VIRTUAL);
    else
      rst = read_object((number - 100), VIRTUAL);
	if (obj_index[GET_OBJ_RNUM(rst)].number > GET_OBJ_RENT(rst))
	  limit = GET_OBJ_RENT(rst);
    else
	  extract_obj(rst);	
  }	
  
  if (limit == 0 && ((number > 65300 && number != 65535) || (number == 65535 && OBJ_FLAGGED(obj, ITEM_QUEST)))) {
	if (GET_OBJ_TYPE(obj) == ITEM_RESTRICTED) {
	  if (!(card = read_object((number + 100), VIRTUAL))) {
		if (GET_OBJ_TIMER(obj) > 0)
	      send_to_char(ch, "YAMEROOO! %s is too much valuable to waste like that, \tYput in \tnyour \tYbinder\tn quickly!!!\r\n", obj->short_description);
		return (0);
	  } else
		SET_BIT_AR(GET_OBJ_EXTRA(card), ITEM_NOGAIN);
    } else {
	  if (number == 65535)
		card = read_object((GET_OBJ_RENT(obj) - 100), VIRTUAL); 
	  else
        card = read_object((number - 100), VIRTUAL);
	}
    if (!(CAN_WEAR(card, ITEM_WEAR_TAKE)))
	  obj_to_room(card, IN_ROOM(ch));
	else
      obj_to_char(card, ch);    
    if (show)	
	  send_to_room(IN_ROOM(ch), "%s's %s turns into %s!\r\n", GET_NAME(ch), obj->short_description, card->short_description);
	extract_obj(obj);	
  } else if ((number == 65535 && !OBJ_FLAGGED(obj, ITEM_QUEST)) || (number != 65535 && GET_OBJ_TYPE(obj) == ITEM_CARD)) {
	if (GET_OBJ_RENT(obj) == 0)
	  return (0);
    card = read_object(GET_OBJ_RENT(obj), VIRTUAL);
	if (!(CAN_WEAR(card, ITEM_WEAR_TAKE)))
	  obj_to_room(card, IN_ROOM(ch));
	else {
      obj_to_char(card, ch);
	  get_check_money(ch, card); 
	}
    SET_BIT_AR(GET_OBJ_EXTRA(card), ITEM_NOGAIN);
    if (show)	
	  send_to_room(IN_ROOM(ch), "%s's %s turns into %s.\r\n", GET_NAME(ch), obj->short_description, card->short_description);
	extract_obj(obj);
/*  } else if (number > 65400 && number < 65535 && obj_index[GET_OBJ_RNUM(obj)].number > GET_OBJ_RENT(obj)) {
    send_to_char(IN_ROOM(ch), "The \tGG.I. \tDRing\tn flashes indicating that %s global transformation already reached the limit of %d.", obj->short_description, GET_OBJ_RENT(obj));
	return;
} else if (GET_OBJ_DURABILITY(obj) < 100) {
	if (show)
	  send_to_char(ch, "Huuum... %s appears to be damaged.\r\n", obj->short_description);
	return;  
*/  } else {
    char buf2[MAX_NAME_LENGTH + 64];	
    const char *word[] = {
    "H-",
    "G-",	
	"F-",
    "E-",
    "D-",
    "C-",
    "B-",
    "A-",
    "S-",
    "SS-"
  };
  
  int a, b, i, rarity, letter, value = 0, value2 = 0, maxCost;
  
  card = create_obj();  
  
  card->item_number = NOTHING;
  IN_ROOM(card) = NOWHERE;
  
  if (GET_OBJ_TYPE(obj) == ITEM_WEAPON){
	value += (((GET_OBJ_VAL(obj, 1) * GET_OBJ_VAL(obj, 1)) * 10) * (GET_OBJ_VAL(obj, 2) * 10));	
  } else if (GET_OBJ_TYPE(obj) == ITEM_ARMOR){
	value += ((GET_OBJ_VAL(obj, 0) * GET_OBJ_VAL(obj, 0)) * 300);
  }
  
  /* Check for a value increment of magic items from non-enfolded items */  
  if (!OBJ_FLAGGED(obj, ITEM_ENFOLDED) && limit == 0){
	for (i = 0; i < MAX_OBJ_AFFECT; i++){
      if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0)) {
		switch (obj->affected[i].location) {
		  case APPLY_STR:
		  case APPLY_DEX:
		  case APPLY_INT:
		  case APPLY_WIS:
		  case APPLY_CON:
		  case APPLY_CHA:
		    value2 += (obj->affected[i].modifier * 1.5);
			break;
		  case APPLY_MANA: 
		  case APPLY_HIT:
		  case APPLY_MOVE:
		    value2 += (obj->affected[i].modifier / 5);
			break;
		  case APPLY_AC:
		    value2 += ((obj->affected[i].modifier * -1) / 10);
		    break;
		  case APPLY_HITROLL:
		  case APPLY_DAMROLL:
		    value2 += obj->affected[i].modifier;
			break;
		  case APPLY_SAVING_SPELL:
		    value2 += ((obj->affected[i].modifier * -1) * 2);
		    break;
		}
	  }	
    }
  }
  
  if (limit == 0)
    value = (value + GET_OBJ_COST(obj));
  else
	value = GET_OBJ_COST(rst);
  
  if (value2 <= 0){
	value2 = 1;
  }
  
  maxCost = (value * value2);
  
  if (maxCost < 0){	
    maxCost = value;
  }
  
  if (maxCost == 0) {
    rarity = 0;
	letter = 0;
  } else if (maxCost < 5) {
	letter = 0;
  	switch (maxCost) {
	  case 1: rarity = 1000; break;
	  case 2: rarity = 900; break;
	  case 3: rarity = 800; break;
	  case 4: rarity = 700; break;
	}
  } else if (maxCost < 500) {
      a = 500 - maxCost;
	  b = 150;
	  rarity = a + b; 
      letter = 0; /* H */
  } else if (maxCost < 1000) {   
      a = 1000 - maxCost;
      a = a / 1.25; /* 400 */
	  b = 100;
	  rarity = a + b;
      letter = 1; /* G */
  } else if (maxCost < 2500) {   
      a = 2500 - maxCost;
	  a = a / 4; /* 375 */
	  b = 75;
	  rarity = a + b;
	  letter = 2; /* F */
  } else if (maxCost < 5000) {    
      a = 5000 - maxCost;
	  a = a / 8.31; /* 300 */
	  b = 50;
	  rarity = a + b;
	  letter = 3; /* E */
  } else if (maxCost < 10000) {    
      a = 10000 - maxCost;
	  a = a / 20; /* 250 */
	  b = 40;
	  rarity = a + b;
	  letter = 4; /* D */
  } else if (maxCost < 25000) {   
      a = 25000 - maxCost;
	  a = a / 75; /* 200 */
	  b = 35;
	  rarity = a + b;
	  letter = 5; /* C */
  } else if (maxCost < 50000) {    
      a = 50000 - maxCost;
	  a = a / 166; /* 150 */
	  b = 30;
	  rarity = a + b;
	  letter = 6; /* B */
  } else if (maxCost < 100000) {    
      a = 100000 - maxCost;
	  a = a / 500; /* 100 */
	  b = 25;
	  rarity = a + b;
	  letter = 7; /* A */
  } else if (maxCost < 250000) {   
      a = 250000 - maxCost;
	  a = a / 3000; /* 50 */
	  b = 20;
	  rarity = a + b;
      letter = 8; /* S */	  
  } else if (maxCost < 500000) {  
      a = 500000 - maxCost;
	  a = a / 12500; /* 20 */
	  b = 10;
	  rarity = a + b;
	  letter = 9; /* SS */	  
  } else if (maxCost < 1000000) {  
      a = 1000000 - maxCost;
	  a = a / 62501; /* 7 */
	  b = 2;
	  rarity = a + b;
	  letter = 9; /* between SS-9 and 2 */
  } else {
	  rarity = 1; /* > 1 million J */
	  letter = 9; /* SS-1 */
  }
  
  if (((OBJ_FLAGGED(obj, ITEM_INVISIBLE) && !OBJ_FLAGGED(obj, ITEM_CONCEALED)) || (OBJ_FLAGGED(obj, ITEM_NODROP))) && GET_OBJ_TYPE(obj) != ITEM_RESTRICTED && limit == 0){
	
	if (OBJ_FLAGGED(obj, ITEM_INVISIBLE)) {  	
    snprintf(buf2, sizeof(buf2), "%s invisible card", obj->name);
    card->name = strdup(buf2);
    
    snprintf(buf2, sizeof(buf2), "%s%d %c%s%s (invisible) %s%d card%s", KYEL, number, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], rarity, CNRM);
    card->short_description = strdup(buf2);
  
    snprintf(buf2, sizeof(buf2), "%s%d %c%s%s (invisible) %s%d card is lying here.%s", KYEL, number, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], rarity, CNRM);
    card->description = strdup(buf2);
	}
	else {
	  snprintf(buf2, sizeof(buf2), "%s cursed card", obj->name);
      card->name = strdup(buf2);
    
      snprintf(buf2, sizeof(buf2), "%s%d Cursed %c%s%s %s%d card%s", KYEL, number, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], rarity, CNRM);
      card->short_description = strdup(buf2);
  
      snprintf(buf2, sizeof(buf2), "%s%d Cursed %c%s%s %s%d card is lying here.%s", KYEL, number, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], rarity, CNRM);
      card->description = strdup(buf2);
	}
  
  } else {
	  if (limit != 0 && GET_OBJ_TYPE(rst) == ITEM_RESTRICTED){		
		if (!OBJ_FLAGGED(obj, ITEM_QUEST) || (number == 65535 && OBJ_FLAGGED(obj, ITEM_QUEST))) {
	      if (show)
		    send_to_char(ch, "Your ring starts to flash and a female voice says, 'Transformation limit reached: %d/%d'\r\n", obj_index[GET_OBJ_RNUM(rst)].number - 1, limit);
		  extract_obj(rst);
		  return (0);
		}
		rarity = 111 - letter;
        snprintf(buf2, sizeof(buf2), "%s voucher card", obj->name);
        card->name = strdup(buf2);
		
		snprintf(buf2, sizeof(buf2), "%s%d %c%s%s voucher %s150 card%s", KYEL, rarity, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], CNRM);
        card->short_description = strdup(buf2);  
	
	    snprintf(buf2, sizeof(buf2), "%s%d %c%s%s voucher %s150 card %sis lying here.", KYEL, rarity, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], CNRM);
        card->description = strdup(buf2);
		
/*		snprintf(buf2, sizeof(buf2), "%s voucher card", obj->name);
		card->ex_description->next->keyword = strdup(buf2);
		
		snprintf(buf2, sizeof(buf2), "A ticket that can be exchanged with %s.\r\nThis is only obtained when the maximum transform limit\r\nof %d for %s has been reached.%s", obj->short_description, limit, obj->short_description, CNRM);
		card->ex_description->next->description = strdup(buf2);
*/		
		maxCost = 140000;
		SET_BIT_AR(GET_OBJ_EXTRA(card), ITEM_QUEST);	
		extract_obj(rst);
	  }	else {
        snprintf(buf2, sizeof(buf2), "%s card", obj->name);
        card->name = strdup(buf2);
		
	    snprintf(buf2, sizeof(buf2), "%s%d %c%s%s %s%d card%s", KYEL, number, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], rarity, CNRM);
        card->short_description = strdup(buf2);  
	
	    snprintf(buf2, sizeof(buf2), "%s%d %c%s%s %s%d card%s is lying here.", KYEL, number, UPPER(*obj->short_description), obj->short_description + 1, KYEL, word[letter], rarity, CNRM);
        card->description = strdup(buf2);
	  }	  
  }  
  GET_OBJ_TYPE(card) = ITEM_CARD; 
  SET_BIT_AR(GET_OBJ_WEAR(card), ITEM_WEAR_TAKE);
  SET_BIT_AR(GET_OBJ_WEAR(card), ITEM_WEAR_HOLD);
  SET_BIT_AR(GET_OBJ_EXTRA(card), ITEM_NODONATE);  
  GET_OBJ_WEIGHT(card) = 0;
  GET_OBJ_COST(card) = maxCost; 
  GET_OBJ_RENT(card) = number;
  GET_OBJ_TIMER(card) = 62;  
  obj_to_char(card, ch);
  if (show)
    send_to_room(IN_ROOM(ch), "%s's %s turns into %s.\r\n", GET_NAME(ch), obj->short_description, card->short_description);
  extract_obj(obj);	
  }
  return (1);
}

void make_card_cnt(struct obj_data *obj)
{
  struct obj_data *card, *cnt;
    
  cnt = obj->in_obj;
  
  if (!obj || GET_OBJ_VNUM(cnt) == 3203 || OBJ_FLAGGED(obj, ITEM_NOGAIN) || GET_OBJ_TYPE(obj) != ITEM_CARD)
	return;  
  
  if (IS_CARD(obj) || GET_OBJ_RENT(obj) > 0) {
	if (!(card = read_object(GET_OBJ_RENT(obj), VIRTUAL)) || !cnt || card == cnt) {
	  if (GET_OBJ_TIMER(obj) == 0) {		
		goto skip;
	  } else
	    return;
	}
	obj_to_obj(card, cnt);
	SET_BIT_AR(GET_OBJ_EXTRA(cnt->contains), ITEM_NOGAIN);    
	skip:
    if (cnt->carried_by)
	  send_to_room(IN_ROOM(cnt->carried_by), "Some card turned into an item inside %s's %s.\r\n", GET_NAME(cnt->carried_by), cnt->short_description);
    else if (cnt->worn_by)
	  send_to_room(IN_ROOM(cnt->worn_by), "Some card turned into an item inside %s's %s.\r\n", GET_NAME(cnt->worn_by), cnt->short_description);
    else if (world[IN_ROOM(cnt)].people) {
	  send_to_room(IN_ROOM(cnt), "Some card turned into an item inside %s.\r\n", cnt->short_description);	  
	}
  }  
  extract_obj(obj);
}

void make_card_room(struct obj_data *obj)
{
  struct obj_data *card;    

  if (!obj || IN_ROOM(obj) == NOWHERE || OBJ_FLAGGED(obj, ITEM_NOGAIN) || GET_OBJ_TYPE(obj) != ITEM_CARD)
	return;  
  
  if (GET_OBJ_RENT(obj) > 0) {    
	if (!(card = read_object(GET_OBJ_RENT(obj), VIRTUAL))) {
	  if (GET_OBJ_TIMER(obj) == 0) {		
		goto skip;
	  } else
	    return;
	}
    obj_to_room(card, IN_ROOM(obj));
	SET_BIT_AR(GET_OBJ_EXTRA(card), ITEM_NOGAIN);
	skip:
    if (world[IN_ROOM(obj)].people)
	  send_to_room(IN_ROOM(obj), "Time's up! %s turns into %s here.\r\n", obj->short_description, card->short_description);  	    
  }  
  extract_obj(obj);
}

static void perform_give(struct char_data *ch, struct char_data *vict, struct obj_data *obj)
{
  if (!give_otrigger(obj, ch, vict))
    return;
  if (!receive_mtrigger(vict, ch, obj))
    return;
  if (GET_OBJ_VNUM(obj) == 3203 && !IS_NPC(ch) && GET_LEVEL(ch) < LVL_GOD)
	return;

  if (OBJ_FLAGGED(obj, ITEM_NODROP) && !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
    act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict) && GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(vict) < LVL_IMMORT) {
    act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict) > CAN_CARRY_W(vict) && GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(vict) < LVL_IMMORT) {
    act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  obj_from_char(obj);
  obj_to_char(obj, vict);
  act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
  act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
  act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);

  autoquest_trigger_check( ch, vict, obj, AQ_OBJ_RETURN);
  
  if (IS_NPC(ch) && !IS_NPC(vict) && !IS_CARD(obj))
	make_card(vict, obj, TRUE);
}

/* utility function for give */
static struct char_data *give_find_vict(struct char_data *ch, char *arg)
{
  struct char_data *vict;

  skip_spaces(&arg);
  if (!*arg)
    send_to_char(ch, "To who?\r\n");
  else if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (vict == ch)
    send_to_char(ch, "What's the point of that?\r\n");
  else
    return (vict);

  return (NULL);
}

static void perform_give_gold(struct char_data *ch, struct char_data *vict,
		            int amount)
{
  char buf[MAX_STRING_LENGTH];

  if (amount <= 0) {
    send_to_char(ch, "Heh heh heh ... we are jolly funny today, eh?\r\n");
    return;
  }
  if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))) {
    send_to_char(ch, "You don't have that many coins!\r\n");
    return;
  }
  send_to_char(ch, "%s", CONFIG_OK);

  snprintf(buf, sizeof(buf), "$n gives you %d gold coin%s.", amount, amount == 1 ? "" : "s");
  act(buf, FALSE, ch, 0, vict, TO_VICT);

  snprintf(buf, sizeof(buf), "$n gives %s to $N.", money_desc(amount));
  act(buf, TRUE, ch, 0, vict, TO_NOTVICT);

  if (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))
    decrease_gold(ch, amount);
    
  increase_gold(vict, amount);
  bribe_mtrigger(vict, ch, amount);
}

ACMD(do_calculate)
{
  int r_num, a, b, i, rarity, letter, value = 0, value2 = 0, maxCost;
  struct obj_data *obj;  
  char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];	
  const char *word[] = {
  "H-",
  "G-",	
  "F-",
  "E-",
  "D-",
  "C-",
  "B-",
  "A-",
  "S-",
  "SS-"
  };

  two_arguments(argument, arg1, arg2);
  
  if (!*arg1) {
	if (!subcmd)
	  send_to_char(ch, "\tcSyntax: \tCcalc <value> or <rarity letter> <rarity number>\tn\r\n");
    else
	  send_to_char(ch, "\tcSyntax: \tCocalc <vnum>\tn\r\n");
	return;
  }	  
  
/*  if (subcmd) {
	if (!*arg1) {
	  send_to_char(ch, "\tcSyntax: \tCcalc <obj vnum>\tn\r\n");
      return;
    } else if ((r_num = real_object(atoi(arg1))) == NOTHING) {
      send_to_char(ch, "There is no object with that number.\r\n");
      return;
	} else {
	  obj = read_object(r_num, REAL);
	  value = GET_OBJ_COST(obj);
	if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
	value += (((GET_OBJ_VAL(obj, 1) * GET_OBJ_VAL(obj, 1)) * 10) * (GET_OBJ_VAL(obj, 2) * 10));	
  } else if (GET_OBJ_TYPE(obj) == ITEM_ARMOR){
	value += ((GET_OBJ_VAL(obj, 0) * GET_OBJ_VAL(obj, 0)) * 300);
  }  

  if (!OBJ_FLAGGED(obj, ITEM_ENFOLDED)){
	for (i = 0; i < MAX_OBJ_AFFECT; i++){
      if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0)) {
		switch (obj->affected[i].location) {
		  case APPLY_STR:
		  case APPLY_DEX:
		  case APPLY_INT:
		  case APPLY_WIS:
		  case APPLY_CON:
		  case APPLY_CHA:
		    value2 += (obj->affected[i].modifier * 1.5);
			break;
		  case APPLY_MANA: 
		  case APPLY_HIT:
		  case APPLY_MOVE:
		    value2 += (obj->affected[i].modifier / 5);
			break;
		  case APPLY_AC:
		    value2 += ((obj->affected[i].modifier * -1) / 10);
		    break;
		  case APPLY_HITROLL:
		  case APPLY_DAMROLL:
		    value2 += obj->affected[i].modifier;
			break;
		  case APPLY_SAVING_SPELL:
		    value2 += ((obj->affected[i].modifier * -1) * 2);
		    break;
		}
	  }	
    }
  }  
  
  value = (value + GET_OBJ_COST(obj));
   
  if (value2 <= 0){
	value2 = 1;
  }
    
  maxCost = (value * value2);  
  
  if (maxCost < 0){	
    maxCost = value;
  }
	goto skip;
	}
  }
*/  
  if (is_number(arg1)) {
	maxCost = atoi(arg1);
	*arg2 = '\0';	
  } else {
	if (!is_number(arg2) || atoi(arg2) < 1) {
	  send_to_char(ch, "\tcSyntax: \tCcalc <rarity letter> and <rarity number>\tn\r\n");
	  return; 
	}
	rarity = atoi(arg2);
	switch (*arg1) {
	  case 'h':
	  case 'H':
	    letter = 0;
	    if (rarity > 900) {
		  maxCost = 1;
		} else if (rarity > 800) {
		  maxCost = 2;
		} else if (rarity > 700) {
		  maxCost = 3;
		} else if (rarity > 645) {
		  maxCost = 4;		  
		} else {		  
		  maxCost = (650 - rarity);
		}
		goto end;
	    break;
	  case 'g': case 'G':
	    maxCost = ((((1000 / 1.25) + 100) - rarity) * 1.25);
	    letter = 1;
		goto end;
	    break;
	  case 'f': case 'F':
	    letter = 2;
		maxCost = ((((2500 / 4) + 75) - rarity) * 4);
		goto end;
	    break;
	  case 'e': case 'E':
	    letter = 3;
		maxCost = ((((5000 / 8.30) + 50) - rarity) * 8.30);
		goto end;
	    break;
	  case 'd': case 'D':
	    letter = 4;
		maxCost = ((((10000 / 20) + 40) - rarity) * 20);
		goto end;
	    break;
	  case 'c': case 'C':
	    letter = 5;
		maxCost = ((((25000 / 75) + 35) - rarity) * 75);
		goto end;
	    break;
	  case 'b': case 'B':
	    letter = 6;
		maxCost = ((((50000 / 165) + 30) - rarity) * 165);
		goto end;
	    break;
	  case 'a': case 'A':
	    letter = 7;
		maxCost = ((((100000 / 500) + 25) - rarity) * 500);
		goto end;
	    break;
	  case 's': case 'S':
	    letter = 8;
		maxCost = ((((250000 / 3000) + 20) - rarity) * 3000);
		goto end;
	    break;
	  case 'z': case 'Z':
	    letter = 9;
		if (rarity == 1)
		  maxCost = 2000000;
	    else if (rarity < 10)
		  maxCost = ((((1000000 / 62500) + 2) - rarity) * 62500);
	    else
		  maxCost = ((((500000 / 12500) + 10) - rarity) * 12500);
		goto end;
	    break;
	  default:
	    send_to_char(ch, "First digit must be a rarity letter between SS and H.\r\n");
		return;
	}	
  }
  
  skip:
  
  if (maxCost <= 0) {
    rarity = 0;
	letter = 0;
  } else if (maxCost < 5) {
	letter = 0;
  	switch (maxCost) {
	  case 1: rarity = 1000; break;
	  case 2: rarity = 900; break;
	  case 3: rarity = 800; break;
	  case 4: rarity = 700; break;
	}
  } else if (maxCost < 500) {
      a = 500 - maxCost;
	  b = 150;
	  rarity = a + b; 
      letter = 0; /* H */
  } else if (maxCost < 1000) {   
      a = 1000 - maxCost;
      a = a / 1.25; /* 400 */
	  b = 100;
	  rarity = a + b;
      letter = 1; /* G */
  } else if (maxCost < 2500) {   
      a = 2500 - maxCost;
	  a = a / 4; /* 375 */
	  b = 75;
	  rarity = a + b;
	  letter = 2; /* F */
  } else if (maxCost < 5000) {    
      a = 5000 - maxCost;
	  a = a / 8.31; /* 300 */
	  b = 50;
	  rarity = a + b;
	  letter = 3; /* E */
  } else if (maxCost < 10000) {    
      a = 10000 - maxCost;
	  a = a / 20; /* 250 */
	  b = 40;
	  rarity = a + b;
	  letter = 4; /* D */
  } else if (maxCost < 25000) {   
      a = 25000 - maxCost;
	  a = a / 75; /* 200 */
	  b = 35;
	  rarity = a + b;
	  letter = 5; /* C */
  } else if (maxCost < 50000) {    
      a = 50000 - maxCost;
	  a = a / 166; /* 150 */
	  b = 30;
	  rarity = a + b;
	  letter = 6; /* B */
  } else if (maxCost < 100000) {    
      a = 100000 - maxCost;
	  a = a / 500; /* 100 */
	  b = 25;
	  rarity = a + b;
	  letter = 7; /* A */
  } else if (maxCost < 250000) {   
      a = 250000 - maxCost;
	  a = a / 3000; /* 50 */
	  b = 20;
	  rarity = a + b;
      letter = 8; /* S */	  
  } else if (maxCost < 500000) {  
      a = 500000 - maxCost;
	  a = a / 12500; /* 20 */
	  b = 10;
	  rarity = a + b;
	  letter = 9; /* SS */	  
  } else if (maxCost < 1000000) {  
      a = 1000000 - maxCost;
	  a = a / 62501; /* 7 */
	  b = 2;
	  rarity = a + b;
	  letter = 9; /* between SS-9 and 2 */
  } else {
	  rarity = 1; /* > 1 million J */
	  letter = 9; /* SS-1 */
  }
  end:
  
  if (obj != NULL) {
	send_to_char(ch, "Obj: %s | Result: %s%d\r\n", obj->short_description, word[letter], rarity);
	extract_obj(obj);
  } else if (is_number(arg1))
    send_to_char(ch, "Value: %d | Result: %s%d\r\n", atoi(arg1), word[letter], rarity);
  else
	send_to_char(ch, "Value: %s%d | Result: %d jenny\r\n", word[letter], rarity, maxCost);
}

ACMD(do_give)
{
  char arg[MAX_STRING_LENGTH];
  int amount, dotmode;
  struct char_data *vict;
  struct obj_data *obj, *next_obj;

  argument = one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Give what to who?\r\n");
  else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg) || !str_cmp("money", arg) || !str_cmp("gold", arg) || !str_cmp("jenny", arg)) {
      one_argument(argument, arg);
      if ((vict = give_find_vict(ch, arg)) != NULL)
	perform_give_gold(ch, vict, amount);
      return;
    } else if (!*arg) /* Give multiple code. */
      send_to_char(ch, "What do you want to give %d of?\r\n", amount);
    else if (!(vict = give_find_vict(ch, argument)))
      return;
    else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
      send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);
    else {
      while (obj && amount--) {
	next_obj = get_obj_in_list_vis(ch, arg, NULL, obj->next_content);
	perform_give(ch, vict, obj);
	obj = next_obj;
      }
    }
  } else {
    char buf1[MAX_INPUT_LENGTH];

    one_argument(argument, buf1);
    if (!(vict = give_find_vict(ch, buf1)))
      return;
    dotmode = find_all_dots(arg);
    if (dotmode == FIND_INDIV) {
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      else
	perform_give(ch, vict, obj);
    } else {
      if (dotmode == FIND_ALLDOT && !*arg) {
	send_to_char(ch, "All of what?\r\n");
	return;
      }
      if (!ch->carrying)
	send_to_char(ch, "You don't seem to be holding anything.\r\n");
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (CAN_SEE_OBJ(ch, obj) &&
	      ((dotmode == FIND_ALL || isname(arg, obj->name))))
	    perform_give(ch, vict, obj);
	}
    }
  }
}

void weight_change_object(struct obj_data *obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if (IN_ROOM(obj) != NOWHERE) {
    GET_OBJ_WEIGHT(obj) += weight;
  } else if ((tmp_ch = obj->carried_by)) {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if ((tmp_obj = obj->in_obj)) {
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    log("SYSERR: Unknown attempt to subtract weight from an object.");
    /* SYSERR_DESC: weight_change_object() outputs this error when weight is
     * attempted to be removed from an object that is not carried or in
     * another object. */
  }
}

void name_from_drinkcon(struct obj_data *obj)
{
  char *new_name, *cur_name, *next;
  const char *liqname;
  int liqlen, cpylen;

  if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  liqname = drinknames[GET_OBJ_VAL(obj, 2)];
  if (!isname(liqname, obj->name)) {
    log("SYSERR: Can't remove liquid '%s' from '%s' (%d) item.", liqname, obj->name, obj->item_number);
    /* SYSERR_DESC: From name_from_drinkcon(), this error comes about if the
     * object noted (by keywords and item vnum) does not contain the liquid
     * string being searched for. */
    return;
  }

  liqlen = strlen(liqname);
  CREATE(new_name, char, strlen(obj->name) - strlen(liqname)); /* +1 for NUL, -1 for space */

  for (cur_name = obj->name; cur_name; cur_name = next) {
    if (*cur_name == ' ')
      cur_name++;

    if ((next = strchr(cur_name, ' ')))
      cpylen = next - cur_name;
    else
      cpylen = strlen(cur_name);

    if (!strn_cmp(cur_name, liqname, liqlen))
      continue;

    if (*new_name)
      strcat(new_name, " "); /* strcat: OK (size precalculated) */
    strncat(new_name, cur_name, cpylen); /* strncat: OK (size precalculated) */
  }

  if (GET_OBJ_RNUM(obj) == NOTHING || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);
  obj->name = new_name;
}

void name_to_drinkcon(struct obj_data *obj, int type)
{
  char *new_name;

  if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
  sprintf(new_name, "%s %s", obj->name, drinknames[type]); /* sprintf: OK */

  if (GET_OBJ_RNUM(obj) == NOTHING || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);

  obj->name = new_name;
}

ACMD(do_drink)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *temp;
  struct affected_type af;
  int amount, weight;
  int on_ground = 0;

  one_argument(argument, arg);

  if (IS_NPC(ch)) /* Cannot use GET_COND() on mobs. */
    return;

  if (!*arg) {
    char buf[MAX_STRING_LENGTH];
    switch (SECT(IN_ROOM(ch))) {
      case SECT_WATER_SWIM:
      case SECT_WATER_NOSWIM:
      case SECT_UNDERWATER:
        if ((GET_COND(ch, HUNGER) > 100) && (GET_COND(ch, THIRST) > 0)) {
          send_to_char(ch, "Your stomach can't contain anymore!\r\n");
        }
        snprintf(buf, sizeof(buf), "$n takes a refreshing drink.");
        act(buf, TRUE, ch, 0, 0, TO_ROOM);
        send_to_char(ch, "You take a refreshing drink.\r\n");
        gain_condition(ch, THIRST, 1);
        if (GET_COND(ch, THIRST) >= 100)
          send_to_char(ch, "You don't feel thirsty any more.\r\n");
	    return;
      default:
    send_to_char(ch, "Drink from what?\r\n");
    return;
    }
  }
  if (!(temp = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
    if (!(temp = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    } else
      on_ground = 1;
  }
  if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
      (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
    send_to_char(ch, "You can't drink from that!\r\n");
    return;
  }
  if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON)) {
    send_to_char(ch, "You have to be holding that to drink from it.\r\n");
    return;
  }
  if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
    /* The pig is drunk */
    send_to_char(ch, "You can't seem to get close enough to your mouth.\r\n");
    act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
/*  if ((GET_COND(ch, HUNGER) > 100) && (GET_COND(ch, THIRST) > 0)) {
    send_to_char(ch, "Your stomach can't contain anymore!\r\n");
    return;
  } */
  if (GET_COND(ch, THIRST) >= 100 && GET_COND(ch, HUNGER) >= 100) {
    send_to_char(ch, "Your stomach can't contain anymore!\r\n");
    return;
  }
  if (GET_OBJ_VAL(temp, 1) == 0) {
    send_to_char(ch, "It is empty.\r\n");
    return;
  }  

  if (!consume_otrigger(temp, ch, OCMD_DRINK))  /* check trigger */
    return;

  if (subcmd == SCMD_DRINK) {
    char buf[MAX_STRING_LENGTH];

    snprintf(buf, sizeof(buf), "$n drinks %s from $p.", drinks[GET_OBJ_VAL(temp, 2)]);
    act(buf, TRUE, ch, temp, 0, TO_ROOM);
    if (GET_COND(ch, THIRST) < 100) 
      send_to_char(ch, "You drink the %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    else {
	  send_to_char(ch, "NO! You bladder is about to explode!\r\n");
	  return;
	}

    if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
      amount = (25 - GET_COND(ch, THIRST)) / drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK];
    else
      amount = rand_number(3, 10);

  } else {
    act("$n sips from $p.", TRUE, ch, temp, 0, TO_ROOM);
    send_to_char(ch, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    amount = 1;
  }

  amount = MIN(amount, GET_OBJ_VAL(temp, 1));

  /* You can't subtract more than the object weighs, unless its unlimited. */
  if (GET_OBJ_VAL(temp, 0) > 0) {
    weight = MIN(amount, GET_OBJ_WEIGHT(temp));
    weight_change_object(temp, -weight); /* Subtract amount */
  }  

  gain_condition(ch, DRUNK,  drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK]  * amount / 4);
  gain_condition(ch, HUNGER,   drink_aff[GET_OBJ_VAL(temp, 2)][HUNGER]   * amount / 4);
  gain_condition(ch, THIRST, drink_aff[GET_OBJ_VAL(temp, 2)][THIRST] * amount / 4);

  if (GET_COND(ch, DRUNK) > 10)
    send_to_char(ch, "You feel drunk.\r\n");

  if (GET_COND(ch, THIRST) >= 100 && GET_COND(ch, HUNGER) >= 100)
    send_to_char(ch, "You are full.\r\n");
  else if (GET_COND(ch, THIRST) >= 100)
    send_to_char(ch, "You don't feel thirsty any more.\r\n");

  if (GET_OBJ_VAL(temp, 3) && GET_LEVEL(ch) < LVL_IMMORT) { /* The crap was poisoned ! */
    send_to_char(ch, "Oops, it tasted rather strange!\r\n");
    act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

    new_affect(&af);
    af.spell = SPELL_POISON;
    af.duration = amount * 3;
    SET_BIT_AR(af.bitvector, AFF_POISON);
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  /* Empty the container (unless unlimited), and no longer poison. */
  if (GET_OBJ_VAL(temp, 0) > 0) {
    GET_OBJ_VAL(temp, 1) -= amount;
    if (!GET_OBJ_VAL(temp, 1)) { /* The last bit */
      name_from_drinkcon(temp);
      GET_OBJ_VAL(temp, 2) = 0;
      GET_OBJ_VAL(temp, 3) = 0;
    }
  }
  return;
}

ACMD(do_eat)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *food;
  struct affected_type af;
  int amount;

  one_argument(argument, arg);

  if (IS_NPC(ch)) /* Cannot use GET_COND() on mobs. */
    return;

  if (!*arg) {
    send_to_char(ch, "Eat what?\r\n");
    return;
  }
  if (!(food = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    return;
  }
  if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE(food) == ITEM_DRINKCON) ||
			       (GET_OBJ_TYPE(food) == ITEM_FOUNTAIN))) {
    do_drink(ch, argument, 0, SCMD_SIP);
    return;
  }
  if ((GET_OBJ_TYPE(food) != ITEM_FOOD) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char(ch, "You can't eat THAT!\r\n");
    return;
  }
  if (GET_COND(ch, HUNGER) >= 100) { /* Stomach full */
    send_to_char(ch, "You are too full to eat more!\r\n");
    return;
  }

  if (!consume_otrigger(food, ch, OCMD_EAT)) /* check trigger */
    return;

  if (subcmd == SCMD_EAT) {
    act("You eat $p.", FALSE, ch, food, 0, TO_CHAR);
    act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
  } else {
    act("You nibble a little bit of $p.", FALSE, ch, food, 0, TO_CHAR);
    act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
  }

  amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL(food, 0) : 1);

  gain_condition(ch, HUNGER, amount);

  if (GET_COND(ch, HUNGER) >= 100)
    send_to_char(ch, "You are full.\r\n");

  if (GET_OBJ_VAL(food, 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    /* The crap was poisoned ! */
    send_to_char(ch, "Oops, that tasted rather strange!\r\n");
    act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

    new_affect(&af);
    af.spell = SPELL_POISON;
    af.duration = amount * 2;
    SET_BIT_AR(af.bitvector, AFF_POISON);
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  if (subcmd == SCMD_EAT)
    extract_obj(food);
  else {
    if (!(--GET_OBJ_VAL(food, 0))) {
      send_to_char(ch, "There's nothing left now.\r\n");
      extract_obj(food);
    }
  }
}

ACMD(do_pour)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  struct obj_data *from_obj = NULL, *to_obj = NULL;
  int amount = 0;

  two_arguments(argument, arg1, arg2);

  if (subcmd == SCMD_POUR) {
    if (!*arg1) { /* No arguments */
      send_to_char(ch, "From what do you want to pour?\r\n");
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
      send_to_char(ch, "You can't pour from that!\r\n");
      return;
    }
  }
  if (subcmd == SCMD_FILL) {
    if (!*arg1) { /* no arguments */
      send_to_char(ch, "What do you want to fill?  And what are you filling it from?\r\n");
      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    }
    if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
      act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!*arg2) { /* no 2nd argument */
      act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg2, NULL, world[IN_ROOM(ch)].contents))) {
      send_to_char(ch, "There doesn't seem to be %s %s here.\r\n", AN(arg2), arg2);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN) {
      act("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
    }
  }
  if (GET_OBJ_VAL(from_obj, 1) == 0) {
    act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_POUR) { /* pour */
    if (!*arg2) {
      send_to_char(ch, "Where do you want it?  Out or in what?\r\n");
      return;
    }
    if (!str_cmp(arg2, "out")) {
      if (GET_OBJ_VAL(from_obj, 0) > 0) {
        act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
        act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

        weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1)); /* Empty */

        name_from_drinkcon(from_obj);
        GET_OBJ_VAL(from_obj, 1) = 0;
        GET_OBJ_VAL(from_obj, 2) = 0;
        GET_OBJ_VAL(from_obj, 3) = 0;
      }
      else
        send_to_char(ch, "You can't possibly pour that container out!\r\n");

      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg2, NULL, ch->carrying))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    }
    if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) &&
	(GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)) {
      send_to_char(ch, "You can't pour anything into that.\r\n");
      return;
    }
  }
  if (to_obj == from_obj) {
    send_to_char(ch, "A most unproductive effort.\r\n");
    return;
  }
  if ((GET_OBJ_VAL(to_obj, 0) < 0) ||
      (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0)))) {
    send_to_char(ch, "There is already another liquid in it!\r\n");
    return;
  }
  if (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0))) {
    send_to_char(ch, "There is no room for more.\r\n");
    return;
  }
  if (subcmd == SCMD_POUR)
    send_to_char(ch, "You pour the %s into the %s.", drinks[GET_OBJ_VAL(from_obj, 2)], arg2);

  if (subcmd == SCMD_FILL) {
    act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
    act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
  }
  /* New alias */
  if (GET_OBJ_VAL(to_obj, 1) == 0)
    name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));

  /* First same type liq. */
  GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

  /* Then how much to pour */
  if (GET_OBJ_VAL(from_obj, 0) > 0) {
    GET_OBJ_VAL(from_obj, 1) -= (amount =
        (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));

    GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

    if (GET_OBJ_VAL(from_obj, 1) < 0) {	/* There was too little */
      GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
      amount += GET_OBJ_VAL(from_obj, 1);
      name_from_drinkcon(from_obj);
      GET_OBJ_VAL(from_obj, 1) = 0;
      GET_OBJ_VAL(from_obj, 2) = 0;
      GET_OBJ_VAL(from_obj, 3) = 0;
    }
  }
  else {
    GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);
    amount = GET_OBJ_VAL(to_obj, 0);
  }
  /* Poisoned? */
  GET_OBJ_VAL(to_obj, 3) = (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3))
;
  /* Weight change, except for unlimited. */
  if (GET_OBJ_VAL(from_obj, 0) > 0) {
    weight_change_object(from_obj, -amount);
  }
  weight_change_object(to_obj, amount); /* Add weight */
}

static void wear_message(struct char_data *ch, struct obj_data *obj, int where)
{
  const char *wear_messages[][2] = {
    {"$n lights $p and holds it.",
    "You light $p and hold it."},

    {"$n slides $p on to $s right ring finger.",
    "You slide $p on to your right ring finger."},

    {"$n slides $p on to $s left ring finger.",
    "You slide $p on to your left ring finger."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p on $s body.",
    "You wear $p on your body."},

    {"$n wears $p on $s head.",
    "You wear $p on your head."},

    {"$n puts $p on $s legs.",
    "You put $p on your legs."},

    {"$n wears $p on $s feet.",
    "You wear $p on your feet."},

    {"$n puts $p on $s hands.",
    "You put $p on your hands."},

    {"$n wears $p on $s arms.",
    "You wear $p on your arms."},

    {"$n straps $p around $s arm as a shield.",
    "You start to use $p as a shield."},

    {"$n wears $p about $s body.",
    "You wear $p around your body."},

    {"$n wears $p around $s waist.",
    "You wear $p around your waist."},

    {"$n puts $p on around $s right wrist.",
    "You put $p on around your right wrist."},

    {"$n puts $p on around $s left wrist.",
    "You put $p on around your left wrist."},

    {"$n wields $p.",
    "You wield $p."},

    {"$n grabs $p.",
    "You grab $p."}
  };

  act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM);
  act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}

static void perform_wear(struct char_data *ch, struct obj_data *obj, int where)
{
  /*
   * ITEM_WEAR_TAKE is used for objects that do not require special bits
   * to be put into that position (e.g. you can hold any object, not just
   * an object with a HOLD bit.)
   */

  int wear_bitvectors[] = {
    ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
    ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
    ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
    ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
    ITEM_WEAR_WIELD, ITEM_WEAR_TAKE
  };

  const char *already_wearing[] = {
    "You're already using a light.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your ring fingers.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You can't wear anything else around your neck.\r\n",
    "You're already wearing something on your body.\r\n",
    "You're already wearing something on your head.\r\n",
    "You're already wearing something on your legs.\r\n",
    "You're already wearing something on your feet.\r\n",
    "You're already wearing something on your hands.\r\n",
    "You're already wearing something on your arms.\r\n",
    "You're already using a shield.\r\n",
    "You're already wearing something about your body.\r\n",
    "You already have something around your waist.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something around both of your wrists.\r\n",
    "You're already wielding a weapon.\r\n",
    "You're already holding something.\r\n"
  };

  /* first, make sure that the wear position is valid. */
  if (!CAN_WEAR(obj, wear_bitvectors[where])) {
    act("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
  if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R))
    if (GET_EQ(ch, where))
      where++;   

  /* See if a trigger disallows it */
  if (!wear_otrigger(obj, ch, where) || (obj->carried_by != ch))
    return;

  /* Remove worn item to equip another */
  if (!IS_NPC(ch) && GET_EQ(ch, where)) {
	perform_remove(ch, where);
    if (GET_EQ(ch, where)) { /* Might be cursed or something, return it */
	  send_to_char(ch, "%s", already_wearing[where]);
	  return;
	}
  } else if (IS_NPC(ch) && GET_EQ(ch, where))
	  return;  

  wear_message(ch, obj, where);
  obj_from_char(obj);
  equip_char(ch, obj, where);
}

int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg)
{
  int where = -1;

  const char *keywords[] = {
    "!RESERVED!",
    "finger",
    "!RESERVED!",
    "neck",
    "!RESERVED!",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "shield",
    "about",
    "waist",
    "wrist",
    "!RESERVED!",
    "!RESERVED!",
    "!RESERVED!",
    "\n"
  };

  if (!arg || !*arg) {
    if (CAN_WEAR(obj, ITEM_WEAR_FINGER))      where = WEAR_FINGER_R;
    if (CAN_WEAR(obj, ITEM_WEAR_NECK))        where = WEAR_NECK_1;
    if (CAN_WEAR(obj, ITEM_WEAR_BODY))        where = WEAR_BODY;
    if (CAN_WEAR(obj, ITEM_WEAR_HEAD))        where = WEAR_HEAD;
    if (CAN_WEAR(obj, ITEM_WEAR_LEGS))        where = WEAR_LEGS;
    if (CAN_WEAR(obj, ITEM_WEAR_FEET))        where = WEAR_FEET;
    if (CAN_WEAR(obj, ITEM_WEAR_HANDS))       where = WEAR_HANDS;
    if (CAN_WEAR(obj, ITEM_WEAR_ARMS))        where = WEAR_ARMS;
    if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))      where = WEAR_SHIELD;
    if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))       where = WEAR_ABOUT;
    if (CAN_WEAR(obj, ITEM_WEAR_WAIST))       where = WEAR_WAIST;
    if (CAN_WEAR(obj, ITEM_WEAR_WRIST))       where = WEAR_WRIST_R;
  } else if ((where = search_block(arg, keywords, FALSE)) < 0)
    send_to_char(ch, "'%s'?  What part of your body is THAT?\r\n", arg);

  return (where);
}

ACMD(do_wear)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj;
  int where, dotmode, items_worn = 0;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char(ch, "Wear what?\r\n");
    return;
  }
  dotmode = find_all_dots(arg1);

  if (*arg2 && (dotmode != FIND_INDIV)) {
    send_to_char(ch, "You can't specify the same body location for more than one item!\r\n");
    return;
  }
  
  if (dotmode == FIND_ALL) {
    for (obj = ch->carrying; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, 0)) >= 0) {
        if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
          send_to_char(ch, "You are not experienced enough to use that.\r\n");
        else {
          items_worn++;
	  perform_wear(ch, obj, where);
	}
      }
    }
    if (!items_worn)
      send_to_char(ch, "You don't seem to have anything wearable.\r\n");
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg1) {
      send_to_char(ch, "Wear all of what?\r\n");
      return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying)))
      send_to_char(ch, "You don't seem to have any %ss.\r\n", arg1);
    else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
      send_to_char(ch, "You are not experienced enough to use that.\r\n");
    else
      while (obj) {
	  next_obj = get_obj_in_list_vis(ch, arg1, NULL, obj->next_content);
      if ((where = find_eq_pos(ch, obj, 0)) >= 0)
        perform_wear(ch, obj, where);
	  else
	    act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
	  obj = next_obj;
      }
  } else {
    if (!(obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying)))
      send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);    
	else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
      send_to_char(ch, "You are not experienced enough to use that.\r\n");
    else {
      if ((where = find_eq_pos(ch, obj, arg2)) >= 0 ) {
		if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R)) {
          if (GET_EQ(ch, where))
			where++;
            perform_wear(ch, obj, where);
		} else        
	      perform_wear(ch, obj, where);		
	} else if (!*arg2) {
	act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR); }
    }
  }
}

ACMD(do_wield)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Wield what?\r\n");
  else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
  else {
    if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
      send_to_char(ch, "You can't wield that.\r\n");
    else if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w && !OBJ_FLAGGED(obj, ITEM_BLESS))
      send_to_char(ch, "It's too heavy for you to use.\r\n");    
    else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
      send_to_char(ch, "You are not experienced enough to use that.\r\n");
    else
	  perform_wear(ch, obj, WEAR_WIELD);
  }
}

ACMD(do_grab)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Hold what?\r\n");
  else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);	
  else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
    send_to_char(ch, "You are not experienced enough to use that.\r\n");
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
	  perform_wear(ch, obj, WEAR_LIGHT);
    else {
	  if (!CAN_WEAR(obj, ITEM_WEAR_HOLD) && GET_OBJ_TYPE(obj) != ITEM_WAND &&
      GET_OBJ_TYPE(obj) != ITEM_STAFF && GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
	  GET_OBJ_TYPE(obj) != ITEM_POTION)
	send_to_char(ch, "You can't hold that.\r\n");
	  else
		perform_wear(ch, obj, WEAR_HOLD);	  
    }
  }
}

static void perform_remove(struct char_data *ch, int pos)
{
  struct obj_data *obj;

  if (!(obj = GET_EQ(ch, pos)))
    log("SYSERR: perform_remove: bad pos %d passed by %s.", pos, GET_NAME(ch));
    /*  This error occurs when perform_remove() is passed a bad 'pos'
     *  (location) to remove an object from. */
  else if (OBJ_FLAGGED(obj, ITEM_NODROP) && !PRF_FLAGGED(ch, PRF_NOHASSLE))
    act("You can't remove $p, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
  else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)&& !PRF_FLAGGED(ch, PRF_NOHASSLE))
    act("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
  else {
    if (!remove_otrigger(obj, ch))
      return;

    obj_to_char(unequip_char(ch, pos), ch);
    act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);
	affect_total(ch);
  }
}

ACMD(do_remove)
{
  char arg[MAX_INPUT_LENGTH];
  int i, dotmode, found;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Remove what?\r\n");
    return;
  }
  dotmode = find_all_dots(arg);

  if (dotmode == FIND_ALL) {
    found = 0;
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
	perform_remove(ch, i);
	found = 1;
      }
    if (!found)
      send_to_char(ch, "You're not using anything.\r\n");
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg)
      send_to_char(ch, "Remove all of what?\r\n");
    else {
      found = 0;
      for (i = 0; i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
	    isname(arg, GET_EQ(ch, i)->name)) {
	  perform_remove(ch, i);
	  found = 1;
	}
      if (!found)
	send_to_char(ch, "You don't seem to be using any %ss.\r\n", arg);
    }
  } else {
    if ((i = get_obj_pos_in_equip_vis(ch, arg, NULL, ch->equipment)) < 0)
      send_to_char(ch, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
    else
      perform_remove(ch, i);
  }
}

ACMD(do_sac)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *j, *jj, *next_thing2;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Sacrifice what?\n\r");
    return;
  }
    
  if (!(j = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents)) && (!(j = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))) {
    send_to_char(ch, "It doesn't seem to be here.\n\r");
    return;
  }

  if (!CAN_WEAR(j, ITEM_WEAR_TAKE)) {
    send_to_char(ch, "You can't sacrifice that!\n\r");
    return;
  }

   act("$n sacrifices $p.", FALSE, ch, j, 0, TO_ROOM);

  switch (rand_number(0, 5)) {
    case 0:
      send_to_char(ch, "You sacrifice %s to the Game Masters.\r\nYou receive one gold coin for your humility.\r\n", GET_OBJ_SHORT(j));
      increase_gold(ch, 1);
    break;
    case 1:
      send_to_char(ch, "You sacrifice %s to the Game Masters.\r\nThe Game Masters ignore your sacrifice.\r\n", GET_OBJ_SHORT(j));
    break;
    case 2:
      send_to_char(ch, "You sacrifice %s to the Game Masters.\r\nThe Game Masters give you %d experience points.\r\n", GET_OBJ_SHORT(j), 1+2*GET_OBJ_LEVEL(j));
      GET_EXP(ch) += (1+2*GET_OBJ_LEVEL(j));
    break;
    case 3:
      send_to_char(ch, "You sacrifice %s to the Game Masters.\r\nYou receive %d experience points.\r\n", GET_OBJ_SHORT(j), 1+GET_OBJ_LEVEL(j));
      GET_EXP(ch) += (1+GET_OBJ_LEVEL(j));
    break;
    case 4:
      send_to_char(ch, "Your sacrifice to the Game Masters is rewarded with %d Jenny.\r\n", 1+GET_OBJ_LEVEL(j));
      increase_gold(ch, (1+GET_OBJ_LEVEL(j)));
    break;
    case 5:
      send_to_char(ch, "Your sacrifice to the Game Masters is rewarded with %d Jenny\r\n", (1+2*GET_OBJ_LEVEL(j)));
      increase_gold(ch, (1+2*GET_OBJ_LEVEL(j)));
    break;
    default:
      send_to_char(ch, "You sacrifice %s to the Game Masters.\r\nYou receive one gold coin for your humility.\r\n",GET_OBJ_SHORT(j));
      increase_gold(ch, 1);
    break;
  }
  for (jj = j->contains; jj; jj = next_thing2) {
    next_thing2 = jj->next_content;       /* Next in inventory */
    obj_from_obj(jj);

    if (j->carried_by)
      obj_to_room(jj, IN_ROOM(j));
    else if (IN_ROOM(j) != NOWHERE)
      obj_to_room(jj, IN_ROOM(j));
    else
      assert(FALSE);
  }
  extract_obj(j);
}

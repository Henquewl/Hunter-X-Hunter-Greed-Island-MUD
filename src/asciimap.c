/**************************************************************************
*  File: asciimap.c                                        Part of tbaMUD *
*  Usage: Generates an ASCII map of the player's surroundings.            *
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
#include "house.h"
#include "constants.h"
#include "dg_scripts.h"
#include "class.h"
#include "shop.h"
#include "asciimap.h"

/******************************************************************************
 * Begin Local (File Scope) Defines and Global Variables
 *****************************************************************************/
/* Do not blindly change these values, as many values cause the map to stop working - backup first */
#define CANVAS_HEIGHT 33
#define CANVAS_WIDTH  101
#define LEGEND_WIDTH  15

#define DEFAULT_MAP_SIZE CONFIG_MAP_SIZE

#define MAX_MAP_SIZE (CANVAS_WIDTH - 1)/4
#define MAX_MAP      CANVAS_WIDTH

#define MAX_MAP_DIR 10
#define MAX_MAP_FOLLOW 10

#define SECT_EMPTY 30 /* anything greater than num sect types */
#define SECT_STRANGE (SECT_EMPTY + 1)
#define SECT_HERE  (SECT_STRANGE + 1)
#define SECT_PLAYER (SECT_HERE + 1)
#define SECT_MOB (SECT_PLAYER + 1)
#define SECT_DARK (SECT_MOB + 1)
#define SECT_SAFE (SECT_DARK + 1)
#define SECT_SHOP (SECT_SAFE + 1)

#define DOOR_NS   -1
#define DOOR_EW   -2
#define DOOR_UP   -3
#define DOOR_DOWN -4
#define DOOR_DIAGNE      -5
#define DOOR_DIAGNW      -6
#define VDOOR_NS         -7
#define VDOOR_EW         -8
#define VDOOR_DIAGNE     -9
#define VDOOR_DIAGNW     -10
#define DOOR_UP_AND_NE   -11
#define DOOR_DOWN_AND_SE -12
#define DOOR_NONE        -13
#define NUM_DOOR_TYPES 13

#define MAP_CIRCLE    0
#define MAP_RECTANGLE 1

#define MAP_NORMAL  0
#define MAP_COMPACT 1
#define MAP_MOBILE 2

static bool show_worldmap(struct char_data *ch);

struct map_info_type
{
  int  sector_type;
  char disp[20];
};

static struct map_info_type door_info[] =
{
  { DOOR_NONE, "   " },
  { DOOR_DOWN_AND_SE, "\tr-\tn\\ " },
  { DOOR_UP_AND_NE,   "\tr+\tn/ " },
  { VDOOR_DIAGNW,     " \tm+\tn " },
  { VDOOR_DIAGNE,     " \tm+\tn "},
  { VDOOR_EW,  " \tm+\tn " },
  { VDOOR_NS,  " \tm+\tn "},
  { DOOR_DIAGNW,      " \\ " },
  { DOOR_DIAGNE,      " / " },
  { DOOR_DOWN, "\tr-\tn  " },
  { DOOR_UP,   "\tr+\tn  " },
  { DOOR_EW,   " - " },
  { DOOR_NS,   " | " }
};

static struct map_info_type compact_door_info[] =
{
  { DOOR_NONE, " " },
  { DOOR_DOWN_AND_SE, "\tR\\\tn" },
  { DOOR_UP_AND_NE,   "\tR/\tn" },
  { VDOOR_DIAGNW,   "\tm+\tn" },
  { VDOOR_DIAGNE,   "\tm+\tn"},
  { VDOOR_EW,  " \tm+\tn " },
  { VDOOR_NS,  " \tm+\tn "},
  { DOOR_DIAGNW,"\\" },
  { DOOR_DIAGNE,"/" },
  { DOOR_DOWN, "\tr-\tn" },
  { DOOR_UP,   "\tr+\tn" },
  { DOOR_EW,   "-" },
  { DOOR_NS,   " | " }
};

/* Add new sector types below for both map_info and world_map_info     */
/* The last 3 MUST remain the same, although the symbol can be changed */
/* New sectors also need to be added to the perform_map function below */
static struct map_info_type map_info[] =
{
  { SECT_INSIDE,       "\tc[\ty.\tc]\tn"  }, /* 0 */
  { SECT_CITY,         "\tc[\tW.\tc]\tn"  },
  { SECT_FIELD,        "\tc[\tg.\tc]\tn"  },
  { SECT_FOREST,       "\tc[\tG.\tc]\tn"  },
  { SECT_HILLS,        "\tc[\ty^\tc]\tn"  },
  { SECT_MOUNTAIN,     "\tc[\tD^\tc]\tn"  }, /* 5 */
  { SECT_WATER_SWIM,   "\tc[\tB~\tc]\tn"  },
  { SECT_WATER_NOSWIM, "\tc[\tb~\tc]\tn"  },
  { SECT_FLYING,       "\tc[\tW~\tc]\tn"  },
  { SECT_UNDERWATER,   "\tc[\tD~\tc]\tn"  },
  { SECT_PORT,         "\tc[\tD@\tc]\tn"  },  /* 10 */
  { SECT_CITYENT,      "\tc[\tg@\tc]\tn"  },
  { SECT_MYSTERY,      "\tc[\ty|\tc]\tn"  },
  { SECT_START,        ""        },
  { SECT_LEAVE,        ""        },
  { -1,                ""        }, /* 15 */
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        }, /* 20 */
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        }, /* 25 */
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        },
  { -1,                ""        },
  { SECT_EMPTY,        "   "     }, /* 30 */
  { SECT_STRANGE,      "\tc[\tn?\tc]\tn" },
  { SECT_HERE,         "\tc[\tC*\tc]\tn"     },
  { SECT_PLAYER,	   "\tc[\tR*\tc]\tn" },
  { SECT_MOB,	   "\tc[\ty*\tc]\tn" },
  { SECT_DARK,	   "\tc[\tD.\tc]\tn" },
  { SECT_SAFE,	   "\tc[\tY.\tc]\tn" },
  { SECT_SHOP,	   "\tc[\tM*\tc]\tn" },
};

static struct map_info_type world_map_info[] =
{
  { SECT_INSIDE,       "\ty."  }, /* 0 */
  { SECT_CITY,         "\tW."  },
  { SECT_FIELD,        "\tg."  },
  { SECT_FOREST,       "\tG."  },
  { SECT_HILLS,        "\ty^"  },
  { SECT_MOUNTAIN,     "\tD^"  }, /* 5 */
  { SECT_WATER_SWIM,   "\tB~"  },
  { SECT_WATER_NOSWIM, "\tb~"  },
  { SECT_FLYING,       "\tW~"  },
  { SECT_UNDERWATER,   "\tD~"  },
  { SECT_PORT,         "\tD@"  },  /* 10 */
  { SECT_CITYENT,      "\tg@"  },
  { SECT_MYSTERY,      "\ty|"  },
  { SECT_START,        ""     },
  { SECT_LEAVE,        ""     },
  { -1,                ""     }, /* 15 */
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     }, /* 20 */
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     }, /* 25 */
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     },
  { -1,                ""     },
  { SECT_EMPTY,        "\tb~"  }, /* 30 */
  { SECT_STRANGE,      "\tn?"  },
  { SECT_HERE,         "\tC*"  },
  { SECT_PLAYER,       "\tR*"  },
  { SECT_MOB, 	       "\ty*"  },
  { SECT_DARK, 	       "\tD."  },
  { SECT_SAFE, 	       "\tY."  },
  { SECT_SHOP, 	       "\tM*"  },
};


static int map[MAX_MAP][MAX_MAP];
static int cnt_nearby;
static int nearby[32];
/*
static int offsets[4][2] ={ {-2, 0},{ 0, 2},{ 2, 0},{ 0, -2} };
static int offsets_worldmap[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0, -1} };
static int door_offsets[6][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0, -1},{ -1, 1},{ 1, 1} };
static int door_marks[6] = { DOOR_NS, DOOR_EW, DOOR_NS, DOOR_EW, DOOR_UP, DOOR_DOWN };
*/
static int offsets[10][2] ={ {-2, 0},{ 0, 2},{ 2, 0},{ 0, -2},{0, 0},{ 0, 0},{ -2, -2},{ -2, 2},{2, 2},{ 2, -2} };
static int offsets_worldmap[10][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0, -1},{0, 0},{ 0, 0},{ -1, -1},{ -1, 1},{1, 1},{ 1, -1} };
static int door_offsets[10][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0, -1},{ -1, 1},{ 1, 1},{ -1, -1},{ -1, 1},{ 1, 1},{ 1, -1} };
static int door_marks[10] = { DOOR_NS, DOOR_EW, DOOR_NS, DOOR_EW, DOOR_UP, DOOR_DOWN, DOOR_DIAGNW, DOOR_DIAGNE, DOOR_DIAGNW, DOOR_DIAGNE};
static int vdoor_marks[4] = { VDOOR_NS, VDOOR_EW, VDOOR_NS, VDOOR_EW };
/******************************************************************************
 * End Local (File Scope) Defines and Global Variables
 *****************************************************************************/

/******************************************************************************
 * Begin Local (File Scope) Function Prototypes
 *****************************************************************************/
static void player_sense(struct char_data *ch);
static void MapArea(room_rnum room, struct char_data *ch, int x, int y, int min, int max, sh_int xpos, sh_int ypos, bool worldmap);
static char *StringMap(int centre, int size);
static char *WorldMap(int centre, int size, int mapshape, int maptype );
static char *CompactStringMap(int centre, int size);
static void perform_map( struct char_data *ch, char *argument, bool worldmap );
/******************************************************************************
 * End Local (File Scope) Function Prototypes
 *****************************************************************************/


bool can_see_map(struct char_data *ch) {
  /* Is the map funcionality disabled? */
  if (CONFIG_MAP == MAP_OFF)
    return FALSE;
  else if ((CONFIG_MAP == MAP_IMM_ONLY) && (GET_LEVEL(ch) < LVL_IMMORT))
    return FALSE;

  return TRUE;
}

static void player_sense(struct char_data *ch)
{
  struct char_data *i;
  struct descriptor_data *d;
  int cnt;
  bool found = FALSE;

  if (!ch)
    return;

    for (d = descriptor_list; d; d = d->next) {	  
      if (STATE(d) != CON_PLAYING || d->character == ch)
		continue;
      if ((i = (d->original ? d->original : d->character)) == NULL)
		continue;
      if (IN_ROOM(i) == NOWHERE || !CAN_SEE(ch, i) || (IS_DARK(IN_ROOM(i)) && !CAN_SEE_IN_DARK(ch)) || (GET_HIT(i) <= 5 && !IS_NPC(i)))
		continue;
	  if (GET_SKILL(ch, SKILL_ANALYSIS) < 100) {
		found = TRUE;
	    goto end;
	  }
	  for (cnt = 0; cnt < cnt_nearby; cnt++) {
		if (!found && nearby[cnt] == GET_IDNUM(i)) {
		  send_to_char(ch, "\tc[ Players Nearby: %s%s ", CLASS_COLOR(i), GET_NAME(i));
		  found = TRUE;
	    } else if (found && nearby[cnt] == GET_IDNUM(i))
	      send_to_char(ch, "\tc| %s%s ", CLASS_COLOR(i), GET_NAME(i));	    
	  }	  
    }
  
  if (found)
	send_to_char(ch, "\tc]\tn\r\n");
  end:
  if (GET_SKILL(ch, SKILL_ANALYSIS) < 100 && found)
	send_to_char(ch, "\r\nYou feel the presence of players nearby in this area.\r\n");
  cnt_nearby = 0;
  return;
}

/* MapArea function - create the actual map */
static void MapArea(room_rnum room, struct char_data *ch, int x, int y, int min, int max, sh_int xpos, sh_int ypos, bool worldmap)
{
  room_rnum prospect_room;
  struct room_direction_data *pexit;
  struct char_data *player;
  int door, ew_size=0, ns_size=0, x_exit_pos=0, y_exit_pos=0;
  sh_int prospect_xpos, prospect_ypos;

  if (map[x][y] < 0)
    return; /* this is a door */

  /* Check if is dark or has players and marks the room as visited */
  if ((IS_DARK(room) && !CAN_SEE_IN_DARK(ch)) || AFF_FLAGGED(ch, AFF_BLIND))
	map[x][y] = SECT_DARK;
  else if (room == IN_ROOM(ch))
    map[x][y] = SECT_HERE;
  else if (ROOM_FLAGGED(room, ROOM_PEACEFUL))
	map[x][y] = SECT_SAFE;
  else if (ROOM_FLAGGED(room, ROOM_CRATER))
	map[x][y] = SECT_PORT;
  else
    map[x][y] = SECT(room);	 
  
  if (world[room].people) {
	  for (player = world[room].people; player; player = player->next_in_room) {
	    if (!IS_NPC(player) && player != ch && CAN_SEE(ch, player) && GET_HIT(player) > 5) { 
		  if (map[x][y] == SECT(room) || map[x][y] == SECT_SAFE)
		    map[x][y] = SECT_PLAYER;
		  if (!cnt_nearby)
		    cnt_nearby = 0;
		  nearby[cnt_nearby] = GET_IDNUM(player);
		  cnt_nearby++;
		  break;
		} else if (!worldmap && IS_NPC(player) && CAN_SEE(ch, player)) {
		  if (GET_MOB_SPEC(player))
		    map[x][y] = SECT_SHOP;	
	      else
		    map[x][y] = SECT_MOB;
		}
	  }
    }
  
  if ( (x < min) || ( y < min) || ( x > max ) || ( y > max) ) return;

  /* Check for exits */
  for ( door = 0; door < MAX_MAP_DIR; door++ ) {

    if( door < MAX_MAP_FOLLOW &&
        xpos+door_offsets[door][0] >= 0 &&
        xpos+door_offsets[door][0] <= ns_size &&
        ypos+door_offsets[door][1] >= 0 &&
        ypos+door_offsets[door][1] <= ew_size)
    { /* Virtual exit */

      map[x+door_offsets[door][0]][y+door_offsets[door][1]] = vdoor_marks[door] ;
      if (map[x+offsets[door][0]][y+offsets[door][1]] == SECT_EMPTY )
        MapArea(room,ch,x + offsets[door][0], y + offsets[door][1], min, max, xpos+door_offsets[door][0], ypos+door_offsets[door][1], worldmap);
      continue;
    }

    if ( (pexit = world[room].dir_option[door]) != NULL  &&
         (pexit->to_room > 0 ) && (pexit->to_room != NOWHERE) &&
         (!IS_SET(pexit->exit_info, EX_CLOSED)) &&
         (!IS_SET(pexit->exit_info, EX_HIDDEN) || PRF_FLAGGED(ch, PRF_HOLYLIGHT) || (AFF_FLAGGED(ch, AFF_INFRAVISION) && AFF_FLAGGED(ch, AFF_DETECT_INVIS))) )
    { /* A real exit */
	
	if (ROOM_FLAGGED(pexit->to_room, ROOM_DARK) && !CAN_SEE_IN_DARK(ch))
	  continue;

      /* But is the door here... */
      switch (door) {
      case NORTH:
        if(xpos > 0 || ypos!=y_exit_pos) continue;
        break;
      case SOUTH:
        if(xpos < ns_size || ypos!=y_exit_pos) continue;
        break;
      case EAST:
        if(ypos < ew_size || xpos!=x_exit_pos) continue;
        break;
      case WEST:
        if(ypos > 0 || xpos!=x_exit_pos) continue;
        break;
      case NORTHWEST:
        if(xpos > 0 || ypos!=y_exit_pos || ypos > 0 || xpos!=x_exit_pos) continue;
        break;
      case NORTHEAST:
        if(xpos > 0 || ypos!=y_exit_pos || ypos < ew_size || xpos!=x_exit_pos) continue;
        break;
      case SOUTHEAST:
        if(xpos < ns_size || ypos!=y_exit_pos || ypos < ew_size || xpos!=x_exit_pos) continue;
        break;
      case SOUTHWEST:
        if(xpos < ns_size || ypos!=y_exit_pos || ypos > 0 || xpos!=x_exit_pos) continue;
        break;
      }


 /*     if ( (x < min) || ( y < min) || ( x > max ) || ( y > max) ) return;*/
      prospect_room = pexit->to_room;

        /* one way into area OR maze */
        if ( world[prospect_room].dir_option[rev_dir[door]] &&
             world[prospect_room].dir_option[rev_dir[door]]->to_room != room) {
          map[x][y] = SECT_STRANGE;
        return;
        }

      if(!worldmap) {
        int ddx = x + door_offsets[door][0];
        int ddy = y + door_offsets[door][1];
        if (ddx >= 0 && ddx < MAX_MAP && ddy >= 0 && ddy < MAX_MAP) {
          if ((map[ddx][ddy] == DOOR_NONE) || (map[ddx][ddy] == SECT_EMPTY)) {
            map[ddx][ddy] = door_marks[door];
          } else {
            if ( ((door == NORTHEAST) && (map[ddx][ddy] == DOOR_UP)) ||
                 ((door == UP)        && (map[ddx][ddy] == DOOR_DIAGNE)) ) {
              map[ddx][ddy] = DOOR_UP_AND_NE;
            }
            else if ( ((door == SOUTHEAST) && (map[ddx][ddy] == DOOR_DOWN)) ||
                      ((door == DOWN)      && (map[ddx][ddy] == DOOR_DIAGNW)) ) {
              map[ddx][ddy] = DOOR_DOWN_AND_SE;
            }
          }
        }
      }

      prospect_xpos = prospect_ypos = 0;
      switch (door) {
      case NORTH:
        prospect_xpos = ns_size;
      case SOUTH:
        prospect_ypos = world[prospect_room].dir_option[rev_dir[door]] ? y_exit_pos : ew_size/2;
      break;
      case WEST:
        prospect_ypos = ew_size;
      case EAST:
        prospect_xpos = world[prospect_room].dir_option[rev_dir[door]] ? x_exit_pos : ns_size/2;
        break;
      case NORTHEAST:
      case NORTHWEST:
      case SOUTHEAST:
      case SOUTHWEST:
        prospect_xpos = world[prospect_room].dir_option[rev_dir[door]] ? x_exit_pos : ns_size/2;
        prospect_ypos = world[prospect_room].dir_option[rev_dir[door]] ? y_exit_pos : ew_size/2;
        break;
      }

      if(worldmap) {
        int nx = x + offsets_worldmap[door][0];
        int ny = y + offsets_worldmap[door][1];
        if ( door < MAX_MAP_FOLLOW && nx >= 0 && nx < MAX_MAP && ny >= 0 && ny < MAX_MAP && map[nx][ny] == SECT_EMPTY )
          MapArea(pexit->to_room,ch,nx, ny, min, max, prospect_xpos, prospect_ypos, worldmap);
      } else {
        int nx = x + offsets[door][0];
        int ny = y + offsets[door][1];
        if ( door < MAX_MAP_FOLLOW && nx >= 0 && nx < MAX_MAP && ny >= 0 && ny < MAX_MAP && map[nx][ny] == SECT_EMPTY )
          MapArea(pexit->to_room,ch,nx, ny, min, max, prospect_xpos, prospect_ypos, worldmap);
      }
    } /* end if exit there */
  }
  return;
}

/* Returns a string representation of the map */
static char *StringMap(int centre, int size)
{
  static char strmap[MAX_MAP*MAX_MAP*11 + MAX_MAP*2 + 1];
  char *mp = strmap;
  char *tmp;
  int x, y;

  /* every row */
  for (x = centre - CANVAS_HEIGHT/2; x <= centre + CANVAS_HEIGHT/2; x++) {
    /* every column */
    for (y = centre - CANVAS_WIDTH/6; y <= centre + CANVAS_WIDTH/6; y++) {
      if (abs(centre - x)<=size && abs(centre-y)<=size)
        tmp = (map[x][y]<0) ? \
       door_info[NUM_DOOR_TYPES + map[x][y]].disp : \
       map_info[map[x][y]].disp ;
      else
 tmp = map_info[SECT_EMPTY].disp;
      strcpy(mp, tmp);
      mp += strlen(tmp);
    }
    strcpy(mp, "\r\n");
    mp+=2;
  }
  *mp='\0';
  return strmap;
}

static char *WorldMap(int centre, int size, int mapshape, int maptype )
{
  static char strmap[MAX_MAP*MAX_MAP*4 + MAX_MAP*2 + 1], buf[MAX_STRING_LENGTH];
  char *mp = strmap;
  int x, y/*, i, u = 1*/;
  int xmin, xmax, ymin, ymax;

  switch(maptype) {
    case MAP_COMPACT:
      xmin = centre - size;
      xmax = centre + size;
      ymin = centre - 2*size;
      ymax = centre + 2*size;
      break;
	case MAP_MOBILE:
	  xmin = centre - CANVAS_HEIGHT/2;
      xmax = centre + CANVAS_HEIGHT/2;
      ymin = centre - CANVAS_HEIGHT/2;
      ymax = centre + CANVAS_HEIGHT/2;
	  break;
    default:
      /* MAP_NORMAL: hug the content (size vertically, 2*size horizontally)
         instead of padding to the full canvas, so the [ ] borders sit right
         next to the map edge with no blank margin. Clip to the canvas max. */
      {
        int vr = size, hr = 2 * size;
        if (vr > CANVAS_HEIGHT/2) vr = CANVAS_HEIGHT/2;
        if (hr > CANVAS_WIDTH/2)  hr = CANVAS_WIDTH/2;
        xmin = centre - vr;
        xmax = centre + vr;
        ymin = centre - hr;
        ymax = centre + hr;
      }
  }

  /* every row */
  /* for (x = centre - size; x <= centre + size; x++) { */
  for (x = xmin; x <= xmax; x++) {
    /* every column */
    /* for (y = centre - (2*size) ; y <= centre + (2*size) ; y++) {  */
    for (y = (ymin - 1); y <= (ymax + 1) ; y++) {
		
	if (y == (ymin - 1))
	  strcpy(mp++, "[");
    else if (y == (ymax + 1)) {
	  sprintf(buf, "\tn]");
	  strcpy(mp, buf);
	  mp += strlen(buf);
	} else if(((mapshape == MAP_RECTANGLE && abs(centre - y) <= size*2  && abs(centre - x) <= size ) ||
   ((mapshape == MAP_CIRCLE) && (centre-x)*(centre-x) + (centre-y)*(centre-y)/4 <= (size * size + 1))) && (y <= ymax)) {
        strcpy(mp, world_map_info[map[x][y]].disp);
        mp += strlen(world_map_info[map[x][y]].disp);
      } else {
/*		if (y == (ymax + 1))
		  strcpy(mp++, "[");
		else if (y == (ymax + 2)) {
		  strcpy(mp, "\tn\r\n");
		  if (i >= 0)
		    sprintf(buf, "%d", i);		    
		  else {
			sprintf(buf, "%d", u);		    
		    u++;
		  }
		  i--;
		  strcpy(mp++, buf);
		} else if (y == (ymax + 3))
		  strcpy(mp++, "]");
	    else
*/		  strcpy(mp++, " ");
      }	  
		
    }	
    strcpy(mp, "\tn\r\n");
    mp+=4;
  }  
  *mp='\0';
  return strmap;
}

static char *CompactStringMap(int centre, int size)
{
  static char strmap[MAX_MAP*MAX_MAP*12 + MAX_MAP*2 + 1];
  char *mp = strmap;
  int x, y;

  /* every row */
  for (x = centre - size; x <= centre + size; x++) {
    /* every column */
    for (y = centre - size; y <= centre + size; y++) {
      strcpy(mp, (map[x][y]<0) ? \
       compact_door_info[NUM_DOOR_TYPES + map[x][y]].disp : \
       map_info[map[x][y]].disp);
      mp += strlen((map[x][y]<0) ? \
       compact_door_info[NUM_DOOR_TYPES + map[x][y]].disp : \
       map_info[map[x][y]].disp);
    }
    strcpy(mp, "\r\n");
    mp+=2;
  }
  *mp='\0';
  return strmap;
}

/* Display a nicely formatted map with a legend */
static void perform_map( struct char_data *ch, char *argument, bool worldmap )
{
  int size = DEFAULT_MAP_SIZE;
  int centre, x, y, min, max;
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int count = 0;
  int ew_size=0, ns_size=0;
  int mapshape = MAP_CIRCLE;

  one_argument(argument, arg);
  
  size = 8 + GET_LEVEL(ch) / 6;

  if (*arg)
  {
    if (is_abbrev(arg, "normal")) worldmap=FALSE;
    else if (is_abbrev(arg, "world")) worldmap=TRUE;
    else {
      send_to_char(ch, "Usage: \tymap [ normal | world ]\tn");
      return;
    }
  }
  
  if (AFF_FLAGGED(ch, AFF_FLYING))
    mapshape = MAP_RECTANGLE;

  if(size<0)
    size = -size;    
  
  size = URANGE(1,size,MAX_MAP_SIZE);

  centre = MAX_MAP/2;

  if(worldmap) {
    min = centre - 2*size;
    max = centre + 2*size;
  } else {
    min = centre - size;
    max = centre + size;
  }

  /* Blank the map */
  for (x = 0; x < MAX_MAP; ++x)
      for (y = 0; y < MAX_MAP; ++y)
           map[x][y]= (!(y%2) && !worldmap) ? DOOR_NONE : SECT_EMPTY;

  /* starts the mapping with the centre room */
  
    MapArea(IN_ROOM(ch), ch, centre, centre, min, max, ns_size/2, ew_size/2, worldmap);

  /* marks the center, where ch is */
  map[centre][centre] = SECT_HERE;

  /* Feel free to put your own MUD name or header in here */
  send_to_char(ch, " \tY-\tyGreed Island Map System\tY-\tn\r\n"
                   "\tD  .-.__--.,--.__.-.\tn\r\n" );

  /* Print the map itself */
  if (worldmap) {
    if (GET_SCREEN_WIDTH(ch) == 40 || PRF_FLAGGED(ch, PRF_COMPACT))
      send_to_char(ch, "%s", WorldMap(centre, size, mapshape, MAP_MOBILE));
    else
      send_to_char(ch, "%s", WorldMap(centre, size, mapshape, MAP_NORMAL));
  } else {
    send_to_char(ch, "%s", StringMap(centre, size));
  }

  send_to_char(ch, "\tD  `.-.__--.,-.__.-.-'\tn\r\n");

  /* Tabular legend below the map */
  {
    const char *ldisp[24];
    const char *llabel[24];
    int ln = 0, i, plen, cols;
    const char *p;

    if (worldmap) {
      ldisp[ln]=world_map_info[SECT_HERE].disp;        llabel[ln++]="You";
      ldisp[ln]=world_map_info[SECT_PLAYER].disp;      llabel[ln++]="Player";
      ldisp[ln]=world_map_info[SECT_SAFE].disp;        llabel[ln++]="Safe";
      ldisp[ln]=world_map_info[SECT_DARK].disp;        llabel[ln++]="Dark";
      ldisp[ln]=world_map_info[SECT_EMPTY].disp;       llabel[ln++]="Wilderness";
      ldisp[ln]=world_map_info[SECT_STRANGE].disp;     llabel[ln++]="Displaced";
      ldisp[ln]=world_map_info[SECT_INSIDE].disp;      llabel[ln++]="Place";
      ldisp[ln]=world_map_info[SECT_CITY].disp;        llabel[ln++]="City";
      ldisp[ln]=world_map_info[SECT_FIELD].disp;       llabel[ln++]="Field";
      ldisp[ln]=world_map_info[SECT_FOREST].disp;      llabel[ln++]="Forest";
      ldisp[ln]=world_map_info[SECT_HILLS].disp;       llabel[ln++]="Hills";
      ldisp[ln]=world_map_info[SECT_MOUNTAIN].disp;    llabel[ln++]="Mountain";
      ldisp[ln]=world_map_info[SECT_WATER_SWIM].disp;  llabel[ln++]="Shallow";
      ldisp[ln]=world_map_info[SECT_WATER_NOSWIM].disp;llabel[ln++]="Deep";
      ldisp[ln]=world_map_info[SECT_FLYING].disp;      llabel[ln++]="Flying";
      ldisp[ln]=world_map_info[SECT_UNDERWATER].disp;  llabel[ln++]="Underwater";
      ldisp[ln]=world_map_info[SECT_PORT].disp;         llabel[ln++]="Port";
      ldisp[ln]=world_map_info[SECT_CITYENT].disp;     llabel[ln++]="City";
      ldisp[ln]=world_map_info[SECT_MYSTERY].disp;     llabel[ln++]="Entrance";
    } else {
      ldisp[ln]=door_info[NUM_DOOR_TYPES + DOOR_UP].disp;   llabel[ln++]="Up";
      ldisp[ln]=door_info[NUM_DOOR_TYPES + DOOR_DOWN].disp; llabel[ln++]="Down";
      ldisp[ln]=map_info[SECT_HERE].disp;        llabel[ln++]="You";
      ldisp[ln]=map_info[SECT_PLAYER].disp;      llabel[ln++]="Player";
      ldisp[ln]=map_info[SECT_MOB].disp;         llabel[ln++]="Mob";
      ldisp[ln]=map_info[SECT_SHOP].disp;        llabel[ln++]="NPC";
      ldisp[ln]=map_info[SECT_SAFE].disp;        llabel[ln++]="Safe";
      ldisp[ln]=map_info[SECT_DARK].disp;        llabel[ln++]="Dark";
      ldisp[ln]=map_info[SECT_STRANGE].disp;     llabel[ln++]="Displaced";
      ldisp[ln]=map_info[SECT_INSIDE].disp;      llabel[ln++]="Place";
      ldisp[ln]=map_info[SECT_CITY].disp;        llabel[ln++]="Road";
      ldisp[ln]=map_info[SECT_FIELD].disp;       llabel[ln++]="Field";
      ldisp[ln]=map_info[SECT_FOREST].disp;      llabel[ln++]="Forest";
      ldisp[ln]=map_info[SECT_HILLS].disp;       llabel[ln++]="Hills";
      ldisp[ln]=map_info[SECT_MOUNTAIN].disp;    llabel[ln++]="Mountain";
      ldisp[ln]=map_info[SECT_WATER_SWIM].disp;  llabel[ln++]="Shallow";
      ldisp[ln]=map_info[SECT_WATER_NOSWIM].disp;llabel[ln++]="Deep";
      ldisp[ln]=map_info[SECT_FLYING].disp;      llabel[ln++]="Flying";
      ldisp[ln]=map_info[SECT_UNDERWATER].disp;  llabel[ln++]="Underwater";
      ldisp[ln]=map_info[SECT_PORT].disp;         llabel[ln++]="Port";
      ldisp[ln]=map_info[SECT_CITYENT].disp;     llabel[ln++]="City";
      ldisp[ln]=map_info[SECT_MYSTERY].disp;     llabel[ln++]="Entrance";
    }

    cols = (GET_SCREEN_WIDTH(ch) >= 80) ? 4 : 2;

    for (i = 0; i < ln; i++) {
      send_to_char(ch, "\tn%s %s", ldisp[i], llabel[i]);
      /* Pad to a fixed column width using the printable length (skip color codes) */
      plen = 0;
      for (p = ldisp[i]; *p; p++) {
        if (*p == '\t' && *(p + 1)) { p++; continue; }
        plen++;
      }
      plen += 1 + strlen(llabel[i]);
      if ((i % cols) != (cols - 1))
        for (; plen < 18; plen++)
          send_to_char(ch, " ");
      if ((i % cols) == (cols - 1))
        send_to_char(ch, "\r\n");
    }
    if ((ln % cols) != 0)
      send_to_char(ch, "\r\n");
  }
  return;
}

/* Display a string with the map beside it */
void str_and_map(char *str, struct char_data *ch, room_vnum target_room, bool onlysense) {
  int size, centre, x, y, min, max, char_size;
  int ew_size=0, ns_size=0;
  bool worldmap;
  int mapshape;

  /* Check MUDs map config options - if disabled, just show room decsription */
  if (!can_see_map(ch) && !onlysense) {
    send_to_char(ch, "%s", strfrmt(str, GET_SCREEN_WIDTH(ch), 1, FALSE, FALSE, FALSE));
    return;
  }

  worldmap = show_worldmap(ch);
  
  if (onlysense && *str)
	*str = '\0';

  if (!PRF_FLAGGED(ch, PRF_AUTOMAP) && !PRF_FLAGGED(ch, PRF_BRIEF)) {
    send_to_char(ch, "%s", strfrmt(str, GET_SCREEN_WIDTH(ch), 1, FALSE, FALSE, FALSE));
	if (!onlysense)
      return;
  }  

  size = URANGE(1, 4 + GET_LEVEL(ch) / 10, MAX_MAP_SIZE);
  /* The compact minimap is rendered beside the room description, so the map
     column (char_size up to 4*size+7) plus a minimal text column must fit the
     player's screen width. Without this cap, GET_SCREEN_WIDTH - char_size goes
     negative and strfrmt's memset() crashes. */
  {
    int fit = (GET_SCREEN_WIDTH(ch) - 17) / 4;
    size = URANGE(1, size, fit < 1 ? 1 : fit);
  }
  centre = MAX_MAP/2;
  min = centre - 2*size;
  max = centre + 2*size;

  for (x = 0; x < MAX_MAP; ++x)
    for (y = 0; y < MAX_MAP; ++y)
      map[x][y]= (!(y%2) && !worldmap) ? DOOR_NONE : SECT_EMPTY;

  /* starts the mapping with the center room */
MapArea(target_room, ch, centre, centre, min, max, ns_size/2, ew_size/2, worldmap ); 
  map[centre][centre] = SECT_HERE;

  /* char_size = rooms + doors + padding */
  if(worldmap)
    char_size = size * 4 + 5;
  else
    char_size = 3*(size+1) + (size) + 4;
  
  if (!onlysense) {
  if(worldmap) {
	if ((weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK) && !AFF_FLAGGED(ch, AFF_FLYING))
	  mapshape = MAP_CIRCLE;
    else
	  mapshape = MAP_RECTANGLE;

	if (strlen(str) < 12)
	  send_to_char(ch, "%s \tn", WorldMap(centre, size, mapshape, MAP_COMPACT));
	else
	send_to_char(ch, "%s", strpaste(WorldMap(centre, size, mapshape, MAP_COMPACT), strfrmt(str, GET_SCREEN_WIDTH(ch) - char_size, size*2 + 1, FALSE, TRUE, TRUE), " \tn"));	
//      send_to_char(ch, "%s", strpaste(strfrmt(str, GET_SCREEN_WIDTH(ch) - char_size, size*2 + 1, FALSE, TRUE, TRUE), WorldMap(centre, size, mapshape, MAP_COMPACT), " \tn"));
	
  } else
    send_to_char(ch, "%s", strpaste(CompactStringMap(centre, size), strfrmt(str, GET_SCREEN_WIDTH(ch) - char_size, size*2 + 1, FALSE, TRUE, TRUE), " \tn"));
  }

  if (GET_SKILL(ch, SKILL_ANALYSIS) && cnt_nearby)
    player_sense(ch);  
}

static bool show_worldmap(struct char_data *ch) {
  room_rnum rm = IN_ROOM(ch);
  zone_rnum zn = GET_ROOM_ZONE(rm);

  if (ZONE_FLAGGED(zn, ZONE_WORLDMAP) && !ROOM_FLAGGED(rm, ROOM_WORLDMAP)) return TRUE;
  else if (ROOM_FLAGGED(rm, ROOM_WORLDMAP) && !ZONE_FLAGGED(zn, ZONE_WORLDMAP)) return TRUE;
  else return FALSE;
}

ACMD(do_map) {
  if (IS_NPC(ch) || !can_see_map(ch)) {
    send_to_char(ch, "Sorry, the map is disabled!\r\n");
    return;
  }
  if (AFF_FLAGGED(ch, AFF_BLIND) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char(ch, "You can't see the map while blind!\r\n");
    return;
  }
  perform_map(ch, argument, show_worldmap(ch));
  
  if (GET_SKILL(ch, SKILL_ANALYSIS) && cnt_nearby)
    player_sense(ch);
  
}

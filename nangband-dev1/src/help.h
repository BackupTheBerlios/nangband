/* ------------------------------------------------------------ ajps ---
 * This file contains #defined macros, function prototypes and data
 * structure definitions for use with the help.c functions.  It is only
 * needed internally by that file, at present at least.
 *
 * Copyright (c) 2002 Antony Sidwell
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * This source file is also available under the GNU GENERAL PUBLIC LICENCE.
 * --------------------------------------------------------------------- */

#ifndef INCLUDED_HELP_H
#define INCLUDED_HELP_H

/*
 * Holds a history stack entry -
 * the file and the line we were on when we left the file.
 */
typedef struct {
	cptr file;
	int line;
	} history_blk;

/* Holds all information we need to store about a link */
typedef struct {
	int type;
	void *next;
	char key;
	union {
		int status;
		int block;
	} data;
	int start_line;
	int end_line;
	} link_blk;

/*
 * Everything we need to know about the
 * plotting style at any time.
 */
typedef struct {
	int align;
	int link;
	char underline;
	char colour;
	char em;
	char strong;
	} plot_style;

typedef struct {
	int align;
	int type;
	byte flag;
	byte width;
	byte lmargin;
	byte rmargin;
	} block_style;

typedef struct {
	int width;
	int type;
	int item_no;
	} list_blk;

typedef struct {
	char *start;
	int  end_line;
	} block_blk;

typedef int *table_blk;

/* Holds a ton of information about a parsed help file */
typedef struct {
	/* Pointer to the display block */
	char *display_block;

	/* Pointer to the links block */
	char *links_block;

	/* Pointer to the information about the redraw blocks */
	block_blk *block_data;

	/* The number of redraw blocks */
	int no_blks;

	/* The number of lines in the file */
	int total_lines;

	} helpfile_blk;


/* Modes of operation for the system */
#define MODE_NAV 0
#define MODE_LINK 1

/* List flags */
#define LIST_COMPACT 1
#define LIST_NOCOMPACT 0

/* List types */
#define LIST_TYPE_BULLET 0
#define LIST_TYPE_i 1
#define LIST_TYPE_I 2
#define LIST_TYPE_a 3
#define LIST_TYPE_A 4
#define LIST_TYPE_1 5
#define LIST_BULLET_POINT '-'

/* Redraw Codes */
#define REDRAW_CODE_BLOCKSTART 3
#define REDRAW_CODE_BLOCKEND 0
#define REDRAW_CODE_COLOUR 1
#define REDRAW_CODE_UNDERLINE 6
#define REDRAW_CODE_MEGAUNDERLINE 7
#define REDRAW_CODE_LINKPOINT 2
#define REDRAW_CODE_LINKREF 4
#define REDRAW_CODE_HRULE 8
#define REDRAW_CODE_EMPH 11
#define REDRAW_CODE_STRONG 12

/* Link status codes */
#define LINK_UNSELECTED 1
#define LINK_SELECTED 2

/* Alignment codes */
#define ALIGN_LEFT 0
#define ALIGN_RIGHT 1
#define ALIGN_CENTRE 2

/* Blocktypes */
#define BLOCKTYPE_TEXT 0
#define BLOCKTYPE_TABLE 1
#define BLOCKTYPE_LIST 2

/* Table flags */
#define FLAG_TABLE_NORULES 0
#define FLAG_TABLE_HRULES 1
#define FLAG_TABLE_VRULES 2
#define FLAG_TABLE_ALLRULES 3

/* Reasonable values for buffers and things */
#define REASONABLE_LINELENGTH 120
#define MAX_CHARENTITY_LENGTH 20
#define HISTORY_DEPTH 10
#define MAX_LINKPOINT_LENGTH 49
#define XMLHELP_FILENAME_LENGTH 100
#define PAGE_BORDER 1

/* Character options, for various purposes */
#define HRULE_CHARACTER '-'
#define VRULE_CHARACTER '|'
#define MEGAUNDERLINE_CHARACTER '='
#define UNDERLINE_CHARACTER '-'

/* Keyset choices for navigation, etc. */
#define CHAR_LINK_ESCAPE 'L'
#define CHAR_UP_LINE ';'
#define CHAR_DOWN_LINE '.'
#define CHAR_UP_PAGE ':'
#define CHAR_DOWN_PAGE '>'
#define CHAR_LINK_DOWN ','
#define CHAR_LINK_UP 'l'
#define CHAR_SELECT '\r'
#define CHAR_RELOAD 'R'
#define CHAR_FORWARD '@'
#define CHAR_BACK CHAR_ESCAPE

/* The character which exits the help system */
#define CHAR_QUIT '?'

/* Return codes for the plot function */
#define PLOT_FAILED -1
#define PLOT_FINISHED -2

/* alignment and colouring options */
#define LINK_COLOUR_SELECTED TERM_L_GREEN
#define LINK_COLOUR_UNSELECTED TERM_L_WHITE

#define h1_ALIGN ALIGN_CENTRE
#define h2_ALIGN ALIGN_LEFT
#define h3_ALIGN ALIGN_LEFT

#define h1_COLOUR TERM_ORANGE
#define h2_COLOUR TERM_ORANGE
#define h3_COLOUR TERM_ORANGE

#define EMPH_COLOUR TERM_L_UMBER
#define STRONG_COLOUR TERM_RED

#define LINK_TYPE_END 0
#define LINK_TYPE_POINT 1
#define LINK_TYPE_REF 2

/*
 * These are deliberately numbered higher that LINK_TYPE_REF, so that
 * we can treat them as LINK_REFs using >=.  None "action" types should
 * be inserted "below" LINK_TYPE_REF.
 */
#define LINK_TYPE_BACK 3
#define LINK_TYPE_EXIT 4
#define LINK_TYPE_FORWARD 5
#define LINK_TYPE_RETURN 6

/* Function-like macros */
#define FILE_TITLE(block) (char *)(block + sizeof(int))
#define DISPLAY_BLOCK(block) (block + WORDALIGN(strlen(FILE_TITLE(block)) + 5))
#define LINK_TEXT(block) (((char *) block) + sizeof(link_blk))
#define LINK_HELP_TEXT(block) (((char *) block) + sizeof(link_blk) + strlen(LINK_TEXT(block)) + 1)
#define WORDALIGN(ADDR) (((ADDR) + 3) & (~3))
#define TABLE_H_RULE(block) (block.type == BLOCKTYPE_TABLE) && ((block.flag & FLAG_TABLE_HRULES) == FLAG_TABLE_HRULES)

/* A literal define for the character code for ESC should be in defines.h*/
#define CHAR_ESCAPE 27

/* Directions - should be moved to defines.h for xtra2.c */
#define DIRECTION_LEFT 4
#define DIRECTION_RIGHT 6
#define DIRECTION_UP 8
#define DIRECTION_DOWN 2
#define DIRECTION_UP_RIGHT 9
#define DIRECTION_UP_LEFT 7
#define DIRECTION_DOWN_LEFT 1
#define DIRECTION_DOWN_RIGHT 3
#define DIRECTION_NONE 0

/* Actions that can be performed on a keypress */
#define ACTION_QUIT_HELP 0
#define ACTION_PAGE_DOWN 1
#define ACTION_PAGE_UP 2
#define ACTION_STEP_DOWN 3
#define ACTION_STEP_UP 4
#define ACTION_GOTO_END 5
#define ACTION_GOTO_START 6
#define ACTION_JUMP_TO_LINK 7
#define ACTION_DO_LINK 8
#define ACTION_LINK_UP 9
#define ACTION_LINK_DOWN 10
#define ACTION_LINK_MODE 11
#define ACTION_RELOAD 12
#define ACTION_BACK 13
#define ACTION_FORWARD 14
#define ACTION_NOTHING 15

bool parse_body(BULP *bptr,char **display, char **links, int *link_no, int *block_no, bool block_change, int *table_info, list_blk *list_info, char *err_message);
bool history_add(history_blk *stack, char *file, int line_no);
bool go_to_link(link_blk *link_dat,char *filename,int filename_len, char *mark, history_blk *history, u32b *passback);
history_blk *history_loop(history_blk *stack,int direction);

#endif /* INCLUDED_HELP_H */

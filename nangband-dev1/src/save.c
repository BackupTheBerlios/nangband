/*
 * File: save.c
 *
 * Abstract: Savefile saving code.
 *
 * Author: Andrew Sidwell (takkaria)
 *
 * Licences: GNU GPL, version 2 or any later version.
 *    or the Traditional Angband Licence, see angband.h
 */
#include "angband.h"

/*
 * Version numbers of the savefile code.
 */
#define SAVEFILE_VERSION_MAJOR   0
#define SAVEFILE_VERSION_MINOR   7

/*
 * The new savefile format was first the idea of Ben Harrison.
 *
 * Most code has been rewritten, but several functions have been rearranged
 * and had the name changed; these are presumably Ben's/Robert's code, and
 * therefore is GPL'able anyway.  Any claims to the contrary should be
 * directed at to <nevermiah@hotmail.com>.
 *
 * The basic format of Nangband savefiles is simple.
 *
 * It is made of a header (6 bytes), then "blocks".
 *
 * The file header has a four-byte "magic number", followed by two bytes
 * for savefile code version. When a savefile is loaded, a check can be
 * performed to see if the first four bytes are the "magic number", or
 * the version number of an old savefile. This can keep savefile
 * compatibility.
 *
 * Each "block" is a "type" (2 bytes), plus a "size" (2 bytes), plus "data",
 * plus a "check" (2 bytes), plus a "stamp" (2 bytes).  The format of the
 * "check" and "stamp" bytes is still being contemplated, but it would be
 * nice for one to be a simple byte-checksum, and the other to be a complex
 * additive checksum of some kind.  Both should be zero if the block is empty.
 *
 * Standard types:
 *   TYPE_BIRTH --> creation info
 *   TYPE_OPTIONS --> option settings
 *   TYPE_MESSAGES --> message recall
 *   TYPE_PLAYER --> player information
 *   TYPE_SPELLS --> spell information
 *   TYPE_INVEN --> player inven/equip
 *   TYPE_STORES --> store information
 *   TYPE_RACES --> monster race data
 *   TYPE_KINDS --> object kind data
 *   TYPE_UNIQUES --> unique info
 *   TYPE_ARTIFACTS --> artifact info
 *   TYPE_QUESTS --> quest info
 *
 * Dungeon information:
 *   TYPE_DUNGEON --> dungeon info
 *   TYPE_FEATURES --> dungeon features
 *   TYPE_OBJECTS --> dungeon objects
 *   TYPE_MONSTERS --> dungeon monsters
 *
 * Conversions:
 *   Break old "races" into normals/uniques
 *   Extract info about the "unique" monsters
 *
 * Question:
 *   Should there be a single "block" for info about all the stores, or one
 *   "block" for each store?  Or one "block", which contains "sub-blocks" of
 *   some kind?  Should we dump every "sub-block", or just the "useful" ones?
 *
 * Question:
 *   Should the normals/uniques be broken for 2.8.0, or should 2.8.0 simply
 *   be a "fixed point" into which older savefiles are converted, and then
 *   future versions could ignore older savefiles, and the "conversions"
 *   would be much simpler.
 */

/* The possible types of block */
#define BLOCK_TYPE_ERROR         0
#define BLOCK_TYPE_HEADER        1
#define BLOCK_TYPE_OPTIONS       2
#define BLOCK_TYPE_PLAYER        3
#define BLOCK_TYPE_RNG           4
#define BLOCK_TYPE_MESSAGES      5
#define BLOCK_TYPE_MONLORE       6
#define BLOCK_TYPE_OBJLORE       7
#define BLOCK_TYPE_QUESTS        8
#define BLOCK_TYPE_ARTIFACTS     9
#define BLOCK_TYPE_RANDARTS     10
#define BLOCK_TYPE_STORES       11
#define BLOCK_TYPE_DUNGEON      12

/* Block versions */
#define BLOCK_VERSION_ERROR      0
#define BLOCK_VERSION_HEADER     1
#define BLOCK_VERSION_OPTIONS    1
#define BLOCK_VERSION_PLAYER     1
#define BLOCK_VERSION_RNG        1
#define BLOCK_VERSION_MESSAGES   1
#define BLOCK_VERSION_MONLORE    1
#define BLOCK_VERSION_OBJLORE    1
#define BLOCK_VERSION_QUESTS     1
#define BLOCK_VERSION_ARTIFACTS  1
#define BLOCK_VERSION_RANDARTS   1
#define BLOCK_VERSION_STORES     1
#define BLOCK_VERSION_DUNGEON    1

/* The smallest block we can have */
#define BLOCK_INCREMENT         32

/* The size of the header (in bytes) */
#define BLOCK_HEAD_SIZE         11

/* The two possible "operations" */
#define PUT    FALSE
#define GET    TRUE

/* Variables needed by the code */
static byte *savefile_head;      /* Current block's header */
static byte savefile_head_type;  /* The type of block */
static byte savefile_head_ver;   /* The version of this block */
static byte *savefile_block;     /* Current block */
static u32b savefile_blocksize;  /* Current block's size */
static u32b savefile_blockused;  /* The amount of the block that has been used */

/* Hack - Counter */
int __pnt__;

/*
 * Show information on the screen, one line at a time.
 */
static void note(const char *msg, bool type)
{
	if (type == PUT) return;

	/* Draw the message */
	prt(msg, __pnt__, 0);

	/* Advance one line */
	__pnt__++;

	/* Flush it */
	Term_fresh();

	/* We are done */
	return;
}

/*
 * All generic savefile "block" adding functions
 */

/*
 * Recallocate some memory
 */
static void block_realloc(int newsize, void *mem)
{
	byte *savefile_new;

	/* Make space for the new block. */
	savefile_new = C_RNEW(savefile_blocksize + BLOCK_INCREMENT, byte);

	/* Copy the memory across */
	COPY(savefile_new, mem, vptr);

	/* Wipe the old block. */
	KILL(mem);

	/* The new block is now the savefile block. */
	mem = savefile_new;

	/* We are done */
	return;
}

/*
 * This function gets/puts a byte to the savefile block.
 * If more memory is needed, it allocates more memory, and makes
 * savefile_block point to it.
 */
static void savefile_do_byte(byte *v, bool type)
{
	/* See if we need to allocate more memory */
	if ((savefile_blocksize == savefile_blockused) && (type == PUT))
	{
		/* Reallocate the memory */
		block_realloc(savefile_blocksize + BLOCK_INCREMENT, savefile_block);

		/* Increment variables */
		savefile_blocksize += BLOCK_INCREMENT;
	}

	/* If we are supposed to be putting, do so */
	if (type == PUT)
	{
		/* Put the byte in the block. */
		savefile_block[savefile_blockused] = *v;
	}
	else
	{
		/* Get the byte from the block. */
		*v = savefile_block[savefile_blockused];
	}

	/* Increase the "used" counter */
	savefile_blockused++;

	/* We are done */
 	return;
}

/*
 * This function gets/puts an unsigned 16-bit integer.
 */
static void savefile_do_u16b(u16b *v, bool type)
{
	if (type == PUT)
	{
		savefile_do_byte(((byte *)((*v) & 0xFF)), PUT);
		savefile_do_byte(((byte *)(((*v) >> 8) & 0xFF)), PUT);
	}
	else
	{
		byte i;

		savefile_do_byte(&i, GET);
		(*v) = i;

		savefile_do_byte(&i, GET);
		(*v) |= ((u32b) i << 8);
	}

	/* We are done */
	return;
}

/*
 * This function gets/puts an unsigned 32-bit integer.
 */
static void savefile_do_u32b(u32b *v, bool type)
{
	if (type == PUT)
	{
		savefile_do_byte((byte *) ((*v) & 0xFF), PUT);
		savefile_do_byte((byte *) (((*v) >> 8) & 0xFF), PUT);
		savefile_do_byte((byte *) (((*v) >> 16) & 0xFF), PUT);
		savefile_do_byte((byte *) (((*v) >> 24) & 0xFF), PUT);
	}
	else
	{
		byte i;

		savefile_do_byte(&i, GET);
		(*v) = i;

		savefile_do_byte(&i, GET);
		(*v) |= ((u32b)i << 8);

		savefile_do_byte(&i, GET);
		(*v) |= ((u32b)i << 16);

		savefile_do_byte(&i, GET);
		(*v) |= ((u32b)i << 24);
	}

	/* We are done */
	return;
}

/*
 * Either put or get a string.
 */
static void savefile_do_string(char *str, bool record, bool type)
{
	byte i = 255;
	int pos = 0;

	/* Count */
	if (type == PUT) i = (byte) strlen(str);

	/* Put/Get the length of the string first */
	if (record) savefile_do_byte(&i, type);

	/* Put/Get the string, one byte at a time */
	while (TRUE)
	{
		/* Put the byte */
		savefile_do_byte(&str[pos], type);

		/* Check the value */
		if (str[pos - 1] != '\0' || pos < i) pos++;
		else break;
	}

	/* We are done */
	return;
}

/* Here we define space-saving macros */
#define savefile_do_s16b(v, t)     savefile_do_u16b((u16b *) v, t);
#define savefile_do_s32b(v, t)     savefile_do_u32b((u32b *) v, t);

#if 0
/*
 * This function adds a byte to the savefile block.
 * If more memory is needed, it allocates more memory, and makes
 * savefile_block point to it.
 */
static void savefile_add_byte(byte v)
{
	/* See if we need to allocate more memory */
	if (savefile_blocksize == savefile_blockused)
	{
		byte *savefile_new;

		/* Make space for the new block. */
		savefile_new = C_RNEW(savefile_blocksize + BLOCK_INCREMENT, byte);

		/* Copy the memory across */
		COPY(savefile_new, savefile_block, vptr);

		/* Wipe the old block. */
		KILL(savefile_block);

		/* The new block is now the savefile block. */
		savefile_block = savefile_new;

		/* Increment variables */
		savefile_blocksize += BLOCK_INCREMENT;
	}

	/* Add the byte to the block. */
	savefile_block[savefile_blockused] = v;

	/* Increase the "used" counter */
	savefile_blockused++;

	/* We are done */
 	return;
}

/*
 * This function adds an unsigned 16-bit integer to the savefile block.
 */
static void savefile_do_u16b(u16b v)
{
	savefile_do_byte((byte)(v & 0xFF));
	savefile_do_byte((byte)((v >> 8) & 0xFF));

	return;
}
/*
 * This function adds an unsigned 32-bit integer to the savefile block.
 */
static void savefile_do_u32b(u32b v)
{
	savefile_do_byte((byte) (v & 0xFF));
	savefile_do_byte((byte) ((v >> 8) & 0xFF));
	savefile_do_byte((byte) ((v >> 16) & 0xFF));
	savefile_do_byte((byte) ((v >> 24) & 0xFF));

	return;
}

/*
 * This function adds a string to the savefile block.
 */
static void savefile_do_string(char *str, bool record)
{

	/* Cycle through the string */
	if (record)
	{
		int i;

		/* Count */
		i = strlen(str);

		/* Record it */
		savefile_do_byte((byte) i);
	}

	/* Put the string, one at a time */
	for (; *str; *str++) savefile_do_byte(*str);
	savefile_do_byte(*str);

	/* We are done. */
	return;
}

/* Here we define space-saving macros */
#define savefile_do_s16b(v)     savefile_do_u16b((u16b) v);
#define savefile_do_s32b(v)     savefile_do_u32b((u32b) v);

#endif

/*
 * Functions to deal with creating, writing, and freeing "blocks".
 */

/*
 * This function starts a new block.
 */
static void savefile_new_block(int type, int ver)
{
	/* Make the header */
	C_MAKE(savefile_head, BLOCK_HEAD_SIZE, byte);

	/* Make the block */
	C_MAKE(savefile_block, BLOCK_INCREMENT, byte);

	/* Initialise the values */
	savefile_blocksize = BLOCK_INCREMENT;
	savefile_blockused = 0;

	savefile_head_type = type;
	savefile_head_ver = ver;

	return;
}

/*
 * This function writes a block to the savefile.
 */
static void savefile_write_block(vptr block, int fd)
{
	int pos = 0;

	/* Create the header - first, the type */
	savefile_head[pos++] = (savefile_head_type & 0xFF);
	savefile_head[pos++] = ((savefile_head_type >> 8) & 0xFF);

	/* Add the version of this block. */
	savefile_head[pos++] = (savefile_head_ver & 0xFF);
	savefile_head[pos++] = ((savefile_head_ver >> 8) & 0xFF);

	/* Add the size of this block. */
	savefile_head[pos++] = (savefile_blockused & 0xFF);
	savefile_head[pos++] = ((savefile_blockused >> 8) & 0xFF);
	savefile_head[pos++] = ((savefile_blockused >> 16) & 0xFF);
	savefile_head[pos++] = ((savefile_blockused  >> 24) & 0xFF);

	/* Indicate end of header */
	savefile_head[pos++] = 0;
	savefile_head[pos++] = 0;

	/* Write the header */
	fd_write(fd, (cptr) savefile_head, BLOCK_HEAD_SIZE);

	/* Free up the header's memory */
	KILL(savefile_head);

	/* Reset the header type */
	savefile_head_type = BLOCK_TYPE_ERROR;
	savefile_head_ver = BLOCK_VERSION_ERROR;

	/* Write the block */
	if (savefile_block) fd_write(fd, (cptr) savefile_block, savefile_blockused);

	/* Free the block's memory */
	if (savefile_block) KILL(savefile_block);

	/* We are done */
	return;
}

/*
 * "Helper" functions to add specific data structures.
 */

/*
 * Add an item to the savefile.
 */
static void savefile_helper_item(const object_type *o_ptr, bool type)
{
	/* Write the "kind" index */
	savefile_do_s16b(&o_ptr->k_idx, type);

	/* Location in the dungeon */
	savefile_do_byte(&o_ptr->iy, type);
	savefile_do_byte(&o_ptr->ix, type);

	/* tval/sval/pval */
	savefile_do_byte(&o_ptr->tval, type);
	savefile_do_byte(&o_ptr->sval, type);
	savefile_do_s16b(&o_ptr->pval, type);

	/* Discount (if any) */
	savefile_do_byte(&o_ptr->discount, type);

	/* Number of items */
	savefile_do_byte(&o_ptr->number, type);

	/* Weight of the item */
	savefile_do_s16b(&o_ptr->weight, type);

	/* Artifact/Ego-item status */
	savefile_do_byte(&o_ptr->name1, type);
	savefile_do_byte(&o_ptr->name2, type);

	/* Timeout field */
	savefile_do_s16b(&o_ptr->timeout, type);

	/* Any enchantments */
	savefile_do_s16b(&o_ptr->to_h, type);
	savefile_do_s16b(&o_ptr->to_d, type);
	savefile_do_s16b(&o_ptr->to_a, type);
	savefile_do_s16b(&o_ptr->ac, type);

	/* Damage dice and sides */
	savefile_do_byte(&o_ptr->dd, type);
	savefile_do_byte(&o_ptr->ds, type);

	/* Identification info */
	savefile_do_byte(&o_ptr->ident, type);

	/* ??? */
	savefile_do_byte(&o_ptr->marked, type);

	/* Write the index of the monster holding this item */
	savefile_do_s16b(&o_ptr->held_m_idx, type);

	/* Extra information */
	savefile_do_byte(&o_ptr->xtra1, type);
	savefile_do_byte(&o_ptr->xtra2, type);

	/* Save the inscription (if any) */
	savefile_do_string(o_ptr->note ? (char *) quark_str(o_ptr->note) : "", TRUE, type);

	/* We are done. */
	return;
}

/*
 * Add a monster record to the savefile.
 */
static void savefile_helper_monster(const monster_type *m_ptr, bool type)
{
	/* Monster race */
	savefile_do_s16b(&m_ptr->r_idx, type);

	/* Position of the monster */
	savefile_do_byte(&m_ptr->fy, type);
	savefile_do_byte(&m_ptr->fx, type);

	/* Current/Maximum HP */
	savefile_do_s16b(&m_ptr->hp, type);
	savefile_do_s16b(&m_ptr->maxhp, type);

	/* Status flags; sleeping, stunned, confused, scared */
	savefile_do_s16b(&m_ptr->csleep, type);
	savefile_do_byte(&m_ptr->stunned, type);
	savefile_do_byte(&m_ptr->confused, type);
	savefile_do_byte(&m_ptr->monfear, type);

	/* Speed and energy */
	savefile_do_byte(&m_ptr->mspeed, type);
	savefile_do_byte(&m_ptr->energy, type);

	/* We are done */
	return;
}


/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Add a store record to the current block.
 * --------------------------------------------------------------------- */
static void savefile_helper_store(const store_type *st_ptr, bool type)
{
	int i;

	/* Save the "open" counter */
	savefile_do_u32b(&st_ptr->store_open, type);

	/* Save the "insults" */
	savefile_do_s16b(&st_ptr->insult_cur, type);

	/* Save the current owner */
	savefile_do_byte(&st_ptr->owner, type);

	/* Save the stock size */
	savefile_do_byte(&st_ptr->stock_num, type);

	/* Save the "haggle" info */
	savefile_do_s16b(&st_ptr->good_buy, type);
	savefile_do_s16b(&st_ptr->bad_buy, type);

	/* Save the stock */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		/* Save each item in stock */
		savefile_helper_item(&st_ptr->stock[i], type);
	}

	/* We are done. */
	return;
}

/*
 * Functions to do with the data stored in the savefile.
 */

/*
 * Start the savefile.
 */
static void savefile_start(int fd)
{
	byte *header_pos;

	/* Make the block */
	C_MAKE(savefile_head, 6, byte);

	/* Make a pointer to the block */
	header_pos = (byte *) savefile_head;

	/* Start with the variant "stamp" */
	*header_pos++ = 83;
	*header_pos++ = 97;
	*header_pos++ = 118;
	*header_pos++ = 101;

	/* Now, dump the versions */
	*header_pos++ = SAVEFILE_VERSION_MAJOR;
	*header_pos++ = SAVEFILE_VERSION_MINOR;

	/* Write the header */
	fd_write(fd, (cptr) savefile_head, 6);

	/* Wipe the block */
	KILL(savefile_head);

	/* We are done. */
	return;
}

/*
 * Write information about this variant.
 */
static void savefile_do_block_header(bool type)
{
	/* Add the variant string */
	savefile_do_string(VERSION_NAME, TRUE, type);

	/* Marker - XXX XXX XXX */
	savefile_do_byte(&0, type);

	/* Add the version number */
	savefile_do_byte(&VERSION_MAJOR, type);
	savefile_do_byte(&VERSION_MINOR, type);
	savefile_do_byte(&VERSION_PATCH, type);
	savefile_do_byte(&VERSION_EXTRA, type);

	/* We are done. */
	return;
}

/*
 * Write the player's options.
 */
static void savefile_do_block_options(bool type)
{
	int n;

	/* Add delay factor and hitpoint warning "special options" */
	savefile_do_byte(&op_ptr->delay_factor, type);
	savefile_do_byte(&op_ptr->hitpoint_warn, type);

	/* Add the "normal" options */
	for (n = 0; n < OPT_MAX; n++)
	{
		if (op_ptr->opt[n]) savefile_do_byte(&1, type);
		else savefile_do_byte(&0, type);
	}

	/* Add the "window" options */
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
	{
		if (op_ptr->window_flag[n]) savefile_do_byte(&1, type);
		else savefile_do_byte(&0, type);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the player block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_player(bool type)
{
	int i;

	/* Add number of past lives */
	savefile_do_u16b(&sf_lives, type);

	/* Dump the player's HP entries */
	savefile_do_byte(&PY_MAX_LEVEL, type);

	for (i = 0; i < PY_MAX_LEVEL; i++)
		savefile_do_s16b(&p_ptr->player_hp[i], type);

	/* Write spell data */
	savefile_do_u32b(&p_ptr->spell_learned1, type);
	savefile_do_u32b(&p_ptr->spell_learned2, type);
	savefile_do_u32b(&p_ptr->spell_worked1, type);
	savefile_do_u32b(&p_ptr->spell_worked2, type);
	savefile_do_u32b(&p_ptr->spell_forgotten1, type);
	savefile_do_u32b(&p_ptr->spell_forgotten2, type);

	/* Dump the ordered spells */
	for (i = 0; i < PY_MAX_SPELLS; i++)
		savefile_do_byte(&p_ptr->spell_order[i], type);

	/* What the player's name is */
	savefile_do_string(op_ptr->full_name, TRUE, type);

	/* What the player died from */
	savefile_do_string(p_ptr->died_from, TRUE, type);

#if 0
	/* Player's history */
	savefile_do_string(p_ptr->history, type);
#endif

	/* Sex/Race/Class */
	savefile_do_byte(&p_ptr->psex, type);
	savefile_do_byte(&p_ptr->prace, type);
	savefile_do_byte(&p_ptr->pclass, type);

	/* Hit dice and experience factor */
	savefile_do_byte(&p_ptr->hitdie, type);
	savefile_do_byte(&p_ptr->expfact, type);

	/* Age, Weight and Height */
	savefile_do_s16b(&p_ptr->age, type);
	savefile_do_s16b(&p_ptr->ht, type);
	savefile_do_s16b(&p_ptr->wt, type);

	/* Dump the stats (maximum and current) */
	for (i = 0; i < A_MAX; ++i) savefile_do_s16b(&p_ptr->stat_max[i], type);
	for (i = 0; i < A_MAX; ++i) savefile_do_s16b(&p_ptr->stat_cur[i], type);

	/* Player's Gold and Social Class */
	savefile_do_u32b(&p_ptr->au, type);
	savefile_do_s16b(&p_ptr->sc, type);

	/* Maximum and current experience */
	savefile_do_u32b(&p_ptr->max_exp, type);
	savefile_do_u32b(&p_ptr->exp, type);
	savefile_do_u16b(&p_ptr->exp_frac, type);

	/* Character level */
	savefile_do_s16b(&p_ptr->lev, type);

	/* Maximum and current HPs */
	savefile_do_s16b(&p_ptr->mhp, type);
	savefile_do_s16b(&p_ptr->chp, type);
	savefile_do_u16b(&p_ptr->chp_frac, type);

	/* Maximum and current Mana */
	savefile_do_s16b(&p_ptr->msp, type);
	savefile_do_s16b(&p_ptr->csp, type);
	savefile_do_u16b(&p_ptr->csp_frac, type);

	/* Max Player and Dungeon Levels */
	savefile_do_s16b(&p_ptr->max_lev, type);
	savefile_do_s16b(&p_ptr->max_depth, type);

	/* Vaious states */
	savefile_do_s16b(&p_ptr->blind, type);
	savefile_do_s16b(&p_ptr->paralyzed, type);
	savefile_do_s16b(&p_ptr->confused, type);
	savefile_do_s16b(&p_ptr->food, type);
	savefile_do_s16b(&p_ptr->energy, type);
	savefile_do_s16b(&p_ptr->fast, type);
	savefile_do_s16b(&p_ptr->slow, type);
	savefile_do_s16b(&p_ptr->afraid, type);
	savefile_do_s16b(&p_ptr->cut, type);
	savefile_do_s16b(&p_ptr->stun, type);
	savefile_do_s16b(&p_ptr->poisoned, type);
	savefile_do_s16b(&p_ptr->image, type);
	savefile_do_s16b(&p_ptr->protevil, type);
	savefile_do_s16b(&p_ptr->invuln, type);
	savefile_do_s16b(&p_ptr->hero, type);
	savefile_do_s16b(&p_ptr->shero, type);
	savefile_do_s16b(&p_ptr->shield, type);
	savefile_do_s16b(&p_ptr->blessed, type);
	savefile_do_s16b(&p_ptr->tim_invis, type);
	savefile_do_s16b(&p_ptr->word_recall, type);
	savefile_do_s16b(&p_ptr->see_infra, type);
	savefile_do_s16b(&p_ptr->tim_infra, type);

	/* Timed resistances */
	for (i = 0; i < RES_MAX; i++) savefile_do_byte(&p_ptr->resist_timed[i], type);

	savefile_do_byte(&p_ptr->confusing, type);
	savefile_do_byte(&p_ptr->searching, type);

	/* Random artifact version */
	savefile_do_u32b(&RANDART_VERSION, type);

	/* Special stuff */
	savefile_do_u16b(&p_ptr->panic_save, type);
	savefile_do_u16b(&p_ptr->total_winner, type);
	savefile_do_u16b(&p_ptr->noscore, type);

	/* Write death */
	savefile_do_byte(&p_ptr->is_dead, type);

	/* Write feeling */
	savefile_do_byte(&feeling, type);

	/* Turn of last "feeling" */
	savefile_do_s32b(&old_turn, type);

	/* Current turn */
	savefile_do_s32b(&turn, type);

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the RNG block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_rng(bool type)
{
	int i;

	/* Add the "place" */
	savefile_do_u16b(&Rand_place, type);

	/* State */
	for (i = 0; i < RAND_DEG; i++) savefile_do_u32b(&Rand_state[i], type);

	/* Add the random artifact seed */
	savefile_do_u32b(&seed_randart, type);

	/* Add the object seeds */
	savefile_do_u32b(&seed_flavor, type);
	savefile_do_u32b(&seed_town, type);

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the player's messages block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_messages(bool type)
{
	int i = 0, num = 0;

	/* Get the number of messages to print */
	if (type == PUT)
	{
		num = message_num();

		/* Trim messages if requested */
		if (compress_savefile && (num > 40)) num = 40;
	}

	/* Get the counter a counter to the block */
	savefile_do_s16b((s16b *) num, type);


	/* Dump the messages, oldest first */
	for (i = num - 1; i >= 0; i--)
	{
		cptr buffer[512];
		u16b mess_type;

		if (type == PUT)
		{
			buffer = message_str((s16b) i);
			mess_type = message_type((s16b) i);
		}

		/* Write the string */
		savefile_do_string((char *) buffer, TRUE, type);

		/* Write the type */
		savefile_do_u16b(&mess_type, type);

		/* Save to message to the buffer */
		if (type == GET) message_add((char *) buffer, mess_type);
	}


	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the random artifacts.
 * --------------------------------------------------------------------- */
static void savefile_do_block_randarts(bool type)
{
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = z_info->a_max;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) n, type);

	if (!adult_rand_artifacts) return;

	/* Write all randart data */
	for (i = 0; i < n; i++)
	{
		artifact_type *a_ptr = &a_info[i];

		/* tval/sval/pval */
		savefile_do_byte(&a_ptr->tval, type);
		savefile_do_byte(&a_ptr->sval, type);
		savefile_do_s16b(&a_ptr->pval, type);

		/* Bonuses */
		savefile_do_s16b(&a_ptr->to_h, type);
		savefile_do_s16b(&a_ptr->to_d, type);
		savefile_do_s16b(&a_ptr->to_a, type);
		savefile_do_s16b(&a_ptr->ac, type);

		/* Base damage dice */
		savefile_do_byte(&a_ptr->dd, type);
		savefile_do_byte(&a_ptr->ds, type);

		/* Weight */
		savefile_do_s16b(&a_ptr->weight, type);

		/* Cost */
		savefile_do_s32b(&a_ptr->cost, type);

		/* Flags */
		savefile_do_u32b(&a_ptr->flags1, type);
		savefile_do_u32b(&a_ptr->flags2, type);
		savefile_do_u32b(&a_ptr->flags3, type);

		/* Level/Rarity */
		savefile_do_byte(&a_ptr->level, type);
		savefile_do_byte(&a_ptr->rarity, type);

		/* Activation, timeout and randtime (?) */
		savefile_do_byte(&a_ptr->activation, type);
		savefile_do_u16b(&a_ptr->time, type);
		savefile_do_u16b(&a_ptr->randtime, type);
	}

	/* We are done. */
	return;
}

/*
 * The cave grid flags that get saved in the savefile
 */
#define IMPORTANT_FLAGS          (CAVE_MARK | CAVE_GLOW | CAVE_ICKY | CAVE_ROOM)


/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the dungeon block.
 *
 * This function is *extremely* hacky.
 * --------------------------------------------------------------------- */
static errr savefile_do_block_dungeon(bool type, int b_ver)
{
	int i, y, x;
	u16b xmax = 0, ymax = 0;
	s16b py, px;

	byte tmp8u;

	u16b limit;

	byte count = 0, prev_char = 0;

	/*** Basic info ***/

	/* Dungeon depth */
	savefile_do_u16b(&p_ptr->depth, type);

	/* Set the player's co-ordinates, if applicable */
	if (type == PUT)
	{
		py = p_ptr->py;
		px = p_ptr->px;
	}

	savefile_do_u16b(&py, type);
	savefile_do_u16b(&px, type);

	/* Perform paranoia */
	if ((p_ptr->depth >= MAX_DEPTH) || (p_ptr->depth < 0))
	{
		/* Reset it */
		p_ptr->depth = 0;

		/* Warn the user */
		note(format("Invalid player depth - %i!", p_ptr->depth), type);
	}

	/* Set the height, if applicable */
	if (type == PUT) ymax = DUNGEON_HGT;
	savefile_do_u16b(&ymax, type);

	/* Set the width, if applicable */
	if (type == PUT) xmax = DUNGEON_WID;
	savefile_do_u16b(&xmax, type);

	/* Perform paranoia */
	if ((ymax != DUNGEON_HGT) || (xmax != DUNGEON_WID))
	{
		/* Warn the user */
		note(format("Invalid dungeon size - %dx%d!", ymax, xmax), type);

		/* Fatal error */
		return (0);
	}

	/*** Simple "Run-Length-Encoding" of cave ***/
#if 0
	/*
	 * Note that there are two wasted bytes here.
	 */
	if (type == GET)
	{
		u16b whoops;

		rd_u16b(&whoops);
		rd_u16b(&whoops);
	}
#endif

	/* Do RLE/RLD */
	if (type == PUT)
	{
		/* Dump the cave */
		for (y = 0; y < DUNGEON_HGT; y++)
		{
			for (x = 0; x < DUNGEON_WID; x++)
			{
				/* Extract the important cave_info flags */
				tmp8u = (cave_info[y][x] & (IMPORTANT_FLAGS));

				/* If the run is broken, or too full, flush it */
				if ((tmp8u != prev_char) || (count == MAX_UCHAR))
				{
					savefile_do_byte(&count, type);
					savefile_do_byte(&prev_char, type);
					prev_char = tmp8u;
					count = 1;
				}

				/* Continue the run */
				else
				{
					count++;
				}
			}
		}

		/* Flush the data (if any) */
		if (count)
		{
			savefile_do_byte(&count, type);
			savefile_do_byte(&prev_char, type);
		}
	}
	else
	{
		/* Load the dungeon data */
		for (x = y = 0; y < DUNGEON_HGT; )
		{
			/* Grab RLE info */
			savefile_do_byte(&count, type);
			savefile_do_byte(&tmp8u, type);

			/* Apply the RLE info */
			for (i = count; i > 0; i--)
			{
				/* Extract "info" */
				cave_info[y][x] = tmp8u;

				/* Advance/Wrap */
				if (++x >= DUNGEON_WID)
				{
					/* Wrap */
					x = 0;

					/* Advance/Wrap */
					if (++y >= DUNGEON_HGT) break;
				}
			}
		}
	}

	/*** Simple "Run-Length-Encoding" of cave ***/

	/* Reset */
	count = 0;
	prev_char = 0;

	if (type == PUT)
	{
		/* Dump the cave */
		for (y = 0; y < DUNGEON_HGT; y++)
		{
			for (x = 0; x < DUNGEON_WID; x++)
			{
				/* Extract a byte */
				tmp8u = cave_feat[y][x];

				/* If the run is broken, or too full, flush it */
				if ((tmp8u != prev_char) || (count == MAX_UCHAR))
				{
					savefile_do_byte(&count, type);
					savefile_do_byte(&prev_char, type);
					prev_char = tmp8u;
					count = 1;
				}

				/* Continue the run */
				else
				{
					count++;
				}
			}
		}

		/* Flush the data (if any) */
		if (count)
		{
			savefile_do_byte(&count, type);
			savefile_do_byte(&prev_char, type);
		}
	}
	else
	{
		/* Load the dungeon data */
		for (x = y = 0; y < DUNGEON_HGT; )
		{
			/* Grab RLE info */
			savefile_do_byte(&count, type);
			savefile_do_byte(&tmp8u, type);

			/* Apply the RLE info */
			for (i = count; i > 0; i--)
			{
				/* Extract "feat" */
				cave_set_feat(y, x, tmp8u);

				/* Advance/Wrap */
				if (++x >= DUNGEON_WID)
				{
					/* Wrap */
					x = 0;

					/* Advance/Wrap */
					if (++y >= DUNGEON_HGT) break;
				}
			}
		}
	}

	/* Compact if we are saving */
	if (type == PUT)
	{
		/* Compact the objects */
		compact_objects(0);

		/* Compact the monsters */
		compact_monsters(0);
	}


	/* Place player in dungeon if we are loading */
	if (type == GET)
	{
		if (!player_place(py, px))
		{
			note(format("Cannot place player (%d,%d)!", py, px), type);
			return (-1);
		}
	}

	/* Put/Get objects */

	/* If we are putting, set limit */
	if (type == PUT) limit = o_max;

	/* Do objects */
	savefile_do_u16b(&limit, type);

	/* Verify maximum */
	if (limit >= z_info->o_max)
	{
		note(format("Too many (%d) object entries!", limit), type);
		return (-1);
	}

	/* Dump the objects */
	if (type == PUT)
	{
		for (i = 1; i < limit; i++)
		{
			object_type *o_ptr = &o_list[i];

			/* Dump it */
			savefile_helper_item(o_ptr, type);
		}
	}
	else
	{
		for (i = 1; i < limit; i++)
		{
			object_type *i_ptr;
			object_type object_type_body;

			s16b o_idx;
			object_type *o_ptr;


			/* Get the object */
			i_ptr = &object_type_body;

			/* Wipe the object */
			object_wipe(i_ptr);

			/* Read the item */
			savefile_helper_item(i_ptr, type);

			/* Make an object */
			o_idx = o_pop();

			/* Paranoia */
			if (o_idx != i)
			{
				note(format("Cannot place object %d!", i), type);
				return (-1);
			}

			/* Get the object */
			o_ptr = &o_list[o_idx];

			/* Structure Copy */
			object_copy(o_ptr, i_ptr);

			/* Dungeon floor */
			if (!i_ptr->held_m_idx)
			{
				int x = i_ptr->ix;
				int y = i_ptr->iy;

				/* ToDo: Verify coordinates */

				/* Link the object to the pile */
				o_ptr->next_o_idx = cave_o_idx[y][x];

				/* Link the floor to the object */
				cave_o_idx[y][x] = o_idx;
			}
		}
	}

	/* Put/Get Monsters */

	/* If we are putting, set limit */
	if (type == PUT) limit = m_max;

	/* Do monsters */
	savefile_do_u16b(&limit, type);

	/* XXX XXX XXX */
	if (type == PUT)
	{
		/* Dump the monsters */
		for (i = 1; i < limit; i++)
		{
			monster_type *m_ptr = &m_list[i];

			/* Dump it */
			savefile_helper_monster(m_ptr, type);
		}
	}
	else
	{
 	  	/* Read the monsters */
 		for (i = 1; i < limit; i++)
 		{
 			monster_type *n_ptr;
 			monster_type monster_type_body;

 			/* Get local monster */
 			n_ptr = &monster_type_body;

 			/* Clear the monster */
 			(void)WIPE(n_ptr, monster_type);

 			/* Read the monster */
 			savefile_helper_monster(n_ptr, type);

 			/* Place monster in dungeon */
 			if (monster_place(n_ptr->fy, n_ptr->fx, n_ptr) != i)
 			{
 				note(format("Cannot place monster %d", i), type);
 				return (-1);
 			}
 		}
	}

	if (type == GET)
	{
		/*** Holding ***/

		/* Reacquire objects */
		for (i = 1; i < o_max; ++i)
		{
			object_type *o_ptr;

			monster_type *m_ptr;

			/* Get the object */
			o_ptr = &o_list[i];

			/* Ignore dungeon objects */
			if (!o_ptr->held_m_idx) continue;

			/* Verify monster index */
			if (o_ptr->held_m_idx >= z_info->m_max)
			{
				note("Invalid monster index", type);
				return (-1);
			}

			/* Get the monster */
			m_ptr = &m_list[o_ptr->held_m_idx];

			/* Link the object to the pile */
			o_ptr->next_o_idx = m_ptr->hold_o_idx;

			/* Link the monster to the object */
			m_ptr->hold_o_idx = i;
		}
	}

	/* Dungeon is ready */
	if (type == GET)
	{
		/* if (b_ver < x && p_ptr->depth == 0) character_dungeon = FALSE; else */
		character_dungeon = TRUE;
	}

	/* We are done. */
	return (0);
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the monster lore block.
 * --------------------------------------------------------------------- */
 static void savefile_do_block_monlore(bool type)
 {
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = z_info->r_max;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) n, type);

	/* Write the individual entries */
	for (i = 0; i < n; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Count sights/deaths/kills */
		savefile_do_s16b(&l_ptr->sights, type);
		savefile_do_s16b(&l_ptr->deaths, type);
		savefile_do_s16b(&l_ptr->pkills, type);
		savefile_do_s16b(&l_ptr->tkills, type);

		/* Count wakes and ignores */
		savefile_do_byte(&l_ptr->wake, type);
		savefile_do_byte(&l_ptr->ignore, type);

		/* Extra stuff */
		savefile_do_byte(&l_ptr->xtra1, type);
		savefile_do_byte(&l_ptr->xtra2, type);

		/* Count drops */
		savefile_do_byte(&l_ptr->drop_gold, type);
		savefile_do_byte(&l_ptr->drop_item, type);

		/* Count spells */
		savefile_do_byte(&l_ptr->cast_innate, type);
		savefile_do_byte(&l_ptr->cast_spell, type);

		/* Count blows of each type */
		for (n = 0; n < MONSTER_BLOW_MAX; n++)
			savefile_do_byte(&l_ptr->blows[n], type);

		/* Memorize flags */
		savefile_do_u32b(&l_ptr->flags1, type);
		savefile_do_u32b(&l_ptr->flags2, type);
		savefile_do_u32b(&l_ptr->flags3, type);
		savefile_do_u32b(&l_ptr->flags4, type);
		savefile_do_u32b(&l_ptr->flags5, type);
		savefile_do_u32b(&l_ptr->flags6, type);

		/* Monster limit per level */
		savefile_do_byte(&r_ptr->max_num, type);
	}

	/* We are done. */
	return;
}


/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the "object lore" block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_objlore(bool type)
{
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = z_info->k_max;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) n, type);

	/* Write the individual entries */
	for (i = 0; i < n; i++)
	{
		byte temp = 0;
		object_kind *k_ptr = &k_info[i];

		/* Evaluate */
		if (k_ptr->aware) temp |= 0x01;
		if (k_ptr->tried) temp |= 0x02;

		/* Write the byte */
		savefile_do_byte(&temp, type);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the artifacts block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_artifacts(bool type)
{
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = z_info->a_max;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) n, type);

	/* Write the individual entries */
	for (i = 0; i < n; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		savefile_do_byte(&a_ptr->cur_num, type);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the quests block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_quests(bool type)
{
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = MAX_Q_IDX;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) n, type);

	/* Put/Get the individual entries */
	for (i = 0; i < n; i++)
	{
		savefile_do_byte(&q_list[i].level, type);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the store block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_stores(bool type)
{
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = MAX_STORES;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) n, type);

	/* Write the individual entries */
	for (i = 0; i < n; i++)
	{
		savefile_helper_store(&store[i], type);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * The actual code to do the savefile.
 * --------------------------------------------------------------------- */
static bool write_savefile(int fd)
{
	/* Start the savefile */
	savefile_start(fd);

	/* Put information about the variant that saved the file */
	savefile_new_block(BLOCK_TYPE_HEADER, BLOCK_VERSION_HEADER);
	savefile_do_block_header();
	savefile_write_block(savefile_block, fd);

	/* Do the player options */
	savefile_new_block(BLOCK_TYPE_OPTIONS, BLOCK_VERSION_OPTIONS);
	savefile_do_block_options();
	savefile_write_block(savefile_block, fd);

	/* Do the player information */
	savefile_new_block(BLOCK_TYPE_PLAYER, BLOCK_VERSION_PLAYER);
	savefile_do_block_player();
	savefile_write_block(savefile_block, fd);

	/* Do the RNG state */
	savefile_new_block(BLOCK_TYPE_RNG, BLOCK_VERSION_RNG);
	savefile_do_block_rng();
	savefile_write_block(savefile_block, fd);

	/* Write player messages */
	savefile_new_block(BLOCK_TYPE_MESSAGES, BLOCK_VERSION_MESSAGES);
	savefile_do_block_messages();
	savefile_write_block(savefile_block, fd);

	/* Write Monster Lore */
	savefile_new_block(BLOCK_TYPE_MONLORE, BLOCK_VERSION_MONLORE);
	savefile_do_block_monlore();
	savefile_write_block(savefile_block, fd);

	/* Write "Object Lore" */
	savefile_new_block(BLOCK_TYPE_OBJLORE, BLOCK_VERSION_OBJLORE);
	savefile_do_block_objlore();
	savefile_write_block(savefile_block, fd);

	/* Write Quests */
	savefile_new_block(BLOCK_TYPE_QUESTS, BLOCK_VERSION_QUESTS);
	savefile_do_block_quests();
	savefile_write_block(savefile_block, fd);

	/* Write Artifacts */
	savefile_new_block(BLOCK_TYPE_ARTIFACTS, BLOCK_VERSION_ARTIFACTS);
	savefile_do_block_artifacts();
	savefile_write_block(savefile_block, fd);

	/* Write the randarts */
	savefile_new_block(BLOCK_TYPE_RANDARTS, BLOCK_VERSION_RANDARTS);
	savefile_do_block_randarts();
	savefile_write_block(savefile_block, fd);

	/* Write the stores */
	savefile_new_block(BLOCK_TYPE_STORES, BLOCK_VERSION_STORES);
	savefile_do_block_stores();
	savefile_write_block(savefile_block, fd);

	/* Write the dungeon */
	savefile_new_block(BLOCK_TYPE_DUNGEON, BLOCK_VERSION_DUNGEON);
	savefile_do_block_dungeon();
	savefile_write_block(savefile_block, fd);

#if 0
	/* End the savefile */
	savefile_write_block(NULL, fd);
#endif

	/* We are done */
	return (TRUE);
}

/*
 * Save the game to a savefile.
 */
bool save_game(void)
{
	int fd = -1;
	int result = FALSE;
	char safe[1024];
	bool ok = TRUE;
	int mode = 0644;

#ifdef SET_UID
# ifdef SECURE
	/* Get "games" permissions */
	beGames();
# endif
#endif

	/* New savefile */
	strcpy(safe, savefile);
	strcat(safe, ".new");

	/* Grab permissions */
	safe_setuid_grab();

	/* Remove it */
	fd_kill(safe);

	/* Drop permissions */
	safe_setuid_drop();

	/* File type is "SAVE" */
	FILE_TYPE(FILE_TYPE_SAVE);

	/* Grab permissions */
	safe_setuid_grab();

	/* Create the savefile */
	fd = fd_make(safe, mode);

	/* Drop permissions */
	safe_setuid_drop();

	/* File is okay */
	if (fd >= 0)
	{
		/* Write the savefile */
		if (!write_savefile(fd)) ok = FALSE;

		/* Grab permissions */
		safe_setuid_grab();

		/* Remove "broken" files */
		if (!ok) fd_kill(safe);

		/* Drop permissions */
		safe_setuid_drop();

		/* ARRRRRRRRRRRGGGGHHHH! */
	}

	/* Attempt to save the player */
	if (ok)
	{
		char temp[1024];

		/* Make a temporary filename for backup purposes */
		strcpy(temp, savefile);
		strcat(temp, ".bac");

		/* Grab permissions */
		safe_setuid_grab();

		/* Remove it */
		fd_kill(temp);

		/* Preserve old savefile */
		fd_move(savefile, temp);

		/* Activate new savefile */
		fd_move(safe, savefile);

		/* Remove preserved savefile */
		fd_kill(temp);

		/* Drop permissions */
		safe_setuid_drop();

		/* Hack -- Pretend the character was loaded */
		character_loaded = TRUE;

		/* Success */
		result = TRUE;
	}


#ifdef SET_UID
# ifdef SECURE
	/* Drop "games" permissions */
	bePlayer();
# endif /* SECURE */
#endif /* SET_UID */

	/* Return the result */
	return (result);
}


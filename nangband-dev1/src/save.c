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
#define SAVEFILE_VERSION_MINOR   3

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
#define BLOCK_INCREMENT         128

/* The size of the header (in bytes) */
#define BLOCK_HEAD_SIZE         11

/*
 * Variables needed by the code
 */
static vptr savefile_head;       /* Current block's header */
static byte savefile_head_type;  /* The type of block */
static byte savefile_head_ver;   /* The version of this block */
static vptr savefile_block;      /* Current block */
static u32b savefile_blocksize;  /* Current block's size */
static u32b savefile_blockused;  /* The amount of the block that has been used */
static byte *savefile_blockpos;  /* The posiition inside the block */

/*
 * All generic savefile "block" adding functions
 */

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
		vptr savefile_new;

		/* Make space for the new block. */
		C_MAKE(savefile_new, savefile_blocksize + BLOCK_INCREMENT, byte);

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
	*savefile_blockpos++ = v;

	/* Increase the "used" counter */
	savefile_blockused++;

	/* We are done */
 	return;
}

/*
 * This function adds an unsigned 16-bit integer to the savefile block.
 */
static void savefile_add_u16b(u16b v)
{
	savefile_add_byte((byte)(v & 0xFF));
	savefile_add_byte((byte)((v >> 8) & 0xFF));

	return;
}
/*
 * This function adds an unsigned 32-bit integer to the savefile block.
 */
static void savefile_add_u32b(u32b v)
{
	savefile_add_byte((byte) (v & 0xFF));
	savefile_add_byte((byte) ((v >> 8) & 0xFF));
	savefile_add_byte((byte) ((v >> 16) & 0xFF));
	savefile_add_byte((byte) ((v >> 24) & 0xFF));

	return;
}

/*
 * This function adds a string to the savefile block.
 */
static void savefile_add_string(char *str, bool record)
{
	/* Cycle through the string */
	if (record)
	{
		int i;

		/* Count */
		i = strlen(str);

		/* Record it */
		savefile_add_byte((byte) i);
	}

	/* Put the string, one at a time */
	for (; *str; *str++) savefile_add_byte(*str);
	savefile_add_byte(*str);

	/* We are done. */
	return;
}

/* Here we define space-saving macros */
#define savefile_add_s16b(v)     savefile_add_u16b((u16b) v);
#define savefile_add_s32b(v)     savefile_add_u32b((u32b) v);

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
	savefile_blockpos = (byte *)savefile_block;

	savefile_head_type = type;
	savefile_head_ver = ver;

	return;
}

/*
 * This function writes a block to the savefile.
 */
static void savefile_write_block(vptr block, int fd)
{
	byte *header_pos = (byte *) savefile_head;

	/* Create the header - first, the type */
	*header_pos++ = (savefile_head_type & 0xFF);
	*header_pos++ = ((savefile_head_type >> 8) & 0xFF);

	/* Add the version of this block. */
	*header_pos++ = (savefile_head_ver & 0xFF);
	*header_pos++ = ((savefile_head_ver >> 8) & 0xFF);

	/* Add the size of this block. */
	*header_pos++ = (savefile_blockused & 0xFF);
	*header_pos++ = ((savefile_blockused >> 8) & 0xFF);
	*header_pos++ = ((savefile_blockused >> 16) & 0xFF);
	*header_pos++ = ((savefile_blockused  >> 24) & 0xFF);

	/* Indicate end of header */
	*header_pos++ = 0;
	*header_pos++ = 0;

	/* Write the header */
	fd_write(fd, (cptr) savefile_head, BLOCK_HEAD_SIZE);

	/* Free up the header's memory */
	header_pos = NULL;
	KILL(savefile_head);

	/* Reset the header type */
	savefile_head_type = BLOCK_TYPE_ERROR;
	savefile_head_ver = 0;

	/* Write the block */
	if (savefile_block) fd_write(fd, (cptr) savefile_block, savefile_blockused);

	/* Free the block's memory */
	*(int *)savefile_block = 0;
	savefile_blockpos = NULL;
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
static void savefile_helper_item(const object_type *o_ptr)
{
	/* Write the "kind" index */
	savefile_add_s16b(o_ptr->k_idx);

	/* Location in the dungeon */
	savefile_add_byte(o_ptr->iy);
	savefile_add_byte(o_ptr->ix);

	/* tval/sval/pval */
	savefile_add_byte(o_ptr->tval);
	savefile_add_byte(o_ptr->sval);
	savefile_add_s16b(o_ptr->pval);

	/* Discount (if any) */
	savefile_add_byte(o_ptr->discount);

	/* Number of items */
	savefile_add_byte(o_ptr->number);

	/* Weight of the item */
	savefile_add_s16b(o_ptr->weight);

	/* Artifact/Ego-item status */
	savefile_add_byte(o_ptr->name1);
	savefile_add_byte(o_ptr->name2);

	/* Timeout field */
	savefile_add_s16b(o_ptr->timeout);

	/* Any enchantments */
	savefile_add_s16b(o_ptr->to_h);
	savefile_add_s16b(o_ptr->to_d);
	savefile_add_s16b(o_ptr->to_a);
	savefile_add_s16b(o_ptr->ac);

	/* Damage dice and sides */
	savefile_add_byte(o_ptr->dd);
	savefile_add_byte(o_ptr->ds);

	/* Identification info */
	savefile_add_byte(o_ptr->ident);

	/* ??? */
	savefile_add_byte(o_ptr->marked);

	/* Write the index of the monster holding this item */
	savefile_add_s16b(o_ptr->held_m_idx);

	/* Extra information */
	savefile_add_byte(o_ptr->xtra1);
	savefile_add_byte(o_ptr->xtra2);

	/* Save the inscription (if any) */
	savefile_add_string(o_ptr->note ? (char *) quark_str(o_ptr->note) : "", TRUE);

	/* We are done. */
	return;
}

/*
 * Add a monster record to the savefile.
 */
static void savefile_helper_monster(const monster_type *m_ptr)
{
	/* Monster race */
	savefile_add_s16b(m_ptr->r_idx);

	/* Position of the monster */
	savefile_add_byte(m_ptr->fy);
	savefile_add_byte(m_ptr->fx);

	/* Current/Maximum HP */
	savefile_add_s16b(m_ptr->hp);
	savefile_add_s16b(m_ptr->maxhp);

	/* Status flags; sleeping, stunned, confused, scared */
	savefile_add_s16b(m_ptr->csleep);
	savefile_add_byte(m_ptr->stunned);
	savefile_add_byte(m_ptr->confused);
	savefile_add_byte(m_ptr->monfear);

	/* Speed and energy */
	savefile_add_byte(m_ptr->mspeed);
	savefile_add_byte(m_ptr->energy);

	/* We are done */
	return;
}


/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Add a store record to the current block.
 * --------------------------------------------------------------------- */
static void savefile_helper_store(const store_type *st_ptr)
{
	int i;

	/* Save the "open" counter */
	savefile_add_u32b(st_ptr->store_open);

	/* Save the "insults" */
	savefile_add_s16b(st_ptr->insult_cur);

	/* Save the current owner */
	savefile_add_byte(st_ptr->owner);

	/* Save the stock size */
	savefile_add_byte(st_ptr->stock_num);

	/* Save the "haggle" info */
	savefile_add_s16b(st_ptr->good_buy);
	savefile_add_s16b(st_ptr->bad_buy);

	/* Save the stock */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		/* Save each item in stock */
		savefile_helper_item(&st_ptr->stock[i]);
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
static void savefile_do_block_header(void)
{
	/* Add the variant string */
	savefile_add_string(VERSION_NAME, TRUE);

	/* Marker - XXX XXX XXX */
	savefile_add_byte(0);

	/* Add the version number */
	savefile_add_byte(VERSION_MAJOR);
	savefile_add_byte(VERSION_MINOR);
	savefile_add_byte(VERSION_PATCH);
	savefile_add_byte(VERSION_EXTRA);

	/* We are done. */
	return;
}

/*
 * Write the player's options.
 */
static void savefile_do_block_options(void)
{
	int n;

	/* Add delay factor and hitpoint warning "special options" */
	savefile_add_byte(op_ptr->delay_factor);
	savefile_add_byte(op_ptr->hitpoint_warn);

	/* Add the "normal" options */
	for (n = 0; n < OPT_MAX; n++)
	{
		if (op_ptr->opt[n]) savefile_add_byte(1);
		else savefile_add_byte(0);
	}

	/* Add the "window" options */
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
	{
		if (op_ptr->window_flag[n]) savefile_add_byte(1);
		else savefile_add_byte(0);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the player block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_player(void)
{
	int i;

	/* Add number of past lives */
	savefile_add_u16b(sf_lives);

	/* Dump the player's HP entries */
	savefile_add_byte(PY_MAX_LEVEL);

	for (i = 0; i < PY_MAX_LEVEL; i++)
		savefile_add_s16b(p_ptr->player_hp[i]);

	/* Write spell data */
	savefile_add_u32b(p_ptr->spell_learned1);
	savefile_add_u32b(p_ptr->spell_learned2);
	savefile_add_u32b(p_ptr->spell_worked1);
	savefile_add_u32b(p_ptr->spell_worked2);
	savefile_add_u32b(p_ptr->spell_forgotten1);
	savefile_add_u32b(p_ptr->spell_forgotten2);

	/* Dump the ordered spells */
	for (i = 0; i < PY_MAX_SPELLS; i++)
		savefile_add_byte(p_ptr->spell_order[i]);

	/* What the player's name is */
	savefile_add_string(op_ptr->full_name, TRUE);

	/* What the player died from */
	savefile_add_string(p_ptr->died_from, TRUE);

#if 0
	/* Player's history */
	savefile_add_string(p_ptr->history);
#endif

	/* Sex/Race/Class */
	savefile_add_byte(p_ptr->psex);
	savefile_add_byte(p_ptr->prace);
	savefile_add_byte(p_ptr->pclass);

	/* Hit dice and experience factor */
	savefile_add_byte(p_ptr->hitdie);
	savefile_add_byte(p_ptr->expfact);

	/* Age, Weight and Height */
	savefile_add_s16b(p_ptr->age);
	savefile_add_s16b(p_ptr->ht);
	savefile_add_s16b(p_ptr->wt);

	/* Dump the stats (maximum and current) */
	for (i = 0; i < A_MAX; ++i) savefile_add_s16b(p_ptr->stat_max[i]);
	for (i = 0; i < A_MAX; ++i) savefile_add_s16b(p_ptr->stat_cur[i]);

	/* Player's Gold and Social Class */
	savefile_add_u32b(p_ptr->au);
	savefile_add_s16b(p_ptr->sc);

	/* Maximum and current experience */
	savefile_add_u32b(p_ptr->max_exp);
	savefile_add_u32b(p_ptr->exp);
	savefile_add_u16b(p_ptr->exp_frac);

	/* Character level */
	savefile_add_s16b(p_ptr->lev);

	/* Maximum and current HPs */
	savefile_add_s16b(p_ptr->mhp);
	savefile_add_s16b(p_ptr->chp);
	savefile_add_u16b(p_ptr->chp_frac);

	/* Maximum and current Mana */
	savefile_add_s16b(p_ptr->msp);
	savefile_add_s16b(p_ptr->csp);
	savefile_add_u16b(p_ptr->csp_frac);

	/* Max Player and Dungeon Levels */
	savefile_add_s16b(p_ptr->max_lev);
	savefile_add_s16b(p_ptr->max_depth);

	/* Vaious states */
	savefile_add_s16b(p_ptr->blind);
	savefile_add_s16b(p_ptr->paralyzed);
	savefile_add_s16b(p_ptr->confused);
	savefile_add_s16b(p_ptr->food);
	savefile_add_s16b(p_ptr->energy);
	savefile_add_s16b(p_ptr->fast);
	savefile_add_s16b(p_ptr->slow);
	savefile_add_s16b(p_ptr->afraid);
	savefile_add_s16b(p_ptr->cut);
	savefile_add_s16b(p_ptr->stun);
	savefile_add_s16b(p_ptr->poisoned);
	savefile_add_s16b(p_ptr->image);
	savefile_add_s16b(p_ptr->protevil);
	savefile_add_s16b(p_ptr->invuln);
	savefile_add_s16b(p_ptr->hero);
	savefile_add_s16b(p_ptr->shero);
	savefile_add_s16b(p_ptr->shield);
	savefile_add_s16b(p_ptr->blessed);
	savefile_add_s16b(p_ptr->tim_invis);
	savefile_add_s16b(p_ptr->word_recall);
	savefile_add_s16b(p_ptr->see_infra);
	savefile_add_s16b(p_ptr->tim_infra);

	/* Timed resistances */
	for (i = 0; i < RES_MAX; i++) savefile_add_byte(p_ptr->resist_timed[i]);

	savefile_add_byte(p_ptr->confusing);
	savefile_add_byte(p_ptr->searching);

	/* Random artifact version */
	savefile_add_u32b(RANDART_VERSION);

	/* Special stuff */
	savefile_add_u16b(p_ptr->panic_save);
	savefile_add_u16b(p_ptr->total_winner);
	savefile_add_u16b(p_ptr->noscore);

	/* Write death */
	savefile_add_byte(p_ptr->is_dead);

	/* Write feeling */
	savefile_add_byte(feeling);

	/* Turn of last "feeling" */
	savefile_add_s32b(old_turn);

	/* Current turn */
	savefile_add_s32b(turn);

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the RNG block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_rng(void)
{
	int i;

	/* Add the "place" */
	savefile_add_u16b(Rand_place);

	/* State */
	for (i = 0; i < RAND_DEG; i++) savefile_add_u32b(Rand_state[i]);

	/* Add the random artifact seed */
	savefile_add_u32b(seed_randart);

	/* Add the object seeds */
	savefile_add_u32b(seed_flavor);
	savefile_add_u32b(seed_town);

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the player's messages block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_messages(void)
{
	int i, numtowr;

	/* Get the number of messages to print */
	numtowr = message_num();

	/* Trim messages if requested */
	if (compress_savefile && (numtowr > 40)) numtowr = 40;

	/* Add a counter to the block */
	savefile_add_u16b(numtowr);

	/* Dump the messages, oldest first */
	for (i = numtowr - 1; i >= 0; i--)
	{
		/* Write the string */
		savefile_add_string((char *) message_str((s16b) i), TRUE);

		/* Write the type */
		savefile_add_u16b(message_type((s16b) i));
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the random artifacts.
 * --------------------------------------------------------------------- */
static void savefile_do_block_randarts(void)
{
	int i;

	/* Write the number of randarts */
	if (adult_rand_artifacts) savefile_add_u16b(z_info->a_max);
	else {savefile_add_u16b(0); return;}

	/* Write all randart data */
	for (i = 0; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];

		/* tval/sval/pval */
		savefile_add_byte(a_ptr->tval);
		savefile_add_byte(a_ptr->sval);
		savefile_add_s16b(a_ptr->pval);

		/* Bonuses */
		savefile_add_s16b(a_ptr->to_h);
		savefile_add_s16b(a_ptr->to_d);
		savefile_add_s16b(a_ptr->to_a);
		savefile_add_s16b(a_ptr->ac);

		/* Base damage dice */
		savefile_add_byte(a_ptr->dd);
		savefile_add_byte(a_ptr->ds);

		/* Weight */
		savefile_add_s16b(a_ptr->weight);

		/* Cost */
		savefile_add_s32b(a_ptr->cost);

		/* Flags */
		savefile_add_u32b(a_ptr->flags1);
		savefile_add_u32b(a_ptr->flags2);
		savefile_add_u32b(a_ptr->flags3);

		/* Level/Rarity */
		savefile_add_byte(a_ptr->level);
		savefile_add_byte(a_ptr->rarity);

		/* Activation, timeout and randtime (?) */
		savefile_add_byte(a_ptr->activation);
		savefile_add_u16b(a_ptr->time);
		savefile_add_u16b(a_ptr->randtime);
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
 * --------------------------------------------------------------------- */
static void savefile_do_block_dungeon(void)
{
	int i, y, x;

	byte tmp8u;

	byte count;
	byte prev_char;

	/*** Basic info ***/

	/* Dungeon specific info follows */
	savefile_add_u16b(p_ptr->depth);
	savefile_add_u16b(p_ptr->py);
	savefile_add_u16b(p_ptr->px);
	savefile_add_u16b(DUNGEON_HGT);
	savefile_add_u16b(DUNGEON_WID);

	/*** Simple "Run-Length-Encoding" of cave ***/

	/* Note that this will induce two wasted bytes */
	count = 0;
	prev_char = 0;

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
				savefile_add_byte((byte) count);
				savefile_add_byte((byte) prev_char);
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
		savefile_add_byte((byte) count);
		savefile_add_byte((byte) prev_char);
	}


	/*** Simple "Run-Length-Encoding" of cave ***/

	/* Note that this will induce two wasted bytes */
	count = 0;
	prev_char = 0;

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
				savefile_add_byte((byte)count);
				savefile_add_byte((byte)prev_char);
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
		savefile_add_byte((byte) count);
		savefile_add_byte((byte) prev_char);
	}


	/*** Compact ***/

	/* Compact the objects */
	compact_objects(0);

	/* Compact the monsters */
	compact_monsters(0);


	/*** Dump objects ***/

	/* Total objects */
	savefile_add_u16b(o_max);

	/* Dump the objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Dump it */
		savefile_helper_item(o_ptr);
	}


	/*** Dump the monsters ***/

	/* Total monsters */
	savefile_add_u16b(m_max);

	/* Dump the monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Dump it */
		savefile_helper_monster(m_ptr);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the monster lore block.
 * --------------------------------------------------------------------- */
 static void savefile_do_block_monlore(void)
 {
	int i;

	/* Write the count */
	savefile_add_u32b((u32b) z_info->r_max);

	/* Write the individual entries */
	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Count sights/deaths/kills */
		savefile_add_s16b(l_ptr->sights);
		savefile_add_s16b(l_ptr->deaths);
		savefile_add_s16b(l_ptr->pkills);
		savefile_add_s16b(l_ptr->tkills);

		/* Count wakes and ignores */
		savefile_add_byte(l_ptr->wake);
		savefile_add_byte(l_ptr->ignore);

		/* Extra stuff */
		savefile_add_byte(l_ptr->xtra1);
		savefile_add_byte(l_ptr->xtra2);

		/* Count drops */
		savefile_add_byte(l_ptr->drop_gold);
		savefile_add_byte(l_ptr->drop_item);

		/* Count spells */
		savefile_add_byte(l_ptr->cast_innate);
		savefile_add_byte(l_ptr->cast_spell);

		/* Count blows of each type */
		for (i = 0; i < MONSTER_BLOW_MAX; i++)
			savefile_add_byte(l_ptr->blows[i]);

		/* Memorize flags */
		savefile_add_u32b(l_ptr->flags1);
		savefile_add_u32b(l_ptr->flags2);
		savefile_add_u32b(l_ptr->flags3);
		savefile_add_u32b(l_ptr->flags4);
		savefile_add_u32b(l_ptr->flags5);
		savefile_add_u32b(l_ptr->flags6);

		/* Monster limit per level */
		savefile_add_byte(r_ptr->max_num);
	}

	/* We are done. */
	return;
}


/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the "object lore" block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_objlore(void)
{
	int i;

	/* Write the count */
	savefile_add_u32b((u32b) z_info->k_max);

	/* Write the individual entries */
	for (i = 0; i < z_info->r_max; i++)
	{
		byte temp = 0;
		object_kind *k_ptr = &k_info[i];

		/* Evaluate */
		if (k_ptr->aware) temp |= 0x01;
		if (k_ptr->tried) temp |= 0x02;

		/* Write the byte */
		savefile_add_byte(temp);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the artifacts block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_artifacts(void)
{
	int i;

	/* Write the count */
	savefile_add_u32b(z_info->a_max);

	/* Write the individual entries */
	for (i = 0; i < z_info->a_max; i++)
	{
  	artifact_type *a_ptr = &a_info[i];
		savefile_add_byte(a_ptr->cur_num);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the quests block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_quests(void)
{
	int i;

	/* Write the count */
	savefile_add_u32b(MAX_Q_IDX);

	/* Write the individual entries */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		savefile_add_byte(q_list[i].level);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the store block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_stores(void)
{
	int i;                         

	/* Write the count */
	savefile_add_u32b(MAX_STORES);

	/* Write the individual entries */
	for (i = 0; i < MAX_STORES; i++)
	{
		savefile_helper_store(&store[i]);
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


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

#define DEBUGGING

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
#define BLOCK_TYPE_END           0
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
#define BLOCK_TYPE_INVENTORY    13

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
#define BLOCK_VERSION_INVENTORY  1

/* The smallest block we can have */
#define BLOCK_INCREMENT         32

/* The size of the header (in bytes) */
#define BLOCK_HEAD_SIZE         10

/* The two possible "operations" */
#define PUT    FALSE
#define GET    TRUE

/* Variables needed by the code */
static byte savefile_head_type;  /* The type of block */
static int  savefile_head_ver;   /* The version of this block */
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

#ifdef DEBUGGING
	printf("%s\n", msg);
#endif

	/* We are done */
	return;
}

/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
static bool wearable_p(const object_type *o_ptr)
{
	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BELT:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_LITE:
		case TV_ORB:
		case TV_AMULET:
		case TV_RING:
		{
			return (TRUE);
		}
	}

	/* Nope */
	return (FALSE);
}

/*
 * All generic savefile "block" adding functions
 */

/*
 * This function gets/puts a byte to the savefile block.
 * If more memory is needed, it allocates more memory, and makes
 * savefile_block point to it.
 */
static byte savefile_do_byte(byte *v, bool type)
{
	/* See if we need to allocate more memory */
	if ((savefile_blocksize == savefile_blockused) && (type == PUT))
	{
		byte *savefile_new;

		/* Make space for the new block. */
		savefile_new = C_RNEW(savefile_blocksize + BLOCK_INCREMENT, byte);

		/* Copy the memory across */
		C_COPY(savefile_new, savefile_block, savefile_blocksize, byte);

		/* Wipe the old block. */
		KILL(savefile_block);

		/* The new block is now the savefile block. */
		savefile_block = savefile_new;

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
		v = &savefile_block[savefile_blockused];
	}

	/* Increase the "used" counter */
	savefile_blockused++;

	/* We are done */
 	return(*v);
}

/*
 * This function gets/puts an unsigned 16-bit integer.
 */
static u16b savefile_do_u16b(u16b *v, bool type)
{
	byte i;

	if (type == PUT)
	{
		i = ((*v) & 0xFF);
		savefile_do_byte(&i, PUT);

		i = (((*v) >> 8) & 0xFF);
		savefile_do_byte(&i, PUT);
	}
	else
	{
		savefile_do_byte(&i, GET);
		(*v) = i;

		savefile_do_byte(&i, GET);
		(*v) |= (i << 8);
	}

	/* We are done */
	return (*v);
}

/*
 * This function gets/puts an unsigned 32-bit integer.
 */
static u32b savefile_do_u32b(u32b *v, bool type)
{
	byte i;

	if (type == PUT)
	{
		i = ((*v) & 0xFF);
		savefile_do_byte(&i, PUT);

		i = (((*v) >> 8) & 0xFF);
		savefile_do_byte(&i, PUT);

		i = (((*v) >> 16) & 0xFF);
		savefile_do_byte(&i, PUT);

		i = (((*v) >> 24) & 0xFF);
		savefile_do_byte(&i, PUT);
	}
	else
	{
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
	return (*v);
}

/*
 * Either put or get a string.  Maximum length is 255 characters,
 * Behaviour for longer strings is undefined.
 */
static char *savefile_do_string(char *str, bool type)
{
	byte i = 255;

	/* Counter */
	int n = 0;

	/* Count */
	if (type == PUT) i = (byte) strlen(str) + 1;

	/* Put/Get the length of the string first */
	savefile_do_byte(&i, type);

	/* Put/Get the string, one byte at a time */
	for (n = 0; n < i; n++)
	{
		/* Put the byte */
		savefile_do_byte((byte *) &str[n], type);
	}

#ifdef DEBUGGING
	printf("String done: %s\n", str);
#endif

	/* We are done */
	return (str);
}

/* Here we define space-saving macros */
#define savefile_do_s16b(v, t)     (s16b) savefile_do_u16b((u16b *) v, t);
#define savefile_do_s32b(v, t)     (s32b) savefile_do_u32b((u32b *) v, t);


/*
 * Functions to deal with creating, writing, and freeing "blocks".
 */

/*
 * This function starts a new block.
 */
static void savefile_new_block(void)
{
	/* Make the block */
	C_MAKE(savefile_block, BLOCK_INCREMENT, byte);

	/* Initialise the values */
	savefile_blocksize = BLOCK_INCREMENT;
	savefile_blockused = 0;

#ifdef DEBUGGING
	printf("New block created.\n");
#endif

	return;
}

/*
 * This function writes a block to the savefile.
 */
static void savefile_write_block(int fd, byte type, byte ver)
{
	int pos = 0;
	byte *savefile_head;

#ifdef DEBUGGING
	printf("Block being written to the savefile: ver = %i, type = %i\n", ver, type);
#endif

	/* Make the header */
	C_MAKE(savefile_head, BLOCK_HEAD_SIZE, byte);

	/* Create the header - first, the type */
	savefile_head[pos++] = (type & 0xFF);
	savefile_head[pos++] = ((type >> 8) & 0xFF);

	/* Add the version of this block. */
	savefile_head[pos++] = (ver & 0xFF);
	savefile_head[pos++] = ((ver >> 8) & 0xFF);

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

#ifdef DEBUGGING
	printf("Block size is %i\n", (int) savefile_blockused);
#endif

	/* Write the block */
	if (savefile_block)
	{
		printf("We really are saving this bit.\n");
		fd_write(fd, (cptr) savefile_block, savefile_blockused);
	}

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
static errr savefile_helper_item(object_type *o_ptr, bool type)
{
	int temp_kind;
	object_kind *k_ptr;

	byte temp;

	byte old_dd = 0;
	byte old_ds = 0;

	u32b f1, f2, f3;

	/* tval/sval/pval */
	savefile_do_byte(&o_ptr->tval, type);
	savefile_do_byte(&o_ptr->sval, type);
	savefile_do_s16b(&o_ptr->pval, type);

	/* Look it up */
	temp_kind = lookup_kind(o_ptr->tval, o_ptr->sval);
	o_ptr->k_idx = temp_kind;

	/* Write the "kind" index */
	savefile_do_s16b(&o_ptr->k_idx, type);

	/* Restore the value */
	o_ptr->k_idx = temp_kind;

	/* Paranoia */
	if ((o_ptr->k_idx < 0) || (o_ptr->k_idx >= z_info->k_max))
		return (-1);

	/* Obtain the "kind" template */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Location in the dungeon */
	savefile_do_byte(&o_ptr->iy, type);
	savefile_do_byte(&o_ptr->ix, type);

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

	/* Do the inscription */
	if (o_ptr->note && type == PUT) temp = TRUE;
	else if (type == PUT) temp = FALSE;

	savefile_do_byte(&temp, type);

	/* Inscription? */
	if (temp)
	{
		char buffer[128];

		/* Get the note */
		if (type == PUT) sprintf(buffer, "%s", (char *) quark_str(o_ptr->note));

		/* Get the inscription */
		savefile_do_string((char *) &buffer, type);

		/* Add the note */
		if (type == GET) o_ptr->note = quark_add(buffer);
	}

	/* We might be done. */
	if (type == PUT) return (0);

	/* Hack -- notice "broken" items */
	if (k_ptr->cost <= 0) o_ptr->ident |= (IDENT_BROKEN);

	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Get the correct fields */
		o_ptr->to_h = k_ptr->to_h;
		o_ptr->to_d = k_ptr->to_d;
		o_ptr->to_a = k_ptr->to_a;

		/* Get the correct fields */
		o_ptr->ac = k_ptr->ac;
		o_ptr->dd = k_ptr->dd;
		o_ptr->ds = k_ptr->ds;

		/* Get the correct weight */
		o_ptr->weight = k_ptr->weight;

		/* Paranoia */
		o_ptr->name1 = o_ptr->name2 = 0;

		/* All done */
		return (0);
	}

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);


	/* Paranoia */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Paranoia */
		if (o_ptr->name1 >= z_info->a_max) return (-1);

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Verify that artifact */
		if (!a_ptr->name) o_ptr->name1 = 0;
	}

	/* Paranoia */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Paranoia */
		if (o_ptr->name2 >= z_info->e_max) return (-1);

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Verify that ego-item */
		if (!e_ptr->name) o_ptr->name2 = 0;
	}


	/* Get the standard fields */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Get the standard weight */
	o_ptr->weight = k_ptr->weight;

	/* Hack -- extract the "broken" flag */
	if (o_ptr->pval < 0) o_ptr->ident |= (IDENT_BROKEN);


	/* Artifacts */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Get the new artifact "pval" */
		o_ptr->pval = a_ptr->pval;

		/* Get the new artifact fields */
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;

		/* Get the new artifact weight */
		o_ptr->weight = a_ptr->weight;

		/* Hack -- extract the "broken" flag */
		if (!a_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);
	}

	/* Ego items */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Hack -- keep some old fields */
		if ((o_ptr->dd < old_dd) && (o_ptr->ds == old_ds))
		{
			/* Keep old boosted damage dice */
			o_ptr->dd = old_dd;
		}

		/* Hack -- extract the "broken" flag */
		if (!e_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

		/* Hack -- enforce legal pval */
		if (e_ptr->flags1 & (TR1_PVAL_MASK))
		{
			/* Force a meaningful pval */
			if (!o_ptr->pval) o_ptr->pval = 1;
		}

		/* Mega-Hack - Enforce the special broken items */
		if ((o_ptr->name2 == EGO_BLASTED) ||
			(o_ptr->name2 == EGO_SHATTERED))
		{
			/* These were set to k_info values by preceding code */
			o_ptr->ac = 0;
			o_ptr->dd = 0;
			o_ptr->ds = 0;
		}
	}

	/* We are done. */
	return (0);
}

/*
 * Add a monster record to the savefile.
 */
static void savefile_helper_monster(monster_type *m_ptr, bool type)
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
static void savefile_helper_store(store_type *st_ptr, bool type)
{
	int i, j;

	/* Save the "open" counter */
	savefile_do_s32b(&st_ptr->store_open, type);

	/* Save the "insults" */
	savefile_do_s16b(&st_ptr->insult_cur, type);

	/* Save the current owner */
	savefile_do_byte(&st_ptr->owner, type);

	/* Paranoia */
	if (st_ptr->owner >= z_info->b_max)
	{
		/* Whoops */
		note("Illegal store owner!", type);

		/* Reset it */
		st_ptr->owner = 0;
	}

	/* Save the stock size */
	savefile_do_byte(&st_ptr->stock_num, type);

	/* Save the "haggle" info */
	savefile_do_s16b(&st_ptr->good_buy, type);
	savefile_do_s16b(&st_ptr->bad_buy, type);

	/* Oo the stock */
	if (type == PUT)
	{
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			/* Save each item in stock */
			savefile_helper_item(&st_ptr->stock[i], type);
		}
	}
	else
	{
		/* Read the items */
		for (j = 0; j < st_ptr->stock_num; j++)
		{
			object_type *i_ptr;
			object_type object_type_body;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Wipe the object */
			object_wipe(i_ptr);

			/* Read the item */
			savefile_helper_item(i_ptr, type);

			/* Accept any valid items */
			if (st_ptr->stock_num < STORE_INVEN_MAX)
			{
				int k = st_ptr->stock_num++;

				/* Accept the item */
				object_copy(&st_ptr->stock[k], i_ptr);
			}
		}
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
	int pos = 0;
	byte *block;

#ifdef DEBUGGING
	printf("savefile_start() called.\n");
#endif

	/* Make the block */
	C_MAKE(block, 6, byte);

	/* Start with the variant "stamp" */
	block[pos++] = 83;
	block[pos++] = 97;
	block[pos++] = 118;
	block[pos++] = 101;

	/* Now, dump the versions */
	block[pos++] = SAVEFILE_VERSION_MAJOR;
	block[pos++] = SAVEFILE_VERSION_MINOR;

#ifdef DEBUGGING
	printf("Block is: %s\n", (char *) block);
#endif

	/* Write the header */
	fd_write(fd, (cptr) block, 6);

	/* Wipe the block */
	KILL(block);

#ifdef DEBUGGING
	printf("savefile_start() returning.\n");
#endif

	/* We are done. */
	return;
}

/*
 * Write information about this variant.
 */
static void savefile_do_block_header(bool type, int ver)
{
	char v_name[32];
	byte v_j, v_m, v_p, v_x;

#ifdef DEBUGGING
	printf("savefile_block_header()\n");
#endif

	if (type == PUT)
	{
		strcpy(v_name, VERSION_NAME);
		v_j = VERSION_MAJOR;
		v_m = VERSION_MINOR;
		v_p = VERSION_PATCH;
		v_x = VERSION_EXTRA;
	}

	/* Add the variant string */
	savefile_do_string(&v_name, type);

	/* Add the version number */
	savefile_do_byte(&v_j, type);
	savefile_do_byte(&v_m, type);
	savefile_do_byte(&v_p, type);
	savefile_do_byte(&v_x, type);

	/* Tell the user about the savefile */
	note(format("Loading a %s %i.%i.%i savefile.", v_name, v_j, v_m, v_p), type);

#ifdef DEBUGGING
	printf("returning ...\n");
#endif

	/* We are done. */
	return;
}

/*
 * Write the player's options.
 */
static void savefile_do_block_options(bool type, int ver)
{
	int n = 0;

#ifdef DEBUGGING
	printf("savefile_do_block_options()\n");
#endif

	/* Add delay factor and hitpoint warning "special options" */
	savefile_do_byte(&op_ptr->delay_factor, type);
	savefile_do_byte(&op_ptr->hitpoint_warn, type);

	/* Add the "normal" options */
	for (n = 0; n < OPT_MAX; n++)
		savefile_do_byte((bool *) &op_ptr->opt[n], type);

	/* Add the "window" options */
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
		savefile_do_u32b(&op_ptr->window_flag[n], type);

#ifdef DEBUGGING
	printf("returning ...\n");
#endif

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the player block.
 * --------------------------------------------------------------------- */
static errr savefile_do_block_player(bool type, int ver)
{
	int i;

	/* Add number of past lives */
	savefile_do_u16b(&sf_lives, type);

	/* Set the level */
	if (type == PUT) i = PY_MAX_LEVEL;

	/* Do the numebr of HP entries */
	savefile_do_byte((byte *) &i, type);

	/* Incompatible save files */
	if (i > PY_MAX_LEVEL)
	{
		note(format("Too many (%u) hitpoint entries!", i), type);
		return (-1);
	}

	/* Do the entries */
	for (i = 0; i < PY_MAX_LEVEL; i++)
		savefile_do_s16b(&p_ptr->player_hp[i], type);

	/* Do spell data */
	savefile_do_u32b(&p_ptr->spell_learned1, type);
	savefile_do_u32b(&p_ptr->spell_learned2, type);
	savefile_do_u32b(&p_ptr->spell_worked1, type);
	savefile_do_u32b(&p_ptr->spell_worked2, type);
	savefile_do_u32b(&p_ptr->spell_forgotten1, type);
	savefile_do_u32b(&p_ptr->spell_forgotten2, type);

#if 0
	/* The magic spells were changed drastically in Angband 2.9.7 */
	if (older_than(2, 9, 7) &&
	    (c_info[p_ptr->pclass].spell_book == TV_MAGIC_BOOK))
	{
		/* Discard old spell info */
		strip_bytes(24);

		/* Discard old spell order */
		strip_bytes(PY_MAX_SPELLS);

		/* None of the spells have been learned yet */
		for (i = 0; i < PY_MAX_SPELLS; i++)
			p_ptr->spell_order[i] = 99;
	}
#endif

	/* Do the ordered spells */
	for (i = 0; i < PY_MAX_SPELLS; i++)
		savefile_do_byte(&p_ptr->spell_order[i], type);

	/* What the player's name is */
	savefile_do_string(op_ptr->full_name, type);

	/* What the player died from */
	savefile_do_string(p_ptr->died_from, type);

#if 0
	/* Player's history */
	savefile_do_string(p_ptr->history, type);
#endif

	/* Sex */
	savefile_do_byte(&p_ptr->psex, type);

	/* Race */
	savefile_do_byte(&p_ptr->prace, type);

	/* Verify player race */
	if (p_ptr->prace >= z_info->p_max)
	{
		/* Warn the user */
		note(format("Invalid player race - %d! Resetting to 0.", p_ptr->prace), type);

		/* Reset */
		p_ptr->prace = 0;
	}

	/* Class */
	savefile_do_byte(&p_ptr->pclass, type);

	/* Verify player class */
	if (p_ptr->pclass >= z_info->c_max)
	{
		/* Warn the user */
		note(format("Invalid player class - %d! Resetting to 0..", p_ptr->pclass), type);

		/* Reset */
		p_ptr->pclass = 0;
	}
    
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
	savefile_do_s32b(&p_ptr->au, type);
	savefile_do_s16b(&p_ptr->sc, type);

	/* Maximum and current experience */
	savefile_do_s32b(&p_ptr->max_exp, type);
	savefile_do_s32b(&p_ptr->exp, type);
	savefile_do_u16b(&p_ptr->exp_frac, type);

	/* Character level */
	savefile_do_s16b(&p_ptr->lev, type);

    note(format("Player level is %i", p_ptr->lev), type);
    
	/* Verify player level */
	if ((p_ptr->lev < 1) || (p_ptr->lev > PY_MAX_LEVEL))
	{
		/* Warn the user */
		note(format("Invalid player level - %d! Resetting to 1.", p_ptr->lev), type);

		/* Reset */
		p_ptr->lev = 1;
	}

	/* Max Player and Dungeon Levels */
	savefile_do_s16b(&p_ptr->max_lev, type);
	savefile_do_s16b(&p_ptr->max_depth, type);

	/* Repair maximum player level */
	if (p_ptr->max_lev < p_ptr->lev) p_ptr->max_lev = p_ptr->lev;

	/* Repair maximum dungeon level */
	if (p_ptr->max_depth < 0) p_ptr->max_depth = 1;

	/* Maximum and current HPs */
	savefile_do_s16b(&p_ptr->mhp, type);
	savefile_do_s16b(&p_ptr->chp, type);
	savefile_do_u16b(&p_ptr->chp_frac, type);

	/* Maximum and current Mana */
	savefile_do_s16b(&p_ptr->msp, type);
	savefile_do_s16b(&p_ptr->csp, type);
	savefile_do_u16b(&p_ptr->csp_frac, type);


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

	/* Special stuff */
	savefile_do_u16b(&p_ptr->panic_save, type);
	savefile_do_u16b(&p_ptr->total_winner, type);
	savefile_do_u16b(&p_ptr->noscore, type);

	/* Write death */
	savefile_do_byte((byte *) &p_ptr->is_dead, type);

	/* Write feeling */
	savefile_do_byte(&feeling, type);

	/* Turn of last "feeling" */
	savefile_do_s32b(&old_turn, type);

	/* Current turn */
	savefile_do_s32b(&turn, type);

	/* We are done. */
	return (0);
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the RNG block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_rng(bool type, int ver)
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

	/* XXX */
	Rand_quick = FALSE;

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the player's messages block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_messages(bool type, int ver)
{
	int i = 0;
	s16b num = 0;

	/* Get the number of messages to print */
	if (type == PUT)
	{
		num = message_num();

		/* Trim messages if requested */
		if (compress_savefile && (num > 40)) num = 40;
	}

	/* Get the counter a counter to the block */
	savefile_do_s16b(&num, type);

	/* Dump the messages, oldest first */
	for (i = num - 1; i >= 0; i--)
	{
		char buffer[512];
		u16b mess_type = 0;

		/* Do some special stuff */
		if (type == PUT)
		{
			cptr temp;

			/* XXX */
			temp = message_str((s16b) i);
			strcpy(buffer, temp);

			/* Get the message type */
			mess_type = message_type((s16b) i);
		}

		/* Write the string */
		savefile_do_string((char *) &buffer, type);

		/* Write the type */
		savefile_do_u16b(&mess_type, type);

		/* Save to message to the buffer */
		if (type == GET) message_add((char *) &buffer, mess_type);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the random artifacts.
 * --------------------------------------------------------------------- */
static errr savefile_do_block_randarts(bool type, int ver)
{
	int i = 0;
	u32b n = 0;

	/* Grab the version */
	if (type == PUT) n = RANDART_VERSION;

	/* Put/Get the version */
	savefile_do_u32b(&n, type);

	/* Paranoia */
	if ((n != RANDART_VERSION) && adult_rand_artifacts)
	{
		if (type == GET)
		{
			char ch = 0;

			note(format("Random artifact version has changed; this may break savefiles. Continue? (y/n)"), type);

			ch = inkey();

			if (tolower(ch) != 'y')
			{
				note(format("Aborting loading due to user request."), type);
				return (-1);
			}
		}
	}

	/* Grab the count */
	if (type == PUT) n = z_info->a_max;

	/* Put/Get the count */
	savefile_do_u32b(&n, type);

	if (!adult_rand_artifacts) return (0);

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

	if (type == GET)
	{
		/* Initialize only the randart names */
		do_randart(seed_randart, FALSE);
	}

	/* We are done. */
	return (0);
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
static errr savefile_do_block_dungeon(bool type, int ver)
{
	int i, y, x;
	u16b xmax = 0, ymax = 0;
	u16b py, px;

	byte tmp8u;

	u16b limit;

	byte count = 0, prev_char = 0;

	/*** Basic info ***/

	/* Dungeon depth */
	savefile_do_s16b(&p_ptr->depth, type);

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
		/* if (ver < x && p_ptr->depth == 0) character_dungeon = FALSE; else */
		character_dungeon = TRUE;
	}

	/* We are done. */
	return (0);
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the monster lore block.
 * --------------------------------------------------------------------- */
static errr savefile_do_block_monlore(bool type, int ver)
{
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = z_info->r_max;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) &n, type);

	/* Check incompatibilities */
	if (type == GET && n != z_info->r_max)
	{
		/* Warn the user */
		note("Savefile has too many monsters. Not loading monster lore.", type);

		/* Error, but we ignored it. */
		return (0);
	}

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

		/* Repair the lore flags */
		if (type == GET)
		{
			/* Repair the lore flags */
			l_ptr->flags1 &= r_ptr->flags1;
			l_ptr->flags2 &= r_ptr->flags2;
			l_ptr->flags3 &= r_ptr->flags3;
			l_ptr->flags4 &= r_ptr->flags4;
			l_ptr->flags5 &= r_ptr->flags5;
			l_ptr->flags6 &= r_ptr->flags6;
		}
	}

	/* We are done. */
	return (0);
}


/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the "object lore" block.
 * --------------------------------------------------------------------- */
static errr savefile_do_block_objlore(bool type, int ver)
{
	int i = 0, n = 0;

	/* Grab the count */
	if (type == PUT) n = z_info->k_max;

	/* Put/Get the count */
	savefile_do_u32b((u32b *) &n, type);

	/* Check incompatibilities */
	if (type == GET && n != z_info->k_max)
	{
		/* Warn the user */
		note("Savefile has too many objects. Not loading object lore.", type);

		/* Error, but we ignored it. */
		return (0);
	}

	/* Write the individual entries */
	for (i = 0; i < n; i++)
	{
		byte temp = 0;
		object_kind *k_ptr = &k_info[i];

		if (type == PUT)
		{
			/* Evaluate */
			if (k_ptr->aware) temp |= 0x01;
			if (k_ptr->tried) temp |= 0x02;

			/* Write the byte */
			savefile_do_byte(&temp, type);
		}
		else
		{
			/* Write the byte */
			savefile_do_byte(&temp, type);

			k_ptr->aware = (temp & 0x01) ? TRUE : FALSE;
			k_ptr->tried = (temp & 0x02) ? TRUE : FALSE;
		}
	}

	/* We are done. */
	return (0);
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * Do the artifacts block.
 * --------------------------------------------------------------------- */
static void savefile_do_block_artifacts(bool type, int ver)
{
	int i = 0;
	u32b n = 0;

	/* Grab the count */
	if (type == PUT) n = z_info->a_max;

	/* Put/Get the count */
	savefile_do_u32b(&n, type);

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
static void savefile_do_block_quests(bool type, int ver)
{
	int i = 0;
	u32b n = 0;

	/* Grab the count */
	if (type == PUT) n = MAX_Q_IDX;

	/* Put/Get the count */
	savefile_do_u32b(&n, type);

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
static void savefile_do_block_stores(bool type, int ver)
{
	int i = 0;
	u32b n = 0;

	/* Grab the count */
	if (type == PUT) n = MAX_STORES;

	/* Put/Get the count */
	savefile_do_u32b(&n, type);

	/* Write the individual entries */
	for (i = 0; i < n; i++)
	{
		savefile_helper_store(&store[i], type);
	}

	/* We are done. */
	return;
}

/* -------------------------------------------- takkaria, 2002-04-21 ---
 * Do the player's inventory.
 * --------------------------------------------------------------------- */
static errr savefile_do_block_inventory(bool type, int ver)
{
	u16b i = 0;
	int slot = 0;
	object_type *o_ptr = NULL, object_type_body;

	/* Write the inventory */
	while (1)
	{
		/* Hack */
		if (type == PUT)
		{
			if (i++ > INVEN_TOTAL) break;

			/* Point to the object */
			o_ptr = &inventory[i];

			/* Skip non-objects */
			if (!o_ptr->k_idx) continue;
		}

		/* Dump index */
		savefile_do_u16b(&i, type);

		/* Check if we should finish */
		if (type == GET && i == 0xFFFF) break;

		/* Do some stuff */
		if (type == GET)
		{
			/* Get local object */
			o_ptr = &object_type_body;

			/* Wipe the object */
			object_wipe(o_ptr);
		}

		/* Do object */
		savefile_helper_item(o_ptr, type);

		if (type == GET)
		{
			int n = 0;

			/* Hack -- verify item */
			if (!o_ptr->k_idx) return (-1);

			/* Verify slot */
			if (n >= INVEN_TOTAL) return (-1);

			/* Wield equipment */
			if (n >= INVEN_WIELD)
			{
				/* Copy object */
				object_copy(&inventory[n], o_ptr);

				/* Add the weight */
				p_ptr->total_weight += (o_ptr->number * o_ptr->weight);

				/* One more item */
				p_ptr->equip_cnt++;
			}

			/* Warning -- backpack is full */
			else if (p_ptr->inven_cnt == INVEN_PACK)
			{
				/* Oops */
				note("Too many items in the inventory!", type);

				/* Fail */
				return (-1);
			}

			/* Carry inventory */
			else
			{
				/* Get a slot */
				n = slot++;

				/* Copy object */
				object_copy(&inventory[n], o_ptr);

				/* Add the weight */
				p_ptr->total_weight += (o_ptr->number * o_ptr->weight);

				/* One more item */
				p_ptr->inven_cnt++;
			}
		}
	}

	/* We are done. */
	return (0);
}

/* -------------------------------------------- takkaria, 2002-04-18 ---
 * The actual code to do the savefile.
 * --------------------------------------------------------------------- */
static bool write_savefile(int fd)
{
	/* Start the savefile */
	savefile_start(fd);

	printf("write_savefile()\n");

	/* Put information about the variant that saved the file */
	savefile_new_block();
	savefile_do_block_header(PUT, BLOCK_VERSION_HEADER);
	savefile_write_block(fd, BLOCK_TYPE_HEADER, BLOCK_VERSION_HEADER);
	printf("doing header\n");

	/* Do the player options */
	savefile_new_block();
	savefile_do_block_options(PUT, BLOCK_VERSION_OPTIONS);
	savefile_write_block(fd, BLOCK_TYPE_OPTIONS, BLOCK_VERSION_OPTIONS);
	printf("doing options\n");

	/* Do the player information */
	savefile_new_block();
	savefile_do_block_player(PUT, BLOCK_VERSION_PLAYER);
	savefile_write_block(fd, BLOCK_TYPE_PLAYER, BLOCK_VERSION_PLAYER);
	printf("doing player\n");

	/* Do the inventory */
	savefile_new_block();
	savefile_do_block_inventory(PUT, BLOCK_VERSION_INVENTORY);
	savefile_write_block(fd, BLOCK_TYPE_INVENTORY, BLOCK_VERSION_INVENTORY);
	printf("doing inventory\n");

	/* Do the RNG state */
	savefile_new_block();
	savefile_do_block_rng(PUT, BLOCK_VERSION_RNG);
	savefile_write_block(fd, BLOCK_TYPE_RNG, BLOCK_VERSION_RNG);
	printf("doing RNG\n");

	/* Write player messages */
	savefile_new_block();
	savefile_do_block_messages(PUT, savefile_head_ver);
	savefile_write_block(fd, BLOCK_TYPE_MESSAGES, BLOCK_VERSION_MESSAGES);

	/* Write Monster Lore */
	savefile_new_block();
	savefile_do_block_monlore(PUT, BLOCK_VERSION_MONLORE);
	savefile_write_block(fd, BLOCK_TYPE_MONLORE, BLOCK_VERSION_MONLORE);

	/* Write "Object Lore" */
	savefile_new_block();
	savefile_do_block_objlore(PUT, BLOCK_VERSION_OBJLORE);
	savefile_write_block(fd, BLOCK_TYPE_OBJLORE, BLOCK_VERSION_OBJLORE);

	/* Write Quests */
	savefile_new_block();
	savefile_do_block_quests(PUT, BLOCK_VERSION_QUESTS);
	savefile_write_block(fd, BLOCK_TYPE_QUESTS, BLOCK_VERSION_QUESTS);

	/* Write Artifacts */
	savefile_new_block();
	savefile_do_block_artifacts(PUT, BLOCK_VERSION_ARTIFACTS);
	savefile_write_block(fd, BLOCK_TYPE_ARTIFACTS, BLOCK_VERSION_ARTIFACTS);

	/* Write the randarts */
	savefile_new_block();
	savefile_do_block_randarts(PUT, BLOCK_VERSION_RANDARTS);
	savefile_write_block(fd, BLOCK_TYPE_RANDARTS, BLOCK_VERSION_RANDARTS);

	/* Write the stores */
	savefile_new_block();
	savefile_do_block_stores(PUT, BLOCK_VERSION_STORES);
	savefile_write_block(fd, BLOCK_TYPE_STORES, BLOCK_VERSION_STORES);

	/* Write the dungeon */
	savefile_new_block();
	savefile_do_block_dungeon(PUT, BLOCK_VERSION_DUNGEON);
	savefile_write_block(fd, BLOCK_TYPE_DUNGEON, BLOCK_VERSION_DUNGEON);

    /* End the file */
    savefile_new_block();
    savefile_do_string("Eric! Eric! Eric! This ends the file!", PUT);
    savefile_write_block(fd, BLOCK_TYPE_END, 0);

#if 0
	/* End the savefile */
	savefile_write_block(NULL, fd);
#endif

	/* We are done */
	return (TRUE);
}

/* -------------------------------------------- takkaria, 2002-04-21 ---
 * Read a new savefile.
 * --------------------------------------------------------------------- */
static errr read_savefile(int fd)
{
	byte *savefile_head;
    bool finished = FALSE;

#ifdef DEBUGGING
	printf("read_savefile()\n");
#endif

	while (!finished)
	{
		byte i;
		byte temp;

		int pos = 0;
		int version;

		/* Allocate memory for the header */
		savefile_head = C_RNEW(BLOCK_HEAD_SIZE, byte);

		/* Read the data */
		fd_read(fd, (char *) savefile_head, BLOCK_HEAD_SIZE);

		/* Extract type from the header */
		temp = savefile_head[pos++];
		i = temp;
		temp = savefile_head[pos++];
		i |= ((u32b) temp << 8);
		savefile_head_type = i;

#ifdef DEBUGGING
		printf("savefile_head_type = %i\n",savefile_head_type);
#endif

		/* Get the version of the block */
		temp = savefile_head[pos++];
		i = temp;
		temp = savefile_head[pos++];
		i |= ((u32b) temp << 8);
		version = i;

#ifdef DEBUGGING
		printf("version = %i\n",version);
#endif

		/* Get the size of the block. */
		temp = savefile_head[pos++];
		i = temp;
		temp = savefile_head[pos++];
		i |= ((u32b) temp << 8);
		temp = savefile_head[pos++];
		i |= ((u32b) temp << 16);
		temp = savefile_head[pos++];
		i |= ((u32b) temp << 24);
		savefile_blocksize = i;

#ifdef DEBUGGING
		printf("savefile_blocksize = %i\n", (int) savefile_blocksize);
#endif

		/* Free memory for the header */
		KILL(savefile_head);

		/* Allocate memory for the block */
		savefile_block = C_RNEW(savefile_blocksize, byte);

		/* Reset the used amount */
		savefile_blockused = 0;

		/* Read the data */
		fd_read(fd, (char *) savefile_block, savefile_blocksize);

		/* Switch */
		switch (savefile_head_type)
		{
			case BLOCK_TYPE_HEADER:
				savefile_do_block_header(GET, version);
				break;
			case BLOCK_TYPE_OPTIONS:
				savefile_do_block_options(GET, version);
				break;
			case BLOCK_TYPE_PLAYER:
				savefile_do_block_player(GET, version);
				break;
			case BLOCK_TYPE_RNG:
				savefile_do_block_rng(GET, version);
				break;
			case BLOCK_TYPE_MESSAGES:
				savefile_do_block_messages(GET, version);
				break;
			case BLOCK_TYPE_MONLORE:
				savefile_do_block_monlore(GET, version);
				break;
			case BLOCK_TYPE_OBJLORE:
				savefile_do_block_objlore(GET, version);
				break;
			case BLOCK_TYPE_QUESTS:
				savefile_do_block_quests(GET, version);
				break;
			case BLOCK_TYPE_ARTIFACTS:
				savefile_do_block_artifacts(GET, version);
				break;
			case BLOCK_TYPE_RANDARTS:
				savefile_do_block_randarts(GET, version);
				break;
			case BLOCK_TYPE_STORES:
				savefile_do_block_stores(GET, version);
				break;
			case BLOCK_TYPE_DUNGEON:
				savefile_do_block_dungeon(GET, version);
				break;
			case BLOCK_TYPE_INVENTORY:
				savefile_do_block_inventory(GET, version);
				break;
            case BLOCK_TYPE_END:
                finished = TRUE;
                break;
		}

		/* Free the memory */
		KILL(savefile_block);
	}

	/* Important -- Initialize the sex */
	sp_ptr = &sex_info[p_ptr->psex];

	/* Important -- Initialize the race/class */
	rp_ptr = &p_info[p_ptr->prace];
	cp_ptr = &c_info[p_ptr->pclass];

	/* Important -- Initialize the magic */
	mp_ptr = &cp_ptr->spells;

	/* Calculate things */
	p_ptr->update = (PU_BONUS | PU_TORCH | PU_HP | PU_MANA | PU_SPELLS);
	update_stuff();

	/* We are done */
	return (0);
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

/*
 * Attempt to Load a "savefile"
 *
 * On multi-user systems, you may only "read" a savefile if you will be
 * allowed to "write" it later, this prevents painful situations in which
 * the player loads a savefile belonging to someone else, and then is not
 * allowed to save his game when he quits.
 *
 * We return "TRUE" if the savefile was usable, and we set the global
 * flag "character_loaded" if a real, living, character was loaded.
 *
 * Note that we always try to load the "current" savefile, even if
 * there is no such file, so we must check for "empty" savefile names.
 */
bool load_player(void)
{
	int fd = -1;
	errr err = 0;
	cptr what = "generic";
	bool result = TRUE;

	/* Allow empty savefile name */
	if (!savefile[0]) return (TRUE);

	/* Grab permissions */
	safe_setuid_grab();

	/* Open the savefile */
	fd = fd_open(savefile, O_RDONLY);

	/* Drop permissions */
	safe_setuid_drop();

	/* No file */
	if (fd < 0)
	{
		/* Give a message */
		msg_print("Savefile does not exist.");
		message_flush();

		/* Allow this */
		return (TRUE);
	}

	/* Close the file */
	fd_close(fd);

#ifdef VERIFY_SAVEFILE

	/* Verify savefile usage */
	if (!err)
	{
		FILE *fkk;

		char temp[1024];

		/* Extract name of lock file */
		strcpy(temp, savefile);
		strcat(temp, ".lok");

		/* Grab permissions */
		safe_setuid_grab();

		/* Check for lock */
		fkk = my_fopen(temp, "r");

		/* Drop permissions */
		safe_setuid_drop();

		/* Oops, lock exists */
		if (fkk)
		{
			/* Close the file */
			my_fclose(fkk);

			/* Message */
			msg_print("Savefile is currently in use.");
			message_flush();

			/* Oops */
			return (FALSE);
		}

		/* Grab permissions */
		safe_setuid_grab();

		/* Create a lock file */
		fkk = my_fopen(temp, "w");

		/* Drop permissions */
		safe_setuid_drop();

		/* Dump a line of info */
		fprintf(fkk, "Lock file for savefile '%s'\n", savefile);

		/* Close the lock file */
		my_fclose(fkk);
	}

#endif /* VERIFY_SAVEFILE */

	/* Okay */
	if (!err)
	{
		/* Grab permissions */
		safe_setuid_grab();

		/* Open the savefile */
		fd = fd_open(savefile, O_RDONLY);

		/* Drop permissions */
		safe_setuid_drop();

		/* No file */
		if (fd < 0) err = -1;

		/* Message (below) */
		if (err) what = "Cannot open savefile";
	}

	/* Process file */
	if (!err)
	{
		byte *header;
		int pos = 0;

		/* Allocate memory */
		C_MAKE(header, 6, byte);

		/* Read in the data */
		if (fd_read(fd, (char *) header, 6)) err = -1;

		/* What */
		if (err) what = "Cannot read savefile";

		/* Close the file */
		if (err) fd_close(fd);

		/* Don't do stuff */
		if (!err)
		{
			/* Clear the screen */
			Term_clear();

            /* Check savefile versions */
            if (header[pos++] != 83) { what = "Invalid savefile"; err = 1; }
            else if (header[pos++] != 97) { what = "Invalid savefile"; err = 1; }
            else if (header[pos++] != 118) { what = "Invalid savefile"; err = 1; }
            else if (header[pos++] != 101) { what = "Invalid savefile"; err = 1; }
            else
            {
                /* Read the savefile */
				err = (errr) read_savefile(fd);

                /* Error! */
                if (err) what = "Cannot parse savefile";
            }

			fd_close(fd);
		}

		/* Free the memory for the header */
		KILL(header);
	}

	/* A character was loaded */
	character_loaded = TRUE;

	/* Paranoia */
	if (!err)
	{
		/* Invalid turn */
		if (!turn) err = -1;

		/* Message (below) */
		if (err) what = "Broken savefile";
	}

    inkey();

	if (!err)
	{
		/* Player is dead */
		if (p_ptr->is_dead)
		{
			/* Forget death */
			p_ptr->is_dead = FALSE;

			/* Cheat death */
			if (arg_wizard)
			{
				/* A character was loaded */
				character_loaded = TRUE;

				/* Done */
				return (TRUE);
			}

			/* Count lives */
			sf_lives++;

			/* Forget turns */
			turn = old_turn = 0;

			/* Done */
			return (TRUE);
		}

		/* A character was loaded */
		character_loaded = TRUE;

		/* Still alive */
		if (p_ptr->chp >= 0)
		{
			/* Reset cause of death */
			strcpy(p_ptr->died_from, "(alive and well)");
		}

		/* Success */
		return (TRUE);
	}

#ifdef VERIFY_SAVEFILE

	/* XXX */
	if (TRUE)
	{
		char temp[1024];

		/* Extract name of lock file */
		strcpy(temp, savefile);
		strcat(temp, ".lok");

		/* Grab permissions */
		safe_setuid_grab();

		/* Remove lock */
		fd_kill(temp);

		/* Drop permissions */
		safe_setuid_drop();
	}

#endif /* VERIFY_SAVEFILE */

	/* Oops */
	return (result);
}


/* File: object1.c */


/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * Max sizes of the following arrays.
 */
#define MAX_TITLES     50       /* Used with scrolls (min 48) */
#define MAX_SYLLABLES 160       /* Used with scrolls (see below) */


/*
 * Syllables for scrolls (must be 1-4 letters each).
 */

static cptr syllables[MAX_SYLLABLES] =
{
	"a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
	"arg", "arze", "ash", "aus", "ban", "bar", "bat", "bek",
	"bie", "bin", "bit", "bjor", "blu", "bot", "bu",
	"byt", "comp", "con", "cos", "cre", "dalf", "dan",
	"den", "der", "doe", "dok", "eep", "el", "eng", "er", "ere", "erk",
	"esh", "evs", "fa", "fid", "flit", "for", "fri", "fu", "gan",
	"gar", "glen", "gop", "gre", "ha", "he", "hyd", "i",
	"ing", "ion", "ip", "ish", "it", "ite", "iv", "jo",
	"kho", "kli", "klis", "la", "lech", "man", "mar",
	"me", "mi", "mic", "mik", "mon", "mung", "mur", "nag", "nej",
	"nelg", "nep", "ner", "nes", "nis", "nih", "nin", "o",
	"od", "ood", "org", "orn", "ox", "oxy", "pay", "pet",
	"ple", "plu", "po", "pot", "prok", "re", "rea", "rhov",
	"ri", "ro", "rog", "rok", "rol", "sa", "san", "sat",
	"see", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
	"sno", "so", "sol", "sri", "sta", "sun", "ta", "tab",
	"tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
	"ulk", "um", "un", "uni", "ur", "val", "viv", "vly",
	"vom", "wah", "wed", "werg", "wex", "whon", "wun", "x",
	"yerg", "yp", "zun", "tri", "timo", "petelia", "blaa"
};


/*
 * Hold the titles of scrolls, 6 to 14 characters each.
 *
 * Also keep an array of scroll colors (always WHITE for now).
 */

static char scroll_adj[MAX_TITLES][16];


static void flavor_assign_fixed(void)
{
	int i, j;

	for (i = 0; i < z_info->flavor_max; i++)
	{
		flavor_type *flavor_ptr = &flavor_info[i];

		/* Skip random flavors */
		if (flavor_ptr->sval == SV_UNKNOWN) continue;

		for (j = 0; j < z_info->k_max; j++)
		{
			/* Skip other objects */
			if ((k_info[j].tval == flavor_ptr->tval) &&
			    (k_info[j].sval == flavor_ptr->sval))
			{
				/* Store the flavor index */
				k_info[j].flavor = i;
			}
		}
	}
}


static void flavor_assign_random(byte tval)
{
	int i, j;
	int flavor_count = 0;
	int choice;

	/* Count the random flavors for the given tval */
	for (i = 0; i < z_info->flavor_max; i++)
	{
		if ((flavor_info[i].tval == tval) &&
		    (flavor_info[i].sval == SV_UNKNOWN))
		{
			flavor_count++;
		}
	}

	for (i = 0; i < z_info->k_max; i++)
	{
		/* Skip other object types */
		if (k_info[i].tval != tval) continue;

		/* Skip objects that already are flavored */
		if (k_info[i].flavor != 0) continue;

		/* HACK - Ordinary food is "boring" */
		if ((tval == TV_FOOD) && (k_info[i].sval >= SV_FOOD_MIN_FOOD))
			continue;

		if (!flavor_count) quit_fmt("Not enough flavors for tval %d.", tval);

		/* Select a flavor */
		choice = rand_int(flavor_count);
	
		/* Find and store the flavor */
		for (j = 0; j < z_info->flavor_max; j++)
		{
			/* Skip other tvals */
			if (flavor_info[j].tval != tval) continue;

			/* Skip assigned svals */
			if (flavor_info[j].sval != SV_UNKNOWN) continue;

			if (choice == 0)
			{
				/* Store the flavor index */
				k_info[i].flavor = j;

				/* Mark the flavor as used */
				flavor_info[j].sval = k_info[i].sval;

				/* One less flavor to choose from */
				flavor_count--;

				break;
			}

			choice--;
		}
	}
}


/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * Scroll titles are always between 6 and 14 letters long.  This is
 * ensured because every title is composed of whole words, where every
 * word is from 1 to 8 letters long (one or two syllables of 1 to 4
 * letters each), and that no scroll is finished until it attempts to
 * grow beyond 15 letters.  The first time this can happen is when the
 * current title has 6 letters and the new word has 8 letters, which
 * would result in a 6 letter scroll title.
 *
 * Duplicate titles are avoided by requiring that no two scrolls share
 * the same first four letters (not the most efficient method, and not
 * the least efficient method, but it will always work).
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 */
void flavor_init(void)
{
	int i, j;


	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;


	flavor_assign_fixed();

	flavor_assign_random(TV_RING);
	flavor_assign_random(TV_AMULET);
	flavor_assign_random(TV_STAFF);
	flavor_assign_random(TV_WAND);
	flavor_assign_random(TV_ROD);
	flavor_assign_random(TV_FOOD);
	flavor_assign_random(TV_POTION);
	flavor_assign_random(TV_SCROLL);
	/* flavor_assign_random(TV_ORB); */

	/* Scrolls (random titles, always white) */
	for (i = 0; i < MAX_TITLES; i++)
	{
		/* Get a new title */
		while (TRUE)
		{
			char buf[80];

			bool okay;

			/* Start a new title */
			buf[0] = '\0';

			/* Collect words until done */
			while (1)
			{
				int q, s;

				char tmp[80];

				/* Start a new word */
				tmp[0] = '\0';

				/* Choose one or two syllables */
				s = ((rand_int(100) < 30) ? 1 : 2);

				/* Add a one or two syllable word */
				for (q = 0; q < s; q++)
				{
					/* Add the syllable */
					strcat(tmp, syllables[rand_int(MAX_SYLLABLES)]);
				}

				/* Stop before getting too long */
				if (strlen(buf) + 1 + strlen(tmp) > 15) break;

				/* Add a space */
				strcat(buf, " ");

				/* Add the word */
				strcat(buf, tmp);
			}

			/* Save the title */
			strcpy(scroll_adj[i], buf+1);

			/* Assume okay */
			okay = TRUE;

			/* Check for "duplicate" scroll titles */
			for (j = 0; j < i; j++)
			{
				cptr hack1 = scroll_adj[j];
				cptr hack2 = scroll_adj[i];

				/* Compare first four characters */
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;

				/* Not okay */
				okay = FALSE;

				/* Stop looking */
				break;
			}

			/* Break when done */
			if (okay) break;
		}
	}


	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;

	/* Analyze every object */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* No flavor yields aware */
		if (!k_ptr->flavor) k_ptr->aware = TRUE;
	}
}



#ifdef ALLOW_BORG_GRAPHICS
extern void init_translate_visuals(void);
#endif /* ALLOW_BORG_GRAPHICS */


/*
 * Reset the "visual" lists
 *
 * This involves resetting various things to their "default" state.
 *
 * If the "prefs" flag is TRUE, then we will also load the appropriate
 * "user pref file" based on the current setting of the "use_graphics"
 * flag.  This is useful for switching "graphics" on/off.
 *
 * The features, objects, and monsters, should all be encoded in the
 * relevant "font.pref" and/or "graf.prf" files.  XXX XXX XXX
 *
 * The "prefs" parameter is no longer meaningful.  XXX XXX XXX
 */
void reset_visuals(bool unused)
{
	int i;

	/* Unused parameter */
	(void)unused;

	/* Extract default attr/char code for features */
	for (i = 0; i < z_info->f_max; i++)
	{
		feature_type *f_ptr = &f_info[i];

		/* Assume we will use the underlying values */
		f_ptr->x_attr = f_ptr->d_attr;
		f_ptr->x_char = f_ptr->d_char;
	}

	/* Extract default attr/char code for objects */
	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Default attr/char */
		k_ptr->x_attr = k_ptr->d_attr;
		k_ptr->x_char = k_ptr->d_char;
	}

	/* Extract default attr/char code for monsters */
	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Default attr/char */
		r_ptr->x_attr = r_ptr->d_attr;
		r_ptr->x_char = r_ptr->d_char;
	}

	/* Extract default attr/char code for flavors */
	for (i = 0; i < z_info->flavor_max; i++)
	{
		flavor_type *flavor_ptr = &flavor_info[i];

		/* Default attr/char */
		flavor_ptr->x_attr = flavor_ptr->d_attr;
		flavor_ptr->x_char = flavor_ptr->d_char;
	}

	/* Extract attr/chars for inventory objects (by tval) */
	for (i = 0; i < 128; i++)
	{
		/* Default to white */
		tval_to_attr[i] = TERM_WHITE;
	}


	/* Graphic symbols */
	if (use_graphics)
	{
		/* Process "graf.prf" */
		process_pref_file("graf.prf");
	}

	/* Normal symbols */
	else
	{
		/* Process "font.prf" */
		process_pref_file("font.prf");
	}

#ifdef ALLOW_BORG_GRAPHICS
	/* Initialize the translation table for the borg */
	init_translate_visuals();
#endif /* ALLOW_BORG_GRAPHICS */
}


/*
 * Modes of object_*_aux()
 */
#define OBJECT_AUX_FULL   1 /* Full info */
#define OBJECT_AUX_KNOWN  2 /* Only what's known to the player */


/*
 * Obtain the stat mods for an item
 */
static void object_stat_bonuses_aux(int mode, const object_type *o_ptr, s16b *mods)
{
	object_kind *k_ptr = NULL;
	bool fully_known = FALSE;
	int n;

	/* Unused parameter */
	(void) mode;

	/* Make k_ptr useful */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Clear */
	for (n = 0; n < A_MAX; n++) mods[n] = 0;

#ifdef SPOIL_ARTIFACTS
	/* Full knowledge for some artifacts */
	if (artifact_p(o_ptr)) fully_known = TRUE;
#endif /* SPOIL_ARTIFACTS */

#ifdef SPOIL_EGO_ITEMS
	/* Full knowledge for some ego-items */
	if (ego_item_p(o_ptr)) fully_known = TRUE;
#endif /* SPOIL_ARTIFACTS */

	/* Stat bonuses always show up. */
	if (object_known_p(o_ptr)) fully_known = TRUE;

	/* Get the resists from the base object */
	for (n = 0; n < A_MAX; n++)
	{
		/* Copy it across */
		mods[n] += k_ptr->stat_mods[n];
	}

	/* Get the resists from the object itself */
	for (n = 0; n < A_MAX; n++)
	{
		/* Copy it across */
		mods[n] += o_ptr->stat_mods[n];
	}

	/* Show all bonuses if fully known */
	if (fully_known)
	{
		/* Artifact */
		if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			for (n = 0; n < A_MAX; n++)
			{
				mods[n] += a_ptr->stat_mods[n];
			}
		}

		/* Ego-item */
		if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			for (n = 0; n < A_MAX; n++)
			{
				mods[n] += e_ptr->stat_mods[n];
			}
		}
	}

	/* We are done. */
	return;
}


/*
 * Obtain the resists for an item
 */
static void object_resists_aux(int mode, const object_type *o_ptr, byte *resists)
{
	object_kind *k_ptr = NULL;
	bool fully_known = FALSE;
	int n;

	/* Make k_ptr useful */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Clear */
	for (n = 0; n < RES_MAX; n++) resists[n] = 0;

#ifdef SPOIL_ARTIFACTS
	/* Full knowledge for some artifacts */
	if (artifact_p(o_ptr)) fully_known = TRUE;
#endif /* SPOIL_ARTIFACTS */

#ifdef SPOIL_EGO_ITEMS
	/* Full knowledge for some ego-items */
	if (ego_item_p(o_ptr)) fully_known = TRUE;
#endif /* SPOIL_ARTIFACTS */

	if (mode == OBJECT_AUX_FULL) fully_known = TRUE;
	if (o_ptr->ident & (IDENT_MENTAL)) fully_known = TRUE;

	/* Must be identified */
	if (!fully_known)
	{
		if (!object_known_p(o_ptr)) return;
	}

	/* Get the resists from the base object */
	for (n = 0; n < RES_MAX; n++)
	{
		/* Copy it across */
		resists[n] += k_ptr->resists[n];
	}

	/* Show all resists if fully known */
	if (fully_known)
	{
		/* Artifact */
		if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			for (n = 0; n < RES_MAX; n++)
			{
				resists[n] += a_ptr->resists[n];
			}
		}

		/* Ego-item */
		if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			for (n = 0; n < RES_MAX; n++)
			{
				resists[n] += e_ptr->resists[n];
			}
		}

		/* Randart */
		if (o_ptr->name3)
		{
			randart_type *x_ptr = &x_info[o_ptr->name3];

			for (n = 0; n < RES_MAX; n++)
			{
				resists[n] += x_ptr->resists[n];
			}
		}
	}

	return;
}

/*
 * Obtain the "flags" for an item
 */
static void object_flags_aux(int mode, const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	object_kind *k_ptr = NULL;
	bool fully_known = FALSE;

	/* Clear */
	(*f1) = (*f2) = (*f3) = 0L;

	/* Make k_ptr useful */
	k_ptr = &k_info[o_ptr->k_idx];

#ifdef SPOIL_ARTIFACTS
	/* Full knowledge for some artifacts */
	if (artifact_p(o_ptr)) fully_known = TRUE;
#endif /* SPOIL_ARTIFACTS */

#ifdef SPOIL_EGO_ITEMS
	/* Full knowledge for some ego-items */
	if (ego_item_p(o_ptr)) fully_known = TRUE;
#endif /* SPOIL_ARTIFACTS */

	if (mode == OBJECT_AUX_FULL) fully_known = TRUE;
	if (o_ptr->ident & (IDENT_MENTAL)) fully_known = TRUE;

	/* Must be identified */
	if (!fully_known)
	{
		if (!object_known_p(o_ptr)) return;
	}

	/* Base flags */
	(*f1) = k_ptr->flags1;
	(*f2) = k_ptr->flags2;
	(*f3) = k_ptr->flags3;
	
	/* Show all flags if fully known */
	if (fully_known)
	{
		/* Artifact */
		if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			(*f1) |= a_ptr->flags1;
			(*f2) |= a_ptr->flags2;
			(*f3) |= a_ptr->flags3;
		}

		/* Ego-item */
		if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			(*f1) |= e_ptr->flags1;
			(*f2) |= e_ptr->flags2;
			(*f3) |= e_ptr->flags3;
		}

		/* Randarts */
		if (o_ptr->name3)
		{
			randart_type *x_ptr = &x_info[o_ptr->name3];

			(*f1) |= x_ptr->flags1;
			(*f2) |= x_ptr->flags2;
			(*f3) |= x_ptr->flags3;
		}
	}
	else
	{
		/* Obvious artifact flags */
		if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			/* Obvious flags (pval) */
			(*f1) = (a_ptr->flags1 & (TR1_PVAL_MASK));
			(*f3) = (a_ptr->flags3 & (TR3_IGNORE_MASK));
		}
		
		/* Obvious ego-item flags */
		if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			/* Obvious flags (pval) */
			(*f1) = (e_ptr->flags1 & (TR1_PVAL_MASK));
			(*f3) = (e_ptr->flags3 & (TR3_IGNORE_MASK));
		}

		/* Randarts */
		if (o_ptr->name3)
		{
			randart_type *x_ptr = &x_info[o_ptr->name3];

			/* Obvious flags (pval) */
			(*f1) = (x_ptr->flags1 & (TR1_PVAL_MASK));
			(*f3) = (x_ptr->flags3 & (TR3_IGNORE_MASK));
		}
	}

	/* Extra powers */
	switch (o_ptr->xtra1)
	{
		case OBJECT_XTRA_TYPE_SUSTAIN:
		{
			/* OBJECT_XTRA_WHAT_SUSTAIN == 1 */
			if (!fully_known) return;
			(*f2) |= (OBJECT_XTRA_BASE_SUSTAIN << o_ptr->xtra2);
			break;
		}

		case OBJECT_XTRA_TYPE_POWER:
		{
			/* OBJECT_XTRA_WHAT_POWER == 3 */
			if (!fully_known) return;
			(*f3) |= (OBJECT_XTRA_BASE_POWER << o_ptr->xtra2);
			break;
		}
	}
	
	return;
}




/*
 * Obtain the "flags" for an item
 */
void object_flags(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	object_flags_aux(OBJECT_AUX_FULL, o_ptr, f1, f2, f3);
}

/*
 * Obtain the "flags" for an item which are known to the player
 */
void object_flags_known(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	object_flags_aux(OBJECT_AUX_KNOWN, o_ptr, f1, f2, f3);
}

/*
 * Obtain the "resists" for an item
 */
void object_resists(const object_type *o_ptr, byte *resists)
{
	object_resists_aux(OBJECT_AUX_FULL, o_ptr, resists);
}

/*
 * Obtain the "resists" for an item which are known to the player
 */
void object_resists_known(const object_type *o_ptr, byte *resists)
{
	object_resists_aux(OBJECT_AUX_KNOWN, o_ptr, resists);
}

/*
 * Obtain the stat bonuses for an item
 */
void object_stat_bonuses(const object_type *o_ptr, s16b *mods)
{
	object_stat_bonuses_aux(OBJECT_AUX_FULL, o_ptr, mods);
}

/*
 * Obtain the stat bonuses for an item which are known to the player
 */
void object_stat_bonuses_known(const object_type *o_ptr, s16b *mods)
{
	object_stat_bonuses_aux(OBJECT_AUX_KNOWN, o_ptr, mods);
}

/*
 * Efficient version of '(T) += strfmt((T), "%c", (C))'
 */
#define object_desc_chr_macro(T,C) do { \
 \
	/* Copy the char */ \
	*(T)++ = (C); \
 \
} while (0)



/*
 * Efficient version of '(T) += strfmt((T), "%s", (S))'
 */
#define object_desc_str_macro(T,S) do { \
 \
	cptr s = (S); \
 \
	/* Copy the string */ \
	while (*s) *(T)++ = *s++; \
 \
} while (0)



/*
 * Efficient version of '(T) += strfmt((T), "%u", (N))'
 */
#define object_desc_num_macro(T,N) do { \
 \
	uint n = (N); \
 \
	uint p; \
 \
	/* Find "size" of "n" */ \
	for (p = 1; n >= p * 10; p = p * 10) /* loop */; \
 \
	/* Dump each digit */ \
	while (p >= 1) \
	{ \
		/* Dump the digit */ \
		*(T)++ = I2D(n / p); \
 \
		/* Remove the digit */ \
		n = n % p; \
 \
		/* Process next digit */ \
		p = p / 10; \
	} \
 \
} while (0)



/*
 * Efficient version of '(T) += strfmt((T), "%+d", (I))'
 */
#define object_desc_int_macro(T,I) do { \
 \
	sint i = (I); \
 \
	/* Negative */ \
	if (i < 0) \
	{ \
		/* Take the absolute value */ \
		i = 0 - i; \
 \
		/* Use a "minus" sign */ \
		*(T)++ = '-'; \
	} \
 \
	/* Positive (or zero) */ \
	else \
	{ \
		/* Use a "plus" sign */ \
		*(T)++ = '+'; \
	} \
 \
	/* Dump the number itself */ \
	object_desc_num_macro(T, i); \
 \
} while (0)




/*
 * Creates a description of the item "o_ptr", and stores it in "out_val".
 *
 * One can choose the "verbosity" of the description, including whether
 * or not the "number" of items should be described, and how much detail
 * should be used when describing the item.
 *
 * The given "buf" must be 80 chars long to hold the longest possible
 * description, which can get pretty long, including incriptions, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
 * Note that the inscription will be clipped to keep the total description
 * under 79 chars (plus a terminator).
 *
 * This function uses a big temporary array to create the description,
 * and then copies up to 79 characters from this array into the buffer,
 * which will prevent crashes (but not ugliness) if any object name uses
 * more than 79 characters.
 *
 * Note the use of "object_desc_int_macro()" and "object_desc_num_macro()"
 * and "object_desc_str_macro()" and "object_desc_chr_macro()" as extremely
 * efficient, portable, versions of some common "strfmt()" commands (without
 * the bounds checking or termination writing), which allow a pointer to
 * efficiently move through a buffer while modifying it in various ways.
 *
 * Various improper uses and/or placements of "&" or "~" characters can
 * easily induce out-of-bounds memory accesses.  Some of these could be
 * easily checked for, if efficiency was not a concern.
 *
 * Note that all ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should never happen.
 *
 * Note that all artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lights, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Light's use the "k_info" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_info" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_info" name.  But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
 * at least, those which have no "pluses", such as the three artifact lights.
 *
 * The "pluralization" rules are extremely hackish, in fact, for efficiency,
 * we only handle things like "torch"/"torches" and "cutlass"/"cutlasses",
 * and we would not handle "box"/"boxes", or "knife"/"knives", correctly.
 * Of course, it would be easy to add rules for these forms.
 *
 * If "pref" is true then a "numeric" prefix will be pre-pended, else is is
 * assumed that a string such as "The" or "Your" will be pre-pended later.
 *
 * Modes ("pref" is TRUE):
 *   0 -- Chain Mail of Death
 *   1 -- A Cloak of Death [1,+3]
 *   2 -- An Amulet of Death [1,+3] (+2 to Stealth)
 *   3 -- 5 Rings of Death [1,+3] (+2 to Stealth) {nifty}
 *
 * Modes ("pref" is FALSE):
 *   0 -- Chain Mail of Death
 *   1 -- Cloak of Death [1,+3]
 *   2 -- Amulet of Death [1,+3] (+2 to Stealth)
 *   3 -- Rings of Death [1,+3] (+2 to Stealth) {nifty}
 */
void object_desc(char *buf, const object_type *o_ptr, int pref, int mode)
{
	cptr basenm;
	cptr modstr;

	int power;

	bool aware;
	bool known;

	bool flavor;

	bool append_name;

	bool show_weapon;
	bool show_armour;

	char *b;

	char *t;

	cptr s;

	cptr u;
	cptr v;

	char p1 = '(', p2 = ')';
	char b1 = '[', b2 = ']';
	char c1 = '{', c2 = '}';

	char discount_buf[80];

	char tmp_buf[128];

	u32b f1, f2, f3;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];


	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3);


	/* See if the object is "aware" */
	aware = (object_aware_p(o_ptr) ? TRUE : FALSE);

	/* See if the object is "known" */
	known = (object_known_p(o_ptr) ? TRUE : FALSE);

	/* See if the object is "flavored" */
	flavor = (k_ptr->flavor ? TRUE : FALSE);

	/* Allow flavors to be hidden when aware */
	if (aware && !show_flavors) flavor = FALSE;

	/* Assume no name appending */
	append_name = FALSE;

	/* Assume no need to show "weapon" bonuses */
	show_weapon = FALSE;

	/* Assume no need to show "armour" bonuses */
	show_armour = FALSE;

	/* Extract default "base" string */
	basenm = (k_name + k_ptr->name);

	/* Assume no "modifier" string */
	modstr = "";


	/* Analyze the object */
	switch (o_ptr->tval)
	{
		/* Some objects are easy to describe */
		case TV_SKELETON:
		case TV_BOTTLE:
		case TV_JUNK:
		case TV_SPIKE:
		case TV_FLASK:
		case TV_CHEST:
		{
			break;
		}

		/* Missiles/Bows/Weapons */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			show_weapon = TRUE;
			break;
		}

		/* Armour */
	        case TV_BELT:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			show_armour = TRUE;
			break;
		}

		/* Lights (including a few "Specials") */
		case TV_LIGHT:
		{
			break;
		}

		/* Amulets (including a few "Specials") */
		case TV_AMULET:
		{
			/* Hack -- Known artifacts */
			if (artifact_p(o_ptr) && aware) break;

			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Amulet~" : "& Amulet~");

			break;
		}

		/* Orbs */
		case TV_ORB:
		{
			/* Color the object */
			/* modstr = flavor_text +
flavor_info[k_ptr->flavor].text; */
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Orb~" : "& Orb~");

			break;
		}

		/* Rings (including a few "Specials") */
		case TV_RING:
		{
			/* Hack -- Known artifacts */
			if (artifact_p(o_ptr) && aware) break;

			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Ring~" : "& Ring~");

			break;
		}

		/* Staffs */
		case TV_STAFF:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Staff~" : "& Staff~");

			break;
		}

		/* Wands */
		case TV_WAND:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Wand~" : "& Wand~");

			break;
		}

		/* Rods */
		case TV_ROD:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Rod~" : "& Rod~");

			break;
		}

		/* Scrolls */
		case TV_SCROLL:
		{
			/* Color the object */
			modstr = scroll_adj[o_ptr->sval];
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& Scroll~ titled \"#\"" : "& Scroll~");

			break;
		}

		/* Potions */
		case TV_POTION:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Potion~" : "& Potion~");

			break;
		}

		/* Food */
		case TV_FOOD:
		{
			/* Ordinary food is "boring" */
			if (o_ptr->sval >= SV_FOOD_MIN_FOOD) break;

			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Mushroom~" : "& Mushroom~");

			break;
		}

		/* Magic Books */
		case TV_MAGIC_BOOK:
		{
			modstr = basenm;
			basenm = "& Book~ of Magic Spells #";
			break;
		}

		/* Prayer Books */
		case TV_PRAYER_BOOK:
		{
			modstr = basenm;
			basenm = "& Holy Book~ of Prayers #";
			break;
		}

		/* Hack -- Gold/Gems */
		case TV_GOLD:
		{
			strcpy(buf, basenm);
			return;
		}

		/* Hack -- Default -- Used in the "inventory" routine */
		default:
		{
			strcpy(buf, "(nothing)");
			return;
		}
	}


	/* Start dumping the result */
	t = b = tmp_buf;

	/* Begin */
	s = basenm;

	/* Handle objects which sometimes use "a" or "an" */
	if (*s == '&')
	{
		/* Paranoia XXX XXX XXX */
		/* ASSERT(s[1] == ' '); */

		/* Skip the ampersand and the following space */
		s += 2;

		/* No prefix */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- None left */
		else if (o_ptr->number <= 0)
		{
			object_desc_str_macro(t, "no more ");
		}

		/* Extract the number */
		else if (o_ptr->number > 1)
		{
			object_desc_num_macro(t, o_ptr->number);
			object_desc_chr_macro(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && artifact_p(o_ptr))
		{
			object_desc_str_macro(t, "The ");
		}

		/* Hack -- A single one, and next character will be a vowel */
		else if ((*s == '#') ? is_a_vowel(modstr[0]) : is_a_vowel(*s))
		{
			object_desc_str_macro(t, "an ");
		}

		/* A single one, and next character will be a non-vowel */
		else
		{
			object_desc_str_macro(t, "a ");
		}
	}

	/* Handle objects which never use "a" or "an" */
	else
	{
		/* No pref */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- all gone */
		else if (o_ptr->number <= 0)
		{
			object_desc_str_macro(t, "no more ");
		}

		/* Prefix a number if required */
		else if (o_ptr->number > 1)
		{
			object_desc_num_macro(t, o_ptr->number);
			object_desc_chr_macro(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && artifact_p(o_ptr))
		{
			object_desc_str_macro(t, "The ");
		}

		/* Hack -- A single item, so no prefix needed */
		else
		{
			/* Nothing */
		}
	}


	/* Paranoia XXX XXX XXX */
	/* ASSERT(*s != '~'); */

	/* Copy the string */
	for (; *s; s++)
	{
		/* Pluralizer */
		if (*s == '~')
		{
			/* Add a plural if needed */
			if ((o_ptr->number != 1) && !(known && artifact_p(o_ptr)))
			{
				char k = t[-1];

				/* Hack -- "Cutlass-es" and "Torch-es" */
				if ((k == 's') || (k == 'h')) *t++ = 'e';

				/* Add an 's' */
				*t++ = 's';
			}
		}

		/* Modifier */
		else if (*s == '#')
		{
			/* Append the modifier */
			object_desc_str_macro(t, modstr);
		}

		/* Normal */
		else
		{
			/* Copy */
			*t++ = *s;
		}
	}


	/* Append the "kind name" to the "base name" */
	if (append_name)
	{
		object_desc_str_macro(t, " of ");
		object_desc_str_macro(t, (k_name + k_ptr->name));
	}


	/* Hack -- Append "Artifact" or "Special" names */
	if (known)
	{
		/* Grab any artifact name */
		if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			object_desc_chr_macro(t, ' ');
			object_desc_str_macro(t, (a_name + a_ptr->name));
		}

		/* Grab any randart names */
		else if (o_ptr->name3)
		{
			randart_type *x_ptr = &x_info[o_ptr->name3];

			object_desc_chr_macro(t, ' ');
			object_desc_str_macro(t, x_ptr->name);
		}

		/* Grab any ego-item name */
		else if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			object_desc_chr_macro(t, ' ');
			object_desc_str_macro(t, (e_name + e_ptr->name));
		}
	}


	/* No more details wanted */
	if (mode < 1) goto object_desc_done;


	/* Hack -- Chests must be described in detail */
	if (o_ptr->tval == TV_CHEST)
	{
		cptr tail = "";

		/* Not searched yet */
		if (!known)
		{
			/* Nothing */
		}

		/* May be "empty" */
		else if (!o_ptr->pval)
		{
			tail = " (empty)";
		}

		/* May be "disarmed" */
		else if (o_ptr->pval < 0)
		{
			if (chest_traps[0 - o_ptr->pval])
			{
				tail = " (disarmed)";
			}
			else
			{
				tail = " (unlocked)";
			}
		}

		/* Describe the traps, if any */
		else
		{
			/* Describe the traps */
			switch (chest_traps[o_ptr->pval])
			{
				case 0:
				{
					tail = " (Locked)";
					break;
				}
				case CHEST_LOSE_STR:
				{
					tail = " (Poison Needle)";
					break;
				}
				case CHEST_LOSE_CON:
				{
					tail = " (Poison Needle)";
					break;
				}
				case CHEST_POISON:
				{
					tail = " (Gas Trap)";
					break;
				}
				case CHEST_PARALYZE:
				{
					tail = " (Gas Trap)";
					break;
				}
				case CHEST_EXPLODE:
				{
					tail = " (Explosion Device)";
					break;
				}
				case CHEST_SUMMON:
				{
					tail = " (Summoning Runes)";
					break;
				}
				default:
				{
					tail = " (Multiple Traps)";
					break;
				}
			}
		}

		/* Append the tail */
		object_desc_str_macro(t, tail);
	}


	/* Display the item like a weapon */
	if (f3 & (TR3_SHOW_MODS)) show_weapon = TRUE;

	/* Display the item like a weapon */
	if (o_ptr->to_h && o_ptr->to_d) show_weapon = TRUE;

	/* Display the item like armour */
	if (o_ptr->ac) show_armour = TRUE;


	/* Dump base weapon info */
	switch (o_ptr->tval)
	{
		/* Missiles */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		{
			/* Fall through */
		}

		/* Weapons */
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			/* Append a "damage" string */
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_num_macro(t, o_ptr->dd);
			object_desc_chr_macro(t, 'd');
			object_desc_num_macro(t, o_ptr->ds);
			object_desc_chr_macro(t, p2);

			/* All done */
			break;
		}

		/* Bows */
		case TV_BOW:
		{
			/* Hack -- Extract the "base power" */
			power = (o_ptr->sval % 10);

			/* Append a "power" string */
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_chr_macro(t, 'x');
			object_desc_num_macro(t, power);
			object_desc_chr_macro(t, p2);

			/* All done */
			break;
		}
	}


	/* Add the weapon bonuses */
	if (known)
	{
		/* Show the tohit/todam on request */
		if (show_weapon)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_int_macro(t, o_ptr->to_h);
			object_desc_chr_macro(t, ',');
			object_desc_int_macro(t, o_ptr->to_d);
			object_desc_chr_macro(t, p2);
		}

		/* Show the tohit if needed */
		else if (o_ptr->to_h)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_int_macro(t, o_ptr->to_h);
			object_desc_chr_macro(t, p2);
		}

		/* Show the todam if needed */
		else if (o_ptr->to_d)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_int_macro(t, o_ptr->to_d);
			object_desc_chr_macro(t, p2);
		}
	}


	/* Add the armor bonuses */
	if (known)
	{
		/* Show the armor class info */
		if (show_armour)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, b1);
			object_desc_num_macro(t, o_ptr->ac);
			object_desc_chr_macro(t, ',');
			object_desc_int_macro(t, o_ptr->to_a);
			object_desc_chr_macro(t, b2);
		}

		/* No base armor, but does increase armor */
		else if (o_ptr->to_a)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, b1);
			object_desc_int_macro(t, o_ptr->to_a);
			object_desc_chr_macro(t, b2);
		}
	}

	/* Hack -- always show base armor */
	else if (show_armour)
	{
		object_desc_chr_macro(t, ' ');
		object_desc_chr_macro(t, b1);
		object_desc_num_macro(t, o_ptr->ac);
		object_desc_chr_macro(t, b2);
	}


	/* No more details wanted */
	if (mode < 2) goto object_desc_done;


	/* Hack -- Wands and Staffs have charges */
	if (known &&
	    ((o_ptr->tval == TV_STAFF) ||
	     (o_ptr->tval == TV_WAND)))
	{
		/* Dump " (N charges)" */
		object_desc_chr_macro(t, ' ');
		object_desc_chr_macro(t, p1);
		object_desc_num_macro(t, o_ptr->pval);
		object_desc_str_macro(t, " charge");
		if (o_ptr->pval != 1)
		{
			object_desc_chr_macro(t, 's');
		}
		object_desc_chr_macro(t, p2);
	}

	/* Hack -- Rods have a "charging" indicator */
	else if (known && (o_ptr->tval == TV_ROD))
	{
		/* Hack -- Dump " (charging)" if relevant */
		if (o_ptr->pval)
		{
			object_desc_str_macro(t, " (charging)");
		}
	}

	/* Hack -- Process Lanterns/Torches */
	else if ((o_ptr->tval == TV_LIGHT) && (!artifact_p(o_ptr)))
	{
		/* Hack -- Turns of light for normal lights */
		object_desc_str_macro(t, " (with ");
		object_desc_num_macro(t, o_ptr->pval);
		object_desc_str_macro(t, " turns of light)");
	}


	/* Dump "pval" flags for wearable items */
	if (known && (f1 & (TR1_PVAL_MASK)))
	{
		cptr tail = "";
		cptr tail2 = "";

		/* Start the display */
		object_desc_chr_macro(t, ' ');
		object_desc_chr_macro(t, p1);

		/* Dump the "pval" itself */
		object_desc_int_macro(t, o_ptr->pval);

		/* Do not display the "pval" flags */
		if (f3 & (TR3_HIDE_TYPE))
		{
			/* Nothing */
		}

		/* Stealth */
		else if (f1 & (TR1_STEALTH))
		{
			/* Dump " to stealth" */
			tail = " to stealth";
		}

		/* Searching */
		else if (f1 & (TR1_SEARCH))
		{
			/* Dump " to searching" */
			tail = " to searching";
		}

		/* Infravision */
		else if (f1 & (TR1_INFRA))
		{
			/* Dump " to infravision" */
			tail = " to infravision";
		}

#if 0

		/* Tunneling */
		else if (f1 & (TR1_TUNNEL))
		{
			/* Dump " to digging" */
			tail = " to digging";
		}

#endif

		/* Speed */
		else if (f1 & (TR1_SPEED))
		{
			/* Dump " to speed" */
			tail = " to speed";
		}

		/* Blows */
		else if (f1 & (TR1_BLOWS))
		{
			/* Add " attack" */
			tail = " attack";

			/* Add "attacks" */
			if (ABS(o_ptr->pval) != 1) tail2 = "s";
		}

#if 0

		/* Shots */
		else if (f1 & (TR1_SHOTS))
		{
			/* Nothing */
		}

		/* Might */
		else if (f1 & (TR1_MIGHT))
		{
			/* Nothing */
		}

#endif

		/* Add the descriptor */
		object_desc_str_macro(t, tail);
		object_desc_str_macro(t, tail2);

		/* Finish the display */
		object_desc_chr_macro(t, p2);
	}


	/* Indicate "charging" artifacts */
	if (known && o_ptr->timeout)
	{
		/* Hack -- Dump " (charging)" if relevant */
		object_desc_str_macro(t, " (charging)");
	}


	/* No more details wanted */
	if (mode < 3) goto object_desc_done;


	/* Use standard inscription */
	if (o_ptr->note)
	{
		u = quark_str(o_ptr->note);
	}

	/* Use nothing */
	else
	{
		u = NULL;
	}


	/* Use special inscription, if any */
	if (o_ptr->discount >= INSCRIP_NULL)
	{
		v = inscrip_text[o_ptr->discount - INSCRIP_NULL];
	}

	/* Use "cursed" if the item is known to be cursed */
	else if (cursed_p(o_ptr) && known)
	{
		v = "cursed";
	}

	/* Hack -- Use "empty" for empty wands/staffs */
	else if (!known && (o_ptr->ident & (IDENT_EMPTY)))
	{
		v = "empty";
	}

	/* Use "tried" if the object has been tested unsuccessfully */
	else if (!aware && object_tried_p(o_ptr))
	{
		v = "tried";
	}

	/* Use the discount, if any */
	else if (o_ptr->discount > 0)
	{
		char *q = discount_buf;
		object_desc_num_macro(q, o_ptr->discount);
		object_desc_str_macro(q, "% off");
		*q = '\0';
		v = discount_buf;
	}

	/* Nothing */
	else
	{
		v = NULL;
	}


	/* Inscription */
	if (u || v)
	{
		/* Begin the inscription */
		*t++ = ' ';
		*t++ = c1;

		/* Standard inscription */
		if (u)
		{
			/* Append the inscription */
			while ((t < b + 75) && *u) *t++ = *u++;
		}

		/* Special inscription too */
		if (u && v && (t < b + 75))
		{
			/* Separator */
			*t++ = ',';
			*t++ = ' ';
		}

		/* Special inscription */
		if (v)
		{
			/* Append the inscription */
			while ((t < b + 75) && *v) *t++ = *v++;
		}

		/* Terminate the inscription */
		*t++ = c2;
	}


object_desc_done:

	/* Terminate */
	*t = '\0';

	/* Truncate the string to 80 chars */
	tmp_buf[79] = '\0';

	/* Copy the string over */
	strcpy(buf, tmp_buf);
}


/*
 * Hack -- describe an item currently in a store's inventory
 * This allows an item to *look* like the player is "aware" of it
 */
void object_desc_store(char *buf, const object_type *o_ptr, int pref, int mode)
{
	object_type *i_ptr;
	object_type object_type_body;

	u16b hack_flavor;
	bool hack_aware;


	/* Get local object */
	i_ptr = &object_type_body;

	/* Copy the object */
	object_copy(i_ptr, o_ptr);

	/* Save the "flavor" */
	hack_flavor = k_info[i_ptr->k_idx].flavor;

	/* Save the "aware" flag */
	hack_aware = k_info[i_ptr->k_idx].aware;

	/* Clear the flavor */
	k_info[i_ptr->k_idx].flavor = 0;

	/* Set the "known" flag */
	i_ptr->ident |= (IDENT_KNOWN);

	/* Force "aware" for description */
	k_info[i_ptr->k_idx].aware = TRUE;

	/* Describe the object */
	object_desc(buf, i_ptr, pref, mode);

	/* Restore "flavor" value */
	k_info[i_ptr->k_idx].flavor = hack_flavor;

	/* Restore "aware" flag */
	k_info[i_ptr->k_idx].aware = hack_aware;
}
/*
 * A qsort() hook for sorting bonus data.
 */
int sort_bonus_data(const void *arg1, const void *arg2)
{
	bonus_data *comp1 = (bonus_data *) arg1;
	bonus_data *comp2 = (bonus_data *) arg2;

	/* If they are different */
	if (comp1->bonus != comp2->bonus)
	{
		/* A zero is lower than anything else */
		if (comp1->bonus == 0) return (+1);
		if (comp2->bonus == 0) return (-1);
	}

	return (comp1->bonus - comp2->bonus);
}

/* Used for the 'item_info_desc'/'item_info_brief' functions */
#define IDENTIFY_STATUS_NONE	0
#define IDENTIFY_STATUS_KNOWN	1
#define IDENTIFY_STATUS_MENTAL	2

/*
 * Print a given object's resists/immunities.
 */
void obj_info_resists(byte *resists, bool shorten, char *buffer)
{
	int vn = 0, i;

	/* Count the stats affected */
	for (i = 0; i < RES_MAX; i++) if (resists[i]) vn++;

	/* Describe */
	if (vn)
	{
		bonus_data stat[RES_MAX];
		int current_stat = 0;
		int n = 0, iter = 0;
		int pluses = 0, minuses = 0;

		/* Make the structure */
		for (n = 0; n < RES_MAX; n++)
		{
			stat[n].name = (char *) res_names[n];
			stat[n].bonus = resists[n];
		}

		/* Sort the stats */
		qsort((void *) stat, RES_MAX, sizeof(bonus_data), sort_bonus_data);

		/* So we can start on 1, and avoid (n - 1) < 0 problems with array access */
		if (stat[0].bonus < 0) minuses ++;

		/* Loop over negative bonuses */
		for (n = 1; (n < RES_MAX) && (stat[n].bonus < 0); n++)
		{
			if (stat[n - 1].bonus != stat[n].bonus) minuses ++;
		}

		/* So we can avoid (n - 1) < 0 problems with array access */
		if (n < RES_MAX && stat[n].bonus > 0) pluses ++;

		/* Loop over negative bonuses */
		for (; (n < RES_MAX) && (stat[n].bonus > 0); n++)
		{
			if (stat[n - 1].bonus != stat[n].bonus) pluses ++;
		}

		/* Intro */
		if (!shorten) strcat(buffer, "It increases your resistance to ");

		n = 0;

		/* Loop - zeroes are at the bottom, we can stop at the first one */
		while (current_stat < RES_MAX && stat[current_stat].bonus != 0)
		{
			/* Temp */
			int no_the_same = 0;

			/* Count number of identical stats bounuses / penalties */
			vn = current_stat;
			while ((vn < RES_MAX) && (stat[vn].bonus == stat[current_stat].bonus))
			{
				no_the_same++;
				vn++;
			}

			/* Do the stats */
			for (iter = 0; iter < no_the_same; iter ++)
			{
				/* Stat name */
				strcat(buffer, stat[current_stat + iter].name);

				/* Connectives */
				if (iter == (no_the_same - 2)) strcat(buffer, " and ");
				else if (iter < (no_the_same - 2)) strcat(buffer, ", ");
				else strcat(buffer, " ");
			}

			/* Write out the bonus */
			strcat(buffer, "by ");
			strcat(buffer, format("%i", ABS(stat[current_stat].bonus)));
			strcat(buffer, "%");

			/* Move on */
			current_stat += no_the_same;

			n ++;

			/* Is there another stat to print? */
			if ((current_stat < RES_MAX) && (stat[current_stat].bonus))
			{
				strcat(buffer, ", ");
				if (stat[current_stat].bonus > 0)
				{
					if (n == pluses - 1) strcat(buffer, "and ");
				}
				else
				{
					if (n == minuses - 1) strcat(buffer, "and ");
				}

				strcat(buffer, "to ");
			}
		}

		/* Finish */
		strcat(buffer, ".  ");
	}

	/* We are done. */
	return;
}

/*
 * Calculate the multiplier we'll get with a given bow type.  This is done
 * differently in 2.8.2 than it was in 2.8.1.
 */
static int bow_multiplier(int sval)
{
	switch (sval)
	{
		case SV_SLING:
		case SV_SHORT_BOW:
			return 2;
		case SV_LONG_BOW:
		case SV_LIGHT_XBOW:
			return 3;
		case SV_HEAVY_XBOW:
			return 4;
		default:
			msg_format("Illegal bow sval %s\n", sval);
	}

	return 0;
}

/*
 * Output a description of an item.
 */
static void item_info_desc(const object_type *o_ptr, int mode)
{
	/* Does the item posess any special abilities at all? */
	bool abilities = FALSE;

	/* The "level" of identification of the item */
	byte identify_status = IDENTIFY_STATUS_NONE;

	/* An "object kind" */
	object_kind *k_ptr;

	/* The object name */
	char o_name[80];

	/* In the entire Angband! */
	int i = 0;

	/* A counter, and text */
	int vn;
	cptr vp[32];

	/* Temp */
	char temp[256];

	/* The item's flags, bonuses && resists */
	u32b f1, f2, f3;
	byte resists[RES_MAX];
	s16b stat_bonuses[A_MAX];

	/* Extract the correct flags, bonuses && resists */
	object_flags_aux(mode, o_ptr, &f1, &f2, &f3);
	object_resists_aux(mode, o_ptr, resists);
	object_stat_bonuses_aux(mode, o_ptr, stat_bonuses);

	/* Prepare the object base kind */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Check the level of identification */
	if (object_known_p(o_ptr)) identify_status = IDENTIFY_STATUS_KNOWN;
	if (o_ptr->ident & (IDENT_MENTAL)) identify_status = IDENTIFY_STATUS_MENTAL;

	/* Get the object description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Show the name */
	text_out_c(TERM_L_BLUE, format("%c%c%^s\n", 1, TERM_RED, o_name));

	/* Display the object description (if any) */
	if (k_info[o_ptr->k_idx].text)
	{
		text_out_c(TERM_ORANGE, k_text + k_info[o_ptr->k_idx].text);
		text_out("\n");
	}

	/* Describe activation */
	if (f3 & (TR3_ACTIVATE))
	{
		text_out("When it is worn, it can be activated ");
		describe_item_activation(o_ptr);
		text_out(".  ");

		abilities = TRUE;
	}

	/* Describe lights */
	if (f2 & ((TR2_LIGHT1) | (TR2_LIGHT2) | (TR2_LIGHT3) | (TR2_LIGHT4)))
	{
		/* Describe */
		text_out("It provides light, ");

		/* Show the radius */
		if (f2 & (TR2_LIGHT1)) text_out("radius 1");
		else if (f2 & (TR2_LIGHT2)) text_out("radius 2");
		else if (f2 & (TR2_LIGHT3)) text_out("radius 3");
		else if (f2 & (TR2_LIGHT4)) text_out("radius 4");

		/* Complete */
		if (f2 & (TR2_NEEDS_FUEL)) text_out(", when it is fueled");
		else text_out(", permanently");

		/* Finish */
		text_out(".  ");

		/* Item has abilities */
		abilities = TRUE;
	}

	/* Count the stats affected */
	vn = 0;
	for (i = 0; i < A_MAX; i++) if (stat_bonuses[i]) vn++;

	/* Describe */
	if (vn)
	{
		bonus_data stat[A_MAX];
		int current_stat = 0;
		int n = 0, iter = 0;
		int pluses = 0, minuses = 0;

		/* Make the structure */
		for (n = 0; n < A_MAX; n++)
		{
			stat[n].name = stat_names_full[n];
			stat[n].bonus = stat_bonuses[n];
		}

		/* Sort the stats */
		qsort((void *) stat, A_MAX, sizeof(bonus_data), sort_bonus_data);

		/* So we can start on 1, and avoid (n - 1) < 0 problems with array access */
		if (stat[0].bonus < 0) minuses ++;

		/* Loop over negative bonuses */
		for (n = 1; (n < A_MAX) && (stat[n].bonus < 0); n++)
		{
			if (stat[n - 1].bonus != stat[n].bonus) minuses ++;
		}

		/* So we can avoid (n - 1) < 0 problems with array access */
		if (n < A_MAX && stat[n].bonus > 0) pluses ++;

		/* Loop over negative bonuses */
		for (; (n < A_MAX) && (stat[n].bonus > 0); n++)
		{
			if (stat[n - 1].bonus != stat[n].bonus) pluses ++;
		}

		/* Intro */
		text_out("It ");

		/* Value */
		if (stat[current_stat].bonus > 0) text_out_c(TERM_L_GREEN, "increases");
		else if (stat[current_stat].bonus < 0) text_out_c(TERM_ORANGE, "decreases");

		n = 0;

		/* Loop - zeroes are at the bottom, we can stop at the first one */
		while (current_stat < A_MAX && stat[current_stat].bonus != 0)
		{
			/* Temp */
			int no_the_same = 0;

			/* Connective */
			text_out(" your ");

			/* Count number of identical stats bounuses / penalties */
			vn = current_stat;
			while ((vn < A_MAX) && (stat[vn].bonus == stat[current_stat].bonus))
			{
				no_the_same++;
				vn++;
			}

			/* Do the stats */
			for (iter = 0; iter < no_the_same; iter ++)
			{
				/* Stat name */
				text_out(stat[current_stat + iter].name);

				/* Connectives */
				if (iter == (no_the_same - 2)) text_out(" and ");
				else if (iter < (no_the_same - 2)) text_out(", ");
				else text_out(" ");
			}

			/* Write out the bonus */
			text_out("by ");
			text_out_c(TERM_L_GREEN, format("%i", ABS(stat[current_stat].bonus)));

			/* Move on */
			current_stat += no_the_same;

			n ++;

			/* Is there another stat to print? */
			if ((current_stat < A_MAX) && (stat[current_stat].bonus))
			{
				text_out(", ");
				if (stat[current_stat].bonus > 0)
				{
					if (stat[current_stat - no_the_same].bonus < 0)
					{
						text_out("but also increases");
						n = 0;
					}
					if (n == pluses - 1) text_out("and");
				}
				else
				{
					if (n == minuses - 1) text_out("and");
				}
			}
		}

		/* Finish */
		text_out(".  ");

		/* The object has abilities */
		abilities = TRUE;
	}

	/* Collect sustains */
	vn = 0;
	if (f2 & (TR2_SUST_STR)) vp[vn++] = "strength";
	if (f2 & (TR2_SUST_INT)) vp[vn++] = "intelligence";
	if (f2 & (TR2_SUST_WIS)) vp[vn++] = "wisdom";
	if (f2 & (TR2_SUST_DEX)) vp[vn++] = "dexterity";
	if (f2 & (TR2_SUST_CON)) vp[vn++] = "constitution";
	if (f2 & (TR2_SUST_CHR)) vp[vn++] = "charisma";

	/* Shorten, if appropriate */
	if (vn == 6)
	{
		text_out("It sustains all your stats.  ");
	}
	else if (vn)
	{
		int n;

		text_out("It sustains your ");

		for (n = 0; n < vn; n++)
		{
			text_out(vp[n]);

			if (n < vn - 2) text_out(", ");
			else if (n < (vn - 1)) text_out(" and ");
		}

		/* Finish */
		text_out(".  ");

		abilities = TRUE;
	}

	/* Count the stats affected */
	vn = 0;
	if (f1 & (TR1_STEALTH)) vp[vn++] = "stealth";
	if (f1 & (TR1_SEARCH)) vp[vn++] = "searching";
	if (f1 & (TR1_TUNNEL)) vp[vn++] = "ability to tunnel";
	if (f1 & (TR1_SPEED)) vp[vn++] = "speed";

	/* Describe */
	if (vn)
	{
		int n;

		/* Intro */
		text_out("It ");

		/* What does it do? */
		if (o_ptr->pval > 0) text_out_c(TERM_L_GREEN, "increases");
		else if (o_ptr->pval < 0) text_out_c(TERM_RED, "decreases");
		else if (o_ptr->pval == 0) text_out("does not affect [software bug]");

		/* Print stats */
		for (n = 0; n < vn; n++)
		{
			/* Connectives */
			if (n == 0) text_out(" your ");
			else if (n < (vn - 1)) text_out(", ");
			else text_out(" and ");

			/* Dump the stat */
			text_out(vp[n]);
		}

		/* What's the bonus? */
		text_out(" by ");
		text_out_c(TERM_L_GREEN, format("%i", ABS(o_ptr->pval)));

		/* Finish */
		text_out(".  ");

		abilities = TRUE;
	}


	if (f1 & (TR1_INFRA))
	{
		text_out(format("It increases your range of infravision by %i'.  ", o_ptr->pval * 10));
		abilities = TRUE;
	}

	if (f1 & (TR1_BLOWS))
	{
		text_out(format("It deals %i extra blow%s per round. ", o_ptr->pval, o_ptr->pval > 1 ? "s" : ""));
		abilities = TRUE;
	}

	if (f1 & (TR1_SHOTS))
	{
		text_out(format("It fires %i extra shot%s per round. ", o_ptr->pval, o_ptr->pval > 1 ? "s" : ""));
		abilities = TRUE;
	}

	if (f1 & (TR1_MIGHT))
	{
		text_out(format("Its damage multiplier is increased to x%i.  ", bow_multiplier(o_ptr->sval) + o_ptr->pval));
		abilities = TRUE;
	}

	/* Collect the slays */
	vn = 0;
	if (f1 & (TR1_SLAY_ANIMAL)) vp[vn++] = "natural creatures";
	if (f1 & (TR1_SLAY_EVIL)) vp[vn++] = "evil";
	if (f1 & (TR1_SLAY_ORC)) vp[vn++] = "orcs";
	if (f1 & (TR1_SLAY_TROLL)) vp[vn++] = "trolls";
	if (f1 & (TR1_SLAY_GIANT)) vp[vn++] = "giants";
	if (f1 & (TR1_SLAY_UNDEAD) && !(f1 & (TR1_KILL_UNDEAD))) vp[vn++] = "undead";
	if (f1 & (TR1_SLAY_DEMON) && !(f1 & (TR1_KILL_DEMON))) vp[vn++] = "demons";
	if (f1 & (TR1_SLAY_DRAGON) && !(f1 & (TR1_KILL_DRAGON))) vp[vn++] = "dragons";

	/* Describe */
	if (vn)
	{
		int n;

		/* Intro */
		text_out("It is very deadly");

		/* List the slays */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" against ");
			else if (n < (vn - 1)) text_out(", ");
			else text_out(" and ");

			/* Dump it */
			text_out(vp[n]);
		}

		text_out(".  ");

		abilities = TRUE;
	}

	/* Collect the executes */
	vn = 0;
	if (f1 & (TR1_KILL_DRAGON)) vp[vn++] = "dragons";
	if (f1 & (TR1_KILL_DEMON)) vp[vn++] = "demons";
	if (f1 & (TR1_KILL_UNDEAD)) vp[vn++] = "undead";

	/* Describe */
	if (vn)
	{
		int n;

		/* Intro */
		text_out("It is a great bane");

		/* List the slays */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" of ");
			else if (n < (vn - 1)) text_out(", ");
			else text_out(" and ");

			/* Dump it */
			text_out(vp[n]);
		}

		text_out(".  ");

		abilities = TRUE;
	}

	/* Collect the executes */
	vn = 0;
	if (f2 & (TR2_BRAND_ACID)) vp[vn++] = "acid";
	if (f2 & (TR2_BRAND_ELEC)) vp[vn++] = "electricity";
	if (f2 & (TR2_BRAND_FIRE)) vp[vn++] = "fire";
	if (f2 & (TR2_BRAND_COLD)) vp[vn++] = "cold";
	if (f2 & (TR2_BRAND_POIS)) vp[vn++] = "poison";
	if (f2 & (TR2_BRAND_NETHER)) vp[vn++] = "nether";
	if (f2 & (TR2_BRAND_CHAOS)) vp[vn++] = "chaos";
	if (f2 & (TR2_BRAND_NEXUS)) vp[vn++] = "nexus";

	/* Describe */
	if (vn)
	{
		int n;

		/* Intro */
		text_out("It is branded");

		/* List the slays */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" with ");
			else if (n < (vn - 1)) text_out(", ");
			else text_out(" and ");

			/* Dump it */
			text_out(vp[n]);
		}

		text_out(".  ");

		abilities = TRUE;
	}

	/* Describe the resists/immunities */
	temp[0] = '\0';
	obj_info_resists(resists, FALSE, temp);
	text_out(temp);

	/* Collect strange effects */
	vn = 0;
	if (f3 & (TR3_IMPACT)) vp[vn++] = "earthquakes";
	if (f3 & (TR3_HUNGER)) vp[vn++] = "hunger";
	if (f3 & (TR3_DRAIN_EXP)) vp[vn++] = "life draining";
	if (f3 & (TR3_AGGRAVATE)) vp[vn++] = "an aura of aggravation";

	/* Describe */
	if (vn > 0)
	{
		int n;

		text_out("It burdens its wearer ");

		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" with ");
			else if (n < (vn - 1)) text_out(", ");
			else text_out(" and ");

			/* Dump it */
			text_out(vp[n]);
		}

		text_out(".  ");
		abilities = TRUE;
	}

	/* Describe other (weird) things */

	if (f2 & (TR2_NO_BLIND)) text_out("It grants you immunity to blindness.  ");
	if (f2 & (TR2_NO_DISENCHANT)) text_out("It cannot be disenchanted.  ");
	if (f3 & (TR3_SLOW_DIGEST)) text_out("It slows your metabolism.  ");
	if (f3 & (TR3_FEATHER)) text_out("It induces feather falling.  ");
	if (f3 & (TR3_REGEN)) text_out("It speeds your regenerative powers.  ");
	if (f3 & (TR3_TELEPATHY)) text_out("It gives telepathic powers.  ");
	if (f3 & (TR3_SEE_INVIS)) text_out("It allows you to see invisible monsters.  ");
	if (f3 & (TR3_FREE_ACT)) text_out("It provides immunity to paralysis.  ");
	if (f3 & (TR3_HOLD_LIFE)) text_out("It provides resistance to life draining.  ");
	if (f3 & (TR3_TELEPORT)) text_out("It induces random teleportation.  ");
	if (f3 & (TR3_BLESSED)) text_out("It has been blessed by the gods.  ");

	/* Write curse status */
	if (object_known_p(o_ptr) && cursed_p(o_ptr))
	{
		if (f3 & (TR3_PERMA_CURSE))
		{
			text_out("It can never be uncursed.  ");
		}
		else if (f3 & (TR3_HEAVY_CURSE))
		{
			text_out("It carries a heavy curse. ");
		}
		else
		{
			text_out("It is cursed.  ");
		}

		abilities = TRUE;
	}

	/* Count the "ignore"s */
	vn = 0;
	if (f1 & (TR3_IGNORE_ACID)) vp[vn++] = "acid";
	if (f1 & (TR3_IGNORE_ELEC)) vp[vn++] = "lightning";
	if (f1 & (TR3_IGNORE_FIRE)) vp[vn++] = "fire";
	if (f1 & (TR3_IGNORE_COLD)) vp[vn++] = "cold";

	/* Describe */
	if (vn)
	{
		int n;

		text_out("It cannot be harmed ");


		/* Simplify things */
		if (vn == 3)
		{
			text_out("by the elements.  ");
		}

		else for (n = 0; n < vn; n++)
		{
			/* Connectives */
			if (n == 0) text_out("by ");
			else if (n < (vn - 1)) text_out(", ");
			else text_out(" and ");

			/* Dump the entry */
			text_out(vp[n]);
		}


		/* Finish */
		text_out(".  ");

		abilities = TRUE;
	}

	if (!abilities) text_out("There is nothing special about this item.");

	/* XXX */
	text_out("\n");

	/* Give the player warning of un-id'd things */
	if (identify_status == (IDENTIFY_STATUS_NONE))
	{
		text_out_c(TERM_RED, "You have not identified this item at all.  \n");
	}
	else if (identify_status == (IDENTIFY_STATUS_KNOWN))
	{
		text_out_c(TERM_L_BLUE, "You have identified this item.  \n");
	}
	else if (identify_status == (IDENTIFY_STATUS_MENTAL))
	{
		text_out_c(TERM_L_BLUE, "You have full knowledge of this item.  \n");
	}

	return;
}

/*
 * Show all abilities that an item posesses that a player knows about.
 */
void item_show_knowledge(const object_type *o_ptr)
{
	/* Save the screen */
	screen_save();

	/* Begin recall */
	Term_erase(0, 0, 255);

	/* Redirect output to the screen */
	text_out_hook = text_out_to_screen;

	/* Output the description */
	item_info_desc(o_ptr, OBJECT_AUX_KNOWN);

	/* Wait for a keypress */
	(void)inkey();

	/* Load the screen */
	screen_load();

	/* Return control */
	return;
}



/*
 * Describe an item's random attributes for "character dumps"
 */
void identify_backend(const object_type *o_ptr)
{
//	item_info_brief(o_ptr, OBJECT_AUX_KNOWN);

	return;
}


/*
 * Convert an inventory index into a one character label.
 *
 * Note that the label does NOT distinguish inven/equip.
 */
char index_to_label(int i)
{
	/* Indexes for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indexes for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Convert a label into the index of a item in the "equip".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_equip(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}



/*
 * Determine which equipment slot (if any) an item likes
 */
s16b wield_slot(const object_type *o_ptr)
{
	/* Slot for equipment */
	switch (o_ptr->tval)
	{
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			return (INVEN_WIELD);
		}

		case TV_BOW:
		{
			return (INVEN_BOW);
		}

		case TV_RING:
		{
			/* Use the right hand first */
			if (!inventory[INVEN_RIGHT].k_idx) return (INVEN_RIGHT);

			/* Use the left hand for swapping (by default) */
			return (INVEN_LEFT);
		}

		case TV_AMULET:
		{
			return (INVEN_NECK);
		}

		case TV_ORB:
		case TV_LIGHT:
		{
			return (INVEN_LIGHT);
		}

		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		{
			return (INVEN_BODY);
		}

		case TV_CLOAK:
		{
			return (INVEN_OUTER);
		}

		case TV_SHIELD:
		{
			return (INVEN_ARM);
		}

		case TV_CROWN:
		case TV_HELM:
		{
			return (INVEN_HEAD);
		}

		case TV_GLOVES:
		{
			return (INVEN_HANDS);
		}

		case TV_BOOTS:
		{
			return (INVEN_FEET);
		}

	        case TV_BELT:
		{
			return (INVEN_WAIST);
		}
}

	/* No slot available */
	return (-1);
}


/*
 * Return a string mentioning how a given item is carried
 */
cptr mention_use(int i)
{
	cptr p;

	/* Examine the location */
	switch (i)
	{
		case INVEN_WIELD: p = "Wielding"; break;
		case INVEN_BOW:   p = "Shooting"; break;
		case INVEN_LEFT:  p = "On left hand"; break;
		case INVEN_RIGHT: p = "On right hand"; break;
		case INVEN_NECK:  p = "Around neck"; break;
		case INVEN_LIGHT:  p = "Light source"; break;
		case INVEN_BODY:  p = "On body"; break;
		case INVEN_OUTER: p = "About body"; break;
		case INVEN_WAIST: p = "Around waist"; break;
		case INVEN_ARM:   p = "On arm"; break;
		case INVEN_HEAD:  p = "On head"; break;
		case INVEN_HANDS: p = "On hands"; break;
		case INVEN_FEET:  p = "On feet"; break;
		default:          p = "In pack"; break;
	}

	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "Just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "Just holding";
		}
	}

	/* Return the result */
	return (p);
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
cptr describe_use(int i)
{
	cptr p;

	switch (i)
	{
		case INVEN_WIELD: p = "attacking monsters with"; break;
		case INVEN_BOW:   p = "shooting missiles with"; break;
		case INVEN_LEFT:  p = "wearing on your left hand"; break;
		case INVEN_RIGHT: p = "wearing on your right hand"; break;
		case INVEN_NECK:  p = "wearing around your neck"; break;
		case INVEN_LIGHT:  p = "using to light the way"; break;
		case INVEN_BODY:  p = "wearing on your body"; break;
		case INVEN_OUTER: p = "wearing on your back"; break;
		case INVEN_WAIST: p = "wearing around your waist"; break;
		case INVEN_ARM:   p = "wearing on your arm"; break;
		case INVEN_HEAD:  p = "wearing on your head"; break;
		case INVEN_HANDS: p = "wearing on your hands"; break;
		case INVEN_FEET:  p = "wearing on your feet"; break;
		default:          p = "carrying in your pack"; break;
	}

	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just holding";
		}
	}

	/* Return the result */
	return p;
}





/*
 * Check an item against the item tester info
 */
bool item_tester_okay(const object_type *o_ptr)
{
	/* Hack -- allow listing empty slots */
	if (item_tester_full) return (TRUE);

	/* Require an item */
	if (!o_ptr->k_idx) return (FALSE);

	/* Hack -- ignore "gold" */
	if (o_ptr->tval == TV_GOLD) return (FALSE);

	/* Check the tval */
	if (item_tester_tval)
	{
		if (!(item_tester_tval == o_ptr->tval)) return (FALSE);
	}

	/* Check the hook */
	if (item_tester_hook)
	{
		if (!(*item_tester_hook)(o_ptr)) return (FALSE);
	}

	/* Assume okay */
	return (TRUE);
}



/*
 * Get the indexes of objects at a given floor location.
 *
 * Return the number of object indexes acquired.
 *
 * Never acquire more than "size" object indexes, and never return a
 * number bigger than "size", even if more floor objects exist.
 *
 * Valid flags are any combination of the bits:
 *
 *   0x01 -- Verify item tester
 *   0x02 -- Marked items only
 */
sint scan_floor(int *items, int size, int y, int x, int mode)
{
	int this_o_idx, next_o_idx;

	int num = 0;

	/* Sanity */
	if (!in_bounds(y, x)) return (0);

	/* Scan all objects in the grid */
	for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Verify item tester */
		if ((mode & 0x01) && !item_tester_okay(o_ptr)) continue;

		/* Marked items only */
		if ((mode & 0x02) && !o_ptr->marked) continue;

		/* Accept this item */
		items[num++] = this_o_idx;

		/* Enforce size limit */
		if (num >= size) break;
	}

	/* Result */
	return (num);
}



/*
 * Choice window "shadow" of the "show_inven()" function
 */
void display_inven(void)
{
	register int i, n, z = 0;

	object_type *o_ptr;

	byte attr;

	char tmp_val[80];

	char o_name[80];


	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the pack */
	for (i = 0; i < z; i++)
	{
		/* Examine the item */
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = index_to_label(i);

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i, 3, TERM_WHITE, tmp_val);

		/* Obtain an item description */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get inventory color */
		attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Display the entry itself */
		Term_putstr(3, i, n, attr, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i, 255);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(71, i, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = z; i < Term->hgt; i++)
	{
		/* Erase the line */
		Term_erase(0, i, 255);
	}
}



/*
 * Choice window "shadow" of the "show_equip()" function
 */
void display_equip(void)
{
	register int i, n;
	object_type *o_ptr;
	byte attr;

	char tmp_val[80];

	char o_name[80];


	/* Display the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Examine the item */
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = index_to_label(i);

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i - INVEN_WIELD, 3, TERM_WHITE, tmp_val);

		/* Obtain an item description */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get inventory color */
		attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Display the entry itself */
		Term_putstr(3, i - INVEN_WIELD, n, attr, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i - INVEN_WIELD, 255);

		/* Display the slot description (if needed) */
		if (show_labels)
		{
			Term_putstr(61, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
			Term_putstr(65, i - INVEN_WIELD, -1, TERM_WHITE, mention_use(i));
		}

		/* Display the weight (if needed) */
		if (show_weights && o_ptr->weight)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			int col = (show_labels ? 52 : 71);
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = INVEN_TOTAL - INVEN_WIELD; i < Term->hgt; i++)
	{
		/* Clear that line */
		Term_erase(0, i, 255);
	}
}



/*
 * Display the inventory.
 *
 * Hack -- do not display "trailing" empty slots
 */
void show_inven(void)
{
	int i, j, k, l, z = 0;
	int col, len, lim;

	object_type *o_ptr;

	char o_name[80];

	char tmp_val[80];

	int out_index[24];
	byte out_color[24];
	char out_desc[24][80];


	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;


	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the inventory */
	for (k = 0, i = 0; i < z; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the index */
		out_index[k] = i;

		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Save the object description */
		strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		if (show_weights) l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}


/*
 * Display the equipment.
 */
void show_equip(void)
{
	int i, j, k, l;
	int col, len, lim;

	object_type *o_ptr;

	char tmp_val[80];

	char o_name[80];

	int out_index[24];
	byte out_color[24];
	char out_desc[24][80];


	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for labels (if needed) */
	if (show_labels) lim -= (14 + 2);

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;

	/* Scan the equipment list */
	for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Description */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Truncate the description */
		o_name[lim] = 0;

		/* Save the index */
		out_index[k] = i;

		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Save the description */
		strcpy(out_desc[k], o_name);

		/* Extract the maximal length (see below) */
		l = strlen(out_desc[k]) + (2 + 3);

		/* Increase length for labels (if needed) */
		if (show_labels) l += (14 + 2);

		/* Increase length for weight (if needed) */
		if (show_weights) l += 9;

		/* Maintain the max-length */
		if (l > len) len = l;

		/* Advance the entry */
		k++;
	}

	/* Hack -- Find a column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j+1, col);

		/* Use labels */
		if (show_labels)
		{
			/* Mention the use */
			sprintf(tmp_val, "%-14s: ", mention_use(i));
			put_str(tmp_val, j+1, col + 3);

			/* Display the entry itself */
			c_put_str(out_color[j], out_desc[j], j+1, col + 3 + 14 + 2);
		}

		/* No labels */
		else
		{
			/* Display the entry itself */
			c_put_str(out_color[j], out_desc[j], j+1, col + 3);
		}

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j+1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}


#ifdef ALLOW_EASY_FLOOR

/*
 * Display a list of the items on the floor at the given location.
 */
void show_floor(const int *floor_list, int floor_num)
{
	int i, j, k, l;
	int col, len, lim;

	object_type *o_ptr;

	char o_name[80];

	char tmp_val[80];

	int out_index[24];
	byte out_color[24];
	char out_desc[24][80];


	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;

	/* Display the inventory */
	for (k = 0, i = 0; i < floor_num; i++)
	{
		o_ptr = &o_list[floor_list[i]];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the index */
		out_index[k] = i;

		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Save the object description */
		strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		if (show_weights) l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = floor_list[out_index[j]];

		/* Get the item */
		o_ptr = &o_list[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(out_index[j]));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}

#endif /* ALLOW_EASY_FLOOR */



/*
 * Flip "inven" and "equip" in any sub-windows
 */
void toggle_inven_equip(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* Unused */
		if (!angband_term[j]) continue;

		/* Flip inven to equip */
		if (op_ptr->window_flag[j] & (PW_INVEN))
		{
			/* Flip flags */
			op_ptr->window_flag[j] &= ~(PW_INVEN);
			op_ptr->window_flag[j] |= (PW_EQUIP);

			/* Window stuff */
			p_ptr->window |= (PW_EQUIP);
		}

		/* Flip inven to equip */
		else if (op_ptr->window_flag[j] & (PW_EQUIP))
		{
			/* Flip flags */
			op_ptr->window_flag[j] &= ~(PW_EQUIP);
			op_ptr->window_flag[j] |= (PW_INVEN);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN);
		}
	}
}




/*
 * Verify the choice of an item.
 *
 * The item can be negative to mean "item on floor".
 */
static bool verify_item(cptr prompt, int item)
{
	char o_name[80];

	char out_val[160];

	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Describe */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Prompt */
	sprintf(out_val, "%s %s? ", prompt, o_name);

	/* Query */
	return (get_check(out_val));
}


/*
 * Hack -- allow user to "prevent" certain choices.
 *
 * The item can be negative to mean "item on floor".
 */
static bool get_item_allow(int item)
{
	cptr s;

	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* No inscription */
	if (!o_ptr->note) return (TRUE);

	/* Find a '!' */
	s = strchr(quark_str(o_ptr->note), '!');

	/* Process preventions */
	while (s)
	{
		/* Check the "restriction" */
		if ((s[1] == p_ptr->command_cmd) || (s[1] == '*'))
		{
			/* Verify the choice */
			if (!verify_item("Really try", item)) return (FALSE);
		}

		/* Find another '!' */
		s = strchr(s + 1, '!');
	}

	/* Allow it */
	return (TRUE);
}


/*
 * Verify the "okayness" of a given item.
 *
 * The item can be negative to mean "item on floor".
 */
static bool get_item_okay(int item)
{
	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Verify the item */
	return (item_tester_okay(o_ptr));
}



/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the "current" p_ptr->command_cmd code.
 */
static int get_tag(int *cp, char tag)
{
	int i;
	cptr s;


	/* Check every object */
	for (i = 0; i < INVEN_TOTAL; ++i)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Skip empty inscriptions */
		if (!o_ptr->note) continue;

		/* Find a '@' */
		s = strchr(quark_str(o_ptr->note), '@');

		/* Process all tags */
		while (s)
		{
			/* Check the normal tags */
			if (s[1] == tag)
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Check the special tags */
			if ((s[1] == p_ptr->command_cmd) && (s[2] == tag))
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}

	/* No such tag */
	return (FALSE);
}



/*
 * Let the user select an item, save its "index"
 *
 * Return TRUE only if an acceptable item was chosen by the user.
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.
 *
 * The equipment or inventory are displayed (even if no acceptable
 * items are in that location) if the proper flag was given.
 *
 * If there are no acceptable items available anywhere, and "str" is
 * not NULL, then it will be used as the text of a warning message
 * before the function returns.
 *
 * Note that the user must press "-" to specify the item on the floor,
 * and there is no way to "examine" the item on the floor, while the
 * use of "capital" letters will "examine" an inventory/equipment item,
 * and prompt for its use.
 *
 * If a legal item is selected from the inventory, we save it in "cp"
 * directly (0 to 35), and return TRUE.
 *
 * If a legal item is selected from the floor, we save it in "cp" as
 * a negative (-1 to -511), and return TRUE.
 *
 * If no item is available, we do nothing to "cp", and we display a
 * warning message, using "str" if available, and return FALSE.
 *
 * If no item is selected, we do nothing to "cp", and return FALSE.
 *
 * Global "p_ptr->command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens, and
 * also to induce "auto-enter" of stores, and other such stuff.
 *
 * Global "p_ptr->command_see" may be set before calling this function to start
 * out in "browse" mode.  It is cleared before this function returns.
 *
 * Global "p_ptr->command_wrk" is used to choose between equip/inven/floor
 * listings.  It is equal to USE_INVEN or USE_EQUIP or USE_FLOOR, except
 * when this function is first called, when it is equal to zero, which will
 * cause it to be set to USE_INVEN.
 *
 * We always erase the prompt when we are done, leaving a blank line,
 * or a warning message, if appropriate, if no items are available.
 *
 * Note that the "easy_floor" option affects this function in several ways.
 *
 * Note that only "acceptable" floor objects get indexes, so between two
 * commands, the indexes of floor objects may change.  XXX XXX XXX
 */
bool get_item(int *cp, cptr pmt, cptr str, int mode)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	char which;

	int i, j, k;

	int i1, i2;
	int e1, e2;
	int f1, f2;

	bool done, item;

	bool oops = FALSE;

	bool use_inven = ((mode & (USE_INVEN)) ? TRUE : FALSE);
	bool use_equip = ((mode & (USE_EQUIP)) ? TRUE : FALSE);
	bool use_floor = ((mode & (USE_FLOOR)) ? TRUE : FALSE);

	bool allow_inven = FALSE;
	bool allow_equip = FALSE;
	bool allow_floor = FALSE;

	bool toggle = FALSE;

	char tmp_val[160];
	char out_val[160];

	int floor_list[24];
	int floor_num;


#ifdef ALLOW_REPEAT

	/* Get the item index */
	if (repeat_pull(cp))
	{
		/* Verify the item */
		if (get_item_okay(*cp))
		{
			/* Forget the item_tester_tval restriction */
			item_tester_tval = 0;

			/* Forget the item_tester_hook restriction */
			item_tester_hook = NULL;

			/* Success */
			return (TRUE);
		}
		else
		{
			/* Invalid repeat - reset it */
			repeat_clear();
		}
	}

#endif /* ALLOW_REPEAT */


	/* Paranoia XXX XXX XXX */
	message_flush();


	/* Not done */
	done = FALSE;

	/* No item selected */
	item = FALSE;


	/* Full inventory */
	i1 = 0;
	i2 = INVEN_PACK - 1;

	/* Forbid inventory */
	if (!use_inven) i2 = -1;

	/* Restrict inventory indexes */
	while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
	while ((i1 <= i2) && (!get_item_okay(i2))) i2--;

	/* Accept inventory */
	if (i1 <= i2) allow_inven = TRUE;


	/* Full equipment */
	e1 = INVEN_WIELD;
	e2 = INVEN_TOTAL - 1;

	/* Forbid equipment */
	if (!use_equip) e2 = -1;

	/* Restrict equipment indexes */
	while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
	while ((e1 <= e2) && (!get_item_okay(e2))) e2--;

	/* Accept equipment */
	if (e1 <= e2) allow_equip = TRUE;


	/* Scan all objects in the grid */
	floor_num = scan_floor(floor_list, 23, py, px, 0x00);

	/* Full floor */
	f1 = 0;
	f2 = floor_num - 1;

	/* Forbid floor */
	if (!use_floor) f2 = -1;

	/* Restrict floor indexes */
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f1]))) f1++;
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f2]))) f2--;

	/* Accept floor */
	if (f1 <= f2) allow_floor = TRUE;


	/* Require at least one legal choice */
	if (!allow_inven && !allow_equip && !allow_floor)
	{
		/* Cancel p_ptr->command_see */
		p_ptr->command_see = FALSE;

		/* Oops */
		oops = TRUE;

		/* Done */
		done = TRUE;
	}

	/* Analyze choices */
	else
	{
		/* Hack -- Start on equipment if requested */
		if (p_ptr->command_see &&
		    (p_ptr->command_wrk == (USE_EQUIP)) &&
		    use_equip)
		{
			p_ptr->command_wrk = (USE_EQUIP);
		}

		/* Use inventory if allowed */
		else if (use_inven)
		{
			p_ptr->command_wrk = (USE_INVEN);
		}

		/* Use equipment if allowed */
		else if (use_equip)
		{
			p_ptr->command_wrk = (USE_EQUIP);
		}

#ifdef ALLOW_EASY_FLOOR

		/* Use floor if allowed */
		else if (easy_floor)
		{
			p_ptr->command_wrk = (USE_FLOOR);
		}

#endif /* ALLOW_EASY_FLOOR */

		/* Hack -- Use (empty) inventory */
		else
		{
			p_ptr->command_wrk = (USE_INVEN);
		}
	}


	/* Start out in "display" mode */
	if (p_ptr->command_see)
	{
		/* Save screen */
		screen_save();
	}


	/* Repeat until done */
	while (!done)
	{
		/* Show choices */
		if (show_choices)
		{
			int ni = 0;
			int ne = 0;

			/* Scan windows */
			for (j = 0; j < ANGBAND_TERM_MAX; j++)
			{
				/* Unused */
				if (!angband_term[j]) continue;

				/* Count windows displaying inven */
				if (op_ptr->window_flag[j] & (PW_INVEN)) ni++;

				/* Count windows displaying equip */
				if (op_ptr->window_flag[j] & (PW_EQUIP)) ne++;
			}

			/* Toggle if needed */
			if (((p_ptr->command_wrk == (USE_EQUIP)) && ni && !ne) ||
			    ((p_ptr->command_wrk == (USE_INVEN)) && !ni && ne))
			{
				/* Toggle */
				toggle_inven_equip();

				/* Track toggles */
				toggle = !toggle;
			}

			/* Update */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);

			/* Redraw windows */
			window_stuff();
		}

		/* Viewing inventory */
		if (p_ptr->command_wrk == (USE_INVEN))
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_inven();

			/* Begin the prompt */
			sprintf(out_val, "Inven:");

			/* List choices */
			if (i1 <= i2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,",
				        index_to_label(i1), index_to_label(i2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");

			/* Indicate legality of "toggle" */
			if (use_equip) strcat(out_val, " / for Equip,");

			/* Indicate legality of the "floor" */
			if (allow_floor) strcat(out_val, " - for floor,");
		}

		/* Viewing equipment */
		else if (p_ptr->command_wrk == (USE_EQUIP))
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_equip();

			/* Begin the prompt */
			sprintf(out_val, "Equip:");

			/* List choices */
			if (e1 <= e2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,",
				        index_to_label(e1), index_to_label(e2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");

			/* Indicate legality of "toggle" */
			if (use_inven) strcat(out_val, " / for Inven,");

			/* Indicate legality of the "floor" */
			if (allow_floor) strcat(out_val, " - for floor,");
		}

#ifdef ALLOW_EASY_FLOOR

		/* Viewing floor */
		else
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_floor(floor_list, floor_num);

			/* Begin the prompt */
			sprintf(out_val, "Floor:");

			/* List choices */
			if (f1 <= f2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,", I2A(f1), I2A(f2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");

			/* Append */
			if (use_inven) strcat(out_val, " / for Inven,");

			/* Append */
			else if (use_equip) strcat(out_val, " / for Equip,");
		}

#endif /* ALLOW_EASY_FLOOR */

		/* Finish the prompt */
		strcat(out_val, " ESC");

		/* Build the prompt */
		sprintf(tmp_val, "(%s) %s", out_val, pmt);

		/* Show the prompt */
		prt(tmp_val, 0, 0);


		/* Get a key */
		which = inkey();

		/* Parse it */
		switch (which)
		{
			case ESCAPE:
			{
				done = TRUE;
				break;
			}

			case '*':
			case '?':
			case ' ':
			{
				/* Hide the list */
				if (p_ptr->command_see)
				{
					/* Flip flag */
					p_ptr->command_see = FALSE;

					/* Load screen */
					screen_load();
				}

				/* Show the list */
				else
				{
					/* Save screen */
					screen_save();

					/* Flip flag */
					p_ptr->command_see = TRUE;
				}

				break;
			}

			case '/':
			{
				/* Toggle to inventory */
				if (use_inven && (p_ptr->command_wrk != (USE_INVEN)))
				{
					p_ptr->command_wrk = (USE_INVEN);
				}

				/* Toggle to equipment */
				else if (use_equip && (p_ptr->command_wrk != (USE_EQUIP)))
				{
					p_ptr->command_wrk = (USE_EQUIP);
				}

				/* No toggle allowed */
				else
				{
					bell("Cannot switch item selector!");
					break;
				}

				/* Hack -- Fix screen */
				if (p_ptr->command_see)
				{
					/* Load screen */
					screen_load();

					/* Save screen */
					screen_save();
				}

				/* Need to redraw */
				break;
			}

			case '-':
			{
				/* Paranoia */
				if (!allow_floor)
				{
					bell("Cannot select floor!");
					break;
				}

#ifdef ALLOW_EASY_FLOOR

				if (easy_floor)
				{
					/* There is only one item */
					if (floor_num == 1)
					{
						/* Hack -- Auto-Select */
						if ((p_ptr->command_wrk == (USE_FLOOR)) ||
						    (!floor_query_flag))
						{
							/* Special index */
							k = 0 - floor_list[0];

							/* Allow player to "refuse" certain actions */
							if (!get_item_allow(k))
							{
								done = TRUE;
								break;
							}

							/* Accept that choice */
							(*cp) = k;
							item = TRUE;
							done = TRUE;

							break;
						}
					}

					/* Hack -- Fix screen */
					if (p_ptr->command_see)
					{
						/* Load screen */
						screen_load();

						/* Save screen */
						screen_save();
					}

					p_ptr->command_wrk = (USE_FLOOR);

					break;
				}

#endif /* ALLOW_EASY_FLOOR */

				/* Check each legal object */
				for (i = 0; i < floor_num; ++i)
				{
					/* Special index */
					k = 0 - floor_list[i];

					/* Skip non-okay objects */
					if (!get_item_okay(k)) continue;

					/* Verify the item (if required) */
					if (floor_query_flag && !verify_item("Try", k)) continue;

					/* Allow player to "refuse" certain actions */
					if (!get_item_allow(k)) continue;

					/* Accept that choice */
					(*cp) = k;
					item = TRUE;
					done = TRUE;
					break;
				}

				break;
			}

			case '0':
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
			{
				/* Look up the tag */
				if (!get_tag(&k, which))
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Hack -- Validate the item */
				if ((k < INVEN_WIELD) ? !allow_inven : !allow_equip)
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			case '\n':
			case '\r':
			{
				/* Choose "default" inventory item */
				if (p_ptr->command_wrk == (USE_INVEN))
				{
					if (i1 != i2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = i1;
				}

				/* Choose "default" equipment item */
				else if (p_ptr->command_wrk == (USE_EQUIP))
				{
					if (e1 != e2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = e1;
				}

#ifdef ALLOW_EASY_FLOOR

				/* Choose "default" floor item */
				else
				{
					if (f1 != f2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = 0 - floor_list[f1];
				}

#endif /* ALLOW_EASY_FLOOR */

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (default)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			default:
			{
				bool verify;

				/* Note verify */
				verify = (isupper(which) ? TRUE : FALSE);

				/* Lowercase */
				which = tolower(which);

				/* Convert letter to inventory index */
				if (p_ptr->command_wrk == (USE_INVEN))
				{
					k = label_to_inven(which);

					if (k < 0)
					{
						bell("Illegal object choice (inven)!");
						break;
					}
				}

				/* Convert letter to equipment index */
				else if (p_ptr->command_wrk == (USE_EQUIP))
				{
					k = label_to_equip(which);

					if (k < 0)
					{
						bell("Illegal object choice (equip)!");
						break;
					}
				}

#ifdef ALLOW_EASY_FLOOR

				/* Convert letter to floor index */
				else
				{
					k = (islower(which) ? A2I(which) : -1);

					if (k < 0 || k >= floor_num)
					{
						bell("Illegal object choice (floor)!");
						break;
					}

					/* Special index */
					k = 0 - floor_list[k];
				}

#endif /* ALLOW_EASY_FLOOR */

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (normal)!");
					break;
				}

				/* Verify the item */
				if (verify && !verify_item("Try", k))
				{
					done = TRUE;
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
		}
	}


	/* Fix the screen if necessary */
	if (p_ptr->command_see)
	{
		/* Load screen */
		screen_load();

		/* Hack -- Cancel "display" */
		p_ptr->command_see = FALSE;
	}


	/* Forget the item_tester_tval restriction */
	item_tester_tval = 0;

	/* Forget the item_tester_hook restriction */
	item_tester_hook = NULL;


	/* Clean up */
	if (show_choices)
	{
		/* Toggle again if needed */
		if (toggle) toggle_inven_equip();

		/* Update */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Window stuff */
		window_stuff();
	}


	/* Clear the prompt line */
	prt("", 0, 0);

	/* Warning if needed */
	if (oops && str) msg_print(str);

#ifdef ALLOW_REPEAT

	/* Save item if available */
	if (item) repeat_push(*cp);

#endif /* ALLOW_REPEAT */

	/* Result */
	return (item);
}

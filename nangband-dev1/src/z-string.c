/*
 * File: z-string.c
 *
 * Abstract: Helper functions for modifying, comparing and managing
 *           strings.
 *
 * For licencing terms, please see angband.h.
 */
#include "z-string.h"
#include "z-virt.h"

/*** String comparison functions ***/

/*
 * Perform a case-insensitive comparison between "s1" and "s2",
 * two NUL-terminated strings.
 *
 * In the case of a difference, we return the difference between
 * the first two characters which do not match; otherwise, we
 * return 0.
 */
int my_stricmp(cptr s1, cptr s2)
{
	char ch1, ch2;

	/* Cycle through the entire string */
	while ((*s1) || (*s2))
	{
		/* Simplify the comparison */
		ch1 = toupper(*s1);
		ch2 = toupper(*s2);

		/* If the two characters don't match, return the difference */
		if (ch1 != ch2) return ((int) (ch1 - ch2));

		/* Step on through both strings */
		s1++;
		s2++;
	}

	return (0);
}

/*
 * Determine if string "s1" is equal to string "s2".
 */
bool streq(cptr s1, cptr s2)
{
	return (!strcmp(s1, s2));
}

/*
 * Determine if string "s1" is case-insensitively equal to "s2".
 */
bool strieq(cptr s1, cptr s2)
{
	return (!my_stricmp(s1, s2));
}


/*
 * Determine if string "t" is a suffix of string "s".
 */
bool suffix(cptr s, cptr t)
{
	int tlen = strlen(t);
	int slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (FALSE);

	/* Compare "t" to the end of "s" */
	return (!strcmp(s + slen - tlen, t));
}


/*
 * Determine if string "t" is a prefix of string "s".
 */
bool prefix(cptr s, cptr t)
{
	/* Scan "t" */
	while (*t)
	{
		/* Compare content and length */
		if (*t++ != *s++) return (FALSE);
	}

	/* Matched, we have a prefix */
	return (TRUE);
}


/*** String modification functions ***/

/* None - take from Angband CVS. */


/*** String management functions ***/

/*
 * The "strtable" functions deal with managing dynamic strings in an indexed
 * table.  They are similar to the "quark" and "message" packages in that
 * they all use a "string table" (which is not, by the way, a table made
 * out of fabric).  However, this package is designed for a more general use
 * than that of the "quark" and "message" packages.
 *
 * Basic use is to keep an index stored (this is a u32b), which is used by
 * this package.  To create a string, use "strtable_add(string)", where
 * "string" is the string you want to create.  Assign the return value to
 * a safe place; this will be used to get it back and/or modify it later.
 * To retrieve the string, use "strtable_content(index)"; to modify it,
 * use "strtable_modify(index, newstring)".
 *
 * When you are finished with it, you MUST call "strtable_remove(index)"
 * to prevent nasty memory-related things.
 *
 * Package created by Andrew Sidwell, 20:20 2002-08-10.
 */

/*
 * Table of pointers to content.
 */
static cptr strtable__content[MAX_SIZE_STRTABLE];

/*
 * Refcounting table for the content table.
 */
static byte strtable__refcount[MAX_SIZE_STRTABLE];


/*
 * Find the first free index in strtable__content[].
 * Return a refrence to it, 0 if one can't be found.
 */
static u32b strtable_locate_first_free(void)
{
	int i;

	for (i = 1; i < MAX_SIZE_STRTABLE; i++)
	{
		if (!strtable__refcount[i]) return (i);
	}

	return (0);
}

/*
 * Return the content of a given index.
 */
cptr strtable_content(u32b index)
{
	/* Paranoia */
	if (index >= MAX_SIZE_STRTABLE) index = 0;
	if (!strtable__refcount[index]) index = 0;

	/* Return the content */
	return (strtable__content[index]);
}

/*
 * Add an entry to the strtable
 *
 * We search strtable__content for a string equal to the new one; if we find
 * it, we return the index of that string in the table, after increasing the
 * refcount for that string. Otherwise, we call "make_string()" to allocate
 * memory for the new string and then we place it at the first free slot.
 */
u32b strtable_add(cptr text)
{
	int i;
	int free = 0;

	/* First try and find an entry which is the same as "text" */
	for (i = 1; i < MAX_SIZE_STRTABLE; i++)
	{
		if (!strtable__refcount[i]) continue;

		if (streq(strtable__content[i], text))
		{
			/* Increase the refcount */
			strtable__refcount[i]++;

			/* Return the index we found */
			return (i);
		}
	}

	/* Find the first free string */
	free = strtable_locate_first_free();

	/* Check that we found a free space */
	if (free == 0) return (0);

	/* Ensure the next available space really is available */
	if (strtable__refcount[free]) return (0);

	/* We are referencing this */
	strtable__refcount[free]++;

	/* Create a stable memory location */
	strtable__content[free] = string_make(text);

	/* Return the index to this string */
	return (free);
}

/*
 * Remove (or lower the refcount) an entry in the strtable.
 *
 * This proceduce consists of deallocating the memory of the old string if it
 * is not referenced by anything else; in either case, we decrease the
 * refcount by one.
 */
void strtable_remove(u32b idx)
{
	/* Paranoia */
	if (idx >= MAX_SIZE_STRTABLE) return;
	if (strtable__refcount[idx] == 0) return;

	/* Decease the refcount */
	strtable__refcount[idx]--;

	/* Free the string if nothing references this string */
	if (!strtable__refcount[idx])
	{
		string_free(strtable__content[idx]);

	}

	/* We are finished here. */
	return;
}

/*
 * Modify a string in the table.
 *
 * Note that we actually just call strtable_remove() and strtable_add() -
 * not the most efficient way, but it saves on code duplication. XXX XXX
 *
 * Note that the index of the text may change; always use something
 * similar to the following when calling:
 * 'name = strtable_modify(name, "Boo!");',
 * where "name" is a "u32b".
 */
u32b strtable_modify(u32b idx, cptr text)
{
	/* Free the old string */
	strtable_remove(idx);

	/* Make the new string */
	return (strtable_add(text));
}

/*
 * "Copy" from from to to.
 */
void strtable_copy(u32b from, u32b *to)
{
	/* Paranoia */
	if (from >= MAX_SIZE_STRTABLE) return;
	if (!strtable__refcount[from]) return;

	/* Increase the refcount */
	strtable__refcount[from]++;

	/* "Copy" */
	*to = from;

	/* Done */
	return;
}

/*
 * Initialise the table (set all memory to 0).
 *
 * Return FALSE if we fail.
 */
bool strtable_init(void)
{
	int i;

	/* Reset the memory */
	for (i = 0; i < MAX_SIZE_STRTABLE; i++)
		strtable__content[i] = 0;
	for (i = 0; i < MAX_SIZE_STRTABLE; i++)
		strtable__refcount[i] = 0;

	/* We succeded */
	return (TRUE);
}

/*
 * Clean up the string table function.
 */
void strtable_cleanup(void)
{
	int i;

	for (i = 0; i < MAX_SIZE_STRTABLE; i++)
	{
		if (strtable__refcount[i])
		{
			string_free(strtable__content[i]);
			strtable__refcount[i] = 0;
		}
	}

	return;
}

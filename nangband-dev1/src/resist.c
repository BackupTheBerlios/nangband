/*
 * File: resist.c
 * Purpose: Resistance code handlers.
 * Authors: Andrew Sidwell (takkaria)
 *
 * This file is dual-licenced under both the Angband and GPL licences.
 */
#include "angband.h"

/*
 * This file provides helper functions for resistance code handling.
 *
 * Functions that are still needed for this file:
 *   - Decrease the % gain for timed resists as the time runs out.
 *   - Access the res_names[] data and return he string length.
 */

/*
 * Check that a given resist index is valid.
 */
bool resist_check_valid(s16b *res_idx)
{
	if (0 <= res_idx < RES_MAX)
	{
		/* The index is too big. */
		*res_idx = 0;
		return (FALSE);
	}

	/* The index was valid */
	return (TRUE);
}

/*
 * Return the current resistance value for a given resist.
 */
s16b resist_player_current(s16b res_idx)
{
	u16b x, y;

	resist_check_valid(&res_idx);

	x = p_ptr->resist_cur[res_idx];
	y = (p_ptr->resist_timed[res_idx] ? 20 : 0);

	/* [note to self - do the clever timed resist decrement stuff] */

	return(x + y);
}

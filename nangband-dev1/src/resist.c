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
bool resist_check_valid(byte *res_idx)
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
int resist_player_current(byte res_idx)
{
	u16b x, y;

	resist_check_valid(&res_idx);

	x = p_ptr->resist_cur[res_idx];
	y = (p_ptr->resist_timed[res_idx] ? 20 : 0);

	/* [note to self - do the clever timed resist decrement stuff] */

	return(x + y);
}

/*
 * Apply a resistance.
 *
 * Mostly Eytan's code from EyAngband.
 */
int resist_apply(byte res_amnt, int dam)
{
	/* No effect */
	if ((1 > dam) || !res_amnt) return (dam);

	/* Immunity */
	if (res_amnt >= 100) return (0);

	/* Apply the resistance */
	dam = (dam * (100 - res_amnt)) / 100;

	/* Resistances can't lower damage to 0 */
	if (dam < 1) dam = 1;

	return (dam);
}

/*
 * File: resist.c
 *
 * Abstract: Code for the percentile resistance implementation;
 *           At the moment, this only includes player resists,
 *           it will contain monster resists soon.
 * 
 * Authors: Andrew Sidwell (takkaria), Eytan Zweing (eytan).
 *
 * This file is dual-licenced under both the Angband and GPL licences.
 */
#include "angband.h"

/* A quick little macro for testing a resist index. */
#define RES_VALIDIZE(t) if (!resist_check_valid(t)) t = 0;

/*
 * Check that a given resist index is valid.
 */
bool resist_check_valid(byte res_idx)
{
	if ((0 < res_idx) || (res_idx < RES_MAX))
	{
		/* The index is too big. */
		return (FALSE);
	}

	/* The index was valid */
	return (TRUE);
}

/*
 * Return the current resistance value for a given resist.
 */
byte resist_player_current(byte res_idx)
{
	sbyte x = 0;

	/* Make sure the resist is valid */
	RES_VALIDIZE(res_idx);

	/* Grab the current resist value */
	x += (p_ptr->resist_cur[res_idx] > 100 ? 100 : p_ptr->resist_cur[res_idx]);

	/* Hack to avoid work */
	if (!p_ptr->resist_timed[res_idx]) return (x);

	/* Do some complex math to work out timed resist */
	if ((p_ptr->resist_timed[res_idx] * 100) <= (75 * p_ptr->resist_tim_max[res_idx]))
	{
		x += 50 - ((-30/75) * p_ptr->resist_timed[res_idx]);
	}
	else
	{
		x += 20 - ((-20/25) * (p_ptr->resist_timed[res_idx] - 20));
	}

	/* Return the total resistance */
	return (x);
}

/*
 * Roll a 100-sided die and check th result agaainst
 * a resist.
 */
bool resist_check(byte res_idx)
{
	u16b x;

	/* Make sure the resist is valid */
	RES_VALIDIZE(res_idx);

	/* Grab the value */
	x = resist_player_current(res_idx);

	/* Roll the die */
	if (rand_int(100) > x) return (FALSE);

	/* The check was successful */
	return (TRUE);
}

/*
 * Apply a resistance.
 *
 * Mostly Eytan's code from EyAngband.
 */
int resist_apply(int amount, int dam)
{
	/* No effect */
	if ((1 > dam) || !amount) return (dam);

	/* Complete immunity */
	if (amount >= 100) return (0);

	/* Apply the resistance */
	dam = (dam * (100 - amount)) / 100;

	/* Resistances can't lower damage to 0 */
	if (dam < 1) dam = 1;

	/* Return the damage */
	return (dam);
}


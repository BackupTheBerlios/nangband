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
		x += 50 + ((30 * p_ptr->resist_timed[res_idx]) / 75);
	}
	else
	{
		x += 20 - ((-20 * (p_ptr->resist_timed[res_idx] - 20)) / 25);
	}

	/* Return the total resistance */
	return (x);
}

/*
 * Roll a 100-sided die and check the result agaainst a resist.
 */
bool resist_check(byte res_idx)
{
	u16b x;

	/* Make sure the resist is valid */
	RES_VALIDIZE(res_idx);

	/* Grab the value */
	x = resist_player_current(res_idx);

	/* Return the correct value */
	return (rand_int(100) > resist_player_current(res_idx));
}

/*
 * Return (100 - res_percent)% of 'dam'.  If 'dam' is below 1, or 'res_percent'
 * is 0, 'dam' is returned.  If res_percent is over/above 100, then we return 0;
 * we ensure that resistances can't lower damage down to 0.
 */
int resist_apply(sbyte res_percent, int dam)
{
	/* No effect */
	if ((dam < 1) || !res_percent) return (dam);

	/* Complete immunity */
	if (res_percent >= 100) return (0);

	/* Apply the resistance */
	dam = (dam * (100 - res_percent)) / 100;

	/* Resistances can't lower damage to 0 */
	if (dam < 1) dam = 1;

	/* Return the damage */
	return (dam);
}


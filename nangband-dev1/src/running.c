/*
 * File: running.c
 *
 * Abstract: Running code for Nangband.
 *
 * Author: Andrew Sidwell (takkaria)
 *
 * Licence: The GNU GPL, version 2 or any later version.
 */
#include "angband.h"

static byte run_dir;

static const byte x_mod[10] = {0, -1, +0, +1, -1, +0, +1, -1, +0, +1};
static const byte y_mod[10] = {0, +1, +1, +1, +0, +0, +0, -1, -1, -1};

static const byte cycle_index[8] = {0, 1, 2, 4, 5, 6, 7, 3};
static const byte iter_to_dir[8] = {1, 2, 3, 4, 6, 7, 8, 9};

static const byte xy_to_abs[3][3] =
{{5, 6, 7},
 {3, 8, 4},
 {0, 1, 2}};

/*
 * Running helper function: is_walkable(x, y).
 * This function decides whether position (x,y) is 'walkable' or not.
 *
 * Returns TRUE if it is, FALSE if not.
 */
static bool is_walkable(int x, int y)
{
	/* The curent grid is not 'walkable' */
	if ((x == p_ptr->py) && (y == p_ptr->py)) return (FALSE);

	/* Things that block movement are not walkable */
	if (f_info[cave_feat[y][x]].f1 & (FF1_BLOCKING)) return (FALSE);

	/* We don't want to run into stores */
	if (f_info[cave_feat[y][x]].f1 & (FF1_STORE)) return (FALSE);

	/* We default to TRUE */
	return (TRUE);
}

/*
 * Main function: continue running.
 */
static bool run_continue(void)
{
	bool okay[8];
	bool in_path = FALSE;
	int i, d, p, x, y;

	/*** Step One - Build Flow Information and Map ***/

	/* Get the basic okayness. */
	for (i = 0, d = 1, p = 0; i < 8; i++, d = iter_to_dir[i])
	{
		/* Set x & y for accessing possible player positions */
		x = (p_ptr->px + x_mod[d]);
		y = (p_ptr->py + y_mod[d]);

		/* Check for okay-ness */
		if (is_walkable(x, y)) okay[i] = TRUE;
		else okay[i] = FALSE;

		/* The previous direction isn't walkable - XXX */
		if (d == (10 - run_dir)) okay[i] = FALSE;

		/* Increase the number of known paths */
		if (okay[i]) p++;
	}

 	/* If we have no known paths, then we can't go anywhere */
	if (!p) return (FALSE);

	/*** Stage Two - Count "Distinct Paths" ***/

	/* Cycle through the okay grids */
	for (i = 0, d = 0, p = 0, x = 0; i < 8; i++, d = cycle_index[i])
	{
		/* If we're not in a path, but we're on an okay grid ... */
		if (!in_path && okay[d])
		{
			/* We are now in a path */
			in_path = TRUE;
		}

		/* Otherwise, if we're in a path and we're on a bad grid ... */
		else if (in_path && !okay[d])
		{
			/* We are no longer in a path */
			in_path = FALSE;

			/* Increase the count if we have a distinct path */
			if (y < 3)
			{
				x = y;
				p++;
			}

			/* Reset the 'y' ("grid-count") counter */
			y = 0;
		}

		/* If we're in a path, increase the "grid-count" */
		if (in_path) y++;
	}

	/* Stop here if we have more than one path, or less than one */
	if (p != 1) return (FALSE);

	/*** Stage Three - Optimization ***/

	/* Reiterate and gather more information */
	if (x > 1)
	{
		for (i = 0, d = 1; i < 4; i++, d += ((d == 3) ? 4 : 2))
		{
			int xy1, xy2, xm, ym;

			/* Don't bother if the current diagonal isn't okay */
			if (!okay[xy_to_abs[y_mod[d] + 1][x_mod[d] + 1]]) continue;

			/* Preparation - Initialize variables */
			xm = ((d == 1 || d == 7) ? 1 : -1);
			ym = ((d == 1 || d == 3) ? 3 : -3);

			xy1 = xy_to_abs[y_mod[d + ym] + 1][x_mod[d] + 1];
			xy2 = xy_to_abs[y_mod[d] + 1][x_mod[d + xm] + 1];

			/* Stage 3 - Simple Optimization */
			if (okay[xy1]) okay[xy1] = FALSE;
			if (okay[xy2]) okay[xy2] = FALSE;
		}
	}

	/*** Step Four - Act ***/
	for (i = 0, d = 1; i < 8; i++, d = iter_to_dir[i])
	{
		/* As soon as we hit the first okay grid, break */
		if (okay[d]) break;
	}

	/* Set run_dir */
	run_dir = d;

	/* Move */
	return (TRUE);
}

/*
 * Start a run.
 */
static bool run_start(int dir)
{
	/* Set the direction (for reference) */
	run_dir = dir;

	/* Move if the grid is walkable */
	if (is_walkable((p_ptr->px + x_mod[dir]), (p_ptr->py + y_mod[dir])))
	{
		return (TRUE);
	}

	/* Otherwise, don't. */
	return (FALSE);
}

/*
 * Basic Running code.
 */
void run_step(int dir)
{
	bool act;

	/* Start Run */
	if (dir)
	{
		/* Set the counter */
		p_ptr->running = (p_ptr->command_arg ? p_ptr->command_arg : -1);

		/* Run */
		act = run_start(dir);
	}
	else
	{
		/* Run */
		act = run_continue();
	}

	/* If we should act, then do so */
	if (act)
	{
		/* Decrease running counter */
		p_ptr->running--;

		/* Take time */
		p_ptr->energy_use = 100;

		/* Move the player */
		move_player(run_dir, FALSE);
	}

	return;
}


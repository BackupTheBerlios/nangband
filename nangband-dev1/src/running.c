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

static const sbyte x_mod[10] = {0, -1, +0, +1, -1, +0, +1, -1, +0, +1};
static const sbyte y_mod[10] = {0, +1, +1, +1, +0, +0, +0, -1, -1, -1};

static const byte cycle_index[8] = {1, 2, 3, 6, 9, 8, 7, 4};
static const byte iter_to_dir[8] = {1, 2, 3, 4, 6, 7, 8, 9};

static const byte xy_to_abs[3][3] =
{{5, 6, 7},
 {3, 9, 4},
 {0, 1, 2}};

static const byte xy_to_humanreadable[3][3] =
{{7, 8, 9},
 {4, 5, 6},
 {1, 2, 3}};

static bool okay[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

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

int count_dpaths(void)
{
	int i = 0, d = 0, p = 0, x = 0, y = 0;
	bool in_path = FALSE;
	bool old_in_path, new_in_path = FALSE;

	/* Cycle through the okay grids */
	for (; i < 8; i++, d = xy_to_abs[y_mod[cycle_index[i]] + 1][x_mod[cycle_index[i]] + 1])
	{
		old_in_path = new_in_path;
		new_in_path = FALSE;

		debug_out("iteration %d; direction is %d, human-readable direction is %d",
			i, d, xy_to_humanreadable[y_mod[cycle_index[i]] + 1][x_mod[cycle_index[i]] + 1]);
		debug_down;
		debug_out("in_path = %d", in_path);
		debug_out("okay[d] = %d", okay[d]);
		debug_out("y (grid-count) = %d", y);
		debug_out("x (largest number of grids together found) = %d", x);

		/* If the current grid is okay, then we are in a path */
		if (okay[d]) new_in_path = TRUE;

		/* If a change has been made ... */
		if (new_in_path != old_in_path)
		{
			/* Increase the count if we have a distinct path */
			p++;

			/* Reset the grid counter ('y') */
			y = 0;
		}
		else if (new_in_path)
		{
			/* Increase the grid count if we're in a path */
			y++;

			/* Debug */
			debug_out("increasing 'y' (grid count) to %d", y);
		}

		debug_up;
	}

	return (p);
}

/*
 * Main function: continue running.
 */
static bool run_continue(void)
{
	int i, d, p, x, y;

	debug_out("run_continue(void) called.");

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
	if (!p)
	{
		debug_out("*** Step One - no walkable paths found.  Returning 0.");
		return (FALSE);
	}

	/*** Stage Two - Count "Distinct Paths" ***/

	/* Branch to a seperate function */
	p = count_dpaths();

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
			else if (okay[xy2]) okay[xy2] = FALSE;
		}
	}

	/*** Step Four - Act ***/
	for (i = 0, d = 1; i < 8; i++, d = iter_to_dir[i])
	{
		/* As soon as we hit the first okay grid, break */
		if (okay[i]) break;
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

	/* Set up the debugging stuff*/
	debug_out_hook = debug_out_to_stderr;
	debug_resetlevel();

	debug_out("run_step(%d) called.", dir);

	/* Start Run */
	if (dir)
	{
		/* Set the counter */
		p_ptr->running = (p_ptr->command_arg ? p_ptr->command_arg : 100);

		debug_down; debug_out("starting run:");

		debug_down;
		debug_out("p_ptr->running set to %d", p_ptr->running);
		debug_out("p_ptr->command_arg is %d", p_ptr->command_arg);
		debug_out("calling run_start(%d).", dir);

		/* Run */
		act = run_start(dir);

		debug_out("run_start(%d) returned %d.", dir, act); debug_up; debug_up;
	}
	else
	{
		/* Run */
		debug_down; debug_out("continuing run:"); debug_down;

		debug_down; act = run_continue(); debug_up;

		debug_out("run_continue() returned %d.", act); debug_up; debug_up;
	}

	/* If we should act, then do so */
	if (act)
	{
		debug_down;

		/* Decrease running counter */
		debug_out("acting:");
		debug_down;

		p_ptr->running--;
		debug_out("decreased p_ptr->running (%d)", p_ptr->running);

		/* Take time */
		p_ptr->energy_use = 100;
		debug_out("energy use set to 100");

		/* Move the player */
		move_player(run_dir, FALSE);
		debug_out("moved player in direction run_dir (%d)", run_dir);

		debug_up;
		debug_up;
	}
	else
	{
		debug_down; debug_out("not acting:"); debug_down;
		
		p_ptr->running = 0;

		debug_out("reset p_ptr->running (%d)", p_ptr->running); debug_up; debug_up;
	}

	debug_out("\n");

	return;
}


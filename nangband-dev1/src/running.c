/*
 * File: running.c
 *
 * Abstract: Running code for Nangband,
 *
 * Author: Andrew Sidwell (takkaria)
 *
 * Licence: The GNU GPL, version 2 or any later version.
 */
#include "angband.h"

static const int x_mod[9] =
{0, -1, +0, +1, -1, +0, +1, -1, +0, +1};

static const int y_mod[9] =
{0, +1, +1, +1, +0, +0, +0, -1, -1, -1};

int run_dir;

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
 * Work out which grids are walkable and which are not.
 */
static int run_set_grids(void)
{
	int x = 0, y = 0, n, l;

	for (n = 1, l = 0; n < 10; n++)
	{
		x = p_ptr->px;
		y = p_ptr->py;

		x += x_mod[n];
		y += y_mod[n];

		if (!is_walkable(x, y))
		{
			place_okay[n] = FALSE;
		}
		else
		{
			place_okay[n] = TRUE;
            l += 1;
		}
	}

	/* Return the number of walkable paths */
	return (l);
}

/*
 * Main function: continue running.
 */
static void run_continue(void)
{
	bool okay[10];
	int i, d, x, y, p;

	/*** Step One - Build Flow Information and Map ***/

	/* Get the basic okayness. */
	for (i = 0, d = 1, p = 0; i < 8; i++, d += ((i == 4) ? 2 : 1))
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

	/* Reiterate and gather more information */
	for (i = 0, d = 1; i < 4; i++, d += ((d == 3) ? 4 : 2))
	{
		int x1, y1, x2, y2, xm, ym;

		/* (x, y) is the point of the current diagonal */
		x = (p_ptr->px + x_mod[d]);
		y = (p_ptr->py + y_mod[d]);

#define vert(z)  ((z == 1 || z == 3) ? 3 : -3)
#define horiz(z) ((z == 1 || z == 7) ? 1 : -1)

		/*** Do (simple) optimization, Stage 1a. ***/
		xm = vert(x);
		x1 = (p_ptr->px + x_mod[x + xm]);
		y1 = (p_ptr->py + y_mod[y2];

		ym = horiz(y);
		x2 = (p_ptr->px + x_mod[x2];
		y2 = (p_ptr->py + y_mod[y + ym]);

#undef vert
#undef horiz

		/*** Do (simple) optimization, Stage 1b ***/
		if (okay[] && okay[x + xm] && okay[y + ym])

		/*** Do (simple) optimization, Stage 1c ***/
	}

	/*** Step Two - Evaluate Condition ***/

}

/*
 * Start a run.
 */
static void run_start(int dir)
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
		p_ptr->running = (p_ptr->command_arg ? p_ptr->command_arg : -1);
	}

	/* Whatever */
	act = (dir ? run_start(dir) : run_continue());

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


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

/* These should all be moved into running.h */
typedef struct {
	int base;
	int left;
	int right;
} keypad_similar;

keypad_similar run_lr[] =
{
	{1, 4, 2},
	{2, 1, 3},
	{3, 2, 6},
	{6, 3, 9},
	{9, 6, 8},
	{8, 9, 7},
	{7, 8, 4},
	{4, 7, 1}
};

keypad_similar run_diag[] =
{
	{4, 2, 8},
	{8, 6, 4},
	{6, 2, 4},
	{2, 4, 6}
};

bool place_okay[10] =
{ FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE };

static const int x_mod[] =
{0, -1, +0, +1, -1, +0, +1, -1, +0, +1};

static const int y_mod[] =
{0, +1, +1, +1, +0, +0, +0, -1, -1, -1};

int run_dir, no_paths;
bool run_energy = TRUE;

/*
 * Running helper function: is_walkable(x, y).
 * This function decides whether position (x,y) is 'walkable' or not.
 *
 * Returns TRUE if it is, FALSE if not.
 */
static bool is_walkable(int x, int y)
{
	/* Non-walls are not walls */
	if (cave_feat[y][x] < FEAT_SECRET) return (TRUE);

	/* Hack: the town has problems */
/*	if (p_ptr->depth)
	{ */
		/* Unknown grids are not known walls */
		if (!(cave_info[y][x] & (CAVE_MARK))) return (TRUE);
/* 	} */

	/* The curent grid is not 'walkable' */
	if ((x == p_ptr->py) && (y == p_ptr->py)) return (FALSE);

	return(FALSE);
}

/*
 * Looks up a base number in run_lr[].
 */
static keypad_similar *run_lookup_lr(int base_direction)
{
	int n;
	keypad_similar *current_record;

	for (n = 0; n < 9; n++)
	{
		current_record = &run_lr[n];

		if (current_record->base == base_direction)
		{
			return(current_record);
		}
	}

	return (NULL);
}

/*
 * Looks up a base number in run_diag[].
 */
static keypad_similar *run_lookup_diag(int base_direction)
{
	int n;
	keypad_similar *current_record;

	for (n = 0; n < 4; n++)
	{
		current_record = &run_diag[n];

		if (current_record->base == base_direction)
		{
			return(current_record);
		}
	}

	return (NULL);
}


/*
 * Work out which grids are walkable and which are not.
 */
static int run_set_grids(void)
{
	int x = 0,
	    y = 0,
	    n = 0,
	    l = -1;

	for (n = 1; n < 10; n++)
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

    /* Hacks */
    if (run_dir == 1) place_okay[9] = FALSE;
    else if (run_dir == 2) place_okay[8] = FALSE;
    else if (run_dir == 3) place_okay[7] = FALSE;
    else if (run_dir == 4) place_okay[6] = FALSE;
    else if (run_dir == 6) place_okay[4] = FALSE;
    else if (run_dir == 7) place_okay[3] = FALSE;
    else if (run_dir == 8) place_okay[2] = FALSE;
    else if (run_dir == 9) place_okay[1] = FALSE;

	return(l);
}

/*
 * Main function: continue running.
 */
static void run_continue(void)
{
	int n;

	run_energy = TRUE;

	n = run_set_grids();

	if (((n - no_paths) > 2) || ((n - no_paths) < -2))
	{
		disturb(0, 0);
		run_energy = FALSE;
		return;
	}

	if (place_okay[run_dir] == FALSE)
	{
 		if (n == 1 || n == 2)
 		{
 			if (place_okay[(run_lookup_lr(run_dir))->left])
 			{
 				run_dir = (run_lookup_lr(run_dir))->left;
 			}
 			else if (place_okay[(run_lookup_lr(run_dir))->right])
 			{
 				run_dir = (run_lookup_lr(run_dir))->right;
 			}
  			if (place_okay[(run_lookup_diag(run_dir))->left])
 			{
 				run_dir = (run_lookup_diag(run_dir))->left;
 			}
 			else if (place_okay[(run_lookup_diag(run_dir))->right])
 			{
 				run_dir = (run_lookup_diag(run_dir))->right;
 			}
			else
			{
				disturb(0, 0);
				run_energy = FALSE;
				return;
			}
 		}
 		else
	    {
			disturb(0, 0);
			run_energy = FALSE;
 			return;
 		}
	}

	no_paths = n;

	return;
}

/*
 * Start a run.
 */
static void run_start(int dir)
{
	run_dir = dir;

	no_paths = run_set_grids();

	run_continue();

	return;
}

/*
 * Basic Running code.
 */
void run_step(int dir)
{
	/* Start Run */
	if (dir)
	{
		p_ptr->running = (p_ptr->command_arg ? p_ptr->command_arg : -1);

		/* Start Running */
		run_start(dir);
	}
	else
	{
		/* Or Continue */
		run_continue();
	}

	if (run_energy)
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

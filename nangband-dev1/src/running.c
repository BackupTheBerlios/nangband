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
#include <stdarg.h>

static byte run_dir;

static const sbyte x_mod[10] = {0, -1, +0, +1, -1, +0, +1, -1, +0, +1};
static const sbyte y_mod[10] = {0, +1, +1, +1, +0, +0, +0, -1, -1, -1};

static const byte cycle_index[8] = {0, 1, 2, 4, 5, 6, 7, 3};
static const byte iter_to_dir[8] = {1, 2, 3, 4, 6, 7, 8, 9};

static const byte xy_to_abs[3][3] =
{{5, 6, 7},
 {3, 8, 4},
 {0, 1, 2}};

/* Flexible debugging system */
void (*debug_out_hook)(const char *) = NULL;
int debug_level = 0;

void debug_out_to_stderr(const char *out)
{
	/* Output to stderr */
	fprintf(stderr, out);

	/* Return */
	return;
}

#define debug_resetlevel() debug_level = 0
#define debug_downlevel()  ++debug_level
#define debug_uplevel() --debug_level

void debug_out(const char *format, ...)
{
	va_list vp;
	char buffer[1024] = "";
	char *p = buffer;

	/* If the hook hasn't been set, don't output */
	if (!debug_out_hook) return;

	/* Prepare the buffer */
	if (debug_level)
	{
		int i;
		for (i = 0; i < debug_level; i++) strcat(buffer, "  ");
		strcat(buffer, "- ");
		i = strlen(buffer);
		p += i;
	}

	/* Now, just make the string */
	va_start(vp, format);
	vsprintf(p, format, vp);
	va_end(vp);

	strcat(buffer, "\n");

	/* Output to the debug hook */
	debug_out_hook(buffer);

	/* We are done */
	return;
}

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

	debug_out("run_start(%d) called.", dir);

	/* Move if the grid is walkable */
	if (is_walkable((p_ptr->px + x_mod[dir]), (p_ptr->py + y_mod[dir])))
	{
		debug_downlevel();
		debug_out("returning TRUE, dir (%d) is walkable.", dir);
		debug_uplevel();
		return (TRUE);
	}

	/* Otherwise, don't. */
	debug_downlevel();
	debug_out("returning FALSE, dir (%d) is not walkable.", dir);
	debug_uplevel();
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

		debug_downlevel();
		debug_out("starting run:");

		debug_downlevel();
		debug_out("p_ptr->running set to %d", p_ptr->running);
		debug_out("p_ptr->command_arg is %d", p_ptr->command_arg);
		debug_out("calling run_start(%d).", dir);

		/* Run */
		debug_downlevel();
		act = run_start(dir);
		debug_uplevel();

		debug_out("run_start(%d) returned %d.", dir, act);
		debug_uplevel();
		debug_uplevel();
	}
	else
	{
		/* Run */
		debug_downlevel();
		debug_out("continuing run:");
		debug_downlevel();

		debug_downlevel();
		act = run_continue();
		debug_uplevel();

		debug_out("run_continue() returned %d.", act);
		debug_uplevel();
		debug_uplevel();
	}

	/* If we should act, then do so */
	if (act)
	{
		debug_downlevel();

		/* Decrease running counter */
		debug_out("acting:");
		debug_downlevel();

		p_ptr->running--;
		debug_out("decreased p_ptr->running (%d)", p_ptr->running);

		/* Take time */
		p_ptr->energy_use = 100;
		debug_out("energy use set to 100");

		/* Move the player */
		move_player(run_dir, FALSE);
		debug_out("moved player in direction run_dir (%d)", run_dir);

		debug_uplevel();
		debug_uplevel();
	}
	else
	{
		debug_downlevel();

		debug_out("not acting:");
		debug_downlevel();
		
		p_ptr->running = 0;
		debug_out("reset p_ptr->running (%d)", p_ptr->running);

		debug_uplevel();
		debug_uplevel();
	}

	debug_out("\n");

	return;
}


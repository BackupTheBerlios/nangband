/*
 * running.c - Running code.
 * (c) Andrew Sidwell 2001.
 *
 * The source to this file must be publicly available if there is a
 * compiled binary containing any sections of this file.
 */
#include "angband.h"

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

	return (current_record);
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

	return (current_record);
}


/*
 * Debugging.
 */
static void run_log(cptr string)
{
	message_add(string, MSG_GENERIC);

	return;
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

#if 0
/*
 * Hack -- Check for a "known wall" (see below)
 */
static int see_wall(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are not known walls XXX XXX XXX */
	if (!in_bounds(y, x)) return (FALSE);

	/* Non-wall grids are not known walls */
	if (cave_feat[y][x] < FEAT_SECRET) return (FALSE);

	/* Unknown walls are not known walls */
	if (!(cave_info[y][x] & (CAVE_MARK))) return (FALSE);

	/* Default */
	return (TRUE);
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are unknown XXX XXX XXX */
	if (!in_bounds(y, x)) return (TRUE);

	/* Memorized grids are always known */
	if (cave_info[y][x] & (CAVE_MARK)) return (FALSE);

	/* Default */
	return (TRUE);
}

/*
 * The running algorithm  -CJS-
 *
 * Basically, once you start running, you keep moving until something
 * interesting happens.  In an enclosed space, you run straight, but
 * you follow corners as needed (i.e. hallways).  In an open space,
 * you run straight, but you stop before entering an enclosed space
 * (i.e. a room with a doorway).  In a semi-open space (with walls on
 * one side only), you run straight, but you stop before entering an
 * enclosed space or an open space (i.e. running along side a wall).
 *
 * All discussions below refer to what the player can see, that is,
 * an unknown wall is just like a normal floor.  This means that we
 * must be careful when dealing with "illegal" grids.
 *
 * No assumptions are made about the layout of the dungeon, so this
 * algorithm works in hallways, rooms, town, destroyed areas, etc.
 *
 * In the diagrams below, the player has just arrived in the grid
 * marked as '@', and he has just come from a grid marked as 'o',
 * and he is about to enter the grid marked as 'x'.
 *
 * Running while confused is not allowed, and so running into a wall
 * is only possible when the wall is not seen by the player.  This
 * will take a turn and stop the running.
 *
 * Several conditions are tracked by the running variables.
 *
 *   p_ptr->run_open_area (in the open on at least one side)
 *   p_ptr->run_break_left (wall on the left, stop if it opens)
 *   p_ptr->run_break_right (wall on the right, stop if it opens)
 *
 * When running begins, these conditions are initialized by examining
 * the grids adjacent to the requested destination grid (marked 'x'),
 * two on each side (marked 'L' and 'R').  If either one of the two
 * grids on a given side is a wall, then that side is considered to
 * be "closed".  Both sides enclosed yields a hallway.
 *
 *    LL                     @L
 *    @x      (normal)       RxL   (diagonal)
 *    RR      (east)          R    (south-east)
 *
 * In the diagram below, in which the player is running east along a
 * hallway, he will stop as indicated before attempting to enter the
 * intersection (marked 'x').  Starting a new run in any direction
 * will begin a new hallway run.
 *
 * #.#
 * ##.##
 * o@x..
 * ##.##
 * #.#
 *
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * In the diagram below, the player is running east down a hallway,
 * and will stop in the grid (marked '1') before the intersection.
 * Continuing the run to the south-east would result in a long run
 * stopping at the end of the hallway (marked '2').
 *
 * ##################
 * o@x       1
 * ########### ######
 * #2          #
 * #############
 *
 * After each step, the surroundings are examined to determine if
 * the running should stop, and to determine if the running should
 * change direction.  We examine the new current player location
 * (at which the runner has just arrived) and the direction from
 * which the runner is considered to have come.
 *
 * Moving one grid in some direction places you adjacent to three
 * or five new grids (for straight and diagonal moves respectively)
 * to which you were not previously adjacent (marked as '!').
 *
 *   ...!              ...
 *   .o@!  (normal)    .o.!  (diagonal)
 *   ...!  (east)      ..@!  (south east)
 *                      !!!
 *
 * If any of the newly adjacent grids are "interesting" (monsters,
 * objects, some terrain features) then running stops.
 *
 * If any of the newly adjacent grids seem to be open, and you are
 * looking for a break on that side, then running stops.
 *
 * If any of the newly adjacent grids do not seem to be open, and
 * you are in an open area, and the non-open side was previously
 * entirely open, then running stops.
 *
 * If you are in a hallway, then the algorithm must determine if
 * the running should continue, turn, or stop.  If only one of the
 * newly adjacent grids appears to be open, then running continues
 * in that direction, turning if necessary.  If there are more than
 * two possible choices, then running stops.  If there are exactly
 * two possible choices, separated by a grid which does not seem
 * to be open, then running stops.  Otherwise, as shown below, the
 * player has probably reached a "corner".
 *
 *    ###             o##
 *    o@x  (normal)   #@!   (diagonal)
 *    ##!  (east)     ##x   (south east)
 *
 * In this situation, there will be two newly adjacent open grids,
 * one touching the player on a diagonal, and one directly adjacent.
 * We must consider the two "option" grids further out (marked '?').
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid.  For some unknown reason, we assign "check_dir" to
 * the grid marked 's', which may be incorrectly labelled.
 *
 *    ###s
 *    o@x?   (may be incorrect diagram!)
 *    ##!?
 *
 * If both "option" grids are closed, then there is no reason to enter
 * the corner, and so we can cut the corner, by moving into the other
 * grid (diagonally).  If we choose not to cut the corner, then we may
 * go straight, but we pretend that we got there by moving diagonally.
 * Below, we avoid the obvious grid (marked 'x') and cut the corner
 * instead (marked 'n').
 *
 *    ###:               o##
 *    o@x#   (normal)    #@n    (maybe?)
 *    ##n#   (east)      ##x#
 *                       ####
 *
 * If one of the "option" grids is open, then we may have a choice, so
 * we check to see whether it is a potential corner or an intersection
 * (or room entrance).  If the grid two spaces straight ahead, and the
 * space marked with 's' are both open, then it is a potential corner
 * and we enter it if requested.  Otherwise, we stop, because it is
 * not a corner, and is instead an intersection or a room entrance.
 *
 *    ###
 *    o@x
 *    ##!#
 *
 * I do not think this documentation is correct.
 */




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static const byte cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static const byte chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. (?)
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                  p
 */
static void run_init(int dir)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, row, col;

	bool deepleft, deepright;
	bool shortleft, shortright;


	/* Save the direction */
	p_ptr->run_cur_dir = dir;

	/* Assume running straight */
	p_ptr->run_old_dir = dir;

	/* Assume looking for open area */
	p_ptr->run_open_area = TRUE;

	/* Assume not looking for breaks */
	p_ptr->run_break_right = FALSE;
	p_ptr->run_break_left = FALSE;

	/* Assume no nearby walls */
	deepleft = deepright = FALSE;
	shortright = shortleft = FALSE;

	/* Find the destination grid */
	row = py + ddy[dir];
	col = px + ddx[dir];

	/* Extract cycle index */
	i = chome[dir];

	/* Check for nearby wall */
	if (see_wall(cycle[i+1], py, px))
	{
		p_ptr->run_break_left = TRUE;
		shortleft = TRUE;
	}

	/* Check for distant wall */
	else if (see_wall(cycle[i+1], row, col))
	{
		p_ptr->run_break_left = TRUE;
		deepleft = TRUE;
	}

	/* Check for nearby wall */
	if (see_wall(cycle[i-1], py, px))
	{
		p_ptr->run_break_right = TRUE;
		shortright = TRUE;
	}

	/* Check for distant wall */
	else if (see_wall(cycle[i-1], row, col))
	{
		p_ptr->run_break_right = TRUE;
		deepright = TRUE;
	}

	/* Looking for a break */
	if (p_ptr->run_break_left && p_ptr->run_break_right)
	{
		/* Not looking for open area */
		p_ptr->run_open_area = FALSE;

		/* Hack -- allow angled corridor entry */
		if (dir & 0x01)
		{
			if (deepleft && !deepright)
			{
				p_ptr->run_old_dir = cycle[i - 1];
			}
			else if (deepright && !deepleft)
			{
				p_ptr->run_old_dir = cycle[i + 1];
			}
		}

		/* Hack -- allow blunt corridor entry */
		else if (see_wall(cycle[i], row, col))
		{
			if (shortleft && !shortright)
			{
				p_ptr->run_old_dir = cycle[i - 2];
			}
			else if (shortright && !shortleft)
			{
				p_ptr->run_old_dir = cycle[i + 2];
			}
		}
	}
}


/*
 * Update the current "run" path
 *
 * Return TRUE if the running should be stopped
 */
static bool run_test(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int prev_dir;
	int new_dir;
	int check_dir = 0;

	int row, col;
	int i, max, inv;
	int option, option2;


	/* No options yet */
	option = 0;
	option2 = 0;

	/* Where we came from */
	prev_dir = p_ptr->run_old_dir;


	/* Range of newly adjacent grids */
	max = (prev_dir & 0x01) + 1;


	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++)
	{
		s16b this_o_idx, next_o_idx = 0;


		/* New direction */
		new_dir = cycle[chome[prev_dir] + i];

		/* New location */
		row = py + ddy[new_dir];
		col = px + ddx[new_dir];


		/* Visible monsters abort running */
		if (cave_m_idx[row][col] > 0)
		{
			monster_type *m_ptr = &m_list[cave_m_idx[row][col]];

			/* Visible monster */
			if (m_ptr->ml) return (TRUE);
		}

		/* Visible objects abort running */
		for (this_o_idx = cave_o_idx[row][col]; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;

			/* Get the object */
			o_ptr = &o_list[this_o_idx];

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Visible object */
			if (o_ptr->marked) return (TRUE);
		}


		/* Assume unknown */
		inv = TRUE;

		/* Check memorized grids */
		if (cave_info[row][col] & (CAVE_MARK))
		{
			bool notice = TRUE;

			/* Examine the terrain */
			switch (cave_feat[row][col])
			{
				/* Floors */
				case FEAT_FLOOR:

				/* Invis traps */
				case FEAT_INVIS:

				/* Secret doors */
				case FEAT_SECRET:

				/* Normal veins */
				case FEAT_MAGMA:
				case FEAT_QUARTZ:

				/* Hidden treasure */
				case FEAT_MAGMA_H:
				case FEAT_QUARTZ_H:

				/* Walls */
				case FEAT_WALL_EXTRA:
				case FEAT_WALL_INNER:
				case FEAT_WALL_OUTER:
				case FEAT_WALL_SOLID:
				case FEAT_PERM_EXTRA:
				case FEAT_PERM_INNER:
				case FEAT_PERM_OUTER:
				case FEAT_PERM_SOLID:
				{
					/* Ignore */
					notice = FALSE;

					/* Done */
					break;
				}

				/* Open doors */
				case FEAT_OPEN:
				case FEAT_BROKEN:
				{
					/* Option -- ignore */
					if (run_ignore_doors) notice = FALSE;

					/* Done */
					break;
				}

				/* Stairs */
				case FEAT_LESS:
				case FEAT_MORE:
				{
					/* Option -- ignore */
					if (run_ignore_stairs) notice = FALSE;

					/* Done */
					break;
				}
			}

			/* Interesting feature */
			if (notice) return (TRUE);

			/* The grid is "visible" */
			inv = FALSE;
		}

		/* Analyze unknown grids and floors */
		if (inv || cave_floor_bold(row, col))
		{
			/* Looking for open area */
			if (p_ptr->run_open_area)
			{
				/* Nothing */
			}

			/* The first new direction. */
			else if (!option)
			{
				option = new_dir;
			}

			/* Three new directions. Stop running. */
			else if (option2)
			{
				return (TRUE);
			}

			/* Two non-adjacent new directions.  Stop running. */
			else if (option != cycle[chome[prev_dir] + i - 1])
			{
				return (TRUE);
			}

			/* Two new (adjacent) directions (case 1) */
			else if (new_dir & 0x01)
			{
				check_dir = cycle[chome[prev_dir] + i - 2];
				option2 = new_dir;
			}

			/* Two new (adjacent) directions (case 2) */
			else
			{
				check_dir = cycle[chome[prev_dir] + i + 1];
				option2 = option;
				option = new_dir;
			}
		}

		/* Obstacle, while looking for open area */
		else
		{
			if (p_ptr->run_open_area)
			{
				if (i < 0)
				{
					/* Break to the right */
					p_ptr->run_break_right = TRUE;
				}

				else if (i > 0)
				{
					/* Break to the left */
					p_ptr->run_break_left = TRUE;
				}
			}
		}
	}


	/* Looking for open area */
	if (p_ptr->run_open_area)
	{
		/* Hack -- look again */
		for (i = -max; i < 0; i++)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Unknown grid or non-wall */
			/* Was: cave_floor_bold(row, col) */
			if (!(cave_info[row][col] & (CAVE_MARK)) ||
			    (cave_feat[row][col] < FEAT_SECRET))
			{
				/* Looking to break right */
				if (p_ptr->run_break_right)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break left */
				if (p_ptr->run_break_left)
				{
					return (TRUE);
				}
			}
		}

		/* Hack -- look again */
		for (i = max; i > 0; i--)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Unknown grid or non-wall */
			/* Was: cave_floor_bold(row, col) */
			if (!(cave_info[row][col] & (CAVE_MARK)) ||
			    (cave_feat[row][col] < FEAT_SECRET))
			{
				/* Looking to break left */
				if (p_ptr->run_break_left)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break right */
				if (p_ptr->run_break_right)
				{
					return (TRUE);
				}
			}
		}
	}


	/* Not looking for open area */
	else
	{
		/* No options */
		if (!option)
		{
			return (TRUE);
		}

		/* One option */
		else if (!option2)
		{
			/* Primary option */
			p_ptr->run_cur_dir = option;

			/* No other options */
			p_ptr->run_old_dir = option;
		}

		/* Two options, examining corners */
		else if (run_use_corners && !run_cut_corners)
		{
			/* Primary option */
			p_ptr->run_cur_dir = option;

			/* Hack -- allow curving */
			p_ptr->run_old_dir = option2;
		}

		/* Two options, pick one */
		else
		{
			/* Get next location */
			row = py + ddy[option];
			col = px + ddx[option];

			/* Don't see that it is closed off. */
			/* This could be a potential corner or an intersection. */
			if (!see_wall(option, row, col) ||
			    !see_wall(check_dir, row, col))
			{
				/* Can not see anything ahead and in the direction we */
				/* are turning, assume that it is a potential corner. */
				if (run_use_corners &&
				    see_nothing(option, row, col) &&
				    see_nothing(option2, row, col))
				{
					p_ptr->run_cur_dir = option;
					p_ptr->run_old_dir = option2;
				}

				/* STOP: we are next to an intersection or a room */
				else
				{
					return (TRUE);
				}
			}

			/* This corner is seen to be enclosed; we cut the corner. */
			else if (run_cut_corners)
			{
				p_ptr->run_cur_dir = option2;
				p_ptr->run_old_dir = option2;
			}

			/* This corner is seen to be enclosed, and we */
			/* deliberately go the long way. */
			else
			{
				p_ptr->run_cur_dir = option;
				p_ptr->run_old_dir = option2;
			}
		}
	}


	/* About to hit a known wall, stop */
	if (see_wall(p_ptr->run_cur_dir, py, px))
	{
		return (TRUE);
	}


	/* Failure */
	return (FALSE);
}



/*
 * Take one step along the current "run" path
 *
 * Called with a real direction to begin a new run, and with zero
 * to continue a run in progress.
 */
void run_step(int dir)
{
	/* Start run */
	if (dir)
	{
		/* Initialize */
		run_init(dir);

		/* Hack -- Set the run counter */
		p_ptr->running = (p_ptr->command_arg ? p_ptr->command_arg : 1000);

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);
	}

	/* Continue run */
	else
	{
		/* Update run */
		if (run_test())
		{
			/* Disturb */
			disturb(0, 0);

			/* Done */
			return;
		}
	}

	/* Decrease counter */
	p_ptr->running--;

	/* Take time */
	p_ptr->energy_use = 100;

	/* Move the player */
	move_player(p_ptr->run_cur_dir, FALSE);
}

#endif

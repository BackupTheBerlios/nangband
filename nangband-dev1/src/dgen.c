/*
 * File: dgen.c
 * Purpose: The dungeon generation code.
 * Author: Andrew Sidwell (takkaria)
 *
 * This file is dual-licenced under both the Angband and GPL licences.
 * It is currently distribued under the Angband licence.
 */

#include "angband.h"

static void generate_dungeon(map_grid_t *dungeon, int width, int height, int level, int exeption)
{
	/* Return control */
	return;
}

#ifdef DEBUG

int main(int argc, char **argv)
{
	map_grid_t dungeon[80][80];
	
	generate_dungeon(&dungeon, 80, 80, 2, 0);
	
	return(0);
}

#endif /* DEBUG */
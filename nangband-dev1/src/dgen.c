/*
 * File: dgen.c
 * Purpose: The dungeon generation code.
 * Author: Andrew Sidwell (takkaria)
 *
 * This file is dual-licenced under both the Angband and GPL licences.
 * It is currently distribued under the Angband licence.
 */

#include "angband.h"
#include "dtype.h"

static void generate_dungeon(map_grid_t **dungeon, int width, int height, int level, int exeption)
{
	map_grid_t *this_grid;
	int x, y;
	
	/* Debugging info */
	printf("args are: width = %i, height = %i, level = %i, exeption = %i\n", width, height, level, exeption);

	/* Init the dungeon to empty spaces */
	for (x = 0; x < width; x++)
	{
		for (y = 0; y < height; y++)
		{
			printf("x = %i, y = %i", x, y);
			*dungeon->feature = TERRAIN_TYPE_WALL;
		}
	}
	
	/* Pick a random point in the dungeon */
	this_grid = dungeon[rand_int(width)][rand_int(height)];
	
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
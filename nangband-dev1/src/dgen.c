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

#define DEBUG

/*
 * Memory Management Section
 */

/* Allocate memory for a map */
map_grid_t **map_alloc(int width, int height)
{
	map_grid_t **dungeon;
	int n;
	
	/* Allocate the dungeon */
	C_MAKE(dungeon, width, map_grid_t *);
	
	for (n = 0; n < width; n++)
	{
		/* Allocate the memory */
		C_MAKE(dungeon[n], height, map_grid_t);
	}
		
	/* Return a pointer the the space */
	return (dungeon);
}

/* Free up memory for a map */
void map_free(map_grid_t **dungeon)
{
	/* Free the space up */
	rnfree((vptr) *dungeon);

	return;
}

static void generate_dungeon(map_grid_t **dungeon, int width, int height, int level, int exeption)
{
	map_grid_t *this_grid;
	int x, y;
	
	/* Debugging info */
	printf("args are: width = %i, height = %i, level = %i, exeption = %i\n", width, height, level, exeption);

	/* Fill the dungeon with wall */
	for (x = 0; x < width; x++)
	{
		for (y = 0; y < height; y++)
		{
			dungeon[x][y].feature = TERRAIN_TYPE_WALL;
		}
	}
	
	/* Pick a random point in the dungeon */
	this_grid = &dungeon[rand_int(width)][rand_int(height)];

	/* Do something */
	
	/* Return control */
	return;
}

#ifdef DEBUG

#define MAP_SIZE_X	80
#define MAP_SIZE_Y	60

int main(int argc, char **argv)
{
	map_grid_t **dungeon;
	int x, y;
	
	dungeon = map_alloc(MAP_SIZE_X, MAP_SIZE_Y);
	
	generate_dungeon(dungeon, MAP_SIZE_X, MAP_SIZE_Y, 2, 0);

	for (y = 0; y < MAP_SIZE_Y; y++)
	{
		for (x = 0; x < MAP_SIZE_X; x++)
		{
			switch(dungeon[x][y].feature)
			{
				case TERRAIN_TYPE_WALL:
				{
					printf("#");
					break;
				}
			}
		}

		printf("\n");
	}

	map_free(dungeon);
	
	return(0);
}

#endif /* DEBUG */

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

/* Define all out room generator functions */
void gen_rect_room(map_grid_t **dungeon, int x1, int x2, int y1, int y2, int power, int dlev);

/* A structure of helper functions */
static generate_types generators[MAX_GENS] =
{
	{gen_rect_room, 0, 200}
};

/* Quick, knock-together random number generator */
static int rint(int max)
{
	if (!max) return 0;
	return random() % max;
}

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
	/* [note to self - free all the dungeon up] */
	
	/* Free the space up */
	rnfree((vptr) *dungeon);

	return;
}

/*
 * Room generation functions.
 */

/* ajps */
void gen_rect_room(map_grid_t **dungeon, int x1, int x2, int y1, int y2, int power, int dlev)
{
	int nx = 0, ny = 0;
	
	printf("x1 = %i, x2 = %i, y1 = %i, y2 = %i\n", x1, x2, y1, y2);
	
	for (ny = y1; ny <= y2; ny++)
	{
		for (nx = x1; nx <= x2; nx++)
		{
			if (nx == x1 || nx == x2 || ny == y1 || ny == y2)
			{
				if (dungeon[nx][ny].feature == TERRAIN_WALL)
				{
					dungeon[nx][ny].feature = TERRAIN_WALL_O;
				}
			}
			else
			{
				dungeon[nx][ny].feature = TERRAIN_FLOOR;
			}
		}
	}
	
	return;
}

/*
 * Main function to generate a dungeon.
 */
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
			dungeon[x][y].feature = TERRAIN_WALL;
		}
	}

	{
		int sy0, sx0, sy1, sx1, rx, ry;
		
		while (1)
		{
			rx = rint(width);
			ry = rint(height);
		
			sy0 = ry + (4 - rint(8));
			sx0 = rx + (4 - rint(8));
			sy1 = sy0 + (4 + rint(5));
			sx1 = sx0 + (4 + rint(5));
		
			if (sx0 >= width || sx1 >= width || sy0 >= height || sy1 >= height)
				continue;
			else
				break;
		}
		
		gen_rect_room(dungeon, sx0, sx1, sy0, sy1, 0, 0);
	}
	
	/* Return control */
	return;
}

int main(int argc, char **argv)
{
	map_grid_t **dungeon;
	int x, y;
	
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		srandom(tv.tv_sec + tv.tv_usec);
	}
	
	dungeon = map_alloc(MAP_SIZE_X, MAP_SIZE_Y);
	
	generate_dungeon(dungeon, MAP_SIZE_X, MAP_SIZE_Y, 2, 0);

	for (y = 0; y < MAP_SIZE_Y; y++)
	{
		for (x = 0; x < MAP_SIZE_X; x++)
		{
			switch(dungeon[x][y].feature)
			{
				case TERRAIN_WALL:
				{
					printf("#");
					break;
				}
				case TERRAIN_WALL_O:
				{
					printf("%");
					break;
				}
				case TERRAIN_FLOOR:
				{
					printf(".");
					break;
				}
				case TERRAIN_DOOR:
				{
					printf("+");
					break;
				}
			}
		}

		printf("\n");
	}

	map_free(dungeon);
	
	return(0);
}

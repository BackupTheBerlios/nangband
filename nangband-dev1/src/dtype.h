/*
 * File: dtype.h
 * Purpose: Types for the dungeon generation code.
 * Author: Andrew Sidwell (takkaria)
 *
 * This file is dual-licenced under both the Angband and GPL licences.
 * It is currently distribued under the Angband licence.
 */
#ifndef INCLUDED_D_TYPE_H
#define INCLUDED_D_TYPE_H

#define DEBUG

/* Map sizes */
#define MAP_SIZE_X	80
#define MAP_SIZE_Y	40

/* A constant */
#define MAX_GENS	1

/* Defines for terrain features */
#define TERRAIN_NONE	0
#define TERRAIN_WALL	1
#define TERRAIN_WALL_O	2
#define TERRAIN_FLOOR	3
#define TERRAIN_DOOR	4

typedef unsigned char feat_t;
typedef void *generate_hook(map_grid_t **dungeon, int x1, int x2, int y1, int y2, int power, int dlev);

typedef struct {
	feat_t	feature;
	bool	monster;
	bool	object;	
} map_grid_t;

typedef struct {
	generate_hook function;
	int min_power;
	int max_power;
} generate_types;

#endif

/*
 * File: dtype.h
 * Purpose: Types for the dungeon generation code.
 * Author: Andrew Sidwell (takkaria)
 *
 * This file is dual-licenced under both the Angband and GPL licences.
 * It is currently distribued under the Angband licence.
 */
#ifndef INCLUDED_DTYPE_H
#define INCLUDED_D_TYPE_H

/* Defines for terrain features */
#define TERRAIN_TYPE_NONE	0
#define TERRAIN_TYPE_WALL	1
#define TERRAIN_TYPE_WALL_O	2
#define TERRAIN_TYPE_FLOOR	3

typedef unsigned short int feat_t;

typedef struct {
	feat_t	feature;
	bool	monster;
	bool	object;	
} map_grid_t;

#endif

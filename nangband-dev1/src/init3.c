/*
 * File: init3.c
 *
 * Abstract: Parsing functions which are called more than once.
 *
 * For licencing terms, please see angband.h.
 */
#include "angband.h"

/* Process "D:<dungeon>" -- info for the cave grids */
void make_town_from_file(void)
{
	char buf[1024];
	int char_to_feat[1024];
	FILE *fp;
	int x = 0, y = 0, t = 0;

	for (x = 0; x < z_info->f_max; x++)
	{
		if (f_info[x].t_char)
			char_to_feat[(int) f_info[x].t_char] = x;
	}

	FILE_TYPE(FILE_TYPE_DATA);
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, "town.txt");
	fp = my_fopen(buf, "r");
	if (!fp) quit("Can't open town.txt");

	dungeon_wid = dungeon_hgt = 0;

	while (my_fgets(fp, buf, sizeof(buf)) == 0)
	{
		t = strlen(buf);
		for (x = 0; x < t; x++)
		{
			if (buf[x] == '@') player_place(y, x);
			else cave_feat[y][x] = char_to_feat[(int) buf[x]];
		}

		if (t > dungeon_wid) dungeon_wid = t;

		y++;
	}

	dungeon_hgt = y;
	my_fclose(fp);
	return;
}

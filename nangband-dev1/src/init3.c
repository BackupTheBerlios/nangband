/*
 * File: init3.c
 *
 * Abstract: Parsing functions which are called more than once.
 *
 * For licencing terms, please see angband.h.
 */
#include "angband.h"

byte find_char(char p)
{
	int i;

	for (i = 0; i < z_info->f_max; i++)
		if (f_info[i].t_char == p) return (i);

	return (1);
}

void town_create_from_def(cptr path)
{
	char buf[1024];
	FILE *fp;
	int x = 0, y = 0, t = 0;

	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, path);

	/* Attempt to open the files */
	fp = my_fopen(buf, "r");

	/* Quit if we can't */
	if (!fp) quit_fmt("Can't open town file '%s'.", path);

	/* Reset width and height */
	dungeon_wid = 0;
	dungeon_hgt = 0;

	/* Read in the file, line by line */
	while (my_fgets(fp, buf, sizeof(buf)) == 0)
	{
		t = strlen(buf);
		for (x = 0; x < t; x++)
		{
			if (buf[x] == '@') player_place(y, x);
			else cave_feat[y][x] = find_char(buf[x]);
		}

		if (t > dungeon_wid) dungeon_wid = t;

		y++;
	}

	dungeon_hgt = y;
	my_fclose(fp);
	return;
}

/*
 * File: l-dynamic.c
 *
 * Abstract: A collection of functions to export variably
 * initialized data to lua.
 *
 * Author: Andrew Sidwell (takkaria).
 *
 * Licences: GNU GPL, version 2 or any later version
 *  also the Angband Licence, see angband.h
 */
#include "angband.h"
#include "lua/tolua.h"

extern int tolua_dynamic_open(lua_State* tolua_S);
extern void tolua_dynamic_close(lua_State* tolua_S);

/*
 * At the moment, this function only has commented out code.
 * It serves as a brief example of how to export information
 * dynamically using tolua.
 */
int tolua_dynamic_open(lua_State* tolua_S)
{
	int i;

	/* Open the lua directories */
	tolua_open(tolua_S);

	/* Export the spell names */
	for (i = 0; i < z_info->c_max; i++)
	{
		/* Buffer */
		char buffer[20];
		char class_name[14];
		unsigned int r;

		/* Grab the class */
		cp_ptr = &c_info[i];

		/* Set the class name */
		strcpy(class_name, (c_name + cp_ptr->name));

		/* Make it uppercase */
		for (r = 0; r < strlen(class_name); r++)
		{
			class_name[r] = toupper(class_name[r]);
		}

		/* Prepare the buffer */
		sprintf(buffer, "CLASS_%s", class_name);

		/* Make it a constant - state, null, name, value */
		tolua_constant(tolua_S, NULL, buffer, i);
	}

	/* We are done. */
	return (1);
}

void tolua_dynamic_close(lua_State* tolua_S)
{
	(void)tolua_S;

	/* Nothing to do. */
	return;
}

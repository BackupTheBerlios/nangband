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

/*
 * At the moment, this function only has commented out code.
 * It serves as a brief example of how to export information
 * dynamically using tolua.
 */
int tolua_dynamic_open(lua_State* tolua_S)
{
#if 0
	magic_spell_type *s_ptr = NULL;
	int n;
#endif

	tolua_open(tolua_S);

#if 0
	/* Export the spell names */
	for (n = 0; n < z_info->magic_spell_max; n++)
	{
		char buffer[46];

		s_ptr = &magic_spell_info[n];

		sprintf(buffer, "SPELL_%s", s_ptr->constant);
		tolua_constant(tolua_S, NULL, buffer, n);
	}
#endif

	return (1);
}

void tolua_dynamic_close(lua_State* tolua_S)
{
	return;
}

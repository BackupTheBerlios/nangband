/*
 * File: l-dynamic.c
 * Purpose: Exporting of dynamic variables to lua scripts.
 *
 * This file may be distributes under either the GPL or the Angband
 * licences.
 */
#include "angband.h"
#include "lua/tolua.h"

int tolua_dynamic_open(lua_State* tolua_S)
{
	magic_spell_type *s_ptr = NULL;
	int n;

	tolua_open(tolua_S);

	/* Export the spell names */
	for (n = 0; n < z_info->magic_spell_max; n++)
	{
		char buffer[46];

		s_ptr = &magic_spell_info[n];

		sprintf(buffer, "SPELL_%s", s_ptr->constant);
		tolua_constant(tolua_S, NULL, buffer, n);
	}

	return (1);
}

void tolua_dynamic_close(lua_State* tolua_S)
{
	magic_spell_type *s_ptr = NULL;
	int n;

	/* Export the spell names */
	for (n = 0; n < z_info->magic_spell_max; n++)
	{
		char buffer[46];

		s_ptr = &magic_spell_info[n];

		sprintf(buffer, "SPELL_%s", s_ptr->constant);

		lua_pushnil(tolua_S);
		lua_setglobal(tolua_S, buffer);
	}

	return;
}

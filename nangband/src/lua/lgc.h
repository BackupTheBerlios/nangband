/*
** $Id: lgc.h,v 1.1 2002/12/21 11:52:44 takkaria Exp $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef lgc_h
#define lgc_h


#include "lobject.h"


void luaC_collect (lua_State *L, int all);
void luaC_collectgarbage (lua_State *L);
void luaC_checkGC (lua_State *L);


#endif

/* tolua: error handling
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_eh.h,v 1.1 2002/12/21 11:52:44 takkaria Exp $
*/

/* This code is free software; you can redistribute it and/or modify it.
** The software provided hereunder is on an "as is" basis, and
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications.
*/



#ifndef tolua_eh_h
#define tolua_eh_h

void toluaI_eh_set 
(lua_State* L, int narg, const char* provided, const char* expected);


#endif

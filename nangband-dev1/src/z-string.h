/*
 * File: z-string.h
 *
 * Abstract: Header file for z-string.c.
 *
 * For licencing terms, please see angband.h.
 */
#ifndef INCLUDED_Z_STRING_H
#define INCLUDED_Z_STRING_H

#include "h-basic.h"

/*** String comparison functions ***/
extern int my_stricmp(cptr s1, cptr s2);

extern bool streq(cptr s1, cptr s2);
extern bool strieq(cptr s1, cptr s2);

extern bool prefix(cptr s, cptr t);
extern bool suffix(cptr s, cptr t);

/*** String management functions ***/

/* Constants */
#define MAX_SIZE_STRTABLE     1024

/* Functions */
extern u32b strtable_add(cptr text); 
extern cptr strtable_content(u32b index); 
extern u32b strtable_modify(u32b idx, cptr text); 
extern void strtable_remove(u32b idx); 
extern void strtable_copy(u32b from, u32b *to);

/* Do not use these outside of init*.c */
extern bool strtable_init(void); 
extern void strtable_cleanup(void); 

#endif

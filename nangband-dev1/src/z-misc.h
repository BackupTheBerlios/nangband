/*
 * File: z-misc.h
 *
 * Abstract: Header file for z-misc.c.
 *
 * For licencing terms, please see angband.h.
 */
#ifndef INCLUDED_Z_MISC_H
#define INCLUDED_Z_MISC_H

#include "h-basic.h"

/* The name Angband was invoked by */
extern cptr argv0;

/* Auxillary functions */
extern void (*plog_aux)(cptr);
extern void (*quit_aux)(cptr);
extern void (*core_aux)(cptr);

/* Print an error message */
extern void plog(cptr str);

/* Exit, with optional message */
extern void quit(cptr str);

/* Dump core, with optional message */
extern void core(cptr str);

#endif

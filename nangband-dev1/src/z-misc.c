/*
 * File: z-misc.c
 *
 * Abstract: Various miscellaneous functions for the Angband game.
 *
 * For licencing terms, please see angband.h.
 */
#include "z-misc.h"


/*
 * Convenient storage of the program name
 */
cptr argv0 = NULL;

/*
 * Redefinable "plog" action
 */
void (*plog_aux)(cptr) = NULL;

/*
 * Print (or log) a "warning" message (ala "perror()")
 * Note the use of the (optional) "plog_aux" hook.
 */
void plog(cptr str)
{  
   /* Use the "alternative" function if possible */
   if (plog_aux) (*plog_aux)(str);

   /* Just do a labeled fprintf to stderr */
   else (void)(fprintf(stderr, "%s: %s\n", argv0 ? argv0 : "?", str));
}



/*
 * Redefinable "quit" action
 */
void (*quit_aux)(cptr) = NULL;

/*
 * Exit (ala "exit()").  If 'str' is NULL, do "exit(0)".
 * If 'str' begins with "+" or "-", do "exit(atoi(str))".
 * Otherwise, plog() 'str' and exit with an error code of -1.
 * But always use 'quit_aux', if set, before anything else.
 */
void quit(cptr str)
{
   /* Attempt to use the aux function */
   if (quit_aux) (*quit_aux)(str);

   /* Success */
   if (!str) (void)(exit(0));

   /* Extract a "special error code" */
   if ((str[0] == '-') || (str[0] == '+')) (void)(exit(atoi(str)));

   /* Send the string to plog() */
   plog(str);

   /* Failure */
   (void)(exit(EXIT_FAILURE));
}



/*
 * Redefinable "core" action
 */
void (*core_aux)(cptr) = NULL;

/*
 * Dump a core file, after printing a warning message 
 * As with "quit()", try to use the "core_aux()" hook first.
 */
void core(cptr str)
{
   char *crash = NULL;

   /* Use the aux function */
   if (core_aux) (*core_aux)(str);

   /* Dump the warning string */
   if (str) plog(str);

   /* Attempt to Crash */
   (*crash) = (*crash);

   /* Be sure we exited */
   quit("core() failed");
}


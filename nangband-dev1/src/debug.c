/*
 * File: debug.c
 *
 * Abstract: A simple method for debugging parts of Angband.
 *
 * Author: Andrew Sidwell (takkaria)
 *
 * Licence: The GNU GPL, version 2 or any later version.
 */
#include "angband.h"
#include <stdarg.h>

/*
 * A simple debug output hook; prints directly to stderr.
 */
void debug_out_to_stderr(const char *out)
{
	/* Output to stderr */
	fprintf(stderr, out);

	/* Return */
	return;
}

void debug_out(const char *format, ...)
{
	va_list vp;
	char buffer[1024] = "";
	char *p = buffer;

	/* If the hook hasn't been set, don't output */
	if (!debug_out_hook) return;

	/* Prepare the buffer */
	if (debug_level)
	{
		int i;
		for (i = 0; i < debug_level; i++) strcat(buffer, "  ");
		strcat(buffer, "- ");
		i = strlen(buffer);
		p += i;
	}

	/* Now, just make the string */
	va_start(vp, format);
	vsprintf(p, format, vp);
	va_end(vp);

	strcat(buffer, "\n");

	/* Output to the debug hook */
	debug_out_hook(buffer);

	/* We are done */
	return;
}

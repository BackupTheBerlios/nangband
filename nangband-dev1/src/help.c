/* ------------------------------------------------------------ ajps ---
 * This file contains routines for loading and moving through the online
 * xml-based help system.  Honest it does.
 *
 * (c) Antony Sidwell, 2002
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * This source file is also available under the GNU GENERAL PUBLIC LICENCE.
 * --------------------------------------------------------------------- */
#include "angband.h"
#include "xmlbulp.h"
#include "help.h"

/* Compile-time options */

/*
 * LINKS_ON_PAGE stops you from selecting a link outside of the page
 * being viewed, so as you scroll up or down the page, the selection
 * moves with you.
 */
#define LINKS_ON_PAGE

/*
 * LINKS_EXTRA allows the use of a "title" attribute in links, to display
 * alternative help text at the bottom of the screen for different links.
 * eg. [Press return to read 'Help about help'].
 *
 * Uses the static string help_format to control the format it is written in.
 */
#define LINKS_EXTRA

#ifdef LINKS_EXTRA
static char help_format[]="[Press return for '%s']";
#endif

/*
 * HELP_TIMING is pointless.  It times how long it takes to load the file and
 * parse it into redraw structures, and prints the time (in clock() ticks) at
 * the top right of the screen.
 *
 * I used it during early coding to tell me where I could speed up the code.
 */
/* #define HELP_TIMING */

/*
 * HELP_DEBUG is used to #define out the debug information.  Useless to
 * other people, I suspect, as it is for the RISC OS port.
 */
/* #define HELP_DEBUG */
#ifdef HELP_DEBUG
#define HELP_DEBUG_FILEDUMP
#else
/* #define HELP_DEBUG_FILEDUMP */
#endif

/*
 * BULP_TYPE "m" reads into memory, and parses from there,
 * BULP_TYPE "f" reads & parses directly from the files.
 */
#define BULP_TYPE "m"

/* Help line text strings */

/* This is displayed when nothing else overrides it */
static char help_help[80] =
"[Press up, down, left, right, page up, page down, Return, or '?' to exit]";

/* This is displayed when you escape to link mode */
static char link_help[80] = "[Press the key for the link you require]";

/* Quick and dirty prototype */
char *help_path_build(char *buffer, int buff_size, const char *file, const char *lastfile, history_blk *stack);

/* ------------------------------------------------------------ ajps ---
 * Draws a horizontal rule, well a line of dashes really.
 * --------------------------------------------------------------------- */
static void horiz_rule(int x, int y, int width, char attr)
{
	/* Counter */
	int n;

	/* Plots characters from (x, y) to (x + width, y) */
	for (n = x; n < (x + width); n++)
	{
		Term_putch(n, y, attr, HRULE_CHARACTER);
	}
}

/* ------------------------------------------------------------ ajps ---
 * Write a block header to the display block for a plain text block,
 * (rather than a table or list) given a block_style block.
 * --------------------------------------------------------------------- */
static void write_textblock_header(char **display, block_style *header)
{
	/* Simply put the information into the appropriate places */
	(*display)[0] = REDRAW_CODE_BLOCKSTART;
	(*display)[1] = BLOCKTYPE_TEXT;
	(*display)[2] = header->width;
	(*display)[3] = header->align;
	(*display)[4] = header->lmargin;
	(*display)[5] = header->rmargin;

	/* Move the write position past the header */
	(*display) += 6;
}

/* ------------------------------------------------------------ ajps ---
 * Draws a vertical rule, well a line of dashes really.
 * --------------------------------------------------------------------- */
static void vert_rule(int x, int y, int height, char attr)
{
	/* Counter */
	u16b n;

	/* Plots characters from (x, y) to (x, y + height) */
	for (n = y; n < (y+height); n++)
	{
		Term_putch(x, n, attr, VRULE_CHARACTER);
	}
}

/* ------------------------------------------------------------ ajps ---
 * Returns the length of the word starting at cptr.
 *
 * White space before the word is counted, and the count stops at
 * the first white space in the string, or at a zero-terminator.
 *
 * Control characters are considered non-printable, and so are counted
 * separately but they will NOT terminate the string unless they are the
 * end-of-block code or the start-of-block code.
 * --------------------------------------------------------------------- */
static u32b get_word_length(char **cptr, u32b *control_chars)
{
	u32b length=0;

	/* include whitespace before the text */
	while (((*cptr)[0] == ' ') || ((*cptr)[0] == '\t'))
	{
		(*cptr)++;
		length++;
	}

	/* while the current character isn't whitespace */
	while (!xmlbulp_is_whitespace((*cptr)[0]))
	{
		/* check the current character */
		switch ((*cptr)[0])
		{
			case REDRAW_CODE_BLOCKSTART:
			case REDRAW_CODE_BLOCKEND:
			{
				/* The word has finished */
				return (length);

				break;
			}

			case REDRAW_CODE_LINKPOINT:
			case REDRAW_CODE_LINKREF:
			case REDRAW_CODE_COLOUR:
			{
				/*
				 * These are all control codes which need a character
				 * afterwards as a parameter, so we skip the next
				 * two characters.
				 */
				(*cptr) += 2;

				/* Increase the control character counter by two */
				(*control_chars) += 2;

				break;
			}

			case REDRAW_CODE_MEGAUNDERLINE:
			case REDRAW_CODE_UNDERLINE:
			case REDRAW_CODE_EMPH:
			case REDRAW_CODE_STRONG:
			{
				/* These control codes are single character, skip one char */
				(*cptr)++;

				/* Increment the counter by one */
				(*control_chars)++;

				break;
			}

			case REDRAW_CODE_HRULE:
			{
				/* This control code is 3 charactera, skip them */
				(*cptr) += 3;

				/* Increment the counter */
				(*control_chars) += 3;

				break;
			}


			default:
			{
				/* These are normal (printable) ASCII characters */
				if ((*cptr)[0] >= 32) length++;

				/* Move onto the next character */
				(*cptr)++;
			}
		}
	}

	/* Return the measured word length */
	return (length);
}

/* ------------------------------------------------------------ ajps ---
 * Returns a pointer to the information about a link, when passed a
 * reference code.  If the reference code is out-of-bounds, and last
 * isn't NULL, we fill it with the last valid reference code, still
 * returning NULL.
 * --------------------------------------------------------------------- */
static link_blk *link_info(char *links, int ref, int *last)
{
	/* Set ptr to the first link in the block */
	link_blk *ptr = (link_blk *)links;

	/* Init counter */
	int n = 0;

	/* LINK_TYPE_END marks the end of the links block */
	while (ptr->type != LINK_TYPE_END)
	{
		/* Don't want to know about link points, only references */
		if (ptr->type >= LINK_TYPE_REF)
		{
			/* Increase the counter */
			n++;

			/* If this is what we are looking for */
			if (n == ref)
			{
				return (ptr);
			}
		}

		/* Move onto the next link in the block */
		ptr = ptr->next;
	}

	/* If the last link has been requested, return the ref. number */
	if (last != NULL)
	{
		*last = n;
	}

	/* If we reach this point, we've gone past the last link */
	return (NULL);
}

/* ------------------------------------------------------------ ajps ---
 * Returns a block number given the id of a link point.
 * --------------------------------------------------------------------- */
static int link_point_block(char *links, char *id)
{
	/* Set ptr to the first link in the block */
	link_blk *ptr = (link_blk *)links;

	/* Init counter */
	u16b n = 0;

	/* This is used to mark the end of the block */
	while (ptr->type != LINK_TYPE_END)
	{
		/* We are only interested in points */
		if (ptr->type == LINK_TYPE_POINT)
		{
			/* Increment the counter */
			n++;

			/* if the id requested matches the one stored */
			if (my_stricmp(id, LINK_TEXT(ptr)) == 0)
			{
				/* Return the block number */
				return (ptr->data.block);
			}
		}
		/* Move onto the next link */
		ptr = ptr->next;
	}

	/* not found, return zero (top of file) */
	return (0);
}

/* ------------------------------------------------------------ ajps ---
 * Pretty much as you'd expect, it links to a file (with reference to a
 * mark point, or not).  Straightforward.
 * --------------------------------------------------------------------- */
static void link_to_file(char *link_file, char *filename, int filename_len, char *mark, history_blk *history)
{
	/* This is used to sotre the position of the mark id within the link */
	char *markpt=NULL;

	/* We can't do much with this */
	if (link_file == NULL) return;

	/* See if we can find the mark separator in the link */
	markpt = strchr(link_file, '#');

	/* We have found a #, so need to split it from the file name */
	if (markpt != NULL)
	{
		/* Zero-terminate the file name by replacing the # */
		markpt[0]=0;

		/* We copy it here to return to the calling function */
		strncpy(mark, markpt + 1, MAX_LINKPOINT_LENGTH);
	}
	else
	{
		/* Make sure we don't accidentally leave a mark there from before */
		strcpy(mark, "");
	}

	/* If there is a link at all now we've removed any mark point */
	if (strlen(link_file) != 0)
	{
		/* Builds path the to file, and stores it in the history stack. */
		help_path_build(filename, filename_len, link_file, (char *)(history[0].file), history);
	}             
	else
	{
		/* There is no file, so make sure the calling function knows that */
		strcpy(filename, "");
	}

	/* Restore the # we removed before. */
	if (markpt != NULL)
	{
		markpt[0] = '#';
	}

	return;
}

/* ------------------------------------------------------------ ajps ---
 * Does the action appropriate to the type of link that has been
 * selected - load a new file, move back or forward through the history,
 * or exit the help system with or without a return value.
 * --------------------------------------------------------------------- */
bool go_to_link(link_blk *linkptr, char *filename, int filename_len, char *mark, history_blk *history, u32b *passback)
{
	/* This isn't really a link */
	if (linkptr == NULL) return (TRUE);

	/* Choose on the basis of the type of link */
	switch (linkptr->type)
	{
		case LINK_TYPE_REF:
		{
			/*
			 * This is a standard link to a file, or a mark point
			 * within that file or the current file.
			 */
			link_to_file(LINK_TEXT(linkptr), filename, filename_len, mark, history);
			break;
		}

		case LINK_TYPE_BACK:
		{
			/*
			 * Push the current file to the bottom of the history
			 * stack and thereby bring the previous file to the
			 * top.
			 */
			history_loop(history, -1);

			/*
			 * Get the canonical path to the file at the top of the stack.
			 */
			help_path_build(filename, filename_len, (char *)(history[0].file), "", NULL);
			break;
		}

		case LINK_TYPE_FORWARD:
		{           
			/*
			 * Pull the file from the bottom of the history up to the
			 * top, pushing the current file down by one in the process.
			 */
			history_loop(history, +1);
              
			/*
			 * Get the canonical path to the file at the top of the stack.
			 */
			help_path_build(filename, filename_len, (char *)(history[0].file), "", NULL);
			break;
		}

		case LINK_TYPE_RETURN:
		{
			/*
			 * The file has requested that we leave help and return some
			 * information (a number).  We place this number into the
			 * variable passback so that it can be used by the calling
			 * function.
			 */
			*passback = atoi(LINK_TEXT(linkptr));

			/* We are leaving the help system, so return FALSE */
			return (FALSE);

			break;
		}

		case LINK_TYPE_EXIT:
		{
			/* There is no information to pass back to the calling function */
			*passback = 0;

			/* But we are leaving help nonetheless */
			return (FALSE);

			break;
		}
	}

	/* We are staying inside the help system */
	return (TRUE);
}

/* ------------------------------------------------------------ ajps ---
 * Returns a pointer to the currently highlighted link, and (optionally)
 * its reference number.
 * --------------------------------------------------------------------- */
static link_blk *get_selected_link(char *links, int *ref)
{
	/* Start at the first link in the block */
	link_blk *ptr = (link_blk *)links;

	/* While there are still links to check */
	while (ptr->type != LINK_TYPE_END)
	{
		/* We are not interested in link points, only references */
		if (ptr->type >= LINK_TYPE_REF)
		{
			/* If the link number is wanted, increment it now */
			if (ref != NULL) (*ref)++;

			if (ptr->data.status == LINK_SELECTED)
			{
				/* We've found the selected link, so return a pointer to it */
				return (ptr);
			}
		}
		/* Move onto the next link in the block */
		ptr = ptr->next;
	}

	/*
	 * We haven't found a selected link, so if a reference number is
	 * wanted, we say it is zero to indicate this.
	 */
	if (ref != NULL) *ref = 0;

	/* We've not found a selected link, so return NULL */
	return (NULL);
}

/* ------------------------------------------------------------ ajps ---
 * Finds the link that corresponds to a keycode.
 * --------------------------------------------------------------------- */
static link_blk *find_link_from_key(char *links, char key)
{
	/* Start at the beginning of the links block */
	link_blk *ptr = (link_blk *)links;

	while (ptr->type != LINK_TYPE_END)
	{
		/* Not interested in link points, only references */
		if (ptr->type >= LINK_TYPE_REF)
		{
			if ((char)(ptr->key) == key)
			{
				/* We've found a match */
				return (ptr);
			}
		}
		/* Move onto the next link */
		ptr = ptr->next;
	}

	/* We've not found one, so return zero */
	return (NULL);
}

/* ------------------------------------------------------------ ajps ---
 * This returns the next link geographically, if you will, rather than
 * the next link in the links block.  NULL if there isn't one.
 *
 * We need this because of the use of tables, where a link can be much
 * later in the file than it appears on screen.
 * --------------------------------------------------------------------- */
static link_blk *next_link(char *links, link_blk *ptr)
{
	/*
	 * current_winner holds the location of the link which seems
	 * at the current time to be that next on the page.  We test new
	 * cases against this and ptr to find its position on the page
	 */
	link_blk *current_winner = NULL;

	/* This is the link we are currently looking at */
	link_blk *checking = (link_blk *)links;

	/* Whether we want to check the current link against the current winner */
	bool do_check = FALSE;

	/* Carry on until the end of the links block */
	while (checking->type != LINK_TYPE_END)
	{
		/* Only interested in links to places, not mark points */
		if (checking->type >= LINK_TYPE_REF)
		{
			/* We start off assuming we don't want to test this */
			do_check = FALSE;

			/* If ptr != NULL we have to check more things */
			if (ptr != NULL)
			{
				/*
				 * this is a special case where the two links start on
				 * the same line, so we need to check their relative x
				 * positions to determine which is later on the page.
				 *
				 * We only want to examine those links later than the
				 * one passed to us in ptr.
				 */
				if (checking->start_line == ptr->start_line)
				{
					/*
					 * if the link we are checking is further to the
					 * right on the page than the one we were passed
					 * then we are interested in checking it against the
					 * current winner.
					 */
					if (checking > ptr)
					{
						do_check = TRUE;
					}
				}
				/*
				 * if the link we are checking starts further down the
				 * page than our start-point we are interested in testing
				 * it against the current winner.
				 */
				else if (checking->start_line > ptr->start_line)
				{
				   	do_check = TRUE;
				}
			}
			/* if ptr == NULL we need to check, to find the first link */
			else
			{
				do_check = TRUE;
			}

			if (do_check)
			{
				/* if we have no current winner, this one is, by default */
				if (current_winner == NULL)
				{
					current_winner = checking;
				}
				/*
				 * If the link we are checking is before the current winner
				 * it becomes the current winner.
				 *
				 * We already know it is later than that we were passed.
				 */
				else if (checking->start_line < current_winner->start_line)
				{
					current_winner = checking;
				}
				/* If they are on the same line, we need to look more closely */
				else if (checking->start_line == current_winner->start_line)
				{
					/*
					 * if the one we are checking is earlier than the winner
					 * (we already know that it is later the the one we were
					 * passed).
					 */
					if (checking < current_winner)
					{
						current_winner = checking;
					}
				}
			}
		} /* end if (link type) */
		checking = checking->next;
	} /* end while */

	return (current_winner);
}

/* ------------------------------------------------------------ ajps ---
 * This returns the previous link geographically rather than
 * the previous link in the links block.  NULL if there isn't one.
 *
 * We need this because of the use of tables, where a link can be much
 * later in the file than it appears on screen.
 * --------------------------------------------------------------------- */
static link_blk *prev_link(char *links, link_blk *ptr)
{
	/*
	 * This is the current link that is closest to ptr but before
	 * it on screen, at the current time.
	 */
	link_blk *current_winner = NULL;

	/*
	 * This is the link we are checking at the moment.  Starts at
	 * the beginning of the links block.
	 */
	link_blk *checking = (link_blk *)links;

	/*
	 * This is whether or not we want to compare the link we are
	 * checking against the current winner.  ie. if "checking" is
	 * actually before the ptr we were passed on the screen.
	 */
	bool do_check = FALSE;

	while (checking->type != LINK_TYPE_END)
	{
		/* We don't want to check link points */
		if (checking->type >= LINK_TYPE_REF)
		{
			do_check = FALSE;

			/* There is a chance that checking appears before ptr on screen */
			if (checking->start_line <= ptr->start_line)
			{
				/* Weed out the case where they are on the same line */
				if (checking->start_line == ptr->start_line)
				{
					/*
					 * If checking is before ptr in memory but on the same
					 * line, then it is to the left (before) ptr on screen.
					 */
					if (checking < ptr)
					{
						do_check = TRUE;
					}
				}
				else
				{
					/*
					 * If the line number is earlier,
				  	 * it is definately earlier on screen.
				  	 */
					do_check = TRUE;
				}

				if (do_check)
				{
					/* If there is no winner, this is, by default. */
					if (current_winner == NULL)
					{
						current_winner = checking;
					}
					else if (checking->start_line > current_winner->start_line)
					{
						/* We are closer to ptr than the previous winner */
						current_winner = checking;
					}
					else if (checking->start_line == current_winner->start_line)
					{
						/*
						 * We check memory location in the case
						 * of the same line.
						 */
						if (checking > current_winner)
						{
							current_winner = checking;
						}
					}
				}
			}
		}
		/* move onto the next link in the block */
		checking = checking->next;

	} /* end while */

	/*
	 * return the location of the winner.  This will be NULL if
	 * there is no link before the one we were passed.
	 */
	return (current_winner);
}

/* ------------------------------------------------------------ ajps ---
 * This moves the selection to the first link currently on screen.
 * We determine what is on screen by the top and bottom line number
 * which is passed to us.
 * --------------------------------------------------------------------- */
static void select_first_link(char *links, int top, int bottom)
{
	link_blk *linkptr;

	/* Get the first link in the file */
	linkptr = next_link(links, NULL);

	/* Move through the links until one appears "on screen" */
	while (linkptr != NULL && (linkptr->end_line < top))
	{
		linkptr = next_link(links, linkptr);
	}

	/* As long as this link isn't off the bottom, set it selected */
	if ( (linkptr != NULL) && (linkptr->start_line <= bottom))
	{
		linkptr->data.status = LINK_SELECTED;
	}

	/*
	 * We can now assume that if a link lies between to lines given,
	 * it is selected.
	 */
	return;
}

/* ------------------------------------------------------------ ajps ---
 * This selected the last link on the visible page, specified by the
 * top and bottom line we are passed.
 * --------------------------------------------------------------------- */
static void select_last_link(char *links, int top, int bottom)
{
	link_blk *linkptr;

	/*
	 * We need a secondary pointer as otherwise we may step
	 * past the end of the links block.
	 */
	link_blk *temp = NULL;

	/* Start with the first link in the file */
	linkptr = next_link(links, NULL);

	/* Step through while we are above the bottom line */
	while (linkptr != NULL && (linkptr->start_line <= bottom))
	{
		temp = linkptr;
		linkptr = next_link(links, linkptr);
	}

	/*
	 * Compare the last one before we went off screen with the
	 * top-of-screen line to see if it is visible or not.
	 */
	if ((temp != NULL) && (temp->end_line >= top))
	{
		/* Select it if it is. */
		temp->data.status = LINK_SELECTED;
	}

	return;
}

/* ------------------------------------------------------------ ajps ---
 * Move the selection down one.  If this takes you off the bottom of
 * the screen (specified by top and bottom line), go to the first link
 * on screen instead.
 * --------------------------------------------------------------------- */
static void link_sel_down(char *links, int top, int bottom)
{
	link_blk *linkptr;

	/* Find the link currently selected */
	linkptr = get_selected_link(links, NULL);

	/* No link selected */
	if (linkptr == NULL)
	{
		select_first_link(links, top, bottom);
	}
	else
	{
		/* Deselect the current link */
		linkptr->data.status = LINK_UNSELECTED;

		/* Move on by one link */
		linkptr = next_link(links, linkptr);

		/* We've run out of links in the block */
		if (linkptr == NULL)
		{
			select_first_link(links, top, bottom);
		}
		/* We've gone off the bottom of the screen */
		else if (linkptr->start_line > bottom)
		{
			select_first_link(links, top, bottom);
		}
		/* We've found a satisfactory link */
		else
		{
			/* Select it */
			linkptr->data.status = LINK_SELECTED;
		}
	}

	return;
}

/* ------------------------------------------------------------ ajps ---
 * Move the link selection up by one.  If this takes it off the screen
 * or past the first link, we go to the bottom link on the page.
 * --------------------------------------------------------------------- */
static void link_sel_up(char *links, int top, int bottom)
{
	link_blk *linkptr;

	/* Find the currently selected link */
	linkptr = get_selected_link(links, NULL);

	/* There is no link selected */
	if (linkptr == NULL)
	{
		select_last_link(links, top, bottom);
	}
	/* There is a link selected */
	else
	{
		/* Deselect the current link */
		linkptr->data.status = LINK_UNSELECTED;

		/* Find the previous link */
		linkptr = prev_link(links, linkptr);

		/* There is no previous link */
		if (linkptr == NULL)
		{
			select_last_link(links, top, bottom);
		}
		else
		{
			/* If the previous link is off the screen */
			if (linkptr->end_line < top)
			{
				/* Go to the bottom of the page */
				select_last_link(links, top, bottom);
			}
			/* It is on the screen */
			else
			{
				/* Select it */
				linkptr->data.status = LINK_SELECTED;
			}
		}
	}

	return;
}

/* ------------------------------------------------------------ ajps ---
 * Puts information into the links block, returns length of block.
 * --------------------------------------------------------------------- */
static int store_link(link_blk *links, int link_type, int flag, int key, char *help_text, char *string)
{
	int size;
	char *ptr;

	/* Place data into the block */
	links->type = link_type;
	links->data.status = flag;
	links->key = (int)(key);

	/* set size to the standard block size */
	size = sizeof(link_blk);

	/*
	 * We want to store extra information, such as a file name or
	 * return value.  And either a help string or not
	 */
	if (string != NULL)
	{
		/* Set ptr to after the standard block information */
		ptr = ((char *)links)+sizeof(link_blk);

		/* copy the string into the block, after the header */
		strcpy(ptr, string);

		/* Increase size by an appropriate amount */
		size += strlen(ptr) + 1;

		/* Move ptr on past the string */
		ptr += strlen(ptr) + 1;

		/* copy any help text into the block */
		strcpy(ptr, help_text);

		/* Increase size by an appropriate amount */
		size += strlen(ptr) + 1;

		/* Make sure size is a multiple of four. */
		size = WORDALIGN(size);
	}

	/* Fill in where the next block will be in memory */
	links->next = (char *)links + size;

	/* Mark the next block as the end (it is at the moment) */
	((link_blk *)links->next)->type = LINK_TYPE_END;

	/* Return the size of the block */
	return (size);
}

/* ------------------------------------------------------------ ajps ---
 * Parses the link tag data, putting the appropriate parts into the
 * link block and the display block.
 * --------------------------------------------------------------------- */
void add_linkptra(char **display, char **links, int *link_no, int *block_no, char *buffer)
{
	char *ptr;

	/* key starts at zero - this means no key is allocated. */
	char key = 0;

	/* This will contain the size of this link in the links block. */
	int size;
	int init = FALSE;
	char help_text[80] = "";


	/*
	 * We check for an id first, as a link may specify both a
	 * point for other links to link to and specify the start
	 * of link text itself.
	 */
	ptr = xmlbulp_check_attr(buffer, "id");

	if (ptr != NULL)
	{
		/* Store the redraw block that we will serve as a link to */
		size = store_link((link_blk *) *links, LINK_TYPE_POINT, *block_no - 1,
						key, help_text, ptr);

		/* Increment the link number for later, or next time around. */
		*links += size;
	}

#ifdef LINKS_ON_PAGE
	/* We don't want a default selection at all */
#else /* LINKS_ON_PAGE */
	/* Set the first link as selected */
	if (*link_no == 1) init=TRUE;
#endif /* LINKS_ON_PAGE */

	/* Check the key attribute */
	ptr = xmlbulp_check_attr(buffer, "key");

	/* key attribute exists */
	if (ptr != NULL)
	{
		/* set key equal to the first character in the attribute */
		key = ptr[0];
	}

#ifdef LINKS_EXTRA
	/* Check if there is a title attribute */
	ptr = xmlbulp_check_attr(buffer, "title");

	/* There is... */
	if (ptr != NULL)
	{
		/* Copy up to 79 characters into help_text, for later storage */
		strncpy(help_text, ptr, 79);

		/* Zero-terminate, on the off-chance we cut something short. */
		help_text[79] = 0;
	}
#endif

	/* Check if there is a file attribute - a normal link */
	ptr = xmlbulp_check_attr(buffer, "file");

	if (ptr != NULL)
	{
		/* Store the link opening in the display block */
		(*display)[0] = REDRAW_CODE_LINKREF;

		/*
		 * We need the link number so that we can work out what
		 * it does when chosen, or whether it is highlighted
		 * or not in the plotting function.
		 */
		(*display)[1] = *link_no;

		/* Move the write point past the data we just stored. */
		(*display) += 2;

		/* Store the information we've gathered in the links block */
		size = store_link((link_blk *) *links,
						LINK_TYPE_REF,
						init ?
						LINK_SELECTED : LINK_UNSELECTED,
						key,
						help_text,
						ptr);

		/* Move the write point for the links block to after this link */
		*links += size;

		/* Increment the link number for next time around. */
		(*link_no)++;

		/* If it is a file link, it can't be any other type of link */
		return;
	}

	/* Check for the action attribute */
	ptr = xmlbulp_check_attr(buffer, "action");

	if (ptr != NULL)
	{
		int link_type = -1;

		/*
		 * This will stay NULL unless we want to store information
		 * for some reason.  This stops us storing rubbish by accident.
		 */
		char *extra_info = NULL;

		/* If action="back", set the type */
		if (my_stricmp(ptr, "back") == 0)
		{
			link_type = LINK_TYPE_BACK;
		}

		/* If action="exit", set the type */
		if (my_stricmp(ptr, "exit") == 0)
		{
			link_type = LINK_TYPE_EXIT;
		}

		/* If action="forward", set the type */
		if (my_stricmp(ptr, "forward") == 0)
		{
			link_type = LINK_TYPE_FORWARD;
		}

		/* If action="return" */
		if (my_stricmp(ptr, "return") == 0)
		{
			/* Set the type properly */
			link_type = LINK_TYPE_RETURN;

			/* set extra_info to point to the value it is supposed to return */
			extra_info = xmlbulp_check_attr(buffer, "value");
		}

		/* If there has been a meaningful action specified */
		if (link_type != -1)
		{
			/* Mark the start of the link in the redraw block */
			(*display)[0] = REDRAW_CODE_LINKREF;

			/* Store the reference number, for later reference */
			(*display)[1] = *link_no;

			/* Move the write point past the data we've just written */
			(*display) += 2;

			/* Store the file link in the links block */
			size = store_link((link_blk *) *links,
							link_type,
							init ?
							LINK_SELECTED : LINK_UNSELECTED,
							key,
							help_text,
							extra_info);

			/* Move the write point for the links point past this link */
			*links += size;

			/* Increment the link number for next time around. */
			(*link_no)++;

			return;
		}

		/*
		 * If action has been chosen, it can't be another type of link,
		 * even if the action isn't meaningful.
		 */
		return;
	}

	/* We're done. */
	return;
}

/* ------------------------------------------------------------ ajps ---
 * This handle the writing to screen of a display block, and any sub-
 * blocks.
 *
 * It implicitly believes the width passed to it, and may go wrong if
 * you lie to it.  It also assumes you are starting with colour
 * TERM_WHITE - for now at least.
 *
 * It returns the length of the block, in lines, as it is displayed
 * under the specified width.  This is whether or not those lines will
 * fit on screen (though we don't print outside the boundaries passed
 * to us).	We use this to judge which blocks to print, elsewhere, and
 * other odds and ends.
 *
 * There are a couple of (smallish) hacks in here, but I think it's
 * fairly clean given the situation. :)
 *
 * I may still be tidied up somewhat, though.
 * --------------------------------------------------------------------- */
int plot_helpfile_block(int x_pos, int y_pos, int width, int height, char **redraw, char *links, int from_line, link_blk **cur_link)
{
	/* This counts how many lines long the block is */
	int line_count = 0;

	/* Stores the style we should plot things in */
	plot_style style;

	/* Stores information about the block we are in */
	block_style this_block;

	/* Stores information about a sub-block we are about to plot */
	block_style next_block;

	/* Stores the running total of percentage widths */
	int percentage_total = 0;

	/* Where we should print, initialised to the points we were passed */
	int print_x = x_pos, print_y = y_pos;

	/* The width of a block in characters */
	int blk_width = 0;

	/* The lines in this "level", for instance a row of a table or list */
	int level_line_count = 0;

	/* The widths of columns in characters.  For doing vertical rules */
	int column_widths[20];

	/* The column we are in.  For tables. */
	int column_no = 0;

	/* Counter */
	int n = 0;

	/* temporary number store */
	int temp = 0;

	/* Check if our start point is usable or not */
	if ((*redraw)[0] != REDRAW_CODE_BLOCKSTART)
	{
		/* We return an error here. */
		return (PLOT_FAILED);
	}

	/* Set the initial style of the text we plot */
	style.colour = TERM_WHITE;
	style.link = FALSE;
	style.underline = FALSE;
	style.em = FALSE;
	style.strong = FALSE;;

	/* Extract meaningful data from the block header. */
	this_block.type = (*redraw)[1];
	this_block.width = (*redraw)[2];
	this_block.align = (*redraw)[3];

	/* Set the alignment of this block to the value given in the header */
	style.align = this_block.align;

	/* Move the read point past the standard header. */
	(*redraw) += 4;

	/* We do special things for different block types */
	switch (this_block.type)
	{
		case BLOCKTYPE_TEXT:
		{
			/* The next two bytes are left and right indent, we read this in. */
			this_block.lmargin = (*redraw)[0];
			this_block.rmargin = (*redraw)[1];

			/* Move the read point past the header completely */
			(*redraw) += 2;

			/* Put the adjusted block width into blk_width */
			blk_width = width - this_block.lmargin - this_block.rmargin;

			/*
			 * Only change the width and write position if it is sensible
			 * to do so.
			 */
			if (blk_width > 0)
			{
				width = blk_width;
				x_pos += this_block.lmargin;
			}

			break;
		}

		case BLOCKTYPE_TABLE:
		{
			/* The table flag is stored in the next byte */
			this_block.flag = (*redraw)[0];

			/* Move the read point past the header completely */
			(*redraw)++;

			/* We can have a table not really filling the width. */
			if (this_block.width < 100)
			{
				/* Work out the width in characters being asked for */
				blk_width = (width * this_block.width) / 100;

				/* Only change things if it is sensible to do so. */
				if (blk_width > 0)
				{
					if (this_block.align == ALIGN_RIGHT)
					{
						/* Move the write position to the right of the screen */
						x_pos += width - blk_width;
					}

					if (this_block.align == ALIGN_CENTRE)
					{
						/* Move the write position into the centre */
						x_pos += (width-blk_width) / 2;
					}

					/* Set width to the block width */
					width = blk_width;

					/* Reset this so we don't get confused later */
					blk_width = 0;

					/* Set the write position as well. */
					print_x = x_pos;
				}
			}
			break;
		}

		case BLOCKTYPE_LIST:
		{
			/* There is a flag stored in the next character */
			this_block.flag = (*redraw)[0];

			/* Move the read position past the header completely. */
			(*redraw)++;

			break;
		}
	}

	/* If this is a table and a horizontal rule is wanted */
	if (TABLE_H_RULE(this_block))
	{
		/* Check if the line is actually inside the print boundary */
		if (print_y >= (y_pos + from_line) &&
			print_y < (y_pos + from_line + height))
		{
			/* Plot a line the width of the table */
			horiz_rule(print_x, print_y, width, TERM_WHITE);
		}
		/* Count this rule as a line */
		line_count++;

		/* Move the print position down a line. */
		print_y++;
	}

	/* Process the block until we reach the end of it */
	while ((*redraw)[0] != REDRAW_CODE_BLOCKEND)
	{
		/* We've hit the start of a sub-block */
		if ((*redraw)[0] == REDRAW_CODE_BLOCKSTART)
		{
			/* Extract data from the header of the next block. */
			next_block.type = (*redraw)[1];
			next_block.width = (*redraw)[2];
			next_block.align = (*redraw)[3];

			/* We need to know the alignment of this sub-block */
			style.align = next_block.align;

			/* How we deal with the block depends on type. */
			switch (next_block.type)
			{
				case BLOCKTYPE_TEXT:
				{
					next_block.lmargin = (*redraw)[4];
					next_block.rmargin = (*redraw)[5];

					break;
				}

				case BLOCKTYPE_TABLE:
				{
					next_block.flag = (*redraw)[4];

					/*
					 * Whatever width has been specified, we treat it
					 * in /this/ block as if it fills the width.
					 */
					next_block.width = 100;

					break;
				}
			}

			/* Reset the column when we are back the the beginning */
			if (print_x == x_pos) column_no = 0;

			/*
			 * We use a fiddle to specify a fixed width
			 * for the first part of the list.
			 */
			if (this_block.type == BLOCKTYPE_LIST)
			{
				/* This is the bullet-point section */
				if (percentage_total == 0)
				{
					/*
					 * We set the blk_width to be the number
					 * of characters specified, unlike any other block.
					 */
					blk_width = next_block.width;

					/* This is just so we can tell we have plotted this part */
					percentage_total = 1;
				}
				/* therefore this is the textual part */
				else
				{
					/* We want this to fill the rest of the width */
					blk_width = width + x_pos - print_x;

					/* And acknowledge that it fills the width */
					percentage_total = 100;
				}
			}
			/* This isn't a list - the widths are percentages. */
			else
			{
				/* Keep the running total up to date. */
				percentage_total += next_block.width;

				if (percentage_total == 100)
				{
					/* Make sure a total of 100% fills the width! */
					blk_width = width + x_pos - print_x;
				}
				else
				{
					/* Estimate using percentage - we always round down */
					blk_width = (width * next_block.width) / 100;
				}
			}

			/* Recurse into this block */
			if (next_block.width != 100)
			{
				if (this_block.type == BLOCKTYPE_TABLE)
				{
					/* Store the width of the column */
					column_widths[column_no] = blk_width;

					/* Increment the column number for next time */
					column_no++;

					/* Take two off the width to space out the cells */
					blk_width -= 2;

					/* Move the print position by one (one space either side) */
					print_x++;
				}

				/* Plot the block, and store its length in lines */
				temp = plot_helpfile_block(print_x, print_y,
										   blk_width,
										   height,
										   redraw, links,
										   from_line-line_count,
										   cur_link);

				/* Strange error occurred during plotting */
				if (temp == PLOT_FAILED) return (temp);

				/*
				 * For tables and non-compact lists when we've filled
				 * a whole line.
				 */
				if (((this_block.type == BLOCKTYPE_TABLE) ||
					(this_block.type == BLOCKTYPE_LIST &&
					this_block.flag == LIST_NOCOMPACT))
					&& percentage_total == 100)
				{
					/* If there is another row of blocks afterwards */
					if ((*redraw)[0] == REDRAW_CODE_BLOCKSTART)
					{
						/* Increase the number of lines to add a spacer */
						temp++;
						level_line_count++;
					}
				}
			}
			/* A block that fills the whole width */
			else
			{
				/* plot it, and store the length in lines in temp */
				temp = plot_helpfile_block(print_x, print_y,
										   blk_width,
										   height,
										   redraw, links,
										   from_line-line_count,
										   cur_link);

				/* Some error occurred while plotting */
				if (temp == PLOT_FAILED) return (temp);

				/* Adds spacing line /between/ whole line blocks */
				if ((*redraw)[0] != REDRAW_CODE_BLOCKEND) temp++;
			}

			/* Move the print position past this block on the line */
			print_x += blk_width;

			/*
			 * If this block was longer than previous blocks, store
			 * the length.
			 */
			if (temp > level_line_count) level_line_count = temp;

			/* Finished this "row" of blocks */
			if (percentage_total == 100)
			{
				/* Move line_count on by the number of lines filled */
				line_count += level_line_count;

				/* Move the y position to match */
				print_y = y_pos + line_count;

				/* And reset the x position to the beginning of line */
				print_x = x_pos;

				/* If we are in a table and we want horizontal rules */
				if (TABLE_H_RULE(this_block))
				{
					/* As long as we aren't at the end of the table */
					if ((*redraw)[0] != REDRAW_CODE_BLOCKEND)
					{
						/* And the line will be on-screen */
						if (print_y >= (y_pos + from_line + 1) &&
							 print_y < (y_pos + from_line + height + 1))

						/* Plot the rule as required. */
						horiz_rule(print_x + 1, print_y - 1, width - 2,
								   TERM_WHITE);
					}
				}

				/* Reset these counters for the next level */
				level_line_count = 0;
				percentage_total = 0;
			}
		}
		/* This is a block of text to print */
		else
		{
			/* We use this as a work-around for when we are underlining */
			int xtra_lines = 0;

			u32b word_length = 0;

			/* actual length of printable line within the width */
			u32b line_length = 0;

			/* Counters for 'real' characters and control codes respectively */
			u32b n = 0, m = 0;

			/* Initialise ptr to the current read position */
			char *ptr = *redraw;

			/* number of control characters in line */
			u32b controls = 0;

			/* Temp variables, mirroring ptr and controls */
			char *temp_ptr = *redraw;
			u32b temp_controls = 0;

			/*
			 * Establish how many (printable) characters we can
			 * fit on this line.
			 */
			while ((line_length + word_length) <= (u32b) width)
			{
				/* This word fits, so add it to the line */
				line_length += word_length;

				/* Move the pointer forward to the next line */
				ptr = temp_ptr;

				/* Inrease the number of control characters appropriately */
				controls += temp_controls;

				/* We stop looking at newlines and block starts/endings */
				if ((ptr[0] == '\n') ||
					(ptr[0] == '\r') ||
					(ptr[0] == REDRAW_CODE_BLOCKEND) ||
					(ptr[0] == REDRAW_CODE_BLOCKSTART))
					break;

				/* Rest the control character counter */
				temp_controls = 0;

				/* Find the length of the next word */
				word_length = get_word_length(&temp_ptr, &temp_controls);
			}

			/* If there is no word break, we character break instead. */
			if (line_length == 0 && ptr[0] != '\n' && ptr[0] != '\r' && ptr[0] != '\0')
			{
				/* Set the line width to fill the width */
				line_length = width;

				/* Make sure we have no control characters at the end of line */
				while (ptr[line_length - 1] < 32)
				{
					line_length--;
				}

				/* count control characters */
				for (n = 0; n < line_length; n++)
				{
					if (ptr[n] < 32) controls++;
				}

				/* Make the line length shorter still. */
				line_length -= controls;
			}

			/* Set print_x according to alignment (not the D&D type) */
			switch (style.align)
			{
				case ALIGN_RIGHT:
				{
					print_x = x_pos + width - (int) line_length;
					break;
				}

				case ALIGN_CENTRE:
				{
					print_x = x_pos + ((width - (int) line_length) / 2);
					break;
				}

				case ALIGN_LEFT:
				{
					print_x = x_pos;
					break;
				}
			}

			/* Set print_y to the correct line */
			print_y = y_pos + line_count;

			/* Reset counters */
			n = 0; m = 0;

			/* Analyse the line character-by-character */
			while ((n < line_length) || (m < controls))
			{
				/* (*redraw)[0] is the first character at the read position */
				switch ((*redraw)[0])
				{
					/* Plot a horizontal rule here. */
					case REDRAW_CODE_HRULE:
					{
						/* If the line is on-screen */
						if (line_count >= from_line &&
							print_y < (y_pos + from_line + height))
						{
							/* Set the width as asked for */
							int rule_width = (width * (*redraw)[2]) / 100;

							int x_rule = x_pos;

							switch ((*redraw)[1])
							{
								case ALIGN_LEFT:
								{
									x_rule = x_pos;
									break;
								}

								case ALIGN_RIGHT:
								{
									x_rule = x_pos + width - rule_width;
									break;
								}

								case ALIGN_CENTRE:
								{
									x_rule = x_pos + (width - rule_width) / 2;
									break;
								}
							}

							/* Plot rule, in the present colour */
							horiz_rule(x_rule, print_y, rule_width, style.colour);
						}

						/* Move read position by 3 */
						(*redraw) += 3;

						/* Increment the control character counter */
						m += 3;

						break;
					}

					/* Change colour */
					case REDRAW_CODE_COLOUR:
					{
						/* The next byte contains the colour code */
						style.colour = (*redraw)[1];

						/* Move read position by 2 */
						*redraw += 2;

						/* Increment the control character counter */
						m += 2;

						break;
					}

					/* Change into or out of emphasis */
					case REDRAW_CODE_EMPH:
					{
						/* toggle emphasis */
						style.em = !style.em;

						/* Set colour according to the style */
						if (style.em)
						{
							style.colour = EMPH_COLOUR;
						}
						else
						{
							style.colour = TERM_WHITE;
						}

						/* Move read position by 1 */
						(*redraw)++;

						/* And control character counter by one */
						m++;

						break;
					}

					/* Change into or out of strong */
					case REDRAW_CODE_STRONG:
					{
						/* toggle emphasis */
						style.strong = !style.strong;

						/* Set colour according to the style */
						if (style.strong)
						{
							style.colour = STRONG_COLOUR;
						}
						else
						{
							style.colour = TERM_WHITE;
						}

						/* Move read position by 1 */
						(*redraw)++;

						/* And control character counter by one */
						m++;

						break;
					}

					/* Switch underline on or off. */
					case REDRAW_CODE_UNDERLINE:
					{
						/*
						 * Minor hack:
						 * style.underline is also used by the mega-underline,
						 * so we only turn it off if it is a 'standard'
						 * underline character.
						 */
						switch (style.underline)
						{
							case FALSE:
							{
								style.underline = UNDERLINE_CHARACTER;
								break;
							}

							default:
							{
								style.underline = FALSE;
							}
						}

						/* Move the read position on by one */
						(*redraw)++;

						/* Increase the control character counter */
						m++;

						break;
					}

					/* Switch mega-underline on or off. (used for h1 tags)*/
					case REDRAW_CODE_MEGAUNDERLINE:
					{
						/*
						 * Minor hack:
						 * style.underline is also used by the underline,
						 * so we only turn it off if it is a "mega"
						 * underline character.
						 */
						switch (style.underline)
						{
							case 0:
							{
								style.underline = MEGAUNDERLINE_CHARACTER;
								break;
							}

							default:
							{
								style.underline = 0;
							}
						}

						/* Move the read position on by one */
						(*redraw)++;

						/* Increase the control character counter */
						m++;

						break;
					}

					/* Some sort of link */
					case REDRAW_CODE_LINKREF:
					{
						/* the end of the link text */
						if ((*redraw)[1] == 0)
						{
							/* If we are being asked to fill in link data */
							if (cur_link != NULL)
							{

								/* If we aren't at the start of a line */
								if (print_x != x_pos)
								{
									/* Store this line as where the link ends */
									(*cur_link)->end_line = print_y;
								}
								else
								{
									/*
									 * Store the line before as where
									 * the link ends
									 */
									(*cur_link)->end_line = print_y - 1;
								}

								/*
								 * Set cur_link to point to the next link
								 * of this type.
								 */
								while ((*cur_link)->type != LINK_TYPE_END)
								{
									/* Move to the next link */
									*cur_link = (*cur_link)->next;

									/* Check the type is suitable */
									if ((*cur_link)->type >= LINK_TYPE_REF)
									{
										break;
									}
								}
							}

							/* Turn off the link style */
							style.link = FALSE;
						}
						else
						/* the start of some link text */
						{
							/* If we are being asked to store link data */
							if (cur_link != NULL)
							{
								/*
								 * If we've reached the end of the
								 * links prematurely, return an error
								 */
								if ((*cur_link)->type == LINK_TYPE_END)
								{
									return (FALSE);
								}

								/* Store the start line of this link */
								(*cur_link)->start_line = print_y;
							}

							/*
							 * Set the link style to the status of
							 * the link, ie. highlighted or not.
							 */
							style.link = link_info(links,
											(*redraw)[1], NULL)->data.status;
						}

						/* Move the redraw position on by two */
						(*redraw) += 2;

						/* Count the control characters we've dealt with */
						m += 2;
						break;
					}

					/* Print the character to screen! */
					default:
					{
						/* If the text is underlined */
						if (style.underline)
						{
							/* Store the fact we are using an extra line */
							xtra_lines = 1;
						}

						/*
						 * We only bother printing it if it is within the
						 * bounds of teh screen as we were given it.
						 */
						if (line_count >= from_line &&
							 print_y < (y_pos + from_line + height))
						{
							/* If it is not a link */
							if (!style.link)
							{
								/* Print the character */
								Term_putch(print_x, print_y, style.colour,
										   (*redraw)[0]);
							}
							/* It is a link */
							else
							{
								/*
								 * Print the character coloured according
								 * to whether it is highlighted or not.
								 */
								Term_putch(print_x, print_y,
								(style.link == LINK_SELECTED) ?
								 LINK_COLOUR_SELECTED : LINK_COLOUR_UNSELECTED,
								 (*redraw)[0]);
							}
						}

						/* If we need to underline that character */
						if (style.underline)
						{
							/* Check the next line is on screen */
							if (line_count + 1 >= from_line &&
								 print_y + 1 < (y_pos + from_line + height))
							{
								/* Plot the underline character */
								Term_putch(print_x, print_y + 1, style.colour,
											style.underline);
							}
						}

						/* Move the print position by one */
						print_x++;

						/* Move past the character we've just printed */
						(*redraw)++;

						/* Count the normal character we've printed */
						n++;
					}
				}
			}
			/* We've now finished that line */

			/* If the next character is a newline */
			if ((*redraw)[0] == '\n')
			{
				/* Deal with consecutive newlines */
				while ((*redraw)[0] == '\n')
				{
					/* Count in any extra lines we've used */
					line_count += xtra_lines;

					/* Reset extra-lines */
					xtra_lines = 0;

					/* Increase the count by the usual single line */
					line_count++;

					/* Move the read position past the newline */
					(*redraw)++;

					/* Ignore a CR immediately after an LF */
					if ((*redraw)[0] == '\r') (*redraw)++;
				}
			}
			/* If the next character is a carriage return */
			else if ((*redraw)[0] == '\r')
			{
				/* Deal with consecutive carriage returns */
				while ((*redraw)[0] == '\r')
				{
					/* Count in any extra lines we've used */
					line_count += xtra_lines;

					/* Reset extra-lines */
					xtra_lines = 0;

					/* Increase the count by the usual single line */
					line_count++;

					/* Move the read position past the newline */
					(*redraw)++;

					/* Ignore an LF immediately after a CR */
					if ((*redraw)[0] == '\r') (*redraw)++;
				}
			}
			/* If the next character isn't any sort of newline */
			else
			{
				/* Count in any extra lines we've used */
				line_count += xtra_lines;

				/* Increase the count by the usual single line */
				line_count++;

				/* Strip the whitespace at the start of the next line */
				while ((*redraw)[0] == ' ') (*redraw)++;
			}

			/* If the next character marks the start of a block */
			if ((*redraw)[0] == REDRAW_CODE_BLOCKSTART)
			{
				/* Reset the x position and increase the y position */
				print_x = x_pos;
				print_y++;
			}
		}
	}

	/* We've finished normal processing of the redraw block, but... */

	/* If this is a table */
	if (this_block.type == BLOCKTYPE_TABLE)
	{
		int mod=0;

		/* If the table wants horizontal rules */
		if ((this_block.flag & FLAG_TABLE_HRULES) == FLAG_TABLE_HRULES)
		{
			/* If the line should be printed */
			if (print_y >= (y_pos + from_line) &&
				print_y < (y_pos + from_line + height))
			{
				horiz_rule(print_x, print_y, width, TERM_WHITE);
			}

			/* Reflect that we have added a line */
			line_count++;

			/*
			 * If we have horizontal rules, it affects how
			 * we deal with vertical rules differently
			 */
			mod = 1;
		}

		/* If the table wants vertical rules */
		if ((this_block.flag & FLAG_TABLE_VRULES) == FLAG_TABLE_VRULES)
		{
			int print_yy;
			int print_height;

			/* Set the y print position */
			print_yy = print_y - line_count + mod;

			/* Set the height of the line */
			print_height = line_count - mod;

			/* If the start position is off the bottom of the screen */
			if (print_yy < (y_pos + from_line))
			{
				/* Adjust the height accordingly */
				print_height -= (y_pos + from_line - print_yy);

				/* And bring it back onto the screen */
				print_yy = (y_pos + from_line);
			}

			/* If the start position is off the top of the screen */
			if (print_yy >= (y_pos + from_line + height))
			{
				/* We don't have to do anything else */

				/* Move past the end-of-block code */
				(*redraw)++;

				/* Return the number of lines we've produced (printed or not) */
				return (line_count);
			}

			/* If the end position is off the top of the screen  */
			if (print_yy + print_height >= (y_pos + from_line + height))
			{
				/* Adjust it so that it isn't */
				print_height = (y_pos + from_line + height - print_yy) ;
			}

			/* Step through column-by-column */
			for (n = 0; n <= column_no; n++)
			{
				/* Plot the line for this column */
				vert_rule(print_x,
						  print_yy,
						  print_height,
						  TERM_WHITE);

				/* Move onto the next column divider */
				print_x += column_widths[n] - 1;
			}
		}
	}
	/* Move past the end-of-block code */
	(*redraw)++;

	/* Return the number of lines we've produced (printed or not) */
	return (line_count);
}

/* ------------------------------------------------------------ ajps ---
 * Simply checks if the string consists solely of white space.
 * --------------------------------------------------------------------- */
bool xml_whitespace(char *string)
{
	size_t count=0;

	/* For each character in the string */
	while (count < strlen(string))
	{
		/* if it isn't any sort of whitepace */
		if (!((string[count] == ' ') ||
				(string[count] == '\r') ||
				(string[count] == '\n')))
		{
			/* Return false */
			return (FALSE);
		}

		count++;
	}

	/* It must all have been whitespace */
	return (TRUE);
}

/* ------------------------------------------------------------ ajps ---
 * Adds colour information to the redraw block.
 * --------------------------------------------------------------------- */
void add_colour(char **display, char *buffer)
{
	char *ptr;

	/* is there a name attribute? */
	ptr = xmlbulp_check_attr(buffer, "name");

	if (ptr != NULL)
	{
		/* Add the information to the redraw block */
		(*display)[0] = REDRAW_CODE_COLOUR;
		(*display)[1] = color_text_to_attr(ptr);

		/* Move the write position on by two */
		*display += 2;
	}
}

/* ------------------------------------------------------------ ajps ---
 * Return the alignment code given it as a string
 * --------------------------------------------------------------------- */
char set_alignment(char *alignment)
{
	if (my_stricmp(alignment, "left") == 0)
	{
		return (ALIGN_LEFT);
	}

	if (my_stricmp(alignment, "centre") == 0 ||
		my_stricmp(alignment, "center") == 0)
	{
		return (ALIGN_CENTRE);
	}

	if (my_stricmp(alignment, "right") == 0)
	{
		return (ALIGN_RIGHT);
	}

	/* The default for all alignments is left */
	return (ALIGN_LEFT);
}

/* ------------------------------------------------------------ ajps ---
 * Takes a number, and fills the buffer with it in roman numerals.
 * It then converts the case if necessary.
 *
 * It's simple enough, but frankly the function isn't beautiful, and
 * could do with a bit more thought and better code.
 * --------------------------------------------------------------------- */
void make_roman_numerals(char *buffer, u32b number, bool upper_case)
{
	u32b remainder, result;
	size_t n;

	/* result = number of thousands */
	result = number / 1000;

	/* remainder = what's left */
	remainder = number % 1000;

	/* For each thousand in the number */
	for (n = 0; n < result; n++)
	{
		/* add an "m" to the buffer */
		strcat(buffer, "m");
	}

	/* having dealt with thousands, we set number = everything else */
	number = remainder;

	/* result = number of hundreds */
	result = number / 100;

	/* remainder = everything else */
	remainder = number % 100;

	/* Deal with the case of nine hundred (one less than one thousand) */
	if (result == 9)
	{
		strcat(buffer, "cm");
	}
	/* and four hundred (one less than five hundred */
	else if (result == 4)
	{
		strcat(buffer, "cd");
	}
	/* All other cases */
	else
	{
		/* result = number of five hundreds (should be 1 or 0) */
		result = number / 500;

		/* There is a five hundred */
		if (result == 1)
		{
			strcat(buffer, "d");
		}

		/* Set remainder to the number of hundreds not dealt with */
		remainder = (number / 100) - (5 * result);

		/* For each hundred, add a 'c' to the buffer */
		for (n = 0; n < remainder; n++)
		{
			strcat(buffer, "c");
		}
	}

	/* Set number = number left after hundreds are dealt with */
	number = number % 100;

	/* Set result = number of tens */
	result = number / 10;

	/* Set remainder = whatever's left */
	remainder = number % 10;

	/* Case of one-less-than-ten tens */
	if (result == 9)
	{
		strcat(buffer, "xc");
	}
	/* Case of one-less-than-five tens */
	else if (result == 4)
	{
		strcat(buffer, "xl");
	}
	/* Other cases */
	else
	{
		/* extract number of fifties (0 or 1) */
		result = number / 50;

		/* If there is a fifty */
		if (result == 1)
		{
			/* add 'l' to the buffer */
			strcat(buffer, "l");
		}

		/* Extract the number of tens not dealt with yet */
		remainder = (number / 10) - (5 * result);

		/* Add an 'x' for each ten left */
		for (n = 0; n < remainder; n++)
		{
			strcat(buffer, "x");
		}
	}

	/* Set number to the number of units left over now */
	number = number % 10;

	/* One-less-than-ten */
	if (number == 9)
	{
		strcat(buffer, "ix");
	}
	/* One-less-than-five */
	else if (number == 4)
	{
		strcat(buffer, "iv");
	}
	else
	{
		/* Extract number of fives */
		result = number / 5;

		/* And whatever is left over */
		remainder = number % 5;

		/* If there is a five */
		if (result == 1)
		{
			/* Add a 'v' */
			strcat(buffer, "v");
		}

		/* For each unit left, add an 'i' */
		for (n = 0; n < remainder; n++)
		{
			strcat(buffer, "i");
		}
	}

	/* If they want it in upper case */
	if (upper_case)
	{
		/* Just change it to upper case. */
		for (n = 0; n < strlen(buffer); n++)
		{
			buffer[n] = toupper(buffer[n]);
		}
	}

	return;
}

/* ------------------------------------------------------------ ajps ---
 * This extracts the width as an int from a tag.  Returns zero
 * if is hasn't been specified or specified improperly.
 * --------------------------------------------------------------------- */
int get_percentage_width(char *buffer)
{
	char *ptr = buffer;

	ptr = xmlbulp_check_attr(buffer, "width");

	if (ptr != NULL)
	{
		/* Check for a percentage sign */
		if (ptr[strlen(ptr)-1] == '%')
		{
			/* Convert the text to an int */
			return (atoi(ptr));
		}
	}

	/* If we've not got a meaningful width, we return zero */
	return (0);
}

/* ------------------------------------------------------------ ajps ---
 * Makes the text for a list bullet point, whatever the type of list.
 * --------------------------------------------------------------------- */
char *make_list_point(char *buffer, list_blk *list_data)
{
	switch (list_data->type)
	{
		/* Numbered list */
		case LIST_TYPE_1:
		{
			/* Just put in the number */
			sprintf(buffer, "(%i) ", list_data->item_no);
			break;
		}

		/* lower case letters */
		case LIST_TYPE_a:
		{
			/* Count up from ASCII 'a' - hope there aren't more than 26 */
			sprintf(buffer, "(%c) ", 'a'+list_data->item_no-1);
			break;
		}

		/* upper case letters */
		case LIST_TYPE_A:
		{
			/* Count up from ASCII 'A' - hope there aren't more than 26 */
			sprintf(buffer, "(%c) ", 'A'+list_data->item_no-1);
			break;
		}

		/* lower case roman numerals */
		case LIST_TYPE_i:
		{
			sprintf(buffer, "(");

			/* Add the roman numberals, FALSE means lower-case */
			make_roman_numerals(buffer, list_data->item_no, FALSE);

			strcat(buffer, ") ");

			break;
		}

		/* upper case roman numerals */
		case LIST_TYPE_I:
		{
			sprintf(buffer, "(");

			/* Add the roman numberals, TRUE means upper-case */
			make_roman_numerals(buffer, list_data->item_no, TRUE);

			strcat(buffer, ") ");

			break;
		}

		/* Bullet point */
		case LIST_TYPE_BULLET:
		{
			/* Bullet point and space */
			sprintf(buffer, "%c ", LIST_BULLET_POINT);
			break;
		}
	}

	/* Return a pointer to the buffer we were passed */
	return (buffer);
}


/* ------------------------------------------------------------ ajps ---
 * Adds a horizontal rule to the text
 * --------------------------------------------------------------------- */
void add_hrule(char **display, char *tag, int width)
{
	char *ptr;

	/* We'll put the data into this first */
	block_style header;

	/* percent */
	header.width = width;

	/* default setting */
	header.align = ALIGN_LEFT;

	/* left margin */
	header.lmargin = 0;

	/* right margin */
	header.rmargin = 0;

	/* Write the header to the display block */
	write_textblock_header(display, &header);

	/* Actual redraw codes, defaults */
	(*display)[0] = REDRAW_CODE_HRULE;
	(*display)[1] = ALIGN_CENTRE;

	/* Extracts a vidth if there is one - zero otherwise */
	(*display)[2] = get_percentage_width(tag);

	/* Set to the deafult if there was nothing specified. */
	if ((*display)[2] == 0) (*display)[2] = 100;

	/* Check if an alignment was specified. */
	ptr = xmlbulp_check_attr(tag, "align");

	if (ptr != NULL)
	{
		/* Extract the alignment code from the text */
		(*display)[1] = set_alignment(ptr);
	}

	(*display) += 3;
}

/* ------------------------------------------------------------ ajps ---
 * Adds the start-of-block information for a paragraph to the display
 * block.
 * --------------------------------------------------------------------- */
void add_paragraph(char **display, char *buffer, int width)
{
	char *ptr = buffer;

	/* We'll put the data into this first */
	block_style header;

	/* percent */
	header.width = width;

	/* default setting */
	header.align = ALIGN_LEFT;

	/* left margin */
	header.lmargin = 0;

	/* right margin */
	header.rmargin = 0;

	ptr = xmlbulp_check_attr(buffer, "align");
	if (ptr != NULL)
	{
		/* Extract the alignment code from the text */
		header.align = set_alignment(ptr);
	}

	ptr = xmlbulp_check_attr(buffer, "lmargin");
	if (ptr != NULL)
	{
		/* The margin should just be a number of characters */
		header.lmargin = atoi(ptr);
	}

	ptr = xmlbulp_check_attr(buffer, "rmargin");
	if (ptr != NULL)
	{
		/* The margin should just be a number of characters */
		header.rmargin = atoi(ptr);
	}

	/* Write the header to the display block */
	write_textblock_header(display, &header);
}

/* ------------------------------------------------------------ ajps ---
 * Adds the start-of-block information for a head to the display
 * block.
 * --------------------------------------------------------------------- */
void add_head_data_open(char **display, char *buffer)
{
	char *ptr;

	/* Holds the header information as we extract it */
	block_style header;

	/* Level of heading: 1, 2 or 3 */
	int level;

	/* Get level by looking at second character (h1, h2 or h3) */
	level = atoi(buffer + 1);

	/* Check for ludicrousity */
	if (level > 3) return;

	/* Always 100 percent */
	header.width = 100;

	/* Default settings generally */
	header.align = ALIGN_LEFT;
	header.lmargin = 0;
	header.rmargin = 0;

	/* Choose alignment based on heading type */
	switch (level)
	{
		case 1:
		{
			header.align = h1_ALIGN;
			break;
		}

		case 2:
		{
			header.align = h2_ALIGN;
			break;
		}

		case 3:
		{
			header.align = h3_ALIGN;
			break;
		}
	}

	/* Extract the alignment, if specified it overrides the normal one */
	ptr = xmlbulp_check_attr(buffer, "align");
	if (ptr != NULL)
	{
		header.align = set_alignment(ptr);
	}

	/* Write the header to the display block */
	write_textblock_header(display, &header);

	switch (level)
	{
		case 1:
		{
			/* Add an underline code */
			(*display)[0] = REDRAW_CODE_MEGAUNDERLINE;
			(*display)++;

			/* And a change in colour */
			(*display)[0] = REDRAW_CODE_COLOUR;
			(*display)[1] = h1_COLOUR;
			(*display) += 2;

			break;
		}

		case 2:
		{
			/* Add an underline code */
			(*display)[0] = REDRAW_CODE_UNDERLINE;
			(*display)++;

			/* And a change in colour */
			(*display)[0] = REDRAW_CODE_COLOUR;
			(*display)[1] = h2_COLOUR;
			(*display) += 2;

			break;
		}

		case 3:
		{
			/* Change the colour */
			(*display)[0] = REDRAW_CODE_COLOUR;
			(*display)[1] = h3_COLOUR;
			(*display) += 2;

			break;
		}
	}

	return;
}

/* ------------------------------------------------------------ ajps ---
 * Adds close-of-header bits to the redraw block.
 * --------------------------------------------------------------------- */
void add_head_data_close(char **display, char *buffer)
{
	int level;

	/* Extract the level code from the tag */
	level = atoi(buffer + 1);

	/* Sanity check */
	if (level > 3) return;

	switch (level)
	{
		case 1:
		{
			/* Turn off the colouring */
			(*display)[0] = REDRAW_CODE_COLOUR;
			(*display)[1] = TERM_WHITE;
			(*display) += 2;

			/* Turn off the underlining */
			(*display)[0] = REDRAW_CODE_MEGAUNDERLINE;
			(*display)++;

			break;
		}

		case 2:
		{
			/* Turn off the colouring */
			(*display)[0] = REDRAW_CODE_COLOUR;
			(*display)[1] = TERM_WHITE;
			(*display) += 2;

			/* Turn off the underlining */
			(*display)[0] = REDRAW_CODE_UNDERLINE;
			(*display)++;

			break;
		}

		case 3:
		{
			/* Turn off the colouring */
			(*display)[0] = REDRAW_CODE_COLOUR;
			(*display)[1] = TERM_WHITE;
			(*display) += 2;

			break;
		}
	}

	/* A title is a self-contained block.  Close it. */
	(*display)[0] = REDRAW_CODE_BLOCKEND;
	(*display)++;

	return;
}

/* ------------------------------------------------------------ ajps ---
 * Here we do a preparse pass over the list section, in order to
 * collect the maximum width for the first column mainly.
 * --------------------------------------------------------------------- */
bool preparse_list(BULP *bptr, list_blk *list_info, char *err_message)
{
	int chunk_type;
	size_t size;
	int list_no = 0;
	int width;

	/* While still in the file */
	while (!bulp_feof(bptr))
	{
		/* Get the type and size of the next chunk */
		chunk_type = xmlbulp_get_next_bit(bptr, &size);

		switch (chunk_type)
		{
			/* fatal errors returned by xmlbulp_get_next_bit */
			case GETPOS_FAILED:
			case TYPE_EOF:
			{
				sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
				return (FALSE);
				break;
			}

			/* We aren't interested at the moment */
			case TYPE_COMMENT:
			case TYPE_DOCTYPE:
			case TYPE_ENTITYDEF:
			case TYPE_TEXT:
			case TYPE_CDATA:
			case TYPE_CHARENTITY:
			{
				/* Just skip them */
				xmlbulp_skip_chunk(bptr, size);

				break;
			}

			case TYPE_TAG:
			{
				int tag_type;
				char buffer[REASONABLE_LINELENGTH];

				/* Read in the tag */
				tag_type = xmlbulp_get_tag_and_type(bptr, size, buffer);

				switch (tag_type)
				{
					case TAG_OPEN:
					{
						/* List opened */
						if (my_stricmp(buffer, "list") == 0)
						{
							list_no++;
						}

						/*
						 * if an item has been opened in "this" list -
						 * the one we are preparsing information for.
						 */
						if (my_stricmp(buffer, "item") == 0 && list_no == 0)
						{
							char temp[20];

							/* Increment the number of items in the list. */
							list_info->item_no++;

							/* Make the "bullet point" bit */
							width = strlen(make_list_point(temp, list_info));

							/*
							 * If this is wider than the list at
							 * the moment, increase the width of the list.
							 */
							if (width > list_info->width)
							{
								list_info->width = width;
							}
						}
						break;
					}

					case TAG_CLOSE:
					{
						if (my_stricmp(buffer, "list") == 0)
						{
							/* If we aren't in "this" list */
							if (list_no > 0)
							{
								/* Keep track of where we are */
								list_no--;
							}
							else
							{
								/* done */
								return (TRUE);
							}
						}
						break;
					}
				}
				break;
			}
		}
	}

	/* We shouldn't reach end-of-file */
	sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
	return (FALSE);
}

/* ------------------------------------------------------------ ajps ---
 * Here we do a preparse pass over the table section, in order to
 * collect the widths of table columns (if they've been specified).
 * --------------------------------------------------------------------- */
bool preparse_table(BULP *bptr, int *table_info, char *err_message)
{
	int chunk_type;
	size_t size;
	int in_cell=0;
	int in_row=0;
	int cell_number=0;
	int table_no = 0;

	/* While in the table */
	while (!bulp_feof(bptr))
	{
		chunk_type = xmlbulp_get_next_bit(bptr, &size);

		switch (chunk_type)
		{
			/* fatal errors returned by xmlbulp_get_next_bit */
			case GETPOS_FAILED:
			case TYPE_EOF:
			{
				sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
				return (FALSE);
				break;
			}

			/* We're not dealing with these at present. */
			case TYPE_COMMENT:
			case TYPE_DOCTYPE:
			case TYPE_ENTITYDEF:
			case TYPE_TEXT:
			case TYPE_CDATA:
			case TYPE_CHARENTITY:
			{
				/* We care little for such things */
				xmlbulp_skip_chunk(bptr, size);
				break;
			}

			case TYPE_TAG:
			{
				int tag_type;
				char buffer[REASONABLE_LINELENGTH];

				/* Get the tag */
				tag_type = xmlbulp_get_tag_and_type(bptr, size, buffer);

				switch (tag_type)
				{
					case TAG_OPEN:
					{
						/* Keep track of the table level we are at */
						if (my_stricmp(buffer, "table") == 0)
						{
							table_no++;
						}

						/* If we are in "this" table and a row is opened */
						if (table_no == 0 && !in_row &&
							my_stricmp(buffer, "row") == 0)
						{
							/* Mark that we are in a row */
							in_row = TRUE;

							/* Reset the cell number for a new row */
							cell_number=0;
						}

						/* If we have found a cell opening and it is allowed */
						if (table_no == 0 &&
							 in_row &&
							 !in_cell &&
							  my_stricmp(buffer, "cell") == 0)
						{
							/* Mark that we are in a cell */
							in_cell=TRUE;


							/* We've strayed out of bounds */
							if (table_info[cell_number] == -1)
							{
								sprintf(err_message, "A cell tag may have been closed without being opened");
								return (FALSE);
							}

							/* If there is no width stored for this column */
							if (table_info[cell_number] == 0)
							{
								/* Store the width */
								table_info[cell_number] =
								get_percentage_width(buffer);
							}

							/* Increase this for the next cell. */
							cell_number++;
						}
						break;
					}

					case TAG_CLOSE:
					{
						/* If a row is closed and it is permitted */
						if (table_no == 0 && in_row &&
							my_stricmp(buffer, "row")==0)
						{
							/* note that we are not in a row any more */
							in_row = FALSE;
						}

						/* If a cell is closed and it is permitted */
						if (table_no == 0 && in_row &&
							my_stricmp(buffer, "cell") == 0)
						{
							in_cell=FALSE;
						}

						/* If a table is closed */
						if (my_stricmp(buffer, "table") == 0)
						{
							/* If it isn't "our" table */
							if (table_no > 0)
							{
								/* Note that it has closed */
								table_no--;
							}
							else
							{
								/* Done */
								return (TRUE);
							}
						}
						break;
					}
					break;
				}
			}
		}
	}

	/* We shouldn't reach end-of-file */
	sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
	return (FALSE);
}

/* ------------------------------------------------------------ ajps ---
 * Put a list header into the display block.
 * --------------------------------------------------------------------- */
bool add_list_data(char **display, list_blk *list_info)
{
	block_style header;

	/* Fill the block */
	header.width = list_info->width;
	header.align = ALIGN_RIGHT;
	header.lmargin = 0;
	header.rmargin = 0;

	/* Write the header to the display block */
	write_textblock_header(display, &header);

	/* make the list point */
	make_list_point((*display), list_info);

	/* Move the write position past it */
	(*display) += strlen(*display);

	/* Add end-of-block marker */
	(*display)[0]=REDRAW_CODE_BLOCKEND;
	(*display)++;

	/* width can be anything except 100, so that it won't confuse the plot */
	header.width = 99;

	/* Standard properties of this block */
	header.align = ALIGN_LEFT;
	header.lmargin = 0;
	header.rmargin = 0;

	/* Write header to the display block */
	write_textblock_header(display, &header);

	return (TRUE);
}

bool parse_list(BULP *bptr, char **display, char **links, int *link_no, int *block_no, char *list_tag, char *err_message)
{
	list_blk list_info;
	char *ptr;
	u32b file_pos;

#ifdef HELP_DEBUG
	{ void __swi(0x104) vdu4(void); vdu4(); }
	printf("parse list()\n");
#endif

	/* List header */
	(*display)[0]=REDRAW_CODE_BLOCKSTART;
	(*display)[1]=BLOCKTYPE_LIST;

	/* %age width */
	(*display)[2]=100;

	/* This doesn't really make a difference at the mo */
	(*display)[3]=ALIGN_LEFT;

	/* Default option */
	(*display)[4]=LIST_NOCOMPACT;

	/* 2 is the default width for the left hand column */
	list_info.width = 2;

	/* We start off with no items in the list */
	list_info.item_no = 0;

	/* Bullet point is default */
	list_info.type = LIST_TYPE_BULLET;

	/* Check if a compact state has been specified */
	ptr = xmlbulp_check_attr(list_tag, "compact");

	if (ptr != NULL)
	{
		if (my_stricmp(ptr, "yes") == 0 ||
			 my_stricmp(ptr, "true") == 0)
		{
			(*display)[4]=LIST_COMPACT;
		}
	}

	/* Check if a list type has been chosen */
	ptr = xmlbulp_check_attr(list_tag, "type");

	if (ptr != NULL)
	{
		switch (ptr[0])
		{
			case '-':
			{
				list_info.type = LIST_TYPE_BULLET;
				break;
			}

			case 'i':
			{
				list_info.type = LIST_TYPE_i;
				break;
			}

			case 'I':
			{
				list_info.type = LIST_TYPE_I;
				break;
			}

			case 'a':
			{
				list_info.type = LIST_TYPE_a;
				break;
			};

			case 'A':
			{
				list_info.type = LIST_TYPE_A;
				break;
			}

			case '1':
			{
				list_info.type = LIST_TYPE_1;
				break;
			}
		}
	}

	/* Check if a start number was specified */
	ptr = xmlbulp_check_attr(list_tag, "start");

	if (ptr != NULL)
	{
		/* Do a simple string conversion */
		list_info.item_no = atoi(ptr) - 1;
	}

	/* Store the file position before the preparse */
	bulp_getpos(bptr, &file_pos);

	/* We need to preparse the list if it has numbers, letters, etc. */
	if (list_info.type != LIST_TYPE_BULLET)
	{
		/* We need to recall this later. */
		int start_no = list_info.item_no;

		/* Go and gather width information on the item numbers */
		preparse_list(bptr, &list_info, err_message);

		/* Set the file position to where it was before */
		bulp_setpos(bptr, &file_pos);

		/* And likewise the start number */
		list_info.item_no = start_no;
	}

	/* We leave an extra space for presentation */
	list_info.width++;

	/* Move past the header now - we're done with it */
	(*display)+=5;

	/* Call parse_body again to parse the list contents */
	if (parse_body(bptr, display, links, link_no, block_no, FALSE, NULL,
					&list_info, err_message) == FALSE)
	{
		/* Just pass on the error message */
		return (FALSE);
	}
	else
	{
		/* List block termination code */
		(*display)[0]=0;
		(*display)++;

		return (TRUE);
	}
}

/* ------------------------------------------------------------ ajps ---
 * We need to know the number of columns and the width of each.	 We then
 * need to store this and retrieve it when we start a new cell.
 * --------------------------------------------------------------------- */
bool parse_table(BULP *bptr, char **display, char **links, int *link_no, int *block_no, char *table_tag, char *err_message)
{
	int *table_info = NULL;
	char *ptr = table_tag;
	int no_cols = -1;
	int n, total;
	u32b file_pos;
	int no_empty_cols = 0;

#ifdef HELP_DEBUG
	{ void __swi(0x104) vdu4(void); vdu4(); }
	printf("parse table()\n");
#endif

	/* Write the block header */
	(*display)[0] = REDRAW_CODE_BLOCKSTART;
	(*display)[1] = BLOCKTYPE_TABLE;

	/* Try to extract a width from the table tag */
	(*display)[2] = get_percentage_width(table_tag);

	/* If there was no width, width = 100% */
	if ((*display)[2] == 0) (*display)[2] = 100;

	/* Standard defaults for a table */
	(*display)[3]=ALIGN_LEFT;
	(*display)[4]=FLAG_TABLE_NORULES;

	/* Check if an alignment was specified */
	ptr = xmlbulp_check_attr(table_tag, "align");

	if (ptr != NULL)
	{
		/* Extract the alignment */
		(*display)[3] = set_alignment(ptr);
	}

	/* Check if any rules were specified */
	ptr = xmlbulp_check_attr(table_tag, "rules");

	if (ptr != NULL)
	{
		if (my_stricmp(ptr, "all") == 0) (*display)[4] = FLAG_TABLE_ALLRULES;
		if (my_stricmp(ptr, "rows") == 0) (*display)[4] = FLAG_TABLE_HRULES;
		if (my_stricmp(ptr, "cols") == 0) (*display)[4] = FLAG_TABLE_VRULES;
		if (my_stricmp(ptr, "none") == 0) (*display)[4] = FLAG_TABLE_NORULES;
	}


	/* Check for the number of columns.  This is a required tag. */
	ptr = xmlbulp_check_attr(table_tag, "cols");
	if (ptr != NULL)
	{
		no_cols = atoi(ptr);
	}
	else
	{
		sprintf(err_message, "A table tag must specify a number of columns.");
		return (FALSE);
	}

	if (no_cols <= 0)
	{
		sprintf(err_message, "The number of columns specified is bizarre.");
		return (FALSE);
	}


	/* Skip past the header now, we're done */
	(*display)+=5;

	/*
	 * We know the number of columns, so now we claim memory
	 * for a table block, and initialise all entries as zero.
	 */
	table_info = C_ZNEW(no_cols + 1, int);

	/* Mark the end of table_info */
	table_info[no_cols] = -1;

	/* We come back here later */
	bulp_getpos(bptr, &file_pos);

	/* read in the specified widths */
	if (preparse_table(bptr, table_info, err_message) == FALSE)
	{
		KILL(table_info);
		return (FALSE);
	}

	/* jump back to the top */
	bulp_setpos(bptr, &file_pos);

	/* Add up space used, and number of non-specified widths */
	for (total=0, n=0; n < no_cols; n++)
	{
		total += table_info[n];

		/*
		 * We don't want a cell that wide - it's crazy!
		 * this makes the next section clear the widths.
		 */
		if (table_info[n] >= 100) total = 1000;

		/* Count columns without a width specified */
		if (table_info[n] == 0) no_empty_cols++;
	}

	/* So they think they can mess with us, eh? */
	if (total > 100)
	{
		/* Reasonable fall-back position, ensures equal widths for columns */
		for (n = 0; n < no_cols; n++)
		{
			table_info[n] = 0;
		}

		no_empty_cols = n;

		total = 0;
	}

	/* Fill in the gaps */
	for (n = 0; n < no_cols; n++)
	{
		/* If we have no width already */
		if (table_info[n] == 0)
		{
			/* Share out the space left equally */
			table_info[n] = (100 - total) / no_empty_cols;
		}
	}

	/* Make sure they add up to 100 */
	for (total = 0, n = 0; n < no_cols; n++)
	{
		total += table_info[n];
	}

	/* We always boost the last column if there are bits left over */
	if (total != 100) table_info[no_cols-1] += (100-total);

	/* Then we go back to parse_body to get the parsing done */
	if (parse_body(bptr, display, links, link_no, block_no, FALSE,
					table_info, NULL, err_message) == FALSE)
	{
		/* Free the memory used */
		KILL(table_info);

		/* return the error */
		return (FALSE);
	}
	else
	{
		/* Write the end-of-block code */
		(*display)[0]=0;

		/* Move the write position past it */
		(*display)++;

		/* Free the memory we used to create the table */
		KILL(table_info);

		return (TRUE);
	}
}

/* ------------------------------------------------------------ ajps ---
 * We do the actual parsing here, filling up display and links as
 * appropriate, and recursing with parse_table & parse_list if we come
 * across one.
 *
 * Complications:  We only pass table_info or list_info if we are
 * specifically parsing one or the other.  Otherwise we should pass NULL.
 * This allows us to pick up on parse errors we would otherwise miss.
 *
 * One thing that could be added is a proper nested colour tag, so that
 * we change back to the correct colour when it is closed.
 * --------------------------------------------------------------------- */
bool parse_body(BULP *bptr, char **display, char **links, int *link_no, int *block_no, bool block_change, int *table_info, list_blk *list_info, char *err_message)
{
	/*
	 * This has to be big enough to contain a whole tag or char entity,
	 * but the text is read in in stages, so the size shouldn't matter
	 * for that.
	 */
	char buffer[REASONABLE_LINELENGTH];

	/* Stores the type of the chunk being read */
	int chunk_type;

	/* Whether we should read text in or not.  Incremental */
	int gather_text = 0;

	/* The size of the chunk we are reading in */
	size_t size;

	/* only used for tables */
	int cell_number = -2;

	/* Set TRUE when we are to read in text as formatted in the file. */
	int prefab = FALSE;

	/* The following are to detect parse errors */

	/* The number of tags we have open - the level of tag we are reading */
	int level = 0;

	/* In paragraph test */
	bool in_p = FALSE;

	/* In list item test */
	bool list_item = FALSE;

	/* In row test */
	bool row = FALSE;

#ifdef HELP_DEBUG
	{ void __swi(0x104) vdu4(void); vdu4(); }
	printf("parse body()\n");
#endif

	/* While there is file left */
	while (!bulp_feof(bptr))
	{
		chunk_type = xmlbulp_get_next_bit(bptr, &size);

		switch (chunk_type)
		{
			/* fatal errors returned by xmlbulp_get_next_bit */
			case GETPOS_FAILED:
			case TYPE_EOF:
			{
				sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
				return (FALSE);
				break;
			}

			case TYPE_COMMENT:
			case TYPE_DOCTYPE:
			case TYPE_ENTITYDEF:
			{
				/* We care little for such things */
				xmlbulp_skip_chunk(bptr, size);
				break;
			}

			case TYPE_TEXT:
			{
				if (gather_text)
				{
					/*
					 * Fetched in stages, cont is the variable
					 * used to manage this
					 */
					int cont = 0;

					/* While there is text to read */
					while (cont != -1)
					{
						/* Empty the buffer */
						strcpy(buffer, "");

						/* If text is not preformatted */
						if (!prefab)
						{
							/* Read in text stripping newlines, etc. */
							cont = xmlbulp_get_text_chunk_st(bptr,
									size, cont, buffer, REASONABLE_LINELENGTH);

							/* If the text is just whitespace, nuke it */
							if (xml_whitespace(buffer))
							{
								strcpy(buffer, "");
							}
						}
						else
						{
							/* Read in text "as is" */
							cont = xmlbulp_get_text_chunk(bptr,
									size, cont, buffer, REASONABLE_LINELENGTH);
						}

						/* Copy this chunk into the redraw block */
						strcpy((*display), buffer);

						/* And move the write position past it */
						(*display) += strlen((*display));
					}
				}
				/* Not gathering text */
				else
				{
					xmlbulp_skip_chunk(bptr, size);
				}
				break;
			}

			/* XML-standard tag, treat all data inside  as plain text */
			case TYPE_CDATA:
			{
				if (gather_text)
				{
					int cont = 0;

					/* the read function returns -1 when it is finished */
					while (cont != -1)
					{
						/* Empty buffer */
						strcpy(buffer, "");

						/* Get the next chunk of text */
						cont = xmlbulp_get_CDATA_chunk(bptr, size, cont,
											buffer, REASONABLE_LINELENGTH);

						/* copy text to the redraw block */
						strcpy((*display), buffer);

						/*	move write position past text  */
						(*display) += strlen((*display));
					}
				}
				/* Not gathering text */
				else
				{
					xmlbulp_skip_chunk(bptr, size);
				}
				break;
			}

			/* Any character entity , eg. &amp; */
			case TYPE_CHARENTITY:
			{
				/* Only bother is we are gathering text */
				if (gather_text)
				{
					int entity_type;

					/* Read in information from the file */
					entity_type = xmlbulp_get_char_entity(bptr, size, buffer);

					/* if the entity is a name eg. &lt; */
					if (entity_type == CHARENTITY_NAMED)
					{
						/* Add character to the redraw block */
						(*display)[0] =	xmlbulp_get_named_charentity(buffer);

						if ((*display)[0] == 0)
						{
							sprintf(err_message, "Named entity not recognised: '%s'.", buffer);
							return (FALSE);
						}
					}
					/* If the entity is numbered, eg. &#38; */
					else
					{
						/* Add character to the redraw block */
						(*display)[0] =	xmlbulp_get_unicode_charentity(buffer);
					}

					/* Move write position on */
					(*display)++;
				}
				/* Not gathering text */
				else
				{
					xmlbulp_skip_chunk(bptr, size);
				}
				break;
			}

			case TYPE_TAG:
			{
				int tag_type;

				if (size > REASONABLE_LINELENGTH)
				{
					sprintf(err_message, "A tag was abnormally large (more then %i characters).", REASONABLE_LINELENGTH);
					return (FALSE);
				}

				/* Get tag information from file */
				tag_type = xmlbulp_get_tag_and_type(bptr, size, buffer);

				/* choose action depending on type */
				switch (tag_type)
				{
					case TAG_OPEN:
					{
						/*
						 * If we are on the "base level" and allowed to
						 * alter the block number, we do this when a
						 * new tag is opened.
						 */
						if (level == 0 && block_change)
						{
							(*block_no)++;
						}

						/* We always increment level when a tag is opened */
						level++;

						/* If we have a table tag */
						if (my_stricmp(buffer, "table") == 0)
						{
							/* Parse the table */
							if (parse_table(bptr,
											 display,
											 links,
											 link_no,
											 block_no,
											 buffer,
											 err_message)
								  == FALSE)
							{
								return (FALSE);
							}

							/* The tag is closed if the function has returned */
							level--;

							break;
						}

						/* if we have a list tag */
						if (my_stricmp(buffer, "list") == 0)
						{
							/* Parse the list */
							if (parse_list(bptr,
											display,
											links,
											link_no,
											block_no,
											buffer,
											err_message)
								== FALSE)
							{
								return (FALSE);
							}

							/* the tag is closed if parse_list has returned */
							level--;

							break;
						}

						/* Ony deal with the item if we aren't in one */
						if (my_stricmp(buffer, "item") == 0 && !list_item)
						{
							if (list_info != NULL)
							{
								/* Remember we are in a tag */
								list_item = TRUE;

								/* increase the number of list items */
								list_info->item_no++;

								/* Read in and add the list data */
								add_list_data(display, list_info);

								/* We want to gather text */
								gather_text++;
							}
							else
							{
								sprintf(err_message, "An item tag was opened outside of any list.");
								return (FALSE);
							}
							break;
						}

						/* Row tag */
						if (my_stricmp(buffer, "row") == 0)
						{
							/* If we are allowed to open a row */
							if (table_info != NULL && row == FALSE)
							{
								row = TRUE;

								/* Reset the cell number */
								cell_number = -1;
							}
							else
							{
								if (table_info != NULL)
									sprintf(err_message, "A row was opened outside a table.");

								if (row != FALSE)
									sprintf(err_message, "A row was opened inside another row.");

								return (FALSE);
							}
							break;
						}

						if (my_stricmp(buffer, "cell") == 0)
						{
							/* If we are allowed to open a cell */
							if (table_info != NULL && row == TRUE)
							{
								/* Add one to the cell number */
								cell_number++;

								if (table_info[cell_number] == -1)
								{
									sprintf(err_message, "There are more columns than was claimed in the table tag.");
									return (FALSE);
								}
								else
								{
									/* If valid, add a paragraph header */
									add_paragraph(display, buffer,
												table_info[cell_number]);

									/* We want to gather text */
									gather_text++;
								}
							}
							else
							{
								if (table_info == NULL)
									sprintf(err_message, "A cell has been opened outside a table.");

								if (row == FALSE)
									sprintf(err_message, "A cell has been opened outside a row.");

								return (FALSE);
							}
							break;
						}

						if (my_stricmp(buffer, "link") == 0)
						{
							/* Add data about the link into the redraw block */
							add_linkptra(display,
											links,
											link_no,
											block_no,
											buffer);
							break;
						}

						if (my_stricmp(buffer, "p") == 0)
						{
							if (in_p)
							{
								sprintf(err_message, "A paragraph tag was opened inside another.  This is not permitted.");
								return (FALSE);
							}

							/* Remember we are in a paragraph */
							in_p = TRUE;

							/* Add a paragraph header, width 100% */
							add_paragraph(display, buffer, 100);

							/* We gather text inside paragraphs */
							gather_text++;

							break;
						}

						if (my_stricmp(buffer, "pre") == 0)
						{
							if (prefab)
							{
								sprintf(err_message, "A pre tag was opened inside another.");
								return (FALSE);
							}
							else
							{
								/* All text read in now should be pre-formed */
								prefab = TRUE;
							}
							break;
						}

						if (my_stricmp(buffer, "em") == 0)
						{
							/* Add an emph redraw code */
							(*display)[0] = REDRAW_CODE_EMPH;
							(*display)++;
							break;
						}

						if (my_stricmp(buffer, "strong") == 0)
						{
							/* Add a strong redraw code */
							(*display)[0] = REDRAW_CODE_STRONG;
							(*display)++;
							break;
						}

						if (my_stricmp(buffer, "colour") == 0 ||
							my_stricmp(buffer, "color") == 0)
						{
							/* Add a colour change code. */
							add_colour(display, buffer);
							break;
						}

						if (my_stricmp(buffer, "h1") == 0 ||
							 my_stricmp(buffer, "h2") == 0 ||
							  my_stricmp(buffer, "h3") == 0)
						{
							/* Add the various header codes to the redraw */
							add_head_data_open(display, buffer);

							/* A header is a valid block - gather text */
							gather_text++;

							break;
						}

						if (my_stricmp(buffer, "br") == 0)
						{
							/* Just insert a newline character */
							(*display)[0]='\n';
							(*display)++;
							break;
						}

						if (my_stricmp(buffer, "hr") == 0)
						{
							/* Add horizontal rule */
							add_hrule(display, buffer, 100);

							break;
						}

						break;
					}

					case TAG_COMPLETE:
					{
						/* A complete tag can still be a block */
						if (level == 0 && block_change)
						{
							(*block_no)++;
						}

						/* We can fill in an error message in place of this tag */
						if (my_stricmp(buffer, "err") == 0)
						{
							strcpy((*display), err_message);
							(*display) += strlen(err_message);

							break;
						}

						/* A self-contained link */
						if (my_stricmp(buffer, "link") == 0)
						{
							add_linkptra(display,
											links,
											link_no,
											block_no,
											buffer);

							/* It DOESN't constitute a block */
							if (level == 0) (*block_no)--;

							break;
						}

						/* line break */
						if (my_stricmp(buffer, "br") == 0)
						{
							(*display)[0]='\n';
							(*display)++;

							break;
						}

						if (my_stricmp(buffer, "hr") == 0)
						{
							/* Add horizontal rule */
							add_hrule(display, buffer, 100);

							/* Mark end of block */
							(*display)[0] = REDRAW_CODE_BLOCKEND;
							(*display)++;

							break;
						}

						break;
					}

					case TAG_CLOSE:
					{
						/* Always drop back a level when a tag closes */
						level--;

						if (my_stricmp(buffer, "item") == 0 && list_item)
						{
							/* No longer in an item */
							list_item = FALSE;

							/* Mark the end of the list item */
							(*display)[0] = REDRAW_CODE_BLOCKEND;
							(*display)++;

							/* Don't gather text */
							gather_text--;

							break;
						}

						if (my_stricmp(buffer, "row") == 0)
						{
							if (table_info != NULL && row == TRUE)
							{
								/* No longer in a row */
								row = FALSE;
							}
							else
							{
								if (table_info == NULL)
									sprintf(err_message, "A row was closed outside a table.");

								if (row == FALSE)
									sprintf(err_message, "A row was closed without one being opened.");

								return (FALSE);
							}
							break;
						}

						if (my_stricmp(buffer, "cell") == 0)
						{
							if (table_info != NULL && row == TRUE)
							{
								/* Add a end-of-cell marker */
								(*display)[0] = REDRAW_CODE_BLOCKEND;
								(*display)++;

								/* Don't gather text */
								gather_text--;
							}
							else
							{
								if (table_info == NULL)
									sprintf(err_message, "A cell was closed outside a table.");

								if (row == FALSE)
									sprintf(err_message, "A cell was closed without being in a row.");

								return (FALSE);
							}
							break;
						}

						if (my_stricmp(buffer, "pre") == 0)
						{
							if (!prefab)
							{
								sprintf(err_message, "A pre tag was closed without being opened.");
								return (FALSE);
							}
							else
							{
								/* Stop reading things in leterally */
								prefab = FALSE;
							}
							break;
						}

						if (my_stricmp(buffer, "hr") == 0)
						{
							/* Mark end of block */
							(*display)[0] = REDRAW_CODE_BLOCKEND;
							(*display)++;

							break;
						}

						if (my_stricmp(buffer, "p") == 0)
						{
							/* Mark end of block */
							(*display)[0] = REDRAW_CODE_BLOCKEND;
							(*display)++;

							/* Stop gathering text */
							gather_text--;

							/* no longer in a paragraph */
							in_p = FALSE;

							break;
						}

						if (my_stricmp(buffer, "link") == 0)
						{
							/* These two bytes mark the end of a link */
							(*display)[0] = REDRAW_CODE_LINKREF;
							(*display)[1] = 0;

							(*display) += 2;

							break;
						}

						if (my_stricmp(buffer, "h1") == 0 ||
							my_stricmp(buffer, "h2") == 0 ||
							my_stricmp(buffer, "h3") == 0)
						{
							/* Close the header block */
							add_head_data_close(display, buffer);

							/* Stop gatherng text */
							gather_text--;

							break;
						}

						if (my_stricmp(buffer, "em") == 0)
						{
							/* Toggle emphasis off */
							(*display)[0] = REDRAW_CODE_EMPH;
							(*display)++;

							break;
						}

						if (my_stricmp(buffer, "strong") == 0)
						{
							/* Toggle strong off */
							(*display)[0] = REDRAW_CODE_STRONG;
							(*display)++;

							break;
						}

						if (my_stricmp(buffer, "colour") == 0 ||
							my_stricmp(buffer, "color") == 0)
						{
							/* Set colour back to white */
							(*display)[0] = REDRAW_CODE_COLOUR;
							(*display)[1] = TERM_WHITE;

							(*display) += 2;

							break;
						}

						/*
						 * We're done, we can leave now, unless
						 * we are still inside a table block, in
						 * which case we are still done, but have
						 * failed.
						 */
						if (my_stricmp(buffer, "body") == 0)
						{
							if (table_info == NULL && list_info == NULL)
							{
								/* Mark the end of the whole redraw block */
								(*display)[0] = REDRAW_CODE_BLOCKEND;

								return (TRUE);
							}
							else
							{
								if (table_info != NULL)
									sprintf(err_message, "Reached the </body> tag without closing a table.");

								if (list_info != NULL)
									sprintf(err_message, "Reached the </body> tag without closing a list.");

								return (FALSE);
							}
							break;
						}

						/*
						 * If a table has been closed, and we were told
						 * to parse a table, then we return TRUE.
						 * Otherwise, we return FALSE, because of the error.
						 */
						if (my_stricmp(buffer, "table") == 0)
						{
							if (table_info != NULL)
							{
								/* Mark the end of the table */
								(*display)[0] = REDRAW_CODE_BLOCKEND;

								return (TRUE);
							}
							else
							{
								if (table_info == NULL)
									sprintf(err_message, "A table was closed that wasn't opened.");

								return (FALSE);
							}
							break;
						}

						/*
						 * If a list has been closed, and we were told
						 * to parse a list, then we return TRUE.
						 * Otherwise, we return FALSE, because of the error.
						 */
						if (my_stricmp(buffer, "list") == 0)
						{
							if (list_info != NULL)
							{
								return (TRUE);
							}
							else
							{
								sprintf(err_message, "A list was closed that wasn't opened");
								return (FALSE);
							}
							break;
						}

						break;
					}
				}
				break;
			}
		}
	}

	/* We shouldn't reach this, really. */
	sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
	return (FALSE);
}

/* ------------------------------------------------------------ ajps ---
 * This does a pass across the <body> section.
 * This pass allows us to get an approximate size for the links_block by
 * adding up the space taken up by each link.  We can then claim a
 * reasonable amount of memory to hold the links.
 * --------------------------------------------------------------------- */
bool preparse_body(BULP *bptr, char **links_block, int *no_blocks, char *err_message)
{
	/*
	 * Minimum size of block, to hold size and an end marker
	 * This stops things getting confused if there are no links at all.
	 */
	int links_size=8;

	/* The file position at the start, to be returned to at the end */
	u32b start_pos;

	/* Are we finished yet? */
	bool done=FALSE;

	/* Type of the chunk being read in */
	int chunk_type;

	/* The size of the chunk being read in */
	size_t size;

	/* The level we are at (for nested tags, like a table in a table, etc. */
	int level = 0;

	/* A buffer to hold any single tag, and smallish chunks of text. */
	char buffer[REASONABLE_LINELENGTH];

	/* We start inside the body tag, so this counts tags */
	int badly_formed = 1;

#ifdef HELP_DEBUG
	{ void __swi(0x104) vdu4(void); vdu4(); }
	printf("preparse body()");
#endif

	/* We are supposed to count the number of "big blocks" in the file */
	*no_blocks = 0;

	/* Store the start position of the file */
	bulp_getpos(bptr, &start_pos);

	/* While there are things to be done */
	while (!done && !bulp_feof(bptr))
	{
		chunk_type = xmlbulp_get_next_bit(bptr, &size);

		switch (chunk_type)
		{
			/* fatal errors returned by xmlbulp_get_next_bit */
			case GETPOS_FAILED:
			case TYPE_EOF:
			{
				sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
				return (FALSE);
				break;
			}

			/* Do a simple check for an un-closed entity, like just an & */
			case TYPE_CHARENTITY:
			{
				if (size == 0)
				{
					sprintf(err_message, "Character entity was strangely long.  Probably an & that should be an &amp;");
					return (FALSE);
				}
			}

			/* These aren't important in pre-parse */
			case TYPE_COMMENT:
			case TYPE_DOCTYPE:
			case TYPE_ENTITYDEF:
			case TYPE_TEXT:
			case TYPE_CDATA:
			{
				xmlbulp_skip_chunk(bptr, size);
				break;
			}

			/* This is important */
			case TYPE_TAG:
			{
				int tag_type;

				/* Extract the tag & attributes */
				tag_type = xmlbulp_get_tag_and_type(bptr, size, buffer);

				switch (tag_type)
				{
					case TAG_OPEN:
					{
						/* Note that we've opened a tag */
						badly_formed++;

						/* If we are outside of all tags except body */
						if (level == 0)
						{
							/* Increase the number of "big blocks" */
							(*no_blocks)++;
						}

						/* Note that we are now inside another tag */
						level++;

						if (my_stricmp(buffer, "link") == 0)
						{
							/*
							 * Add the size of a standard link block,
							 * and the size of the text in the tag,
							 * minus the characters we know are there:
							 * <link> (6)
							 */
							links_size += sizeof(link_blk) + size - 6;
						}
						break;
					}

					case TAG_COMPLETE:
					{
						/* If we are outside tags, note this is a "big block" */
						if (level == 0) (*no_blocks)++;

						if (my_stricmp(buffer, "link") == 0)
						{
							/* A link on the base level isn't a block */
							if (level == 0) (*no_blocks)--;

							/*
							 * Add the size of a standard link block,
							 * and the size of the text in the tag,
							 * minus the characters we know are there:
							 * <link /> (8)
							 */
							links_size += sizeof(link_blk) + size - 8;
						}
						break;
					}

					case TAG_CLOSE:
					{
						/* Note that a tag has been closed */
						badly_formed--;

						/* Note that we are now outside one more tag */
						level--;

						/* The body tag has been closed */
						if (my_stricmp(buffer, "body") == 0)
						{
							/* If more tags were opened than were closed */
							if (badly_formed > 0)
							{
								sprintf(err_message, "More tags were opened than were closed.");
								return (FALSE);
							}

							/* If more tags were closed than were opened. */
							if (badly_formed < 0)
							{
								sprintf(err_message, "More tags were closed than were opened.");
								return (FALSE);
							}

							/* Otherwise it's all OK */
							done=TRUE;
						}
						break;
					}
				}
				break;
			}
		}
	}

	/* If we aren't done, we shouldn't be here. */
	if (!done)
	{
		sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
		return (FALSE);
	}

	/* Allocate the estimated amount of memory for the links block */
	*links_block = (char *) ralloc(links_size);

	/* If the allocation failed... */
	if (*links_block == NULL)
	{
		sprintf(err_message, "Couldn't allocate memory to hold link information.");
		return (FALSE);
	}

	/*
	 * Set the first int in the links block to be the size of the block.
	 * This is so that it can be rnfree()d properly - we nominally need
	 * to supply the size of the memory we wish to be freed.
	 */
	*((int *) *links_block) = links_size;

	/* Go back to start of file */
	bulp_setpos(bptr, &start_pos);

	/* Pre-parse completed successfully. */
	return (TRUE);
}

/* ------------------------------------------------------------ ajps ---
 * This should parse the title of the file, the bit enclosed by <title>
 * tags.
 *
 * When the function exits, the title will be zero-terminated, at the
 * start of display_block.
 *
 * Note: no tags are allowed inside <title></title>, though character
 * entities (eg. &quot;) are.  There shouldn't be any newlines, either.
 * --------------------------------------------------------------------- */
int parse_title(BULP *bptr, char *display_block, char *err_message)
{
	/* Size of the chunk being read */
	size_t size;

	/* Type of the chunk */
	int chunk_type;

	/* A buffer capable of holding any single tag in the file */
	char buffer[REASONABLE_LINELENGTH]="";

	/* Read in chunk information */
	chunk_type = xmlbulp_get_next_bit(bptr, &size);

	switch (chunk_type)
	{
		/* fatal errors returned by xmlbulp_get_next_bit */
		case GETPOS_FAILED:
		case TYPE_EOF:
		{
			sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
			return (FALSE);
			break;
		}

		/* We care little for such things */
		case TYPE_COMMENT:
		case TYPE_DOCTYPE:
		case TYPE_ENTITYDEF:
		{
			xmlbulp_skip_chunk(bptr, size);
			break;
		}

		/* Deal with an entity */
		case TYPE_CHARENTITY:
		{
			int entity_type;

			/* the block length */
			int b_len = strlen(display_block);

			entity_type = xmlbulp_get_char_entity(bptr, size, buffer);

			if (entity_type == CHARENTITY_NAMED)
			{
				/* Add the character to the end of the title */
				display_block[b_len] = xmlbulp_get_named_charentity(buffer);

				/* And zero-terminate it */
				display_block[b_len + 1] = 0;
			}
			else
			{
				/* Add the character to the end of the title */
				display_block[b_len] = xmlbulp_get_unicode_charentity(buffer);

				/* And zero-terminate it */
				display_block[b_len+1] = 0;
			}
			break;
		}

		/* Deal with ordinary text */
		case TYPE_TEXT:
		{
			int cont = 0;

			/* cont will equal -1 when all the text in this chunk's been read */
			while (cont != -1)
			{
				/* Empty the buffer */
				strcpy(buffer, "");

				/* Get the chunk and continuation point. */
				cont = xmlbulp_get_text_chunk_st(bptr, size, cont, buffer, REASONABLE_LINELENGTH);

				/* Add the buffer contents to the title */
				strcat(display_block, buffer);
			}
			break;
		}

		case TYPE_CDATA:
		{
			int cont = 0;

			/* cont will equal -1 when all the text in this chunk's been read */
			while (cont != -1)
			{
				/* Empty the buffer */
				strcpy(buffer, "");

				/* Get the chunk and continuation point. */
				cont = xmlbulp_get_CDATA_chunk(bptr, size, cont, buffer, REASONABLE_LINELENGTH);

				/* Add the buffer contents to the title */
				strcat(display_block, buffer);
			}
			break;
		}

		case TYPE_TAG:
		{
			int tag_type;

			tag_type = xmlbulp_get_tag_and_type(bptr, size, buffer);

			switch (tag_type)
			{
				case TAG_OPEN:
				case TAG_COMPLETE:
				{
					/* No tags are allowed inside <title>, we ignore them */
					break;
				}

				case TAG_CLOSE:
				{
					/* We're done, carry on with other things */
					if (my_stricmp(buffer, "title") == 0) return (TRUE);

					break;
				}
			}
			break;
		}
	}

	/* Strangely, we've looped this externally. This means we've finished. */
	return (-1);
}

/* ------------------------------------------------------------ ajps ---
 * This does the actual parsing of the xml file into a block suitable
 * for passing to the display functions.
 *
 * This uses the xmlbulp library, meaning that all stepping through the
 * XML file is done as if it were in-file - it may be, or it may be
 * in-memory, depending on how the file is opened.
 *
 * It should be noted that the size of the display block is currently set
 * to the size of the input file.  This is a simple approzimation, and
 * will normally mean that the display block is at least 10% too big.
 * However, we can be certain that the parsed file will be smaller than
 * the source, but not of anything else, so we persist anyway.
 * --------------------------------------------------------------------- */
bool parse_helpfile(BULP *help_file, char **display_blk, char **links_blk, int *no_blocks, char *err_message)
{
	/* Type of chunk being read in */
	int chunk_type;

	/* Are we inside the ahml tag? */
	bool ahml = FALSE;

	/* The size of the display block = size of input file */
	int display_size;

	/* We start at link number 1, as zero marks the end of a link */
	int link_no = 1;

	/* We start at "big block" zero */
	int block_no = 0;

	/* The size of the chunk being read in */
	size_t size = 0;

	/* The display block write position */
	char *display = NULL;

	/* The links block write position */
	char *links = NULL;

	/* Are we looping? */
	bool looping = TRUE;

	/* This has to be big enough to hold any tag */
	char buffer[REASONABLE_LINELENGTH];

#ifdef HELP_DEBUG
	{ void __swi(0x104) vdu4(void); vdu4(); }
	printf("parse helpfile()");
#endif

	/* Get the size of the file, and set the size of the display block. */
	display_size = bulp_size(help_file);

	/* We unset this when we are done */
	while (looping)
	{
		/* Get text chunk */
		chunk_type = xmlbulp_get_next_bit(help_file, &size);

		if (chunk_type == TYPE_TAG)
		{
			int tag_type;

			/* There's no way the first line should be so long! */
			if (size > REASONABLE_LINELENGTH) continue;

			/* Get more information about the tag */
			tag_type = xmlbulp_get_tag_and_type(help_file, size, buffer);

			/* If it is the xml declaration, we can continue in peace */
			if (tag_type == TAG_XML)
			{
				/*
				 * We don't really use the information in this tag (such
				 * as charset, etc.), but /shouldn't/ just ignore it
				 * We might use it some time later?
				 */
				looping = FALSE;
			}
		}
		/* Anything other than a tag */
		else
		{
			/* Just check for a terminal error */
			if (chunk_type == TYPE_EOF || chunk_type == GETPOS_FAILED)
			{
				sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
				return (FALSE);
			}

			/* skip the chunk - we don't really care. */
			xmlbulp_skip_chunk(help_file, size);
		}
	}

	/* Claim a suitable amount of memory for the display block! */
	*display_blk = ralloc(display_size);

	/* If it couldn't be claimed, return the error */
	if (*display_blk == NULL) return (FALSE);

	/*
	 * Store the size of the block in the first int, so that we can
	 * pass this to rnfree later.
	 */
	*((int *) *display_blk) = display_size;

	/* Set the write position to after the block size */
	display = (*display_blk + sizeof(int));

	/* Set a zero-terminator there so we can strcat() things to it. */
	display[0] = 0;

	/* While we are in the file */
	while (!bulp_feof(help_file))
	{
		chunk_type = xmlbulp_get_next_bit(help_file, &size);

		switch (chunk_type)
		{
			/* Terminal errors */
			case GETPOS_FAILED:
			case TYPE_EOF:
			{
				sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
				return (FALSE);
				break;
			}

			/*
			 * Until we are inside <ahml> and some other tag (like
			 * body or title), we ignore everything
			 */
			case TYPE_COMMENT:
			case TYPE_DOCTYPE:
			case TYPE_ENTITYDEF:
			case TYPE_CHARENTITY:
			case TYPE_TEXT:
			case TYPE_CDATA:
			{
				xmlbulp_skip_chunk(help_file, size);
				break;
			}

			case TYPE_TAG:
			{
				int tag_type;

				tag_type = xmlbulp_get_tag_and_type(help_file, size, buffer);

				switch (tag_type)
				{
					case TAG_OPEN:
					{
						if (my_stricmp(buffer, "ahml") == 0)
						{
							/* If it has already been opened */
							if (ahml)
							{
								sprintf(err_message, "ahml has been opened at least twice.");
								return (FALSE);
							}

							/* Set that we are now inside ahml */
							ahml = TRUE;
						}

						/* The title tag */
						if (my_stricmp(buffer, "title") == 0 && ahml)
						{
							int result = -1;

							/* We loop this here, returns -1 when done */
							while (result == -1 && !bulp_feof(help_file))
							{
								result = parse_title(help_file, display,
												err_message);

								/* If an error was returned, return an error */
								if (result == FALSE) return (FALSE);
							}
						}

						/* The body tag */
						if (my_stricmp(buffer, "body") == 0 && ahml)
						{
							/*
							 * Preparse the body to get the size
							 * of links_blk and the number of blocks.
							 */
							if (preparse_body(help_file,
											  links_blk,
											  no_blocks,
											  err_message) == FALSE)
							{
								return (FALSE);
							}

							/* Set the write position of the links block */
							links = *links_blk + sizeof(int);

							/*
							 * Mark the end of the links block.  This will
							 * be overwritten if links are added later.
							 */
							((link_blk *)links)->type = LINK_TYPE_END;

							/* Move the display write position past the title */
							display = display + WORDALIGN(strlen(display) + 1);

							/* Parse the body */
							if (parse_body(help_file,
											&display,
											&links,
											&link_no,
											&block_no,
											TRUE,
											NULL,
											NULL,
											err_message
											) == FALSE)
							{
								return (FALSE);
							}
						}
						break;
					}

					case TAG_CLOSE:
					{
						/* We're done with this file */
						if (my_stricmp(buffer, "ahml") == 0) return (TRUE);
						break;
					}
				}
				break;
			}
		}
	}

	sprintf(err_message, "The end of file was reached unexpectedly, so could not be parsed.");
	return (FALSE);
}

/* ------------------------------------------------------------ ajps ---
 * This plots the file to screen, starting from the first block that has
 * any text displayed within the boundaries, and finishing with the first
 * block that goes off the bottom.
 *
 * This should ensure a quick redraw however large and complicated the
 * file is (as long as it isn't one huge table).  On the other hand, the
 * redraw code is quick enough that (at the moment) it makes no real
 * difference.
 * --------------------------------------------------------------------- */
bool plot_help(char *display, block_blk *blocks, int from_line, char *links, int x, int y, int width, int height, char *err_message)
{
	int temp = 0;

	/* The number of lines printed (or virutally printed) */
	int line_count = 0;

	/* The start y position */
	int print_y = y - from_line;

	/* counter */
	int n = 0;

	/* The end line of the previous block */
	int prev_end_line = 0;

	/* While there are blocks to process */
	while (blocks[n].end_line != -1)
	{
		/* If the end line is on the screen */
  		if (blocks[n].end_line > from_line)
		{
			/* Set the block to display */
			display = blocks[n].start;

			/* And start to display it */
			break;
		}
		else
		{
			/* Store the end position of this block */
			prev_end_line = blocks[n].end_line;
		}

		/* Move on a block */
		n++;
	}

	/* Set the initial line count to the number of lines "virtually" printed */
	line_count = prev_end_line;

	/* Set the print position to match */
	print_y = y - from_line + line_count;

	/* While there are blocks to print */
	while (display[0] != REDRAW_CODE_BLOCKEND)
	{
		/* Plot the block, store the number of lines produced in temp */
		temp = plot_helpfile_block(x,
								   print_y,
								   width,
								   height,
								   &display,
								   links,
								   from_line-line_count,
								   NULL);

		/* Returned if there is a freakish plot error */
		if (temp == PLOT_FAILED)
		{
			sprintf(err_message, "There was a bizarre plotting failure.  Sorry.");
			return (FALSE);
		}

		/* Adds spacing line between the blocks */
		temp++;

		/* Move the line count on to take account of the block printed */
		line_count += temp;

		/* Move the print position to the current place */
		print_y = y - from_line + line_count;

		/* If we are off the end of the screen, stop plotting */
		if (print_y >= (height + y)) return (TRUE);
	}

	return (TRUE);
}

/* ------------------------------------------------------------ ajps ---
 * This fills an array of block_blks with the location in memory of each
 * major redraw block, and the line it finishes on.
 *
 * This is to cut down on redraw times when moving through a file, and
 * to enable link points inside a page to work effectively.
 *
 * We now also gather information on the position of links on the page,
 * so that highlighting of links can be limited to those on the page -
 * if required.
 * --------------------------------------------------------------------- */
bool get_block_info(char *display, int width, block_blk *blocks, char *links, char *err_message)
{
	/* This is used to hold the number of lines returned by the plotting function */
	int temp = 0;

	/* This is the running total of lines in the file */
	int line_count = 0;

	/* This is the block number - which block we are plotting */
	int block_no = 0;

	/* We need to remember this to add information to the links block */
	int print_y = 0;

	/* Set cur_link to point to the first link in the block */
	link_blk *cur_link = (link_blk *)(links);

	/* 	Make sure that we are pointing to a link reference, not a mark point */
	while (cur_link->type != LINK_TYPE_END)
	{
		/* We do have a link ref, carry on with the rest of the function */
		if (cur_link->type >= LINK_TYPE_REF) break;

		/* Move onto the next link in the block */
		cur_link = cur_link->next;
	}

	/* We have a REDRAW_CODE_BLOCKEND as the last character in the block */
	while (display[0] != REDRAW_CODE_BLOCKEND)
	{
		/*
		 * We've reached the end of the block to store information
		 * before we've run out of blocks.  This shouldn't happen.
		 */
		if (blocks[block_no].end_line == -1)
		{
			sprintf(err_message, "Strange problem.  The number of redraw blocks was incorrectly calulated.");
			return (FALSE);
		}

		/* Store the memory position where this block starts */
		blocks[block_no].start = display;

		/* Set print_y to be the current total of lines in the file */
		print_y = line_count;

		/*
		 * Use plot_helpfile_block to work out how many lines this block will
		 * take up when printed on screen.  It returns that value, and as we
		 * are also passing the cur_link pointer, it will fill the links block
		 * with the position on screen of each link as it comes across it.
		 */
		temp = plot_helpfile_block(0,
								   print_y,
								   width,
								   0,
								   &display,
								   links,
								   0-line_count,
								   &cur_link);

		/* Some strange error happened in the plot_helpfile_block function */
		if (temp == PLOT_FAILED)
		{
			sprintf(err_message, "Strange plotting problem.  Sorry.");
			return (FALSE);
		}

		/*
		 * Adds a single spacing line after a block,
		 * if it isn't the last block in the memory.
		 */
		if (display[0] != REDRAW_CODE_BLOCKEND) temp++;

		/* We update the running total with the size of this block */
		line_count += temp;

		/* Record the blocks (end) position next to its memory position */
		blocks[block_no].end_line = line_count;

		/* Move onto the next block. */
		block_no++;
	}

	/* It's all gone as planned. */
	return (TRUE);
}


/* ------------------------------------------------------------ ajps ---
 * Moves a file from current location (start) to another location (end)
 * in the stack, shuffling the other entries up or down to fill up the
 * space left, and make space for it at the destination end.
 * --------------------------------------------------------------------- */
void shuffle_stack(history_blk *stack, int end, int start)
{
	/* Counter */
	int n;

	/* The direction we are moving things in */
	int direction;

	/* A temporary store while we make space */
	history_blk temp;

	/* Store the current start entry */
	temp = stack[start];

	/* Work out the direction */
	direction = (end - start) / abs(end - start);

	/* Shift the entries from start to end */
	for (n = start; abs(n - end) > 0; n += direction)
	{
		stack[n] = stack[n + direction];
	}

	/* Place the entry that was at the start at the end */
	stack[end] = temp;

	return;
}

/* ------------------------------------------------------------ ajps ---
 * This puts the file that was at the top of the stack to the bottom.
 * When we leave a page by selecting "back" rather than a specified file,
 * we want to move the page we just left to the bottom, and move to the
 * one now at the top. (for now, anyway).
 *
 * Returns the location of the file that was at the top of the stack.
 * --------------------------------------------------------------------- */
history_blk *history_loop(history_blk *stack, int direction)
{
	/* Will equal the number of files in the stack */
	int n = 0;

	/* Holds a pointer to the current top-of-stack, before the loop */
	history_blk *prev_top = &stack[0];

	/* Count the files in the stack at present */
	while (n < HISTORY_DEPTH &&  strlen(stack[n].file) > 0) n++;

	/* If there is more than one file */
	if (n > 1)
	{
		/* Move from bottom to top */
		if (direction == -1)
		{
			/* Shuffle the bottom file to the top */
			shuffle_stack(stack, n - 1, 0);
		}

		/* Move from top to bottom */
		if (direction == +1)
		{
			/* Shuffle the top file to the bottom */
			shuffle_stack(stack, 0, n - 1);
		}
	}

	return (prev_top);
}

/* ------------------------------------------------------------ ajps ---
 * Adds a file to the history stack, or moves it to the top if it is
 * already there.
 * If necessary, it will chuck the bottom one off to make room.
 * --------------------------------------------------------------------- */
bool history_add(history_blk *stack, char *file, int line)
{
	int n;
	int current_position = -1;

	/* We can't store this in the history - reject it */
	if (strlen(file) > XMLHELP_FILENAME_LENGTH) return (FALSE);

	/* Search the history stack for the file being added */
	for (n=0; (n < HISTORY_DEPTH &&  strlen(stack[n].file) > 0); n++)
	{
		/* If the file is found */
		if (strcmp(stack[n].file, file) == 0)
		{
			/* Mark the curent position */
			current_position = n;

			/* And escape the loop */
			break;
		}
	}

	/* If the file is already in the stack */
	if (current_position > 0)
	{
		/* Move it to the top */
		shuffle_stack(stack, 0, current_position);

		/* But reset the line number to line */
		stack[0].line = line;
	}
	/* If the file is already at the top */
	else if (current_position == 0)
	{
		/* Return */
		return (TRUE);
	}
	/* The file isn't in the stack at all */
	else
	{
		/* If we have a full history stack */
		if (n == HISTORY_DEPTH)
		{
			/* Shuffle the bottom one to the top, to be wiped */
			shuffle_stack(stack, 0, HISTORY_DEPTH - 1);
		}
		else
		{
			/* Shuffle all entries down to make room at the top */
			if (n > 0) shuffle_stack(stack, 0, n);
		}

		/* Copy the filename in */
		strcpy((char *)(stack[0].file), file);

		/* And reset the line stored to line */
		stack[0].line = line;
	}

	/* All done, once more */
	return (TRUE);
}

/* ------------------------------------------------------------ ajps ---
 * Takes a file link (in file) and builds the appropriate path in buffer
 * --------------------------------------------------------------------- */
char *help_path_build(char *buffer, int buff_size, const char *file, const char *lastfile, history_blk *stack)
{
	char *file_ptr = NULL;

	/* Fill the buffer with the path to the "lib" directory */
	strncpy(buffer,ANGBAND_DIR, buff_size);

	/* This will give the path without the ANGBAND_DIR bit */
	file_ptr = buffer + strlen(buffer);

	/* We've been passed a full path relative to ANGBAND_DIR */
	if (strcmp(lastfile,"") == 0 || file[0] == '/')
	{
		if (file[0] == '/')
		{
			/* Snip the preceding '/' */
			strncat(buffer, file + 1, buff_size - strlen(buffer));
		}
		else
		{
			strncat(buffer, file, buff_size - strlen(buffer));
		}
	}
	/* Path relative to previous file */
	else
	{
		char *ptr = NULL;
		char *temp_path = NULL;
		const char *file_start = NULL;

		/* We take a copy so we can chop it around */
		temp_path = (char *) string_make(lastfile);

		file_start = file;

		/* Find last directory separator in path */
		ptr = strrchr(temp_path, '/');
                   
		/* There should always have been a directory there - try from root */
		if (ptr == NULL)
		{
			strncat(buffer, file, buff_size - strlen(buffer));
		}
		else
		{
			/* Chop off the file name, just leave the path. */
			ptr[0] = 0;

			/* Do we need to step back? */
			while (strncmp(file_start,"../",3) == 0)
			{   
				/* Find last directory separator in path */
				ptr = strrchr(temp_path, '/');

				if (ptr == NULL)
				{
					/* We've gone back too far - ignore it */
				}
				else
				{
					ptr[0] = 0;
				}

				/* Move past the "back" command */
				file_start += 3;
			}

			/* Append the path to the buffer */
            strncat(buffer, temp_path, buff_size - strlen(buffer));

			/* Append a directory separator */ 
            strncat(buffer, "/", buff_size - strlen(buffer));

			/* Append the file to the buffer */
            strncat(buffer, file_start, buff_size - strlen(buffer));

			/* We don't need this anymore */
			string_free(temp_path);
		}
	}

	/* Store the path in the history stack */
	if (stack != NULL)
	{
		history_add(stack, file_ptr, 0);
	}

	return (buffer);
}

/* ------------------------------------------------------------ ajps ---
 * Moves down the help display by the amount by.  Returns FALSE if
 * we are already at the bottom of the file.  If by = -1, go straight
 * to the bottom of the file.
 * --------------------------------------------------------------------- */
bool move_down(helpfile_blk *help_file, int *from_line, int height, int by, int no_unused_lines)
{
	bool at_bottom = FALSE;
                       
	/* If the  file doesn't fill up one screen */
	if (help_file->total_lines <= (height - no_unused_lines))
	{
		at_bottom = TRUE;
	}
	else
	{                       
       	if (*from_line == help_file->total_lines - height + no_unused_lines)
       	{  
       		at_bottom = TRUE;
       	}
       	else
       	{
			/* Move down by an amount*/
			*from_line += by;

			/* Check if we have gone beyond the end of the screen */
			if ((*from_line + height - no_unused_lines) > help_file->total_lines ||
			    by == -1)
			{
				/* Move back so we are just on screen. */
				*from_line = help_file->total_lines - height + no_unused_lines;
			}
		}
	}

	return (!at_bottom);
}

/* ------------------------------------------------------------ ajps ---
 * Moves up the help display by the amount by.  Returns FALSE if we are
 * already at the top, TRUE otherwise. If by = -1, go straight to the top.
 * --------------------------------------------------------------------- */
bool move_up(int *from_line, int height, int by)
{
	bool at_top = FALSE;
 
	/* Unused parameters */
	(void) height;
                
	/* Establish if we were at the top before */
	if (*from_line == 0)
	{
		at_top = TRUE;
	}
	else
	{
		if (by >= 0)
		{
	    	/* Move up by an amount */
			*from_line -= by;
        }
        else
        {
        	/* Let the next statement take the brunt */
        	*from_line = -1;
        }

		/* If we've gone too far, go back to the top. */
		if (*from_line < 0) *from_line = 0;
	}

	return (!at_top);
}

/* ------------------------------------------------------------ ajps ---
 * Frees the memory used by the history stack, only freeing those
 * elements that were actually allocated.
 * --------------------------------------------------------------------- */
void free_history_stack(history_blk *history)
{
	int n;

	/* For all entries in the stack */
	for (n = 0; n < HISTORY_DEPTH; n++)
	{
		/* As long as the memory has been claimed */
		if (history[n].file != NULL)
		{
			/* Free the memory it uses */
			rnfree((vptr) history[n].file);
		}
	}
}

/* ------------------------------------------------------------ ajps ---
 * Free all memory we've used in this file.
 * --------------------------------------------------------------------- */
void free_file_memory(helpfile_blk *help_file)
{
	/* If memory has been claimed for the display block, free it */
	if (help_file->display_block != NULL)
	{
		rnfree(help_file->display_block);
		help_file->display_block = NULL;
	}

	/* If memory has been claimed for the links block, free it */
	if (help_file->links_block != NULL)
	{
		rnfree(help_file->links_block); 
		help_file->links_block = NULL;
	}

	/* If memory has been claimed for the block data, free it */
	if (help_file->block_data != NULL)
	{
		KILL(help_file->block_data); 
		help_file->block_data = NULL;
	}
}

/* ------------------------------------------------------------ ajps ---
 * Initialises an array of x cptrs to hold the history stack.
 * --------------------------------------------------------------------- */
bool init_history_stack(history_blk *history)
{
	/* Counter */
	int n;

	/* Set all the memory pointers to NULL */
	for (n = 0; n < HISTORY_DEPTH; n++)
	{
		history[n].file = NULL;
		history[n].line = 0;
	}

	/* Claim memory for each history file name */
	for (n = 0; n < HISTORY_DEPTH; n++)
	{
		/* Claim memory for the 'n'th history filename */
		history[n].file = ralloc(XMLHELP_FILENAME_LENGTH + 1);

		/* If memory couldn't be claimed */
		if (history[n].file == NULL)
		{
			/* Free all memory claimed */
			free_history_stack(history);

			/* And return the error */
			return (FALSE);
		}

		/* Set the buffer to count nothing. */
		strcpy((char *)history[n].file, "");
	}

	/* All done */
	return (TRUE);
}

/* ------------------------------------------------------------ ajps ---
 * Load and parse the helpfile, ready for display on a screen of width
 * width.
 * --------------------------------------------------------------------- */
bool load_helpfile(char *filename, int filename_length, helpfile_blk *help_file, int width, history_blk *history, char *err_message)
{
	char *display, *links;
	BULP *bptr;

	/* Open file */
	bptr = bulp_open(filename, BULP_TYPE);

	/* If the file didn't open properly */
	if (bptr == NULL)
	{
		/* Write the file name into err_message*/
		sprintf(err_message, "%s", history[0].file);

		/* Build a path to the notfound error file */   
		path_build(filename, filename_length, ANGBAND_DIR_HELP, "err/notfound.xml");

		/* Return that we couldn't open the file */
		return (FALSE);
	}

	/* Parse file */
	if (parse_helpfile(bptr, &((*help_file).display_block),
					   &((*help_file).links_block), &(help_file->no_blks),
					   err_message) == FALSE)
	{
		/* Close the file */
		bulp_close(bptr);

		/* Build a path to the notparsed error file */                 
		path_build(filename, filename_length, ANGBAND_DIR_HELP, "err/notparsed.xml");

		return(FALSE);
	}

	/* Close file */
	bulp_close(bptr);

	/* Set aside memory for information about the redraw blocks */
	help_file->block_data = C_ZNEW(help_file->no_blks + 1, block_blk);

	/* If memory couldn't be allocated */
	if  (help_file->block_data == NULL)
	{
		/* Print this into the error message */
		sprintf(err_message, "Help system couldn't claim memory.");

		/* Free the memory used for the file */
		free_file_memory(help_file);

		return (FALSE);
	}

	/* set display to point to the start of actual redraw information */
	display = DISPLAY_BLOCK(help_file->display_block);

	/* Set links to point to the start of the links data */
	links = help_file->links_block + sizeof(int);

	/* Set an end-marker in the redraw block data */
	help_file->block_data[help_file->no_blks].end_line = -1;

	/* Read the information about all of the redraw blocks */
	if (get_block_info(display, width, help_file->block_data, links,
					   err_message) == FALSE)
	{
		/* The information collecting failed - free memory */
		free_file_memory(help_file);
		
		/* Build a path to the notparsed error file */                 
		path_build(filename, filename_length, ANGBAND_DIR_HELP, "err/notparsed.xml");

		/* And return an error */
		return (FALSE);
	}

	/* Record the total number of lines in the file - it is used a lot. */
	help_file->total_lines =
	help_file->block_data[help_file->no_blks - 1].end_line;

	return (TRUE);
}
                                  
/* ------------------------------------------------------------ ajps ---
 * Given a keypress and a mode of operation, we return the action that
 * should be taken by the help system.
 * --------------------------------------------------------------------- */
u32b process_help_keypress(char keypress, char mode)
{
	if (mode == MODE_LINK) return (ACTION_JUMP_TO_LINK);

	/* Return an action based on the keypress received */
	switch (keypress)
	{
		/* Quit help */
		case CHAR_QUIT:
		{
			return (ACTION_QUIT_HELP);
			break;
		}
		/* Page down */
		case ' ':
		{
			return (ACTION_PAGE_DOWN);
			break;
		}
		/* Down a line */
		case '=':
		case '+':
		{
			return (ACTION_STEP_DOWN);
			break;
		}
		/* Up a line */
		case '-':
		case '_':
		{
			return (ACTION_STEP_UP);
			break;
		}
		/* down a link */
		case '\t':
		{
			return (ACTION_LINK_DOWN);
			break;
		}
		/* up a link */
		case 'q':
		{
			return (ACTION_LINK_UP);
			break;
		}
		/* Change to link mode */
		case CHAR_LINK_ESCAPE:
		case '\\':
		{
			return (ACTION_LINK_MODE);
			break;
		}
		/* Go back */
		case CHAR_BACK:
		case '/':
		{
			return (ACTION_BACK);
			break;
		}
		/* Go forward, or more accurately "anti-back" */
		case '*':
		{
			return (ACTION_FORWARD);
			break;
		}
		/* Reload the page */
		case CHAR_RELOAD:
		{
			return (ACTION_RELOAD);
			break;
		}
		/* Select a link */
		case CHAR_SELECT:
		{
			return (ACTION_DO_LINK);
			break;
		}
		/* Maybe it's a direction */
		default:
		{
			 /* Extract direction from keypress */
			int direction = target_dir(keypress);

			if (!direction) break;

			/* Up & Right = PageUp */
			if (direction == DIRECTION_UP_RIGHT)
			{
				return (ACTION_PAGE_UP);
				break;
			}
			/* Down & Right = PageDown */
			else if (direction == DIRECTION_DOWN_RIGHT)
			{
				return (ACTION_PAGE_DOWN);
				break;
			}
			/* Up a line */
			else if (direction == DIRECTION_UP)
			{
				return (ACTION_STEP_UP);
				break;
			}
			/* Down a line */
			else if (direction == DIRECTION_DOWN)
			{
				return (ACTION_STEP_DOWN);
				break;
			}
			/* Start of text */
			else if (direction == DIRECTION_UP_LEFT)
			{
				return (ACTION_GOTO_START);
				break;
			}
			/* End of text */
			else if (direction == DIRECTION_DOWN_LEFT)
			{
				return (ACTION_GOTO_END);
				break;
			}
			/* To the right, down a link */
			else if (direction == DIRECTION_RIGHT)
			{
				return (ACTION_LINK_DOWN);
				break;
			}
			/* To the left, up a link */
			else if (direction == DIRECTION_LEFT)
			{
				return (ACTION_LINK_UP);
				break;
			}
		} /* End of default */
	}  /* End of keypress switch */

	return (ACTION_NOTHING);
}

/* ------------------------------------------------------------ ajps ---
 * Displays the file passed to it, if it can be found and parsed.
 * Most of the parameters are fully explained in "render_xml_file()".
 * If dht is TRUE, help text is displayed, otherwise not.
 * If persist is TRUE, the system will wait for keypresses and act on
 * them, otherwise not.
 * --------------------------------------------------------------------- */
bool show_helpfile(char *filename, int filename_length, char *mark, helpfile_blk *help_file, history_blk *history, int x, int y, int width, int height, char
*err_message, u32b *passback, bool persist, bool dht, char *entry_file)
{
#ifdef HELP_DEBUG_FILEDUMP
	FILE *debug;
#endif

#ifdef HELP_TIMING
	char timestr[8] = "";
	clock_t start, stop;
#endif
	
	/* Holds the user-level (as it were) pointers to those blocks */
	char *display;
	char *links;

	/* Mode of operation */
	char op_mode = MODE_NAV;

	char keypress = 0;

	u32b action;

	int	 from_line = 0;

	int start_block;

	/* In case it goes horribly, horribly wrong. */
	bool helping = TRUE;

	/* Do we need to redraw the whole screen, or just re-plot? */
	bool clear = FALSE;

	/* Number of lines not used for the main help display */
	int no_unused_lines = 0;
	
	/* The y offset of the main help display */
	int mhd_offset = 0;
	
	if (dht)
	{
		no_unused_lines = 4;
		mhd_offset = 2; 
	}

#ifdef HELP_TIMING
	start = clock();
#endif
	/*
	 * If filename="" we are moving to a point within the
	 * currently loaded file, so we can skip all the loading/init
	 * bits and just move to the right point.
	 */
	if (strcmp(filename, "") != 0)
	{
		if (load_helpfile(filename, filename_length, help_file, width - PAGE_BORDER * 2,
						  history, err_message) == FALSE)
		{
			if (load_helpfile(filename, filename_length, help_file, width - PAGE_BORDER * 2,
							  history, err_message) == FALSE)
			{
				/*
				 * We only allow two failures: the original file
				 * and the error file
				 */
				Term_putstr(x, y, -1, TERM_WHITE,
				"Couldn't access any help files properly, so must exit the help system.");

				/* Wait for user to clear any error before continuing */
				inkey();
				return (FALSE);
			}
		}
	}

	/* Set display to point to the redraw part of the display block */
	display = DISPLAY_BLOCK(help_file->display_block);

	/* Set links to point to the links information in the block */
	links = help_file->links_block + sizeof(int);

	/* Reset the keypress  */
	keypress=0;

	/* Don't clear the screen inside the redraw/keypress loop */
	clear=FALSE;

	/* We should really only clear the area of screen we've been given */
	Term_clear();

	/* Set display to the first line */
	from_line=0;

	/* Find mark point (if wanted) */
	if (my_stricmp(mark, "") != 0)
	{
		/* Find the block number of the mark specified */
		start_block = link_point_block(links, mark);

		if (start_block > 0 && start_block < help_file->no_blks)
		{
			from_line = help_file->block_data[start_block - 1].end_line;
		}
	}
	else
	{
		/* We restore the file to the line stored in the history */
		from_line = history[0].line;
	}

	/* If the display has gone past the end of file, pull it back. */
	if ((from_line + height - no_unused_lines) > help_file->total_lines)
	{
		if (help_file->total_lines <= (height - no_unused_lines))
		{
			from_line = 0;
		}
		else
		{
			from_line = help_file->total_lines - height + no_unused_lines;
		}
	}
	
	/* Enter keypress / display loop */
	while (keypress != CHAR_QUIT)
	{
#ifdef LINKS_ON_PAGE
		/* Find if there is a selected link */
		link_blk *selected = get_selected_link(links, NULL);

		/* If there isn't */
		if (selected == NULL)
		{
			/* Select first link on the page */
			select_first_link(links, from_line, from_line + height - no_unused_lines - 1);
		}
		else
		{
			/* If the selected link is now off the top of the page */
			if (selected->end_line < from_line)
			{
				/* Deselect the link */
				selected->data.status = LINK_UNSELECTED;

				/* Find the first link on the page */
				select_first_link(links, from_line,
								  from_line + height - no_unused_lines - 1);
			}
			/* If the selected link is now off the bottom of the page */
			else if (selected->start_line > (from_line + height - no_unused_lines - 1))
			{
				/* Deselect the link */
				selected->data.status = LINK_UNSELECTED;

				/* Find the last link on the page */
				select_last_link(links, from_line,
								 from_line + height - no_unused_lines - 1);
			}
		}
#endif
		if (clear) clear_from(y + mhd_offset);

#ifdef HELP_TIMING
		stop = clock();
		sprintf(timestr, "%3i cs",(int) (stop-start));

		/* Display timing */
		Term_putstr(width - strlen(timestr), 0, -1, TERM_L_BLUE, timestr);
#endif
		if (dht)
		{		
		/* Display file title */
		Term_putstr(0, 0, width, TERM_L_BLUE,
					FILE_TITLE(help_file->display_block));

		/* Draw rule under the title */
		horiz_rule(0, y+1, width, TERM_L_BLUE);
		}

		/* Update the history to the current line */
		history[0].line = from_line;

		/* Display selected chunk of file */
		if (plot_help(	display,
						help_file->block_data,
						from_line,
						links, x + PAGE_BORDER,
						y + mhd_offset,
						width - (PAGE_BORDER * 2),
						height - no_unused_lines,
						err_message) == FALSE)
		{
			helping = FALSE;
			break;
		}

		/* We've been politely asked to leave */
		if (!persist)
		{
			return (FALSE);
		}

		
		if (dht)
		{
			horiz_rule(0, y + height - mhd_offset, width, TERM_L_BLUE);
			
			/* Clear the help line, regardless */
			if (!clear) clear_from(y + height - 1);
        	
			/* Print help for the link mode */
			if (op_mode == MODE_LINK)
			{
				/* Print the help text for this mode */
				Term_putstr(0, y + height - 1, width, TERM_L_BLUE, link_help);
			}
			/* Print help for the navigation mode */
			else
			{
#ifdef LINKS_EXTRA
				/* ptr to an associated title */
				char *link_help_text = NULL;
        	
				/* pointer to the selected link */
				link_blk *selected = get_selected_link(links, NULL);
        	
				if (selected != NULL)
				{
					/* ptr to te associated title */
					link_help_text = LINK_HELP_TEXT(selected);
				}
        	
				/* If there is a link selected and a title to go with it */
				if (selected != NULL && strlen(link_help_text) > 0)
				{
					/* Buffer to hold the help text */
					char help_text[80]="";
        	
					/*
					 * Print the help text into the buffer under the
					 * control of the format string, help_format.
					 */
					sprintf(help_text, help_format, link_help_text);
        	
					/* zero-terminate in case it's too long */
					help_text[79] = 0;
        	
					/* Print it on the bottom line of the screen */
					Term_putstr(0, y + height - 1, width, TERM_L_BLUE,
								help_text);
				}
				else
#endif /* LINKS_EXTRA */
				{
					/* Just write the normal help text */
					Term_putstr(0, y + height - 1, width, TERM_L_BLUE,
								help_help);
				}
			}
		}
		
		/* Put the cursor at the bottom right */
		Term_gotoxy(width - 1, height - 1);

		/* We set this as a default for the next loop */
		clear = TRUE;

		/* Wait for keypress */
		keypress = inkey();

#ifdef HELP_TIMING
		start = clock();
#endif /* HELP_TIMING */

		action = process_help_keypress(keypress, op_mode);

		/* Do an action based on the keypress received */
		switch (action)
		{
			/* Quit help */
			case ACTION_QUIT_HELP:
			{
				helping = FALSE;

				break;
			}
			/* Page down */
			case ACTION_PAGE_DOWN:
			{
				move_down(help_file, &from_line, height, height - no_unused_lines, no_unused_lines);
				break;
			}
			/* Page up */
			case ACTION_PAGE_UP:
			{
				move_up(&from_line, height, (height - no_unused_lines));
				break;
			}
			/* Down a line */
			case ACTION_STEP_DOWN:
			{
				if (!move_down(help_file, &from_line, height, 1, no_unused_lines))
				{  
#ifdef LINKS_ON_PAGE
					link_sel_down(links, from_line, from_line + height - no_unused_lines - 1);
#else /* LINKS_ON_PAGE */
					link_sel_down(links, 0, help_file->total_lines);
#endif /* LINKS_ON_PAGE */
					clear = FALSE;
				}
				break;
			}
			/* Up a line */
			case ACTION_STEP_UP:
			{
				if (!move_up(&from_line, height, 1))
				{
#ifdef LINKS_ON_PAGE
					link_sel_up(links, from_line, from_line + height - no_unused_lines - 1);
#else /* LINKS_ON_PAGE */
					link_sel_down(links, 0, help_file->total_lines);
#endif /* LINKS_ON_PAGE */
				}
				break;
			}
			/* down a link */
			case ACTION_LINK_DOWN:
			{
#ifdef LINKS_ON_PAGE
				link_sel_down(links, from_line, from_line + height - no_unused_lines - 1);
#else /* LINKS_ON_PAGE */
				link_sel_down(links, 0, help_file->total_lines);
#endif /* LINKS_ON_PAGE */
				clear = FALSE;
				break;
			}
			/* up a link */
			case ACTION_LINK_UP:
			{
#ifdef LINKS_ON_PAGE
				link_sel_up(links, from_line, from_line + height - no_unused_lines - 1);
#else /* LINKS_ON_PAGE */
				link_sel_down(links, 0, help_file->total_lines);
#endif /* LINKS_ON_PAGE */
				break;
			}
			/* Change to link mode */
			case ACTION_LINK_MODE:
			{
				/* Next keypress will directly select a link */
				op_mode = MODE_LINK;

				break;
			}
			/* Go back */
			case ACTION_BACK:
			{
				/* Special case: if we are on the entry page, back = exit */ 
				if (strncmp(entry_file,history[0].file,strlen(entry_file)) == 0)
				{
                    helping = FALSE;
                    keypress = CHAR_QUIT;
				}
				else
				{
				/* Pulls the bottom page from the history up to the top */
				history_loop(history, -1);

				/* Finds the full path for it */
				help_path_build(filename, filename_length, (char *)(history[0].file), "", NULL);

				/* Escape from the redraw/keypres loop */
				keypress = CHAR_QUIT;
				}

				break;
			}
			/* Go forward, or more accurately "anti-back" */
			case ACTION_FORWARD:
			{
				history_loop(history, +1);
				help_path_build(filename, filename_length, (char *)(history[0].file), "", NULL);
				keypress = CHAR_QUIT;
				break;
			}
			/* Reload the page */
			case ACTION_RELOAD:
			{
				/* Jump out of the redraw/keypress loop */
				keypress = CHAR_QUIT;
				break;
			}
			case ACTION_GOTO_START:
			{
				/* Reset to the top of the screen */
				from_line = 0;
				break;
			}
			case ACTION_GOTO_END:
			{
				/* Set from_line to just show the bottom of the file */
				move_down(help_file, &from_line, height, -1, no_unused_lines);
				break;
			}
			/* Select a link by keypress */
			case ACTION_JUMP_TO_LINK:
			{
				/* See if that keypress corresponds to a link  */
				link_blk *linkptr = find_link_from_key(links, keypress);

				/* We've found a link */
				if (linkptr != NULL)
				{
					/* will return if the help system is wanted or not */
					helping = go_to_link(linkptr, filename, filename_length, mark,
										 history, passback);

					/* Breaks out of the redraw/keypress loop */
					keypress = CHAR_QUIT;
				}
				else
				{
					/* Do absolutely nothing, not even a redraw */
					clear = FALSE;
				}

				/* Change back to the normal mode of operation */
				op_mode = MODE_NAV;

				break;
			}
			/* Select a link */
			case ACTION_DO_LINK:
			{
				/* Get a pointer to the selected link */
				link_blk *linkptr = get_selected_link(links, NULL);

				/* If there is a link selected */
				if (linkptr != NULL)
				{
					/* Returns whether the help system is still wanted */
					helping = go_to_link(linkptr, filename, filename_length, mark,
										 history, passback);

					/* Jump out of this keypress/redraw loop */
					keypress = CHAR_QUIT;
				}
				else
				{
					/* Do absolutely nothing, not even a redraw */
					clear = FALSE;
				}
				break;
			}
		}  /* End of keypress switch */
	} /* End of keypress/redraw loop */

	return (helping);
}

/* ------------------------------------------------------------ ajps ---
 * Takes a file, specified as a normal link (ie. "/help/foo.xml#bar") and
 * displays it full-terminal.
 *
 * If clear = TRUE, then the screen state will be saved before the file
 * is displayed and reloaded when you are finished.
 *
 * If help_text = TRUE, then the (normally) light blue text and lines
 * and the top and the bottom of the screen will be shown, otherwise not.
 *
 * If persist = TRUE, it will act as an interactive system with the
 * ability to select links, and so on.  Otherwise, it will render the
 * file to screen and then return to the calling function.
 * --------------------------------------------------------------------- */
u32b render_xml_file(char *file, bool clear, bool help_text, bool persist)
{
	/*
	 * We use this to return a value from the help system so that
	 * it can be used for menus, etc.  It will normally stay zero.
	 */
	u32b passback = 0;

	/* Terminal size */
	int term_width, term_height;

	/* Error message buffer.  No special reason it being 120 characters */
	char err_message[120]="I just don't know why.";

	bool cont = TRUE;

	/* Holds the base-level information on the file being used atm */
	helpfile_blk help_file;

	/* Store the (help) path to the entry page, so we can make back exit */
	cptr entry_file = NULL;

	/* Holds the text of the mark being moved to */
	char mark[MAX_LINKPOINT_LENGTH+1]="";

	/* Holds the full path of the file being moved to */
	char filename[1024]="";

	/* The history stack, used for back/forward */
	history_blk history[HISTORY_DEPTH];

	/*
	 * Reset the help file information -
	 * so we know what memory we have claimed
	 */
	help_file.display_block = NULL;
	help_file.links_block = NULL;
	help_file.block_data = NULL;


	/* Initialise the history stack */
	if (init_history_stack(history) == FALSE)
	{
		/* Print an error if this fails */
		Term_putstr(0, 0, -1, TERM_L_BLUE,
					"Help system couldn't claim memory.");
		Term_putstr(0, 1, -1, TERM_L_BLUE, "Sorry, must exit back to game.");
		Term_putstr(0, 2, -1, TERM_L_BLUE, "-- Press a key to continue --");

		/* Wait for the user to acknowledge the error */
		inkey();

		return (0);
	}

	if ( file != NULL )
	{
		/* Use the link function to extract a mark point */
		link_to_file(file, filename, 1024, mark, history);

		/* No file was specified, use index */
		if (strcmp(filename,"") == 0)
		{
			/* Find full path, and store in the history */
			help_path_build(filename, 1024, "help/index.xml", "", history);
		}
	}
	else
	{        
		/* Find full path, and store in the history */
		help_path_build(filename, 1024, "help/index.xml", "", history);
	}
                                    
	/* history[0] contains the path to the entry file, so keep a record */
	entry_file = string_make(history[0].file);

	if (clear)
	{
		/* Save screen */
		screen_save();
    }
    
	/* Get terminal size */
	Term_get_size(&term_width, &term_height);

	/* While we still want the help system running */
	while (cont)
	{
		cont = show_helpfile(filename, 1024, mark, &help_file, history, 0, 0,
							  term_width, term_height, err_message, &passback, persist, help_text, (char *)entry_file);

		/* If we are moving to a new file */
		if (strcmp(filename,"") != 0)
		{                                
			/* free any memory that has been used for the last file */
			free_file_memory(&help_file);
		}

		/* Reset the error message */
		strcpy(err_message, "");

		/* Get terminal size (in case it has changed) */
		Term_get_size(&term_width, &term_height);

	} /* End of helping loop */
                      
	/* free any memory that has been used for the last file */
	free_file_memory(&help_file);
	
	/* Free the history stack memory */
	free_history_stack(history);

	if (entry_file != NULL) string_free(entry_file); 

	if (clear)
	{
		/* Load screen */
		screen_load();
	}
	
	return (passback);
}

/* ------------------------------------------------------------ ajps ---
 * Takes a file, specified as a normal link (ie. "/help/foo.xml#bar") and
 * displays it full-terminal.
 *
 * If file = NULL, the standard index is used.  Will return an integer
 * value as requested by a help file, or zero otherwise.
 * --------------------------------------------------------------------- */
u32b open_help(char *file)
{
	return (render_xml_file(file, TRUE, TRUE, TRUE));
}


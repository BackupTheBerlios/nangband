/* ------------------------------------------------------------ ajps ---
 * This file contains routines for simple access to an XML file, enabling
 * low-level parsing of the file in a relatively straightforward manner.
 *
 * Copyright (c) Antony Sidwell 2001-2002
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * This source file is also available under the GNU GENERAL PUBLIC LICENCE.
 * --------------------------------------------------------------------- */
#include "angband.h"
#include "xmlbulp.h"
          
/* 
 * These are needed because bulp is a library developed for non-angband
 * things, and it saves actually re-writing the whole thing.
 */
#define _bulp_malloc(size) ((void *) ralloc(size))  
#define _bulp_free(ptr,size) ((void *) rnfree(ptr))
#define _bulp_fopen(filename,mode) my_fopen(filename,mode)
#define _bulp_fclose(fptr) my_fclose(fptr)
#define _bulp_fread(ptr, size, nobj, fptr) fread(ptr, size, nobj, fptr)

/* ---------------------------------------------------------------------
 * Does a case insensitive comparison between two strings
 * --------------------------------------------------------------------- */
int bulp_stricmp(const char *s1, const char *s2)
{
	char ch1 = 0;
	char ch2 = 0;

	/* Just loop */
	while (TRUE)
	{
		/* We've reached the end of both strings simultaneously */
		if ((*s1 == 0) && (*s2 == 0))
		{
			/* We're still here, so s1 and s2 are equal */
			return(0);
		}

		ch1 = toupper(*s1);
		ch2 = toupper(*s2);

		/* If the characters don't match */
		if (ch1 != ch2)
		{
			/* return the difference between them */
			return((int) (ch1 - ch2));
		}

		/* Step on through both strings */
		s1++;
		s2++;
	}
}

/* ------------------------------------------------------------ ajps ---
 * Opens a file, and if wanted, allocates a buffer for it and loads it
 * into memory.
 * --------------------------------------------------------------------- */
BULP *bulp_open(char *filename, char *mode)
{
	BULP *return_value;

	/* m means cache into memory */
	if ( bulp_stricmp(mode, "m") == 0 )
	{
		FILE *fptr;

		/* We need to store a BULP struct with info about the file */
		return_value = malloc(sizeof(BULP));

		/* Should it fail, we return NULL */
		if ( return_value == NULL ) return(NULL);

		/* We store the flag to show it is memory-cached */
		return_value->_bulp_type = _BULP_MEM;

		/* We need to allocate a memory structure for info about the cache */
		return_value->_ptr = malloc(sizeof(_bulp_mem));

		/* If it fails, we return NULL */
		if ( return_value->_ptr == NULL )
		{
			/* First freeing memory claimed. */
			free(return_value);

			return(NULL);
		}

		/* Open the file as requested */
		fptr = _bulp_fopen(filename, "rb");

		/* If we can't open the file, return NULL */
		if ( fptr == NULL )
		{
			/* After freeing memory allocated */
			free(return_value->_ptr);
			free(return_value);

			return(NULL);
		}

		/* Here we must load the file into a buffer */
		{
			size_t file_size;

			/* This is the memory block that holds data on the cached file */
			_bulp_mem *mem_blk = return_value->_ptr;

			/* We establish the size of the file */
			fseek(fptr,0,SEEK_END);
			file_size = (size_t) ftell(fptr);
			fseek(fptr,0,SEEK_SET);

			/* If the file isn't there, we return NULL */
			if ( file_size == 0 )
			{
				/* First closing the open file */
				_bulp_fclose(fptr);

				/* And freeing memory used */
				free(return_value->_ptr);
				free(return_value);

				return(NULL);
			}

			/* We try to allocate a buffer for the file */
			mem_blk->base = _bulp_malloc(file_size);

			/* If it fails, we undo everything we've done, and return NULL */
			if ( mem_blk->base == NULL )
			{
				_bulp_fclose(fptr);
				free(return_value->_ptr);
				free(return_value);

				return(NULL);
			}

			/* If we haven't read the file in return NULL */
			if ( _bulp_fread(mem_blk->base, file_size, 1, fptr) != 1 )
			{
				/* After first undoing what we've done */
				_bulp_fclose(fptr);
				free(return_value->_ptr);
				free(return_value);

				return(NULL);
			}

			/* Set the value in the BULP struct to the file size */
			mem_blk->size = file_size;

			/* Set the start position to the beginning of the buffer */
			mem_blk->cur_pos = 0;
		}

		/* We've finished with the file, close it. */
		_bulp_fclose(fptr);
	}
	/* f means we parse directly from file */
	else if ( bulp_stricmp(mode,"f") == 0 )
	{
		/* We need memory to hold the bulp structure */
		return_value = malloc(sizeof(BULP));

		/* If we can't have it, return NULL */
		if ( return_value == NULL )
		{
			return(NULL);
		}

		/* We store that we are reading from file */
		return_value->_bulp_type = _BULP_FILE;

		/* We open the file, storing the file handle in the BULP struct */
		return_value->_ptr = _bulp_fopen(filename,"rb");

		/* If we can't open it, return NULL */
		if ( return_value == NULL )
		{
			/* First freeing the memory used */
			free(return_value);

			return(NULL);
		}
	}
	/* We have been passed an unknown mode */
	else 
	{
		/* Just return NULL */
		return(NULL);
	}

	/* Return the pointer to a BULP struct holding all we need to know */
	return(return_value);
}

/* ------------------------------------------------------------ ajps ---
 * Closes a file.  If it is held in memory the memory is freed.
 * --------------------------------------------------------------------- */
bool bulp_close(BULP *bulp_ptr)
{
	/* Why were we passed this? */
	if ( bulp_ptr == NULL )
	{
		return(TRUE);
	}

	/* If we are reading straight from file */
	if ( bulp_ptr->_bulp_type == _BULP_FILE )
	{
		/* Close the file */
		if ( _bulp_fclose(bulp_ptr->_ptr) != 0 )
			return(FALSE);

		/* Free the memory used by the BULP struct */
		free(bulp_ptr);

		/* Success */
		return(TRUE);
	}

	/* If we have the file cached in memory */
	if ( bulp_ptr->_bulp_type == _BULP_MEM )
	{
		/* This is to make it easier to access the data about the memory */
		_bulp_mem *mem_blk = bulp_ptr->_ptr;

		/* Free the memory for the file */
		_bulp_free(mem_blk->base,mem_blk->size);

		/* Free the memory for the BULP struct */
		free(bulp_ptr);

		/* Success */
		return(TRUE);
	}

	/* We've done nothing */
	return(FALSE);
}

/* ------------------------------------------------------------ ajps ---
 * Get the current position in the bulp.
 * --------------------------------------------------------------------- */
int bulp_getpos(BULP *bulp_ptr, u32b *pos)
{
	/* We can't do anything with this */
	if ( bulp_ptr == NULL )
	{
		return(-1);
	}

	/* We have the file, and so the position, in memory */
	if ( bulp_ptr->_bulp_type == _BULP_MEM )
	{
		/* Get the pointer the relevant data */
		_bulp_mem *mem_blk = bulp_ptr->_ptr;

		/* Set the position to the current read position */
		*pos = mem_blk->cur_pos;

		/* Return success */
		return(0);
	}

	/* We are reading from file, so must determine the file position */
	if ( bulp_ptr->_bulp_type == _BULP_FILE )
	{
		/* ftell() gives us the position from the start of file */
		*pos = ftell(bulp_ptr->_ptr);

		/* Success */
		return(0);
	}

	/* Failure, we've done nothing */
	return(-1);
}

/* ------------------------------------------------------------ ajps ---
 * Set the current position in the bulp - should only be used in
 * conjunction with bulp_getpos
 * --------------------------------------------------------------------- */
int bulp_setpos(BULP *bulp_ptr, u32b *pos)
{
	/* We shouldn't have got this */
	if ( bulp_ptr == NULL ) return(-1);

	/* If we are dealing directly with a file */
	if ( bulp_ptr->_bulp_type == _BULP_FILE )
	{
		/* Just use the standard function */
		return(fseek(bulp_ptr->_ptr, *pos, SEEK_SET));
	}

	/* If we are dealing with a cached file */
	if ( bulp_ptr->_bulp_type == _BULP_MEM )
	{
		_bulp_mem *mem_blk = bulp_ptr->_ptr;

		/* if the requested position is within file bounds */
		if ( *pos <= mem_blk->size )
		{
			/* Set the position */
			mem_blk->cur_pos = *pos;

			/* And return success */
			return(0);
		 }
		 else
		{
			/* Return failure */
			return(-1);
		}
	}

	/* Return failure - we've done nothing */
	return(-1);
}


/* ------------------------------------------------------------ ajps ---
 * Move to a point within a bulped file, either in file or in memory.
 * --------------------------------------------------------------------- */
int bulp_seek(BULP *bulp_ptr, s32b pos, int from)
{
	/* We shouldn't have been called */
	if ( bulp_ptr == NULL ) return(-1);

	/* Cached file */
	if ( bulp_ptr->_bulp_type == _BULP_MEM )
	{
		_bulp_mem *mem_blk = bulp_ptr->_ptr;

		/* If we are setting position wrt current position */
		if ( from == SEEK_CUR )
		{
			/* Convert pos to be from the start of the file */
			pos = mem_blk->cur_pos + pos;

			/* And treat as if it was a SEEK_SET */
			from = SEEK_SET;
		}

		/* As per fseek, this sets from the beginning */
		if ( from == SEEK_SET )
		{
			/* If the position is within the file */
			if ( pos >= 0)
			{
				if ((size_t) pos <= mem_blk->size)
				{
					/* Set position */
					mem_blk->cur_pos = pos;

					/* return successs */
					return(0);
				}
			}

			/* return failure */
			return(-1);
		}

		if ( from == SEEK_END )
		{
			/* If the position is within the file */
			if ( pos <= 0 && (size_t) abs(pos) > mem_blk->size )
			{
				/* Set the position */
				mem_blk->cur_pos = mem_blk->size + pos;

				/* return success */
				return(0);
			}
			else
			{
				/* return failure */
				return(-1);
			}
		}
	}

	/* If we are dealing with a normal file */
	if ( bulp_ptr->_bulp_type == _BULP_FILE )
	{
		/* Use the normal function */
		return(fseek(bulp_ptr->_ptr, pos, from));
	}

	/* we've done nothing - return failure */
	return(-1);
}

/* ------------------------------------------------------------ ajps ---
 * Read nobj objects as size size into the array pointed to by ptr.
 * --------------------------------------------------------------------- */
size_t bulp_read(void *ptr, size_t size, size_t nobj, BULP *bulp_ptr)
{
	/* This shouldn't have been passed here: return nothin read*/
	if ( bulp_ptr == NULL ) return(0);

	/* Cached file */
	if ( bulp_ptr->_bulp_type == _BULP_MEM )
	{
		u16b n;

		char *cptr = (char *) ptr;
		char *read_ptr;

		_bulp_mem *mem_blk = bulp_ptr->_ptr;

		/* Read data from the cache into the buffer we've been passed */
		for ( n=0; n < nobj; n++ )
		{
			/* Set the pointer to the current read position */
			read_ptr = mem_blk->base + mem_blk->cur_pos;

			/* if we are inside the file */
			if ( ( mem_blk->cur_pos + size ) <= mem_blk->size )
			{
				/* copy datum from the cache to the buffer */
				memcpy((cptr) + (size * n), read_ptr, size);

				/* Move the current read position */
				mem_blk->cur_pos += size;
			}
			else
			/* We are outside the file */
			{
				/* return how many items we did read in */
				return(n);
			}
		}

		/* We've read all the objects we were asked to */
		return(nobj);
	}

	/* We are reading direct from file */
	if ( bulp_ptr->_bulp_type == _BULP_FILE )
	{
		/* Return the result of the normal fread */
		return(_bulp_fread(ptr, size, nobj, bulp_ptr->_ptr));
	}

	/* We've read nothing, so return zero */
	return(0);
}

/* ------------------------------------------------------------ ajps ---
 * Returns zero if not at end of file.
 * --------------------------------------------------------------------- */
int bulp_feof(BULP *bulp_ptr)
{
	if ( bulp_ptr == NULL ) return(0);

	/* A cached file */
	if ( bulp_ptr->_bulp_type == _BULP_MEM )
	{
		_bulp_mem *mem_blk = bulp_ptr->_ptr;

		/* If the read position isn't at the end of the file */
		if ( mem_blk->cur_pos < mem_blk->size )
		{
			/* Return zero */
			return(0);
		}
		else
		{
			/* Return non-zero */
			return(1);
		}
	}

	/* Direct file access */
	if ( bulp_ptr->_bulp_type == _BULP_FILE )
	{
		FILE *fptr = bulp_ptr->_ptr;

		/* Return the normal feof */
		return(feof(fptr));
	}

	/* We shouldn't even be here. */
	return(0);
}


/* ------------------------------------------------------------ ajps ---
 * Returns the size of the file being read.
 * --------------------------------------------------------------------- */
size_t bulp_size(BULP *bulp_ptr)
{
	/* It shouldn't be here */
	if ( bulp_ptr == NULL ) return(0);

	/* Cached file */
	if ( bulp_ptr->_bulp_type == _BULP_MEM )
	{
		_bulp_mem *mem_blk = bulp_ptr->_ptr;

		/* We reutn the length we already know */
		return(mem_blk->size);
	}

	/* Direct file access */
	if ( bulp_ptr->_bulp_type == _BULP_FILE )
	{
		FILE *fptr = bulp_ptr->_ptr;
		u32b pos;
		size_t size;

		/* Store current file position */
		pos = ftell(fptr);

		/* Move to the end of file */
		fseek(fptr,0,SEEK_END);

		/* Read the length of file (distance from start to end) */
		size = ftell(fptr);

		/* Go back to where we were */
		fseek(fptr,pos,SEEK_SET);

		/* return the size */
		return(size);
	}

	/* We don't deal with this file, so return zero */
	return(0);
}

/*
 * This finds the next chunk of file that should be dealt with
 * and returns the type of data found.	Types returned are as follows:
 *		TYPE_TAG  is any ordinary tag
 *		TYPE_TEXT is a chunk of plain text
 *		TYPE_DOCTYPE is a doctype specifier tag
 *		TYPE_CDATA is a CDATA chunk
 *		TYPE_COMMENT is a commented-out bit (plain text)
 *		TYPE_CHARENTITY is a character entity, named or numbered
 *		TYPE_ENTITYDEF is an <!ENTITY tag, as defined in XML standards
 *					It can be accessed most easily using the normal tag
 *					access function.
 *		TYPE_EOF is returned at the end of the file
 */
int xmlbulp_get_next_bit(BULP *bptr, size_t *size)
{
	u32b pos;
	char readchar = 0;
	s16b type = -1;
	char readbuf[10] = "";

	/* Get the current position in the file */
	if ( bulp_getpos(bptr, &pos) != 0 )
	{
		/* We've not got the position, so return the fact */
		return(GETPOS_FAILED);
	}

	/* Read a single character from file */
	bulp_read(&readchar, sizeof(char), 1, bptr);

	/* If we are at the end of the file */
	if (bulp_feof(bptr))
	{
		/* Set the size of the chunk = zero */
		*size = 0;
		
		/* Return the end-of-file */
		return(TYPE_EOF);
	}

	/* If the character read opened a tag */
	if (readchar == '<')
	{
		/* Read the next character */
		bulp_read(&readchar, sizeof(char), 1, bptr);
		
		/* Only a few tags have a ! there, comments and CDATA */
		if (readchar == '!')
		{
			/* Read the next character */
			bulp_read(&readchar, sizeof(char), 1, bptr);
			
			/* If it's a '-', check for another dash */
			if (readchar == '-')
			{
				bulp_read(&readchar, sizeof(char), 1, bptr);

				/* If it is, we've found a comment */
				if (readchar == '-')
				{
					type = TYPE_COMMENT;

					/* Go to the start of the tag */
					bulp_setpos(bptr, &pos);

					/* Set size by finding the end of the comment */
					*size = xmlbulp_find_string(bptr, "-->");
				}
			}
			else /* no dash after the ! */
			{
				/* check if it is CDATA */
				if (readchar == '[')
				{
					type = TYPE_CDATA;

					/* Go to start of tag */
					bulp_setpos(bptr,&pos);

					/* find end of CDATA */
					*size = xmlbulp_find_string(bptr, "]]>");
				}
				else
				{
					/* Might be an antity definition */
					if (readchar == 'E')
					{
						/* Read some more */
						bulp_read(readbuf, sizeof(char), 5, bptr);

						/* Check if it is an entity def */
						if (strncmp(readbuf, "NTITY", 5) == 0)
						{
							type = TYPE_ENTITYDEF;

							/* Go back to the start of the tag */
							bulp_setpos(bptr, &pos);

							/* find the end of the definition */
							*size = xmlbulp_find_string(bptr, ">");
						}
					}
					else
					{
						/* The only other type starting with an ! */
						type = TYPE_DOCTYPE; 

						/* Go back to the start of the tag */
						bulp_setpos(bptr, &pos);

						/* Find the end */
						*size = xmlbulp_find_characters(bptr, XMLBULP_MAX_SIZE, 1, '>');
					}
				}
			}
		} /* endifs: special tag checking */

		/* If we've not picked a type yet, it is a user tag */
		if (type == -1)
		{
			type = TYPE_TAG;

			/* Go back to the start of the tag */
			bulp_setpos(bptr, &pos);

			/* Find the end of a generic tag */
			*size = xmlbulp_find_characters(bptr, XMLBULP_MAX_SIZE, 1, '>');
		}
	}
	else /* It's not a tag, but... */
	{
		/* It could be (should be) an entity */
		if (readchar == '&')
		{
			type = TYPE_CHARENTITY;

			/* Go to the start of the thing (one character in this case */
			bulp_setpos(bptr, &pos);

			/* 
			 * It should be ended by a semi-colon.  In case an & wasn't
			 * escaped, we need to limit the size it can return 
			 */
			*size = xmlbulp_find_characters(bptr, CHARENTITY_MAXSIZE, 1, ';');
		}
		else /* It's nothing else, it must be... */
		{
			type = TYPE_TEXT;

			/* Go back to start of section */
			bulp_setpos(bptr, &pos);

			/* 
			 * A text section can be ended by the opening of a tag or an
			 * entity.
			 */
			*size = xmlbulp_find_characters(bptr,XMLBULP_MAX_SIZE,2,'<','&');

			/* It counts up to and including the found character, so */
			(*size)--;
		}
	}

	/* Return the type of the tag. */
	return(type);
}

/*
 * This searches the file from the current point until it finds one
 * of the characters specified.
 * It then returns the size of the chunk between the location in
 * the file when it was called, and the character found (inclusive).
 * reasonable specifies a size that will be considered reasonable, if
 * we go outside this we should return zero.
 */
int xmlbulp_find_characters(BULP *bptr, size_t reasonable, int no_chars, ...)
{
	va_list ap;
	size_t size = 0;
	u32b pos;
	u16b found = 0;
	u16b loop=0;
	char readchar;
	char test_chars[20];

	if (no_chars > 20)
	{
		/* We can't check more than this, I'm afraid, beacuse of the above */
		return(0);
	}

	/* Variable arguments after no_chars */
	va_start(ap, no_chars);

	/* If we can't get the position successfully */
	if (bulp_getpos(bptr, &pos) != 0)
	{
		/* We say so. */
		return(GETPOS_FAILED);
	}

	/* We read the characters we are testing against into the array */
	for (loop = 0; loop < no_chars; loop++)
	{
		test_chars[loop] = va_arg(ap, int);
	}

	/* While we are still in the file and haven't found a match */
	while ( (!bulp_feof(bptr)) && (!found) )
	{
		/* Read the next character */
		bulp_read(&readchar, sizeof(char), 1, bptr);

		/* Reflect this in the size */
		size++;

		/* Test current character against our list */
		for (loop = 0; loop < no_chars; loop++)
		{
			if(test_chars[loop] == readchar) found = 1;
		}

		/* 
		 * If we are checking for reasonableness of size at all 
		 * and are past that point, return zero 
		 */
		if (size > reasonable)
		{
			/* Reset the size */
			size=0;

			/* Pretend we've found something */
			found = TRUE;
		}
	}
     
	/* We've got a copy now. */
	va_end(ap);
	
	/* Set the position back to the start */
	bulp_setpos(bptr,&pos);

	/* return the length of the string */
	return(size);
}

/*
 * This searches the file from the current point until it finds the
 * complete string specified, or reaches the end of the file.
 * It then returns the size of the chunk between the location in
 * the file when it was called, and the *end* of the string.
 */
int xmlbulp_find_string(BULP *bptr, char *string)
{
	size_t size = 0;
	size_t str_size = 0;
	u16b no_correct = 0;
	u32b pos;
	u32b search_pos;
	char readchar = 0;

	/* Store the start position */
	if ( bulp_getpos(bptr, &pos) != 0 )
	{
		/* And return this is we can't */
		return(GETPOS_FAILED);
	}

	/* Store the length of the string being searched for */
	str_size = strlen(string);

	/* Continue searching until the end of the file, or we've matched it */
	while ( (!bulp_feof(bptr)) && (no_correct != str_size) )
	{
		/* Read the next character */
		bulp_read(&readchar, sizeof(char), 1, bptr);

		/* We start from the beginning */
		if ( no_correct == 0 )
		{
			/* store the position */ 
			bulp_getpos(bptr, &search_pos);
		}

		/* if this character is correct */
		if (readchar == string[no_correct])
		{
			/* Move onto the next one */
			no_correct++;
		}
		else
		{
			/* Reduce the size  by the number we have matched so far */
			size -= no_correct;

			/* Reset the number we've matched */
			no_correct = 0;

			/* Go back to the start of the match */
			bulp_setpos(bptr, &search_pos);
		}

		/* Increase the size by one */
		size++;
	}

	/* Go back to the start */
	bulp_setpos(bptr,&pos);

	/* Return the size we've found */
	return(size);
}

/*
 * This is as simple as it can get - the ASCII part of unicode,
 * less than 128, is let through, the rest is stopped.
 * This could do with beefing up if possible.
 */
char xmlbulp_get_unicode_charentity(char *buffer)
{
	u32b unicode_no = atoi(buffer);

	/* If it corresponds to ASCII, let it through */
	if (unicode_no < 128) return((char) unicode_no);

	/* we just return a space if it hasn't matched (for now) */
	return(' ');
}

/*
 * This is a standard handler for dealing with simple occurances
 * of named entities.  If anything more complex is needed, it should
 * be implemented in the main application code.
 * These are the character entities that should be defined as standard
 * in all XML applications, according to the W3C standards.
 */
char xmlbulp_get_named_charentity(char *buffer)
{
	if (strncmp(buffer,"lt", 2) == 0) return('<');
	if (strncmp(buffer,"gt", 2) == 0) return('>');
	if (strncmp(buffer,"quot", 4) == 0) return('"');
	if (strncmp(buffer,"apos", 4) == 0) return('\'');
	if (strncmp(buffer,"amp", 3) == 0) return('&');

	/* This is non-standard, added for Angband purposes */
	if (strncmp(buffer,"nbsp",4) == 0) return(' ');

	/* If we haven't matched, return zero */
	return(0);
}

/*
 * We grab the chunk from the file, identify whether it
 * is a named or numbered entity, and return the name
 * (or number) as appropriate.
 */
int xmlbulp_get_char_entity(BULP *bptr, int size, char *buffer)
{
	char *ptr;
	int type;

	/* Read the first character */
	bulp_read(buffer, sizeof(char), size, bptr);

	/* Deal with the case where this shouldn't have been called */
	if ( buffer[0] != '&' ) return(CHARENTITY_NOT);

	/* We don't actually want the trailing ';' */
	buffer[size - 1] = 0;

	/* With a hash, this is a numbered entity */
	if ( buffer[1] == '#' )
	{
		/* Set ptr to the start of the "factual" bit */
		ptr = buffer + 2;
		type = CHARENTITY_UNICODE;
	}
	else
	{                  
		/* Set ptr to the start of the "factual" bit */
		ptr = buffer + 1;
		type = CHARENTITY_NAMED;
	}

	/* Move the content to the start of the buffer */
	memmove(buffer,ptr,(buffer - ptr) + size);

	/* Return the type detected */
	return(type);
}

/*
 * Simply checks a character against standard whitespace definition.
 */
bool xmlbulp_is_whitespace(char character)
{
	if ( character == ' ' ) return(TRUE);
	if ( character == '\t' ) return(TRUE);
	if ( character == '\n' ) return(TRUE);
	if ( character == '\r' ) return(TRUE);

	return(FALSE);
}

/*
 * Reads in the specified chunk, strips whitespace around newlines.
 * Pass from = 0 on the first call.  Returns -1 when finished, or the
 * number that should be passed as from next time.
 */
int xmlbulp_get_text_chunk_st(BULP *bptr, size_t size, u32b from, char *buffer, size_t bufflen)
{
	char *start, *end, *newline;
	size_t block_size = 0;
	size_t fetchsize = 0;
	u32b file_pos = 0;
	int return_value = 0;
	size_t n;

	/* If we can fit the whole of the rest of the block into the buffer */
	if ( (size - from) <= bufflen )
	{
		/* We set fetchsize to be the rest of the block */
		fetchsize = size - from;

		/* We are done after this, and will return -1 */
		return_value = -1;
	}
	else
	{
		/* Fetch a buffer's-worth */
		fetchsize = bufflen - 1;

		/* return the start position for next time */
		return_value = from + fetchsize;
	}

	/* We might want to come back later. */
	bulp_getpos(bptr, &file_pos);

	/* Read in as much as we are able */
	bulp_read(buffer, sizeof(char), fetchsize, bptr);

	/* zero-terminate so that we can use str functions */
	buffer[fetchsize] = 0;

	/* 
	 * Work out if we need to shorten the amount we pass back
	 * because of a whitespace at the end of the block 
	 */
	if ( return_value != -1 &&
		 xmlbulp_is_whitespace(buffer[fetchsize - 1]) )
	{
		n = fetchsize - 1;

		/* Cut out all the trailing whitespace */
		while (xmlbulp_is_whitespace(buffer[n]) &&  n > 0 )
		{
			n--;
		}

		/* Go forward a character (we preserve one space) */
		n++;

		/* Zero-terminate */
		buffer[n]=0;

		/* Set the fetchsize appropriately */
		fetchsize = n;

		/* Set the return value so that it is correct */
		return_value = from + fetchsize;

		/* 
		 * Add the number of characters we are using
		 * to the position we started from.
		 */
		file_pos += n;

		/* And set the file position so that it reflects that */
		bulp_setpos(bptr,&file_pos);
	}

	/* Check for a newline */
	newline = strchr(buffer,'\n');

	/* If there isn't one, check for a carriage return */
	if (newline == NULL) newline = strchr(buffer,'\r');

	/* While there are newlines (or CRs) left in the buffer */
	while ( newline != NULL )
	{
		/* Go to start of whitespace */
		start = newline;
		while (xmlbulp_is_whitespace(start[0]) && start > buffer)
		{
			start--;
		}

		/* We have stepped back one space too far. */
		if ( start > buffer )
		{
			start++;
		}

		/* Set the end to the location of the newline */
		end = newline;

		/* And then move it forwards until it hits a non-whitepsace */
		while (xmlbulp_is_whitespace(end[0]) && end < (buffer + fetchsize))
		{
			end++;
		}

		/* We've *always* gone one character past whitespace */
		end--;

		/* If we aren't at the start, we want to preserve one space. */
		if ( start != buffer || from != 0 )
		{
			start[0]=' ';
			start++;
		}

		/* Set the block_size to take in everything after "end" */
		block_size = end - buffer;

		/* Move the bit we are interested in to the start of the buffer */
		memmove(start, end + 1, fetchsize - block_size);

		/* block_size now set to the size of the block we removed */
		block_size = (end - start + 1);

		/* And fetchsize is reduced by that amount */
		fetchsize -= block_size;

		/* We look for another newline */
		newline = strchr(buffer,'\n');

		/* Or failing that, a carriage return */
		if ( newline == NULL ) newline = strchr(buffer,'\r');
	}

	/* return correct value */
	return(return_value);
}


/*
 * Reads in the specified chunk, without stripping whitespace around newlines.
 * Pass from = 0 on the first call.  Returns -1 when finished, or the
 * number that should be passed as from next time.
 */
int xmlbulp_get_text_chunk(BULP *bptr, size_t size, u32b from, char *buffer, size_t bufflen)
{
	size_t fetchsize = 0;
	int return_value = 0;

	/* If we can fit the whole of the rest of the block into the buffer */
	if ( (size - from) <= bufflen )
	{
		/* We set fetchsize to be the rest of the block */
		fetchsize = size - from;

		/* We are done after this, and will return -1 */
		return_value = -1;
	}
	else
	{
		/* Fetch a buffer's-worth */
		fetchsize = bufflen - 1;

		/* return the start position for next time */
		return_value = from + fetchsize;
	}

	/* Read in as much as we are able */
	bulp_read(buffer,sizeof(char),fetchsize,bptr);

	/* zero-terminate so that we can use str functions */
	buffer[fetchsize]=0;

	/* return correct value */
	return(return_value);
}

/* ------------------------------------------------------------ ajps ---
 * This takes a tag, as returned by get_tag_and_type, and returns a
 * pointer to the the value of attr, if it has been specified, or NULL
 * otherwise.
 * --------------------------------------------------------------------- */
char *xmlbulp_check_attr(char *tag,char *attr)
{
	char *ptr = tag;

	/* 
	 * Move ptr on by the size of the tag name, 
	 * so that it points to the first attribute, or zero.
	 */
	ptr += strlen(ptr) + 1;

	/* When ptr[0] == 0 we've reached the end of the tag */
	while ( ptr[0] != 0 )
	{
		/* If the case-insensitve match of the attribute name works */
		if (bulp_stricmp(ptr,attr) == 0)
		{
			/* Move the ptr to the value */
			ptr += strlen(ptr) + 1;

			/* And return it */
			return(ptr);
		}
		else
		{
			/* move onto the next attribute */
			ptr += strlen(ptr) + 1;

			/* Attributes & values should be paired, so we skip both */
			if ( ptr[0] != 0 ) ptr += strlen(ptr) + 1;
		}
	}

	/* Return NULL if we couldn't find the attribute. */
	return(NULL);
}


/*
 * We read the chunk from the file, and separate it into pieces,
 * zero-terminated, with a null string to mark the end.
 */
int xmlbulp_get_tag_and_type(BULP *bptr, size_t size, char *buffer)
{
	u16b type=TAG_OPEN;
	char *ptr = buffer;
	char *end_ptr = 0;
	u32b  buf_off = 0;

	/* Read the whole tag into memory */
	bulp_read(buffer, sizeof(char), size, bptr);

	/* And zero-terminate it */
	buffer[size]=0;

	/* Determine the type of the tag */
	if ( buffer[1]=='/' )
	{
		type = TAG_CLOSE;
	}

	/* This will be a self-contained tag */
	if ( buffer[size-2]=='/' ) type = TAG_COMPLETE;

	/* This is an XML identifier tag */
	if ( (buffer[1]=='?') && ( buffer[size-2]=='?' ) ) type = TAG_XML;

	/*
	 * Change unwanted characters to white space, preserving contents
	 * of inverted commas or quotes.
	 */
	while ( ptr[0] != 0 )
	{
		switch (ptr[0])
		{
			case '<':
			case '>':
			case '?':
			case '/':
			case '!':
				ptr[0]=' ';
				break;
				
			case '\'':
				/* This skips the contents of a single-quoted attribute */
				if ( strchr(ptr+1,'\'') != NULL )
				{
					ptr = strchr(ptr+1,'\'');
				}
				break;
				
			case '"':      
				/* This skips the contents of a double-quoted attribute */
				if ( strchr(ptr+1,'"') != NULL )
				{
					ptr = strchr(ptr+1,'"');
				}
				break;
		}
		ptr += 1;
	}

	/* Move the tag name to the start of the buffer block */
	ptr = buffer;

	/* Move ptr past whitespace */
	while (ptr[0] == ' ') ptr += 1;

	/* Set end_ptr to the first whitespace after that */
	end_ptr = strchr(ptr,' ');

	/* If there is no whitespace, something has gone wrong */
	if (end_ptr == NULL) return(TAG_FAILED);

	/* Move the tag name to the start */
	memmove(buffer, ptr, (end_ptr - ptr));

	/* Zero-terminate it */
	buffer[(end_ptr - ptr)] = 0;

	/* Move buf_off to after the tag name */
	buf_off = (end_ptr - ptr) + 1;

	/* And ptr to after its old location */
	ptr = end_ptr + 1;

	/*
	 * Go through the buffer picking out the sections and moving
	 * them together, separated by zeroes.
	 */
	while (ptr < (buffer + size))
	{
		/* skip preceding white space */
		while (ptr[0] == ' ') ptr += 1;

		/*This stops us running off the end of the buffer by accident. */
		if ( ptr[0] == 0 )
		{
			buffer[buf_off] = 0;
			return(type);
		}

		/* If we hit a quote */
		if ( (ptr[0] == '"') || (ptr[0] == '\'') )
		{
			/* Mark the end as being the end of the thing in quotes */
			end_ptr = strchr(ptr + 1, ptr[0]);

			/* Advance past te opening quote */
			ptr += 1;

			/* If there is no closing quote, we're in a mess. */
			if ( end_ptr == NULL ) return(TAG_FAILED);
		}
		else
		{
			/* Mark the end as being the next whitespace */
			end_ptr = strchr(ptr,' ');

			/* Unless there is an = before that */
			if ( (strchr(ptr,'=') < end_ptr) && (strchr(ptr,'=') != NULL) )
			{
				/* Mark the end as being the equals */
				end_ptr = strchr(ptr,'=');
			}

			/* If there is no whitespace or equals, we've gone wrong */
			if ( end_ptr == NULL ) return(TAG_FAILED);
		}

		/* Move this block to immdiately after the last one processed */
		memmove((buffer + buf_off), ptr, (end_ptr - ptr));

		/* zero-terminate it */
		buffer[buf_off + (end_ptr - ptr)] = 0;

		/* Move buf_off to indicate the point after this block now */
		buf_off += ( end_ptr - ptr ) + 1;

		/* Move ptr to be the point after where this block was before */
		ptr = end_ptr+1;
	}

	/* zero-terminate the whole thing, to indicate the end */
	buffer[buf_off]=0;

	/* return the tag type */
	return(type);
}

/*
 * We simply read the chunk from the file, and "top and tail" it.
 * If the end is not present (because it hasn't all been read in)
 * then it merely "tops" it, allowing you to call back and fetch the
 * last part, which will then just be "tailed".  Or the middle part,
 * which will be left untouched.
 */
int xmlbulp_get_CDATA_chunk(BULP *bptr, size_t size, u32b from, char *buffer, size_t bufflen)
{
	char *ptr;
	char *endptr=NULL;
	int return_value = 0;
	u32b file_pos;
	size_t fetchsize=0;


	/* If we can fit the whole of the rest of the block into the buffer */
	if ( (size - from) <= bufflen )
	{
		/* We set fetchsize to be the rest of the block */
		fetchsize = size - from;

		/* We are done after this, and will return -1 */
		return_value = -1;
	}
	else
	{
		/* Fetch a buffer's-worth */
		fetchsize = bufflen - 1;

		/* return the start position for next time */
		return_value = from + fetchsize;
	}
	
	/* We might want to come back later. */
	bulp_getpos(bptr, &file_pos);

	/* Read in as much as we are able */
	bulp_read(buffer, sizeof(char), fetchsize, bptr);

	/* zero-terminate so that we can use str functions */
	buffer[fetchsize] = 0;

	/* If there is a tag opening */
	if (strncmp(buffer, "<![CDATA[", 9) == 0 && from == 0)
	{
		/* Move past it */
		ptr = buffer + 9;
	}
	else
	{
		/* otherwise, we start from the start of the buffer */
		ptr = buffer;
	}

	/* If there is an tag closure, store its position */
	endptr = strstr(ptr,"]]>");

	/* If we didn't find a tag closure, move endptr to the end of the buffer */
	if ( endptr == NULL ) endptr = buffer + fetchsize;

	/* zero-terminate it */
	endptr[0]=0;

	/* Move the block we want to the start of the buffer */
	memmove(buffer,ptr,(endptr-ptr)+1);

	/* If the tag wasn't closed */
	if ( endptr == (buffer + fetchsize) )
	{
		/* look for the start of a closure */
		endptr = strchr(buffer,']');

		/* 
		 * if the ] was within a couple of characters from the end 
		 * we need to move back a bit so that the whole ending is
		 * fetched the next time.
		 */
		if ( endptr != NULL &&
		   ( (unsigned int) (endptr - buffer) >= (strlen(buffer) - 3) )  )
		{
			/* Reduce the fetchsize to reflect this */
			fetchsize -= strlen(buffer) - (endptr - buffer);

			/* And zero-terminate over the [ */
			endptr[0] = 0;

			/* Set the return value */
			return_value = from + fetchsize;

			/* And then set the file position to reflect the bit we've read */
			file_pos += fetchsize;
			bulp_setpos(bptr,&file_pos);
		}
	}

	return(return_value);
}


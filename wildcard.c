/*
 *	wildcard.c - wildcard test matching routines
 */

#include <string.h>

#include "global.h"
#include "wildcard.h"

/*
 *  Recursive wildcard matching function implemented from scratch - let me
 *  know if you can find an example that breaks it ... - GeK 4/96
 */
Bool
wildcard_match (const char *wildcard, const char *match_string)
{
	if (wildcard == NULL || match_string == NULL)
		return (FALSE);

	if (*match_string == '\0')
		/*  Nothing matches an empty string  */
		return (FALSE);

	for (; *wildcard != '\0'; wildcard++, match_string++)
	{
		if (*wildcard == '?')
		{
			/*  ? matches a single character  */
			if (*match_string == '\0')
				return (FALSE);
		}
		else if (*wildcard == '*')
		{
			/*  '*' matches zero or more chars  */

			if (*(++wildcard) == '\0')
				/*  Trailing '*' matches anything  */
				return (TRUE);
			
			/*  Recursively try rest of wildcard successively along string  */
			while (wildcard_match (wildcard, match_string) == FALSE)
			{
				if (*match_string == '\0')
					/*  reached end of string without a match so must fail  */
					return (FALSE);
				else
					/*  Try further along string .. */
					match_string++;
			}

			/*  If we reach here, must have matched wildcard  */
			return (TRUE);
		}
		else
		{
			/*  Not a recognised wildcard char, must match string  */

			if (*match_string != *wildcard)
				/*
				 *  Note end of matchstring will fall out here
				 *  since *wildcard can't be \0 here
				 */
				return (FALSE);
		}
	}

	/*  Fails if any unmatched string remaining here ..  */
	return (*match_string == '\0' ? TRUE : FALSE);
}

/*
 *  Makes a destination filename from a given source filename and a wildcard
 *  returns NULL on error else a malloc'd string
 */
char
*make_dest_filename (const char *source_filename, const char *dest_wildmask)
{
	char	buf[256];
	char	*bufptr = buf;

	buf[0] = '\0';

	/*  No source filename fails ..  */
	if (source_filename == NULL || *source_filename == '\0')
		return (NULL);

	/*  No destination wildmask returns the source filename ..  */
	if (dest_wildmask == NULL || *dest_wildmask == '\0')
		return (strdup (source_filename));

	for (; *dest_wildmask != '\0'; dest_wildmask++)
	{
#ifdef	notdef
		printf ("*dst = '%c' *src = '%c'\n", *dest_wildmask != '\0' ? *dest_wildmask : '0', *source_filename != '\0' ? *source_filename : '0' );
#endif

		if (*dest_wildmask == '?')
		{
			if (*source_filename != '\0' && *source_filename != '.')
				*(bufptr++) = *(source_filename++);
		}
		else if (*dest_wildmask == '*')
		{
			while (*source_filename != '\0' && *source_filename != '.')
				*(bufptr++) = *(source_filename++);
		}
		else 
		{
			*(bufptr++) = *(dest_wildmask);

			if (*dest_wildmask == '.')
			{
				/*  Skip to next dot  */
				while (*source_filename != '\0'
				       && *source_filename != '.')
					source_filename++;
				if (*source_filename == '.')
					source_filename++;
			}
			else
			{
				/*  Don't skip past a dot  */
				if (*source_filename != '\0'
				    && *source_filename != '.')
					source_filename++;
			}
		}
#ifdef	notdef
		*bufptr = '\0';
		printf ("after: *dst = '%c' *src = '%c' buf = '%s'\n", *dest_wildmask != '\0' ? *dest_wildmask : '0', *source_filename != '\0' ? *source_filename : '0', buf);
#endif
	}
	*bufptr = '\0';
	
	return (strdup (buf));
}

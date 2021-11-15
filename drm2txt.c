/*
 *	drm2txt.c
 *
 *	Converts DosDream source files (*.DRM) to plain text format
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: drm2txt.c $
 *	  Revision 1.0  1997/04/15  17:00:58  Graham
 *	  Initial revision
 *
 */
 
#include <stdio.h>

#include "global.h"

#if	defined (__BORLANDC__) && (__BORLANDC__ >= 0x0450 && __BORLANDC__ <= 0x0452)
void
_ExceptInit (void)
{
}
#endif	/*  BC++ v4.0x  */

#define PROG_NAME	"drm2txt"
#define VERSION	"1.0"

#ifdef	RCS
static const char	rcs_id[] = "$Id: drm2txt.c 1.0 1997/04/15 17:00:58 Graham Exp $";
#endif	/* RCS */

int
main (int argc, char *argv[])
{
	FILE	*ipfile;
	FILE	*opfile;
	Byte	buf[256];
	uint	bytes_read;

	/*  Opening banner  */
	fputs (PROG_NAME " v" VERSION " -- Converts Dragon DOS Dream files to text\n"
	       "Graham E. Kinns <gekinns@iee.org> " __DATE__ "\n"
	       "\n", stderr);

	if (argc != 3)
	{
		fprintf (stderr, "Usage: %s input_file output_file\n", argv[0]);
		return (1);
	}
	
	ipfile = fopen (argv[1], "rb");
	if (ipfile == NULL)
	{
		fprintf (stderr, "Error: Unable to open '%s'\n", argv[1]);
		return (1);
	}
	
	opfile = fopen (argv[2], "wt");
	if (opfile == NULL)
	{
		fprintf (stderr, "Error: Unable to create '%s'\n", argv[2]);
		return (1);
	}
	
	while ((bytes_read = fread (buf, 1, 256, ipfile)) == 256)
	{
		uint	buf_idx = 0;
		
		while (buf_idx < 256)
		{
			Byte	byte_count;
	
			byte_count = buf[buf_idx++];
			if (byte_count == 0x00)
				break;	/*  next block  */
			
			/*  Write out next byte_count bytes  */
			fwrite (&buf[buf_idx], 1, byte_count, opfile);
			buf_idx += byte_count;
			
			/*  Add CR/LF  */
			putc ('\n', opfile);
		}
	}

	if (bytes_read != 0)
		fputs ("Warning: Dream file corrupt (size not a multiple of 256 bytes)\n", stderr);

	return (0);
}

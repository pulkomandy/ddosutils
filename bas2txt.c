/*
 *	bas2txt.c
 *
 *	De-tokenises Dragon DOS basic files to plain text
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: bas2txt.c $
 *	  Revision 1.0  1997/04/15  17:01:02  Graham
 *	  Initial revision
 *
 */
 
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "bastoken.h"

#if	defined (__BORLANDC__) && (__BORLANDC__ >= 0x0450 && __BORLANDC__ <= 0x0452)
void
_ExceptInit (void)
{
}
#endif	/*  BC++ v4.0x  */

#define	PROG_NAME	"bas2txt"
#define	VERSION	"1.0"

#ifdef	RCS
static const char	rcs_id[] = "$Id: bas2txt.c 1.0 1997/04/15 17:01:02 Graham Exp $";
#endif	/* RCS */

int
main (int argc, char *argv[])
{
	FILE	*ipfile;
	FILE	*opfile;
	Byte	*buf;
	Word	start_addr;
	Word	length;
	Word	exec_addr;
	Word	next_line_addr;
	Byte	*buf_ptr;

	/*  Opening banner  */
	fputs (PROG_NAME " v" VERSION " -- Converts Dragon DOS tokenised BASIC files to text\n"
	       "Graham E. Kinns <gekinns@iee.org> " __DATE__ "\n"
	       "\n", stderr);

	if (argc != 3)
	{
		fprintf (stderr, "Usage: %s input_file output_file\n", argv[0]);
		return (1);
	}

	buf = calloc (1, 32768U);
	if (buf == NULL)
	{
		fprintf (stderr, "Unable to malloc 32K block\n");
		return (1);
	}

	ipfile = fopen (argv[1], "rb");
	if (ipfile == NULL)
	{
		fprintf (stderr, "Error: Unable to open '%s'\n", argv[1]);
		return (1);
	}

	/*  Check Dragon DOS header  */
	if (fread (buf, 1, 9, ipfile) != 9)
	{
		fprintf (stderr, "Read Error\n");
		return (1);
	}
	
	if (buf[0] != 0x55 || buf[8] != 0xaa || buf[1] != 0x01)
	{
		fprintf (stderr, "Error: not a Dragon DOS Basic file\n");
		return (1);
	}

	start_addr = (((Word) buf[2]) << 8) | buf[3];
	length     = (((Word) buf[4]) << 8) | buf[5];
	exec_addr  = (((Word) buf[6]) << 8) | buf[7];

	printf ("Start: $%04x Length: $%04x Exec: $%04x\n", start_addr, length, exec_addr);

	if (fread (buf, 1, length, ipfile) != length)
	{
		fprintf (stderr, "File too short - corrupt?\n");
		return (1);
	}

	opfile = fopen (argv[2], "wt");
	if (opfile == NULL)
	{
		fprintf (stderr, "Error: Unable to create '%s'\n", argv[2]);
		return (1);
	}
	
	buf_ptr = buf;

	while ((next_line_addr = (((Word) buf_ptr[0]) << 8) | buf_ptr[1]) != 0x0000)
	{
		Word	line_num;
		Byte	token;
		
		line_num = (((Word) buf_ptr[2]) << 8) | (Word) buf_ptr[3];

		fprintf (opfile, "%5u ", line_num);
		buf_ptr += 4;

		while ((token = *buf_ptr++) != 0x00)
		{
			if (token < 0x80)
			{
				putc (token, opfile);
			}
			else if (token == 0xff)
			{
				token = *buf_ptr;
				if (token == 0x00)
					break;
				else if (token < 0x80)
					continue;
				else
					buf_ptr++;
					
				token -= 0x80;
				if (token >= MAX_NUM_FUNCTIONS)
					fprintf (opfile, "{$ff%02x}", token+0x80);
				else
					fputs (function_token[token], opfile);
			}
			else	/*  token >= 0x80 && token < 0xff  */
			{
				token -= 0x80;
				if (token >= MAX_NUM_KEYWORDS)
					fprintf (opfile, "{$%02x}", token+0x80);
				else
					fputs (keyword_token[token], opfile);
			}
		}

		putc ('\n', opfile);

		buf_ptr = &buf[next_line_addr - start_addr];
	}

	free (buf);

	return (0);
}

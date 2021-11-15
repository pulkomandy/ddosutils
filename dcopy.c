/*
 *	dcopy.c    $Id: dcopy.c 1.1 1997/05/08 21:59:08 Graham Rel $
 *
 *	Copies files from Dragon DOS disk
 *
 *	Usage: 'dcopy A:', 'dcopy 1: *.bin', 'dcopy B: *.bin *.bi_'
 *	       'dcopy A: *.bin c:\temp\'
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: dcopy.c $
 *	Revision 1.1  1997/05/08 21:59:08  Graham
 *	Rename directory in split_filepath() prototype to fix warning.
 *
 *	Revision 1.0  1997/04/15 21:05:38  Graham
 *	Initial revision
 */
#ifdef	RCS
static const char	rcs_id[] = "$Id: dcopy.c 1.1 1997/05/08 21:59:08 Graham Rel $";
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "hookdbt.h"
#include "diskbios.h"
#include "dmabound.h"
#include "wildcard.h"

#if	defined (__BORLANDC__) && (__BORLANDC__ >= 0x0450 && __BORLANDC__ <= 0x0452)
void
_ExceptInit (void)
{
}
#endif	/*  BC++ v4.0x  */

#define PROG_NAME	"dcopy"
#define VERSION		"1.0"

#define	MAX_DIR_ENTS	160

static Byte	directory [MAX_DIR_ENTS][25];


/*  Local function prototypes  */
static Byte	get_drive_num (const char * const path);
static Byte	read_dragon_sector (Byte drive_num, Byte track, Byte side, Byte sector, Byte *buf);
static void	read_sector (Byte drive_num, Byte track, Byte side, Byte sector, Byte *buf);
static const char * const	dos_filename (uint dir_entry);
static void	split_filepath (const char * const path, char ** dir_name, char ** filename);
static Bool	file_copy (Byte drive_num, uint start_entry, const char *filename,
           	           uint num_sectors, Byte *buf, ulong *file_size);


static Byte
get_drive_num (const char * const path)
{
	extern const char* FILENAME;
	FILENAME = path;
	return 0;
}

static Byte
read_dragon_sector (Byte drive_num, Byte track, Byte side, Byte sector, Byte *buf)
{
	Disk_Base_Table	dbt;
	Byte	result;
	uint	count = 0;

	hook_disk_base_table (&dbt);
	dbt.table[3] = 0x01;
	dbt.table[4] = 18;
	update_disk_base_table (&dbt);
	
	while (count++ < 3)
	{
		result = (bios_read_sectors (drive_num, track, side, sector, 1, buf, 256) & 0xff00) >> 8;
		
		if (result == 0x00)
			break;
		
		bios_reset_drive (drive_num);
	}
	
	unhook_disk_base_table (&dbt);
	
	return (result);
}

/*  Wrapper for read_dragon_sector that handles fatal errors (may not return) */
static void
read_sector (Byte drive_num, Byte track, Byte side, Byte sector, Byte *buf)
{
	Byte	result;

	result = read_dragon_sector (drive_num, track, side, sector, buf);

	if (result != 0x00)
	{
		fprintf (stderr, "Error: BIOS error 0x%02x: %s\n",
		         result, bios_error_description (result));
		exit (1);
	}
}

static const char * const
dos_filename (uint dir_entry)
{
	static char	filename[13];
	static const char	invalid_dos_chars[] = ":;,.\"|<>*?\\/+=[]";
	uint	i;
	Byte	b;
	char	*p;
	
	p = filename;
	
	for (i = 0; i < 8; i++)
	{
		b = directory[dir_entry][i+1];
		
		if (b == 0x00)
			break;

		if (b <= 0x20 || b > 0x7f || strchr (invalid_dos_chars, (char) b) != NULL)
			*p++ = '_';
		else
			*p++ = (char) b;
	}

	*p++ = '.';

	for (i = 0; i < 3; i++)
	{
		b = directory[dir_entry][i+9];
		
		if (b == 0x00)
			break;

		if (b <= 0x20 || b > 0x7f || strchr (invalid_dos_chars, (char) b) != NULL)
			*p++ = '_';
		else
			*p++ = (char) b;
	}

	*p = '\0';

	strlwr (filename);

	return (filename);
}

/*
 *  Splits supplied path into a directory (ending in \ / or :) and filename
 */
static void
split_filepath (const char * const path, char ** dir_name, char ** filename)
{
	char	buf[128];
	char	*filename_start;
	char	*p;

	*buf = '\0';
	if (path != NULL)
		strcpy (buf, path);
	
	p = filename_start = buf;
	while ((p = strpbrk (p, "\\/:")) != NULL)
		filename_start = ++p;

	if (filename != NULL)
		*filename = strdup ((*filename_start != '\0') ? filename_start : "*.*");

	/*  Terminate dir  */
	*filename_start = '\0';
	
	if (dir_name != NULL)
		*dir_name = strdup (buf);
}

static Bool
file_copy (Byte drive_num, uint start_entry, const char *filename,
           uint num_sectors, Byte *buf, ulong *file_size)
{
	uint	entry;
	uint	byte_idx;
	Word	lsn;
	Byte	sector_count;
	FILE	*opfile;
	Bool	data_in_buf;

	DPRINTF (("filecopy: entry %u %s\n", entry, filename));

	*file_size = 0;

	opfile = fopen (filename, "wb");
	if (opfile == NULL)
	{
		fprintf (stderr, "Error: Unable to create '%s'\n", filename);
		return (FALSE);
	}

	entry = start_entry;
	byte_idx = 12;
	data_in_buf = FALSE;

	for (;;)
	{
		lsn = (((Word) directory[entry][byte_idx]) << 8)
		      | directory[entry][byte_idx+1];
		sector_count = directory[entry][byte_idx+2];

		/*  Assume a sector count of 0 marks the end of used entries  */
		if (lsn == 0x0000 && sector_count == 0x00)
			break;

		DPRINTF (("Entry %3u offset %2u LSN %03x Count %u", entry, byte_idx, lsn, sector_count));

		/*  Read sector_count but only copy sector_count - 1  */
		while (sector_count-- > 0)
		{
			Byte	track, sector, side;
			
			if (data_in_buf)
			{
				fwrite (buf, 1, 256, opfile);
				*file_size += 256;
			}
			
			track  = lsn / num_sectors;
			sector = (lsn % num_sectors) + 1;
			if (sector > 18)
			{
				sector -= 18;
				side = 1;
			}
			else
			{
				side = 0;
			}

			read_sector (drive_num, track, side, sector, buf);

			lsn++;
			data_in_buf = TRUE;
		}

		byte_idx += 3;		

		/*  End of usable entries ?  */
		if (byte_idx > 21)
		{
			Byte	new_entry;
			
			/*  Is this entry continued ?  */
			if ((directory[entry][0] & 0x20) == 0x00)
				break;	/*  for ()  */

			/*  Offset 24 has ptr to next entry  */
			new_entry = directory[entry][24];

			/*  Sanity check for continuation block  */
			if ((directory[new_entry][0] & 0x01) == 0x00)
			{
				fprintf (stderr, "Warning: corrupt directory entry %u points to non-continuation entry %u\n", entry, new_entry);
				break;
			}

			entry = new_entry;
			byte_idx = 1;	/*  Continuation entries start at offset 1  */
		}
	}

	DPRINTF (("Entry %3u offset %2u byte 24 = %02x %3u", entry, byte_idx, directory[entry][24], directory[entry][24]));

	/*  Byte offset 24 is bytes used in last sector (0x00 == 256)  */
	if (data_in_buf)
	{
		fwrite (buf, 1, (directory[entry][24] != 0x00) ? directory[entry][24] : 256, opfile);
		*file_size += (directory[entry][24] != 0x00) ? directory[entry][24] : 256;
	}

	fclose (opfile);

	return (TRUE);
}

int
main (int argc, char *argv[])
{
	Byte	drive_num;
	Byte	*buf;
	uint	num_tracks, num_sectors;
	uint	sector, entry;
	char	*wildcard;
	uint	file_count;
	ulong	file_size;
	char	*output_directory;
	char	*output_filemask;

	fputs (PROG_NAME " v" VERSION " -- Copies files from Dragon DOS disk\n"
	       "Graham E. Kinns <gekinns@iee.org> " __DATE__ "\n"
	       "\n", stderr);

	if (argc < 2
	    || argv[1][0] == '-' || argv[1][0] == '/')
	{
		fprintf (stderr, "Usage: %s drive: [filename_wildcard] [dir\\][output_wildcard]\n\n", argv[0]);
		return (1);
	}

	drive_num = get_drive_num (argv[1]);
	if (drive_num == 0xff)
	{
		fprintf (stderr, "Error: invalid drive specifier in '%s'\n", argv[1]);	
		return (1);
	}

	if (argc > 2)
	{
		wildcard = malloc (strlen (argv[2]) + 3);
		if (wildcard == NULL)
		{
			fprintf (stderr, "Error: malloc failed\n");
			return (1);
		}
		strcpy (wildcard, argv[2]);
		strlwr (wildcard);
		if (strchr (wildcard, '.') == NULL)
			strcat (wildcard, ".*");	
	}
	else
	{
		wildcard = strdup ("*.*");
		if (wildcard == NULL)
		{
			fprintf (stderr, "Error: malloc failed\n");
			return (1);
		}		
	}

	split_filepath ((argc > 3) ? argv[3] : NULL, &output_directory, &output_filemask);

	buf = malloc_dma_page_aligned_block (256);
	if (buf == NULL)
	{
		fprintf (stderr, "Error: malloc failed\n");
		return (1);
	}

	bios_reset_drive (drive_num);

	read_sector (drive_num, 20, 0, 1, buf);

	num_tracks = buf[252];
	num_sectors = buf[253];
	if (num_tracks != (Byte) ~buf[254]
	    || num_sectors != (Byte) ~buf[255])
	{
		fprintf (stderr, "Not a Dragon DOS disk\n");
		return (1);
	}

	for (sector = 0; sector < 16; sector++)
	{
		read_sector (drive_num, 20, 0, sector+3, buf);

		/*  This is a crude, lazy fudge ... ;-)  */
		memcpy (directory[sector * 10], buf, 250);
	}

	file_count = 0;
	for (entry = 0; entry < MAX_DIR_ENTS && (directory[entry][0] & 0x08) == 0x00; entry++)
	{
		const char	*filename;

#ifdef	DEBUG
		{
			uint	i;
	
			printf ("%02x:", entry);

			for (i = 0; i < 25; i++)
				printf (" %02x", directory[entry][i]);
			
			printf ("\n");
		}
#endif

		/*  Skip deleted and continuation entries  */
		if (directory[entry][0] & (0x80 | 0x01))
			continue;

		filename = dos_filename (entry);

		if (wildcard_match (wildcard, filename))
		{
			Bool	result;
			char	*output_filename;
			char	output_filepath[128];
			
			output_filename = make_dest_filename (filename, output_filemask);
			if (output_filename == NULL)
			{
				/*  Failed  */
				fprintf (stderr, "Internal error: Unable to make o/p filename from '%s' '%s'\n", filename, output_filemask);
				continue;
			}
			
			strcpy (output_filepath, output_directory);
			strcat (output_filepath, output_filename);
			free (output_filename);

			printf ("Copying %-12s to %-*s ... ", filename, (int) strlen (output_directory) + 12, output_filepath);
			fflush (stdout);
			result = file_copy (drive_num, entry, output_filepath, num_sectors, buf, &file_size);
			if (result)
				printf ("%6lu bytes\n", file_size);
			else
				printf ("failed\n");
			file_count++;
		}
	}

	printf ("Files copied: %u\n", file_count);

	bios_reset_drive (drive_num);

	free_dma_page_aligned_block (buf);
	free (wildcard);

	return (0);
}

/*
 *	ddir.c
 *
 *	Directory of Dragon DOS disk
 *
 *	Usage: 'ddir A:', 'ddir 1: *.bin'
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: ddir.c $
 *	  Revision 1.0  1997/04/15  20:56:54  Graham
 *	  Initial revision
 *
 */
 
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

#ifdef	RCS
static const char	rcs_id[] = "$Id: ddir.c 1.0 1997/04/15 20:56:54 Graham Exp $";
#endif

#define PROG_NAME	"ddir"
#define VERSION		"1.0"

#define	MAX_DIR_ENTS	160

static Byte	directory [MAX_DIR_ENTS][25];


/*  Local function prototypes  */
static Byte	get_drive_num (const char * const path);
static Byte	read_dragon_sector (Byte drive_num, Byte track, Byte side, Byte sector, Byte *buf);
static void	read_sector (Byte drive_num, Byte track, Byte side, Byte sector, Byte *buf);
static const char * const	printable_filename (uint dir_entry);
static const char * const	dos_filename (uint dir_entry);
static ulong	calc_filesize (uint dir_entry_idx);


static Byte
get_drive_num (const char * const path)
{
	static const char	valid_drives[] = "abAB0123";

	/*  Accept 'A', 'A:', 'A:/' or 'A:\'   */
	
	if (strchr (valid_drives, path[0]) != NULL
	    && (path[1] == '\0'
	        || (path[1] == ':'
	            && (path[2] == '\0'
	                || (path[3] == '\0' 
	                    && (path[2] == '/'
	                        || path[2] == '\\'))))))
		switch (path[0])
		{
			case '0':
			case '1':
			case '2':
			case '3':
				return ((Byte) path[0] - '0');

			case 'a':
			case 'b':
				return ((Byte) path[0] - 'a');

			case 'A':
			case 'B':
				return ((Byte) path[0] - 'A');
		}

	return (0xff);
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
printable_filename (uint dir_entry)
{
	static char	filename[13];
	uint	i;
	Byte	b;
	
	for (i = 0; i < 8; i++)
	{
		b = directory[dir_entry][i+1];
		
		if (b == 0x00)
			break;

		if (b < 0x20 || b > 0x7f)
			filename[i] = '_';
		else
			filename[i] = (char) b;
	}

	for ( ; i < 8; i++)
		filename[i] = ' ';

	filename[8] = '.';

	for (i = 0; i < 3; i++)
	{
		b = directory[dir_entry][i+9];
		
		if (b == 0x00)
			break;

		if (b < 0x20 || b > 0x7f)
			filename[i+9] = '_';
		else
			filename[i+9] = (char) b;
	}

	for ( ; i < 3; i++)
		filename[i+9] = ' ';

	filename[12] = '\0';
	
	return (filename);
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

static ulong
calc_filesize (uint dir_entry_idx)
{
	uint	entry;
	uint	byte_idx;		/*  Ptr to LSN + count  */
	Word	start_lsn;
	Byte	sector_count;
	ulong	file_size = 0;

	entry = dir_entry_idx;
	byte_idx = 12;
	
	for (;;)
	{
		start_lsn = (((Word) directory[entry][byte_idx]) << 8)
		            | directory[entry][byte_idx+1];
		sector_count = directory[entry][byte_idx+2];

		/*  Assume a sector count of 0 marks the end of used entries  */
		if (start_lsn == 0x0000 && sector_count == 0x00)
			break;

		DPRINTF (("Entry %3u offset %2u LSN %03x Count %u", entry, byte_idx, start_lsn, sector_count));

		file_size += (ulong) sector_count * 256;

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
			else
			{
				entry = new_entry;
			}

			byte_idx = 1;
		}
	}

	/*  Byte offset 24 is bytes used in last sector (0x00 == 256)  */
	if (directory[entry][24] != 0x00)
		file_size -= 256 - directory[entry][24];

	DPRINTF (("Entry %3u offset %2u byte 24 = %02x %3u", entry, byte_idx, directory[entry][24], directory[entry][24]));

	return ((ulong) file_size);
}

int
main (int argc, char *argv[])
{
	Byte	drive_num;
	Byte	*buf;
	Bool	is_bootable;
	uint	num_tracks, num_sectors, num_sides;
	uint	sector, entry;
	ulong	bytes_free;
	char	*wildcard;
	uint	file_count;

	fputs (PROG_NAME " v" VERSION " -- Dragon DOS disk directory\n"
	       "Graham E. Kinns <gekinns@iee.org> " __DATE__ "\n"
	       "\n", stderr);

	if (argc < 2
	    || argv[1][0] == '-' || argv[1][0] == '/')
	{
		fprintf (stderr, "Usage: %s drive: [filename_wildcard]\n\n", argv[0]);
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

	buf = malloc_dma_page_aligned_block (256);
	if (buf == NULL)
	{
		fprintf (stderr, "Error: malloc failed\n");
		return (1);
	}


	bios_reset_drive (drive_num);

	/*  Check for boot sector while head is over Track 0  */

	read_sector (drive_num, 0, 0, 3, buf);

	is_bootable = (buf[0] == 'O' || buf[1] == 'S') ? TRUE : FALSE;

	read_sector (drive_num, 20, 0, 1, buf);

	num_tracks = buf[252];
	num_sectors = buf[253];
	if (num_tracks != (Byte) ~buf[254]
	    || num_sectors != (Byte) ~buf[255])
	{
		fprintf (stderr, "Not a Dragon DOS disk\n");
		return (1);
	}
	num_sides = (num_sectors > 18 ? 2 : 1);

	printf ("Disk in drive %u: %02u tracks, %u side%s, %02u sectors, %sbootable\n",
	        drive_num,
	        num_tracks,
	        num_sides,
	        num_sides == 1 ? "" : "s",
	        num_sectors,
	        is_bootable ? "" : "not ");

	for (sector = 0; sector < 16; sector++)
	{
		read_sector (drive_num, 20, 0, sector+3, buf);

		/*  This is a crude, lazy fudge ... ;-)  */
		memcpy (directory[sector * 10], buf, 250);
	}

#ifdef	notdef
	printf ("Files matching '%s':\n", wildcard);
#endif
	printf ("\n");

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
			printf ("%-13s  %6lu  %c  %s\n",
			        printable_filename (entry),
			        calc_filesize (entry),
			        directory[entry][0] & 0x02 ? 'P' : ' ',
		    	    filename);
			file_count++;
		}
	}

	bytes_free = 0;
	for (sector = 1; sector < 3; sector++)
	{
		uint	sector_count = 0;
		uint	byte;

		read_sector (drive_num, 20, 0, sector, buf);

		for (byte = 0; byte < 180; byte++)
		{
			Byte	mask = 0x80;
			uint	bit;
			
			for (bit = 8; bit; bit--, mask >>= 1)
				if (buf[byte] & mask)
					sector_count++;
		}

		bytes_free += (ulong) sector_count * 256;
	}

	printf ("\n");
	printf ("Files matching '%s': %u\n", wildcard, file_count);
	printf ("Free space: %6lu bytes\n", bytes_free);

	bios_reset_drive (drive_num);

	free_dma_page_aligned_block (buf);
	free (wildcard);

	return (0);
}

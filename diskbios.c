/*
 *	diskbios.c
 *
 *	BIOS Int 13h Disk interface functions
 */

#include <stdlib.h>
#ifdef	__DJGPP__
#  include <dpmi.h>
#  include <sys/movedata.h>		/*  dosmemget()  */
#else
#  include <dos.h>
#endif

#include "global.h"
#include "diskbios.h"

/*  Simplifies basic Int 13 calls coding...  */
#ifdef	__DJGPP__
#  define SIMPLE_REGS	__dpmi_regs
#  define SIMPLE_INT(r)	__dpmi_int (0x13, &(r))
#else
#  define SIMPLE_REGS	union REGS
#  define SIMPLE_INT(r)	int86 (0x13, &(r), &(r))
#endif

typedef struct bios_error_entry_struct
{
	const Byte			code;
	const char * const	description;
} BIOS_Error_Entry;

static BIOS_Error_Entry	bios_error_table [] =
{
	{ 0x00,	"No Error" },
	{ 0x01,	"Invalid function call or parameter" },
	{ 0x02,	"Address mark not found - Blank disk or wrong density?" },
	{ 0x03,	"Disk is write-protected" },
	{ 0x04,	"Sector not found - unrecognised format" },
	{ 0x06,	"Disk change not caught" },
	{ 0x08,	"DMA overrun" },
	{ 0x09,	"DMA across 64k page boundary" },
	{ 0x0c,	"Unsupported track/invalid media" },
	{ 0x10,	"CRC error - bad sector?" },
	{ 0x20,	"Controller or hardware failure" },
	{ 0x40,	"Seek failed - wrong format?" },
	{ 0x80,	"Timeout - no such drive or no disk in drive" },
	{ 0xff,	NULL }
};

#ifdef	__DJGPP__
/*
 *	Support functions for accessing a DOS low memory buffer from protected mode
 */

#  define MIN_BUFFER_SIZE			(256*18)

#  ifndef	MIN
#    define MIN(a,b)	((a) < (b) ? (a) : (b))
#  endif

static int	buffer_segment = -1;
static int	buffer_selector;
static uint	buffer_offset;		/*  in bytes  */
static uint	max_buffer_size;	/*  in bytes  */

static void
free_dos_buffer (void)
{
	DPRINTF (("free_dos_buffer()"));
	if (buffer_segment != -1)
		__dpmi_free_dos_memory (buffer_selector);	
}

static Bool
allocate_dos_buffer (void)
{
	unsigned int	buffer_paragraphs;
	unsigned int	boundary_distance;

	if (buffer_segment != -1)
		return (FALSE);
	
	/*  Paras required to hold our buffer  */
	buffer_paragraphs = (MIN_BUFFER_SIZE + 15) >> 4;

	/*  Get DOS memory segment of our transfer buffer, twice required size  */
	buffer_segment = __dpmi_allocate_dos_memory (2 * buffer_paragraphs, &buffer_selector);
	if (buffer_segment == -1)
		return (FALSE);

	/*  Calculate distance (in paras) to next 64K boundary  */
	boundary_distance = ((buffer_segment ^ 0xfff) & 0xfff) + 1;

	if (boundary_distance >= buffer_paragraphs)
	{
		/*  Align buffer to start of allocated dos buffer  */
		buffer_offset = 0;
		max_buffer_size = MIN (boundary_distance, 2 * buffer_paragraphs) << 4;
	}
	else
	{
		/*  Boundary is in lower half, align to boundary  */
		buffer_offset = boundary_distance << 4;
		max_buffer_size = (2 * buffer_paragraphs - boundary_distance) << 4;
	}

	DPRINTF (("dos memory buf: segment %04x selector %04x offset %04x maxsize %04x",
	          buffer_segment, buffer_selector, buffer_offset, max_buffer_size));

	/*  automatically free buffer at exit  */
	atexit (free_dos_buffer);

	return (TRUE);
}
#endif	/* __DJGPP__ */

const char *
bios_error_description (Byte error_code)
{
	BIOS_Error_Entry	*error_ptr;
	
	for (error_ptr = bios_error_table; error_ptr->description != NULL; error_ptr++)
		if (error_code == error_ptr->code)
			return (error_ptr->description);

	return ((const char *) "Internal error");
}

Byte
bios_reset_drive (Byte drive)
{
	SIMPLE_REGS	regs;

	regs.h.ah = 0x00;		/*  Int 13h AH=00h, reset drive  */
	regs.h.dl = drive;		/*  DL = drive number (0-3)      */

	SIMPLE_INT (regs);

	return ((Byte) regs.h.ah);
}

Byte
bios_get_last_error (Byte drive)
{
	SIMPLE_REGS	regs;

	regs.h.ah = 0x01;		/*  Int 13h AH=00h, get last error  */
	regs.h.dl = drive;		/*  DL = drive number (0-3)         */

	SIMPLE_INT (regs);

	return ((Byte) regs.h.ah);
}

Word
bios_read_sectors (Byte drive, Byte track, Byte side, Byte start_sector,
                   Byte num_sectors, Byte FAR *buffer, uint buffer_len)
{
#ifdef	__DJGPP__

	__dpmi_regs	regs;

	if (buffer == NULL || buffer_len == 0)
		return (0xffff);

	/*  Ensure DOS low memory transfer buffer allocated  */
	if (buffer_segment == -1
	    && !allocate_dos_buffer ())
		return (0xffff);

	if (buffer_len > max_buffer_size)
		return (0xffff);

	regs.h.ah = 0x02;				/*  Int 13h AH=02h, read sector  */
	regs.h.al = num_sectors;		/*  AL = num sectors (1-18)      */
	regs.h.dl = drive;				/*  DL = drive number (0-3)      */
	regs.h.dh = side;				/*  DH = side (0-1)              */
	regs.h.ch = track;				/*  CH = track (0-79)            */
	regs.h.cl = start_sector;		/*  CL = start sector (1-18)     */
	regs.x.es = buffer_segment;		/*  ES = data buffer segment     */
	regs.x.bx = buffer_offset;		/*  BX = data buffer offset      */

	DPRINTF (("read sector drive=%u track=%u side=%u stsec=%u nsec=%u buf=%04x:%04x",
	          regs.h.dl, regs.h.ch, regs.h.dh, regs.h.cl, regs.h.al, regs.x.es, regs.x.bx));

	(void) __dpmi_int (0x13, &regs);

	DPRINTF (("Returned cflag=%d AX=0x%04x", regs.x.flags & 1, regs.x.ax));

	/*  Copy low memory transfer buffer to passed buffer  */
	dosmemget ((buffer_segment << 4) + buffer_offset, buffer_len, buffer);

	return ((Word) regs.x.ax);

#else	/* !__DJGPP__ */

	union  REGS		regs;
	struct SREGS	seg_regs;

	if (buffer == NULL || buffer_len == 0)
		return (0xffff);

	regs.h.ah   = 0x02;				/*  Int 13h AH=02h, read sector  */
	regs.h.al   = num_sectors;		/*  AL = num sectors (1-18)      */
	regs.h.dl   = drive;			/*  DL = drive number (0-3)      */
	regs.h.dh   = side;				/*  DH = side (0-1)              */
	regs.h.ch   = track;			/*  CH = track (0-79)            */
	regs.h.cl   = start_sector;		/*  CL = start sector (1-18)     */
	seg_regs.es = FP_SEG (buffer);	/*  ES = data buffer segment     */
	regs.x.bx   = FP_OFF (buffer);	/*  BX = data buffer offset      */

	DPRINTF (("read sector drive=%u track=%u side=%u stsec=%u nsec=%u buf=%04x:%04x",
	          regs.h.dl, regs.h.ch, regs.h.dh, regs.h.cl, regs.h.al, seg_regs.es, regs.x.bx));

	(void) int86x (0x13, &regs, &regs, &seg_regs);

	DPRINTF (("Returned cflag=%d AX=0x%04x", regs.x.cflag, regs.x.ax));

	return ((Word) regs.x.ax);

#endif	/* !__DJGPP__ */
}

Word
bios_write_sectors (Byte drive, Byte track, Byte side, Byte start_sector,
                    Byte num_sectors, Byte FAR *buffer, uint buffer_len)
{
#ifdef	__DJGPP__

	__dpmi_regs	regs;

	if (buffer == NULL || buffer_len == 0)
		return (0xffff);

	/*  Ensure DOS low memory transfer buffer allocated  */
	if (buffer_segment == -1
	    && !allocate_dos_buffer ())
		return (0xffff);

	if (buffer_len > max_buffer_size)
		return (0xffff);

	/*  Copy passed buffer into DOS low memory transfer buffer  */
	dosmemput (buffer, buffer_len, (buffer_segment << 4) + buffer_offset);

	regs.h.ah = 0x03;				/*  Int 13h AH=03h, write sector  */
	regs.h.al = num_sectors;		/*  AL = num sectors (1-18)       */
	regs.h.dl = drive;				/*  DL = drive number (0-3)       */
	regs.h.dh = side;				/*  DH = side (0-1)               */
	regs.h.ch = track;				/*  CH = track (0-79)             */
	regs.h.cl = start_sector;		/*  CL = start sector (1-18)      */
	regs.x.es = buffer_segment;		/*  ES = data buffer segment      */
	regs.x.bx = buffer_offset;		/*  BX = data buffer offset       */

	DPRINTF (("write sector drive=%u track=%u side=%u stsec=%u nsec=%u buf=%04x:%04x",
	          regs.h.dl, regs.h.ch, regs.h.dh, regs.h.cl, regs.h.al, regs.x.es, regs.x.bx));

	(void) __dpmi_int (0x13, &regs);

	DPRINTF (("Returned cflag=%d AX=0x%04x", regs.x.flags & 1, regs.x.ax));

	return ((Word) regs.x.ax);

#else	/* !__DJGPP__ */

	union  REGS		regs;
	struct SREGS	seg_regs;

	if (buffer == NULL || buffer_len == 0)
		return (0xffff);

	regs.h.ah   = 0x03;				/*  Int 13h AH=03h, write sector  */
	regs.h.al   = num_sectors;		/*  AL = num sectors (1-18)       */
	regs.h.dl   = drive;			/*  DL = drive number (0-3)       */
	regs.h.dh   = side;				/*  DH = side (0-1)               */
	regs.h.ch   = track;			/*  CH = track (0-79)             */
	regs.h.cl   = start_sector;		/*  CL = start sector (1-18)      */
	seg_regs.es = FP_SEG (buffer);	/*  ES = data buffer segment      */
	regs.x.bx   = FP_OFF (buffer);	/*  BX = data buffer offset       */

	DPRINTF (("write sector drive=%u track=%u side=%u stsec=%u nsec=%u buf=%04x:%04x",
	          regs.h.dl, regs.h.ch, regs.h.dh, regs.h.cl, regs.h.al, seg_regs.es, regs.x.bx));

	(void) int86x (0x13, &regs, &regs, &seg_regs);

	DPRINTF (("Returned cflag=%d AX=0x%04x", regs.x.cflag, regs.x.ax));

	return ((Word) regs.x.ax);

#endif	/* !__DJGPP__ */
}

Word
bios_format_track (Byte drive, Byte track, Byte side, Byte num_sectors,
                   Byte FAR *buffer, uint buffer_len)
{
#ifdef	__DJGPP__

	__dpmi_regs	regs;

	if (buffer == NULL || buffer_len == 0)
		return (0xffff);

	/*  Ensure DOS low memory transfer buffer allocated  */
	if (buffer_segment == -1
	    && !allocate_dos_buffer ())
		return (0xffff);

	if (buffer_len > max_buffer_size)
		return (0xffff);

	/*  Copy passed buffer into DOS low memory transfer buffer  */
	dosmemput (buffer, buffer_len, (buffer_segment << 4) + buffer_offset);

	regs.h.ah = 0x05;				/*  Int 13h AH=05h, format track  */
	regs.h.al = num_sectors;		/*  AL = num sectors (1-18)       */
	regs.h.dl = drive;				/*  DL = drive number (0-3)       */
	regs.h.dh = side;				/*  DH = side (0-1)               */
	regs.h.ch = track;				/*  CH = track (0-79)             */
	regs.x.es = buffer_segment;		/*  ES = data buffer segment      */
	regs.x.bx = buffer_offset;		/*  BX = data buffer offset       */

	DPRINTF (("format track drive=%u track=%u side=%u nsec=%u buf=%04x:%04x",
	          regs.h.dl, regs.h.ch, regs.h.dh, regs.h.al, regs.x.es, regs.x.bx));

	(void) __dpmi_int (0x13, &regs);

	DPRINTF (("Returned cflag=%d AX=0x%04x", regs.x.flags & 1, regs.x.ax));

	return ((Word) regs.x.ax);	

#else	/* !__DJGPP__ */

	union  REGS		regs;
	struct SREGS	seg_regs;

	if (buffer == NULL || buffer_len == 0)
		return (0xffff);

	regs.h.ah   = 0x05;				/*  Int 13h AH=05h, format track  */
	regs.h.al   = num_sectors;		/*  AL = num sectors (1-18)       */
	regs.h.dl   = drive;			/*  DL = drive number (0-3)       */
	regs.h.dh   = side;				/*  DH = side (0-1)               */
	regs.h.ch   = track;			/*  CH = track (0-79)             */
	seg_regs.es = FP_SEG (buffer);	/*  ES = data buffer segment      */
	regs.x.bx   = FP_OFF (buffer);	/*  BX = data buffer offset       */

	DPRINTF (("format track drive=%u track=%u side=%u stsec=%u nsec=%u buf=%04x:%04x",
	          regs.h.dl, regs.h.ch, regs.h.dh, regs.h.cl, regs.h.al, seg_regs.es, regs.x.bx));

	(void) int86x (0x13, &regs, &regs, &seg_regs);

	DPRINTF (("Returned cflag=%d AX=0x%04x", regs.x.cflag, regs.x.ax));

	return ((Word) regs.x.ax);

#endif	/* !__DJGPP__ */
}

Byte
bios_get_drive_parameters (Byte drive, Byte *num_drives, Byte *drive_type,
                           Byte *max_track, Byte *max_head, Byte *max_sector,
                           Word *param_table_segment, Word *param_table_offset)
{
	SIMPLE_REGS		regs;
#ifndef	__DJGPP__
	struct SREGS	seg_regs;
#endif

	regs.h.ah = 0x08;	/*  Int 13h AH=08h, get drive parameters  */
	regs.h.dl = drive;

#ifdef	__DJGPP__
	__dpmi_int (0x13, &regs);
#else
	int86x (0x13, &regs, &regs, &seg_regs);
#endif

	if (regs.h.ah == 0x00)
	{
		if (num_drives != NULL)	*num_drives = regs.h.dl;
		if (drive_type != NULL)	*drive_type = regs.h.bl;
		if (max_track  != NULL)	*max_track  = regs.h.ch;
		if (max_head   != NULL)	*max_head   = regs.h.dh;
		if (max_sector != NULL)	*max_sector = regs.h.cl;

#ifdef	__DJGPP__
		if (param_table_segment != NULL)	*param_table_segment = regs.x.es;
#else
		if (param_table_segment != NULL)	*param_table_segment = seg_regs.es;
#endif
		if (param_table_offset  != NULL)	*param_table_offset  = regs.x.di;
	}

	return ((Byte) regs.h.ah);
}

Byte
bios_get_dasd_type (Byte drive)
{
	SIMPLE_REGS	regs;

	regs.h.ah = 0x15;		/*  Int 13h AH=15h, get DASD type  */
	regs.h.dl = drive;		/*  DL = drive number (0-3)        */

	SIMPLE_INT (regs);

#ifdef	__DJGPP__
	return ((regs.x.flags & 1) ? 0xff : regs.h.ah);
#else
	return (regs.x.cflag ? 0xff : regs.h.ah);
#endif
}

Byte
bios_get_change_line_status (Byte drive)
{
	SIMPLE_REGS	regs;

	regs.h.ah = 0x16;		/*  Int 13h AH=16h, get diskchange-line status  */
	regs.h.dl = drive;		/*  DL = drive number (0-3)                     */

	SIMPLE_INT (regs);

	return ((Byte) regs.h.ah);
}

Byte
bios_set_dasd_type (Byte drive, Byte dasd_type)
{
	SIMPLE_REGS	regs;

	regs.h.ah = 0x17;		/*  Int 13h AH=17h, set DASD type for format  */
	regs.h.dl = drive;		/*  DL = drive number (0-3)                   */
	regs.h.al = dasd_type;	/*  AL = DASD type: 0x01  360K in 360K
							 *                  0x02  360K in 1.2M
							 *                  0x03  1.2M in 1.2M
							 *                  0x04  720K in 720K or 1.44M
							 */

	SIMPLE_INT (regs);

	return ((Byte) regs.h.ah);
}

Byte
bios_set_media_type (Byte drive, Byte max_track, Byte max_sector,
                     Word *param_table_segment, Word *param_table_offset)
{
	SIMPLE_REGS		regs;
#ifndef	__DJGPP__
	struct SREGS	seg_regs;
#endif

	regs.h.ah = 0x18;		/*  Int 13h AH=18h, set media type for format  */
	regs.h.dl = drive;		/*  DL = drive number (0-3)                    */
	regs.h.ch = max_track;	/*  CH = max cylinder (num cyls - 1)           */
	regs.h.cl = max_sector;	/*  CL = max sector                            */

#ifdef	__DJGPP__
	__dpmi_int (0x13, &regs);
#else
	int86x (0x13, &regs, &regs, &seg_regs);
#endif

	/*  Returns ptr to parameter table in ES:DI  */

#ifdef	__DJGPP__
	if (param_table_segment != NULL)	*param_table_segment = regs.x.es;
#else
	if (param_table_segment != NULL)	*param_table_segment = seg_regs.es;
#endif
	if (param_table_offset  != NULL)	*param_table_offset  = regs.x.di;

	return ((Byte) regs.h.ah);		
}

#ifndef	_DISKBIOS_H
#define	_DISKBIOS_H
/*
 *	diskbios.h
 */

#ifndef	_GLOBAL_H
#  include "global.h"
#endif

/*  Functions in diskbios.c  */
const char *bios_error_description (Byte error_code);
Byte bios_reset_drive (Byte drive);
Byte bios_get_last_error (Byte drive);
Word bios_read_sectors (Byte drive, Byte track, Byte side, Byte start_sector,
                        Byte num_sectors, Byte FAR *buffer, uint buffer_len);
Word bios_write_sectors (Byte drive, Byte track, Byte side, Byte start_sector,
                         Byte num_sectors, Byte FAR *buffer, uint buffer_len);
Word bios_format_track (Byte drive, Byte track, Byte side, Byte num_sectors,
                        Byte FAR *buffer, uint buffer_len);
Byte bios_get_drive_parameters (Byte drive, Byte *num_drives, Byte *drive_type,
                                Byte *max_track, Byte *max_head,
                                Byte *max_sector, Word *param_table_segment,
                                Word *param_table_offset);
Byte bios_get_dasd_type (Byte drive);
Byte bios_get_change_line_status (Byte drive);
Byte bios_set_dasd_type (Byte drive, Byte dasd_type);
Byte bios_set_media_type (Byte drive, Byte max_track, Byte max_sector,
                          Word *param_table_segment, Word *param_table_offset);

#endif	/* !_DISKBIOS_H */

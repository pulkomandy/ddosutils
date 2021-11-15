#ifndef	_HOOKDBT_H
#define	_HOOKDBT_H
/*
 *	hookdbt.h
 */

#define DISK_BASE_TABLE_INT_VECTOR	0x1e

/*
 *	Most conventional references document 11 bytes for the Disk Base Table,
 *	however at least one current IBM BIOS appears to use 12 so this value is
 *	defined as slightly larger.  It shouldn't cause any problems since
 *	a) we don't modify the original memory that the table occupied
 *	b) It should only be accessed via the vector Int 1Eh anyway
 */
#define	DISK_BASE_TABLE_SIZE	16

typedef struct disk_base_table_struct
{
	Word	orig_segment;	/*  Old Int 1Eh pointer  */
	Word	orig_offset;

#ifdef	__DJGPP__
	int		dos_buffer_segment;
	int		dos_buffer_selector;
#endif

	Byte	table[DISK_BASE_TABLE_SIZE];
	
} Disk_Base_Table;

/*  Global public functions in hookdbt.c  */
Bool	hook_disk_base_table (Disk_Base_Table *dbt);
Bool	update_disk_base_table (Disk_Base_Table *dbt);
Bool	unhook_disk_base_table (Disk_Base_Table *dbt);

#endif	/* !_HOOKDBT_H */

/*
 *	hookdbt.c	$Id: hookdbt.c 1.1 1997/05/11 22:32:42 Graham Rel $
 *
 *	Functions for hooking and modifying the BIOS Disk Base Table
 *
 *	$Log: hookdbt.c $
 *	Revision 1.1  1997/05/11 22:32:42  Graham
 *	Added ^C/^Break control.  Added RCS control.
 *
 */
#ifdef	RCS
static const char	rcs_id[] = "$Id: hookdbt.c 1.1 1997/05/11 22:32:42 Graham Rel $";
#endif

#include <stdlib.h>		/*  For a definition of NULL ...  */
#ifdef	__DJGPP__
#  include <dpmi.h>
#  include <sys/movedata.h>		/*  dosmem...()  */
#  include <sys/exceptn.h>		/*  __djgpp_set_ctrl_c()  */
#  include <go32.h>			/*  _go32_want_ctrl_break()  */
#else
#  include <mem.h>		/*  movedata()  */
#endif
#include <dos.h>	/*  putting it here avoids a DJGPP warning from sys/exceptn.h with -Wshadow  */
 
#include "global.h"
#include "msdos.h"
#include "hookdbt.h"

static int	hook_count = 0;
static int	old_dos_cbrk_state;
#ifdef	__DJGPP__
static int	old_ctrl_c_state;
#endif

Bool
hook_disk_base_table (Disk_Base_Table *dbt)
{
	if (dbt == NULL)
		return (FALSE);

	hook_count++;		/*  Track number of hook calls  */
	old_dos_cbrk_state = getcbrk ();
	setcbrk (0);		/*  Turn off (except console I/O)  */

	/*  Get the current Interrupt 1Eh vector  */
	msdos_get_interrupt_vector (DISK_BASE_TABLE_INT_VECTOR,
	                            &(dbt->orig_segment), &(dbt->orig_offset));

#ifdef	__DJGPP__

	/*
	 *  Disable ^C and ^Break to prevent calling program aborting
	 *  whilst DBT interrupt is redirected
	 */
	old_ctrl_c_state = __djgpp_set_ctrl_c (0);	/*  ^C won't raise a SIGINT  */
	_go32_want_ctrl_break (1);	/*  We'll handle ^Break, thanks (not)  */

	/*  Allocate a buffer in DOS low memory to hold a DBT copy  */
	dbt->dos_buffer_segment =
	    __dpmi_allocate_dos_memory ((DISK_BASE_TABLE_SIZE + 15) >> 4,
	                                &(dbt->dos_buffer_selector));
	if (dbt->dos_buffer_segment == -1)
		return (FALSE);
		
	/*  Copy old table into dbt struct  */
	dosmemget ((dbt->orig_segment << 4) + dbt->orig_offset, DISK_BASE_TABLE_SIZE, dbt->table);

	/*  Update low memory buffer from table  */
	update_disk_base_table (dbt);

	/*  Set vector to point at new copy of table  */
	msdos_set_interrupt_vector (DISK_BASE_TABLE_INT_VECTOR, dbt->dos_buffer_segment, 0x0000);

#else	/* !__DJGPP__ */

	/*  Copy old table into dbt struct  */
	movedata (dbt->orig_segment, dbt->orig_offset,
	          FP_SEG (dbt->table), FP_OFF (dbt->table), DISK_BASE_TABLE_SIZE);

	/*  Set vector to point at new copy of table  */
	msdos_set_interrupt_vector (DISK_BASE_TABLE_INT_VECTOR,
	                            FP_SEG (dbt->table), FP_OFF (dbt->table));

#endif	/* !__DJGPP__ */

	return (TRUE);
}

/*  This function is only does anything with DJGPP  */
Bool
update_disk_base_table (Disk_Base_Table *dbt)
{
	if (dbt == NULL)
		return (FALSE);

#ifdef	__DJGPP__

	if (dbt->dos_buffer_segment == -1)
		return (FALSE);

	/*  Copy table in dbt to dos low memory buffer (offset == 0x0000)  */
	dosmemput (dbt->table, DISK_BASE_TABLE_SIZE, dbt->dos_buffer_segment << 4);

#endif	/*  __DJGPP__  */

	return (TRUE);
}

Bool
unhook_disk_base_table (Disk_Base_Table *dbt)
{
	if (dbt == NULL)
		return (FALSE);

	if (dbt->orig_segment == 0x0000 && dbt->orig_offset == 0x0000)
		return (FALSE);

	/*  Restore Interrupt 1Eh vector  */
	msdos_set_interrupt_vector (DISK_BASE_TABLE_INT_VECTOR,
	                            dbt->orig_segment, dbt->orig_offset);

	dbt->orig_segment = 0x0000;
	dbt->orig_offset  = 0x0000;

#ifdef	__DJGPP__
	
	if (dbt->dos_buffer_segment == -1)
		return (FALSE);
		
	/*  Free DOS Low memory buffer  */
	__dpmi_free_dos_memory (dbt->dos_buffer_selector);
	
	dbt->dos_buffer_segment = -1;

#endif

	/*  Restore ^C and ^Break behaviour on last hook  */
	hook_count--;
	if (hook_count <= 0)
	{
#ifdef	__DJGPP__
		__djgpp_set_ctrl_c (old_ctrl_c_state);	/*  Re-enable ^C  */
		_go32_want_ctrl_break (0);	/*  We don't handle ^Break any more  */
#endif
		setcbrk (old_dos_cbrk_state);
		hook_count = 0;
	}

	return (TRUE);
}

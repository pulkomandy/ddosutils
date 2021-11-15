/*
 *	msdos.c
 *
 *	MS-DOS interface functions
 */
 
#ifdef	__DJGPP__
#  include <dpmi.h>
#else
#  include <dos.h>
#endif
 
#include "global.h"
#include "msdos.h"
 
#ifdef	__DJGPP__
void
msdos_get_interrupt_vector (Byte int_num, Word *segment, Word *offset)
{
	__dpmi_regs	regs;

	/*  Int 21h AH=35h returns vector AL in ES:BX  */
	regs.h.ah = 0x35;
	regs.h.al = int_num;

	__dpmi_int (0x21, &regs);

	*segment = regs.x.es;
	*offset = regs.x.bx;
}
#else	/* !__DJGPP__ */
void
msdos_get_interrupt_vector (Byte int_num, Word *segment, Word *offset)
{
	union	REGS	regs;
	struct	SREGS	seg_regs;

	/*  Int 21h AH=35h returns vector AL in ES:BX  */
	regs.h.ah = 0x35;
	regs.h.al = int_num;

	int86x (0x21, &regs, &regs, &seg_regs);

	*segment = seg_regs.es;
	*offset  = regs.x.bx;
}
#endif	/* !__DJGPP__ */

#ifdef	__DJGPP__
void
msdos_set_interrupt_vector (Byte int_num, Word segment, Word offset)
{
	__dpmi_regs	regs;

	/*  Int 21h AH=25h sets vector AL to DS:DX  */
	regs.h.ah = 0x25;
	regs.h.al = int_num;
	regs.x.ds = segment;
	regs.x.dx = offset;

	__dpmi_int (0x21, &regs);
}
#else	/* !__DJGPP__ */
void
msdos_set_interrupt_vector (Byte int_num, Word segment, Word offset)
{
	union	REGS	regs;
	struct	SREGS	seg_regs;

	/*  Int 21h AH=25h sets vector AL to DS:DX  */
	regs.h.ah   = 0x25;
	regs.h.al   = int_num;
	seg_regs.ds = segment;
	regs.x.dx   = offset;

	int86x (0x21, &regs, &regs, &seg_regs);
}
#endif	/* !__DJGPP__ */

#ifndef	_MSDOS_H
#define	_MSDOS_H
/*
 *	msdos.h
 *
 *	Prototypes for MS-DOS interface functions in msdos.c
 */
 
void	msdos_get_interrupt_vector (Byte int_num, Word *segment, Word *offset);
void	msdos_set_interrupt_vector (Byte int_num, Word segment, Word offset);

#endif	/* !_MSDOS_H */

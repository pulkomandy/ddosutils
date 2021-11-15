#ifndef	_GLOBAL_H
#define	_GLOBAL_H
/*
 *	global.h
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: global.h $
 *	  Revision 1.0  1997/04/15  20:00:55  Graham
 *	  Initial revision
 *
 */

typedef enum bool_enum
{
	FALSE = 0,
	TRUE
} Bool;

typedef unsigned char	Byte;
typedef unsigned short	Word;
typedef unsigned long	DWord;

typedef unsigned int	uint;
typedef unsigned long	ulong;

#ifdef	__DJGPP__
#  define FAR
#else
#  define FAR	far
#endif

#ifndef	_DEBUG_H
#  include "debug.h"
#endif	/* !_DEBUG_H */

#endif	/* !_GLOBAL_H */

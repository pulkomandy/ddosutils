#ifndef	_DEBUG_H
#define	_DEBUG_H
/*
 *	debug.h
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: debug.h $
 *	  Revision 1.0  1997/04/15  20:04:20  Graham
 *	  Initial revision
 *
 */
 
#ifdef	DEBUG

/*  In debug.c  */
extern uint			 debug_source_line;
extern const char	*debug_source_file;

void	debug_printf (char *fmt, ...);

# define DPRINTF(x)	{ debug_source_file = __FILE__; \
					  debug_source_line = __LINE__; \
					  debug_printf x; }
 
#else	/* !DEBUG */

# define DPRINTF(x)

#endif	/* !DEBUG */

#endif	/* !_DEBUG_H */

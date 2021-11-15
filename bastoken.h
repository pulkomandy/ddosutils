#ifndef	_BASTOKEN_H
#define	_BASTOKEN_H
/*
 *	bastoken.h
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: bastoken.h $
 *	  Revision 1.0  1997/04/15  20:04:38  Graham
 *	  Initial revision
 *
 */

#define	MAX_NUM_KEYWORDS	(0xe7 - 0x80 + 1)
#define	MAX_NUM_FUNCTIONS	(0xa8 - 0x80 + 1)

/*  Tables in bastoken.c  */
extern	const char * const keyword_token  [MAX_NUM_KEYWORDS];
extern	const char * const function_token [MAX_NUM_FUNCTIONS];

#endif	/* _BASTOKEN_H */

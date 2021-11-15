#ifndef _WILDCARD_H
#define _WILDCARD_H
/*
 *	wildcard.h
 *
 *	wildcard test matching routines header
 */

#ifndef _GLOBAL_H
#  include "global.h"
#endif	/*  _GLOBAL_H  */

Bool	 wildcard_match     (const char *__wildcard, const char *__string);
char	*make_dest_filename (const char *__fname, const char *__wildmask);

#endif	/*  _WILDCARD_H  */

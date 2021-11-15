/*
 *	debug.c
 */
 
#include <stdio.h>
#include <stdarg.h>
 
#include "global.h"
#include "debug.h"
 
#ifdef	DEBUG

const char	*debug_source_file;
uint		 debug_source_line;

#  define DEBUG_FILE	stdout

void	debug_printf (char *fmt, ...)
{
	va_list	var_args;

	fprintf (DEBUG_FILE, "%s %4u ", debug_source_file, debug_source_line);

	va_start (var_args, fmt);
	vfprintf (DEBUG_FILE, fmt, var_args);
	va_end (var_args);

	putc ('\n', DEBUG_FILE);
}
 
#endif	/* DEBUG */

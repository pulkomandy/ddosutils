/*
 *	bastoken.c
 *
 *	Graham E. Kinns  <gekinns@iee.org>  Apr '97
 *
 *	$Log: bastoken.c $
 *	  Revision 1.0  1997/04/15  17:02:46  Graham
 *	  Initial revision
 *
 */
#ifdef	RCS
static const char	rcs_id[] = "$Id: bastoken.c 1.0 1997/04/15 17:02:46 Graham Exp $";
#endif	/* RCS */

#include "bastoken.h"

const char * const keyword_token [MAX_NUM_KEYWORDS] =
{
	/* $80 */	"FOR",
	/* $81 */	"GO",
	/* $82 */	"REM",
	/* $83 */	"'",
	/* $84 */	"ELSE",
	/* $85 */	"IF",
	/* $86 */	"DATA",
	/* $87 */	"PRINT",
	/* $88 */	"ON",
	/* $89 */	"INPUT",
	/* $8a */	"END",
	/* $8b */	"NEXT",
	/* $8c */	"DIM",
	/* $8d */	"READ",
	/* $8e */	"LET",
	/* $8f */	"RUN",
	/* $90 */	"RESTORE",
	/* $91 */	"RETURN",
	/* $92 */	"STOP",
	/* $93 */	"POKE",
	/* $94 */	"CONT",
	/* $95 */	"LIST",
	/* $96 */	"CLEAR",
	/* $97 */	"NEW",
	/* $98 */	"DEF",
	/* $99 */	"CLOAD",
	/* $9a */	"CSAVE",
	/* $9b */	"OPEN",
	/* $9c */	"CLOSE",
	/* $9d */	"LLIST",
	/* $9e */	"SET",
	/* $9f */	"RESET",
	/* $a0 */	"CLS",
	/* $a1 */	"MOTOR",
	/* $a2 */	"SOUND",
	/* $a3 */	"AUDIO",
	/* $a4 */	"EXEC",
	/* $a5 */	"SKIPF",
	/* $a6 */	"DELETE",
	/* $a7 */	"EDIT",
	/* $a8 */	"TRON",
	/* $a9 */	"TROFF",
	/* $aa */	"LINE",
	/* $ab */	"PCLS",
	/* $ac */	"PSET",
	/* $ad */	"PRESET",
	/* $ae */	"SCREEN",
	/* $af */	"PCLEAR",
	/* $b0 */	"COLOR",
	/* $b1 */	"CIRCLE",
	/* $b2 */	"PAINT",
	/* $b3 */	"GET",
	/* $b4 */	"PUT",
	/* $b5 */	"DRAW",
	/* $b6 */	"PCOPY",
	/* $b7 */	"PMODE",
	/* $b8 */	"PLAY",
	/* $b9 */	"DLOAD",
	/* $ba */	"RENUM",
	/* $bb */	"TAB(",
	/* $bc */	"TO",
	/* $bd */	"SUB",
	/* $be */	"FN",
	/* $bf */	"THEN",
	/* $c0 */	"NOT",
	/* $c1 */	"STEP",
	/* $c2 */	"OFF",
	/* $c3 */	"+",
	/* $c4 */	"-",
	/* $c5 */	"*",
	/* $c6 */	"/",
	/* $c7 */	"^",
	/* $c8 */	"AND",
	/* $c9 */	"OR",
	/* $ca */	">",
	/* $cb */	"=",
	/* $cc */	"<",
	/* $cd */	"USING",

/*  Dragon DOS v1.0 tokens:  */

	/* $ce */	"AUTO",
	/* $cf */	"BACKUP",
	/* $d0 */	"BEEP",
	/* $d1 */	"BOOT",
	/* $d2 */	"CHAIN",
	/* $d3 */	"COPY",
	/* $d4 */	"CREATE	",
	/* $d5 */	"DIR",
	/* $d6 */	"DRIVE",
	/* $d7 */	"DSKINIT",
	/* $d8 */	"FREAD",
	/* $d9 */	"FWRITE",
	/* $da */	"ERROR",
	/* $db */	"KILL",
	/* $dc */	"LOAD",
	/* $dd */	"MERGE",
	/* $de */	"PROTECT",
	/* $df */	"WAIT",
	/* $e0 */	"RENAME",
	/* $e1 */	"SAVE",
	/* $e2 */	"SREAD",
	/* $e3 */	"SWRITE",
	/* $e4 */	"VERIFY",
	/* $e5 */	"FROM",
	/* $e6 */	"FLREAD",
	/* $e7 */	"SWAP"
};

const char * const function_token [MAX_NUM_FUNCTIONS] =
{
	/* $ff80 */	"SGN",
	/* $ff81 */	"INT",
	/* $ff82 */	"ABS",
	/* $ff83 */	"POS",
	/* $ff84 */	"RND",
	/* $ff85 */	"SQR",
	/* $ff86 */	"LOG",
	/* $ff87 */	"EXP",
	/* $ff88 */	"SIN",
	/* $ff89 */	"COS",
	/* $ff8a */	"TAN",
	/* $ff8b */	"ATN",
	/* $ff8c */	"PEEK",
	/* $ff8d */	"LEN",
	/* $ff8e */	"STR$",
	/* $ff8f */	"VAL",
	/* $ff90 */	"ASC",
	/* $ff91 */	"CHR$",
	/* $ff92 */	"EOF",
	/* $ff93 */	"JOYSTK",
	/* $ff94 */	"FIX",
	/* $ff95 */	"HEX$",
	/* $ff96 */	"LEFT$",
	/* $ff97 */	"RIGHT$",
	/* $ff98 */	"MID$",
	/* $ff99 */	"POINT",
	/* $ff9a */	"INKEY$",
	/* $ff9b */	"MEM",
	/* $ff9c */	"VARPTR",
	/* $ff9d */	"INSTR",
	/* $ff9e */	"TIMER",
	/* $ff9f */	"PPOINT",
	/* $ffa0 */	"STRING$",
	/* $ffa1 */	"USR",

/*  Dragon DOS v1.0 tokens:  */

	/* $ffa2 */	"LOF",
	/* $ffa3 */	"FREE",
	/* $ffa4 */	"ERL",
	/* $ffa5 */	"ERR",
	/* $ffa6 */	"HIMEM",
	/* $ffa7 */	"LOC",
	/* $ffa8 */	"FRE$"
};

/*
 *	djgppsml.c - provides optional replacement command-line related
 *	functions and fixes to reduce size of linked DJGPP v2 binaries.
 *
 *	Graham E. Kinns		<Graham@grempc.demon.co.uk>	May '96
 *
 *	$Id: djgppsml.c 1.1 1997/05/07 15:25:54 Graham Rel $
 *
 *	$Log: djgppsml.c $
 *	Revision 1.1  1997/05/07 15:25:54  Graham
 *	Added prototype for _npxsetup()
 *	Added NO_IS_EXEC for _is_executable() stub
 *	Added DOS_CRT0_FLAGS for safer Dos filename assumptions
 *
 */

#ifdef  __DJGPP__
#  include <crt0.h>

#  ifdef  RCS

static const char	rcs_id [] = "$Id: djgppsml.c 1.1 1997/05/07 15:25:54 Graham Rel $";

#  endif  /* RCS */

/*  Saves linking in an unused command line expansion fn  */
/*  Command line globbing  */
#  ifdef  NO_GLOB_FN

char **
__crt0_glob_function (char *_argument __attribute__ ((unused)))
{
	/*  do nothing  */
	return ((char**) 0);
}

#  endif  /* NO_GLOB_FN */


#  ifdef  NO_LOAD_ENV_FILE

void
__crt0_load_environment_file (char *_app_name __attribute__ ((unused)))
{
	/*  do nothing  */
}

#  endif  /* NO_LOAD_ENV_FILE */


/*  Sets up argc and argv[]  */
#  ifdef  NO_SETUP_ARGS

void
__crt0_setup_arguments (void)
{
	/*  do nothing  */
}

#  endif  /* NO_SETUP_ARGS */


/*  Speed up stat() calls by setting some things we'll never need ..  */
#  ifdef  FAST_STAT
#    include <sys/stat.h>

unsigned short	_djstat_flags = _STAT_INODE
                              | _STAT_EXEC_EXT
                              | _STAT_EXEC_MAGIC
                              | _STAT_DIRSIZE
                              | _STAT_ROOT_TIME
                              | _STAT_WRITEBIT;
#  endif  /* FAST_STAT */


/*
 *  Bug fix to prevent Windows Dos Box hanging after running DJGPP app
 *  Doesn't work with nested DJGPP applications
 *  destructor attribute causes fn to be called at exit()
 */
#  ifdef  DISABLE_COPRO_AT_EXIT
#    include <dpmi.h>

void	disable_copro_at_exit (void) __attribute__ ((destructor));

void
disable_copro_at_exit (void)
{
	__dpmi_set_coprocessor_emulation (1);
}
#  endif  /* DISABLE_COPRO_AT_EXIT */


/*  No NPX setup to prevent hanging under Win3.1 dos box with no real FPU  */
#  ifdef  NO_NPX_SETUP
#    include <libc/internal.h>

int	__emu387_load_hook;

void
_npxsetup (char *argv0 __attribute__ ((unused)))
{
	/*  do nothing  */
}

#  endif  /* NO_NPX_SETUP */


/*  Dummy _is_executable function to save a bit of space  */
#  ifdef  NO_IS_EXEC
#    include <sys/stat.h>

#ifdef	notdef	/*  Probably don't need this as well as _is_exec...  */
unsigned short
_get_magic (const char *s __attribute__ ((unused)),
            int fh __attribute__ ((unused)))
{
	return (0);
}
#endif	/* notdef */

int
_is_executable (const char *filename __attribute__ ((unused)),
                int fhandle __attribute__ ((unused)),
                const char *extension __attribute__ ((unused)))
{
	return (0);
}

#  endif  /*  NO_IS_EXEC  */


/*
 *  Set the crt0 flags for DOS programs that make assumptions about
 *  filenames, filename sizes and command line behaviour.
 */
#  ifdef  DOS_CRT0_FLAGS
/*  crt0.h already included at top  */

int	_crt0_startup_flags = _CRT0_FLAG_USE_DOS_SLASHES	/*  in argv[0]  */
   	                    | _CRT0_FLAG_DISALLOW_RESPONSE_FILES
   	                    | _CRT0_FLAG_NO_LFN;		/*  No Long filenames  */

#  endif  /*  DOS_CRT0_FLAGS  */


#endif  /* __DJGPP__ */

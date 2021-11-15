Quick and dirty UNIX port of ddosutils
======================================

This is originally an MSDOS tool to read Dragon 32 floppy disks.

This version is crudely converted to operate on VDK files instead. Not a lot of testing was done.

My ugly code in linux.c is under the WTFPL license (do whatever you want with it). The other parts
are under whichever license they were distributed under, I don't know.

The tools were patched to accept a VDK file name instead of a drive number in command line
parameters.

Compile using:

	make -f makefile.gcc

Enjoy!

Quick and dirty UNIX port of Graham E. Kinns' Dragon DOS Utils
==============================================================

A *NIX port of the Dragon DOS Utils, a suite of utilities for MS-DOS designed to handle Dragon DOS floppy disks, originally written by Graham E. Kinns  <gekinns@iee.org> in Apr 1997 and released under an unspecified open-source licence.

This version was ported to *NIX by Adrien Destugues <pulkomandy.tk> in November 2021; port-specific code is available in the file `linux.c` and is published under the WTFPL licence.

The original tool handled physical Dragon DOS floppy disks in a real floppy drive on MS-DOS computer systems; the port uses VDK disk images. If you have physical floppy disks you want to read, use an imaging tool like a KryoFlux to create a VDK.

Compiling
=========

`make -f makefile.gcc`

Clean with `make -f makefile.gcc clean clean_exes`

The tools are as follows, in alphabetical order:

bas2txt
=======

Converts Dragon BASIC BAS files extracted using `dcopy` into plain text files.

*USAGE:* `./bas2txt.exe input_file.bas output_file.txt`

dcopy
=====

Copies files out of a Dragon DOS VDK-format disk image onto your filesystem.

*USAGE:* `./dcopy.exe input_file.vdk [wildcard] [output_directory]`

ddir
====

Lists the files in a Dragon DOS VDK-format disk image.

*USAGE:* `./ddir.exe input_file.vdk [wildcard]`

drm2txt
=======

Converts DosDream DRM format files extracted using `dcopy` into plain text files.

*USAGE:* `./drm2txt.exe input_file.drm output_file.txt`

Enjoy!

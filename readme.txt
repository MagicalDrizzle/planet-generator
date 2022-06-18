ReadMe file for the planet generating program "Planet".
Copyright 1988-2009 Torben Æ. Mogensen


Copyright notice:

The program and manual are the copyright of Torben Æ. Mogensen.

The program and its sources are provided free of charge for use,
modification and redistribution for anyone who might want to do so.

Redistribution of the program or parts or modifications thereof in any
form must contain the original copyright notice and must in no way
restrict distribution of other versions of the program over what is
stipulated in this copyright notice.

Maps produced by the program are not copyrighted.


Disclaimer:

The program is provided "as is" and no guarantee of correctness or
usefulness is given.  The author accepts no responsibility for any
problems you may encounter during use of the program or maps generated
by the program.


Installation:

The program and a number of files (including this manual) are provided
in a Zip file called "planet.zip".  Extract this using e.g. WinZip (on
Windows) or unzip (linux/unix).

A makefile is provided for compiling the program, so if you have make
installed on your computer you can just write "make all" to compile
the program.  If not, you can use the following simplified method:

How to compile the program:

On a unix platform, just write

	cc planet.c -o planet -lm -O

or, if using Gnu C (GCC)

	gcc planet.c -o planet -lm -O

If you have GCC (e.g., DJGPP) installed on a Windows machine, the same
method can be used, but the executable should be named "planet.exe"
instead of just plain "planet".  If you have another C compiler, just
compile the way you normally do.

Enquiries and error reports can be sent to torbenm@diku.dk.


Acknowledgements:

I thank Jim Burrows for contributions to the code.

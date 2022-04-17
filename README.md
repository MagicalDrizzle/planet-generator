# planet-generator
HTTPS mirror of Torben Mogensen's planet generator along with modifications others made (currently, addition of the double orthographic hemispheres projection by Riviera71).  
Original site (warning: no https support): http://hjemmesider.diku.dk/~torbenm/Planet/  
I usually compile the program with clang on MSYS2: `clang -O2 -flto=full planet.c -o planet.exe`  
This produce a smaller and faster binary than gcc in my testings:  
https://topps.diku.dk/torbenm/thread.msp?topic=723973154  
Warning: Currently this program depends on at least one color file in the executable directory, for when you don't specify any color files at all (ie. the default). Currently it is Olsson.col.  
You can change which default file the program use in the code, but there's currently no way to embed the file into the program itself (ie. standalone)  
More information: https://topps.diku.dk/torbenm/thread.msp?topic=392461439  

Also most of the smartness and wonderfulness came from Riviera71: https://topps.diku.dk/torbenm/thread.msp?topic=218566649, I'm just the one doing random things...   


### Original readme file
```
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
```
### Additional copyright information on Torben's website
```
Both the program itself and maps created by the program are free for use, modification and reproduction, both privately and for commercial purposes, as long as this does not limit what other people may do with the program and images they produce with the program.  
For example, you can not copyright an image produced by the program unless you have substantially modified it afterwards, because other people might use the same parameters as you used to produce the same image.  
If you add code to the program that you do not wish to share, you can claim copyright for the code you write yourself, but that does not extend to the program as a whole.  
If you are in doubt about any particular use, you are welcome to contact me using the email address at the bottom if this page.  

Torben Mogensen  
DIKU, University of Copenhagen, Universitetsparken 5, DK-2100 København Ø  
E-mail: torbenm@di.ku.dk Telephone: (+45) 21 84 96 72 Fax: (+45) 35 32 14 01
```

# Makefile for "planet", a fractal planet generator

# Change the following if your C compiler isn't called 'gcc'.
CC=gcc

# And change this to your favourite C compiler flags:
CFLAGS = -O2 -s -w -ansi
# Original code is very old - have to disable warnings else it will vomit walls in your face
# I have attempted to "modernize" the code in my modified version.
CFLAGS_MOD = -O2 -s -Wall -Wextra -pedantic -ansi
LIBS = -lm

all:	planet

planet:
	$(CC) $(CFLAGS) planet.c -o planet $(LIBS)
	$(CC) $(CFLAGS_MOD) planet_mod.c -o planet_mod $(LIBS)	
	@echo "planet made"

clean:
	rm -f planet planet_mod planet.exe planet_mod.exe

SHARFILES = Manual.pdf Makefile readme.txt \
            planet.c planet_mod.c \
            Bathymetric.col Blackbody.col Lefebvre.col Lefebvre2.col grayscale.col \
            Olsson.col Olsson2.col OlssonLight.col default.col defaultB.col \
            burrows.col burrowsB.col mars.col light.col lava.col white.col wood.col yellow.col \
            Colors.rgb Gray.rgb Jons.rgb Mars.rgb White.rgb Yellow.rgb earth.map

share:	$(SHARFILES)
	tar c -f planet.tar $(SHARFILES)

distclean:
	rm -f planet.tar

# Makefile for "planet", a fractal planet generator

# Change the following if your C compiler isn't called 'gcc'.
#CC=gcc
CC=clang

# And change this to your favourite C compiler flags:
#CFLAGS = -O -g -W -Wall -D_USE_LIBM_MATH_H
CFLAGS = -O2

OBJS = planet.o 

LIBS = -lm

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all:	planet

planet: $(OBJS)
	#$(CC) $(CFLAGS) -o planet $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) -o planet $(OBJS)
	@echo "planet made"

clean:
	rm -f $(OBJS) planet

SHARFILES = Manual.txt Makefile ReadMe \
            planet.c \
            default.col defaultB.col burrows.col burrowsB.col mars.col\
            wood.col white.col

zip:	$(SHARFILES)
	zip planet.zip $(SHARFILES)

rmarc:
	rm -f planet.zip

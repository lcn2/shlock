#!/bin/make
#
# shlock - shlock makefile

SHELL= /bin/sh
CC= cc
CFLAGS= -g3 -O2 -n32

DESTDIR = /usr/local/bin
INSTALL= install
TARGETS= shlock

all: ${TARGETS}

shlock: shlock.o
	${CC} ${CFLAGS} shlock.o -o shlock

shlock.o: shlock.c
	${CC} ${CFLAGS} shlock.c -c

install: all
	${INSTALL} -m 0755 -F ${DESTDIR} shlock

clean:
	rm -f *.o

clobber: clean
	rm -f ${TARGETS}

#!/bin/make
#
# shlock - shlock makefile
#
# @(#) $Revision: 1.3 $
# @(#) $Id: Makefile,v 1.3 1999/10/10 00:49:59 chongo Exp chongo $
# @(#) $Source: /usr/local/src/cmd/shlock/RCS/Makefile,v $
#
# Please do not copyright this code.  This code is in the public domain.
#
# LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
# EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#
# chongo <was here> /\oo/\
#
# Share and enjoy!

SHELL= /bin/sh
CC= cc
CFLAGS= -g3 -O2

DESTDIR = /usr/local/bin
MAN1DIR = /usr/local/man/man1
INSTALL= install
TARGETS= shlock

all: ${TARGETS}

shlock: shlock.o
	${CC} ${CFLAGS} shlock.o -o shlock

shlock.o: shlock.c
	${CC} ${CFLAGS} shlock.c -c

install: all shlock.1
	${INSTALL} -m 0755 shlock ${DESTDIR}/shlock
	${INSTALL} -m 0644 shlock.1 ${MAN1DIR}/shlock.1

clean:
	rm -f *.o

clobber: clean
	rm -f ${TARGETS}

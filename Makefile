#!/bin/make
#
# shlock - shlock makefile
#
# @(#) $Revision: 1.7 $
# @(#) $Id: Makefile,v 1.7 2014/03/15 23:45:49 root Exp root $
# @(#) $Source: /usr/local/src/bin/shlock/RCS/Makefile,v $
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
CFLAGS= -g3 -O3 -Wall -W

DESTBIN = /usr/local/bin
MAN1DIR = /usr/local/man/man1
INSTALL= install
TARGETS= shlock

# remote operations
#
THISDIR= shlock
RSRCPSH= rsrcpush
RMAKE= rmake

all: ${TARGETS}

shlock: shlock.o
	${CC} ${CFLAGS} shlock.o -o shlock

shlock.o: shlock.c
	${CC} ${CFLAGS} shlock.c -c

install: all shlock.1
	${INSTALL} -m 0755 shlock ${DESTBIN}/shlock
	${INSTALL} -m 0644 shlock.1 ${MAN1DIR}/shlock.1

clean:
	rm -f *.o

clobber: clean
	rm -f ${TARGETS}

# help
#
help:
	@echo make all
	@echo make install
	@echo make clean
	@echo make clobber
	@echo
	@echo make pushsrc
	@echo make pushsrcn
	@echo
	@echo make rmtall
	@echo make rmtinstall
	@echo make rmtclobber
	@echo
	@echo make univ

# push source to remote sites
#
pushsrc:
	${RSRCPSH} -v -x . ${THISDIR}

pushsrcq:
	@${RSRCPSH} -q . ${THISDIR}

pushsrcn:
	${RSRCPSH} -v -x -n . ${THISDIR}

# run make on remote hosts
#
rmtall:
	${RMAKE} -v ${THISDIR} all

rmtinstall:
	${RMAKE} -v ${THISDIR} install

rmtclean:
	${RMAKE} -v ${THISDIR} clean

rmtclobber:
	${RMAKE} -v ${THISDIR} clobber

# build, install, and cleanup everywhere
#
univ: all install clobber pushsrc rmtall rmtinstall rmtclobber

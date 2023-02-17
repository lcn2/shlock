#!/usr/bin/env make
#
# shlock - create lock files for use in shell scripts
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

SHELL= bash
CC= cc
CFLAGS= -g3 -O3 -Wall -W

DESTBIN = /usr/local/bin
MAN1DIR = /usr/local/man/man1
INSTALL= install
TARGETS= shlock

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

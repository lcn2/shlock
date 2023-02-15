#!/bin/sh
#
# shlock_example - example of how to use shlock

# setup
#
SHLOCK=/usr/local/bin/shlock

USAGE="usage: $0 [-f] arg ..."
F_FLAG=
set -- `/usr/bin/getopt f $*`
if [ $? != 0 ]; then
    echo "$0: unknown or invalid -flag" 1>&2
    echo $USAGE 1>&2
    exit 1
fi
for i in $*; do
    case "$i" in
    -f) F_FLAG="true" ;;
    esac
    shift
done
if [ $# -lt 1 ]; then
    echo "$0: must have at least one arg" 1>&2
    echo $USAGE 1>&2
    exit 2
fi

# ...

# obtain the lock
#
LOCK=/tmp/.lock.progname # XXX - change progname to basename of program
if [ ! -x "$SHLOCK" ]; then
    echo "$0: cannot find executable: $SHLOCK" 1>&2
    exit 3
fi
if ! "$SHLOCK" -p $$ -f "$LOCK"; then
    if [ -z "$FORCE" ]; then
	echo "$0: locked by process" `cat "$LOCK"` 1>&2
	exit 2
    else
    	# the -f forced a lock override, so we must not remove the lock
	LOCK=
    fi
fi
# must trap after locking to avoid removal of another process lock
trap "rm -f $LOCK; exit" 0 1 2 3 15

# do something ...

# All done!!! -- Jessica Noll, Age 2
#
exit 0

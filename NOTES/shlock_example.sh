#!/bin/sh
#
# shlock_example - example of how to use shlock

# setup
#
SHLOCK=/usr/local/bin/shlock

# obtain the lock
#
LOCK=/tmp/.lock.progname # XXX - change progname to basename of program
if [ ! -x "$SHLOCK" ]; then
    echo "$0: cannot find executable: $SHLOCK" 1>&2
    exit 1
fi
if ! "$SHLOCK" -p $$ -f "$LOCK"; then
    echo "$0: locked by process" `cat "$LOCK"` 1>&2
    exit 2
fi
# must trap after locking to avoid removal of another process lock
trap "rm -f $LOCK; exit" 0 1 2 3 15

# do something ...

# All done!!! -- Jessica Noll, Age 2
#
exit 0

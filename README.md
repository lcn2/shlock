# shlock

create lock files for use in shell scripts


# To install

```sh
make clobber all
sudo make install clobber
```


# To use

```sh
/usr/local/bin/shlock [-h] [-V] [-p pid] [-b|-u] [-c] -f file

        -h              print help message and exit
        -V              print version string and exit

        -p pid          if lock is granted, lock with pid (def: parent process ID)
        -b              write pid into lock file in binary format (def: ASCII)
        -u              alias for -b
        -c              test for lock without locking

        -f file         lock filename (NOTE: -f file is required)

without -c:

        exit 0 (true shell exit) if lock was created and granted to pid
        exit 1 (false shell exit) if locked

with -c:

        exit 0 (true shell exit) if not locked
        exit 1 (false shell exit) if locked

shlock version: 1.6.1 2025-03-25
```


# Examples

The following example shows how shlock would be used within a shell script:

    # setup
    #
    SHLOCK=/usr/local/bin/shlock

    # obtain the lock
    #
    # XXX - change the string prog to the basename of the script
    LOCK=/tmp/.lock.prog
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

In another example, let us assume we want to write a script with an -F option that indicated that the script should proceed
even if the lock was not granted.  That is -F means to force past a lock.  Here is how this would be done:

    # setup
    #
    SHLOCK=/usr/local/bin/shlock

    USAGE="usage: $0 [-F] arg ..."
    F_FLAG=
    set -- `/usr/bin/getopt F $*`
    if [ $? != 0 ]; then
        echo "$0: unknown or invalid -flag" 1>&2
        echo $USAGE 1>&2
        exit 1
    fi
    for i in $*; do
        case "$i" in
        -F) F_FLAG="true" ;;
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
    # XXX - change the string prog to the basename of the script
    LOCK=/tmp/.lock.progname
    if [ ! -x "$SHLOCK" ]; then
        echo "$0: cannot find executable: $SHLOCK" 1>&2
        exit 3
    fi
    if ! "$SHLOCK" -p $$ -f "$LOCK"; then
        if [ -z "$FORCE" ]; then
         echo "$0: locked by process" `cat "$LOCK"` 1>&2
         exit 2
        else
         # the -F forced a lock override, so we must not remove the lock
         LOCK=
        fi
    fi
    # must trap after locking to avoid removal of another process lock
    trap "rm -f $LOCK; exit" 0 1 2 3 15

    # do something ...


In this example, we want silently exit if there is lock and proceed otherwise.  We assume that something else will
sometimes hold the lock.  We do not want to lock it ourselves, only ensure that the lock is not held by someone else.  Here
is how this would be done:

    # setup
    #
    SHLOCK=/usr/local/bin/shlock

    # ensure that it is not locked
    #
    LOCK=/tmp/.lock.name
    if [ ! -x "$SHLOCK" ]; then
        echo "$0: cannot find executable: $SHLOCK" 1>&2
        exit 1
    fi
    if ! "$SHLOCK" -c -f "$LOCK"; then
        # locked is else by someone, so silently exit
        exit 0
    fi

    # do something ...

# Reporting Security Issues

To report a security issue, please visit "[Reporting Security Issues](https://github.com/lcn2/XXX/security/policy)".

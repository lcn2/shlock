/*
**  Produce reliable locks for shell scripts, by Peter Honeyman as told
**  to Rich $alz.
**
** @(#) $Revision: 1.6 $
** @(#) $Id: shlock.c,v 1.6 2005/01/05 09:28:02 chongo Exp $
** @(#) $Source: /usr/local/src/bin/shlock/RCS/shlock.c,v $
*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * official version
 */
#define VERSION "1.6.1 2025-03-25"          /* format: major.minor YYYY-MM-DD */

#define NEWSUMASK 0002
#define MAX_TMP_TRY 17		/* max tmp file open retries */
#define TMP_USEC_WAIT 1270000	/* micro secs beteen tmp open or link retries */

static bool	BinaryLock;
static char	CANTUNLINK[] = "Can't unlink \"%s\", %s\n";
static char	CANTOPEN[] = "Can't open \"%s\", %s\n";


/*
**  See if the process named in an existing lock still exists by
**  sending it a null signal.
*/
static bool
ValidLock(char *name, bool JustChecking)
{
    register int	fd;
    register int	i;
    long		pid;
    char		buff[BUFSIZ];

    /* Open the file. */
    errno = 0;
    if ((fd = open(name, O_RDONLY)) < 0) {
	if (JustChecking) {
	    return false;
	} else {
	    (void)fprintf(stderr, CANTOPEN, name, strerror(errno));
	    return true;
	}
    }

    /* Read the PID that is written there. */
    if (BinaryLock) {
	if (read(fd, (char *)&pid, sizeof pid) != sizeof pid) {
	    (void)close(fd);
	    return false;
	}
    }
    else {
	if ((i = read(fd, buff, sizeof buff - 1)) <= 0) {
	    (void)close(fd);
	    return false;
	}
	buff[i] = '\0';
	errno = 0;
	pid = strtol(buff, NULL, 0);
	if (errno != 0) {
	    (void)close(fd);
	    return false;
	}
    }
    (void)close(fd);
    if ((pid_t)pid <= 0)
	return false;

    /* Send the signal. */
    errno = 0;
    if (kill((pid_t)pid, 0) < 0 && errno == ESRCH)
	return false;

    /* Either the kill worked, or we're optimistic about the error code. */
    return true;
}


/*
**  Unlink a file, print a message on error, and exit.
*/
static void
UnlinkAndExit(char *name, int x)
{
    errno = 0;
    if (unlink(name) < 0)
	(void)fprintf(stderr, CANTUNLINK, name, strerror(errno));
    exit(x);
}


/*
**  Print a usage message and exit.
*/
static void
Usage(void)
{
    (void)fprintf(stderr,
        "Usage: shlock [-h] [-V] [-p pid] [-b|-u] [-c] -f file\n"
	"\n"
	"\t-h\t\tprint help message and exit\n"
	"\t-V\t\tprint version string and exit\n"
	"\n"
	"\t-p pid\t\tif lock is granted, lock with pid (def: parent process ID)\n"
	"\t-b\t\twrite pid into lock file in binary format (def: ASCII)\n"
	"\t-u\t\talias for -b\n"
	"\t-c\t\ttest for lock without locking\n"
	"\n"
	"\t-f file\t\tlock filename (NOTE: -f file is required)\n"
	"\n"
	"without -c:\n"
	"\n"
	"\texit 0 (true shell exit) if lock was created and granted to pid\n"
	"\texit 1 (false shell exit) if locked\n"
	"\n"
	"with -c:\n"
	"\n"
	"\texit 0 (true shell exit) if not locked\n"
	"\texit 1 (false shell exit) if locked\n"
	"\n"
	"shlock version: %s\n", VERSION);
    exit(1);
    /* NOTREACHED */
}


int
main(int ac, char *av[])
{
    register int	i;
    register char	*p;
    register int	fd;
    char		tmp[BUFSIZ+1];
    char		buff[BUFSIZ+1];
    char		*name;
    long		pid;
    bool		ok;
    bool		JustChecking;
    extern int		optind;
    extern char *	optarg;
    char *program;

    /* Set defaults. */
    program = av[0];
    pid = 0;
    name = NULL;
    JustChecking = false;
    (void)umask(NEWSUMASK);
    pid = (long)getppid();
    memset(tmp, 0, sizeof(tmp));
    memset(buff, 0, sizeof(buff));

    /* Parse JCL. */
    while ((i = getopt(ac, av, "hVbcup:f:")) != EOF)
	switch (i) {
	case 'h':                   /* -h - print help message and exit */
	    Usage();
	    /* NOTREACHED */

	case 'V':
	    (void) printf("%s\n", VERSION);
	    exit(2); /* ooo */
	    /*NOTREACHED*/

	case 'b':
	case 'u':
	    BinaryLock = true;
	    break;

	case 'c':
	    JustChecking = true;
	    break;

	case 'p':
	    errno = 0;
	    pid = strtol(optarg, NULL, 0);
	    if (errno != 0) {
		pid = 0;
	    }
	    break;

	case 'f':
	    name = optarg;
	    break;

	case ':':
            (void) fprintf(stderr, "%s: ERROR: requires an argument -- %c\n", program, optopt);
	    Usage();
            /*NOTREACHED*/

        case '?':
            (void) fprintf(stderr, "%s: ERROR: illegal option -- %c\n", program, optopt);
	    Usage();
            /*NOTREACHED*/

	default:
	    Usage();
	    /* NOTREACHED */
	    break;
	}
    ac -= optind;
    av += optind;
    if (ac || (pid_t)pid <= 0 || name == NULL)
	Usage();

    /* Create the temp file in the same directory as the destination. */
    if ((p = strrchr(name, '/')) != NULL) {
	*p = '\0';
	(void)sprintf(tmp, "%s/shlock%ld", name, (long)getpid());
	*p = '/';
    }
    else
	(void)sprintf(tmp, "shlock%ld", (long)getpid());

    /* Loop for a while, until we can open the file. */
    i = 0;
    errno = 0;
    while ((fd = open(tmp, O_RDWR | O_CREAT | O_EXCL, 0644)) < 0) {

	/* process open failure */
	switch (errno) {
	default:
	    /* Unknown error -- give up. */
	    (void)fprintf(stderr, CANTOPEN, tmp, strerror(errno));
	    exit(1);
	case EEXIST:
	    /* If we can remove the old temporary, retry the open. */
	    if (unlink(tmp) < 0) {
		(void)fprintf(stderr, CANTUNLINK, tmp, strerror(errno));
		exit(1);
	    }
	    break;
	}

	/* wait and retry */
	if (++i >= MAX_TMP_TRY) {
	    (void)fprintf(stderr, CANTOPEN, tmp, "too many retries");
	    exit(1);
	}

	/* wait before reattempting to get tmp file */
	usleep(TMP_USEC_WAIT);
	errno = 0;
    }

    /* Write the process ID. */
    errno = 0;
    if (BinaryLock)
	ok = write(fd, (void *)pid, (size_t)sizeof pid) == sizeof pid;
    else {
	(void)sprintf(buff, "%ld\n", pid);
	i = strlen(buff);
	ok = write(fd, (void *)buff, (size_t)i) == i;
    }
    if (!ok) {
	(void)fprintf(stderr, "Can't write PID to \"%s\", %s\n",
	    tmp, strerror(errno));
	(void)close(fd);
	UnlinkAndExit(tmp, 1);
    }

    (void)close(fd);

    /* Handle the "-c" flag. */
    if (JustChecking) {
	if (ValidLock(name, true))
	    UnlinkAndExit(tmp, 1);
	UnlinkAndExit(tmp, 0);
    }

    /* Try to link the temporary to the lockfile. */
    i = 0;
    errno = 0;
    while (link(tmp, name) < 0) {
	switch (errno) {
	case EEXIST:
	    /* File exists; if lock is valid, give up. */
	    if (ValidLock(name, false))
		UnlinkAndExit(tmp, 1);
	    if (unlink(name) < 0) {
		(void)fprintf(stderr, CANTUNLINK, name, strerror(errno));
		UnlinkAndExit(tmp, 1);
	    }
	    break;
	default:
	    /* Unknown error -- give up. */
	    (void)fprintf(stderr, "Can't link \"%s\" to \"%s\", %s\n",
		    tmp, name, strerror(errno));
	    UnlinkAndExit(tmp, 1);
	    break;
	}

	/* wait and retry */
	if (++i >= MAX_TMP_TRY) {
	    (void)fprintf(stderr, "Too many link retries of \"%s\" to \"%s\"\n",
			  tmp, name);
	    UnlinkAndExit(tmp, 1);
	    /* NOTREACHED */
	}

	/* wait before reattempting to get relink the tmp file */
	if (i > 1) {
	    usleep(TMP_USEC_WAIT);
	}
	errno = 0;
    }

    UnlinkAndExit(tmp, 0);
    /* NOTREACHED */
    exit(0);
}

/*
**  Produce reliable locks for shell scripts, by Peter Honeyman as told
**  to Rich $alz.
**
** @(#) $Revision: 1.4 $
** @(#) $Id: shlock.c,v 1.4 2004/02/27 22:48:48 chongo Exp chongo $
** @(#) $Source: /usr/local/src/cmd/shlock/RCS/shlock.c,v $
*/
/*#include "configdata.h"*/
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
/*#include "clibrary.h"*/

/*
 * inn setup by hand
 */
#define STATIC static
typedef int BOOL;
typedef pid_t PID_T;
typedef size_t SIZE_T;
typedef void * POINTER;
typedef void NORETURN;
#if !defined(TRUE)
#define TRUE 1
#endif
#if !defined(FALSE)
#define FALSE 0
#endif
#define NEWSUMASK 0002
#define MAXTRY 17	/* max open retries */

STATIC BOOL	BinaryLock;
STATIC char	CANTUNLINK[] = "Can't unlink \"%s\", %s\n";
STATIC char	CANTOPEN[] = "Can't open \"%s\", %s\n";


/*
**  See if the process named in an existing lock still exists by
**  sending it a null signal.
*/
STATIC BOOL
ValidLock(name, JustChecking)
    char		*name;
    BOOL		JustChecking;
{
    register int	fd;
    register int	i;
    int			pid;
    char		buff[BUFSIZ];

    /* Open the file. */
    errno = 0;
    if ((fd = open(name, O_RDONLY)) < 0) {
	if (!JustChecking)
	    (void)fprintf(stderr, CANTOPEN, name, strerror(errno));
	return TRUE;
    }

    /* Read the PID that is written there. */
    if (BinaryLock) {
	if (read(fd, (char *)&pid, sizeof pid) != sizeof pid) {
	    (void)close(fd);
	    return FALSE;
	}
    }
    else {
	if ((i = read(fd, buff, sizeof buff - 1)) <= 0) {
	    (void)close(fd);
	    return FALSE;
	}
	buff[i] = '\0';
	pid = atoi(buff);
    }
    (void)close(fd);
    if (pid <= 1)
	return FALSE;

    /* Send the signal. */
    errno = 0;
    if (kill((PID_T)pid, 0) < 0 && errno == ESRCH)
	return FALSE;

    /* Either the kill worked, or we're optimistic about the error code. */
    return TRUE;
}


/*
**  Unlink a file, print a message on error, and exit.
*/
STATIC NORETURN
UnlinkAndExit(name, x)
    char	*name;
    int		x;
{
    errno = 0;
    if (unlink(name) < 0)
	(void)fprintf(stderr, CANTUNLINK, name, strerror(errno));
    exit(x);
}


/*
**  Print a usage message and exit.
*/
STATIC NORETURN
Usage()
{
    (void)fprintf(stderr, "Usage: shlock [-u|-b] -f file -p pid\n");
    exit(1);
    /* NOTREACHED */
}


int
main(ac, av)
    int			ac;
    char		*av[];
{
    register int	i;
    register char	*p;
    register int	fd;
    char		tmp[BUFSIZ];
    char		buff[BUFSIZ];
    char		*name;
    int			pid;
    BOOL		ok;
    BOOL		JustChecking;
    extern int		optind;
    extern char *	optarg;

    /* Set defaults. */
    pid = 0;
    name = NULL;
    JustChecking = FALSE;
    (void)umask(NEWSUMASK);

    /* Parse JCL. */
    while ((i = getopt(ac, av, "bcup:f:")) != EOF)
	switch (i) {
	default:
	    Usage();
	    /* NOTREACHED */
	case 'b':
	case 'u':
	    BinaryLock = TRUE;
	    break;
	case 'c':
	    JustChecking = TRUE;
	    break;
	case 'p':
	    pid = atoi(optarg);
	    break;
	case 'f':
	    name = optarg;
	    break;
	}
    ac -= optind;
    av += optind;
    if (ac || pid == 0 || name == NULL)
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
	if (++i >= MAXTRY) {
	    (void)fprintf(stderr, CANTOPEN, tmp, "too many retries");
	    exit(1);
	}
	usleep(1270000);	/* wait 1.27 seconds */
	errno = 0;
    }

    /* Write the process ID. */
    errno = 0;
    if (BinaryLock)
	ok = write(fd, (POINTER)pid, (SIZE_T)sizeof pid) == sizeof pid;
    else {
	(void)sprintf(buff, "%d\n", pid);
	i = strlen(buff);
	ok = write(fd, (POINTER)buff, (SIZE_T)i) == i;
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
	if (ValidLock(name, TRUE))
	    UnlinkAndExit(tmp, 1);
	UnlinkAndExit(tmp, 0);
    }

    /* Try to link the temporary to the lockfile. */
    i = 0;
    errno = 0;
    while (link(tmp, name) < 0) {
	switch (errno) {
	default:
	    /* Unknown error -- give up. */
	    (void)fprintf(stderr, "Can't link \"%s\" to \"%s\", %s\n",
		    tmp, name, strerror(errno));
	    UnlinkAndExit(tmp, 1);
	    /* NOTREACHED */
	case EEXIST:
	    /* File exists; if lock is valid, give up. */
	    if (ValidLock(name, FALSE))
		UnlinkAndExit(tmp, 1);
	    if (unlink(name) < 0) {
		(void)fprintf(stderr, CANTUNLINK, name, strerror(errno));
		UnlinkAndExit(tmp, 1);
	    }
	}

	/* wait and retry */
	if (++i >= MAXTRY) {
	    (void)fprintf(stderr, "Too many link retries of \"%s\" to \"%s\"\n",
			  tmp, name);
	    UnlinkAndExit(tmp, 1);
	    /* NOTREACHED */
	}
	usleep(1270000);	/* wait 1.27 seconds */
	errno = 0;
    }

    UnlinkAndExit(tmp, 0);
    /* NOTREACHED */
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "lockfile.h"
#include "sbuf.h"

#define LOCKFILE ".jgmenu-lockfile"
static struct sbuf lockfile;

static void lockfile_unlink(void)
{
	unlink(lockfile.buf);
}

void lockfile_init(void)
{
	struct flock fl;
	int fd;
	char *homedir;

	sbuf_init(&lockfile);
	homedir = getenv("HOME");
	if (!homedir || homedir[0] != '/')
		die("cannot find $HOME");

	sbuf_cpy(&lockfile, homedir);
	sbuf_addch(&lockfile, '/');
	sbuf_addstr(&lockfile, LOCKFILE);

	fd = open(lockfile.buf, O_RDWR | O_CREAT, 0600);
	if (fd < 0)
		die("lockfile '%s'", lockfile.buf);

	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	if (fcntl(fd, F_SETLK, &fl) < 0)
		die("an instance of 'jgmenu' is already running");
	if (close(fd) < 0)
		warn("[%s] error closing file descriptor", __FILE__);

	atexit(lockfile_unlink);
}

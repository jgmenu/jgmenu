#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "lockfile.h"
#include "sbuf.h"
#include "banned.h"

#define LOCKFILE "~/.jgmenu-lockfile"
static struct sbuf lockfile;

void lockfile_unlink(void)
{
	unlink(lockfile.buf);
}

void lockfile_init(void)
{
	int fd;
	struct stat st;

	sbuf_init(&lockfile);
	sbuf_cpy(&lockfile, LOCKFILE);
	sbuf_expand_tilde(&lockfile);
	if (!stat(lockfile.buf, &st))
		die("an instance of 'jgmenu' is already running");
	fd = open(lockfile.buf, O_RDWR | O_CREAT, 0600);
	if (fd < 0)
		die("lockfile '%s'", lockfile.buf);
	if (close(fd) < 0)
		warn("[%s] error closing file descriptor", __FILE__);
	atexit(lockfile_unlink);
}

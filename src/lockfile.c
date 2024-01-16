#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "lockfile.h"
#include "sbuf.h"
#include "banned.h"

//#define LOCKFILE "~/.jgmenu-lockfile"
#define LOCKFILE get_lockfile()

const char* get_lockfile() {
        const char *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
        static char lockfile_path[PATH_MAX];
        if (xdg_runtime_dir != NULL && xdg_runtime_dir[0] != '\0') {
                snprintf(lockfile_path, sizeof(lockfile_path), "%s/jgmenu-lockfile", xdg_runtime_dir);
                return lockfile_path;
        } else {
                return "~/.jgmenu-lockfile";
        }
}

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
	if (!stat(lockfile.buf, &st)) {
		die("Lockfile '%s' exists. This is probably because another\n"
		    "       instance of jgmenu is running. If this is not"
		    " the case, manually delete\n       the lockfile.",
		     LOCKFILE);
	}
	fd = open(lockfile.buf, O_RDWR | O_CREAT, 0600);
	if (fd < 0)
		die("lockfile '%s'", lockfile.buf);
	if (close(fd) < 0)
		warn("[%s] error closing file descriptor", __FILE__);
	atexit(lockfile_unlink);
}

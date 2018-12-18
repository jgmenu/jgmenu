#include <signal.h>

#include "spawn.h"
#include "util.h"
#include "sbuf.h"

/* voids zombie processes */
static void set_no_child_wait(void)
{
	static int done;
	static struct sigaction sigchld_action = {
		.sa_handler = SIG_DFL,
		.sa_flags = SA_NOCLDWAIT
	};

	if (done)
		return;
	sigaction(SIGCHLD, &sigchld_action, NULL);
	done = 1;
}

static void resolve_and_chdir(const char *working_dir)
{
	struct sbuf s;

	sbuf_init(&s);
	sbuf_cpy(&s, working_dir);
	sbuf_expand_tilde(&s);
	sbuf_expand_env_var(&s);
	if (chdir(s.buf) < 0)
		warn("cannot chdir into '%s'", s.buf);
	xfree(s.buf);
}

void spawn(const char *arg, const char *working_dir)
{
	const char default_shell[] = "/bin/sh";
	const char *shell = NULL;

	if (!arg)
		return;
	set_no_child_wait();
	shell = getenv("SHELL");
	if (!shell)
		shell = default_shell;
	switch (fork()) {
	case -1:
		die("unable to fork()");
		break;
	case 0:
		setsid();
		if (working_dir && working_dir[0] != '\0') {
			resolve_and_chdir(working_dir);
		} else {
			if (chdir(getenv("HOME")) < 0)
				warn("cannot chdir into $HOME");
		}
		execl(shell, shell, "-c", arg, (char *)NULL);
		exit(0);
	default:
		break;
	}
}


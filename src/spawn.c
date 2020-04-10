#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>

#include "spawn.h"
#include "util.h"
#include "sbuf.h"
#include "banned.h"

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

void spawn_async(const char *arg, const char *working_dir)
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

void spawn_sync(const char * const*command)
{
	pid_t cpid, w;
	int wstatus;

	cpid = fork();
	switch (cpid) {
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
		break;
	case 0: /* child */
		execvp(command[0], (char * const*)command);
		break;
	default: /* parent */
		do {
			w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
			if (w == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
		} while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
	}
}

static void print_argv(gchar **argv)
{
	printf("spawn ");
	for (; *argv; argv++)
		printf("%s ", *argv);
	printf("\n");
}

void spawn_async_no_shell(char const *cmd, char const *workdir)
{
	GError *err = NULL;
	gchar **argv = NULL;

	g_shell_parse_argv((gchar *)cmd, NULL, &argv, &err);
	if (err) {
		fprintf(stderr, "error parsing command\n");
		g_error_free(err);
		return;
	}
	print_argv(argv);
	g_spawn_async((gchar *)workdir, argv, NULL, G_SPAWN_SEARCH_PATH, NULL,
		      NULL, NULL, &err);
	if (err) {
		fprintf(stderr, "error spawning command %s\n", cmd);
		g_error_free(err);
	}
	g_strfreev(argv);
}

void spawn_command_line_sync(const char *command)
{
	GError *error = NULL;

	if (!command)
		return;
	g_spawn_command_line_sync(command, NULL, NULL, NULL, &error);
	if (error)
		g_warning("unable to launch: %s", error->message);
}

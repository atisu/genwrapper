#include <ctype.h>
#include "git-compat-util.h"

#ifdef BOINC
#include "boinc_api.h"

BOINC_OPTIONS boinc_options;
int boinc_init_called = 0;
#endif

int gitbox_main(int argc, char **argv);

static void prepend_to_path(const char *dir, int len)
{
	const char *old_path = getenv("PATH");
	char *path;
	int path_len = len;

	if (!old_path)
		old_path = "/usr/local/bin:/usr/bin:/bin";

	path_len = len + strlen(old_path) + 1;

	path = xmalloc(path_len + 1);

	memcpy(path, dir, len);
#ifdef __MINGW32__
	path[len] = ';';
#else
	path[len] = ':';
#endif
	memcpy(path + len + 1, old_path, path_len - len);

	setenv("PATH", path, 1);

	free(path);
}

int main(int argc, char **argv)
{
	char *cmd = argv[0];
	char *slash = strrchr(cmd, '/');
	const char *exec_path = NULL;

	/*
	 * Take the basename of argv[0] as the command
	 * name, and the dirname as the default exec_path
	 * if it's an absolute path and we don't have
	 * anything better.
	 */
	if (slash) {
		*slash++ = 0;
		if (*cmd == '/')
			exec_path = cmd;
		cmd = slash;
	}

#ifdef __MINGW32__
	slash = strrchr(cmd, '\\');
	if (slash) {
		*slash++ = 0;
		if (cmd[1] == ':')
			exec_path = cmd;
		cmd = slash;
	}
#endif
	argv[0] = cmd;

	/*
	 * We search for git commands in the following order:
	 *  - git_exec_path()
	 *  - the path of the "git" command if we could find it
	 *    in $0
	 *  - the regular PATH.
	 */
	if (exec_path)
		prepend_to_path(exec_path, strlen(exec_path));
/*	exec_path = git_exec_path();
	prepend_to_path(exec_path, strlen(exec_path)); */

	trace_printf("exec_path=%s, cmd=%s\n",exec_path,cmd);
	trace_argv_printf(argv, -1, "git-box:");
	return gitbox_main(argc, argv);
}

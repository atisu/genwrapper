#include <ctype.h>
#include "git-compat-util.h"
#include "exec_cmd.h"
#include "quote.h"

#ifdef BOINC
#include "boinc_api.h"
char buf_argv2[PATH_MAX];
char buf_argv0[PATH_MAX]; 
#endif

/* Global variable to hold the name of the command */
char *argv0_basename;

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
	path[len] = PATH_SEPARATOR;
	memcpy(path + len + 1, old_path, path_len - len);

	setenv("PATH", path, 1);

	free(path);
}

int main(int argc, char **argv)
{
	const char *exec_path = NULL;

#ifdef BOINC
    boinc_resolve_filename(argv[0], buf_argv0, PATH_MAX-1);
    argv[0] = buf_argv0;
#endif 
	/*
	 * Take the basename of argv[0] as the command
	 * name, and the dirname as the default exec_path
	 * if it's an absolute path and we don't have
	 * anything better or at least the current working directory.
	 */
	argv0_basename = strrchr(argv[0], DIRECTORY_SEPARATOR);
	if(argv0_basename) {
		*argv0_basename++ = '\0';
#ifdef __MINGW32__
		if (argv[0][1] == ':') {
#else
		if (argv[0][0] == '/') {
#endif
		/* If absolute path */
			exec_path = argv[0];
		} else {
			char *cwd;

			cwd = getcwd(NULL, 0);
			chdir(argv[0]); /* may fail but then we take cwdir */
			exec_path = getcwd(NULL, 0);
			if (cwd) {
				chdir(cwd);
				free(cwd);
			}
		}
		argv[0] = argv0_basename;
	} else {
		argv0_basename = argv[0];
	}
	if (!argv0_basename) die("Could not determine basename");

#ifdef BOINC    
    //fprintf(stderr, "in main()...\n");
    if (argc>2 && strcmp(argv[1],"sh")==0) {
		boinc_resolve_filename(argv[2], buf_argv2, PATH_MAX-1);
        argv[2]=buf_argv2;
        //fprintf(stderr, "resolved: argv[0]:%s, argv[1]%s, argv[2]:%s\n", argv[0], argv[1], argv[2]);
    }
#endif 
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

	if (getenv("GIT_TRACE")) {
		char *argv_str = sq_quote_argv((const char**)argv, -1);

		fprintf(stderr, "exec_path=%s, basename=%s\n", exec_path, argv0_basename);
		fprintf(stderr, "git-box:%s\n", argv_str);
		free(argv_str);
	}
	return gitbox_main(argc, argv);
}

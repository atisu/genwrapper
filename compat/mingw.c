#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../git-compat-util.h"
#include "run-command.h"

unsigned int _CRT_fmode = _O_BINARY;

int readlink(const char *path, char *buf, size_t bufsiz)
{
	errno = ENOSYS;
	return -1;
}

int symlink(const char *oldpath, const char *newpath)
{
	errno = ENOSYS;
	return -1;
}

int fchmod(int fildes, mode_t mode)
{
	errno = EBADF;
	return -1;
}

int lstat(const char *file_name, struct stat *buf)
{
	int namelen;
	static char alt_name[PATH_MAX];

	if (!stat(file_name, buf))
		return 0;

	/* if file_name ended in a '/', Windows returned ENOENT;
	 * try again without trailing slashes
	 */
	if (errno != ENOENT)
		return -1;
	namelen = strlen(file_name);
	if (namelen && file_name[namelen-1] != '/')
		return -1;
	while (namelen && file_name[namelen-1] == '/')
		--namelen;
	if (!namelen || namelen >= PATH_MAX)
		return -1;
	memcpy(alt_name, file_name, namelen);
	alt_name[namelen] = 0;
	return stat(alt_name, buf);
}

/* missing: link, mkstemp, fchmod, getuid (?), gettimeofday */
int socketpair(int d, int type, int protocol, int sv[2])
{
	return -1;
}
int syslog(int type, char *bufp, ...)
{
	return -1;
}
unsigned int alarm(unsigned int seconds)
{
	return 0;
}
#include <winsock2.h>
int fork()
{
	return -1;
}

int kill(pid_t pid, int sig)
{
	return -1;
}
unsigned int sleep (unsigned int __seconds)
{
	Sleep(__seconds*1000);
	return 0;
}
const char *inet_ntop(int af, const void *src,
                             char *dst, size_t cnt)
{
	return NULL;
}
int mkstemp (char *__template)
{
	char *filename = mktemp(__template);
	if (filename == NULL)
		return -1;
	return open(filename, O_RDWR | O_CREAT, 0600);
}
#if 0
int gettimeofday(struct timeval *tv, void *tz)
{
	extern time_t my_mktime(struct tm *tm);
	SYSTEMTIME st;
	struct tm tm;
	GetSystemTime(&st);
	tm.tm_year = st.wYear-1900;
	tm.tm_mon = st.wMonth-1;
	tm.tm_mday = st.wDay;
	tm.tm_hour = st.wHour;
	tm.tm_min = st.wMinute;
	tm.tm_sec = st.wSecond;
	tv->tv_sec = my_mktime(&tm);
	if (tv->tv_sec < 0)
		return -1;
	tv->tv_usec = st.wMilliseconds*1000;
	return 0;
}
#endif
int pipe(int filedes[2])
{
	int fd;
	HANDLE h[2], parent;

	if (_pipe(filedes, 8192, 0) < 0)
		return -1;

	parent = GetCurrentProcess();

	if (!DuplicateHandle (parent, (HANDLE)_get_osfhandle(filedes[0]),
			parent, &h[0], 0, FALSE, DUPLICATE_SAME_ACCESS)) {
		close(filedes[0]);
		close(filedes[1]);
		return -1;
	}
	if (!DuplicateHandle (parent, (HANDLE)_get_osfhandle(filedes[1]),
			parent, &h[1], 0, FALSE, DUPLICATE_SAME_ACCESS)) {
		close(filedes[0]);
		close(filedes[1]);
		CloseHandle(h[0]);
		return -1;
	}
	fd = _open_osfhandle(h[0], O_NOINHERIT);
	if (fd < 0) {
		close(filedes[0]);
		close(filedes[1]);
		CloseHandle(h[0]);
		CloseHandle(h[1]);
		return -1;
	}
	close(filedes[0]);
	filedes[0] = fd;
	fd = _open_osfhandle(h[1], O_NOINHERIT);
	if (fd < 0) {
		close(filedes[0]);
		close(filedes[1]);
		CloseHandle(h[1]);
		return -1;
	}
	close(filedes[1]);
	filedes[1] = fd;
	return 0;
}

int poll(struct pollfd *ufds, unsigned int nfds, int timeout)
{
	return -1;
}

#include <time.h>

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	memcpy(result, gmtime(timep), sizeof(struct tm));
	return result;
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
	memcpy(result, localtime(timep), sizeof(struct tm));
	return result;
}

#undef getcwd
char *mingw_getcwd(char *pointer, int len)
{
	char *ret = getcwd(pointer, len);
	if (!ret)
		return ret;
	if (ret[0] != 0 && ret[1] == ':') {
		int i;
		for (i = 2; ret[i]; i++)
			/* Thanks, Bill. You'll burn in hell for that. */
			if (ret[i] == '\\')
				ret[i] = '/';
	}
	return ret;
}

void sync(void)
{
}

void openlog(const char *ident, int option, int facility)
{
}

/* See http://msdn2.microsoft.com/en-us/library/17w5ykft(vs.71).aspx (Parsing C++ Command-Line Arguments */
static const char *quote_arg(const char *arg)
{
	/* count chars to quote */
	int len = 0, n = 0;
	int force_quotes = 0;
	char *q, *d;
	const char *p = arg;
	if (!*p) force_quotes = 1;
	while (*p) {
		if (isspace(*p))
			force_quotes = 1;
		else if (*p == '"')
			n++;
		else if (*p == '\\') {
			int count = 0;
			while (*p == '\\') {
				count++;
				p++;
				len++;
			}
			if (*p == '"')
				n += count*2 + 1;
			continue;
		}
		len++;
		p++;
	}
	if (!force_quotes && n == 0)
		return arg;

	/* insert \ where necessary */
	d = q = xmalloc(len+n+3);
	*d++ = '"';
	while (*arg) {
		if (*arg == '"')
			*d++ = '\\';
		else if (*arg == '\\') {
			int count = 0;
			while (*arg == '\\') {
				count++;
				*d++ = *arg++;
			}
			if (*arg == '"') {
				while (count-- > 0)
					*d++ = '\\';
				*d++ = '\\';
			}
		}
		*d++ = *arg++;
	}
	*d++ = '"';
	*d++ = 0;
	return q;
}

void quote_argv(const char **dst, const char **src)
{
	while (*src)
		*dst++ = quote_arg(*src++);
	*dst = NULL;
}

const char *parse_interpreter(const char *cmd)
{
	static char buf[100];
	char *p, *opt;
	int n, fd;

	/* don't even try a .exe */
	n = strlen(cmd);
	if (n >= 4 && !strcasecmp(cmd+n-4, ".exe"))
		return NULL;

	fd = open(cmd, O_RDONLY);
	if (fd < 0)
		return NULL;
	n = read(fd, buf, sizeof(buf)-1);
	close(fd);
	if (n < 4)	/* at least '#!/x' and not error */
		return NULL;

	if (buf[0] != '#' || buf[1] != '!')
		return NULL;
	buf[n] = '\0';
	p = strchr(buf, '\n');
	if (!p)
		return NULL;

	*p = '\0';
	if (!(p = strrchr(buf+2, '/')) && !(p = strrchr(buf+2, '\\')))
		return NULL;
	/* strip options */
	if ((opt = strchr(p+1, ' ')))
		*opt = '\0';
	return p+1;
}

static int try_shell_exec(const char *cmd, const char **argv, const char **env)
{
	const char **sh_argv;
	int n;
	const char *interpr = parse_interpreter(cmd);
	if (!interpr)
		return 0;

	/*
	 * expand
	 *    git-foo args...
	 * into
	 *    sh git-foo args...
	 */
	for (n = 0; argv[n];) n++;
	sh_argv = xmalloc((n+2)*sizeof(char*));

	/* prefer ourselves for shell */
	if (!strcmp(interpr,"sh")) {
		pid_t pid, waiting;
		int status;

		sh_argv[0] = "sh";
		sh_argv[1] = cmd;
		memcpy(&sh_argv[2], &argv[1], n*sizeof(char*));
		pid = spawnve_git_cmd(sh_argv, NULL, NULL, env);
		if (pid == -1)
			return 1;
		for (;;) {
			waiting = waitpid(pid, &status, 0);
			if (waiting < 0) {
				if (errno == EINTR)
					continue;
				return 1;
			}
			if (waiting != pid || WIFSIGNALED(status) || !WIFEXITED(status))
				return 1;
			exit(WEXITSTATUS(status));
		}
	} else {
		sh_argv[0] = interpr;
		sh_argv[1] = quote_arg(cmd);
		quote_argv(&sh_argv[2], &argv[1]);
		n = spawnvpe(_P_WAIT, interpr, sh_argv, env);

		if (n == -1)
			return 1;	/* indicate that we tried but failed */
	}
	exit(n);
}

void mingw_execve(const char *cmd, const char **argv, const char **env)
{
	/* check if git_command is a shell script */
	if (!try_shell_exec(cmd, argv, env)) {
		int ret = spawnve(_P_WAIT, cmd, argv, env);
		if (ret != -1)
			exit(ret);
	}
}

int mingw_socket(int domain, int type, int protocol)
{
	SOCKET s = WSASocket(domain, type, protocol, NULL, 0, 0);
	if (s == INVALID_SOCKET) {
		/*
		 * WSAGetLastError() values are regular BSD error codes
		 * biased by WSABASEERR.
		 * However, strerror() does not know about networking
		 * specific errors, which are values beginning at 38 or so.
		 * Therefore, we choose to leave the biased error code
		 * in errno so that _if_ someone looks up the code somewhere,
		 * then it is at least the number that are usually listed.
		 */
		errno = WSAGetLastError();
		return -1;
	}
	return s;
}
char *strsep(char **stringp, const char *delim)
{
	char *s, *old_stringp;
	if (!*stringp)
		return NULL;
	old_stringp = s = *stringp;
	while (*s) {
		if (strchr(delim, *s)) {
			*s = '\0';
			*stringp = s+1;
			return old_stringp;
		}
		s++;
	}
	*stringp = NULL;
	return old_stringp;
}
char *realpath(const char *path, char *resolved_path)
{
	return strcpy(resolved_path, path);
}
char *strptime(const char *s, const char *format, struct tm *tm)
{
	return NULL;
}
#if 0
void gitunsetenv(const char *env)
{
}
#endif

#ifndef GIT_COMPAT_UTIL_H
#define GIT_COMPAT_UTIL_H

#define _FILE_OFFSET_BITS 64

#ifndef FLEX_ARRAY
#if defined(__GNUC__) && (__GNUC__ < 3)
#define FLEX_ARRAY 0
#else
#define FLEX_ARRAY /* empty */
#endif
#endif

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#ifdef __GNUC__
#define TYPEOF(x) (__typeof__(x))
#else
#define TYPEOF(x)
#endif

#define MSB(x, bits) ((x) & TYPEOF(x)(~0ULL << (sizeof(x) * 8 - (bits))))

/* Approximation of the length of the decimal representation of this type. */
#define decimal_length(x)	((int)(sizeof(x) * 2.56 + 0.5) + 1)

#if !defined(__APPLE__) && !defined(__FreeBSD__)
#define _XOPEN_SOURCE 600 /* glibc2 and AIX 5.3L need 500, OpenBSD needs 600 for S_ISLNK() */
#define _XOPEN_SOURCE_EXTENDED 1 /* AIX 5.3L needs this */
#endif
#define _ALL_SOURCE 1
#define _GNU_SOURCE 1
#define _BSD_SOURCE 1

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <fnmatch.h>
#ifndef _WIN32
#include <sys/wait.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/select.h>
#endif
#include <assert.h>
#include <regex.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif
#ifndef NO_ETC_PASSWD
#include <netdb.h>
#include <pwd.h>
#include <inttypes.h>
#if defined(__CYGWIN__)
#undef _XOPEN_SOURCE
#include <grp.h>
#define _XOPEN_SOURCE 600
#else
#undef _ALL_SOURCE /* AIX 5.3L defines a struct list with _ALL_SOURCE. */
#include <grp.h>
#define _ALL_SOURCE 1
#endif
#endif

#ifndef NO_ICONV
#include <iconv.h>
#endif

/* On most systems <limits.h> would have given us this, but
 * not on some systems (e.g. GNU/Hurd).
 */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef PRIuMAX
#ifndef _WIN32
#define PRIuMAX "llu"
#else
#define PRIuMAX "I64u"
#endif
#endif

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#ifdef _WIN32
#define DIRECTORY_SEPARATOR  '\\'
#define PATH_SEPARATOR  ';'
#else
#define DIRECTORY_SEPARATOR  '/'
#define PATH_SEPARATOR  ':'
#endif

/* General helper functions */
extern void usage(const char *err) NORETURN;
extern void die(const char *err, ...) NORETURN __attribute__((format (printf, 1, 2)));
extern int error(const char *err, ...) __attribute__((format (printf, 1, 2)));
extern void warning(const char *err, ...) __attribute__((format (printf, 1, 2)));

extern void set_usage_routine(void (*routine)(const char *err) NORETURN);
extern void set_die_routine(void (*routine)(const char *err, va_list params) NORETURN);
extern void set_error_routine(void (*routine)(const char *err, va_list params));
extern void set_warn_routine(void (*routine)(const char *warn, va_list params));
#ifdef _WIN32
extern int vasprintf (char **result, const char *format, va_list args);
#endif

#ifdef NO_MMAP

#ifndef PROT_READ
#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_PRIVATE 1
#define MAP_FAILED ((void*)-1)
#endif

#define mmap git_mmap
#define munmap git_munmap
extern void *git_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
extern int git_munmap(void *start, size_t length);

/* This value must be multiple of (pagesize * 2) */
#define DEFAULT_PACKED_GIT_WINDOW_SIZE (1 * 1024 * 1024)

#else /* NO_MMAP */

#include <sys/mman.h>

/* This value must be multiple of (pagesize * 2) */
#define DEFAULT_PACKED_GIT_WINDOW_SIZE \
	(sizeof(void*) >= 8 \
		?  1 * 1024 * 1024 * 1024 \
		: 32 * 1024 * 1024)

#endif /* NO_MMAP */

#define DEFAULT_PACKED_GIT_LIMIT \
	((1024L * 1024L) * (sizeof(void*) >= 8 ? 8192 : 256))

#ifdef NO_PREAD
#define pread git_pread
extern ssize_t git_pread(int fd, void *buf, size_t count, off_t offset);
#endif

#ifdef NO_SETENV
#define setenv gitsetenv
extern int gitsetenv(const char *, const char *, int);
#endif

#ifdef NO_UNSETENV
#define unsetenv gitunsetenv
extern void gitunsetenv(const char *);
#endif

#ifdef NO_STRCASESTR
#define strcasestr gitstrcasestr
extern char *gitstrcasestr(const char *haystack, const char *needle);
#endif

#ifdef NO_STRTOUMAX
#include "platform.h"
#define strtoumax gitstrtoumax
extern uintmax_t gitstrtoumax(const char *, char **, int);
#endif

#ifdef NO_HSTRERROR
#define hstrerror githstrerror
extern const char *githstrerror(int herror);
#endif

/* extern void release_pack_memory(size_t, int); */
#define release_pack_memory(size_t, int)

static inline char* xstrdup(const char *str)
{
	char *ret = strdup(str);
	if (!ret) {
		release_pack_memory(strlen(str) + 1, -1);
		ret = strdup(str);
		if (!ret)
			die("Out of memory, strdup failed");
	}
	return ret;
}

static inline void *xmalloc(size_t size)
{
	void *ret = malloc(size);
	if (!ret && !size)
		ret = malloc(1);
	if (!ret) {
		release_pack_memory(size, -1);
		ret = malloc(size);
		if (!ret && !size)
			ret = malloc(1);
		if (!ret)
			die("Out of memory, malloc failed");
	}
#ifdef XMALLOC_POISON
	memset(ret, 0xA5, size);
#endif
	return ret;
}

static inline char *xstrndup(const char *str, size_t len)
{
	char *p;

	p = memchr(str, '\0', len);
	if (p)
		len = p - str;
	p = xmalloc(len + 1);
	memcpy(p, str, len);
	p[len] = '\0';
	return p;
}

static inline void *xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);
	if (!ret && !size)
		ret = realloc(ptr, 1);
	if (!ret) {
		release_pack_memory(size, -1);
		ret = realloc(ptr, size);
		if (!ret && !size)
			ret = realloc(ptr, 1);
		if (!ret)
			die("Out of memory, realloc failed");
	}
	return ret;
}

static inline void *xcalloc(size_t nmemb, size_t size)
{
	void *ret = calloc(nmemb, size);
	if (!ret && (!nmemb || !size))
		ret = calloc(1, 1);
	if (!ret) {
		release_pack_memory(nmemb * size, -1);
		ret = calloc(nmemb, size);
		if (!ret && (!nmemb || !size))
			ret = calloc(1, 1);
		if (!ret)
			die("Out of memory, calloc failed");
	}
	return ret;
}

static inline void *xmmap(void *start, size_t length,
	int prot, int flags, int fd, off_t offset)
{
	void *ret = mmap(start, length, prot, flags, fd, offset);
	if (ret == MAP_FAILED) {
		if (!length)
			return NULL;
		release_pack_memory(length, fd);
		ret = mmap(start, length, prot, flags, fd, offset);
		if (ret == MAP_FAILED)
			die("Out of memory? mmap failed: %s", strerror(errno));
	}
	return ret;
}

static inline ssize_t xread(int fd, void *buf, size_t len)
{
	ssize_t nr;
	while (1) {
		nr = read(fd, buf, len);
		if ((nr < 0) && (errno == EAGAIN || errno == EINTR))
			continue;
		return nr;
	}
}

static inline ssize_t xwrite(int fd, const void *buf, size_t len)
{
	ssize_t nr;
	while (1) {
		nr = write(fd, buf, len);
		if ((nr < 0) && (errno == EAGAIN || errno == EINTR))
			continue;
		return nr;
	}
}

static inline int xdup(int fd)
{
	int ret = dup(fd);
	if (ret < 0)
		die("dup failed: %s", strerror(errno));
	return ret;
}

static inline FILE *xfdopen(int fd, const char *mode)
{
	FILE *stream = fdopen(fd, mode);
	if (stream == NULL)
		die("Out of memory? fdopen failed: %s", strerror(errno));
	return stream;
}

static inline size_t xsize_t(off_t len)
{
	return (size_t)len;
}

static inline int has_extension(const char *filename, const char *ext)
{
	size_t len = strlen(filename);
	size_t extlen = strlen(ext);
	return len > extlen && !memcmp(filename + len - extlen, ext, extlen);
}

/* Sane ctype - no locale, and works with signed chars */
#undef isspace
#undef isdigit
#undef isalpha
#undef isalnum
#undef tolower
#undef toupper
extern unsigned char sane_ctype[256];
#define GIT_SPACE 0x01
#define GIT_DIGIT 0x02
#define GIT_ALPHA 0x04
#define sane_istest(x,mask) ((sane_ctype[(unsigned char)(x)] & (mask)) != 0)
#define isspace(x) sane_istest(x,GIT_SPACE)
#define isdigit(x) sane_istest(x,GIT_DIGIT)
#define isalpha(x) sane_istest(x,GIT_ALPHA)
#define isalnum(x) sane_istest(x,GIT_ALPHA | GIT_DIGIT)
#define tolower(x) sane_case((unsigned char)(x), 0x20)
#define toupper(x) sane_case((unsigned char)(x), 0)

static inline int sane_case(int x, int high)
{
	if (sane_istest(x, GIT_ALPHA))
		x = (x & ~0x20) | high;
	return x;
}

static inline int prefixcmp(const char *str, const char *prefix)
{
	return strncmp(str, prefix, strlen(prefix));
}

static inline int strtoul_ui(char const *s, int base, unsigned int *result)
{
	unsigned long ul;
	char *p;

	errno = 0;
	ul = strtoul(s, &p, base);
	if (errno || *p || p == s || (unsigned int) ul != ul)
		return -1;
	*result = ul;
	return 0;
}

#ifdef _WIN32

#ifndef S_ISLNK
#define S_IFLNK    0120000 /* Symbolic link */
#define S_ISLNK(x) (((x) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(x) 0
#endif

#ifndef S_ISGRP
#define S_ISGRP(x) 0
#define S_IRGRP 0
#define S_IWGRP 0
#define S_IXGRP 0
#define S_ISGID 0
#define S_IROTH 0
#define S_IXOTH 0
#endif

#define S_IFSOCK 0
#define S_ISUID 0
#define S_ISVTX 0
#define S_IRWXG 0
#define S_IRWXO 0
#define S_IWOTH 0

typedef int gid_t;
typedef int uid_t;

int readlink(const char *path, char *buf, size_t bufsiz);
int symlink(const char *oldpath, const char *newpath);
#define link symlink
int fchmod(int fildes, mode_t mode);
int lstat(const char *file_name, struct stat *buf);
char *strsep(char **stringp, const char *delim);

/* missing: link, mkstemp, fchmod, getuid (?), gettimeofday */
int socketpair(int d, int type, int protocol, int sv[2]);
int syslog(int type, char *bufp, ...);
#define LOG_ERR 1
#define LOG_INFO 2
#define LOG_DAEMON 4
unsigned int alarm(unsigned int seconds);
#include <winsock2.h>
void mingw_execve(const char *cmd, const char **argv, const char **env);
#define execve mingw_execve
int fork();
typedef int pid_t;
#define waitpid(pid, status, options) \
	((options == 0) ? _cwait((status), (pid), 0) \
		: (errno = EINVAL, -1))
#define WIFEXITED(x) ((unsigned)(x) < 259)	/* STILL_ACTIVE */
#define WEXITSTATUS(x) ((x) & 0xff)
#define WIFSIGNALED(x) ((unsigned)(x) > 259)
#define WTERMSIG(x) (x)
#define WNOHANG 1
#define ECONNABORTED 0

int kill(pid_t pid, int sig);
unsigned int sleep (unsigned int __seconds);
const char *inet_ntop(int af, const void *src,
                             char *dst, size_t cnt);
int mkstemp (char *__template);
int gettimeofday(struct timeval *tv, void *tz);
int pipe(int filedes[2]);

struct pollfd {
	int fd;           /* file descriptor */
	short events;     /* requested events */
	short revents;    /* returned events */
};
int poll(struct pollfd *ufds, unsigned int nfds, int timeout);
#define POLLIN 1
#define POLLHUP 2

static inline int git_mkdir(const char *path, int mode)
{
	return mkdir(path);
}
#define mkdir git_mkdir

static inline int git_unlink(const char *pathname) {
	/* read-only files cannot be removed */
	chmod(pathname, 0666);
	return unlink(pathname);
}
#define unlink git_unlink

static inline int git_rename(const char *oldname, const char *newname)
{
	int result;
	struct stat st;
	if (stat(oldname, &st) < 0) {
		errno = ENOENT;
		return -1;
	}
	if (S_ISDIR(st.st_mode))
		result = MoveFileEx(oldname, newname, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	else
		result = MoveFileEx(oldname, newname, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	result = result ? 0 : -1;
	if (result) /* copied from wine, should be a separate function */
		switch(GetLastError()) {
			case ERROR_ACCESS_DENIED:
			case ERROR_NETWORK_ACCESS_DENIED:
			case ERROR_CANNOT_MAKE:
			case ERROR_SEEK_ON_DEVICE:
			case ERROR_LOCK_FAILED:
			case ERROR_FAIL_I24:
			case ERROR_CURRENT_DIRECTORY:
			case ERROR_DRIVE_LOCKED:
			case ERROR_NOT_LOCKED:
			case ERROR_INVALID_ACCESS:
			case ERROR_LOCK_VIOLATION:
				errno = EACCES;
				break;
			case ERROR_FILE_NOT_FOUND:
			case ERROR_NO_MORE_FILES:
			case ERROR_BAD_PATHNAME:
			case ERROR_BAD_NETPATH:
			case ERROR_INVALID_DRIVE:
			case ERROR_BAD_NET_NAME:
			case ERROR_FILENAME_EXCED_RANGE:
			case ERROR_PATH_NOT_FOUND:
				errno = ENOENT;
				break;
			case ERROR_IO_DEVICE:
				errno = EIO;
				break;
			case ERROR_BAD_FORMAT:
				errno = ENOEXEC;
				break;
			case ERROR_INVALID_HANDLE:
				errno = EBADF;
				break;
			case ERROR_OUTOFMEMORY:
			case ERROR_INVALID_BLOCK:
			case ERROR_NOT_ENOUGH_QUOTA:
			case ERROR_ARENA_TRASHED:
				errno = ENOMEM;
				break;
			case ERROR_BUSY:
				errno = EBUSY;
				break;
			case ERROR_ALREADY_EXISTS:
			case ERROR_FILE_EXISTS:
				errno = EEXIST;
				break;
			case ERROR_BAD_DEVICE:
				errno = ENODEV;
				break;
			case ERROR_TOO_MANY_OPEN_FILES:
				errno = EMFILE;
				break;
			case ERROR_DISK_FULL:
				errno = ENOSPC;
				break;
			case ERROR_BROKEN_PIPE:
				errno = EPIPE;
				break;
			case ERROR_POSSIBLE_DEADLOCK:
				errno = EDEADLK;
				break;
			case ERROR_DIR_NOT_EMPTY:
				errno = ENOTEMPTY;
				break;
			case ERROR_BAD_ENVIRONMENT:
				errno = E2BIG;
				break;
			case ERROR_WAIT_NO_CHILDREN:
			case ERROR_CHILD_NOT_COMPLETE:
				errno = ECHILD;
				break;
			case ERROR_NO_PROC_SLOTS:
			case ERROR_MAX_THRDS_REACHED:
			case ERROR_NESTING_NOT_ALLOWED:
				errno = EAGAIN;
				break;
			default:
				/*  Remaining cases map to EINVAL */
				/* FIXME: may be missing some errors above */
				errno = EINVAL;
		}
	return result;
}
#define rename(a,b) git_rename(a,b)

#define open(P, F, M...) \
	(__builtin_constant_p(*(P)) && !strcmp(P, "/dev/null") ? \
		open("nul", F, ## M) : open(P, F, ## M))

#define fopen(P, M) \
	(!strcmp(P, "/dev/null") ? fopen("nul", M) : fopen(P, M))

#include <time.h>
struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *localtime_r(const time_t *timep, struct tm *result);
#define hstrerror strerror

char *mingw_getcwd(char *pointer, int len);
#define getcwd mingw_getcwd

int mingw_socket(int domain, int type, int protocol);
#define socket mingw_socket

#define setlinebuf(x)
#define fsync(x) 0

extern void quote_argv(const char **dst, const char **src);
extern const char *parse_interpreter(const char *cmd);

#endif /* _WIN32 */

#endif

# The default target of this Makefile is...
all:

# Define V=1 to have a more verbose compile.
#
# Define NO_OPENSSL environment variable if you do not have OpenSSL.
# This also implies MOZILLA_SHA1.
#
# Define NO_CURL if you do not have curl installed.  git-http-pull and
# git-http-push are not built, and you cannot use http:// and https://
# transports.
#
# Define CURLDIR=/foo/bar if your curl header and library files are in
# /foo/bar/include and /foo/bar/lib directories.
#
# Define NO_EXPAT if you do not have expat installed.  git-http-push is
# not built, and you cannot push using http:// and https:// transports.
#
# Define NO_D_INO_IN_DIRENT if you don't have d_ino in your struct dirent.
#
# Define NO_D_TYPE_IN_DIRENT if your platform defines DT_UNKNOWN but lacks
# d_type in struct dirent (latest Cygwin -- will be fixed soonish).
#
# Define NO_C99_FORMAT if your formatted IO functions (printf/scanf et.al.)
# do not support the 'size specifiers' introduced by C99, namely ll, hh,
# j, z, t. (representing long long int, char, intmax_t, size_t, ptrdiff_t).
# some C compilers supported these specifiers prior to C99 as an extension.
#
# Define NO_STRCASESTR if you don't have strcasestr.
#
# Define NO_STRLCPY if you don't have strlcpy.
#
# Define NO_STRTOUMAX if you don't have strtoumax in the C library.
# If your compiler also does not support long long or does not have
# strtoull, define NO_STRTOULL.
#
# Define NO_SETENV if you don't have setenv in the C library.
#
# Define NO_SYMLINK_HEAD if you never want .git/HEAD to be a symbolic link.
# Enable it on Windows.  By default, symrefs are still used.
#
# Define NO_SVN_TESTS if you want to skip time-consuming SVN interoperability
# tests.  These tests take up a significant amount of the total test time
# but are not needed unless you plan to talk to SVN repos.
#
# Define NO_FINK if you are building on Darwin/Mac OS X, have Fink
# installed in /sw, but don't want GIT to link against any libraries
# installed there.  If defined you may specify your own (or Fink's)
# include directories and library directories by defining CFLAGS
# and LDFLAGS appropriately.
#
# Define NO_DARWIN_PORTS if you are building on Darwin/Mac OS X,
# have DarwinPorts installed in /opt/local, but don't want GIT to
# link against any libraries installed there.  If defined you may
# specify your own (or DarwinPort's) include directories and
# library directories by defining CFLAGS and LDFLAGS appropriately.
#
# Define PPC_SHA1 environment variable when running make to make use of
# a bundled SHA1 routine optimized for PowerPC.
#
# Define ARM_SHA1 environment variable when running make to make use of
# a bundled SHA1 routine optimized for ARM.
#
# Define MOZILLA_SHA1 environment variable when running make to make use of
# a bundled SHA1 routine coming from Mozilla. It is GPL'd and should be fast
# on non-x86 architectures (e.g. PowerPC), while the OpenSSL version (default
# choice) has very fast version optimized for i586.
#
# Define NEEDS_SSL_WITH_CRYPTO if you need -lcrypto with -lssl (Darwin).
#
# Define NEEDS_LIBICONV if linking with libc is not enough (Darwin).
#
# Define NEEDS_SOCKET if linking with libc is not enough (SunOS,
# Patrick Mauritz).
#
# Define NO_MMAP if you want to avoid mmap.
#
# Define NO_PREAD if you have a problem with pread() system call (e.g.
# cygwin.dll before v1.5.22).
#
# Define NO_FAST_WORKING_DIRECTORY if accessing objects in pack files is
# generally faster on your platform than accessing the working directory.
#
# Define NO_TRUSTABLE_FILEMODE if your filesystem may claim to support
# the executable mode bit, but doesn't really do so.
#
# Define NO_IPV6 if you lack IPv6 support and getaddrinfo().
#
# Define NO_SOCKADDR_STORAGE if your platform does not have struct
# sockaddr_storage.
#
# Define NO_ICONV if your libc does not properly support iconv.
#
# Define OLD_ICONV if your library has an old iconv(), where the second
# (input buffer pointer) parameter is declared with type (const char **).
#
# Define NO_R_TO_GCC_LINKER if your gcc does not like "-R/path/lib"
# that tells runtime paths to dynamic libraries;
# "-Wl,-rpath=/path/lib" is used instead.
#
# Define USE_NSEC below if you want git to care about sub-second file mtimes
# and ctimes. Note that you need recent glibc (at least 2.2.4) for this, and
# it will BREAK YOUR LOCAL DIFFS! show-diff and anything using it will likely
# randomly break unless your underlying filesystem supports those sub-second
# times (my ext3 doesn't).
#
# Define USE_STDEV below if you want git to care about the underlying device
# change being considered an inode change from the update-cache perspective.
#
# Define ASCIIDOC8 if you want to format documentation with AsciiDoc 8
#

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')
uname_O := $(shell sh -c 'uname -o 2>/dev/null || echo not')
uname_R := $(shell sh -c 'uname -r 2>/dev/null || echo not')
uname_P := $(shell sh -c 'uname -p 2>/dev/null || echo not')

ifneq (,$(findstring MINGW,$(uname_S)))
	MINGW=YesPlease
endif
# CFLAGS and LDFLAGS are for the users to override from the command line.

CFLAGS = -g -O2 -Wall
LDFLAGS =
ALL_CFLAGS = $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS)
STRIP ?= strip

BOINC=no

ifeq ($(BOINC),yes)
ifdef MINGW
OPENSSLDIR=C:/Projects/genwrapper/trunk/win32/openssl/
ALL_LDFLAGS +=-LC:/Projects/boinc_mingw/ -lboinc -lstdc++ -lwinmm
ALL_CFLAGS += -IC:/Projects/boinc_mingw/include/ -DBOINC 
else
BOINC_HOME=${HOME}/svn/boinc
ALL_CFLAGS += -I$(BOINC_HOME)/api -I$(BOINC_HOME)/lib -DBOINC
ALL_LDFLAGS += -L$(BOINC_HOME)/api -lboinc_api -L$(BOINC_HOME)/lib -lboinc -lstdc++ -pthread
endif
endif

prefix = $(HOME)
bindir = $(prefix)/bin
gitexecdir = $(bindir)
sharedir = $(prefix)/share
template_dir = $(sharedir)/git-core/templates
ifeq ($(prefix),/usr)
sysconfdir = /etc
else
sysconfdir = $(prefix)/etc
endif
ETC_GITCONFIG = $(sysconfdir)/gitconfig
# DESTDIR=

export prefix bindir gitexecdir sharedir template_dir sysconfdir

CC = gcc
AR = ar
RM = rm -f
TAR = tar
INSTALL = install

# sparse is architecture-neutral, which means that we need to tell it
# explicitly what architecture to check for. Fix this up for yours..
#SPARSE_FLAGS = -D__BIG_ENDIAN__ -D__powerpc__

GITBOX=YesPlease

### --- END CONFIGURATION SECTION ---

# Those must not be GNU-specific; they are shared with perl/ which may
# be built by a different compiler. (Note that this is an artifact now
# but it still might be nice to keep that distinction.)
BASIC_CFLAGS =
BASIC_LDFLAGS =

PROGRAMS = gitbox $(EXTRA_PROGRAMS)

# Empty...
EXTRA_PROGRAMS =

# what 'all' will build and 'install' will install, in gitexecdir
ALL_PROGRAMS = $(PROGRAMS)

# Set paths to tools early so that they can be used for version tests.
ifndef SHELL_PATH
	SHELL_PATH = /bin/sh
endif

GIT_OBJS = standalone-box.o ctype.o quote.o usage.o \
	 run-command.o exec_cmd.o spawn-pipe.o

BOX_FILE = box/libbox.a

BOX_H = \
	applets.h autoconf.h busybox.h dump.h grp_.h \
	libbb.h platform.h pwd_.h shadow_.h \
	usage.h xatonum.h xregex.h

BOX_OBJS = \
	applets/applets.o \
	archival/bbunzip.o \
	archival/gzip.o \
	archival/libunarchive/check_header_gzip.o \
	archival/libunarchive/data_align.o \
	archival/libunarchive/data_extract_all.o \
	archival/libunarchive/data_extract_to_stdout.o \
	archival/libunarchive/data_skip.o \
	archival/libunarchive/decompress_bunzip2.o \
	archival/libunarchive/decompress_unzip.o \
	archival/libunarchive/filter_accept_all.o \
	archival/libunarchive/filter_accept_reject_list.o \
	archival/libunarchive/find_list_entry.o \
	archival/libunarchive/get_header_tar.o \
	archival/libunarchive/header_list.o \
	archival/libunarchive/header_skip.o \
	archival/libunarchive/init_handle.o \
	archival/libunarchive/seek_by_jump.o \
	archival/libunarchive/seek_by_read.o \
	archival/tar.o \
	archival/unzip.o \
	coreutils/basename.o \
	coreutils/cat.o \
	coreutils/chmod.o \
	coreutils/cp.o \
	coreutils/cmp.o \
	coreutils/cut.o \
	coreutils/date.o \
	coreutils/diff.o \
	coreutils/dirname.o \
	coreutils/echo.o \
	coreutils/env.o \
	coreutils/expr.o \
	coreutils/false.o \
	coreutils/head.o \
	coreutils/libcoreutils/cp_mv_stat.o \
	coreutils/ls.o \
	coreutils/md5_sha1_sum.o \
	coreutils/mkdir.o \
	coreutils/mv.o \
	coreutils/od.o \
	coreutils/printenv.o \
	coreutils/printf.o \
	coreutils/pwd.o \
	coreutils/realpath.o \
	coreutils/rmdir.o \
	coreutils/rm.o \
	coreutils/seq.o \
	coreutils/sleep.o \
	coreutils/sort.o \
	coreutils/sum.o \
	coreutils/tail.o \
	coreutils/test.o \
	coreutils/touch.o \
	coreutils/tr.o \
	coreutils/true.o \
	coreutils/uniq.o \
	coreutils/wc.o \
	coreutils/yes.o \
	debianutils/which.o  \
	editors/awk.o \
	editors/patch.o \
	editors/sed.o \
	findutils/find.o \
	findutils/grep.o \
	libbb/ask_confirmation.o \
	libbb/bb_do_delay.o \
	libbb/bb_strtonum.o \
	libbb/chomp.o \
	libbb/compare_string_array.o \
	libbb/concat_path_file.o \
	libbb/concat_subpath_file.o \
	libbb/copyfd.o \
	libbb/copy_file.o \
	libbb/crc32.o \
	libbb/default_error_retval.o \
	libbb/dump.o \
	libbb/error_msg_and_die.o \
	libbb/error_msg.o \
	libbb/execable.o \
	libbb/fclose_nonstdin.o \
	libbb/fflush_stdout_and_exit.o \
	libbb/fgets_str.o \
	libbb/full_write.o \
	libbb/get_last_path_component.o \
	libbb/get_line_from_file.o \
	libbb/getopt32.o \
	libbb/herror_msg_and_die.o \
	libbb/herror_msg.o \
	libbb/info_msg.o \
	libbb/isdirectory.o \
	libbb/last_char_is.o \
	libbb/llist.o \
	libbb/make_directory.o \
	libbb/mode_string.o \
	libbb/md5.o \
	libbb/messages.o \
	libbb/parse_mode.o \
	libbb/perror_msg_and_die.o \
	libbb/perror_msg.o \
	libbb/perror_nomsg_and_die.o \
	libbb/perror_nomsg.o \
	libbb/process_escape_sequence.o \
	libbb/read.o \
	libbb/recursive_action.o \
	libbb/remove_file.o \
	libbb/safe_strncpy.o \
	libbb/safe_write.o \
	libbb/simplify_path.o \
	libbb/skip_whitespace.o \
	libbb/trim.o \
	libbb/u_signal_names.o \
	libbb/verror_msg.o \
	libbb/vperror_msg.o \
	libbb/warn_ignoring_args.o \
	libbb/wfopen_input.o \
	libbb/wfopen.o \
	libbb/xatonum.o \
	libbb/xfuncs.o \
	libbb/xgetcwd.o \
	libbb/xreadlink.o \
	libbb/xregcomp.o \
	shell/ash.o \

ifeq ($(BOINC),yes)
BOX_OBJS += boinc/boinc.o
endif

BOX_CFLAGS = -Ibox/include -Ibox/libbb -I. -DBB_VER=\"$(GIT_VERSION)\"

BOX_H := $(patsubst %.h,box/include/%.h,$(BOX_H))
BOX_OBJS := $(patsubst %.o,box/%.o,$(BOX_OBJS))

EXTLIBS = -lz

#
# Platform specific tweaks
#

# We choose to avoid "if .. else if .. else .. endif endif"
# because maintaining the nesting to match is a pain.  If
# we had "elif" things would have been much nicer...

ifeq ($(uname_S),Linux)
	NO_STRLCPY = YesPlease
endif
ifeq ($(uname_S),GNU/kFreeBSD)
	NO_STRLCPY = YesPlease
endif
ifeq ($(uname_S),Darwin)
	NEEDS_SSL_WITH_CRYPTO = YesPlease
	NEEDS_LIBICONV = YesPlease
	OLD_ICONV = UnfortunatelyYes
	NO_STRLCPY = YesPlease
endif
ifeq ($(uname_S),SunOS)
	NEEDS_SOCKET = YesPlease
	NEEDS_NSL = YesPlease
	SHELL_PATH = /bin/bash
	NO_STRCASESTR = YesPlease
	NO_HSTRERROR = YesPlease
	ifeq ($(uname_R),5.8)
		NEEDS_LIBICONV = YesPlease
		NO_UNSETENV = YesPlease
		NO_SETENV = YesPlease
		NO_C99_FORMAT = YesPlease
		NO_STRTOUMAX = YesPlease
	endif
	ifeq ($(uname_R),5.9)
		NO_UNSETENV = YesPlease
		NO_SETENV = YesPlease
		NO_C99_FORMAT = YesPlease
		NO_STRTOUMAX = YesPlease
	endif
	INSTALL = ginstall
	TAR = gtar
	BASIC_CFLAGS += -D__EXTENSIONS__
endif
ifeq ($(uname_O),Cygwin)
ifndef MINGW
	NO_D_TYPE_IN_DIRENT = YesPlease
	NO_D_INO_IN_DIRENT = YesPlease
	NO_STRCASESTR = YesPlease
	NO_SYMLINK_HEAD = YesPlease
	NEEDS_LIBICONV = YesPlease
	NO_FAST_WORKING_DIRECTORY = UnfortunatelyYes
	NO_TRUSTABLE_FILEMODE = UnfortunatelyYes
	# There are conflicting reports about this.
	# On some boxes NO_MMAP is needed, and not so elsewhere.
	# Try commenting this out if you suspect MMAP is more efficient
	NO_MMAP = YesPlease
	NO_IPV6 = YesPlease
	X = .exe
endif
endif
ifeq ($(uname_S),FreeBSD)
	NEEDS_LIBICONV = YesPlease
	BASIC_CFLAGS += -I/usr/local/include
	BASIC_LDFLAGS += -L/usr/local/lib
endif
ifeq ($(uname_S),OpenBSD)
	NO_STRCASESTR = YesPlease
	NEEDS_LIBICONV = YesPlease
	BASIC_CFLAGS += -I/usr/local/include
	BASIC_LDFLAGS += -L/usr/local/lib
endif
ifeq ($(uname_S),NetBSD)
	ifeq ($(shell expr "$(uname_R)" : '[01]\.'),2)
		NEEDS_LIBICONV = YesPlease
	endif
	BASIC_CFLAGS += -I/usr/pkg/include
	BASIC_LDFLAGS += -L/usr/pkg/lib
	ALL_LDFLAGS += -Wl,-rpath,/usr/pkg/lib
endif
ifeq ($(uname_S),AIX)
	NO_STRCASESTR=YesPlease
	NO_STRLCPY = YesPlease
	NEEDS_LIBICONV=YesPlease
endif
ifeq ($(uname_S),IRIX64)
	NO_IPV6=YesPlease
	NO_SETENV=YesPlease
	NO_STRCASESTR=YesPlease
	NO_STRLCPY = YesPlease
	NO_SOCKADDR_STORAGE=YesPlease
	SHELL_PATH=/usr/gnu/bin/bash
	BASIC_CFLAGS += -DPATH_MAX=1024
	# for now, build 32-bit version
	BASIC_LDFLAGS += -L/usr/lib32
endif
ifdef MINGW
ifneq (,$(findstring MINGW,$(uname_S)))
	SHELL_PATH = $(shell cd /bin && pwd -W)/sh
endif
	NO_MMAP=YesPlease
	NO_PREAD=YesPlease
#	NO_OPENSSL=YesPlease
	NO_CURL=YesPlease
	NO_SYMLINK_HEAD=YesPlease
	NO_IPV6=YesPlease
	NO_ETC_PASSWD=YesPlease
	NO_SETENV=YesPlease
	NO_UNSETENV=YesPlease
	NO_STRCASESTR=YesPlease
	NO_STRLCPY=YesPlease
	NO_ICONV=YesPlease
	NO_C99_FORMAT = YesPlease
	NO_STRTOUMAX = YesPlease
	NO_SYMLINKS=YesPlease
	NO_SVN_TESTS=YesPlease
	COMPAT_CFLAGS += -DNO_ETC_PASSWD -DNO_ST_BLOCKS -DSTRIP_EXTENSION=\".exe\" -I compat
	COMPAT_OBJS += compat/mingw.o compat/fnmatch.o compat/regex.o
	EXTLIBS += -lws2_32
	X = .exe
	NOEXECTEMPL = .noexec
	template_dir = ../share/git-core/templates/
	ETC_GITCONFIG = ../etc/gitconfig
	SCRIPT_SH += cpio.sh
endif
ifneq (,$(findstring arm,$(uname_M)))
	ARM_SHA1 = YesPlease
endif

-include config.mak.autogen
-include config.mak

ifeq ($(uname_S),Darwin)
	ifndef NO_FINK
		ifeq ($(shell test -d /sw/lib && echo y),y)
			BASIC_CFLAGS += -I/sw/include
			BASIC_LDFLAGS += -L/sw/lib
		endif
	endif
	ifndef NO_DARWIN_PORTS
		ifeq ($(shell test -d /opt/local/lib && echo y),y)
			BASIC_CFLAGS += -I/opt/local/include
			BASIC_LDFLAGS += -L/opt/local/lib
		endif
	endif
endif

ifdef NO_R_TO_GCC_LINKER
	# Some gcc does not accept and pass -R to the linker to specify
	# the runtime dynamic library path.
	CC_LD_DYNPATH = -Wl,-rpath=
else
	CC_LD_DYNPATH = -R
endif

ifndef NO_CURL
	ifdef CURLDIR
		# Try "-Wl,-rpath=$(CURLDIR)/lib" in such a case.
		BASIC_CFLAGS += -I$(CURLDIR)/include
		CURL_LIBCURL = -L$(CURLDIR)/lib $(CC_LD_DYNPATH)$(CURLDIR)/lib -lcurl
	else
		CURL_LIBCURL = -lcurl
	endif
	ifndef NO_EXPAT
		EXPAT_LIBEXPAT = -lexpat
	endif
endif

ifndef NO_OPENSSL
	OPENSSL_LIBSSL = -lssl
	ifdef OPENSSLDIR
		BASIC_CFLAGS += -I$(OPENSSLDIR)/include
		OPENSSL_LINK = -L$(OPENSSLDIR)/lib $(CC_LD_DYNPATH)$(OPENSSLDIR)/lib
	else
		OPENSSL_LINK =
	endif
else
	BASIC_CFLAGS += -DNO_OPENSSL
	MOZILLA_SHA1 = 1
	OPENSSL_LIBSSL =
endif
ifdef NEEDS_SSL_WITH_CRYPTO
	LIB_4_CRYPTO = $(OPENSSL_LINK) -lcrypto -lssl
else
	LIB_4_CRYPTO = $(OPENSSL_LINK) -lcrypto
endif
ifdef NEEDS_LIBICONV
	ifdef ICONVDIR
		BASIC_CFLAGS += -I$(ICONVDIR)/include
		ICONV_LINK = -L$(ICONVDIR)/lib $(CC_LD_DYNPATH)$(ICONVDIR)/lib
	else
		ICONV_LINK =
	endif
	EXTLIBS += $(ICONV_LINK) -liconv
endif
ifdef NEEDS_SOCKET
ifndef MINGW
	EXTLIBS += -lsocket
endif
endif
ifdef NEEDS_NSL
	EXTLIBS += -lnsl
endif
ifdef NO_D_TYPE_IN_DIRENT
	BASIC_CFLAGS += -DNO_D_TYPE_IN_DIRENT
endif
ifdef NO_D_INO_IN_DIRENT
	BASIC_CFLAGS += -DNO_D_INO_IN_DIRENT
endif
ifdef NO_C99_FORMAT
	BASIC_CFLAGS += -DNO_C99_FORMAT
endif
ifdef NO_SYMLINK_HEAD
	BASIC_CFLAGS += -DNO_SYMLINK_HEAD
endif
ifdef NO_STRCASESTR
	COMPAT_CFLAGS += -DNO_STRCASESTR
	COMPAT_OBJS += compat/strcasestr.o
endif
ifdef NO_STRLCPY
	COMPAT_CFLAGS += -DNO_STRLCPY
	COMPAT_OBJS += compat/strlcpy.o
endif
ifdef NO_STRTOUMAX
	COMPAT_CFLAGS += -DNO_STRTOUMAX
	COMPAT_OBJS += compat/strtoumax.o
endif
ifdef NO_STRTOULL
	COMPAT_CFLAGS += -DNO_STRTOULL
endif
ifdef NO_SETENV
	COMPAT_CFLAGS += -DNO_SETENV
	COMPAT_OBJS += compat/setenv.o
endif
ifdef NO_UNSETENV
	COMPAT_CFLAGS += -DNO_UNSETENV
	COMPAT_OBJS += compat/unsetenv.o
endif
ifdef NO_MMAP
	COMPAT_CFLAGS += -DNO_MMAP
	COMPAT_OBJS += compat/mmap.o
endif
ifdef NO_PREAD
	COMPAT_CFLAGS += -DNO_PREAD
	COMPAT_OBJS += compat/pread.o
endif
ifdef NO_FAST_WORKING_DIRECTORY
	BASIC_CFLAGS += -DNO_FAST_WORKING_DIRECTORY
endif
ifdef NO_TRUSTABLE_FILEMODE
	BASIC_CFLAGS += -DNO_TRUSTABLE_FILEMODE
endif
ifdef NO_IPV6
	BASIC_CFLAGS += -DNO_IPV6
endif
ifdef NO_SOCKADDR_STORAGE
ifdef NO_IPV6
	BASIC_CFLAGS += -Dsockaddr_storage=sockaddr_in
else
	BASIC_CFLAGS += -Dsockaddr_storage=sockaddr_in6
endif
endif
ifdef NO_INET_NTOP
	LIB_OBJS += compat/inet_ntop.o
endif
ifdef NO_INET_PTON
	LIB_OBJS += compat/inet_pton.o
endif

ifdef NO_ICONV
	BASIC_CFLAGS += -DNO_ICONV
endif

ifdef OLD_ICONV
	BASIC_CFLAGS += -DOLD_ICONV
endif

ifdef PPC_SHA1
	SHA1_HEADER = "ppc/sha1.h"
	LIB_OBJS += ppc/sha1.o ppc/sha1ppc.o
else
ifdef ARM_SHA1
	SHA1_HEADER = "arm/sha1.h"
	LIB_OBJS += arm/sha1.o arm/sha1_arm.o
else
ifdef MOZILLA_SHA1
	SHA1_HEADER = "mozilla-sha1/sha1.h"
	LIB_OBJS += mozilla-sha1/sha1.o
else
	SHA1_HEADER = <openssl/sha.h>
	EXTLIBS += $(LIB_4_CRYPTO)
endif
endif
endif
ifdef NO_HSTRERROR
	COMPAT_CFLAGS += -DNO_HSTRERROR
	COMPAT_OBJS += compat/hstrerror.o
endif

QUIET_SUBDIR0  = +$(MAKE) -C # space to separate -C and subdir
QUIET_SUBDIR1  =

ifneq ($(findstring $(MAKEFLAGS),w),w)
PRINT_DIR = --no-print-directory
else # "make -w"
NO_SUBDIR = :
endif

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	QUIET_CC       = @echo '   ' CC $@;
	QUIET_AR       = @echo '   ' AR $@;
	QUIET_LINK     = @echo '   ' LINK $@;
	QUIET_BUILT_IN = @echo '   ' BUILTIN $@;
	QUIET_GEN      = @echo '   ' GEN $@;
	QUIET_SUBDIR0  = +@subdir=
	QUIET_SUBDIR1  = ;$(NO_SUBDIR) echo '   ' SUBDIR $$subdir; \
			 $(MAKE) $(PRINT_DIR) -C $$subdir
	export V
	export QUIET_GEN
	export QUIET_BUILT_IN
endif
endif

ifdef ASCIIDOC8
	export ASCIIDOC8
endif

# Shell quote (do not use $(call) to accommodate ancient setups);

SHA1_HEADER_SQ = $(subst ','\'',$(SHA1_HEADER))
ETC_GITCONFIG_SQ = $(subst ','\'',$(ETC_GITCONFIG))

DESTDIR_SQ = $(subst ','\'',$(DESTDIR))
bindir_SQ = $(subst ','\'',$(bindir))
gitexecdir_SQ = $(subst ','\'',$(gitexecdir))
template_dir_SQ = $(subst ','\'',$(template_dir))
prefix_SQ = $(subst ','\'',$(prefix))

SHELL_PATH_SQ = $(subst ','\'',$(SHELL_PATH))

LIBS = $(EXTLIBS)

BASIC_CFLAGS += -DSHA1_HEADER='$(SHA1_HEADER_SQ)' \
	$(COMPAT_CFLAGS)
LIB_OBJS += $(COMPAT_OBJS)

ALL_CFLAGS += $(BASIC_CFLAGS)
ALL_LDFLAGS += $(BASIC_LDFLAGS)

ifdef MINGW
ifdef NO_GITBOX
ALL_CFLAGS += -DNO_GITBOX
else
ALL_CFLAGS += $(BOX_CFLAGS)
ALL_LDFLAGS += $(BOX_LDFLAGS)
endif
endif

ifdef GITBOX
ALL_CFLAGS += $(BOX_CFLAGS) -std=gnu99 -D_GNU_SOURCE -DGITBOX
ALL_LDFLAGS += $(BOX_LDFLAGS) -lm
endif
export TAR INSTALL DESTDIR SHELL_PATH


### Build rules

all: $(ALL_PROGRAMS)

strip: $(PROGRAMS)
	$(STRIP) $(STRIP_OPTS) $(PROGRAMS)

configure: configure.ac
	$(QUIET_GEN)$(RM) $@ $<+ && \
	sed -e 's/@@GIT_VERSION@@/$(GIT_VERSION)/g' \
	    $< > $<+ && \
	autoconf -o $@ $<+ && \
	$(RM) $<+

%.o: %.c 
	$(QUIET_CC)$(CC) -o $*.o -c $(ALL_CFLAGS) $<
%.s: %.c 
	$(QUIET_CC)$(CC) -S $(ALL_CFLAGS) $<
%.o: %.S
	$(QUIET_CC)$(CC) -o $*.o -c $(ALL_CFLAGS) $<

exec_cmd.o: exec_cmd.c 
	$(QUIET_CC)$(CC) -o $*.o -c $(ALL_CFLAGS) '-DGIT_EXEC_PATH="$(gitexecdir_SQ)"' $<

config.o: config.c 
	$(QUIET_CC)$(CC) -o $*.o -c $(ALL_CFLAGS) -DETC_GITCONFIG='"$(ETC_GITCONFIG_SQ)"' $<

$(BOX_OBJS): $(BOX_H) git-compat-util.h exec_cmd.h quote.h run-command.h spawn-pipe.h
box/shell/ash.o: box/shell/ash_fork.c box/shell/ash_fork.h

$(BOX_FILE): $(BOX_OBJS)
	$(QUIET_AR)rm -f $@ && $(AR) rcs $@ $(BOX_OBJS)

gitbox: $(GIT_OBJS) $(BOX_FILE)
	$(QUIET_LINK)$(CC) $(ALL_CFLAGS) -o $@ $^ $(ALL_LDFLAGS) $(LIBS)


### Cleaning rules

clean:
	$(RM) $(ALL_PROGRAMS)
	$(RM) $(BOX_OBJS) $(GIT_OBJS) $(BOX_FILE)
	$(RM) *.spec *.pyc *.pyo */*.pyc */*.pyo common-cmds.h TAGS tags
	$(RM) -r autom4te.cache
	$(RM) configure config.log config.mak.autogen config.mak.append config.status config.cache
	$(RM) *~

.PHONY: all install clean strip

/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"

/* allow default system PATH to be extended via CFLAGS */
#ifndef BB_ADDITIONAL_PATH
#define BB_ADDITIONAL_PATH ""
#endif

/* allow version to be extended, via CFLAGS */
#ifndef BB_EXTRA_VERSION
#define BB_EXTRA_VERSION BB_BT
#endif

#define BANNER "BusyBox v" BB_VER " (" BB_EXTRA_VERSION ")"

const char bb_banner[] ALIGN1 = BANNER;


const char bb_msg_memory_exhausted[] ALIGN1 = "memory exhausted";
const char bb_msg_invalid_date[] ALIGN1 = "invalid date '%s'";
const char bb_msg_write_error[] ALIGN1 = "write error";
const char bb_msg_read_error[] ALIGN1 = "read error";
const char bb_msg_unknown[] ALIGN1 = "(unknown)";
const char bb_msg_can_not_create_raw_socket[] ALIGN1 = "can't create raw socket";
const char bb_msg_perm_denied_are_you_root[] ALIGN1 = "permission denied. (are you root?)";
const char bb_msg_requires_arg[] ALIGN1 = "%s requires an argument";
const char bb_msg_invalid_arg[] ALIGN1 = "invalid argument '%s' to '%s'";
const char bb_msg_standard_input[] ALIGN1 = "standard input";
const char bb_msg_standard_output[] ALIGN1 = "standard output";

const char bb_str_default[] ALIGN1 = "default";
const char bb_hexdigits_upcase[] ALIGN1 = "0123456789ABCDEF";

const char bb_path_passwd_file[] ALIGN1 = "/etc/passwd";
const char bb_path_shadow_file[] ALIGN1 = "/etc/shadow";
const char bb_path_group_file[] ALIGN1 = "/etc/group";
const char bb_path_gshadow_file[] ALIGN1 = "/etc/gshadow";
const char bb_path_motd_file[] ALIGN1 = "/etc/motd";
#ifdef __MINGW32__
const char bb_dev_null[] ALIGN1 = "nul";
#else
const char bb_dev_null[] ALIGN1 = "/dev/null";
#endif
const char bb_default_login_shell[] ALIGN1 = LIBBB_DEFAULT_LOGIN_SHELL;
/* util-linux manpage says /sbin:/bin:/usr/sbin:/usr/bin,
 * but I want to save a few bytes here. Check libbb.h before changing! */
const char bb_PATH_root_path[] ALIGN1 =
	"PATH=/sbin:/usr/sbin:/bin:/usr/bin" BB_ADDITIONAL_PATH;


const int const_int_1 = 1;
/* explicitly = 0, otherwise gcc may make it a common variable
 * and it will end up in bss */
const int const_int_0 = 0;

#ifndef __MINGW32__
#include <utmp.h>
/* This is usually something like "/var/adm/wtmp" or "/var/log/wtmp" */
const char bb_path_wtmp_file[] ALIGN1 =
#if defined _PATH_WTMP
	_PATH_WTMP;
#elif defined WTMP_FILE
	WTMP_FILE;
#else
#error unknown path to wtmp file
#endif
#endif /* __MINGW32__ */

/* We use it for "global" data via *(struct global*)&bb_common_bufsiz1.
 * Since gcc insists on aligning struct global's members, it would be a pity
 * (and an alignment fault on some CPUs) to mess it up. */
char bb_common_bufsiz1[COMMON_BUFSIZE] __attribute__(( aligned(sizeof(long long)) ));

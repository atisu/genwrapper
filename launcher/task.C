//
// Marosi Attila Csaba <atisu@sztaki.hu>
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// original by Andrew J. Younge (ajy4490@umiacs.umd.edu)
//

#include <stdio.h>
#include <errno.h>
#include <vector>
#include <string>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <unistd.h>
#include <sys/wait.h>
#endif // _WIN32
#include "common.h"
#ifdef WANT_DCAPI
#include "dc_client.h"
#endif // WANT_DCAPI
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "gw_common.h"
#include "task.h"

#define POLL_PERIOD 1.0


#ifdef _WIN32
// CreateProcess() takes HANDLEs for the stdin/stdout.
// We need to use CreateFile() to get them.  Ugh.
HANDLE win_fopen(const char* path, const char* mode) {
  SECURITY_ATTRIBUTES sa;
  memset(&sa, 0, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  if (!strcmp(mode, "r")) {
    return CreateFile(
		      path,
		      GENERIC_READ,
		      FILE_SHARE_READ,
		      &sa,
		      OPEN_EXISTING,
		      0, 
		      0
		      );
  } else if (!strcmp(mode, "w")) {
    return CreateFile(
		      path,
		      GENERIC_WRITE,
		      FILE_SHARE_WRITE,
		      &sa,
		      OPEN_ALWAYS,
		      0, 
		      0
		      );
  } else if (!strcmp(mode, "a")) {
    HANDLE hAppend = CreateFile(
				path,
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				&sa,
				OPEN_ALWAYS,
				0, 
				0
				);
    SetFilePointer(hAppend, 0, NULL, FILE_END);
    return hAppend;
  } else {
    return 0;
  }
}
#endif


int TASK::run(vector<string> &args) {
#ifdef _WIN32
  PROCESS_INFORMATION process_info;
  STARTUPINFO startup_info;
  string command;

  ZeroMemory(&startup_info, sizeof(startup_info));
  ZeroMemory(&process_info, sizeof(process_info));
  startup_info.cb = sizeof(startup_info);
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  // we need to redirect stdout/ stderr to somewhere or they'll
  // get lost
  startup_info.hStdError = win_fopen("stderr.txt", "a");
  startup_info.hStdOutput = win_fopen("stdout.txt", "a");
  startup_info.hStdInput = NULL;

  for (vector<string>::const_iterator it = args.begin(); it != args.end(); it++)
    command += (*it) + " ";

  if (!CreateProcess(
		     NULL, 
		     (LPSTR)command.c_str(),
		     NULL,
		     NULL,
		     true,		// bInheritHandles
		     CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
		     NULL,
		     NULL,
		     &startup_info,
		     &process_info
		     )) {
    gw_do_log(LOG_ERR, "CreateProcess failed (%d)\n", GetLastError()); 
    return ERR_EXEC;
  }
  pid_handle = process_info.hProcess;
  thread_handle = process_info.hThread;
  SetThreadPriority(thread_handle, THREAD_PRIORITY_IDLE);
  suspended = false;
  wall_cpu_time = 0;
#else
  pid = fork();
  if (pid == -1) {
    gw_do_log(LOG_ERR, "fork() failed: %s", strerror(errno));
    gw_finish(ERR_FORK);
  }
  if (pid == 0) {
    // we're in the child process here
    //
    // NOTE: if the application is restartable,
    // we should deal with atomicity somehow
    //

    const char **argv = (const char **)malloc(sizeof(*argv) * args.size() + 1);
    size_t i;
    for (i = 0; i < args.size(); i++)
      argv[i] = args.at(i).c_str();
    argv[i] = NULL;

    execv(argv[0], (char *const *)argv);
    gw_do_log(LOG_ERR, "Could not execute '%s': %s", argv[0], strerror(errno));
    exit(ERR_EXEC);
  }
#endif
  return 0;
}

bool TASK::poll(int& status) {
#ifdef _WIN32
  unsigned long exit_code;
  if (GetExitCodeProcess(pid_handle, &exit_code)) {
    if (exit_code != STILL_ACTIVE) {
      status = exit_code;
      // trivial validator needs cpu_time > 0
      final_cpu_time = 1;
      return true;
    }
  }
  if (!suspended) 
    wall_cpu_time += POLL_PERIOD;
#else
  int wpid, wait_status;
  struct rusage ru;

  wpid = wait4(pid, &wait_status, WNOHANG, &ru);
  if (wpid) {
    if (WIFSIGNALED(wait_status))
      status = 255;
    else
      status = WEXITSTATUS(wait_status);
    // trivial validator needs cpu_time > 0
    final_cpu_time = 1;
    return true;
  }
#endif
  return false;
}

// 
// kill/ stop/ resume won't affect any process started
// by gitbox
//

void TASK::kill() {
#ifdef _WIN32
  TerminateProcess(pid_handle, -1);
#else
  ::kill(pid, SIGKILL);
#endif
}


void TASK::stop() {
#ifdef _WIN32
  SuspendThread(thread_handle);
  suspended = true;
#else
  ::kill(pid, SIGSTOP);
#endif
}


void TASK::resume() {
#ifdef _WIN32
  ResumeThread(thread_handle);
  suspended = false;
#else
  ::kill(pid, SIGCONT);
#endif
}




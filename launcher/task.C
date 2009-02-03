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

#ifdef _WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include "boinc_win.h"
#include <process.h>
#include <Tlhelp32.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <syslog.h>
#endif // _WIN32
#include "common.h"
#include "str_util.h"
#include "util.h"
#include "app_ipc.h"
#include "error_numbers.h"
#include "gw_common.h"
#include "task.h"

#ifdef _WIN32

// for SuspendProcess/ ResumeProcess
typedef LONG (NTAPI *_NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG (NTAPI *_NtResumeProcess)(IN HANDLE ProcessHandle);

_NtSuspendProcess NtSuspendProcess;
_NtResumeProcess NtResumeProcess;

// CreateProcess() takes HANDLEs for the stdin/stdout.
// We need to use CreateFile() to get them.  Ugh.
HANDLE win_fopen(const char* path, const char* mode) {
  std::string path_ = gw_resolve_filename(path);
  SECURITY_ATTRIBUTES sa;
  memset(&sa, 0, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  if (!strcmp(mode, "r")) {
    return CreateFile(
		      path_.c_str(),
		      GENERIC_READ,
		      FILE_SHARE_READ,
		      &sa,
		      OPEN_EXISTING,
		      0, 
		      0
		      );
  } else if (!strcmp(mode, "w")) {
    return CreateFile(
		      path_.c_str(),
		      GENERIC_WRITE,
		      FILE_SHARE_WRITE | FILE_SHARE_READ,
		      &sa,
		      OPEN_ALWAYS,
		      0, 
		      0
		      );
  } else if (!strcmp(mode, "a")) {
    HANDLE hAppend = CreateFile(
				path_.c_str(),
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
    return INVALID_HANDLE_VALUE;
  }
}


// not used currently
void killProcessesInJob(HANDLE hJobObject_) {
  JOBOBJECT_BASIC_PROCESS_ID_LIST PidList;
  unsigned int i;
  HANDLE hOpenProcess;
  if (!QueryInformationJobObject(hJobObject_,  (JOBOBJECTINFOCLASS)3, &PidList,
				 sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST)*2, NULL)) {
    gw_do_log(LOG_ERR, "%s: failed to query information (%ld)\n", __FUNCTION__, (long)GetLastError());
  } else {
    for (i=0; i<PidList.NumberOfProcessIdsInList; i++) {
      hOpenProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PidList.ProcessIdList[i]);
      if (hOpenProcess == NULL) {
	gw_do_log(LOG_ERR, "%s: cannot open process", __FUNCTION__);
      } else {  
	if (!TerminateProcess(hOpenProcess, 2)) {
	  gw_do_log(LOG_ERR, "%s: cannot kill process %d (%ld)", PidList.ProcessIdList[i], 
		    __FUNCTION__, (long)GetLastError()); 
	}
	CloseHandle(hOpenProcess);
      }
    }
  }
}


void controlProcessesInJob(HANDLE hJobObject_, BOOL bSuspend) {
  JOBOBJECT_BASIC_PROCESS_ID_LIST PidList;
  unsigned int i;
  HANDLE hOpenProcess;
  if (!QueryInformationJobObject(hJobObject_, (JOBOBJECTINFOCLASS)3, &PidList,
				 sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST)*2, 
				 NULL)) {
    gw_do_log(LOG_ERR, "%s: failed to query information (%ld)", __FUNCTION__, (long)GetLastError());
  } else {
    for (i=0; i<PidList.NumberOfProcessIdsInList; i++) {
      hOpenProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PidList.ProcessIdList[i]);
      if (hOpenProcess == NULL) {
	gw_do_log(LOG_ERR, "%s: cannot open process", __FUNCTION__);
      } else {  
	if (bSuspend) {
	  NtSuspendProcess = 0;
	  NtSuspendProcess = (_NtSuspendProcess) 
	    GetProcAddress( GetModuleHandle( "ntdll" ), "NtSuspendProcess" );
	  if (NtSuspendProcess)
	    NtSuspendProcess(hOpenProcess);
	  else 
	    gw_do_log(LOG_ERR, "%s: cannot import NtSuspendProcess() from ntdll", __FUNCTION__);	  
	} else {
	  NtResumeProcess = 0;
	  NtResumeProcess = (_NtResumeProcess) 
	    GetProcAddress( GetModuleHandle( "ntdll" ), "NtResumeProcess" );
	  if (NtResumeProcess)
	    NtResumeProcess(hOpenProcess);
	  else 
	    gw_do_log(LOG_ERR, "%s: cannot import NtResumeProcess() from ntdll", __FUNCTION__);
	}
	CloseHandle(hOpenProcess);
      }
    }
  }
} 

void suspendProcessesInJob(HANDLE hJobObject_) {
  controlProcessesInJob(hJobObject_, TRUE);
}


void resumeProcessesInJob(HANDLE hJobObject_) {
  controlProcessesInJob(hJobObject_, FALSE);
}


bool addProcessesToJobObject(HANDLE hJobObject_) {
  PROCESSENTRY32 pe32;
  JOBOBJECT_BASIC_PROCESS_ID_LIST PidList;
  unsigned int i;
  HANDLE hOpenProcess = INVALID_HANDLE_VALUE;
  HANDLE hProcessSnap = INVALID_HANDLE_VALUE;

  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE)
    return false;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  if( !Process32First( hProcessSnap, &pe32 ) ) {
    gw_do_log(LOG_ERR, "%s: Process32First() failed", __FUNCTION__ );
    CloseHandle(hProcessSnap);
    return false;
  }
  if (!QueryInformationJobObject(hJobObject_, (JOBOBJECTINFOCLASS)3, &PidList, 
				 sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST)*2, 
				 NULL)) {
    gw_do_log(LOG_ERR, "%s: failed to query information (%ld)", __FUNCTION__, (long)GetLastError());
    CloseHandle(hProcessSnap);
    return false;
  }
  do {
      for (i=0; i<PidList.NumberOfProcessIdsInList; i++) {
	if (pe32.th32ParentProcessID == PidList.ProcessIdList[i]) {
	  hOpenProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pe32.th32ProcessID);
	  if (hOpenProcess == NULL) {
	    gw_do_log(LOG_ERR, "%s: cannot open process", __FUNCTION__);
	    CloseHandle(hProcessSnap);
	    return false;
	  }
	  if (!AssignProcessToJobObject(hJobObject_, hOpenProcess)) {
	    CloseHandle(hProcessSnap);
	    CloseHandle(hOpenProcess);
	    return false;
	  } else {
	    gw_do_log(LOG_DEBUG, "%s: added process %d to JobObject", 
		      __FUNCTION__, PidList.ProcessIdList[i]);
	  }
	  CloseHandle(hOpenProcess);
	} 
      }
  } while(Process32Next(hProcessSnap, &pe32));
  CloseHandle(hProcessSnap);
  return true;
}


void listProcessesInJob(HANDLE hJobObject) {
  JOBOBJECT_BASIC_PROCESS_ID_LIST PidList;
  unsigned int i;

  if (!QueryInformationJobObject(hJobObject, (JOBOBJECTINFOCLASS)3, &PidList, 
				 sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST)*2, 
				 NULL)) {
    gw_do_log(LOG_ERR, "%s: failed to query information (%ld)", __FUNCTION__, (long)GetLastError());
  } else {
    gw_do_log(LOG_DEBUG, "%s: Process id-s in the JobObject:", __FUNCTION__);
    for (i=0; i<PidList.NumberOfProcessIdsInList; i++) {
      gw_do_log(LOG_DEBUG, "%s: %ld", __FUNCTION__, PidList.ProcessIdList[i]);
    }
  }
}
#endif


TASK::TASK() {
  frac_done = 0.0;
  final_cpu_time = 0;
  wall_cpu_time = 0;
}


int TASK::run(vector<string> &args) {
#ifdef _WIN32
  PROCESS_INFORMATION process_info;
  STARTUPINFO startup_info;
  SECURITY_ATTRIBUTES SecAttrs;
  string command;

  ZeroMemory(&SecAttrs, sizeof(SECURITY_ATTRIBUTES));
  SecAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
  SecAttrs.bInheritHandle = TRUE;
  SecAttrs.lpSecurityDescriptor = NULL;
  // create a JobObject without a name to avoid collosions
  hJobObject=CreateJobObject(&SecAttrs, NULL);

  ZeroMemory(&startup_info, sizeof(startup_info));
  ZeroMemory(&process_info, sizeof(process_info));
  startup_info.cb = sizeof(startup_info);
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  // we need to redirect stdout/ stderr to somewhere or they'll
  // get lost. we redirect them to the standard boinc stdout/ stderr,
  // and dc-api will copy them to its stderr/stdout files before exit.
  startup_info.hStdError = win_fopen(STDERR_FILE, "w");
  startup_info.hStdOutput = win_fopen(STDOUT_FILE, "w");
  startup_info.hStdInput = NULL;

  for (vector<string>::const_iterator it = args.begin(); it != args.end(); it++)
    command += (*it) + " ";

  if (!CreateProcess(
		     NULL, 
		     (LPSTR)command.c_str(),
		     NULL,
		     NULL,
		     true,		// bInheritHandles
		     CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
		     NULL,
		     NULL,
		     &startup_info,
		     &process_info
		     )) {
    gw_do_log(LOG_ERR, "CreateProcess failed (%ld)", (long)GetLastError()); 
    return ERR_EXEC;
  }
  hProcess = process_info.hProcess;
  hThread = process_info.hThread;
  SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
  if (!AssignProcessToJobObject(hJobObject, hProcess)) {
    gw_do_log(LOG_ERR, "failed to add current process to the JobObject");
  }
  gw_do_log(LOG_DEBUG, "CreateProcess returns %d", process_info.dwProcessId);
  suspended = false;
#else
  pid = fork();
  if (pid == -1) {
    gw_do_log(LOG_ERR, "fork() failed: %s", strerror(errno));
    gw_finish(ERR_FORK);
  }
  if (pid == 0) {
    // we're in the child process here
    //
    // create a new process group with the id of the child process, so we can control all
    // (future) child processes from the parent.
    // NOTE: when job control is enabled in gitbox, it will create new process groups for its subshells ;(
    if (setpgid(getpid(), getpid()) == -1) {
      gw_do_log(LOG_ERR, "process id and the new process group id does not match !! (%d/%d)", 
		getpgid(0), getpid());
      exit(ERR_EXEC);
    }
    const char **argv = (const char **)malloc(sizeof(*argv) * (args.size() + 1));
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
  double frac_done_t = gw_read_fraction_done();
  if (frac_done_t > frac_done)
    frac_done = frac_done_t;
#ifdef _WIN32
  unsigned long exit_code;
  JOBOBJECT_BASIC_ACCOUNTING_INFORMATION Rusage;

  addProcessesToJobObject(hJobObject);
  if (GetExitCodeProcess(hProcess, &exit_code)) {
    if (!QueryInformationJobObject(hJobObject, (JOBOBJECTINFOCLASS)1, &Rusage, 
				   sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION), 
				   NULL)) {
      gw_do_log(LOG_ERR, "failed to query information on JobObject (%ld)", 
		(long)GetLastError());
    } else {
      final_cpu_time = Rusage.TotalUserTime.QuadPart / 10000000;
    }
    gw_report_status(final_cpu_time, frac_done, false);
    if (exit_code != STILL_ACTIVE) {
      status = exit_code;
      return true;
    }
  }  
#else
  int wpid, wait_status;
  struct rusage ru;
  wpid = wait4(-pid, &wait_status, WNOHANG, &ru);
  if (wpid) {
    if (WIFSIGNALED(wait_status))
      status = 255;
    else
      status = WEXITSTATUS(wait_status);
    final_cpu_time = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;
    gw_report_status(final_cpu_time, frac_done, false);       
    return true;
  }
  if (!suspended)
    wall_cpu_time += POLL_PERIOD;
  gw_report_status(wall_cpu_time, frac_done, false);       
#endif
  return false;
}


void TASK::kill() {
#ifdef _WIN32
  // should be -1 ?
  TerminateJobObject(hJobObject, 1);
#else
  ::killpg(pid, SIGKILL);
#endif
}


void TASK::stop() {
#ifdef _WIN32
  ::suspendProcessesInJob(hJobObject);  
#else
  ::killpg(pid, SIGSTOP);
#endif
  suspended = true;
}


void TASK::resume() {
#ifdef _WIN32
  ::resumeProcessesInJob(hJobObject);  
#else
  ::killpg(pid, SIGCONT);
#endif
  suspended = false;
}

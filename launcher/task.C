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
#endif
#include "common.h"
#ifdef WANT_DCAPI
#endif
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "gw_common.h"
#include "task.h"

#define POLL_PERIOD 1.0

int TASK::run(int argct, char** argvt) {
    string interpreter_path, 
        script_path,
        stdout_path, 
        stdin_path, 
        stderr_path,
        app_path;

    // interpreter_path and script_path do not need to be resolved
    interpreter_path = interpreter;
    script_path = script;
    app_path = interpreter_path;

    // append wrapper's command-line arguments to those in the job file.
    //
    for (int i=2; i<argct; i++){
        command_line += argvt[i];
        if ((i+1) < argct){
            command_line += string(" ");
        }
    }

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    string command;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    command = app_path+ string(" sh ")+ script_path+ 
        string(" ")+ command_line;

    // pass std handles to app
    //
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    if (stdin_filename != "") {
        stdin_path = gw_resolve_filename(stdin_filename.c_str());
	startup_info.hStdInput = win_fopen(stdin_path.c_str(), "r");
    }
    if (stdout_filename != "") {
        stdout_path = gw_resolve_filename(stdout_filename.c_str());
	startup_info.hStdOutput = win_fopen(stdout_path.c_str(), "w");
    }
    if (stderr_filename != "") {
        stderr_path = gw_resolve_filename(stderr_filename.c_str());
        startup_info.hStdError = win_fopen(stderr_path.c_str(), "w");
    }/* else {
        startup_info.hStdError = win_fopen(STDERR_FILE, "a");
    }*/
    if (!CreateProcess(
        app_path.c_str(),
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
        return ERR_EXEC;
    }
    pid_handle = process_info.hProcess;
    thread_handle = process_info.hThread;
    SetThreadPriority(thread_handle, THREAD_PRIORITY_IDLE);
    suspended = false;
    wall_cpu_time = 0;
#else
    int retval, argc;
    char buf[256];
    char* argv[256];
    char arglist[4096];
	FILE* stdout_file;
	FILE* stdin_file;
	FILE* stderr_file;

    pid = fork();
    if (pid == -1) {
        gw_do_log("ERROR: fork() failed");
        gw_finish(ERR_FORK);
    }
    if (pid == 0) {
        // we're in the child process here
        //
        // open stdout, stdin if file names are given
        // NOTE: if the application is restartable,
        // we should deal with atomicity somehow
	//
	if (stdin_filename != "") {
            stdin_path = gw_resolve_filename(stdin_filename);
            stdin_file = freopen(stdin_path.c_str(), "r", stdin);
            if (!stdin_file) return ERR_FOPEN;
	}
	if (stdout_filename != "") {
            stdout_path = gw_resolve_filename(stdout_filename);
            stdout_file = freopen(stdout_path.c_str(), "w", stdout);
            if (!stdout_file) return ERR_FOPEN;
	}
        if (stderr_filename != "") {
            stderr_path = gw_resolve_filename(stderr_filename);
            stderr_file = freopen(stderr_path.c_str(), "w", stderr);
            if (!stderr_file) return ERR_FOPEN;
        }
		// construct argv
        // TODO: use malloc instead of stack var
        //
        strcpy(buf, interpreter_path.c_str());
        argv[0] = buf; 
        argv[1] = strdup("sh");
        argv[2] = (char *)script_path.c_str();        
        strncpy(arglist, command_line.c_str(), sizeof(arglist));
        argc = parse_command_line(arglist, argv+3);
        /*
        fprintf(stderr, "wrapper: running %s %s %s (%s)\n", argv[0], argv[1], argv[2], arglist);
        setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY);
        */
        retval = execv(buf, argv);
        gw_do_log("ERROR: could not execute '%s': %s", argv[0], strerror(errno));
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
            //final_cpu_time = cpu_time();
            // trivial validator needs cpu_time > 0
			final_cpu_time = 1;
			return true;
        }
    }
    if (!suspended) wall_cpu_time += POLL_PERIOD;
#else
    int wpid;
    struct rusage ru;

    wpid = wait4(pid, &status, WNOHANG, &ru);
    if (wpid) {
        //final_cpu_time = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;
        // trivial validator needs cpu_time > 0
		final_cpu_time = 1;
        return true;
    }
#endif
    return false;
}

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

/*
double TASK::cpu_time() {
#ifdef _WIN32
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER tKernel, tUser;
    LONGLONG totTime;

    int retval = GetProcessTimes(
        pid_handle, &creation_time, &exit_time, &kernel_time, &user_time
    );
    if (retval == 0) {
        return wall_cpu_time;
    }

    tKernel.LowPart  = kernel_time.dwLowDateTime;
    tKernel.HighPart = kernel_time.dwHighDateTime;
    tUser.LowPart    = user_time.dwLowDateTime;
    tUser.HighPart   = user_time.dwHighDateTime;
    totTime = tKernel.QuadPart + tUser.QuadPart;

    return totTime / 1.e7;
#else
    return  linux_cpu_time(pid);
#endif
}
*/

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

#include <stdio.h>
#include <libgen.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <errno.h>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <syslog.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32
#include "boinc_api.h"
#include "diagnostics.h" 
// for boinc_sleep()
#include "util.h"
// for parse_command_line()
#include "str_util.h"
#include "gw_common.h"
// box/common.h
#include "common.h"
#include "task.h"
#include "error_numbers.h"

#ifdef _WIN32
#define GENWRAPPER_EXE   "gitbox.exe"
#else
#define GENWRAPPER_EXE   "./gitbox"
#endif // _WIN32
#define PROFILE_SCRIPT   "profile.sh"
#define EXEC_SCRIPT      "gw_tmp.sh"
#ifndef _MAX_PATH
#define _MAX_PATH        255
#endif


#ifdef WANT_DCAPI
static const char* dc_files[] = {
  DC_LABEL_STDOUT,
  DC_LABEL_STDERR,
  DC_LABEL_CLIENTLOG,
  CKPT_LABEL_OUT,
  NULL
};
#endif

const char* WU_SCRIPT = "wu_script.sh";

// from unzip.c in libbb
extern "C" {
  int unzip_main(int argc, char **argv);
}


void poll_boinc_messages(TASK& task) {
  BOINC_STATUS status;
  boinc_get_status(&status);
  if (status.no_heartbeat) {
    task.kill();
    exit(0);
  }
  if (status.quit_request) {
    task.kill();
    exit(0);
  }
  if (status.abort_request) {
    task.kill();
    exit(0);
  }
  if (status.suspended) {
    if (!task.suspended) {
      task.stop();
    }
  } else {
    if (task.suspended) {
      task.resume();
    }
  }
}


int main(int argc, char* argv[]) {
  BOINC_OPTIONS options;

  gw_init();

#ifdef WANT_DCAPI
  boinc_init_diagnostics(BOINC_DIAG_REDIRECTSTDERR | BOINC_DIAG_REDIRECTSTDOUT);
#endif
  gw_do_log(LOG_INFO, "Launcher for GenWrapper (build date %s, %s)", __DATE__, SVNREV);

  memset(&options, 0, sizeof(options));
  options.main_program = true;
  options.check_heartbeat = true;
  options.handle_process_control = true;
  options.send_status_msgs = false;
  boinc_init_options(&options);

#ifdef WANT_DCAPI
  gw_do_log(LOG_INFO, "DC-API enabled version");
  // need to create various files expected by DC-API
  // in case the application fails, DC-API still expects them
  std::string dc_filename_resolved;
  FILE* f;
  for (int i=0; dc_files[i] != NULL; i++) {
    dc_filename_resolved = gw_resolve_filename(dc_files[i]);
    f = fopen(dc_filename_resolved.c_str(), "w");
    if (f) { 
      fclose(f);
    } else {
      gw_do_log(LOG_ERR, "Failed to create DC-API file '%s'", dc_files[i]);
      gw_finish(255);    
    }
  }
#else
  gw_do_log(LOG_INFO, "BOINC enabled version");
#endif

  if (argc < 2)
    gw_do_log(LOG_WARNING, "No workunit script name listed on the command line");
  // Look for & unzip the .zip archive, if any
  std::string filename(basename(argv[0]));

#ifdef _WIN32
  if (filename.compare(filename.length() - 4, 4, ".exe") == 0)
    filename = filename.erase(filename.length() - 4, filename.length()-1);
#endif

  filename.append(".zip");
  std::string zip_filename_resolved = gw_resolve_filename(filename.c_str());
  std::string genwrapper_exe_resolved = gw_resolve_filename(GENWRAPPER_EXE);
  if (!access(zip_filename_resolved.c_str(), R_OK)) {
    const char *zip_argv[] = {
      "unzip", "-o", "-X", zip_filename_resolved.c_str(), 0
    };
    const int zip_argc = sizeof(zip_argv) / sizeof(zip_argv[0]) - 1;

    gw_do_log(LOG_INFO, "Unzipping '%s'", filename.c_str());
    if (unzip_main(zip_argc, (char **)zip_argv)) {
      gw_do_log(LOG_ERR, "Failed to unzip '%s'", zip_filename_resolved.c_str());
      gw_finish(255);
    }
  } else {
    gw_do_log(LOG_INFO, "Zipfile not found '%s'", zip_filename_resolved.c_str());
  }

  // Check for the interpreter
  if (access(genwrapper_exe_resolved.c_str(), X_OK)) {
    gw_do_log(LOG_ERR, "Wrapper executable '%s' is not executable: %s",
	      genwrapper_exe_resolved.c_str(), strerror(errno));
    gw_finish(255);  
  }
  const char *wu_script = argv[1];
  if (wu_script == NULL) {
    gw_do_log(LOG_ERR, "Work unit does not contain work unit script name (should be first command line param), "
	      "going with default name (%s)", WU_SCRIPT);
    wu_script = WU_SCRIPT;
  }
  if (access(wu_script, R_OK)) {
    gw_do_log(LOG_ERR, "Script '%s' does not exist", wu_script);
    gw_finish(255);        
  }

  // create script file which execs profile and the wu supplied (argv[1]) script
  std::ofstream exec_script(EXEC_SCRIPT, std::ios::out);
  exec_script << "set -e\n"
    // profile script is optional
    << "if [ -r ./" PROFILE_SCRIPT " ]; then . ./" PROFILE_SCRIPT "; fi\n"
    << ". `boinc resolve_filename ./" << wu_script << "`\n";
  exec_script.close();
  if (exec_script.fail()) {
    gw_do_log(LOG_ERR, "Failed to create the initialization script");
    gw_finish(255);
  }
  // create task
  TASK gw_task;
  vector<string> args;
  args.push_back(genwrapper_exe_resolved.c_str());
  args.push_back(string("sh"));
  args.push_back(EXEC_SCRIPT);
  for (int i = 2; i < argc; i ++)
    args.push_back(string(argv[i]));
  if (gw_task.run(args) == ERR_EXEC) {
    gw_do_log(LOG_ERR, "Could not exec %s\n", genwrapper_exe_resolved.c_str());
    gw_finish(255);
  }
  
  while(1) {
    int status;
    if (gw_task.poll(status)) {
      if (status) {
	gw_do_log(LOG_ERR, "'%s' exited with error: %d\n", genwrapper_exe_resolved.c_str(), status);
        gw_finish(status, gw_task.final_cpu_time);
      }
      break;
    }
    poll_boinc_messages(gw_task);
    boinc_sleep(POLL_PERIOD);
  }
  gw_finish(0, gw_task.final_cpu_time);
}


#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
  LPSTR command_line;
  char* argv[100];
  int argc;

  command_line = GetCommandLine();
  argc = parse_command_line( command_line, argv );
  return main(argc, argv);
}
#endif

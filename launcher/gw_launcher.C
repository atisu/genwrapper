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
#include <string>
#include <iostream>
#include <fstream>
#include <errno.h>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32
#ifdef WANT_DCAPI
#include "dc_client.h"
#endif // WANT_DCAPI
#include "boinc_api.h"
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
#define GENWRAPPER_EXE  "gitbox.exe"
#else
#define GENWRAPPER_EXE  "./gitbox"
#endif // _WIN32
#define PROFILE_SCRIPT  "profile.sh"
#define EXEC_SCRIPT     "gw_tmp.sh"
#define POLL_PERIOD 1.0

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif


// from unzip.c in libbb
extern "C" {
  int unzip_main(int argc, char **argv);
}


void send_status_message(TASK& task, double frac_done) {
  boinc_report_app_status(1.0, 0.0, frac_done);
}


double read_fraction_done(void) {
  double fraction = 0.0;
  FILE *f = fopen(FILE_FRACTION_DONE, "r");
  if (!f) {
    return 0.0;
  }
  fscanf(f,"%lf", &fraction);
  fclose(f);
  return fraction;
}


int main(int argc, char* argv[]) {
  double frac_done = 0.0; 

  fprintf(stdout, "Launcher for GenWrapper (build date %s)\n", __DATE__);

#ifdef WANT_DCAPI
  fprintf(stdout, "DC-API enabled version\n");
  DC_initClient();
#else
  fprintf(stdout, "BOINC version\n");
  BOINC_OPTIONS options;
  memset(&options, 0, sizeof(options));
  options.main_program = true;
  options.check_heartbeat = true;
  options.handle_process_control = true;
  boinc_init_options(&options);
#endif

  if (argc < 2)
    gw_do_log(LOG_WARNING, "Warging: no script listed on the command line");
  // Look for & unzip the .zip archive, if any
  std::string filename(basename(argv[0]));

#ifdef _WIN32
  if (filename.compare(filename.length() - 4, 4, ".exe") == 0)
    filename = filename.erase(filename.length() - 4, filename.length()-1);
#endif

  filename.append(".zip");
  std::string zip_filename_resolved = gw_resolve_filename(filename.c_str());
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
  if (access(GENWRAPPER_EXE, X_OK)) {
    gw_do_log(LOG_ERR, "Wrapper executable '%s' is not executable: %s",
	      GENWRAPPER_EXE, strerror(errno));
    gw_finish(255);  
  }
  const char *wu_script = argv[1];
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
  args.push_back(GENWRAPPER_EXE);
  args.push_back(string("sh"));
  args.push_back(EXEC_SCRIPT);
  for (int i = 2; i < argc; i ++)
    args.push_back(string(argv[i]));
  if (gw_task.run(args) == ERR_EXEC) {
    gw_do_log(LOG_ERR, "Could not exec %s\n", GENWRAPPER_EXE);
    gw_finish(255);
  }
  
  while(1) {
    int status;
    if (gw_task.poll(status)) {
      if (status) {
	gw_do_log(LOG_ERR, "'%s' exited with error: %d\n", GENWRAPPER_EXE, status);
        gw_finish(status);
      }
      break;
    }
    double frac_done_t = read_fraction_done();
    if (frac_done_t > frac_done)
      frac_done = frac_done_t;
    send_status_message(gw_task, frac_done);
    // no DC-API equivalent, fallback to BOINC API
    boinc_sleep(POLL_PERIOD);
  }
  gw_finish(0);
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

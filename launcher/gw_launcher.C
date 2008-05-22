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

/*
TODO:

DONE - unzip %BASENAME%
- checkpoint (never unzip twice)
DONE - exec profile script
DONE - exec wu script (1st argv - needs to be resolved)
- handle signals
DONE - exit status ?
- should work with DC-API
- should work on WINDOWS (using MINGW)
- test via BOINC
*/

#include <stdio.h>
#include <libgen.h>
#include <vector>
#include <string>
#include <sstream>
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
// from unzip.c in libbb
extern "C" {
    int unzip_main(int argc, char **argv);
}

#ifdef _WIN32
#define GENWRAPPER_EXE  "gitbox.exe"
#else
#define GENWRAPPER_EXE  "./gitbox"
#endif // _WIN32
#define PROFILE_SCRIPT  "profile"
#define EXEC_SCRIPT     "gw_tmp.sh"
#define POLL_PERIOD 1.0

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

int gw_unzip(std::string zip_filename) {
    const char *argv[] = { "unzip", "-o", zip_filename.c_str(), 0 };
    return unzip_main(sizeof(argv) / sizeof(argv[0]) - 1, (char **)argv);
}

void send_status_message(TASK& task, double frac_done) {
    boinc_report_app_status(
        //task.starting_cpu + task.cpu_time(),
        //task.starting_cpu,
        1.0,
        0.0,
        frac_done
    );
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



int main(int argc, char** argv) {
    double frac_done = 0.0; 
    int result;
#ifdef WANT_DCAPI
    DC_initClient();
#else
    BOINC_OPTIONS options;
    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;
    boinc_init_options(&options);
#endif
    if (argc<2) {
        gw_do_log("ERROR: wu supplied shell script name (command line argument 1) is missing");
        gw_finish(255);
    }
    std::string exe_filename(basename(argv[0]));
#ifdef _WIN32
    int loc = exe_filename.rfind(".exe", exe_filename.length());
    if (loc == exe_filename.length()-4)
        exe_filename.erase(loc);
#endif
    std::string zip_filename(exe_filename);
    zip_filename.append(".zip");
    std::string zip_filename_resolved = gw_resolve_filename(zip_filename);
    gw_do_log("resolved zip filename is: %s\n", zip_filename_resolved.c_str());    
    if (gw_file_exist(zip_filename_resolved) != 0)
        gw_finish(255);
    result = gw_unzip(zip_filename_resolved);
    if (result != 0 ) {
        gw_do_log("ERROR: something went wrong during unzipping ('%s')",
         zip_filename_resolved.c_str());
        gw_finish(255);
    }
    if (gw_file_exist(GENWRAPPER_EXE) != 0)
        gw_finish(255);
    std::string wu_script(argv[1]);
    if (gw_file_exist(wu_script) != 0)
        gw_finish(255);
    std::string wu_script_resolved(gw_resolve_filename(wu_script));
    if (gw_file_exist(wu_script_resolved) != 0)
        gw_finish(255);
    // create script file which execs profile and the wu supplied (argv[1]) script
    std::ostringstream exec_script;
    exec_script << "set +e\n"
        << "if [ -r ./" PROFILE_SCRIPT " ]; then . ./" PROFILE_SCRIPT "; fi\n"
        << "sh ./" << wu_script_resolved << " \"$@\"\n"
        // script exits with exit status of the wu script
        << "exit $? \n";    
    if (gw_put_file(EXEC_SCRIPT, exec_script.str()) != 0) {
        gw_do_log("ERROR: error creating main script ('%s')", EXEC_SCRIPT);
    }

    // Mark the interpreter as executable
    chmod(GENWRAPPER_EXE, 0755);

    // create task
    TASK gw_task;
    gw_task.interpreter = GENWRAPPER_EXE;
    gw_task.script = EXEC_SCRIPT;
    gw_task.stdin_filename = "";
    gw_task.stdout_filename = "stdout.txt";
    gw_task.stderr_filename = "stderr.txt";
    gw_task.run(argc, argv);
    while(1) {
        int status;
        if (gw_task.poll(status)) {
            if (status) {
                gw_do_log("app error: 0x%d\n", status);
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

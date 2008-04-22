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
- exit status ?
- should work with DC-API
- should work on WINDOWS (using MINGW)
- test via BOINC
*/

#include <stdio.h>
#include <libgen.h>
#include <vector>
#include <string>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "procinfo.h"
#endif
#include <errno.h>

#ifdef WANT_DCAPI
#include "dc_client.h"
#else
#include "boinc_api.h"
#endif
#include "boinc_zip.h"

#ifdef _WIN32
#define GENWRAPPER_EXE  "gitbox.exe"
#else
#define GENWRAPPER_EXE  "gitbox"
#endif
#define PROFILE_SCRIPT  "profile"
#define EXEC_SCRIPT     "gw_tmp.sh"


#define gw_do_log(FORMAT, ...) \
    fprintf(stderr, "%s[%d]::%s(): ", __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, FORMAT, ## __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    fflush(stderr);

void gw_finish(int status) {
#ifdef WANT_DCAPI
    DC_finish(status);
#else
    boinc_finish(status);
#endif        
}

int gw_file_exist(std::string filename) {
    int result;
    struct stat stat_buf;
    result = stat(filename.c_str(), &stat_buf);
    int my_errno = errno;
    if (result == -1) {
        if (my_errno == ENOENT) {
            gw_do_log("ERROR: file does not exist (%s)", filename.c_str());
            return 255;
        } else {
            gw_do_log("ERROR: unknown error ('%s')", strerror(my_errno));
            return 255;
        }
    }
    return 0;
}

int gw_file_exist(const char *filename) {
    std::string s_filename(filename);
    return gw_file_exist(s_filename);
}


int gw_put_file(char *filename, std::string text) {
    int   retvalue;
    FILE *newfile;

    newfile = fopen(filename, "wt");
    if (!newfile) {
        gw_do_log("ERROR: error creating file (%s)", filename);
        return 255;
    }
    retvalue = fwrite(text.c_str(), sizeof(char), strlen(text.c_str()), newfile);
    fclose(newfile);
    if (retvalue < (signed int)strlen(text.c_str())) {
        gw_do_log("ERROR: %d bytes requested, %d bytes written",
            (int)strlen(text.c_str()), retvalue);
        return 255;    
    }
    return 0;
}

int main(int argc, char** argv) {
    int result;
#ifdef WANT_DCAPI
    DC_init();
#else
    BOINC_OPTIONS options;
    options.main_program = true;
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
#ifdef WANT_DCAPI
    std::string zip_filename_resolved(DC_resolveFilename(DC_FILE_IN, zip_filename.c_str()));
#else
    std::string zip_filename_resolved;
    boinc_resolve_filename_s(zip_filename.c_str(), zip_filename_resolved);
#endif
    printf("%s\n", zip_filename_resolved.c_str());    
    if (gw_file_exist(zip_filename_resolved) != 0)
        gw_finish(255);
    result = boinc_zip(UNZIP_IT, zip_filename_resolved, NULL);
    if (result != 0 ) {
        gw_do_log("ERROR: something went wrong during unzipping ('%s')",
         zip_filename_resolved.c_str());
        gw_finish(255);
    }
    if (gw_file_exist(GENWRAPPER_EXE) != 0)
        gw_finish(255);
    if (gw_file_exist(PROFILE_SCRIPT) != 0)
        gw_finish(255);
    
    std::string wu_script(argv[1]);
    if (gw_file_exist(wu_script) != 0)
        gw_finish(255);
#ifdef WANT_DCAPI
    std::string wu_script_resolved(DC_resolveFilename(DC_FILE_IN, wu_script.c_str()));
#else
    std::string wu_script_resolved;
    boinc_resolve_filename_s(wu_script.c_str(), wu_script_resolved);
#endif
    if (gw_file_exist(wu_script_resolved) != 0)
        gw_finish(255);

    // create script file which execs profile and the wu supplied (argv[1]) 
    // script
    std::string exec_script;
    exec_script += ". ./";
    exec_script += PROFILE_SCRIPT;
    exec_script += "\n";
    exec_script += "./";
    exec_script.append(wu_script_resolved);    
    exec_script += "\n";
    if (gw_put_file(EXEC_SCRIPT, exec_script) != 0) {
        gw_do_log("ERROR: error creating main script ('%s')", EXEC_SCRIPT);
    }
    // exec main script using genwrapper
    std::string system_cmd;
#ifndef _WIN32
    system_cmd += "./";
#endif
    system_cmd += GENWRAPPER_EXE;
    system_cmd += " sh ";
    system_cmd += EXEC_SCRIPT;    
    if (system(system_cmd.c_str()) == -1) {
        gw_do_log("ERROR: failed to start main script");
        gw_finish(255);
    } 
    gw_finish(0);
}


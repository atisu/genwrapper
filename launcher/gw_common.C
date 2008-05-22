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

#include <errno.h>
#ifdef _WIN32
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32
#ifdef WANT_DCAPI
#include "dc_client.h"
//#else
#endif // WANT_DCAPI
#include "boinc_api.h"

#include "gw_common.h"

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


int gw_put_file(const char *filename, std::string text) {
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

std::string gw_resolve_filename(std::string filename) {
#ifdef WANT_DCAPI
    std::string filename_resolved = DC_resolveFileName(DC_FILE_IN, filename.c_str());
#else
    std::string filename_resolved;
    boinc_resolve_filename_s(filename.c_str(), filename_resolved);
#endif
    return filename_resolved;
}

void gw_finish(int status) {
#ifdef WANT_DCAPI
    DC_finishClient(status);
#else
    boinc_finish(status);
#endif        
}

#ifdef _WIN32
// CreateProcess() takes HANDLEs for the stdin/stdout.
// We need to use CreateFile() to get them.  Ugh.
//
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
			0, 0
		);
	} else if (!strcmp(mode, "w")) {
		return CreateFile(
			path,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			&sa,
			OPEN_ALWAYS,
			0, 0
		);
	} else if (!strcmp(mode, "a")) {
		HANDLE hAppend = CreateFile(
			path,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			&sa,
			OPEN_ALWAYS,
			0, 0
		);
        SetFilePointer(hAppend, 0, NULL, FILE_END);
        return hAppend;
	} else {
		return 0;
	}
}
#endif



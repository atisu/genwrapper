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

std::string gw_resolve_filename(const char *filename) {
    std::string filename_resolved;
#ifdef WANT_DCAPI
    char *tmp = DC_resolveFileName(DC_FILE_IN, filename);
    if (tmp)
	filename_resolved = tmp;
    free(tmp);
#else
    boinc_resolve_filename_s(filename, filename_resolved);
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

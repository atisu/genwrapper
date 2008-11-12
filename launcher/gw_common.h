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

#include <stdarg.h>
#include <string>

// DC-API expects some files
#ifdef WANT_DCAPI
#define DC_LABEL_STDOUT "dc_stdout.txt"
#define DC_LABEL_STDERR "dc_stderr.txt"
#define DC_LABEL_CLIENTLOG "dc_clientlog.txt"
#define CKPT_LABEL_OUT "dc_ckpt_out"

#endif // WANT_DCAPI

void gw_do_log(int level, const char *fmt, ...);
void gw_do_vlog(int level, const char *fmt, va_list ap);
std::string gw_resolve_filename(const char *filename);
bool gw_copy_file(const char* src, const char* dst);
void gw_finish(int status, double total_cpu_time = 0);

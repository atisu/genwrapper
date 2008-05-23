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

#include <string>

#ifndef WANT_DCAPI
#define gw_do_log(LEVEL, FORMAT, ...) \
    fprintf(stderr, "%s[%d]::%s(): ", __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, FORMAT, ## __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    fflush(stderr);
#else
#define gw_do_log(LEVEL, FORMAT, ...) DC_log(LEVEL, FORMAT, ## __VA_ARGS__);
#endif

std::string gw_resolve_filename(const char *filename);
void gw_finish(int status);

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

#define gw_do_log(FORMAT, ...) \
    fprintf(stderr, "%s[%d]::%s(): ", __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, FORMAT, ## __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    fflush(stderr);

int gw_file_exist(std::string filename);
int gw_file_exist(const char *filename);
int gw_put_file(const char *filename, std::string text);
std::string gw_resolve_filename(std::string filename);
void gw_finish(int status);
#ifdef _WIN32
HANDLE win_fopen(const char* path, const char* mode);
#endif

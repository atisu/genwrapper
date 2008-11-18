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

#include <string>

using std::vector;
using std::string;

#ifdef _WIN32
// for SuspendProcess/ ResumeProcess
typedef LONG ( NTAPI *_NtSuspendProcess )( IN HANDLE ProcessHandle );
typedef LONG ( NTAPI *_NtResumeProcess )( IN HANDLE ProcessHandle );

static _NtSuspendProcess NtSuspendProcess;
static _NtResumeProcess NtResumeProcess;
#endif

struct TASK {
  double final_cpu_time;
  // how much CPU time was used by tasks before this in the job file
  double starting_cpu;
  bool suspended;
#ifdef _WIN32
  HANDLE hProcess;
  HANDLE hThread;  
  HANDLE hJobObject;
#else
  // process id, also process group id of child 
  int pid;
#endif
  TASK();
  ~TASK();
  bool poll(int& status);
  int run(vector<string> &args);
  void kill();
  void stop();
  void resume();
};

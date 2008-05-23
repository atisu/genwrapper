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
//  original by Andrew J. Younge (ajy4490@umiacs.umd.edu)
//

#include <string>

using std::vector;
using std::string;

struct TASK {
    //string application;
    string stdin_filename;
    string stdout_filename;
    string stderr_filename;
    string interpreter;
    string script;
    double final_cpu_time;
    double starting_cpu;
    // how much CPU time was used by tasks before this in the job file
#ifdef _WIN32
    bool suspended;
    double wall_cpu_time;
        // for estimating CPU time on Win98/ME
    HANDLE pid_handle;
    HANDLE thread_handle;
#else
    int pid;
#endif
    void init();
    /*
    int parse(XML_PARSER&);
    */
    bool poll(int& status);
    int run(int argc, char** argv);
    void kill();
    // will stop and resume only the wrapper not the executed applications
    void stop();
    void resume();
	/*
	double cpu_time();
	*/
};

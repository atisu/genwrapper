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
#include "usage.h"
#include "libbb.h"
#include "boinc_api.h"

//extern BOINC_OPTIONS boinc_options;
//static int boinc_init_called;

// extern int boinc_resolve_filename(const char*, char*, int len);
// extern int boinc_fraction_done(double);

int boinc_resolve_filename_(const char* filename) {
    char buf[PATH_MAX];
    int result;
    result = boinc_resolve_filename(filename, buf, PATH_MAX-1);
    fprintf(stdout, "%s", buf);
    fprintf(stderr, "boinc_resolve_filename('%s','%s') called\n", filename, buf);
    return result;
}

int boinc_fraction_done_(double fraction) {
    return boinc_fraction_done(fraction);
}

int boinc_fraction_done_percent_(int fraction) {
    double d_fraction = fraction / 100;
    return boinc_fraction_done(d_fraction);
}


int boinc_finish_(int status) {
    return boinc_finish(status);
}

int boinc_init_() {
    return 0;
}

int boinc_main(int argc, char **argv);
int boinc_main(int argc, char **argv)
{
    int retval, status;
    int fraction;
    double d_fraction;
    char *endptr;

	retval = EXIT_SUCCESS;
	if (argc<=1) {
        fprintf(stdout, "%s\n", boinc_trivial_usage);
        return retval;
	}

    if (strcmp(argv[1], "resolve_filename")==0) {
        if (argc<=2) {
            fprintf(stderr,"ERROR: missing parameter for resolve_filename\n");
            return retval;
        }
        boinc_resolve_filename_(argv[2]);
    } else if (strcmp(argv[1], "fraction_done_percent")==0) {
        if (argc<=2) {
            fprintf(stderr,"ERROR: missing parameter for fraction_done\n");
            return retval;
        }
        errno=0;
        fraction = strtol(argv[2], &endptr, 10);
        if (errno!=0 || argv[2]==endptr) {
            fprintf(stderr,"ERROR: invalid parameter for fraction_done ('%s'->%d)\n", argv[2], fraction);
            return retval;            
        }
        boinc_fraction_done_percent_(fraction);
        fprintf(stderr,"fraction_done_percent called with fraction==%d\n", fraction);                
    } else if (strcmp(argv[1], "fraction_done")==0) {
        if (argc<=2) {
            fprintf(stderr,"ERROR: missing parameter for fraction_done\n");
            return retval;
        }
        errno=0;
        d_fraction = strtod(argv[2], &endptr);
        if (errno!=0 || argv[2]==endptr) {
            fprintf(stderr,"ERROR: invalid parameter for fraction_done_percent ('%s'->%lf)\n", argv[2], d_fraction);
            return retval;            
        }
        boinc_fraction_done_(d_fraction);
        fprintf(stderr,"fraction_done called with fraction==%lf\n", d_fraction);                
    } else if (strcmp(argv[1], "finish")==0) {
        if (argc<=2) {
            fprintf(stderr,"ERROR: missing parameter for finish\n");
            return retval;
        }
        errno=0;
        status = strtol(argv[2], &endptr, 10);
        if (errno!=0 || argv[2]==endptr) {
            fprintf(stderr,"ERROR: invalid parameter for finish ('%s'->%d)\n", argv[2], status);
            return retval;            
        }
        fprintf(stderr,"boinc_finish(%d) called\n", status);
        boinc_finish_(status);
    } else if (strcmp(argv[1], "init")==0) {
        // boinc_init is invoked by the shell, no reason to invoke it here
        //boinc_init_();
        //fprintf(stderr,"boinc_init() called\n");
    }
	return retval;
}


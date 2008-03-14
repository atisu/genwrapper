# This should work on Linux.  Modify as needed for other platforms.

# Do this first:
# ln -s `g++ -print-file-name=libstdc++.a`
# This creates a symbolic link to the C++ library,
# which is linked statically

# Change the following to match your installation
BOINC_DIR = c:/Projects/boinc_mingw
BOINC_API_DIR = $(BOINC_DIR)
BOINC_LIB_DIR = $(BOINC_DIR)

CXXFLAGS = -g \
    -I../ \
    -I$(BOINC_DIR)/include \
    -L$(BOINC_DIR) \
    -L. \
    -DWIN32 -D_WIN32 -D_MT -DNDEBUG -D_WINDOWS -DBOINC -DCLIENT -DNODB -D_CONSOLE -fexceptions

PROGS = launcher

all: $(PROGS)

clean:
	rm $(PROGS) *.o

launcher: launcher.o $(BOINC_DIR)/libboinc.a 
	g++ $(CXXFLAGS) -o launcher launcher.o -lboinc -lstdc++

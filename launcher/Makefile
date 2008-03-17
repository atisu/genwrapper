# This should work on Linux.  Modify as needed for other platforms.

# Do this first:
# ln -s `g++ -print-file-name=libstdc++.a`
# This creates a symbolic link to the C++ library,
# which is linked statically

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')  
ifneq (,$(findstring MINGW,$(uname_S)))                                                                                                                                                                                 
    MINGW=YesPlease                                                                                                                                                                                                 
endif

# Change the following to match your installation
ifdef MINGW
BOINC_DIR = c:/Projects/boinc_mingw
BOINC_API_DIR = $(BOINC_DIR)
BOINC_LIB_DIR = $(BOINC_DIR)
EXE_SUFFIX=.exe
else
BOINC_DIR = /home/atisu/boinc/source/boinc_core_release_5_10_45
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib
endif

LDFLAGS= \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(BOINC_DIR) \
    -L. 
ifdef MINGW
LOADLIBES = -lboinc -lstdc++
else
LOADLIBES = -lboinc_api -lboinc -lpthread
endif

CXXFLAGS = -g \
    -I../ \
    -I$(BOINC_DIR)/include \
    -I$(BOINC_API_DIR) \
    -I$(BOINC_LIB_DIR) \
    -DBOINC
ifdef MINGW
CXXFLAGS +=  -DWIN32 -D_WIN32 -D_MT -DNDEBUG -D_WINDOWS -DCLIENT -DNODB -D_CONSOLE -fexceptions
endif

PROGS = launcher

all: $(PROGS)

clean:
	rm -rf $(PROGS)$(EXE_SUFFIX) $(PROGS).o

#launcher: launcher.o  
#	g++ $(CXXFLAGS) -o launcher launcher.o  $(LDFLAGS) 

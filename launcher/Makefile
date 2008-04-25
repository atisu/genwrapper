# This should work on Linux.  Modify as needed for other platforms.

# Do this first:
# ln -s `g++ -print-file-name=libstdc++.a`
# This creates a symbolic link to the C++ library,
# which is linked statically

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')  
ifneq (,$(findstring MINGW,$(uname_S)))
    MINGW=YesPlease
endif

#CC=g++

# Change the following options to match your installation
#WANT_DCAPI=YesPlease
## Options for Mingw
ifdef MINGW
DCAPI_DIR=../../../dcapi_mingw/trunk
DCAPI_INCLUDE=\
    -I$(DCAPI_DIR)/include\
    -I$(DCAPI_DIR)/boinc
BOINC_DIR = c:/Projects/boinc_mingw
BOINC_API_DIR = $(BOINC_DIR)
BOINC_LIB_DIR = $(BOINC_DIR)
EXE_SUFFIX=.exe
## Options for Linux/ OS X
else
DCAPI_DIR=../../../../dcapi/trunk/
DCAPI_INCLUDE=\
    -I$(DCAPI_DIR)/include\
    -I$(DCAPI_DIR)/boinc
BOINC_DIR = /home/atisu/boinc/source/boinc_core_release_5_10_45
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_ZIP_DIR = $(BOINC_DIR)/zip
BOINC_LIB_DIR = $(BOINC_DIR)/lib
endif


LDFLAGS= \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(BOINC_DIR) \
    -L../box/ \
    -L. 

ifdef MINGW
LOADLIBES = -lboinc -lbox -lstdc++
else
LOADLIBES = -lbox -lboinc_api -lboinc -lpthread 
endif

CXXFLAGS = -g \
    -I../ \
    -I$(BOINC_DIR)/include \
    -I$(BOINC_API_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I../box/include/ \
    -DBOINC \
	-DUSE_GLIBC_ERRNO

CFLAGS = -I../box/include/ \
	-DUSE_GLIBC_ERRNO


ifdef WANT_DCAPI
CXXFLAGS += -DWANT_DCAPI $(DCAPI_INCLUDE)
endif

ifdef MINGW
CXXFLAGS +=  -DWIN32 -D_WIN32 -D_MT -DNDEBUG -D_WINDOWS -DCLIENT -DNODB -D_CONSOLE -fexceptions
endif

SOURCES = gw_launcher.C \
	task.C \
	gw_common.C

SOURCES_C = ../usage.c \
	../ctype.c
	
OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS = $(SOURCES:.C=.o) 

PROG = gw_launcher$(EXE_SUFFIX)


all: $(OBJECTS) $(OBJECTS_C) $(PROG)

$(PROG): $(OBJECTS) $(OBJECTS_C) ../box/libbox.a
	$(CXX) $(OBJECTS_C) $(OBJECTS) $(LDFLAGS) $(LOADLIBES) -o $(PROG)

clean:
	rm -rf $(PROG) $(OBJECTS) $(OBJECTS_C)


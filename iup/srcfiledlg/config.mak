PROJNAME = iup
LIBNAME = iupfiledlg
OPT = YES

INCLUDES =  ../include ../src ../src/win
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup shlwapi

SRC = iupwin_newfiledlg.cpp

# It does not compile under MingW
# Only Visual C++ is supported

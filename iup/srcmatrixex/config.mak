PROJNAME = iup
LIBNAME = iupmatrixex
OPT = YES

INCLUDES = ../include ../src .

SRC = iupmatrixex.c 

LIBS = iupcontrols
LDIR = ../lib/$(TEC_UNAME)
USE_CD = Yes

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif

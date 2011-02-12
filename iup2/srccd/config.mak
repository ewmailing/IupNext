PROJNAME = iup
LIBNAME = iupcd
OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
SRC = iupcd.c
DEF_FILE = iupcd.def

USE_CD=YES
# Can not use USE_IUP because Tecmake will include "iupcd" in linker

INCLUDES = ../include
LIBS = iup
LDIR = ../lib/$(TEC_UNAME)

ifeq ($(findstring Win, $(TEC_SYSNAME)), )
  CDSUFX := x11
endif

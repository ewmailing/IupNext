PROJNAME = iup
LIBNAME  = iup_plot
OPT = YES

USE_CD = Yes

ifdef DBG
  DEFINES += IUP_ASSERT
  ifndef DBG_DIR
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBNAME := $(LIBNAME)_debug
    endif
  endif  
endif  

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iupcd cdgl

DEFINES = CD_NO_OLD_INTERFACE

SRC = iup_plot.cpp  iup_plot2.cpp

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif

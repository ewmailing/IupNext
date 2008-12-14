PROJNAME = iup
LIBNAME  = iup_pplot
OPT = YES

USE_CD = Yes

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iupcd

DEFINES = _IUP_PPLOT_ CD_NO_OLD_INTERFACE

SRC = PPlot.cpp  PPlotInteraction.cpp  iup_pplot.cpp

ifneq ($(findstring owc, $(TEC_UNAME)), )
  CPPFLAGS = -xr -xst
endif

ifeq "$(TEC_UNAME)" "vc6"
  INCLUDES += D:\LNG\STLport\include
endif

PROJNAME = iup
LIBNAME  = iupim
OPT = YES

USE_IM = Yes 

ifdef DBG
  DEFINES += IUP_ASSERT
endif  

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

SRC = iupim.c

ifneq "$(TEC_SYSNAME)" "Win32"
  USE_MOTIF = Yes
endif

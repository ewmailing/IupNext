PROJNAME = iup
LIBNAME  = iupluagl3
OPT = YES

USE_LUA  = Yes
USE_OPENGL = Yes
                     
SRC = iuplua_glcanvas.c

INCLUDES = ../include
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iuplua3 iupgl

ifeq ($(findstring Win, $(TEC_SYSNAME)), )
  USE_MOTIF = Yes
endif

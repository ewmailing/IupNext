PROJNAME = iup
LIBNAME  = iupluagl3
OPT = YES

USE_LUA  = Yes
USE_OPENGL = Yes
                     
SRC = luaglcanvas.c

INCLUDES = ../include
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iuplua3 iupgl

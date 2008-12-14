PROJNAME = iup
LIBNAME  = iupluaole3
OPT = YES

USE_LUA  = Yes
USE_OPENGL = Yes
                     
SRC    = luaole.c

INCLUDES = ../include
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iuplua3 iupole

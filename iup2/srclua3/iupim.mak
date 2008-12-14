PROJNAME = iup
LIBNAME  = iupluaim3
OPT = YES

USE_LUA  = Yes
USE_IM = Yes
                     
SRC = luaim.c

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iuplua3 iupim

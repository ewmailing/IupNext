PROJNAME = iup
LIBNAME = iupluaimglib
OPT = YES

SRC = iuplua_imglib.c
DEF_FILE = iupluaimglib.def

INCLUDES = ../include
LIBS = iupimglib
LDIR = ../lib/$(TEC_UNAME)

IUP := ..

USE_IUP3 = Yes
USE_IUPLUA = Yes
NO_LUALINK = Yes

ifdef USE_LUA52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
endif

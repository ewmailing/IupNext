PROJNAME = iup
LIBNAME = iupluaimglib

OPT = YES
NO_LUALINK = Yes
LUAMOD_DIR = Yes

SRC = iupluaimglib.c
DEF_FILE = iupluaimglib5.def

INCLUDES = ../include
LIBS = iupimglib
LDIR = ../lib/$(TEC_UNAME)

IUP := ..

USE_IUPLUA = Yes

ifdef USE_LUA52
  LUASFX = 52
  DEFINES += LUA_COMPAT_MODULE
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
LIBNAME := $(LIBNAME)$(LUASFX)

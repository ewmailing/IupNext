PROJNAME = iup
LIBNAME  = iupluaim

OPT = YES
NO_LUALINK = Yes

DEF_FILE = iupluaim5.def
SRC = iupluaim.c

INCLUDES = ../src
LIBS = iupim

IUP := ..

USE_IUPLUA = Yes
USE_IM = Yes

ifdef USE_LUA52
  LUASFX = 52
  DEFINES += LUA_COMPAT_MODULE
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
LIBNAME := $(LIBNAME)$(LUASFX)

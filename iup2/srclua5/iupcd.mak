PROJNAME = iup
LIBNAME = iupluacd

OPT = YES
NO_LUALINK = Yes

DEFINES = CD_NO_OLD_INTERFACE
SRC = iupluacd.c
DEF_FILE = iupluacd5.def

# Can not use USE_IUPLUA because Tecmake will include "iupluacd5X" in linker

INCLUDES = ../include
LDIR = ../lib/$(TEC_UNAME)

IUP := ..

USE_CD = YES
USE_IUP = YES
USE_CDLUA = YES

ifdef USE_LUA52
  LUASFX = 52
  DEFINES += LUA_COMPAT_MODULE
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
LIBNAME := $(LIBNAME)$(LUASFX)

LIBS = iuplua$(LUASFX)

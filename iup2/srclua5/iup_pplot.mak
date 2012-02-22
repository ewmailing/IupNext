PROJNAME = iup
LIBNAME = iuplua_pplot

OPT = YES
NO_LUAOBJECT = Yes
NO_LUALINK = Yes
USE_BIN2C_LUA = Yes

DEF_FILE = iuplua_pplot5.def

IUP := ..

USE_IUPLUA = Yes
USE_CDLUA = Yes
LIBS = iup_pplot

ifdef USE_LUA52
  LUASFX = 52
  DEFINES += LUA_COMPAT_MODULE
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
LIBNAME := $(LIBNAME)$(LUASFX)

DEFINES += IUPLUA_USELH
USE_LH_SUBDIR = Yes
LHDIR = lh

SRCLUA = pplot.lua
EC = pplotfuncs.c
GC := $(addsuffix .c, $(basename $(SRCLUA)))

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC := $(GC) $(EC)

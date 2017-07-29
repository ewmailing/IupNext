PROJNAME = iup
LIBNAME = iupluaole

OPT = YES
NO_LUALINK = Yes
USE_BIN2C_LUA = Yes
NO_LUAOBJECT = Yes
LUAMOD_DIR = Yes

DEF_FILE = iupluaole5.def

IUP := ..

USE_IUPLUA = Yes
LIBS = iupole

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

SRCLUA = olecontrol.lua
GC = $(addsuffix .c, $(basename $(SRCLUA)))

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC	= $(GC) $(EC)

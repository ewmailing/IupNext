PROJNAME = iup
LIBNAME = iupluacontrols

IUP := ..

OPT = YES
NO_LUAOBJECT = Yes
NO_LUALINK = Yes
USE_BIN2C_LUA = Yes

USE_IUPLUA = Yes
USE_CDLUA = Yes

LIBS = iupcontrols
DEF_FILE = iupluacontrols5.def

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

SRCLUA = dial.lua gauge.lua colorbrowser.lua colorbar.lua matrix.lua tree.lua cells.lua val.lua tabs.lua
EC = mask.c controls.c treefuncs.c matrixfuncs.c getparam.c gc.c
GC := $(addsuffix .c, $(basename $(SRCLUA)))

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC := $(GC) $(EC)
